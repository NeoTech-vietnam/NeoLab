# NeoCam camera demo

ESP-IDF **6.0.1**, XIAO ESP32-S3 Sense, VGA JPEG camera preview over a local SoftAP.
The demo is organized as ApplicationLayer → hand-written RTE → BSW. Camera,
Wi-Fi, HTTP and FreeRTOS types stay behind BSW interfaces. This is the documented
AUTOSAR R24-11 adaptation profile in [architecture.md](docs/architecture.md), not
a complete AUTOSAR implementation.

## Build and flash

Open an **ESP-IDF 6.0.1** terminal (or source that SDK's `export.sh`), then:

```sh
idf.py -B build-esp32s3 build
idf.py -B build-esp32s3 -p /dev/ttyACM0 flash monitor
```

The equivalent Make targets are `make build` and
`make PORT=/dev/ttyACM0 flash-monitor`. Specify the actual port; exit the monitor
with **Ctrl+]**. Windows users can run the same `idf.py` commands in the ESP-IDF
terminal with a port such as `COM3`. A first build requires the component registry.
The manifest and lock pin camera **2.1.7** and JPEG **1.3.1**.

`sdkconfig.defaults` selects **8 MB quad flash in DIO mode** and **40 MHz octal
PSRAM**. The connected board was verified with esptool on 2026-11-16: 8 MB flash,
quad flash eFuse configuration, and 8 MB embedded PSRAM. The old sample enabled
octal flash incorrectly; this was corrected after its startup timing failure.
PSRAM is required. Generated `sdkconfig` and build output are ignored.

The initial source, configuration hashes, and installed SDK patch were preserved
locally under `.migration-baseline/` before migration. See [validation.md](docs/validation.md)
for evidence and hardware checks still pending.

## Configure

Edit the module's C configuration, then rebuild:

| Configuration | Location |
|---|---|
| Camera pins, VGA, JPEG quality 12, two native buffers | `BSW/CDD/CamIo/CamIo_Cfg.c` |
| SoftAP credentials, channel, two associated stations | `BSW/ComStack/WifiStack/WifiMgr/WifiMgr_Cfg.c` |
| HTTP port, socket timeout, stream stack, shutdown deadline | `BSW/ComStack/WifiStack/HttpSrv/HttpSrv_Cfg.c` |
| Two 256 KiB copied-JPEG slots | `BSW/Infrastruc/BswLib/MediaBuf/MediaBuf_Cfg.h` |
| Startup order and static port connections | `STConf/ConfAr/NeoCam_Composition.c` |

`menuconfig` configures the SDK/backend. Wi-Fi credentials are now owned by C
configuration rather than duplicate Kconfig options. Invalid or oversized settings
are rejected. NVS initialization errors are logged without erasing the partition.

## Use

Connect to **XIAO-Camera**, password **xiao-camera**, accept the no-Internet warning,
and open **http://192.168.4.1/**. No router is required for this demo.

| Endpoint | Response |
|---|---|
| `/` | Dashboard with detected sensor identity and live preview |
| `/capture` | One original JPEG |
| `/stream` | Multipart MJPEG; one active stream, additional streams receive HTTP 503 |
| `/metrics` | Width, height, JPEG bytes, captures, sent stream frames, failures, capture latency, uptime, lifetime-average stream FPS |

Streaming runs in an asynchronous worker so the server can serve metrics and
captures concurrently. Each acquired native framebuffer is copied and returned
before any network send. Oversized JPEGs fail rather than being truncated. Pool
exhaustion returns busy/503 for capture; a stream yields and retries. A slow client
can retain a copied slot, never a native camera frame.

`latest_capture_ms` measures native camera acquisition wait; it excludes copying
and network sending. `captures` counts acquired capture responses, even if sending
later fails. `stream_frames` counts successful frame sends. `failures` counts
acquisition/validation failures, not client disconnects or temporary pool pressure.
`stream_fps` is an average since the first stream opened, including idle intervals.

## Verify

```sh
python3 tests/run_host_tests.py
ASAN_OPTIONS=detect_leaks=0 python3 tests/run_host_tests.py --sanitize
```

The host suite compiles the real portable modules and HTTP adapter against backend
fakes. It checks header isolation, types, configuration, failure rollback, pool
ownership, endpoint data, async completion, and shutdown retries. The sanitizer
command disables LeakSanitizer because this runtime uses ptrace; explicit resource
counters are still checked. It does not replace board testing.

See [validation.md](docs/validation.md) for board checks and
[architecture.md](docs/architecture.md) for module contracts and sequence diagrams.
Microphone, SD recording, Wi-Fi Station, Mesh, and full AUTOSAR driver modules are
outside this migration.
