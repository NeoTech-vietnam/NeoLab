# NeoCam camera demo

ESP-IDF **6.0.1**, XIAO ESP32-S3 Sense / OV3660, JPEG preview and full-resolution
photos over a local SoftAP.
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
| Camera pins, XCLK, two native buffers | `BSW/CDD/CamIo/CamIo_Cfg.c` |
| Fixed preview/photo profiles, quality and target FPS | `STConf/CONFIG/CameraProfiles/Neo_Profiles.c` |
| Producer, warm-up and operation limits | `BSW/CDD/CamPipe/CamPipe_Cfg.h` |
| SoftAP credentials, channel, bandwidth, two associated stations | `BSW/ComStack/WifiStack/WifiMgr/WifiMgr_Cfg.c` |
| HTTP port, socket timeout, worker stack, send deadlines and shutdown deadline | `BSW/ComStack/WifiStack/HttpSrv/HttpSrv_Cfg.c` |
| Four 512 KiB preview slots and one 1 MiB photo slot | `BSW/Infrastruc/BswLib/MediaBuf/MediaBuf_Cfg.h` |
| Startup order and static port connections | `STConf/ConfAr/NeoCam_Composition.c` |

`menuconfig` configures the SDK/backend. Wi-Fi credentials are now owned by C
configuration rather than duplicate Kconfig options. Invalid or oversized settings
are rejected. NVS initialization errors are logged without erasing the partition.

## Use

Connect to **XIAO-Camera**, password **xiao-camera**, accept the no-Internet warning,
and open **http://192.168.4.1/**. No router is required for this demo.

| Endpoint | Response |
|---|---|
| `/` | Dashboard, fixed profile selection, live preview and photo download |
| `/capture` | Latest preview JPEG at the selected profile |
| `/photo` | 2048×1536 JPEG; pauses preview and restores its profile before a successful JPEG response |
| `/stream` | Multipart MJPEG; one active stream, additional streams receive HTTP 503 |
| `/metrics` | Capture/send cadence, timing percentiles, frame gaps, heap/stack, errors and build settings |
| `GET /api/profile` | Active/requested profile, state, errors and available presets |
| `POST /api/profile` | JSON `{"profile":"smooth"}`; 202 means accepted, poll GET for completion |

One producer acquires native JPEGs continuously, copies them into bounded PSRAM
slots and publishes only the latest frame. It returns native buffers before any
network send. Stream and image responses use separate bounded asynchronous
workers, leaving the server available for controls and metrics. Only one image
request (`/capture` or `/photo`) is admitted at a time. Oversized JPEGs fail rather
than being truncated; slow clients cannot build a frame queue.

Boot selects **safe** (640×480, JPEG quality 12). Other presets are **smooth**
(640×480/q16), **balanced** (1280×720/q12), and **detail** (1600×1200/q10).
Their FPS values are targets; all presets remain `performance_validated:false`.
Lower JPEG quality numbers request better quality and usually larger files.
No automatic quality adjustment occurs. See [optimization.md](docs/optimization.md)
for metrics, experimental CPU/PSRAM/DMA builds and reproducible hardware tests.

## Verify

The complete host suite needs a C compiler, Python 3 and Pillow for full JPEG
decoder tests. No board or ESP-IDF installation is needed for these host tests.

```sh
python3 tests/run_host_tests.py
ASAN_OPTIONS=detect_leaks=0 python3 tests/run_host_tests.py --sanitize
```

The host suite compiles the real portable modules and HTTP adapter against backend
fakes. It checks header isolation, types, configuration, failure rollback, pool
ownership, producer/profile transitions, endpoint data, async completion, send
deadlines and shutdown retries. Python tests check candidate build commands,
benchmark framing/qualification, and a local HTTP stream/photo exchange. The sanitizer
command disables LeakSanitizer because this runtime uses ptrace; explicit resource
counters are still checked. It does not replace board testing.

See [validation.md](docs/validation.md) for board checks and
[architecture.md](docs/architecture.md) for module contracts and sequence diagrams.
Microphone, SD recording, Wi-Fi Station, Mesh, and full AUTOSAR driver modules are
outside this migration.
