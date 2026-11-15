#!/usr/bin/env python3
"""Check actual SDK configuration and placement of the migrated firmware."""
import argparse
import json
from pathlib import Path
import re

parser = argparse.ArgumentParser()
parser.add_argument("build", nargs="?", default="build-esp32s3")
args = parser.parse_args()
build = Path(args.build)
project = json.loads((build / "project_description.json").read_text())
config = json.loads((build / "config/sdkconfig.json").read_text())
assert project["target"] == "esp32s3"
for component in ("BSW", "Rte", "ApplicationLayer"):
    assert component in project["build_component_info"], component
assert config["SPIRAM"] and config["SPIRAM_MODE_OCT"]
assert config["SPIRAM_SPEED"] == 40
assert config["ESPTOOLPY_FLASHSIZE"] == "8MB"
assert not config.get("ESPTOOLPY_OCT_FLASH", False)
assert config["ESPTOOLPY_FLASHMODE"] == "dio"
boot_config = json.loads((build / "bootloader/config/sdkconfig.json").read_text())
for key in ("ESPTOOLPY_FLASHSIZE", "ESPTOOLPY_FLASHMODE"):
    assert boot_config[key] == config[key], f"Stale bootloader configuration: {key}"
assert not boot_config.get("ESPTOOLPY_OCT_FLASH", False)
memory_map = (build / (project["project_name"] + ".map")).read_text()
for object_name in ("NeoCam_Composition.c.obj", "HttpSrv_Cfg.c.obj"):
    match = re.search(r"\.rodata\.neocam\s+0x([0-9a-f]+)\s+0x[0-9a-f]+[^\n]*\(" +
                      re.escape(object_name) + r"\)", memory_map)
    assert match, f"Missing mapped section for {object_name}"
    assert 0x3C000000 <= int(match[1], 16) < 0x3E000000, "Expected ESP32-S3 flash DROM"
match = re.search(r"\.bss\.slots\s+0x([0-9a-f]+)\s+0x[0-9a-f]+[^\n]*\(MediaBuf.c.obj\)", memory_map)
assert match and 0x3FC00000 <= int(match[1], 16) < 0x40000000, "Pool metadata must be in internal DRAM"
binary = build / project["app_bin"]
print(f"PASS build configuration and memory placement; app={binary.stat().st_size} bytes")
