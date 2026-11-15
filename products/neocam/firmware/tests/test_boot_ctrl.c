#include <assert.h>
#include <string.h>
#include "BootCtrl.h"

static int fail_start = -1;
static int fail_stop = -1;
static unsigned events[32], count;
static boolean live[3];

static Neo_StatusType start(unsigned i)
{
    events[count++] = i + 1U;
    live[i] = TRUE; /* Simulate partial ownership even on failure. */
    return (int)i == fail_start ? NEO_E_IO : NEO_OK;
}
static Neo_StatusType stop(unsigned i)
{
    events[count++] = i + 11U;
    if ((int)i == fail_stop) return NEO_E_BUSY;
    live[i] = FALSE;
    return NEO_OK;
}
static Neo_StatusType start0(void) { return start(0); }
static Neo_StatusType start1(void) { return start(1); }
static Neo_StatusType start2(void) { return start(2); }
static Neo_StatusType stop0(void) { return stop(0); }
static Neo_StatusType stop1(void) { return stop(1); }
static Neo_StatusType stop2(void) { return stop(2); }
static const BootCtrl_StepType steps[] = {
    {"zero", start0, stop0}, {"one", start1, stop1}, {"two", start2, stop2}
};
static const BootCtrl_ConfigType config = {steps, 3};

int main(void)
{
    assert(BootCtrl_Stop() == NEO_OK);
    assert(BootCtrl_Start(NULL_PTR) == NEO_E_PARAM);
    for (int i = 0; i < 3; ++i) {
        count = 0;
        fail_start = i;
        assert(BootCtrl_Start(&config) == NEO_E_IO);
        assert(BootCtrl_GetStatus().primary == NEO_E_IO);
        assert(BootCtrl_GetStatus().active_steps == 0);
        for (int j = 0; j <= i; ++j) {
            assert(events[j] == (unsigned)j + 1U);
            assert(events[i + 1 + j] == (unsigned)(i - j) + 11U);
            assert(!live[j]);
        }
    }
    count = 0;
    fail_start = 2;
    fail_stop = 2;
    assert(BootCtrl_Start(&config) == NEO_E_IO);
    BootCtrl_StatusType status = BootCtrl_GetStatus();
    assert(status.active_steps == 3 && status.primary == NEO_E_IO && status.cleanup == NEO_E_BUSY);
    assert(strcmp(status.stage, "two") == 0);
    assert(live[0] && live[1] && live[2]);
    assert(BootCtrl_Start(&config) == NEO_E_STATE);
    fail_stop = -1;
    assert(BootCtrl_Stop() == NEO_OK);
    assert(BootCtrl_GetStatus().primary == NEO_E_IO);
    assert(!live[0] && !live[1] && !live[2]);
    fail_start = -1;
    count = 0;
    assert(BootCtrl_Start(&config) == NEO_OK);
    assert(BootCtrl_Start(&config) == NEO_E_STATE);
    fail_stop = 1;
    assert(BootCtrl_Stop() == NEO_E_BUSY);
    assert(live[0] && live[1] && !live[2]);
    assert(BootCtrl_GetStatus().primary == NEO_E_BUSY);
    fail_stop = -1;
    assert(BootCtrl_Stop() == NEO_OK);
    assert(BootCtrl_Stop() == NEO_OK);
    return 0;
}
