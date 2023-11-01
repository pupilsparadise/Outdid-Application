#include "OtdGsm_TcpApp.h"
#include "OtdUART.h"
#include "OtdDelay.h"
#include <stdio.h>
#include <string.h>

extern OtdGsmApp_SubState_tst GsmSubState_st;
extern volatile uint16_t RxIndex;
extern uint8_t gsm_tx_pending;//flag to confirm all bytes have transmitted
extern volatile char Gsm_RxBuffer[RX_BUFFER_LENGTH];

typedef struct
{
	char ServerDataBuf[256];
	uint16_t DataSize;
	uint8_t ServerDataIsReady;
}OtdGsm_TcpApp_DataPrepare;

OtdGsm_TcpApp_DataPrepare ServerDataPrepare;

static const char cipopen_cmd[] = "AT+CIPOPEN=1,\"TCP\",\"122.166.210.142\",8050\r\n";

typedef struct
{
	volatile uint8_t server_connection: 1; //server with specified ip is connected
	volatile uint8_t tcp_connection: 1;    //tcp connection is estalblish if NETOPEN is success
}OtdGsm_TcpApp_TcpNwState;

OtdGsm_TcpApp_TcpNwState network_state;

/***************************************************TCP Publish and recieve handle of data***********************************/
void OtdGsmTcpApp_TcpSendStateInit(void)
{
	//sub state default init
	GsmSubState_st.Current_en = TCP_NETOPEN;
	GsmSubState_st.Previous_en = GsmSubState_st.Current_en;
	GsmSubState_st.Trials = MAX_TRIALS;
	GsmSubState_st.TimeOut = RX_TIMEOUT;
	GsmSubState_st.CmdSendFlag = 0;
	
	//RxIndex
	RxIndex = 0;		
}

void OtdGsmTcpApp_ServerPrepareData(char *data, uint16_t data_length)
{
	uint16_t data_index;
	/*Clear the data buffer before preparing */
	memset(ServerDataPrepare.ServerDataBuf,'\0',sizeof(ServerDataPrepare.ServerDataBuf));
	
	//Copy the requested datat to global buffer
	for(data_index = 0; data_index < data_length;data_index++)
	{
		ServerDataPrepare.ServerDataBuf[data_index] = data[data_index];
	}
	
	ServerDataPrepare.DataSize = data_length;
	ServerDataPrepare.ServerDataIsReady = READY;
}

static OtdGsmApp_Status_ten OtdGsmTcpApp_TcpSendRxProcess(OtdGsmApp_SubState_ten rx_state)
{
	uint8_t data[512];
	//char __far data[256];
	uint8_t  local_rx_buffer[RX_BUFFER_LENGTH]; 
	char __far *expected_reply;
	volatile OtdGsmApp_Status_ten Status = Gsm_Nok;
	//uint32_t fetched_ICCID;
	
	if(RxIndex > 0)
	{
		#if GSM_DEBUG
		sprintf(data,"TCP_DEBUG >> GSM Rx response====Data Recieved from GSM Module=====Bytes = %d\n",RxIndex);
		OtdUart_DebugSend(data);
		#endif
		
		switch(rx_state)
		{
			case TCP_NETOPEN:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing AT+NETOPEN command=====\n");
					#endif	
					
					expected_reply = "SUCCESS";//update expected reply
					
					if(
						OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),1,local_rx_buffer) == Gsm_Ok||
						// ERROR: 902 and FAIL is recieved if the NETOPEN command is already executed or the connection is already established
						OtdGsmApp_IsSubArrayPresent(local_rx_buffer,RX_BUFFER_LENGTH,(uint8_t*)"ERROR: 902",sizeof("ERROR: 902")-1,0,local_rx_buffer)== Gsm_Ok||
						OtdGsmApp_IsSubArrayPresent(local_rx_buffer,RX_BUFFER_LENGTH,(uint8_t*)"FAIL",sizeof("FAIL")-1,0,local_rx_buffer)== Gsm_Ok
					)
					{
						network_state.tcp_connection = 1; //TCP connected
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						network_state.tcp_connection = 0; //Error in NETOPEN command so the connection is not established
						
						if(GsmSubState_st.TimeOut <= 0)
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
					
			case TCP_CIPOPEN:
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Processing AT+CIPOPEN command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,local_rx_buffer) == Gsm_Ok)
					{
						network_state.server_connection = 1; //Server connection established
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
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
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
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
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
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

		if(GsmSubState_st.TimeOut <= 0)
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
OtdGsmApp_Status_ten OtdGsmTcpApp_TcpSendCmdProcess(void)
{
	uint8_t data[128];
	char CMDBuff[50] = {0};
	volatile OtdGsmApp_Status_ten status_e = Gsm_Nok;
	
	switch(GsmSubState_st.Current_en)
	{		
		case TCP_NETOPEN:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+NETOPEN\r\n");
					OtdDelay_ms(5000);//TODO: "SUCCESS" is recieved after some delay, need to remove this delay later
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====AT+NETOPEN=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;
				}
				
		case TCP_CIPOPEN:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					//OtdGsmApp_SendATCommand("AT+CIPOPEN=1,\"TCP\",\"122.166.210.142\",8050\r\n");
					sprintf(CMDBuff,"AT+CIPOPEN=1,\"TCP\",\"%s\",%s\r\n", SERVER_IP_ADDRESS, SERVER_PORT);
					OtdGsmApp_SendATCommand(CMDBuff);
					//OtdGsmApp_SendATCommand(cipopen_cmd);
					OtdDelay_ms(5000);//TODO: "SUCCESS" is recieved after some delay, need to remove this delay later
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====TCP_CIPOPEN=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;
				}				
				break;	
				
		case TCP_PREPARE_SEND:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 

					
					if(ServerDataPrepare.ServerDataIsReady == READY)
					{
						sprintf(CMDBuff,"AT+CIPSEND=1,%d\r\n",ServerDataPrepare.DataSize);
						OtdGsmApp_SendATCommand(CMDBuff);
					}
					else
					{
						//wait in this state till the data is finalised
						GsmSubState_st.Current_en = TCP_PREPARE_SEND;
						GsmSubState_st.Previous_en = TCP_CIPOPEN;
					}
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====TCP_PREPARE_SEND=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;
				}				
				break;
				
		case TCP_SEND_DATA:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====TCP_SEND_DATA=====send\n");
					#endif	
					
					if(ServerDataPrepare.ServerDataIsReady == READY)
					{
						OtdGsmApp_SendATCommand(ServerDataPrepare.ServerDataBuf);
						OtdDelay_ms(1500);//TODO: "SUCCESS" is recieved after some delay, need to remove this delay later
					}
					else
					{
						//wait in this state till the data is finalised
						GsmSubState_st.Previous_en = TCP_CIPOPEN;
						GsmSubState_st.Current_en = TCP_PREPARE_SEND;

					}
					
					GsmSubState_st.CmdSendFlag = 1;
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
	
	status_e = OtdGsmTcpApp_TcpSendRxProcess(GsmSubState_st.Current_en);//check if required response is recieved from GSM module
	
	if((status_e == Gsm_Nok) || (status_e == Gsm_RxTimeOut))
	{
		if(GsmSubState_st.Trials > 0)
		{
			GsmSubState_st.Trials--;
			GsmSubState_st.TimeOut = RX_TIMEOUT;//reset the timer count
			GsmSubState_st.CmdSendFlag = 0; // Reset command flag
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
		GsmSubState_st.Previous_en = GsmSubState_st.Current_en;
		GsmSubState_st.Current_en += 1; //Move to next state
		if(GsmSubState_st.Current_en < TCP_SEND_MAX_COMMAND)
		{
			GsmSubState_st.CmdSendFlag = 0; // Reset command flag
			GsmSubState_st.Trials = 10; //Reset trials
		}
		else
		{
			GsmSubState_st.Current_en = GSM_AT;
			GsmSubState_st.Previous_en = GsmSubState_st.Current_en;
			
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
	sprintf(data,"TCP_DEBUG >> TCP_IP Sub-Statemachine======================================Trials = %d\n",GsmSubState_st.Trials);
	OtdUart_DebugSend(data);
	#endif
	
	return status_e;
}

/******************************************************end here***********************************/
void OtdGsmTcpApp_TcpStateInit(void)
{
	//sub state default init
	GsmSubState_st.Current_en = TCP_QICSGP;
	GsmSubState_st.Previous_en = GsmSubState_st.Current_en;
	GsmSubState_st.Trials = MAX_TRIALS;
	GsmSubState_st.TimeOut = RX_TIMEOUT;
	GsmSubState_st.CmdSendFlag = 0;
	
	//RxIndex
	RxIndex = 0;		
}
static OtdGsmApp_Status_ten OtdGsmTcpApp_TcpRxProcess(OtdGsmApp_SubState_ten rx_state)
{
	uint8_t data[512];
	//char __far data[256];
	uint8_t local_rx_buffer[RX_BUFFER_LENGTH]; 
	char __far *expected_reply;
	volatile OtdGsmApp_Status_ten Status = Gsm_Nok;
	//uint32_t fetched_ICCID;
	
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
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
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
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),1,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("TCP_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
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

		if(GsmSubState_st.TimeOut <= 0)
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
	
	switch(GsmSubState_st.Current_en)
	{
		case TCP_QICSGP:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+QICSGP=1,1,\"WWW\","",""\r\n");
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====AT+QICSGP=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;
				}
				
				break;	
				
		case TCP_CMGF:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+CMGF=1\r\n");
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> GSM Sub-StateMachine====AT+CMGF=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;
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
	
	status_e = OtdGsmTcpApp_TcpRxProcess(GsmSubState_st.Current_en);//check if required response is recieved from GSM module
	
	if((status_e == Gsm_Nok) || (status_e == Gsm_RxTimeOut))
	{
		if(GsmSubState_st.Trials > 0)
		{
			GsmSubState_st.Trials--;
			GsmSubState_st.TimeOut = RX_TIMEOUT;//reset the timer count
			GsmSubState_st.CmdSendFlag = 0; // Reset command flag
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
		GsmSubState_st.Previous_en = GsmSubState_st.Current_en;
		GsmSubState_st.Current_en += 1; //Move to next state
		if(GsmSubState_st.Current_en < TCP_INIT_MAX_COMMAND)
		{
			GsmSubState_st.CmdSendFlag = 0; // Reset command flag
			GsmSubState_st.Trials = 10; //Reset trials
		}
		else
		{
			GsmSubState_st.Current_en = GSM_AT;
			GsmSubState_st.Previous_en = GsmSubState_st.Current_en;
			
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
	sprintf(data,"TCP_DEBUG >> TCP_IP Sub-Statemachine======================================Trials = %d\n",GsmSubState_st.Trials);
	OtdUart_DebugSend(data);
	#endif
	
	return status_e;
}