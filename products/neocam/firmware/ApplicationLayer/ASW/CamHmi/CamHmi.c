#include "CamHmi.h"
#include "Rte.h"
#include <stdio.h>

static Neo_CameraInfoType camera_info;

Neo_StatusType CamHmi_Init(void)
{
    Neo_CameraInfoType info;
    Neo_StatusType result = Rte_DeviceInfo(&info);
    if (result == NEO_OK) { camera_info = info; Rte_DiagInit(&info); }
    return result;
}

Neo_StatusType CamHmi_Deinit(void) { return NEO_OK; }

static Neo_StatusType page(char *text, uint32 capacity, uint32 *length)
{
    if (text == NULL_PTR || capacity == 0U || length == NULL_PTR) return NEO_E_PARAM;
    int size = snprintf(text, capacity,
        "<!doctype html><meta name=viewport content='width=device-width'>"
        "<title>NeoCam</title><style>body{font:16px system-ui;max-width:1000px;margin:auto;padding:20px;background:#111827;color:#eee}"
        "img{display:block;max-width:100%%;height:auto;margin-top:16px;background:#000;min-height:120px}"
        "button,select,a{font:inherit;padding:8px;margin:4px;border-radius:6px}button,select{background:#25334c;color:white;border:1px solid #607080}"
        "a{color:#8df}code{color:#8df}pre{white-space:pre-wrap;font-size:13px}small{color:#b9c4d2}</style>"
        "<h1>NeoCam · %s</h1><p>Direct Wi-Fi camera preview and full-resolution photos</p>"
        "<label>Profile <select id=profiles aria-label='Camera profile'></select></label>"
        "<button id=apply>Apply profile</button><button id=photo>Download 3 MP photo</button>"
        "<a href=/capture download=neocam-snapshot.jpg>Preview snapshot</a>"
        "<p id=status role=status>Connecting…</p><small>Target FPS is a goal. Unqualified profiles are experimental. "
        "Full-resolution photos briefly pause preview, then restore it.</small>"
        "<img id=view src=/stream alt='Live camera stream'><p id=summary></p>"
        "<details><summary>Detailed metrics</summary><pre id=metrics></pre></details>"
        "<script>const $=id=>document.getElementById(id);let initialized=false,retry=null;"
        "async function json(url,opt){const r=await fetch(url,{cache:'no-store',...opt});if(!r.ok)throw Error(await r.text());return r.json()}"
        "async function refresh(){try{const [p,m]=await Promise.all([json('/api/profile'),json('/metrics')]);"
        "if(!initialized){for(const x of p.profiles.filter(x=>!x.photo_only)){const o=document.createElement('option');o.value=x.id;"
        "o.textContent=x.id+' · '+x.width+'×'+x.height+' · target '+x.target_fps+' FPS'+(x.performance_validated?' · validated':' · experimental');"
        "$('profiles').appendChild(o)}$('profiles').value=p.active;initialized=true}"
        "const busy=['switching','photo','stopping'].includes(p.state);$('apply').disabled=busy;$('photo').disabled=busy;"
        "$('status').textContent=p.state+' · '+p.active+(p.requested!==p.active?' → '+p.requested:'')+"
        "(p.last_error?' · error '+p.last_error:'')+(p.restore_error?' · restore error '+p.restore_error:'');"
        "$('summary').textContent=m.width+'×'+m.height+' · captured '+m.capture_fps_1s+' FPS · sent '+m.send_fps_1s+"
        "' FPS · frame age p95 '+Math.round(m.frame_age_p95_us/1000)+' ms';"
        "$('metrics').textContent=JSON.stringify(m,null,2)}catch(e){$('status').textContent=e.message}}"
        "$('apply').onclick=async()=>{try{$('apply').disabled=true;await json('/api/profile',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({profile:$('profiles').value})});await refresh()}catch(e){$('status').textContent=e.message;$('apply').disabled=false}};"
        "$('photo').onclick=async()=>{try{$('photo').disabled=true;$('status').textContent='Taking photo…';const r=await fetch('/photo',{cache:'no-store'});"
        "if(!r.ok)throw Error(await r.text());const b=await r.blob(),u=URL.createObjectURL(b),a=document.createElement('a');"
        "a.href=u;a.download='neocam-photo.jpg';a.click();setTimeout(()=>URL.revokeObjectURL(u),10000)}"
        "catch(e){$('status').textContent=e.message}finally{$('photo').disabled=false;refresh()}};"
        "$('view').onerror=()=>{if(!retry)retry=setTimeout(()=>{retry=null;$('view').src='/stream?t='+Date.now()},1000)};"
        "refresh();setInterval(refresh,1000);</script>", camera_info.sensor_name);
    if (size < 0 || (uint32)size >= capacity) return NEO_E_TOO_LARGE;
    *length = (uint32)size;
    return NEO_OK;
}

static Neo_StatusType next(uint64 after, uint32 timeout, Neo_FrameType *value)
{
    Neo_StatusType result = Rte_DeviceNext(after, timeout, value);
    if (result != NEO_OK && result != NEO_E_BUSY && result != NEO_E_NOT_READY && result != NEO_E_TIMEOUT)
        Rte_DiagFailed();
    return result;
}

static Neo_StatusType photo(Neo_FrameType *value)
{
    Neo_StatusType result = Rte_Photo(value);
    if (result == NEO_OK) Rte_DiagCaptured(value);
    else if (result != NEO_E_BUSY && result != NEO_E_NOT_READY) Rte_DiagFailed();
    return result;
}

static Neo_StatusType frame(boolean capture, Neo_FrameType *value)
{
    Neo_StatusType result = Rte_DeviceCapture(value);
    if (result == NEO_OK && capture) Rte_DiagCaptured(value);
    else if (result != NEO_OK && result != NEO_E_BUSY && result != NEO_E_NOT_READY && result != NEO_E_TIMEOUT) Rte_DiagFailed();
    return result;
}

const Neo_HttpPortsType CamHmi_HttpPorts = {
    .page = page,
    .metrics = Rte_DiagJson,
    .frame = frame,
    .stream_next = next,
    .photo = photo,
    .profile = Rte_ProfileJson,
    .select_profile = Rte_SelectProfile,
    .release = Rte_DeviceRelease,
    .stream_opened = Rte_DiagOpened,
    .stream_frame = Rte_DiagStreamed
};
