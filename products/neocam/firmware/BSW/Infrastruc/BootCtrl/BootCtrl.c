#include "BootCtrl.h"

static const BootCtrl_ConfigType *active;
static BootCtrl_StatusType status;

Neo_StatusType BootCtrl_Stop(void)
{
    while (active != NULL_PTR && status.active_steps > 0U) {
        const BootCtrl_StepType *step = &active->steps[status.active_steps - 1U];
        Neo_StatusType result = step->stop();
        if (result != NEO_OK) {
            if (status.cleanup == NEO_OK) status.cleanup = result;
            if (status.primary == NEO_OK) {
                status.primary = result;
                status.stage = step->name;
            }
            return result; /* Lower dependencies must remain alive for a retry. */
        }
        status.active_steps--;
    }
    active = NULL_PTR;
    return NEO_OK;
}

Neo_StatusType BootCtrl_Start(const BootCtrl_ConfigType *config)
{
    if (config == NULL_PTR || config->steps == NULL_PTR || config->count == 0U) return NEO_E_PARAM;
    if (active != NULL_PTR) return NEO_E_STATE;
    for (uint32 i = 0; i < config->count; ++i) {
        if (config->steps[i].name == NULL_PTR || config->steps[i].start == NULL_PTR ||
            config->steps[i].stop == NULL_PTR) return NEO_E_PARAM;
    }
    active = config;
    status = (BootCtrl_StatusType){0};
    for (uint32 i = 0; i < config->count; ++i) {
        /* Include the attempted step: its own rollback might itself fail. */
        status.active_steps = i + 1U;
        Neo_StatusType result = config->steps[i].start();
        if (result != NEO_OK) {
            status.primary = result;
            status.stage = config->steps[i].name;
            (void)BootCtrl_Stop();
            return result;
        }
    }
    return NEO_OK;
}

BootCtrl_StatusType BootCtrl_GetStatus(void)
{
    return status;
}
