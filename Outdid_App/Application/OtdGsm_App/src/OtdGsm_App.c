#include <stdio.h>
#include <string.h>
#include "OtdGsm_App.h"
#include "OtdUART.h"
#include "OtdDelay.h"
#include "OtdGsm_TcpApp.h"



volatile uint16_t RxIndex;
extern uint8_t gsm_tx_pending;//flag to confirm all bytes have transmitted
volatile char Gsm_RxBuffer[RX_BUFFER_LENGTH];

OtdGsmApp_MainState_tst GsmMainState_st;
OtdGsmApp_SubState_tst GsmSubState_st;

void OtdGsmApp_ClearRxBuffer(void)
{
	memset((void*)Gsm_RxBuffer,0,RX_BUFFER_LENGTH);
	RxIndex = 0;
}

/**
* \b Description:
*
* convert a char to integer type .
*
* @param Buffer	: Buffer containing the ASCII string
* 		 length	: Buffer Length
* 		 check_type: true/ false(Type of checking (User wants to check integer type of not))
* @return Integer value
*/
//uint64_t variable = 0;
uint64_t OtdGsmApp_ConvertCharToInteger(const char *ptr, uint8_t length, uint8_t check_type)
{
	uint64_t variable = 0;
	uint8_t i;
	for(i = 0; i < length; i++)
	{
		if(check_type == FALSE)
		{
			variable = variable * 10 + ((ptr[i] - '0'));
		}
        else if(ptr[i] >= '0' && ptr[i] <= '9')
		{
			variable = variable * 10 + ((ptr[i] - '0'));
		}
	}
	return variable;
}

uint8_t OtdGsmApp_IsSubArrayPresent(volatile const char *array, uint16_t array_len, const char *subarray, uint16_t subarray_len, uint8_t save_reply, uint8_t *cmd_reply)
{
	OtdGsmApp_Status_ten match = Gsm_Ok;
	uint16_t i,j;
	
	if (array_len < subarray_len) 
	{
		return Gsm_Nok;
	}
	
	if(!(array_len) || (!subarray_len))//length shouldnot be 0
	{
		return Gsm_Nok;
	}

	for (i = 0; i <= (array_len - subarray_len); ++i)
	{
		match = Gsm_Ok;

		for ( j = 0; j < subarray_len; ++j) 
		{
		    if (array[i + j] != subarray[j]) 
		    {
		    	match = Gsm_Nok;
		        break;
		    }
		}
		if (match == Gsm_Ok)
		{
			if(save_reply)
			{
				memcpy(cmd_reply,(const char *)array,array_len);
			}
		    	return Gsm_Ok;
		}
	}

	return Gsm_Nok;
}


void OtdGsmApp_SendATCommand(const char *s)
{ 
	const uint8_t debug_data[128];
	sprintf(debug_data,"%s",s);
	/*
	while(*s != '\0')
	{
		OtdUart_Send((uint8_t *__near)s++, 1,GSM_UART);
		while(!gsm_tx_pending);
		gsm_tx_pending = 0;
	}*/
	
	OtdUart_Send((uint8_t *__near)debug_data, strlen(s),GSM_UART);
	while(!gsm_tx_pending);
	gsm_tx_pending = 0;
}


uint8_t local_rx_buffer[RX_BUFFER_LENGTH];
uint64_t fetched_ICCID;
static OtdGsmApp_Status_ten OtdGsmApp_GsmRxProcess(OtdGsmApp_SubState_ten rx_state)
{
	uint8_t data[512];
	//char __far data[256];
	//uint8_t local_rx_buffer[RX_BUFFER_LENGTH]; 
	char __far *expected_reply;
	volatile OtdGsmApp_Status_ten Status = Gsm_Nok;
	//uint32_t fetched_ICCID;
	
	if(RxIndex > 0)
	{
		#if GSM_DEBUG
		sprintf(data,"GSM_DEBUG >> GSM Rx response====Data Recieved from GSM Module=====Bytes = %d\n",RxIndex);
		OtdUart_DebugSend(data);
		#endif
		
		switch(rx_state)
		{

			case GSM_AT:
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Processing AT command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),1,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;
			case GSM_ATI:
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Processing ATI command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;
			case GSM_GSN:
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Processing AT+GSN command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;	

			case GSM_STATE_CHECK_PIN:
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Processing AT+GSN command=====\n");
					#endif	
					
					expected_reply = "+CPIN: READY";//update expected reply
					
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response for AT+GSN Present=====\n");
						#endif	
						
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;
					
			case GSM_STATE_ICCID:
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Processing AT+ICCID command=====\n");
					#endif	
					
					expected_reply = "+ICCID";//update expected reply //TODO: Check when is SIM is removed what happens and update the error code
					
					//save the reply for verification
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),1,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response for AT+ICCID Present=====\n");
						#endif	
						
						
						#if GSM_DEBUG
						fetched_ICCID = OtdGsmApp_ConvertCharToInteger(&local_rx_buffer[18],19, FALSE);
						sprintf(data,"GSM_DEBUG >> fetched_ICCID = %lld\n",fetched_ICCID);
						OtdUart_DebugSend(data);
						#endif		
						
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;		
					
			case GSM_STATE_SIGNAL_QUALITY:
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Processing GSM_STATE_SIGNAL_QUALITY command=====\n");
					#endif	
					
					expected_reply = "+CSQ:";//update expected reply 
					
					//save the reply for verification
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),1,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response for AT+CSQ Present=====\n");
						#endif	
						
						//TODO: add function to check the Signal Quality is feasible or not
						
						#if GSM_DEBUG
						//fetched_ICCID = OtdGsmApp_ConvertCharToInteger(&local_rx_buffer[18],19, FALSE);
						//sprintf(data,"GSM_DEBUG >> fetched_ICCID = %lld\n",fetched_ICCID);
						//OtdUart_DebugSend(data);
						#endif		
						
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;	
					
			case GSM_CNMI:
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Processing GSM_CNMI command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply 
					
					//save the reply for verification
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response for AT+CNMI Present=====\n");
						#endif	
							
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;	

			case GSM_CGQMIN:
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Processing GSM_CGQMIN command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply 
					
					//save the reply for verification
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response for AT+CGQMIN Present=====\n");
						#endif	
							
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					break;	
					
			case GSM_COPS:
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Processing GSM_COPS command=====\n");
					#endif	
					
					expected_reply = "OK";//update expected reply 
					
					//save the reply for verification
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response for AT+COPS Present=====\n");
						#endif	
							
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					
					break;	
					
			case GSM_COPS_CHECK:
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Processing GSM_COPS_CHECK command=====\n");
					#endif	
					
					expected_reply = "Vi";//update expected reply //TODO: store the opearator name
					
					//save the reply for verification
					if(OtdGsmApp_IsSubArrayPresent(Gsm_RxBuffer,RxIndex,expected_reply,strlen(expected_reply),0,local_rx_buffer) == Gsm_Ok)
					{
						#if GSM_DEBUG
						OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response for AT+COPS_CHECK Present=====\n");
						#endif	
							
						Status = Gsm_Ok;
					}
					else
					{
						if(GsmSubState_st.TimeOut <= 0)
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response Not Present & Timeout=====\n");
							#endif	
							Status = Gsm_RxTimeOut;
						}
						else
						{
							#if GSM_DEBUG
							OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====Required Response is busy & No Timeout=====\n");
							#endif
							Status = Gsm_Busy;
						}
					}
					
					
					break;						
				default:
						#if GSM_DEBUG
						OtdUart_DebugSend("GSM_DEBUG >> ========================================\n");
						OtdUart_DebugSend("GSM_DEBUG >> 	GSM Rx response State Not found	\n");
						OtdUart_DebugSend("GSM_DEBUG >> ========================================\n");
						#endif	
						break;
		}	
	}
	else
	{
		#if GSM_DEBUG
		OtdUart_DebugSend("GSM_DEBUG >> GSM Rx response====No Data Recieved from GSM Module=====\n");
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
 
static OtdGsmApp_Status_ten OtdGsmApp_GsmInitCmdProcess(void)
{
	uint8_t data[128];
	volatile OtdGsmApp_Status_ten status_e = Gsm_Nok;
	
	switch(GsmSubState_st.Current_en)
	{
		case GSM_AT:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT\r\n");
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====AT=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;
				}
				
				break;
		case GSM_ATI:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("ATI\r\n");
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====ATI=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;
				}		
				break;
		case GSM_GSN:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+GSN\r\n");
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====AT+GSN=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;//ensure that the command to gsm is sent one time
				}		
				break;	

		case GSM_STATE_CHECK_PIN:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+CPIN?\r\n");
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====AT+CPIN?=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;//ensure that the command to gsm is sent one time
				}		
				break;

		case GSM_STATE_ICCID:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+ICCID\r\n");//update the command
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====AT+ICCID=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;//ensure that the command to gsm is sent one time
				}		
				break;		
				
		case GSM_STATE_SIGNAL_QUALITY:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+CSQ\r\n");//update the command
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====AT+CSQ=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;//ensure that the command to gsm is sent one time
				}		
				break;		
				
				
		case GSM_CNMI:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+CNMI=2,1,0,0,0\r\n");//update the command
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====AT+CNMI=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;//ensure that the command to gsm is sent one time
				}		
				break;	
				
		case GSM_CGQMIN:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+CGQMIN?\r\n");//update the command
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====AT+CGQMIN=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;//ensure that the command to gsm is sent one time
				}		
				break;	
				
		case GSM_COPS:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+COPS=3,0\r\n");//update the command
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====AT+COPS=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;//ensure that the command to gsm is sent one time
				}		
				break;	
		case GSM_COPS_CHECK:
				if(!GsmSubState_st.CmdSendFlag)
				{
					OtdGsmApp_ClearRxBuffer();//Clear Rx Buffer 
					OtdGsmApp_SendATCommand("AT+COPS?\r\n");//update the command
					//GsmSubState_st.TimeOut = RX_TIMEOUT;
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====AT+COPS_CHECK=====send\n");
					#endif	
					
					GsmSubState_st.CmdSendFlag = 1;//ensure that the command to gsm is sent one time
				}		
				break;						
				
		default:
				#if GSM_DEBUG
				OtdUart_DebugSend("GSM_DEBUG >> ========================================\n");
				OtdUart_DebugSend("GSM_DEBUG >> 	GSM Cmd State Not found		\n");
				OtdUart_DebugSend("GSM_DEBUG >> ========================================\n");
				#endif	
				break;
				
		//call the array of function pointer				
	}
	
	status_e = OtdGsmApp_GsmRxProcess(GsmSubState_st.Current_en);//check if required response is recieved from GSM module
	
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
				OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====Rx Response Maximum Time over=====\n");
				#endif			
			}			
		}
		else
		{
			#if GSM_DEBUG
			OtdUart_DebugSend("GSM_DEBUG >> GSM Sub-StateMachine====Maximum Trials Exceeded=====\n");
			#endif
			//Trials finished
			status_e = Gsm_Nok;
		}
	}
	else if(status_e == Gsm_Ok)
	{
		GsmSubState_st.Previous_en = GsmSubState_st.Current_en;
		GsmSubState_st.Current_en += 1; //Move to next state
		if(GsmSubState_st.Current_en < GSM_INIT_MAX_COMMAND)
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
	sprintf(data,"GSM_DEBUG >> GSM Sub-Statemachine======================================Trials = %d\n",GsmSubState_st.Trials);
	OtdUart_DebugSend(data);
	#endif
	
	return status_e;
}

void OtdGsmApp_MainStateMachine(void)
{
	volatile OtdGsmApp_Status_ten Status = Gsm_Nok;
	
	switch(GsmMainState_st.Current_en)
	{
		case Gsm_Init:
				Status = OtdGsmApp_GsmInitCmdProcess();
				
				if(Status == Gsm_SubStateCompleted)
				{
					GsmMainState_st.Previous_en = GsmMainState_st.Current_en;//update the previous while changing to next state
					GsmMainState_st.Current_en = Tcp_Init;
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> =======================================\n");
					OtdUart_DebugSend("GSM_DEBUG >> 		GSM INIT FINISHED      \n");
					OtdUart_DebugSend("GSM_DEBUG >> =======================================\n");
					#endif
					OtdGsmTcpApp_TcpStateInit();//basic init for Tcp_Init
					#if GSM_DEBUG
					OtdUart_DebugSend("GSM_DEBUG >> Main state switch from ====GSM_INIT to TCP_INIT===\n");
					#endif
				}
				
				#if GSM_DEBUG
				OtdUart_DebugSend("GSM_DEBUG >> Executing MainSateMachine=====Gsm_Init=====\n");
				#endif
				
				//update to next state depending on status
				
				break;
		case Tcp_Init:
				Status = OtdGsmTcpApp_TcpInitCmdProcess();
				
				if(Status == Gsm_SubStateCompleted)
				{
					GsmMainState_st.Previous_en = GsmMainState_st.Current_en;//update the previous while changing to next state
					GsmMainState_st.Current_en = Tcp_Send;
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> =======================================\n");
					OtdUart_DebugSend("TCP_DEBUG >> 		TCP INIT FINISHED      \n");
					OtdUart_DebugSend("TCP_DEBUG >> =======================================\n");
					#endif
					OtdGsmTcpApp_TcpSendStateInit();//basic init for Tcp_Send Init
					#if GSM_DEBUG
					OtdUart_DebugSend("TCP_DEBUG >> Main state switch from ====Tcp_Init to TCP_Send===\n");
					#endif
				}
				#if GSM_DEBUG
				OtdUart_DebugSend("GSM_DEBUG >> Executing MainSateMachine=====Tcp_Init=====\n");
				#endif		
				break;
		case Tcp_Send:
				Status = OtdGsmTcpApp_TcpSendCmdProcess();
				#if GSM_DEBUG
				OtdUart_DebugSend("GSM_DEBUG >> Executing MainSateMachine=====Tcp_Send=====\n");
				#endif			
				break;
		case FirmwareUpdate:
				#if GSM_DEBUG
				OtdUart_DebugSend("GSM_DEBUG >> Executing MainSateMachine=====FirmwareUpdate=====\n");
				#endif			
		
				break;
				
		//call the array of function pointer				
	}

			
}
void OtdGsmApp_GsmStateInit(void)
{
	//main state default init
	GsmMainState_st.Current_en  = Gsm_Init;
	GsmMainState_st.Previous_en = GsmMainState_st.Current_en;
	
	//sub state default init
	GsmSubState_st.Current_en = GSM_AT;
	GsmSubState_st.Previous_en = GsmSubState_st.Current_en;
	GsmSubState_st.Trials = MAX_TRIALS;
	GsmSubState_st.TimeOut = RX_TIMEOUT;
	GsmSubState_st.CmdSendFlag = 0;
	
	//RxIndex
	RxIndex = 0;
	
	#if GSM_DEBUG
	OtdUart_DebugSend("GSM_DEBUG >> GSM StateMachine init completed\n");
	#endif
}