# Migration validation

Migration checks: 2026-11-15. First hardware bring-up: 2026-11-16.
This report separates software checks from board acceptance.

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
