#!/usr/bin/env python3
"""Compile real portable modules against small backend fakes; no external deps."""
import argparse
import os
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "tests/.build"
INCLUDES = [
    "STConf/GLOBAL", "STConf/CONFIG/MemMap_Conf", "STConf/ConfAr",
    "BSW/Infrastruc/CUCEL/Platform", "BSW/Infrastruc/CUCEL/Standard",
    "BSW/Infrastruc/CUCEL/MemMap", "BSW/Infrastruc/BootCtrl",
    "BSW/Infrastruc/BswLib/MediaBuf", "BSW/CDD/CamIo",
    "BSW/SysSrv/EbOs", "BSW/SysSrv/LocalTime",
    "BSW/ComStack/WifiStack/WifiMgr", "BSW/ComStack/WifiStack/WifiIf",
    "BSW/ComStack/WifiStack/HttpSrv", "Rte", "ApplicationLayer/ASW/CamHmi",
    "ApplicationLayer/ASW/CamDiag", "ApplicationLayer/DE/CamDev",
]
CASES = {
    "boot_ctrl": ["BSW/Infrastruc/BootCtrl/BootCtrl.c"],
    "nvs": ["BSW/Infrastruc/BootCtrl/BootCtrl_EspIdf.c"],
    "camera_buffers": ["BSW/CDD/CamIo/CamIo.c", "BSW/CDD/CamIo/CamIo_Cfg.c",
                       "BSW/Infrastruc/BswLib/MediaBuf/MediaBuf.c"],
    "wifi_config": ["BSW/ComStack/WifiStack/WifiMgr/WifiMgr_Config.c",
                    "BSW/ComStack/WifiStack/WifiMgr/WifiMgr_Cfg.c"],
    "wifi_manager": ["BSW/ComStack/WifiStack/WifiMgr/" + name for name in
                     ("WifiMgr.c", "WifiMgr_Config.c", "WifiMgr_Cfg.c")],
    "http_srv": ["BSW/ComStack/WifiStack/HttpSrv/" + name for name in
                 ("HttpSrv.c", "HttpSrv_Cfg.c", "HttpSrv_EspIdf.c")],
    "application": ["Rte/Rte.c", "ApplicationLayer/ASW/CamHmi/CamHmi.c",
                    "ApplicationLayer/ASW/CamDiag/CamDiag.c", "ApplicationLayer/DE/CamDev/CamDev.c"],
}


def architecture_checks(cc, flags, includes):
    forbidden = re.compile(r'\b(esp_err_t|camera_fb_t|httpd_req_t|TaskHandle_t|SemaphoreHandle_t|FILE)\b')
    public = []
    for folder in ("ApplicationLayer", "Rte", "BSW", "STConf"):
        for header in (ROOT / folder).rglob("*.h"):
            if header.name.endswith(("_Backend.h", "_Internal.h", "MemMap.h")):
                continue
            text = header.read_text()
            assert not forbidden.search(text), f"Native type in {header}"
            public.append(header)
    for folder in ("ApplicationLayer", "BSW"):
        for source in (ROOT / folder).rglob("*.[ch]"):
            text = source.read_text()
            if folder == "ApplicationLayer":
                assert not re.search(r'#include\s*[<"](?:esp_|freertos/|CamIo|MediaBuf|Wifi|HttpSrv|EbOs|LocalTime)', text), source
            else:
                assert not re.search(r'#include\s*"(?:Rte|CamHmi|CamDiag|CamDev)', text), source
    for number, header in enumerate(public):
        source = BUILD / f"header_{number}.c"
        source.write_text(f'#include "{header}"\nint main(void) {{ return 0; }}\n')
        subprocess.run([cc, *flags, *includes, "-fsyntax-only", str(source)], check=True)
    source = BUILD / "platform_check.c"
    source.write_text('''#include "Neo_Types.h"
_Static_assert(sizeof(uint8)==1 && sizeof(uint16)==2 && sizeof(uint32)==4 && sizeof(uint64)==8, "integer widths");
_Static_assert(sizeof(float32)==4 && sizeof(float64)==8, "float widths");
_Static_assert(CPU_TYPE==CPU_TYPE_32 && CPU_BYTE_ORDER==LOW_BYTE_FIRST, "ESP32-S3 platform");
int main(void) { return E_OK; }
''')
    subprocess.run([cc, *flags, *includes, "-fsyntax-only", str(source)], check=True)
    for number, markers in enumerate((
        ("NEOCAM_STOP_SEC_CONST_UNSPECIFIED",),
        ("NEOCAM_START_SEC_CONST_UNSPECIFIED", "NEOCAM_START_SEC_VAR_CLEARED_UNSPECIFIED"),
        ("NEOCAM_START_SEC_CONST_UNSPECIFIED", "NEOCAM_STOP_SEC_VAR_CLEARED_UNSPECIFIED"),
    )):
        source = BUILD / f"invalid_memmap_{number}.c"
        source.write_text("".join(f'#define {marker}\n#include "MemMap.h"\n' for marker in markers))
        result = subprocess.run([cc, *flags, *includes, "-fsyntax-only", str(source)], capture_output=True)
        assert result.returncode != 0, f"Invalid section markers accepted: {markers}"
    print(f"PASS architecture and {len(public)} independent public headers", flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sanitize", action="store_true", help="enable address/undefined sanitizers")
    args = parser.parse_args()
    BUILD.mkdir(exist_ok=True)
    cc = os.environ.get("CC", "cc")
    flags = ["-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic", "-g"]
    if args.sanitize:
        flags += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-no-pie"]
    includes = ["-I" + str(ROOT / folder) for folder in INCLUDES]
    architecture_checks(cc, flags, includes)
    for name, sources in CASES.items():
        binary = BUILD / name
        fake_includes = ["-I" + str(ROOT / "tests/fakes_http")] if name in ("http_srv", "nvs") else []
        if name == "nvs": fake_includes.insert(0, "-I" + str(ROOT / "tests/fakes_nvs"))
        command = [cc, *flags, *fake_includes, *includes,
                   str(ROOT / f"tests/test_{name}.c"), *[str(ROOT / p) for p in sources], "-o", str(binary)]
        subprocess.run(command, check=True)
        subprocess.run([str(binary)], check=True)
        print(f"PASS {name}", flush=True)


if __name__ == "__main__":
    main()
