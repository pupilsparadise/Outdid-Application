#ifndef OTDGSMTCPAPP_H
#define OTDGSMTCPAPP_H

#include "OtdGsm_App.h"

void OtdGsmTcpApp_TcpStateInit(void);
void OtdGsmTcpApp_TcpSendStateInit(void);

void OtdGsmTcpApp_ServerPrepareData(char *data, uint16_t data_length);

OtdGsmApp_Status_ten OtdGsmTcpApp_TcpInitCmdProcess(void);
OtdGsmApp_Status_ten OtdGsmTcpApp_TcpSendCmdProcess(void);

#endif