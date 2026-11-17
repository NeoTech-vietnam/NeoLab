#!/usr/bin/env python3
"""Check actual SDK configuration and placement of the migrated firmware."""
import argparse
import json
from pathlib import Path
import re


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def check(build):
    project = json.loads((build / "project_description.json").read_text())
    config = json.loads((build / "config/sdkconfig.json").read_text())
    require(project["target"] == "esp32s3", "Expected ESP32-S3 target")
    require(project.get("git_revision") in ("v6.0.1", "v6.0.1-dirty"), "Expected ESP-IDF v6.0.1 release checkout")
    for component in ("BSW", "Rte", "ApplicationLayer"):
        require(component in project["build_component_info"], f"Missing component: {component}")
    require(config["SPIRAM"] and config["SPIRAM_MODE_OCT"], "Octal PSRAM must be enabled")
    require(config["SPIRAM_SPEED"] in (40, 80), "Unsupported PSRAM clock")
    require(config["ESP_DEFAULT_CPU_FREQ_MHZ"] in (160, 240), "Unsupported CPU clock")
    require(config["CAMERA_JPEG_MODE_FRAME_SIZE_CUSTOM"], "Native JPEG buffer must have an explicit size")
    require(config["CAMERA_JPEG_MODE_FRAME_SIZE"] == 1048576, "Expected 1 MiB native JPEG buffers")
    require(config["ESPTOOLPY_FLASHSIZE"] == "8MB", "Expected 8 MB flash")
    require(not config.get("ESPTOOLPY_OCT_FLASH", False), "Octal flash is incompatible with this board")
    require(config["ESPTOOLPY_FLASHMODE"] == "dio", "Expected DIO flash mode")
    boot_config = json.loads((build / "bootloader/config/sdkconfig.json").read_text())
    for key in ("ESPTOOLPY_FLASHSIZE", "ESPTOOLPY_FLASHMODE", "ESPTOOLPY_FLASHFREQ"):
        require(boot_config.get(key) == config.get(key), f"Stale bootloader configuration: {key}")
    require(not boot_config.get("ESPTOOLPY_OCT_FLASH", False), "Bootloader still selects octal flash")
    memory_map = (build / (project["project_name"] + ".map")).read_text()
    for object_name in ("NeoCam_Composition.c.obj", "HttpSrv_Cfg.c.obj"):
        match = re.search(r"\.rodata\.neocam\s+0x([0-9a-f]+)\s+0x[0-9a-f]+[^\n]*\(" +
                          re.escape(object_name) + r"\)", memory_map)
        require(match is not None, f"Missing mapped section for {object_name}")
        require(0x3C000000 <= int(match[1], 16) < 0x3E000000, "Expected ESP32-S3 flash DROM")
    match = re.search(r"\.bss\.slots\s+0x([0-9a-f]+)\s+0x[0-9a-f]+[^\n]*\(MediaBuf.c.obj\)", memory_map)
    require(match is not None and 0x3FC00000 <= int(match[1], 16) < 0x40000000,
            "Pool metadata must be in internal DRAM")
    binary = build / project["app_bin"]
    require(binary.is_file() and binary.stat().st_size > 0, "Missing/empty application binary")
    return binary.stat().st_size


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("build", nargs="?", default="build-esp32s3")
    args = parser.parse_args()
    try:
        size = check(Path(args.build))
    except (RuntimeError, OSError, KeyError, ValueError) as error:
        parser.exit(1, f"FAIL: {error}\n")
    print(f"PASS build configuration and selected static memory placement; app={size} bytes")


if __name__ == "__main__":
    main()
