idf_component_register(
    SRCS
        "Infrastruc/BootCtrl/BootCtrl.c"
        "Infrastruc/BootCtrl/BootCtrl_EspIdf.c"
        "Infrastruc/BswLib/MediaBuf/MediaBuf.c"
        "Infrastruc/BswLib/MediaBuf/MediaBuf_EspIdf.c"
        "SysSrv/EbOs/EbOs_EspIdf.c"
        "SysSrv/LocalTime/LocalTime_EspIdf.c"
        "CDD/CamIo/CamIo.c"
        "CDD/CamIo/CamIo_Cfg.c"
        "CDD/CamIo/CamIo_EspIdf.c"
        "CDD/CamPipe/CamPipe.c"
        "CDD/CamPipe/CamPipe_EspIdf.c"
        "../STConf/CONFIG/CameraProfiles/Neo_Profiles.c"
        "ComStack/WifiStack/WifiMgr/WifiMgr.c"
        "ComStack/WifiStack/WifiMgr/WifiMgr_Config.c"
        "ComStack/WifiStack/WifiMgr/WifiMgr_Cfg.c"
        "ComStack/WifiStack/WifiIf/WifiIf_EspIdf.c"
        "ComStack/WifiStack/HttpSrv/HttpSrv.c"
        "ComStack/WifiStack/HttpSrv/HttpSrv_Cfg.c"
        "ComStack/WifiStack/HttpSrv/HttpSrv_EspIdf.c"
    INCLUDE_DIRS
        "Infrastruc/CUCEL/Platform"
        "Infrastruc/CUCEL/Standard"
        "Infrastruc/CUCEL/MemMap"
        "Infrastruc/BootCtrl"
        "Infrastruc/BswLib/MediaBuf"
        "SysSrv/EbOs"
        "SysSrv/LocalTime"
        "CDD/CamIo"
        "CDD/CamPipe"
        "ComStack/WifiStack/WifiMgr"
        "ComStack/WifiStack/HttpSrv"
        "../STConf/GLOBAL"
        "../STConf/CONFIG/MemMap_Conf"
    PRIV_INCLUDE_DIRS "ComStack/WifiStack/WifiIf"
    PRIV_REQUIRES esp_event esp_http_server esp_netif esp_psram esp_timer esp_wifi
                  nvs_flash freertos heap espressif__esp32-camera
)

# These overrides are benchmark builds; default presets remain hand-written C.
set(NEOCAM_BENCH_JPEG_QUALITY -1 CACHE STRING "JPEG quality override for controlled experiments")
set(NEOCAM_BENCH_SMOOTH_FPS 30 CACHE STRING "Smooth preset FPS cap (30 or experimental 60)")
set(NEOCAM_PRODUCER_CORE -1 CACHE STRING "Producer affinity: -1, 0, 1")
set(NEOCAM_WIFI_CHANNEL 1 CACHE STRING "SoftAP benchmark channel")
set(NEOCAM_WIFI_BANDWIDTH 20 CACHE STRING "Configured SoftAP bandwidth (20 or 40 MHz)")
if(NOT NEOCAM_BENCH_JPEG_QUALITY MATCHES "^(-1|[0-9]+)$" OR
   NEOCAM_BENCH_JPEG_QUALITY LESS -1 OR NEOCAM_BENCH_JPEG_QUALITY GREATER 63)
    message(FATAL_ERROR "NEOCAM_BENCH_JPEG_QUALITY must be -1 or 0..63")
endif()
if(NOT NEOCAM_BENCH_SMOOTH_FPS MATCHES "^(30|60)$")
    message(FATAL_ERROR "Smooth benchmark FPS must be 30 or 60")
endif()
if(NOT NEOCAM_PRODUCER_CORE MATCHES "^(-1|0|1)$")
    message(FATAL_ERROR "Invalid producer core")
endif()
if(NOT NEOCAM_WIFI_CHANNEL MATCHES "^[0-9]+$" OR
   NEOCAM_WIFI_CHANNEL LESS 1 OR NEOCAM_WIFI_CHANNEL GREATER 11)
    message(FATAL_ERROR "SoftAP channel must be 1..11")
endif()
if(NOT NEOCAM_WIFI_BANDWIDTH MATCHES "^(20|40)$")
    message(FATAL_ERROR "SoftAP bandwidth must be 20 or 40")
endif()
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    NEOCAM_BENCH_JPEG_QUALITY=${NEOCAM_BENCH_JPEG_QUALITY}
    NEOCAM_BENCH_SMOOTH_FPS=${NEOCAM_BENCH_SMOOTH_FPS}
    NEOCAM_PRODUCER_CORE=${NEOCAM_PRODUCER_CORE}
    NEOCAM_WIFI_CHANNEL=${NEOCAM_WIFI_CHANNEL}
    NEOCAM_WIFI_BANDWIDTH=${NEOCAM_WIFI_BANDWIDTH})
