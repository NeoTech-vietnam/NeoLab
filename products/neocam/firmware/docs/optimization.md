# Preview and photo optimization

Implemented 2026-11-17. Hardware qualification is pending: no USB serial device was
visible during this implementation. All `performance_validated` flags remain false.
Builds and host tests establish software behavior, not achievable camera or Wi-Fi FPS.

## Profiles and controls

| Profile | JPEG dimensions | Quality | Stream target | Use |
|---|---:|---:|---:|---|
| safe | 640×480 | 12 | 30 FPS | Boot/default and recovery selection |
| smooth | 640×480 | 16 | 30 FPS | Smaller JPEGs for preview |
| balanced | 1280×720 | 12 | 30 FPS | HD preview candidate |
| detail | 1600×1200 | 10 | 10 FPS | More detail with lower cadence |
| photo | 2048×1536 | 8 | Single image | Full-resolution still |

These are requests, not measured capabilities. JPEG quality ranges from 0 to 63;
lower numbers request higher quality. The sensor creates the JPEG; streaming sends
that original payload without software recompression. Image size depends strongly
on scene detail, motion, lighting and sensor exposure. Preview slots accept at most
512 KiB and photo slots 1 MiB. A larger JPEG fails visibly; it is never truncated.

Use the dashboard, or:

```sh
curl -H 'Content-Type: application/json' -d '{"profile":"balanced"}' http://192.168.4.1/api/profile
curl http://192.168.4.1/api/profile
curl --max-time 25 http://192.168.4.1/photo -o photo.jpg
```

POST returns 202 on admission; GET must subsequently show the requested active
profile, `state:"running"` and zero errors. Competing mode/photo operations return
busy. A still temporarily stops preview, captures into its dedicated slot and
restores the previous preview profile before the response. An existing stream
waits through the change and resumes with a new epoch. Restore failures stay
visible in status rather than reporting a successful photo. A failed profile
switch attempts to restore the previous profile; SAFE is also manually selectable.
Profile selection is fixed until another request or reboot; quality never adapts
automatically to throughput.

The producer continuously replaces one latest publication. Consumer references
keep individual immutable frames alive, while later frames supersede the publisher's
reference. This bounds project buffer retention and avoids an application frame
backlog; TCP and receiver buffering still affect observed latency. One stream and
one image response are allowed concurrently. Per-frame stream send timeout is
500 ms; image send timeout is 5 s after acquisition returns, excluding sensor
switching, capture and restoration. Neither timeout restarts on partial writes.

## Build candidates

Keep the normal build as the comparison baseline: CPU 160 MHz, octal PSRAM 40 MHz,
PSRAM DMA off, 8 MB **quad DIO** flash at 80 MHz. The camera native JPEG allocation
is explicitly two 1 MiB buffers for these profiles. Copied payload capacity is
3 MiB, so native plus copied capacity is about 5 MiB before other overhead.
PSRAM headroom and largest contiguous blocks require board measurements.

Export ESP-IDF 6.0.1, then use the isolated builder. Each variant has its own build
directory and generated sdkconfig; it does not rewrite the normal sdkconfig.

| Variant | Adds to the previous variant |
|---|---|
| baseline | Current producer implementation with baseline SDK settings |
| cpu240 | CPU 240 MHz |
| psram80 | PSRAM 80 MHz |
| perf | Compiler performance optimization; assertions remain enabled |
| dma | Native camera PSRAM DMA |
| cores | Camera driver and producer affinity on core 1 |
| network | TCP send buffer 16 KiB; dynamic Wi-Fi TX buffers 48 |

```sh
python3 STConf/TOOLCONF/performance.py baseline
python3 STConf/TOOLCONF/performance.py cpu240
python3 STConf/TOOLCONF/performance.py psram80
python3 STConf/TOOLCONF/performance.py perf
python3 STConf/TOOLCONF/performance.py dma
python3 STConf/TOOLCONF/performance.py cores
python3 STConf/TOOLCONF/performance.py network --channel 6 --bandwidth 40
```

Measure after each step using the same scene, distance, lighting, host and channel.
Changing RF channel/bandwidth is a separate experiment: the last example changes
both, so compare it with channel 6/20 MHz and with the preceding variant under the
same RF settings before attributing gains. Configured 40 MHz does not establish
negotiated bandwidth or higher throughput. Retain 20 MHz where interference or
client behavior makes it better.

Additional experiments are explicit:

```sh
python3 STConf/TOOLCONF/performance.py network --quality 16 --smooth-fps 60 --dry-run
python3 STConf/TOOLCONF/performance.py network --quality 16 --smooth-fps 60
python3 STConf/TOOLCONF/check_build.py build-perf-network-q16-fps60
```

The quality override applies to **all** profiles, including photos; it is intended
for controlled sweeps. The 60 FPS override changes only smooth's target. It does
not prove the OV3660, driver or receiver can sustain that rate.
`--configure-only` validates generated SDK settings without claiming a built image.
The builder writes SDK/source/configuration/binary identity to `variant.json` and
invalidates old success provenance before each real attempt. Build and flash
completion are separate fields; failed/interrupted attempts cannot reuse an older
success claim. No variant is
promoted to default by building it. Flash a checked candidate by explicitly supplying
`--port /dev/ttyACM0`, or use that directory with `idf.py -B ... -p ... flash`.
No partition erase is required.

## Measurements

`/metrics` preserves the old fields and adds producer and transport measurements.

| Fields | Meaning |
|---|---|
| `capture_frames`, `capture_fps_1s` | Fresh frames published and producer window rate |
| `stream_frames`, `send_fps_1s` | Successful sends and count in the last second |
| `stream_fps` | Legacy lifetime average since first stream, including idle time |
| `captures`, `photos` | Successful image callbacks, with photos a subset of captures; sending may subsequently fail |
| `latest_capture_ms`, `acquire_p95_us`, `copy_p95_us` | Native acquisition wait and copy timing, excluding socket send |
| `send_p95_us`, `send_interval_p95_us` | Send duration and successful-send completion interval |
| `frame_age_p95_us` | Camera timestamp to firmware send completion; excludes receiver decode/display |
| `superseded_frames` | Replaced publications and producer discards; replacements can have been consumed |
| `stream_skipped_frames` | Cumulative sequence gaps between successful sends within an epoch, including gaps across reconnects; not necessarily network loss |
| `capture_failures`, `oversized_frames` | Producer/mode-operation errors and oversized JPEGs |
| `failures` | Application acquisition errors; temporary busy states and disconnects are excluded |
| `free_psram`, `largest_psram`, `min_free_psram` | Current PSRAM, largest block and lifetime minimum |
| `free_internal`, `min_free_internal`, `producer_stack_bytes` | Internal heap and producer stack watermark |
| `pool_outstanding`, `pool_references` | Occupied slots and all references, including the publisher |
| `last_operation_us`, `last_photo_pause_us` | Last mode operation and last still pause, excluding photo socket send |
| `cpu_mhz`, `psram_mhz`, `psram_dma` | Compiled backend settings |
| `wifi_channel`, `wifi_configured_bandwidth_mhz` | Configured AP RF settings |

Percentiles use bounded recent 64-sample rings, not the entire session. Send rings
reset on the first successful send of a new epoch. A ring may retain old samples while idle.
Producer timing rings and JPEG extrema reset when a mode operation returns to
running; producer frame/error counters remain cumulative until producer restart.
Heap minima are allocator lifetime minima, including startup. Send cadence does
not establish capture cadence, and neither establishes camera-to-screen latency.

Each MJPEG part includes `X-Sequence`, `X-Capture-Time-Us` and `X-Profile-Epoch`.
Sequences increase across mode changes. Epochs advance when a profile is set,
including preview restoration, before settling frames are checked; an epoch alone
does not establish successful settling or hardware qualification.

## Receiver benchmark and acceptance

On a host connected to XIAO-Camera, install Pillow into the host's Python environment
for full JPEG decoding. The receiver bounds multipart headers, JPEG size and sample
storage. It polls metrics concurrently and writes JSON even when the run fails.

```sh
python3 tests/benchmark_camera.py --profile smooth --seconds 300 --scene good-light-detail --output results/smooth-detail.json
python3 tests/benchmark_camera.py --profile smooth --seconds 300 --scene motion --output results/smooth-motion.json
python3 tests/benchmark_camera.py --profile smooth --seconds 300 --scene plain --output results/smooth-plain.json
python3 tests/benchmark_camera.py --profile balanced --seconds 1800 --scene motion --output results/balanced-soak.json
python3 tests/benchmark_camera.py --profile safe --seconds 3600 --photos 100 --scene good-light-detail --output results/photo-soak.json
python3 tests/benchmark_camera.py --profile safe --seconds 60 --read-delay-ms 100 --output results/slow-client.json
```

Repeat the three scene runs for every preview preset and candidate. Preserve binary,
configuration and variant hashes with the reports; also record board/sensor identity,
power source, room lighting, host model and RF conditions. Keep the known-flashed
pre-optimization image set in `.migration-baseline/pre-optimization-images/` for an
A/B comparison; its `sha256.json` identifies those local recovery artifacts. The
legacy image has no profile/sequence API, so its receiver result is diagnostic only.

The automated stream gate requires at least 300 s, full decode at the requested
dimensions, strictly increasing frame identities/timestamps, unique received FPS
≥95% of the firmware-reported target (including experimental 60 FPS), p95 receive
interval ≤two target periods, no capture/oversize errors or metric resets, and at
least 512 KiB PSRAM / 64 KiB internal heap headroom. Short, unspecified/low-light,
slow-client, interrupted or legacy runs cannot qualify. A 100-photo run must decode
every full-resolution JPEG and observe preview restoration after each one. Photo
pauses remain included in whole-run cadence; inspect `stream_pass` and `photo_pass`
separately. The tool never updates firmware qualification flags.

Before promoting defaults, complete 30-minute streaming and 60-minute mixed/photo
soaks with no reboot or increasing resource count, 20 disconnect/reconnect cycles,
503 admission checks, and stop/start cleanup tests. Deliberately slow clients must
lose their connection when a backpressured frame reaches its send deadline while
metrics/control remain responsive. This bound starts with that frame's send,
not when the stream connection was first opened.
Visually compare still detail, noise and exposure in the same scene; automatic JPEG
decoding alone does not establish image quality. Measure display latency separately
with a visible timing reference if needed. Select the fastest stable configuration
that meets these gates and image-quality goals, then record the evidence before
changing defaults or marking a preset validated.
