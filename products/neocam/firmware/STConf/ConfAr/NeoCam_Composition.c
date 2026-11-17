#include "NeoCam_Composition.h"
#include "CamHmi.h"
#include "CamDev.h"
#include "CamDiag.h"
#include "CamSupv.h"
#include "CamIo.h"
#include "CamPipe.h"
#include "MediaBuf.h"
#include "WifiMgr.h"
#include "HttpSrv.h"
#include "Rte.h"

static const Rte_DevicePortsType device_ports = {CamDev_Capture, CamDev_Release, CamDev_GetInfo, CamDev_Next};
static const Rte_DiagnosticPortsType diagnostic_ports = {
    CamDiag_Init, CamDiag_Captured, CamDiag_Failed, CamDiag_StreamOpened,
    CamDiag_StreamFrame, CamDiag_GetJson
};
static const Rte_SupervisorPortsType supervisor_ports = {
    CamSupv_SelectProfile, CamSupv_Photo, CamSupv_ProfileJson
};
static Neo_StatusType connect_rte(void)
{
    return Rte_Init(&CamHmi_HttpPorts, &device_ports, &diagnostic_ports, &supervisor_ports);
}
static Neo_StatusType start_http(void) { return HttpSrv_Start(Rte_GetHttpPorts()); }

#define NEOCAM_START_SEC_CONST_UNSPECIFIED
#include "MemMap.h"
NEOCAM_SECTION static const BootCtrl_StepType startup[] = {
    {"NVS", BootCtrl_PlatformInit, BootCtrl_PlatformDeinit},
    {"JPEG pool", MediaBuf_Init, MediaBuf_Deinit},
    {"camera", CamIo_Init, CamIo_Deinit},
    {"RTE", connect_rte, Rte_Deinit},
    {"application", CamHmi_Init, CamHmi_Deinit},
    {"Wi-Fi init", WifiMgr_Init, WifiMgr_Deinit},
    {"Wi-Fi start", WifiMgr_Start, WifiMgr_Stop},
    {"producer", CamPipe_Start, CamPipe_Stop},
    {"HTTP", start_http, HttpSrv_Stop},
};
NEOCAM_SECTION static const BootCtrl_ConfigType config = {
    .steps = startup,
    .count = sizeof(startup) / sizeof(startup[0])
};
#define NEOCAM_STOP_SEC_CONST_UNSPECIFIED
#include "MemMap.h"

Neo_StatusType NeoCam_Start(void) { return BootCtrl_Start(&config); }
Neo_StatusType NeoCam_Stop(void) { return BootCtrl_Stop(); }
