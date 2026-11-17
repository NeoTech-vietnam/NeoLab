#!/usr/bin/env python3
"""Build isolated, cumulative optimization candidates without changing the baseline."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
ORDER = ("cpu240", "psram80", "perf", "dma", "cores", "network")
EXPECTED_IDF = "6.0.1"


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def git_output(directory, *arguments):
    try:
        result = subprocess.run(["git", "--no-optional-locks", "-C", str(directory), *arguments],
                                capture_output=True, text=True, check=False)
    except OSError:
        return None
    return result.stdout.strip() if result.returncode == 0 else None


def git_identity(directory):
    if not any((parent / ".git").exists() for parent in (directory, *directory.parents)):
        return {"head": None, "dirty": None, "diff_sha256": None}
    status = git_output(directory, "status", "--porcelain", "--untracked-files=normal", "--", ".")
    diff = git_output(directory, "diff", "--binary", "HEAD", "--", ".")
    return {"head": git_output(directory, "rev-parse", "HEAD"),
            "dirty": bool(status) if status is not None else None,
            "diff_sha256": hashlib.sha256(diff.encode()).hexdigest() if diff is not None else None}


def sdk_identity(sdk):
    version_file = sdk / "tools/cmake/version.cmake"
    text = version_file.read_text()
    import re
    parts = [re.search(r"set\(IDF_VERSION_" + name + r"\s+(\d+)\)", text)
             for name in ("MAJOR", "MINOR", "PATCH")]
    require(all(parts), "Cannot read ESP-IDF release version")
    version = ".".join(part.group(1) for part in parts)
    require(version == EXPECTED_IDF, f"ESP-IDF {EXPECTED_IDF} required; found {version}")
    identity = {"version": version, "version_file_sha256": digest(version_file), **git_identity(sdk)}
    if (sdk / ".git").exists():
        release = git_output(sdk, "rev-parse", "--verify", f"refs/tags/v{EXPECTED_IDF}^{{commit}}")
        require(identity["head"] is not None and identity["head"] == release,
                f"SDK checkout must have HEAD at the v{EXPECTED_IDF} release tag (local edits are recorded)")
        identity["release_commit"] = release
        identity["verification"] = "release_tag"
    else:
        identity["verification"] = "version_file"
    return identity


def source_identity():
    extensions = {".c", ".h", ".py", ".cmake", ".defaults", ".yml", ".yaml", ".lock", ".lf", ".csv"}
    names = {"CMakeLists.txt", "Makefile", ".gitignore"}
    files = []
    for base, directories, filenames in os.walk(ROOT):
        directories[:] = [name for name in directories if not name.startswith("build") and
                          name not in {"managed_components", ".git", ".migration-baseline", ".agents",
                                       ".codex", ".aws", ".build", "__pycache__"}]
        for name in filenames:
            path = Path(base) / name
            if path.suffix in extensions or name in names:
                files.append(path)
    source_hash = hashlib.sha256()
    for path in sorted(files):
        source_hash.update(path.relative_to(ROOT).as_posix().encode() + b"\0")
        source_hash.update(path.read_bytes() + b"\0")
    return {"sha256": source_hash.hexdigest(), "files": len(files), **git_identity(ROOT)}


def write_manifest(build, manifest):
    temporary = build / "variant.json.tmp"
    temporary.write_text(json.dumps(manifest, indent=2) + "\n")
    temporary.replace(build / "variant.json")


def validate_config(config, selected):
    expected = {
        "ESP_DEFAULT_CPU_FREQ_MHZ": 240 if "cpu240" in selected else 160,
        "SPIRAM_SPEED": 80 if "psram80" in selected else 40,
        "CAMERA_PSRAM_DMA": "dma" in selected,
        "COMPILER_OPTIMIZATION_PERF": "perf" in selected,
        "COMPILER_OPTIMIZATION_ASSERTIONS_ENABLE": True,
        "CAMERA_JPEG_MODE_FRAME_SIZE": 1048576,
        "ESPTOOLPY_FLASHSIZE": "8MB",
        "ESPTOOLPY_FLASHMODE": "dio",
    }
    if "cores" in selected:
        expected["CAMERA_CORE1"] = True
    if "network" in selected:
        expected.update(LWIP_TCP_SND_BUF_DEFAULT=16384, ESP_WIFI_DYNAMIC_TX_BUFFER_NUM=48)
    for key, value in expected.items():
        actual = config.get(key, False if isinstance(value, bool) else None)
        require(actual == value, f"Candidate configuration mismatch: {key}={actual!r}; expected {value!r}")
    require(not config.get("ESPTOOLPY_OCT_FLASH", False), "Candidate must use quad/DIO flash")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("variant", choices=("baseline", *ORDER))
    parser.add_argument("--quality", type=int, choices=range(64), metavar="0..63")
    parser.add_argument("--smooth-fps", type=int, choices=(30, 60), default=30)
    parser.add_argument("--channel", type=int, choices=range(1, 12), default=1)
    parser.add_argument("--bandwidth", type=int, choices=(20, 40), default=20)
    parser.add_argument("--port", help="also flash the candidate to this explicitly selected device")
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--configure-only", action="store_true", help="validate candidate settings without claiming a binary build")
    args = parser.parse_args()
    if args.configure_only and args.port:
        parser.error("--configure-only cannot flash")
    sdk_path = os.environ.get("IDF_PATH")
    if not sdk_path:
        parser.error("export ESP-IDF 6.0.1 first (IDF_PATH is unset)")
    sdk = Path(sdk_path)
    selected = [] if args.variant == "baseline" else list(ORDER[:ORDER.index(args.variant) + 1])
    suffix = f"-q{args.quality}" if args.quality is not None else ""
    if args.smooth_fps != 30:
        suffix += f"-fps{args.smooth_fps}"
    if args.channel != 1 or args.bandwidth != 20:
        suffix += f"-ch{args.channel}-ht{args.bandwidth}"
    build = ROOT / f"build-perf-{args.variant}{suffix}"
    defaults = [ROOT / "sdkconfig.defaults", *[ROOT / "STConf/TOOLCONF/variants" / (x + ".defaults") for x in selected]]
    command = [sys.executable, str(sdk / "tools/idf.py"), "-B", str(build),
               "-D", "IDF_TARGET=esp32s3", "-D", f"SDKCONFIG={build / 'sdkconfig'}",
               "-D", "SDKCONFIG_DEFAULTS=" + ";".join(map(str, defaults)),
               "-D", f"NEOCAM_BENCH_JPEG_QUALITY={args.quality if args.quality is not None else -1}",
               "-D", f"NEOCAM_BENCH_SMOOTH_FPS={args.smooth_fps}",
               "-D", f"NEOCAM_PRODUCER_CORE={1 if 'cores' in selected else -1}",
               "-D", f"NEOCAM_WIFI_CHANNEL={args.channel}",
               "-D", f"NEOCAM_WIFI_BANDWIDTH={args.bandwidth}"]
    command += ["reconfigure" if args.configure_only else "build"]
    print(json.dumps({"command": command, "experimental": True}, indent=2), flush=True)
    if args.dry_run:
        return
    build.mkdir(parents=True, exist_ok=True)
    manifest = {"variant": args.variant, "overlays": selected, "experimental": True,
                "performance_validated": False, "quality_override": args.quality,
                "smooth_fps": args.smooth_fps, "sdkconfig": str(build / "sdkconfig"),
                "state": "pending", "build_completed": False, "flash_completed": False,
                "attempt_utc": datetime.now(timezone.utc).isoformat(), "command": command}
    manifest.update(channel=args.channel, configured_bandwidth_mhz=args.bandwidth,
                    flash_requested=bool(args.port))
    # Replace old success provenance before reconfiguration/build can change any
    # artifacts. A failed or interrupted attempt must never reuse that claim.
    write_manifest(build, manifest)
    try:
        manifest["sdk"] = sdk_identity(sdk)
        manifest["source"] = source_identity()
        subprocess.run(command, cwd=ROOT, check=True)
        config_path = build / "config/sdkconfig.json"
        config = json.loads(config_path.read_text())
        validate_config(config, selected)
        after = source_identity()
        require(after["sha256"] == manifest["source"]["sha256"] and
                after["head"] == manifest["source"]["head"], "Source changed during candidate build")
        manifest["sdkconfig_sha256"] = digest(build / "sdkconfig")
        manifest["config_json_sha256"] = digest(config_path)
        cache = build / "CMakeCache.txt"
        if cache.exists():
            manifest["cmake_cache_sha256"] = digest(cache)
        if not args.configure_only:
            project = json.loads((build / "project_description.json").read_text())
            binary = build / project["app_bin"]
            require(binary.is_file(), "Build returned without an application binary")
            manifest["binary"] = {"path": str(binary), "sha256": digest(binary), "bytes": binary.stat().st_size}
        manifest.update(state="configured" if args.configure_only else "built",
                        build_completed=not args.configure_only)
        write_manifest(build, manifest)
        if args.port:
            subprocess.run([sys.executable, str(sdk / "tools/idf.py"), "-B", str(build),
                            "-p", args.port, "flash"], cwd=ROOT, check=True)
            manifest.update(flash_completed=True, state="flashed")
            write_manifest(build, manifest)
    except Exception as error:
        manifest.update(state="failed", error=str(error))
        write_manifest(build, manifest)
        raise


if __name__ == "__main__":
    main()
