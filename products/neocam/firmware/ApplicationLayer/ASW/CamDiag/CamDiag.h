#ifndef CAMDIAG_H
#define CAMDIAG_H
#include "Neo_Types.h"
void CamDiag_Init(const Neo_CameraInfoType *info);
void CamDiag_Captured(const Neo_FrameType *frame);
void CamDiag_Failed(void);
void CamDiag_StreamOpened(void);
void CamDiag_StreamFrame(const Neo_FrameType *frame, boolean sent);
Neo_StatusType CamDiag_GetJson(char *text, uint32 capacity, uint32 *length);
#endif
