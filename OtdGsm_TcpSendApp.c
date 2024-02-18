#include "OtdGsm_TcpApp.h"
#include "OtdUART.h"
#include "OtdDelay.h"
#include <stdio.h>
#include <string.h>

#define DATA_READY 1

//Here the network is checked and datat is send to the server
typedef enum
{
	/**< TCP Open and send Command */
	TCP_NETWORK_CHECK,	//check network connection
	TCP_NETWORK_CONNECT,	//connect network
	TCP_NETWORK_CLOSE,	//close network
	
	TCP_SERVER_CHECK,	//check server connection
	TCP_SERVER_CONNECT,	//connect server
	TCP_CLOSE_SERVER,	//close server
	
	TCP_SEND_DATA_TO_SERVER,//send data to server
	Tcp_SendMaxSubState
}OtdTcpSendApp_Network_ten;

void OtdTcpSendApp_TcpSendRxProcess(OtdTcpSendApp_Network_ten NetworkState)
{
	switch(NetworkState)
	{
		case TCP_NETWORK_CHECK:
					//check if network is connected
					break;
	}	
}
void OtdTcpSendApp_TcpSendCmdProcess(OtdTcpSendApp_Network_ten NetworkState)
{
	uint8_t status;
	switch(NetworkState)
	{
		case TCP_NETWORK_CHECK:
					//send AT+NETOPEN? to check if network is available
					break;
	}
	
	status = OtdTcpSendApp_TcpSendRxProcess(NetworkState);
}

void OtdTcpSendApp_IsServerDataReady()
{
	//check if server data is available
}
void OtdTcpSendApp_CheckState(OtdTcpSendApp_Network_ten NetworkState)
{
	Status = OtdTcpSendApp_TcpSendCmdProcess(NetworkState);
	
	switch(NetworkState)
	{
		case TCP_NETWORK_CHECK:
					if(Status == SUCCESS)
					{
					
					}
					else if()
	}
}

void OtdTcpSendApp_TcpIdle()
{
	uint8_t status;
	
	if(OtdTcpSendApp_IsServerDataReady != DATA_READY)
	{
		//do nothing
	}
	else//Server data is available
	{
		//Check if Network is connected
		status = OtdTcpSendApp_TcpSendCmdProcess(TCP_NETWORK_CHECK);
	}
}