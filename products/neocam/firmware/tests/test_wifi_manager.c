#include <assert.h>

#include "WifiIf.h"
#include "WifiMgr.h"

static boolean ready;
static boolean events_enabled = TRUE;
static uint64 now_us;
static uint32 stops;
static Neo_StatusType init_result = NEO_OK;
static Neo_StatusType stop_result = NEO_OK;
static Neo_StatusType deinit_result = NEO_OK;

uint64 LocalTime_GetUs(void) { return now_us; }
void EbOs_Delay(uint32 ms) { now_us += (uint64)ms * 1000U; }
Neo_StatusType WifiIf_Init(const WifiMgr_ConfigType *config)
{
    assert(config == &WifiMgr_Config);
    return init_result;
}
Neo_StatusType WifiIf_Start(void)
{
    if (events_enabled) ready = TRUE;
    return NEO_OK;
}
Neo_StatusType WifiIf_Stop(void)
{
    ++stops;
    if (stop_result == NEO_OK && events_enabled) ready = FALSE;
    return stop_result;
}
Neo_StatusType WifiIf_Deinit(void) { return deinit_result; }
boolean WifiIf_IsReady(void) { return ready; }

int main(void)
{
    assert(WifiMgr_Start() == NEO_E_STATE);
    assert(WifiMgr_Stop() == NEO_OK);
    init_result = NEO_E_MEMORY;
    assert(WifiMgr_Init() == NEO_E_MEMORY);
    assert(WifiMgr_Start() == NEO_E_STATE);
    init_result = NEO_OK;
    assert(WifiMgr_Init() == NEO_OK);
    assert(WifiMgr_Init() == NEO_OK);
    assert(WifiMgr_Start() == NEO_OK);
    assert(WifiMgr_IsReady());
    stop_result = NEO_E_IO;
    assert(WifiMgr_Deinit() == NEO_E_IO);
    assert(WifiMgr_IsReady());
    stop_result = NEO_OK;
    assert(WifiMgr_Stop() == NEO_OK);
    assert(!WifiMgr_IsReady());
    events_enabled = FALSE;
    uint32 previous_stops = stops;
    assert(WifiMgr_Start() == NEO_E_TIMEOUT);
    assert(now_us >= (uint64)WIFI_MGR_STATE_TIMEOUT_MS * 1000U);
    assert(stops == previous_stops + 1U);
    deinit_result = NEO_E_IO;
    assert(WifiMgr_Deinit() == NEO_E_IO);
    deinit_result = NEO_OK;
    assert(WifiMgr_Deinit() == NEO_OK);
    assert(WifiMgr_Deinit() == NEO_OK);
    assert(WifiMgr_Start() == NEO_E_STATE);
    events_enabled = TRUE;
    assert(WifiMgr_Init() == NEO_OK);
    assert(WifiMgr_Start() == NEO_OK);
    assert(WifiMgr_Deinit() == NEO_OK);
    return 0;
}
