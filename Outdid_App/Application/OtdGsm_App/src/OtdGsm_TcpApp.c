#include "OtdGsm_TcpApp.h"
#include "OtdUART.h"
#include "OtdDelay.h"
#include <stdio.h>
#include <string.h>

extern OtdGsmApp_SubState_tst GsmSubState_st;
extern volatile uint16_t RxIndex;
extern uint8_t gsm_tx_pending;//flag to confirm all bytes have transmitted
extern volatile char Gsm_RxBuffer[RX_BUFFER_LENGTH];

OtdGsmTcpApp_SubState_tst TcpSubState_st;

OtdGsm_TcpApp_TcpHandle_tst TcpHandle_st;//structure to maintain status and capture data for the server

//static const char cipopen_cmd[] = "AT+CIPOPEN=1,\"TCP\",\"122.166.210.142\",8050\r\n";
/*
const char UrlBuf[] = "/OCPPJ";
const char DevAdd[] = "390606000920";
const char HostBuf[] = SERVER_IP_ADDRESS;
const char PortBuf[] = SERVER_PORT;*/

/***************************************************TCP Publish and recieve handle of data***********************************/

void OtdGsmTcpApp_TcpSendStateInit(void)
{
	//sub state default init
	TcpSubState_st.TcpCurrent_en = TCP_NETWORK_CHECK;
	TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
	TcpSubState_st.TcpTrials = MAX_TRIALS;
	TcpSubState_st.TcpTimeOut = RX_TIMEOUT;
	TcpSubState_st.TcpCmdSendFlag = 0;
	
	//RxIndex
	RxIndex = 0;		
}
uint8_t IsServerDataReady()
{
	return 1;
}
void OtdGsmTcpApp_ServerPrepareData(char *data, uint16_t data_length)
{
	uint16_t data_index;
	/*Clear the data buffer before preparing */
	memset(TcpHandle_st.ServerDataBuf,'\0',sizeof(TcpHandle_st.ServerDataBuf));
	
	//Copy the requested data to global buffer
	for(data_index = 0; data_index < data_length;data_index++)
	{
		TcpHandle_st.ServerDataBuf[data_index] = data[data_index];
	}
	
	TcpHandle_st.DataSize = data_length;
	TcpHandle_st.IsServerDataReady = READY;
}

#define TCP_STATE_CLOSED 0
#define TCP_STATE_OPEN   1
#define TCP_STATE_ERROR	 2 //handles the error of TCP

//Check if TCP is connected or not
static uint8_t OtdGsmTcpApp_ServiceTcpOpenState(void)
{
	//check if TCP is available, if not make sure it should be available
	if((TcpHandle_st.IsNetworkConnected == DISCONNECTED) && (TcpHandle_st.IsServerConnected == DISCONNECTED))
	{
		//change TCP substate to NETOPEN --> in order to connect with network
		return FAILED;
	}
	else if((TcpHandle_st.IsNetworkConnected == CONNECTED) && (TcpHandle_st.IsServerConnected == DISCONNECTED))
	{
		//change TCP substate to CIPOPEN --> in order to connect with server
		
		return FAILED;
	}
	else
	{
		//FIXME: there is no possibility  if TcpHandle_st.IsNetworkConnected == DISCONNECTED and TcpHandle_st.IsServerConnected == CONNECTED
		// or both are connected 
		
		//Change the state to TCP_STATE_OPEN
		return SUCCESS;
	}
}

static void OtdGsmTcpApp_TcpIdleState()
{
	/*Step1: Check is server data to be sent is available*/
	
	/*Step2: is server data is available, check if network is connected*/
		// AT+NETOPEN?
	/*Step3: if network is not connected, */	
	

	
	// if network is not connected chang the current state to 
	
	// check if server is connected
	
	//
}
/*service for the response from GSM modem*/
uint8_t  tcp_local_rx_buffer[RX_BUFFER_LENGTH];
static OtdGsmApp_Status_ten OtdGsmTcpApp_TcpSendRxProcess(OtdGsmTcpApp_SubState_ten rx_state)
{
	uint8_t data[512];
	//char __far data[256];
	//uint8_t  tcp_local_rx_buffer[RX_BUFFER_LENGTH]; 
	char __far *expected_reply;
	volatile OtdGsmApp_Status_ten Status = Gsm_Nok;
	//uint32_t fetched_ICCID;
	
	OtdUart_IsDataAvailable();
	
	/*if data is available RxIndex will be more than 0*/
	if(RxIndex > 0)
	{
		#if GSM_DEBUG
		sprintf(data,"TCP_DEBUG >> GSM Rx response====Data Recieved from GSM Module=====Bytes = %d\n",RxIndex);
		OtdUart_DebugSend(data);
		#endif
		
		switch(rx_state)
		{
			case TCP_IDLESTATE:
						//do nothing
						Status = Gsm_Ok;
						break;
			case TCP_NETWORK_CHECK:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing TCP_NETWORK_CHECK command=====\n");
					#endif	
					
					expected_reply = "+NETOPEN:1";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),1,tcp_local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Network state is already connected =====\n");
						#endif	
						TcpHandle_st.IsNetworkConnected = CONNECTED; //Network is connected
						
						//Update to next correspoding state 
						TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						TcpSubState_st.TcpCurrent_en = TCP_SERVER_CHECK; //State change for checking TCP connection
						
						Status = Gsm_Ok;
					}
					else if(OtdGsmApp_IsSubArrayPresent(tcp_local_rx_buffer,RX_BUFFER_LENGTH,(uint8_t*)"+NETOPEN:0",sizeof("+NETOPEN:0")-1,0,tcp_local_rx_buffer)== Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response==== Network state is diconnected =====\n");
						#endif	
						
						TcpHandle_st.IsNetworkConnected = DISCONNECTED; //TCP state is disconnected
						
						//Update to next correspoding state 
						TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						TcpSubState_st.TcpCurrent_en = TCP_NETWORK_CONNECT; //Connect the network

						Status = Gsm_Ok;
					}
					else
					{
						if(TcpSubState_st.TcpTimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;		
					
			case TCP_SERVER_CHECK:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing TCP_SERVER_CHECK command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),1,tcp_local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required response recieved=====\n");
						#endif	
						sprintf((char *)expected_reply,"+CIPOPEN:1, \"TCP\",%s,%s,-1",SERVER_IP_ADDRESS,SERVER_PORT);
						//expected_reply = "+CIPOPEN:1, \"TCP\",3.108.182.109,1801,-1";
						if(OtdGsmApp_IsSubArrayPresent(tcp_local_rx_buffer,RX_BUFFER_LENGTH,(uint8_t*)expected_reply,sizeof(*expected_reply)-1,0,tcp_local_rx_buffer)== Gsm_Ok)
						{
							TcpHandle_st.IsServerConnected = CONNECTED; //Server is connected
							
							//Update to next correspoding state 
							TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
							TcpSubState_st.TcpCurrent_en = TCP_PREPARE_SEND; //State change for checking TCP connection
							
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response==== Server state is already Connected =====\n");
							#endif						
						}
						else
						{
							TcpHandle_st.IsServerConnected = DISCONNECTED; //Server is disconnected
							
							//Update to next correspoding state 
							TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
							TcpSubState_st.TcpCurrent_en = TCP_SERVER_CONNECT; //State change for checking TCP connection
							
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response==== Server state is diconnected =====\n");
							#endif
						}
					
						Status = Gsm_Ok;
					}
					else
					{
						if(TcpSubState_st.TcpTimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;							
			case TCP_NETWORK_CONNECT:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing AT+NETOPEN command=====\n");
					#endif	
					
					expected_reply = "SUCCESS";//update expected reply
					
					if(
						OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),1,tcp_local_rx_buffer) == Gsm_Ok||
						// ERROR: 902 and FAIL is recieved if the NETOPEN command is already executed or the connection is already established
						OtdGsmApp_IsSubArrayPresent(tcp_local_rx_buffer,RX_BUFFER_LENGTH,(uint8_t*)"ERROR: 902",sizeof("ERROR: 902")-1,0,tcp_local_rx_buffer)== Gsm_Ok||
						OtdGsmApp_IsSubArrayPresent(tcp_local_rx_buffer,RX_BUFFER_LENGTH,(uint8_t*)"FAIL",sizeof("FAIL")-1,0,tcp_local_rx_buffer)== Gsm_Ok
					)
					{
						TcpHandle_st.IsNetworkConnected = CONNECTED; //TCP connected
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						//Update to next correspoding state 
						TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						TcpSubState_st.TcpCurrent_en = TCP_SERVER_CHECK; //State change for checking TCP connection
						
						Status = Gsm_Ok;
					}
					else
					{
						TcpHandle_st.IsNetworkConnected = DISCONNECTED; //Error in NETOPEN command so the connection is not established
						
						if(TcpSubState_st.TcpTimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
						//Update to next correspoding state 
						//TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						//TcpSubState_st.TcpCurrent_en = TCP_SERVER_CONNECT; //State change for checking TCP connection
						
					}
					
					break;	
					
			case TCP_SERVER_CONNECT:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing AT+CIPOPEN command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,tcp_local_rx_buffer) == Gsm_Ok)
					{
						TcpHandle_st.IsServerConnected = CONNECTED; //Server connection established
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	

						//Update to next correspoding state 
						TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						TcpSubState_st.TcpCurrent_en = TCP_PREPARE_SEND; //State change for checking TCP connection
						
						Status = Gsm_Ok;
					}
					else
					{
						TcpHandle_st.IsServerConnected = DISCONNECTED; //Unable to connect with server
						
						if(TcpSubState_st.TcpTimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;	
					
			case TCP_PREPARE_SEND:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing TCP_PREPARE_SEND command=====\n");
					#endif	
					
					expected_reply = ">";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),1,tcp_local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						//Update to next correspoding state 
						TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						TcpSubState_st.TcpCurrent_en = TCP_SEND_DATA; //State change for checking TCP connection
						
						Status = Gsm_Ok;
					}
					else if(OtdGsmApp_IsSubArrayPresent(tcp_local_rx_buffer,RX_BUFFER_LENGTH,(uint8_t*)"ERROR",sizeof("ERROR: 902")-1,0,tcp_local_rx_buffer)== Gsm_Ok)
					{
						//Update to next correspoding state 
						TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						TcpSubState_st.TcpCurrent_en = TCP_CLOSE_SERVER; //State change for closing the Server					
					}
					else
					{
						if(TcpSubState_st.TcpTimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;		
					
			case TCP_SEND_DATA:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing TCP_SEND_DATA command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,tcp_local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						TcpHandle_st.IsServerDataReady = NOT_READY;//update the flag
						
						//Update to next correspoding state 
						TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						TcpSubState_st.TcpCurrent_en = TCP_IDLESTATE; //State change for checking TCP connection
						
						
						Status = Gsm_Ok;
					}
					else
					{
						if(TcpSubState_st.TcpTimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;	
					
			case TCP_CLOSE_SERVER:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing TCP_CLOSE_SERVER command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),1,tcp_local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						//Update to next correspoding state 
						TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						TcpSubState_st.TcpCurrent_en = TCP_NETWORK_CLOSE; //State change for checking TCP connection						
						Status = Gsm_Ok;
					}
					else if(OtdGsmApp_IsSubArrayPresent(tcp_local_rx_buffer,RX_BUFFER_LENGTH,(uint8_t*)"+CIPCLOSE:FAIL,1",sizeof("+CIPCLOSE:FAIL,1")-1,0,tcp_local_rx_buffer)== Gsm_Ok)
					{
						//Update to next correspoding state 
						TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						TcpSubState_st.TcpCurrent_en = TCP_NETWORK_CLOSE; //State change for checking TCP connection						
					}
					else
					{
						if(TcpSubState_st.TcpTimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;			
					
			case TCP_NETWORK_CLOSE:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing TCP_NETWORK_CLOSE command=====\n");
					#endif	
					
					expected_reply = "SUCCESS";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,tcp_local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						//Update to next correspoding state 
						TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						TcpSubState_st.TcpCurrent_en = TCP_IDLESTATE; //TODO: check if data is not send, then it should try to reconnect, AT+NETOPEN												
						
						Status = Gsm_Ok;
					}
					else
					{
						if(TcpSubState_st.TcpTimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;								
										
				default:
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> ========================================\n");
						OtdUart_DebugSend("TCP_DEBUG >> 	Tcp Rx response State Not found	\n");
						OtdUart_DebugSend("TCP_DEBUG >> ========================================\n");
						#endif	
						break;
		}	
	}
	else
	{
		#if GSM_DEBUG
		OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====No Data Recieved from GSM Module=====\n");
		#endif	

		if(TcpSubState_st.TcpTimeOut <= 0)
		{
			Status = Gsm_RxTimeOut;
		}
		else
		{
			Status = Gsm_Busy;
		}
	}
		
	
	return Status;
}

/*service the command to send data to the server via TCP IP*/
OtdGsmApp_Status_ten OtdGsmTcpApp_TcpSendCmdProcess(void)
{
	uint8_t data[128];
	char CMDBuff[50] = {0};
	volatile OtdGsmApp_Status_ten status_e = Gsm_Nok;
	
	switch(TcpSubState_st.TcpCurrent_en)
	{
		case TCP_IDLESTATE:
					/*check if Server data is available*/
					if(IsServerDataReady() == 0)
					{
						//move to next state
						//Update to next correspoding state 
						TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
						TcpSubState_st.TcpCurrent_en = TCP_NETWORK_CHECK; //State change for checking Network connection
						
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-State IDLE====SERVER DATA AVAILABLE\n");
						#endif	
					}
					else
					{
						
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-State IDLE====SERVER DATA NOT AVAILABLE\n");
						#endif							
						//stay here
					}
					break;
					
		case TCP_NETWORK_CHECK:
				if(!TcpSubState_st.TcpCmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+NETOPEN?\r\n");
					TcpSubState_st.TcpTimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====AT+NETOPEN?=====send\n");
					#endif	
					
					TcpSubState_st.TcpCmdSendFlag = 1;
				}
				
		case TCP_SERVER_CHECK:
				if(!TcpSubState_st.TcpCmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+CIPOPEN?\r\n");
					TcpSubState_st.TcpTimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====AT+CIPOPEN?=====send\n");
					#endif	
					
					TcpSubState_st.TcpCmdSendFlag = 1;
				}				
		case TCP_NETWORK_CONNECT:
				if(!TcpSubState_st.TcpCmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+NETOPEN\r\n");
					OtdDelay_ms(5000);//TODO: "SUCCESS" is recieved after some delay, need to remove this delay later
					//TcpSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====AT+NETOPEN=====send\n");
					#endif	
					
					TcpSubState_st.TcpCmdSendFlag = 1;
				}
				
		case TCP_SERVER_CONNECT:
				if(!TcpSubState_st.TcpCmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					//OtdGsmApp_SendATCommand("AT+CIPOPEN=1,\"TCP\",\"122.166.210.142\",8050\r\n");
					sprintf(CMDBuff,"AT+CIPOPEN=1,\"TCP\",\"%s\",%s\r\n", SERVER_IP_ADDRESS, SERVER_PORT);
					OtdGsmApp_SendATCommand(CMDBuff);
					//OtdGsmApp_SendATCommand(cipopen_cmd);
					OtdDelay_ms(5000);//TODO: "SUCCESS" is recieved after some delay, need to remove this delay later
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====TCP_SERVER_CONNECT=====send\n");
					#endif	
					
					TcpSubState_st.TcpCmdSendFlag = 1;
				}				
				break;	
				
		case TCP_PREPARE_SEND:
				if(!TcpSubState_st.TcpCmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					
					
					/*
					TcpHandle_st.DataSize = sprintf((char *)TcpHandle_st.ServerDataBuf,"GET %s/%s HTTP/1.1\r\nHost: %s:%s\r\nConnection: Upgrade\r\nUpgrade: websocket\r\n"
			    		"Sec-WebSocket-Key: +ZB8ujgJwCnW61qhiXZs5w==\r\nConnection: keep-alive, Upgrade\r\nSec-WebSocket-Protocol: ocpp1.6\r\n"
			    		"Sec-WebSocket-Version: 13\r\n\r\n",\
			    		UrlBuf, DevAdd, HostBuf, PortBuf);*/
					
					/*
					TcpHandle_st.DataSize = sprintf((char *)TcpHandle_st.ServerDataBuf, "GET %s/%s HTTP/1.1\r\nHost: %s:%s\r\nConnection: Upgrade\r\nUpgrade: websocket\r\n"
					    "Sec-WebSocket-Key: +ZB8ujgJwCnW61qhiXZs5w==\r\nConnection: keep-alive, Upgrade\r\nSec-WebSocket-Protocol: ocpp1.6\r\n"
					    "Sec-WebSocket-Version: 13\r\n\r\n", UrlBuf, DevAdd, HostBuf, PortBuf);*/	
					
					//OtdGsmTcpApp_ServerPrepareData("HELLO",strlen("HELLO"));

					//TcpHandle_st.IsServerDataReady = READY;
					
					if(TcpHandle_st.IsServerDataReady == READY)
					{
						sprintf(CMDBuff,"AT+CIPSEND=1,%d\r\n",TcpHandle_st.DataSize);
						OtdGsmApp_SendATCommand(CMDBuff);
					}
					else
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====No Server Data Available=====send\n");
						#endif	
					}
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====TCP_PREPARE_SEND=====send\n");
					#endif	
					
					TcpSubState_st.TcpCmdSendFlag = 1;
				}				
				break;
				
		case TCP_SEND_DATA:
				if(!TcpSubState_st.TcpCmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====TCP_SEND_DATA=====send\n");
					#endif	
					
					if(TcpHandle_st.IsServerDataReady == READY)
					{
						OtdGsmApp_SendATCommand(TcpHandle_st.ServerDataBuf);
						OtdDelay_ms(2000);//TODO: "SUCCESS" is recieved after some delay, need to remove this delay later
					}
					else
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====Server Data not available=====send\n");
						#endif							
						//wait in this state till the data is finalised
						//TcpSubState_st.TcpPrevious_en = TCP_SERVER_CONNECT;
						//TcpSubState_st.TcpCurrent_en = TCP_PREPARE_SEND;

					}
					
					TcpSubState_st.TcpCmdSendFlag = 1;
				}				
				break;	
				
		case TCP_CLOSE_SERVER:
				if(!TcpSubState_st.TcpCmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====TCP_CLOSE_SERVER=====send\n");
					#endif	
					
					OtdGsmApp_SendATCommand("AT+CIPCLOSE=1\r\n");
					
					TcpSubState_st.TcpCmdSendFlag = 1;
				}				
				break;	
				
		case TCP_NETWORK_CLOSE:
				if(!TcpSubState_st.TcpCmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====TCP_CLOSE_SERVER=====send\n");
					#endif	
					
					OtdGsmApp_SendATCommand("AT+NETCLOSE\r\n");
					
					TcpSubState_st.TcpCmdSendFlag = 1;
				}				
				break;	
				
		default:
				#if GSM_DEBUG
				OtdUart_DebugSend("TCP_DEBUG >> ========================================\n");
				OtdUart_DebugSend("TCP_DEBUG >> 	TCP Cmd State Not found		\n");
				OtdUart_DebugSend("TCP_DEBUG >> ========================================\n");
				#endif	
				break;			
	}
	
	status_e = OtdGsmTcpApp_TcpSendRxProcess(TcpSubState_st.TcpCurrent_en);//check if required response is recieved from GSM module
	
	if((status_e == Gsm_Nok) || (status_e == Gsm_RxTimeOut))
	{
		if(TcpSubState_st.TcpTrials > 0)
		{
			TcpSubState_st.TcpTrials--;
			TcpSubState_st.TcpTimeOut = RX_TIMEOUT;//reset the timer count
			TcpSubState_st.TcpCmdSendFlag = 0; // Reset command flag
			if(status_e == Gsm_RxTimeOut)
			{
				#if GSM_DEBUG
				OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====Rx Response Maximum Time over=====\n");
				#endif			
			}			
		}
		else
		{
			#if GSM_DEBUG
			OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====Maximum Trials Exceeded=====\n");
			#endif
			//Trials finished
			status_e = Gsm_Nok;
		}
	}
	else if(status_e == Gsm_Ok)
	{
		//TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
		//TcpSubState_st.TcpCurrent_en += 1; //Move to next state
		if(TcpSubState_st.TcpCurrent_en < Tcp_MaxSubState)
		{
			TcpSubState_st.TcpCmdSendFlag = 0; // Reset command flag
			TcpSubState_st.TcpTrials = 10; //Reset trials
		}
		else
		{
			//TcpSubState_st.TcpCurrent_en = GSM_AT;
			//TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
			
			#if GSM_DEBUG
			OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====Iniitalisation finished=====\n");
			#endif
			status_e = Gsm_SubStateCompleted;
		}
	}
	else
	{
		// GSM is busy
		//do nothing
	}
	
	#if GSM_DEBUG
	sprintf(data,"TCP_DEBUG >> TCP_IP Sub-Statemachine======================================Trials = %d\n",TcpSubState_st.TcpTrials);
	OtdUart_DebugSend(data);
	#endif
	
	return status_e;
}

/******************************************************end here***********************************/
void OtdGsmTcpApp_TcpStateInit(void)
{
	//sub state default init
	TcpSubState_st.TcpCurrent_en = TCP_QICSGP;
	TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
	TcpSubState_st.TcpTrials = MAX_TRIALS;
	TcpSubState_st.TcpTimeOut = RX_TIMEOUT;
	TcpSubState_st.TcpCmdSendFlag = 0;
	
	//RxIndex
	RxIndex = 0;		
}
static OtdGsmApp_Status_ten OtdGsmTcpApp_TcpRxProcess(OtdGsmTcpApp_SubState_ten rx_state)
{
	uint8_t data[512];
	//char __far data[256];
	uint8_t tcp_local_rx_buffer[RX_BUFFER_LENGTH]; 
	char __far *expected_reply;
	volatile OtdGsmApp_Status_ten Status = Gsm_Nok;
	//uint32_t fetched_ICCID;
	
	OtdUart_IsDataAvailable();
	
	if(RxIndex > 0)
	{
		#if GSM_DEBUG
		sprintf(data,"TCP_DEBUG >> GSM Rx response====Data Recieved from GSM Module=====Bytes = %d\n",RxIndex);
		OtdUart_DebugSend(data);
		#endif
		
		switch(rx_state)
		{

			case TCP_QICSGP:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing AT+QICSGP command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,tcp_local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						if(TcpSubState_st.TcpTimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;
					
			case TCP_CMGF:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing AT+CMGF command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),1,tcp_local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						if(TcpSubState_st.TcpTimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;	
						
				default:
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> ========================================\n");
						OtdUart_DebugSend("TCP_DEBUG >> 	Tcp Rx response State Not found	\n");
						OtdUart_DebugSend("TCP_DEBUG >> ========================================\n");
						#endif	
						break;
		}	
	}
	else
	{
		#if GSM_DEBUG
		OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====No Data Recieved from GSM Module=====\n");
		#endif	

		if(TcpSubState_st.TcpTimeOut <= 0)
		{
			Status = Gsm_RxTimeOut;
		}
		else
		{
			Status = Gsm_Busy;
		}
	}
		
	
	return Status;
}



OtdGsmApp_Status_ten OtdGsmTcpApp_TcpInitCmdProcess(void)
{
	uint8_t data[128];
	volatile OtdGsmApp_Status_ten status_e = Gsm_Nok;
	
	switch(TcpSubState_st.TcpCurrent_en)
	{
		case TCP_QICSGP:
				if(!TcpSubState_st.TcpCmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					//OtdGsmApp_SendATCommand("AT+QICSGP=1,1,\"WWW\",\"\",\"\"\r\n");
					OtdGsmApp_SendATCommand("AT+QICSGP=1,1,\"airtelgprs.com\",\"\",\"\"\r\n");
					//TcpSubState_st.TcpTimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====AT+QICSGP=====send\n");
					#endif	
					
					TcpSubState_st.TcpCmdSendFlag = 1;
				}
				
				break;	
				
		case TCP_CMGF:
				if(!TcpSubState_st.TcpCmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+CMGF=1\r\n");
					//TcpSubState_st.TcpTimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====AT+CMGF=====send\n");
					#endif	
					
					TcpSubState_st.TcpCmdSendFlag = 1;
				}
				
				break;		
				
		default:
				#if GSM_DEBUG
				OtdUart_DebugSend("TCP_DEBUG >> ========================================\n");
				OtdUart_DebugSend("TCP_DEBUG >> 	TCP Cmd State Not found		\n");
				OtdUart_DebugSend("TCP_DEBUG >> ========================================\n");
				#endif	
				break;
				
		//call the array of function pointer				
	}
	
	status_e = OtdGsmTcpApp_TcpRxProcess(TcpSubState_st.TcpCurrent_en);//check if required response is recieved from GSM module
	
	if((status_e == Gsm_Nok) || (status_e == Gsm_RxTimeOut))
	{
		if(TcpSubState_st.TcpTrials > 0)
		{
			TcpSubState_st.TcpTrials--;
			TcpSubState_st.TcpTimeOut = RX_TIMEOUT;//reset the timer count
			TcpSubState_st.TcpCmdSendFlag = 0; // Reset command flag
			if(status_e == Gsm_RxTimeOut)
			{
				#if GSM_DEBUG
				OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====Rx Response Maximum Time over=====\n");
				#endif			
			}			
		}
		else
		{
			#if GSM_DEBUG
			OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====Maximum Trials Exceeded=====\n");
			#endif
			//Trials finished
			status_e = Gsm_Nok;
		}
	}
	else if(status_e == Gsm_Ok)
	{
		TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
		TcpSubState_st.TcpCurrent_en += 1; //Move to next state
		if(TcpSubState_st.TcpCurrent_en <= TCP_CMGF)
		{
			TcpSubState_st.TcpCmdSendFlag = 0; // Reset command flag
			TcpSubState_st.TcpTrials = 10; //Reset trials
		}
		else
		{
			TcpSubState_st.TcpCurrent_en = GSM_AT;
			TcpSubState_st.TcpPrevious_en = TcpSubState_st.TcpCurrent_en;
			
			#if GSM_DEBUG
			OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====Iniitalisation finished=====\n");
			#endif
			status_e = Gsm_SubStateCompleted;
		}
	}
	else
	{
		// GSM is busy
		//do nothing
	}
	
	#if GSM_DEBUG
	sprintf(data,"TCP_DEBUG >> TCP_IP Sub-Statemachine======================================Trials = %d\n",TcpSubState_st.TcpTrials);
	OtdUart_DebugSend(data);
	#endif
	
	return status_e;
}