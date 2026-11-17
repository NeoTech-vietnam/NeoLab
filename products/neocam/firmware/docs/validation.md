# Migration validation

Migration checks: 2026-11-15. First hardware bring-up: 2026-11-16.
This report separates software checks from board acceptance.

## Preview/photo implementation — 2026-11-17

The optimization changes are implemented; they have **not been flashed or measured
on hardware**. No ESP USB device or `/dev/ttyACM*`/`ttyUSB*` port was visible during
the final checks. The 2026-11-16 boot evidence below belongs to the earlier image.
It must not be treated as validation of the new producer, modes, photos or DMA.

Current software gates:

| Gate | Result and scope |
|---|---|
| Host C regression | Eight groups pass strict C11 and address/undefined sanitizers; 26 public headers compile independently |
| Buffer ownership | Four preview slots plus a separate photo slot; reference lifetime, stale/double tokens, exhaustion, exact native returns and profile reconfiguration checked with fakes |
| Producer/modes | Latest-frame sequencing, epoch changes, warm-up, switch rollback, photo restoration, abandoned results and stop retries checked with deterministic backend fakes |
| HTTP | Separate bounded stream/image workers; admission, partial-write deadlines, disconnect/shutdown release and retry paths checked against SDK fakes |
| Pacing | Quantized 10 ms RTOS clock: 301 frames at 30 FPS and 601 at 60 FPS span 10 s ±10 ms; tests also check epoch changes and slow-send recovery without bursts |
| Build tooling | 11 Python tests: cumulative overlays/options, stale-manifest invalidation, source-change detection, SDK/config/binary provenance, validation under `python -O` |
| Receiver tooling | 10 Python tests: fragmented/bounded multipart parsing, full JPEG decoding, firmware-reported target FPS, monotonic identities, qualification rejection cases, and a real localhost stream/photo/restoration exchange |
| Dashboard | Generated script passes Node syntax checking; HTML/JSON capacity and application dispatch checked by host tests |
| Native firmware | Both baseline and combined experimental candidate build with ESP-IDF 6.0.1 and pass configuration/static-map checks |

The complete host command is now `python3 tests/run_host_tests.py`; Pillow is needed
for decoder/tooling tests. The sanitizer run uses `ASAN_OPTIONS=detect_leaks=0`
for the existing ptrace limitation. Fake resource counters still check cleanup.
Localhost tests validate the host receiver and HTTP framing; they do not exercise
the ESP32 radio, sensor, DMA/cache behavior or physical task scheduling.

| Local artifact | Application size | Free in 1 MiB app partition | Settings |
|---|---:|---:|---|
| `build-esp32s3/xiao_esp32s3_camera_stream.bin` | 935232 bytes | 113344 bytes | CPU 160 MHz, PSRAM 40 MHz, PSRAM DMA off |
| `build-perf-network-ch6-ht40/xiao_esp32s3_camera_stream.bin` | 913744 bytes | 134832 bytes | CPU 240 MHz, PSRAM 80 MHz, performance optimization, DMA, core 1, network overlay, channel 6/configured 40 MHz |

SHA256 of the baseline application:
`d931f0c78d356f8ff67fa8a0469c90cdfe3b6665792cb5cfe7e554c2f3aa4a86`.
SHA256 of the combined candidate:
`72c81a9e40f0fb4c74d4bba57a71dd8c6f1084efac99829362bf0d073a04470c`.
Both retain 8 MB quad DIO flash at 80 MHz and explicit 1 MiB native JPEG buffers.
An isolated `performance.py baseline --configure-only` run also generated and
validated the intended baseline settings from defaults; that separate directory
is marked `state:"configured"`, `build_completed:false` and contains no claimed binary.
The map checker verifies selected constants in flash and pool metadata in internal
DRAM; it does not establish runtime payload/headroom or fragmentation results.

The candidate `variant.json` records source/configuration/binary hashes, SDK
release tag and local SDK edits, `build_completed:true`, `flash_completed:false`
and `performance_validated:false`. A failed rebuild invalidates an earlier success
manifest before changing build artifacts. The unchanged installed SDK HEAD matches
the v6.0.1 release tag; its pre-existing local edit remains recorded.

The pre-optimization source and known-flashed image set are preserved locally in
`.migration-baseline/pre-optimization.tar.gz` and `pre-optimization-images/`.
The latter's SHA256 manifest matches the earlier flashed application fingerprint
below. These ignored local archives are recovery/comparison artifacts, not files
provided by a fresh checkout.

No achievable FPS, optical image-quality improvement, PSRAM headroom, sustained DMA
stability or photo/reconnect/soak acceptance is claimed. Every preset remains
`performance_validated:false`. Follow [optimization.md](optimization.md) for exact
profile/build commands, the receiver tests and remaining qualification gates.

## First hardware bring-up — 2026-11-16

Flashed the connected ESP32-S3 rev v0.2 on `/dev/ttyACM0`; bootloader, partition
table and application writes all passed esptool hash verification. The initial
image stopped in flash timing setup because the inherited sample enabled octal
flash. Read-only flash identification reported manufacturer `c8`, device `4017`,
8 MB capacity and **quad** flash eFuse configuration at 3.3 V.

Corrected sdkconfig.defaults and the active configuration to **DIO / 8 MB flash**,
retaining 80 MHz flash and 40 MHz octal PSRAM. Rebuilt and reflashed bootloader and
application. The build checker now checks both application and bootloader settings
to catch stale bootloader configuration. No NVS erase was performed. The original
active configuration is retained locally as `.migration-baseline/pre-hardware-sdkconfig`.

Verified startup in [the serial log](hardware-boot-2026-11-16.log):

- DIO flash boot with 8 MB configured capacity.
- 8 MB PSRAM detected at 40 MHz; SRAM memory test passed.
- OV3660 detected, PID `0x3660`, configured for 640×480 JPEG.
- SoftAP started; DHCP server on `192.168.4.1`.
- Application reported Ready after HTTP initialization.

The driver logged two `NO-SOI` JPEG-start messages during startup, with no further
messages in the following observation. JPEG capture and live-stream quality have
not yet been checked over Wi-Fi; startup alone does not establish those results.

The flashed application is **920048 bytes**, SHA256
`76c0e1c3334cfe51994e74fe83fe7c48350cce90ad09250703edac93c2dd1db4`.
The current build configuration/memory-placement check passes. The older build-verify
directory and binary fingerprint below are historical pre-bring-up evidence and
must be regenerated before using them to validate the corrected configuration.

## Baseline preserved

Before source replacement, `.migration-baseline/original-firmware.tar.gz` saved the
original main, build files, README, manifest, lockfile and active configurations.
`sha256.json` records the originals and `installed-sdk.patch` records the existing
SDK header modification. These local recovery files are ignored by Git; do not
assume they are available in a fresh checkout. No SDK source was modified/reset.

The installed SDK reports `v6.0.1-dirty`; its FreeRTOSConfig.h patch was compared
with HEAD after line continuations/comments/whitespace normalization and the token
streams matched. The patch was retained rather than silently discarded. Validation
here used that installed SDK; an independent clean SDK installation was not tested.

The unmodified demo rebuilt successfully: application size 914544 bytes, with the
existing 1 MiB application partition. Managed camera/JPEG sources initially matched
their distributed SHA256 manifests. Source files in this firmware directory were
untracked at the start, so no existing firmware commit or per-stage rollback commit
was assumed. No Git commit was created by the migration.

## Automated gates — migration completion, before hardware correction

| Gate | Evidence |
|---|---|
| Public interfaces and dependencies | Host runner independently compiles 22 public headers; checks native-type leakage, forbidden includes, integer/float widths and memory-section misuse |
| Startup/rollback | Fault at every synthetic step, including failed-step cleanup; reverse order, primary error retention, cleanup failure, refusal to restart, safe retry |
| NVS | Real initialization adapter with fake NVS: full/incompatible partitions fail without erase; idempotence and deinit retry |
| Camera/pool | Real portable logic with fake backend: allocation failure, exhaustion, exact native returns, invalid/oversized JPEG, copy independence, stale/double token, partial init cleanup and pin validation |
| Wi-Fi | Configuration boundaries, readiness events, state timeout, stop/deinit failure and restart using backend fakes |
| HTTP | Real HTTP adapter with SDK fakes: registration failure, busy/admission limits, async begin/task failures, disconnect/send failures, exact release/completion, socket shutdown ordering, stop timeout and retry |
| RTE/application | Real static dispatch and application modules: detected sensor page, JSON schema/counters, time-zero FPS start, lifetime average, output bounds |
| Firmware | ESP32-S3 build, binary-size check, mapped constant sections and pool metadata placement |

The seven host groups pass both strict C11 compilation (`-Wall -Wextra -Werror
-pedantic`) and address/undefined sanitizer execution. The header checks also reject
unmatched/nested/mismatched MemMap section transitions. A fresh build using a separate
generated sdkconfig is checked in addition to the original build directory.

Both build directories completed successfully. The fresh generated SDK configuration
matches the original effective configuration (zero differing JSON entries). The
application is **922544 bytes**, leaving **126032 bytes (12%)** in the existing
1 MiB application partition. Both linker maps pass the placement/configuration check.
The fresh `build-verify/xiao_esp32s3_camera_stream.bin` SHA256 is
`cab7d65a08a0ce7e7623863ef14e2153474be1040d51000b75fc0b042ee05818`.

Commands:

```sh
python3 tests/run_host_tests.py
ASAN_OPTIONS=detect_leaks=0 python3 tests/run_host_tests.py --sanitize
idf.py -B build-esp32s3 build
python3 STConf/TOOLCONF/check_build.py
idf.py -B build-verify -D SDKCONFIG=build-verify/sdkconfig build
python3 STConf/TOOLCONF/check_build.py build-verify
```

Address/undefined sanitizers run with LeakSanitizer disabled because the task runtime
uses ptrace. Explicit fake allocation/request/lease counters still check cleanup.
HTTP fakes schedule workers deterministically; they do not establish real-network
performance, SMP race freedom, DMA/cache behavior or hardware timing.

## Remaining hardware acceptance gates

No serial device was visible during the initial migration on 2026-11-15. Flash and
startup were subsequently verified above; the remaining acceptance checks must not
be marked PASS from startup, the build or the host suite alone:

1. Record physical board/sensor PID, flash/PSRAM identity, power source, firmware
   binary hash, SDK/configuration and camera dependency versions.
2. Flash with the actual serial port; verify startup reports detected sensor and
   AP readiness. Confirm `/`, JPEG capture, multipart stream and metrics JSON.
3. Hold a stream for five minutes while repeatedly fetching metrics and captures;
   JPEGs must decode and metrics must progress. An additional stream must return 503.
4. Test a deliberately slow stream, abrupt disconnect and reconnect. Perform twenty
   stream open/close cycles. Ensure JPEG leases/native buffers are not exhausted.
5. Exercise NeoCam_Stop()/NeoCam_Start() through a debugger or a board integration
   test harness, including disconnect during stop; these are C lifecycle entrypoints,
   not new public HTTP control endpoints. Record cleanup status and retry behavior.
6. Run a 30-minute stream soak. Record free/largest internal and PSRAM heap blocks,
   task stack watermarks, outstanding copied slots and worker/session counts using
   IDF/debugger instrumentation alongside metrics. Require no reboot or increasing
   outstanding-resource trend; capture throughput/FPS separately, without promising
   a target rate from the host tests.

Example two-terminal checks after joining XIAO-Camera:

```sh
curl --max-time 300 http://192.168.4.1/stream -o /tmp/neocam-stream.mjpeg
```

```sh
curl --max-time 5 http://192.168.4.1/metrics
curl --max-time 10 http://192.168.4.1/capture -o /tmp/neocam-capture.jpg
curl --max-time 5 -i http://192.168.4.1/stream
```

The first curl timing out after its requested five minutes is expected; the saved
multipart content can then be inspected with a media tool. Preserve evidence with
the tested source/configuration; do not infer board acceptance from this checklist.
