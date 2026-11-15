#include "WifiMgr.h"

#include "EbOs.h"
#include "LocalTime.h"
#include "WifiIf.h"

static boolean initialized;

static Neo_StatusType wait_for_state(boolean ready)
{
    uint64 deadline = LocalTime_GetUs() + (uint64)WIFI_MGR_STATE_TIMEOUT_MS * 1000U;
    while (WifiIf_IsReady() != ready) {
        if (LocalTime_GetUs() >= deadline) return NEO_E_TIMEOUT;
        EbOs_Delay(10U);
    }
    return NEO_OK;
}

Neo_StatusType WifiMgr_Init(void)
{
    if (initialized) return NEO_OK;
    Neo_StatusType result = WifiMgr_ValidateConfig(&WifiMgr_Config);
    if (result != NEO_OK) return result;
    result = WifiIf_Init(&WifiMgr_Config);
    if (result == NEO_OK) initialized = TRUE;
    return result;
}

Neo_StatusType WifiMgr_Start(void)
{
    if (!initialized) return NEO_E_STATE;
    Neo_StatusType result = WifiIf_Start();
    if (result != NEO_OK) return result;
    result = wait_for_state(TRUE);
    if (result != NEO_OK) (void)WifiIf_Stop();
    return result;
}

Neo_StatusType WifiMgr_Stop(void)
{
    Neo_StatusType result = WifiIf_Stop();
    if (result != NEO_OK) return result;
    return wait_for_state(FALSE);
}

Neo_StatusType WifiMgr_Deinit(void)
{
    Neo_StatusType result = WifiMgr_Stop();
    if (result != NEO_OK) return result;
    result = WifiIf_Deinit();
    if (result == NEO_OK) initialized = FALSE;
    return result;
}

boolean WifiMgr_IsReady(void)
{
    return WifiIf_IsReady();
}
