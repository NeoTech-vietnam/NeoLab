#include "LocalTime.h"

#include "esp_timer.h"

uint64 LocalTime_GetUs(void)
{
    return (uint64)esp_timer_get_time();
}
