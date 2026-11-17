# NeoCam camera-demo architecture

This implementation extends the VGA/SoftAP demo with fixed preview profiles and
OV3660 full-resolution photos using the agreed reference hierarchy.
ESP-IDF 6.0.1, camera 2.1.7 and JPEG 1.3.1 are the pinned backends.
The reference workbook taxonomy is retained; NeoCam-specific packages are additions.
No unused Spi/I2c/NvM, recording, microphone, or SD implementation is claimed.

## Module registry and specification profile

| Package | Interface/ownership | Classification |
|---|---|---|
| Infrastruc/CUCEL/Platform, Standard | CPU/endian/types, returns, switches and version structure | R24-11 standard-type subset; no project types added to Platform_Types.h |
| Infrastruc/CUCEL/MemMap | Repeated section markers, invalid/nested/mismatched transition diagnostics | Selected memory-mapping pattern adapted to GCC/ESP-IDF linker sections |
| Infrastruc/BootCtrl | Serialized start/stop of configured steps; primary and cleanup failures | Project lifecycle adapter, not EcuM or AUTOSAR OS startup |
| Infrastruc/BswLib/MediaBuf | Owns reference-counted preview/photo slots and persistent generation tokens | NeoCam extension; independent of NvM and MemMap |
| SysSrv/EbOs | Opaque mutexes; task delay | FreeRTOS adapter, not an AUTOSAR OS implementation |
| SysSrv/LocalTime | Monotonic microseconds | NeoCam extension; not synchronized StbM time |
| CDD/CamIo | Owns camera/SCCB backend and serialized native frame acquisition | CDD adaptation using esp32-camera directly |
| CDD/CamPipe | Single acquisition producer, latest publication, mode/photo state machine, timing and resource sampling | NeoCam CDD extension; no standard AUTOSAR camera API claimed |
| ComStack/WifiStack/WifiIf | Owns AP netif, event registration, Wi-Fi lifecycle and native diagnostics | ESP-IDF adapter |
| ComStack/WifiStack/WifiMgr | Validates C configuration; waits for AP start/stop events | NeoCam extension; not 802.11p WEth |
| ComStack/WifiStack/HttpSrv | Owns server, one async stream and one async image worker, socket leases and absolute send deadlines | NeoCam HTTP transport extension |
| ApplicationLayer/ASW/CamHmi | Dashboard, capture policy and HTTP application ports | Application component |
| ApplicationLayer/ASW/CamDiag | Metrics and JSON schema, serialized through RTE diagnostic mutex | Application component |
| ApplicationLayer/DE/CamDev | Camera device operations mapped to BSW through RTE | Device encapsulation |
| ApplicationLayer/ASV/CamSupv | Validates requested profile names; photo/profile application ports and status JSON | Application service; command execution belongs to CamPipe |
| Rte | Typed, statically wired provider/client ports | Hand-written adaptation, no ARXML/RTE generator |
| STConf/ConfAr | Composition and startup-step tables | C source, compiled with the main composition component |
| STConf/CONFIG/CameraProfiles | One immutable table for backend, application and diagnostics | Hand-written project configuration |

Standards references: [Platform Types](https://www.autosar.org/fileadmin/standards/R24-11/CP/AUTOSAR_CP_SWS_PlatformTypes.pdf),
[Standard Types](https://www.autosar.org/fileadmin/standards/R24-11/CP/AUTOSAR_CP_SWS_StandardTypes.pdf),
[Memory Mapping](https://www.autosar.org/fileadmin/standards/R24-11/CP/AUTOSAR_CP_SWS_MemoryMapping.pdf),
[BSW General](https://www.autosar.org/fileadmin/standards/R24-11/CP/AUTOSAR_CP_SWS_BSWGeneral.pdf).
These define the selected conventions; the project does not claim full module or
methodology conformance. Legacy Compiler.h abstraction, standard EcuM startup,
AUTOSAR OS, SchM generation and full driver suites are not implemented.

Selected requirement anchors: Platform_00002 (no project extensions), Platform_00044
(CPU width), Platform_00046 (byte order), Platform_00026/00027/00034 (boolean), and
Platform_00041/00042 (floating-point representations). Common headers use the exact
standard spellings; project errors, media descriptors and port tables stay in GLOBAL.
Full transformer/message definitions from Std_Types are outside this demo's subset.

The workbook's Mcal is directly under BSW, separate from IoServices. WifiStack is
a NeoCam addition. A future standard Spi driver belongs under Mcal/Spi_STC/Spi;
the camera currently uses SCCB and parallel DVP, not SPI image transport. The
vendor library's internal SCCB transactions remain inside the CDD boundary.

## Dependencies and execution

```mermaid
flowchart TD
    Main[main + STConf composition] --> Boot[BootCtrl]
    Main --> Wiring[Static application/provider ports]
    HTTP[BSW HttpSrv] -->|configured callbacks| RTE[Rte]
    RTE -->|application ports| HMI[CamHmi]
    HMI -->|device, supervisor and diagnostic ports through RTE| App[CamDev / CamSupv / CamDiag]
    App -->|camera services through RTE| Pipe[BSW CamPipe]
    Pipe --> Camera[BSW CamIo]
    Pipe --> Pool
    Camera --> Pool[BSW MediaBuf]
    Camera --> Native[esp32-camera / SCCB / DVP]
    HTTP --> IDF[ESP-IDF HTTP + FreeRTOS worker]
    Wifi[WifiMgr] --> WiFiIf[WifiIf / ESP-IDF Wi-Fi]
```

Function-pointer connections are installed at startup. They permit calls into the
application without a BSW-to-application header/build dependency. Three grouped
IDF components plus main use explicit source lists; package does not mean task.

The IDF HTTP server task handles page, metrics and profile control. Separate bounded
async workers handle the stream and a single capture/photo response. The CamPipe
producer is the only native acquisition caller after startup. SDK camera/Wi-Fi/network
tasks remain backend details. The stream requests a strictly newer sequence and
uses the profile's target cadence with carried absolute deadlines; missed send
opportunities are skipped. There is no catch-up queue or repeated old frame.

All lifecycle calls require one serialized composition owner and quiescent users
before destruction. Data-path calls may run concurrently. Camera acquisition uses
a task mutex, never a CPU critical section. Network sends never hold that mutex or
the pool/diagnostic mutexes. ISR code never calls these application interfaces.

## Configuration and memory

Module-local Cfg.h/Cfg.c own editable settings. STConf/ConfAr owns wiring; GLOBAL
owns project contracts, CONFIG/MemMap_Conf owns section attributes, and TOOLCONF
owns build checks. There are no ECUC values files or generator requirements.
The SDK's sdkconfig.defaults owns only backend build parameters.

Neo_StatusType carries project results: parameter/state/busy/memory/I/O/timeout/
too-large/not-ready. It does not change the meaning of Std_ReturnType. Native errors
are logged privately; WifiIf also retains its first native error for diagnostics.

MemMap maps configured constants to `.rodata.neocam`, which the IDF linker places
in flash DROM. Module and pool metadata use internal DRAM BSS. Runtime payloads are
allocated explicitly with `MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT`; this is CPU-copy
memory, not a promise of DMA suitability. Driver/DMA buffers stay driver-owned.
Run `python3 STConf/TOOLCONF/check_build.py` to inspect the actual map/configuration.

Four 512 KiB preview slots and one dedicated 1 MiB photo slot are allocated at startup
and freed only with zero live leases. Native JPEG buffers are separately configured
as two 1 MiB buffers: approximately 5 MiB of payload capacity in total, before
driver/network/allocator overhead. Actual heap headroom must be measured on the board.
The default keeps CPU 160 MHz, PSRAM 40 MHz and PSRAM DMA disabled. Experimental
overlays are isolated from that baseline; see [optimization.md](optimization.md).
A token contains a slot and generation. Generations survive service restart
and never wrap to an old token: a slot that reaches UINT32_MAX becomes unavailable.
A lease transfers only on successful acquisition. CamPipe retains the latest
publication; each consumer receives its own reference and releases once after send
success or failure. Replacing the latest frame releases only the publisher reference.
Copied JPEGs stay immutable while referenced. The backend and SDK may still allocate
dynamically; this is not static AUTOSAR memory allocation throughout the SDK.

## Sequences

```mermaid
sequenceDiagram
    participant Main
    participant Boot as BootCtrl
    participant Core as NVS/pool/camera
    participant App as RTE/application
    participant Net as Wi-Fi/producer/HTTP
    Main->>Boot: NeoCam_Start()
    Boot->>Core: Initialize in configured order
    Boot->>App: Connect ports, initialize diagnostics
    Boot->>Net: Initialize AP, wait AP_START, start producer, start HTTP
    alt any step fails
        Boot->>Boot: Preserve primary error and failed stage
        Boot->>Net: Reverse cleanup of attempted/acquired steps
        Boot->>App: Disconnect only after users quiesce
        Boot->>Core: Release lower resources last
    end
    Boot-->>Main: Result and retained cleanup status
```

The failed step is included in rollback because its own partial cleanup can fail.
Cleanup stops at an unquiesced/failing resource, retaining lower dependencies for a
later NeoCam_Stop() retry. Start is rejected while resources remain active.
ESP-IDF already starts FreeRTOS before app_main; this is not EcuM_Init/StartOS.
NVS full/incompatible errors are reported without erasing storage.

```mermaid
sequenceDiagram
    participant Browser
    participant HTTP as HttpSrv server task
    participant Worker as Stream worker
    participant App as RTE/application ports
    participant Pipe as CamPipe producer
    participant Cam as CamIo
    participant Native as esp32-camera
    loop Continuous acquisition
        Pipe->>Cam: CamIo_Capture()
        Cam->>Cam: Reserve preview slot and camera mutex
        Cam->>Native: esp_camera_fb_get()
        Native-->>Cam: Native JPEG
        Cam->>Cam: Validate dimensions/size and copy
        Cam->>Native: esp_camera_fb_return()
        Cam-->>Pipe: Copied frame and generation token
        Pipe->>Pipe: Assign sequence/epoch, replace latest publication
    end
    Browser->>HTTP: GET /stream
    HTTP->>HTTP: Reserve single stream, async begin
    HTTP->>Worker: Create worker with owned request
    HTTP-->>HTTP: Return to serve metrics/capture
    Worker->>App: Request next frame after last sequence
    App->>Pipe: CamPipe_AcquireLatest(after_seq, timeout, frame)
    Pipe->>Pipe: Retain latest immutable slot
    Pipe-->>App: Copied JPEG, sequence, epoch, timestamp, token
    App-->>Worker: Frame descriptor
    Worker->>Browser: MIME header + JPEG
    Worker->>App: Update sent-frame metric, release token
    Note over Worker: Repeat at target cadence; retry transient switching/busy states
    Worker->>Worker: shutdown socket while async lease protects it
    Worker->>HTTP: async handler complete exactly once
    Worker->>Worker: Acknowledge quiescence, self-delete
```

Capture retains the latest preview using the same lease rules in the image worker.
Additional streams or image requests return 503. Capture pool pressure returns 503;
a stream yields and retries. Oversized/invalid frames are returned to the native driver and fail.
Send failures release copied slots and do not increment camera-failure metrics.

```mermaid
sequenceDiagram
    participant Browser
    participant Image as HttpSrv image worker
    participant Sup as CamSupv / RTE
    participant Pipe as CamPipe producer
    participant Cam as CamIo / esp32-camera
    Browser->>Image: GET /photo
    Image->>Sup: Photo application callback
    Sup->>Pipe: CamPipe_Photo(frame)
    Pipe->>Pipe: Drop latest publication, drain preview consumer references
    Pipe->>Cam: Set PHOTO profile, discard settling frames
    Pipe->>Cam: Capture into dedicated photo slot
    Pipe->>Cam: Restore previous preview profile, discard settling frames
    Pipe-->>Sup: Photo lease after successful restoration
    Sup-->>Image: JPEG descriptor
    Image->>Browser: Send JPEG under image deadline
    Image->>Sup: Release photo token exactly once
```

Profile POST admits one command and returns 202 before execution. The producer drops
its publication and waits for old preview references before reinitializing the sensor.
Warm-up requires three fresh, correctly sized frames within its deadline; duplicate
native timestamps are discarded without counting toward those three frames.
Each successful profile-setting call advances the epoch before warm-up, including
restoration. An epoch alone does not establish successful settling or qualification.
A failed change attempts to restore the
previous profile; restoration failure enters ERROR and remains visible in status.
Commands are serialized with photos. A timed-out photo caller never owns a late result;
the producer releases it and still attempts preview restoration. Native acquisition
can block beyond a nominal operation deadline by one backend call; cleanup never
forcibly frees memory still in use.

Shutdown sets the stopping flag, rejects new work, and waits up to the configured
10 s for both async workers. Timeout retains the entire server and dependencies;
no task/buffer is forcibly destroyed. Once the worker is quiescent, httpd_stop()
joins the synchronous server task before ports, camera, pool or Wi-Fi are released.
The deadline governs async quiescence, not a forced deadline on httpd_stop(). Each
stream frame has one absolute 500 ms send deadline; each image response has 5 s
after its acquisition callback returns. Acquisition/restoration time is separate,
so this is not a five-second deadline for the whole photo request.
Nonblocking sends/select avoid resetting that deadline for every partial write.
After HTTP quiesces, composition stops the producer before camera/pool destruction.

## Subsequent work

The original targets for video/audio recording, SD, STA preview, Mesh and power
remain unimplemented. Add their real consumers before standard driver modules;
do not represent native SCCB or SDSPI traffic as Spi/I2C calls that do not occur.
Recording can add a real bounded consumer to the timestamped producer, retaining the
public ownership contract and defining its own backpressure/drop policy.
