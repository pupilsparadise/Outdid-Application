#include "OtdOcpp_App.h"
#include "OtdGsm_App.h"
#include "OtdGsm_TcpApp.h"
#include <stdio.h>

extern OtdGsm_TcpApp_TcpHandle_tst TcpHandle_st;

/*
 * @brief OCPP Sending Handshake Message to Server
  * @param  none
  * @retval none
  */
static void OCPP_SendHandshake(void)
{
	const char UrlBuf[] = "/OCPPJ";
	const char DevAdd[] = "390606000920";
	const char HostBuf[] = SERVER_IP_ADDRESS;
	const char PortBuf[] = SERVER_PORT;
	
	TcpHandle_st.DataSize = sprintf((char *)TcpHandle_st.ServerDataBuf, "GET %s/%s HTTP/1.1\r\nHost: %s:%s\r\nConnection: Upgrade\r\nUpgrade: websocket\r\n"
	    "Sec-WebSocket-Key: +ZB8ujgJwCnW61qhiXZs5w==\r\nConnection: keep-alive, Upgrade\r\nSec-WebSocket-Protocol: ocpp1.6\r\n"
	    "Sec-WebSocket-Version: 13\r\n\r\n", UrlBuf, DevAdd, HostBuf, PortBuf);	

	TcpHandle_st.IsServerDataReady = READY;//Server data is updated
}
void OtdOcpp_Cyclic(void)
{
	OCPP_SendHandshake();
}