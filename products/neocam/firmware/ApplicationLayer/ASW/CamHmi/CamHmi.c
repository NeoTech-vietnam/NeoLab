#include "CamHmi.h"
#include "Rte.h"
#include <stdio.h>

Neo_StatusType CamHmi_Init(void)
{
    Neo_CameraInfoType info;
    Neo_StatusType result = Rte_DeviceInfo(&info);
    if (result == NEO_OK) Rte_DiagInit(&info);
    return result;
}

Neo_StatusType CamHmi_Deinit(void) { return NEO_OK; }

static Neo_StatusType page(char *text, uint32 capacity, uint32 *length)
{
    if (text == NULL_PTR || capacity == 0U || length == NULL_PTR) return NEO_E_PARAM;
    Neo_CameraInfoType info;
    Neo_StatusType result = Rte_DeviceInfo(&info);
    if (result != NEO_OK) return result;
    int size = snprintf(text, capacity,
        "<!doctype html><meta name=viewport content='width=device-width'>"
        "<title>NeoCam DIP Lab</title><style>body{font:16px sans-serif;max-width:800px;margin:auto;padding:16px;background:#111;color:#eee}"
        "img{max-width:100%%;height:auto}code{color:#8df}.pipeline{display:flex;gap:8px;flex-wrap:wrap}.pipeline span{background:#333;padding:8px}</style>"
        "<h1>Chapter 1: Image-processing system</h1><div class=pipeline><span>%s acquisition</span><span>JPEG representation</span>"
        "<span>PSRAM buffering</span><span>Wi-Fi transport</span><span>Browser display</span></div>"
        "<p><a href=/capture>Capture one JPEG</a> | <code id=m>loading metrics...</code></p><img src=/stream alt='Live camera stream'>"
        "<script>setInterval(async()=>{try{document.getElementById('m').textContent=JSON.stringify(await(await fetch('/metrics')).json())}"
        "catch(e){document.getElementById('m').textContent=e}},1000)</script>", info.sensor_name);
    if (size < 0 || (uint32)size >= capacity) return NEO_E_TOO_LARGE;
    *length = (uint32)size;
    return NEO_OK;
}

static Neo_StatusType frame(boolean capture, Neo_FrameType *value)
{
    Neo_StatusType result = Rte_DeviceCapture(value);
    if (result == NEO_OK && capture) Rte_DiagCaptured(value);
    else if (result != NEO_OK && result != NEO_E_BUSY) Rte_DiagFailed();
    return result;
}

const Neo_HttpPortsType CamHmi_HttpPorts = {
    .page = page,
    .metrics = Rte_DiagJson,
    .frame = frame,
    .release = Rte_DeviceRelease,
    .stream_opened = Rte_DiagOpened,
    .stream_frame = Rte_DiagStreamed
};
