#include "HttpSrv_Internal.h"

Neo_StatusType HttpSrv_Start(const Neo_HttpPortsType *ports)
{
    if (ports == NULL_PTR || ports->page == NULL_PTR || ports->metrics == NULL_PTR ||
        ports->frame == NULL_PTR || ports->release == NULL_PTR ||
        ports->stream_opened == NULL_PTR || ports->stream_frame == NULL_PTR) return NEO_E_PARAM;
    return HttpSrv_BackendStart(ports);
}

Neo_StatusType HttpSrv_Stop(void)
{
    return HttpSrv_BackendStop();
}
