#ifndef OTDGSMTCPAPP_H
#define OTDGSMTCPAPP_H

#include "OtdGsm_App.h"

#define SERVER_IP_ADDRESS 	"3.108.182.109"
#define SERVER_PORT		"1801"
/*
#define SERVER_IP_ADDRESS 	"122.166.210.142"
#define SERVER_PORT		"8050"*/

#define FAILED		0U
#define SUCCESS		1U

// This sub-state are used only for TCP intitialisation & sending data
//SubState Processing
typedef enum
{
	/**< TCP Init Command */
	TCP_QICSGP = 0,
	TCP_CMGF,
	/**< TCP Open and send Command */
	TCP_IDLESTATE,
	TCP_NETWORK_CHECK,
	TCP_SERVER_CHECK,
	TCP_NETWORK_CONNECT,
	TCP_SERVER_CONNECT,
	TCP_PREPARE_SEND,
	TCP_SEND_DATA,
	TCP_CLOSE_SERVER,
	TCP_NETWORK_CLOSE,
	Tcp_MaxSubState
}OtdGsmTcpApp_SubState_ten;

typedef struct
{
	OtdGsmTcpApp_SubState_ten TcpCurrent_en; //stores the current state
	OtdGsmTcpApp_SubState_ten TcpPrevious_en;//stores the previous state
	volatile uint8_t TcpCmdSendFlag;
	volatile uint8_t TcpTrials;//tracks the number of times the frame executed
	volatile int16_t TcpTimeOut; // tracks the time
}OtdGsmTcpApp_SubState_tst;

typedef struct
{
	uint8_t IsNetworkConnected;    	//tcp connection is estalblish if NETOPEN is success	
	uint8_t IsServerConnected; 	//server with specified ip is connected i.e. CIPOPEN is success
	uint8_t IsServerDataReady;  	//update the flag once data is available for server 
	char ServerDataBuf[256];	//buffer to send data
	uint16_t DataSize;		//Size of packets need to transmit to server
}OtdGsm_TcpApp_TcpHandle_tst;

void OtdGsmTcpApp_TcpStateInit(void);
void OtdGsmTcpApp_TcpSendStateInit(void);

void OtdGsmTcpApp_ServerPrepareData(char *data, uint16_t data_length);

OtdGsmApp_Status_ten OtdGsmTcpApp_TcpInitCmdProcess(void);
OtdGsmApp_Status_ten OtdGsmTcpApp_TcpSendCmdProcess(void);

#endif