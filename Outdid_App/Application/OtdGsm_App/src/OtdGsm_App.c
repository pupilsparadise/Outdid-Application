#include <stdio.h>
#include "OtdGsm_App.h"
#include "OtdCircularBuffer_App.h"
#include "OtdUART.h"
#include "OtdDelay.h"

extern uint8_t gsm_tx_pending;//flag to confirm all bytes have transmitted
extern volatile uint16_t time_out;//count increments on every 1ms timer interrupt
extern volatile uint8_t rx_flag;

volatile static uint16_t tick_start = 0;

extern OtdCircularBufferApp *gsm_rx_ptr;

typedef enum
{
	NW_PASS = 0,
	NW_FAIL,
	NW_WAIT
}OtdGsmApp_ErrorState;

typedef enum
{
	GSM_STATE = 0,
	TCP_STATE,
	FIRMWARE_UPDATE_STATE,
}OtdGsmApp_MainState;

typedef enum
{
	/**< Reinitializing the GSM module */
	GSM_STATE_INIT = 0,		/**< ----GSM state 1	(Send AT for checking) 			*/
	GSM_ATI,			/**< ----GSM state 2	(Product identification)		*/
	GSM_GSN,			/**< ----GSM state 3	(Request IMEI)				*/
	GSM_STATE_CHECK_PIN,		/**< ----GSM state 4	(Check if PIN) 				*/
	GSM_STATE_ENTER_PIN,		/**< ----GSM state 5	(Enter PIN code) 			*/
	GSM_STATE_CCID,			/**< ----GSM state 6	(SIM Serial Number) 			*/
	GSM_STATE_SET_SLEEP_MODE,	/**< ----GSM state 7	(Config sleep mode(Slow clock) 		*/
	GSM_STATE_WAIT_PROVIDER,	/**< ----GSM state 8	(network provider) 			*/
	GSM_STATE_CGREG,		/**< ----GSM state 9	(network registration status)		*/
	GSM_STATE_SIGNAL_QUALITY,	/**< ----GSM state 10 	(signal quality) 			*/
	GSM_STATE_CGATT,		/**< ----GSM state 11 	(Attach to GPRS) 			*/
	GSM_STATE_APN,			/**< ----GSM state 12 	(Select CSD or GPRS(1) as the Bearer	*/
	GSM_QISTAT,			/**< ----GSM state 13 	(Query current connection status) 	*/
	GSM_QIREGAPP,			/**< ----GSM state 14 	(Start TCP task)			*/
	GSM_QIACT,			/**< ----GSM state 15 	(Activate GPRS context) 		*/
	GSM_QIDEACT,			/**< ----GSM state 16 	(Deactivate GPRS context) 		*/
	GSM_STATE_READY,		/**< ----GSM state 17 	(GSM ready) 				*/
	GSM_FGCNT,			/**< ----GSM state 18 	(Foreground context) 			*/
	GSM_CSGP,			/**< ----GSM state 19 	(Select CSD or GPRS(1) as the Bearer 	*/
	GSM_SHUTDOWN,			/**< ----GSM state 20 	(For Shutting down GSM module) 		*/
	
	/**< Commands for sending an SMS */
	GSM_CMGF,											/**< For setting SMS text mode 								*/
	GSM_CSMP,											/**< For setting the parameter to send SMS text mode 		*/
	GSM_CMGS,											/**< For sending it to SMS mode 								*/
	GSM_CSCS,											/**< Set as GSM mode 										*/
	GSM_CNMI,											/**< For setting status receiving management 				*/
	GSM_CMGD,											/**< For deleting Send and Recv Message from In box			*/
	
	GSM_WAIT_STATE
}OtdGsmApp_GsmInitState;

typedef struct
{
	volatile OtdGsmApp_MainState nw_main_state;//stores main state
	uint8_t nw_current_sub_state;//stores current substate
	uint8_t nw_previous_sub_state;//stores previous substate
	uint8_t nw_trials; //stores the number of trials for sending to gsm
	uint8_t nw_timeout;
	uint8_t nw_rx_flag;//indicates if data is recieved
	OtdGsmApp_ErrorState nw_substate_error;//stores error of substate

}OtdGsmApp_NetworkState;

OtdGsmApp_NetworkState network_state = 	{	GSM_STATE, 	//Main state
						GSM_STATE_INIT,	//current substate
						GSM_STATE_INIT,	//previous substate
						10, //trials
						20, //timeout
						0,//rx flag
						NW_FAIL
					};

static void OtdGsmApp_SendATCommand(const char *s)
{
	while(*s != '\0')
	{
		OtdUart_Send((uint8_t *__near)s++, 1,GSM_UART);
		while(!gsm_tx_pending);
		gsm_tx_pending = 0;
	}
}
static OtdGsmApp_ErrorState OtdGsmApp_ProcessRxInitCommand(OtdGsmApp_GsmInitState gsm_rx_state)
{
	OtdGsmApp_ErrorState rx_status = NW_FAIL;
	uint8_t data[80];

	switch(gsm_rx_state)
	{
		case GSM_STATE_INIT:
					//add timeout
					tick_start = OtdDelay_GetTicks();
					if((OtdDelay_GetTicks() -  tick_start)< 20)
					{

					}
					

					break;
		case GSM_ATI:
					//add timeout
					tick_start = OtdDelay_GetTicks();
					if((OtdDelay_GetTicks() -  tick_start)< 20)
					{

					}
					

					break;					
	}
	
	if(OtdCircularBufferApp_IsResponse("OK\r\n") == OtdCircularBufferApp_Pass)
	{
		rx_status = NW_PASS;
		
		#if GSM_DEBUG == 1
		OtdUart_DebugSend("GSM_INIT_PROCESS >> RecvResponse==============OK\n");
		sprintf(data,"GSM_INIT_RX_PROCESS >> Current state = %d\n", gsm_rx_state);
		OtdUart_DebugSend(data);
		#endif		
	}
	else
	{
		rx_status = NW_WAIT;
		#if GSM_DEBUG == 1
		OtdUart_DebugSend("GSM_INIT_PROCESS >> RecvRespons================FAILED\n");
		sprintf(data,"GSM_INIT_RX_PROCESS >> Current state = %d\n", gsm_rx_state);
		OtdUart_DebugSend(data);
		#endif	
	}
						
	
	return rx_status;
}

static uint8_t trials = 0;
volatile uint16_t time_diff = 0;
volatile uint16_t is_data_rx = 10;
//GSM initialisation state machine
uint8_t OtdGsmApp_ProcessInitCommand(void)
{
	volatile uint8_t temp_state;
	uint8_t data[80];
	//static uint8_t trials = 0;
	
	switch(network_state.nw_current_sub_state) 
	{
		case GSM_STATE_INIT:
					if((network_state.nw_substate_error == NW_FAIL) && (trials < network_state.nw_trials))
					{
						OtdCircularBufferApp_BufferClear(GSM_UART);//clear the rx buffer
						OtdGsmApp_SendATCommand("AT\r\n");//send to gsm
						
						
						#if GSM_DEBUG == 1
						OtdUart_DebugSend("GSM_STATE_INIT>> AT command send\n");
						#endif
						
						network_state.nw_current_sub_state = GSM_WAIT_STATE;//change to wait state to recieve data from gsm
						tick_start = OtdDelay_GetTicks();//get the current tick
						trials++;//update trials
						
						#if GSM_DEBUG == 1
						sprintf(data,"trials = %d\n",trials);
						OtdUart_DebugSend(data);
						#endif
					}
					
					if(network_state.nw_previous_sub_state == GSM_WAIT_STATE && (trials < network_state.nw_trials))
					{
						if(network_state.nw_substate_error == NW_PASS)
						{	
							#if GSM_DEBUG == 1
							OtdUart_DebugSend("GSM_STATE_INIT>> response for AT command success\n");
							#endif
							//success
							//change current and previous state to next state
							trials = 0;
							network_state.nw_substate_error = NW_FAIL;
							network_state.nw_current_sub_state = GSM_ATI;
							network_state.nw_previous_sub_state = GSM_ATI;
						}
						else//response failed
						{
							//failed retry again for 10 times
							//change the currrent and previous state to GSM_STATE_INIT
							#if GSM_DEBUG == 1
							OtdUart_DebugSend("GSM_STATE_INIT>> response for AT command failed\n");
							#endif
							network_state.nw_current_sub_state = GSM_STATE_INIT;
							network_state.nw_previous_sub_state = GSM_STATE_INIT;
						}
					}
					else
					{
						#if GSM_DEBUG == 1
						OtdUart_DebugSend("GSM_STATE_INIT>> not got data from wait state\n");
						#endif
					}
					
					break;
					

		case GSM_ATI:
					if((network_state.nw_substate_error == NW_FAIL) && (trials < network_state.nw_trials))
					{
						//OtdCircularBufferApp_BufferClear(GSM_UART);//clear the rx buffer
						OtdGsmApp_SendATCommand("ATI\r\n");//send to gsm
						
						#if GSM_DEBUG == 1
						OtdUart_DebugSend("GSM_STATE_INIT>> ATI command send\n");
						#endif
						
						network_state.nw_current_sub_state = GSM_WAIT_STATE;//change to wait state to recieve data from gsm
						tick_start = OtdDelay_GetTicks();//get the current tick
						trials++;//update trials
						
						#if GSM_DEBUG == 1
						sprintf(data,"trials = %d\n",trials);
						OtdUart_DebugSend(data);
						#endif
					}
					
					if(network_state.nw_previous_sub_state == GSM_WAIT_STATE && (trials < network_state.nw_trials))
					{
						if(network_state.nw_substate_error == NW_PASS)
						{	
							#if GSM_DEBUG == 1
							OtdUart_DebugSend("GSM_STATE_INIT>> response for ATI command success\n");
							#endif
							//success
							//change current and previous state to next state
							trials = 0;
							network_state.nw_substate_error = NW_FAIL;							
							network_state.nw_current_sub_state = GSM_STATE_INIT;
							network_state.nw_previous_sub_state = GSM_STATE_INIT;
						}
						else//response failed
						{
							//failed retry again for 10 times
							//change the currrent and previous state to GSM_STATE_INIT
							#if GSM_DEBUG == 1
							OtdUart_DebugSend("GSM_STATE_INIT>> response for ATI command failed\n");
							#endif
							network_state.nw_current_sub_state = GSM_STATE_INIT;
							network_state.nw_previous_sub_state = GSM_STATE_INIT;
						}
					}
					else
					{
						#if GSM_DEBUG == 1
						OtdUart_DebugSend("GSM_STATE_INIT>> not got data from wait state\n");
						#endif
					}
					
					break;					
		case GSM_WAIT_STATE:
					time_diff = OtdDelay_GetTicks() -  tick_start;
					is_data_rx = OtdCircularBufferApp_IsData(GSM_UART);
					//OtdCircularBufferApp_IsData(GSM_UART); --> if any data recieved then process for next, add a timeout here
					//if((OtdCircularBufferApp_IsData(GSM_UART) == 0) && (time_diff < network_state.nw_timeout))
					if((is_data_rx == 0)/*&& (rx_flag == 1)*/ && (time_diff < network_state.nw_timeout))
					{
						#if GSM_DEBUG == 1
						//rx data not recieved yet
						OtdUart_DebugSend("GSM_WAIT_STATE>> No response time-out\n");
						#endif

					}
					else
					{
						//if((OtdCircularBufferApp_IsData(GSM_UART) != 0))
						is_data_rx = OtdCircularBufferApp_IsData(GSM_UART);
						if((is_data_rx != 0) /*&& (rx_flag == 1)*/)
						{
							//rx data received
							/*call with called state which is updated in "nw_previous_sub_state"*/
							if(OtdGsmApp_ProcessRxInitCommand(network_state.nw_previous_sub_state) == NW_PASS)
							{
								network_state.nw_substate_error = NW_PASS;
								#if GSM_DEBUG == 1
								OtdUart_DebugSend("GSM_WAIT_STATE>> response from GSM success\n");
								#endif
							}
							else
							{
								network_state.nw_substate_error = NW_FAIL;
								#if GSM_DEBUG == 1
								OtdUart_DebugSend("GSM_WAIT_STATE>> response from GSM failed\n");
								#endif
							}
						}
						else
						{
							//data not recieved and timeout has happened
							network_state.nw_substate_error = NW_FAIL;
						}
					}
					
					
					

					/*go back to previous state*/
					/*called state will take decision*/
					temp_state = network_state.nw_previous_sub_state;
					network_state.nw_previous_sub_state = network_state.nw_current_sub_state;
					network_state.nw_current_sub_state = temp_state;
					
					break;
	}
	
}


//Network main state machine
void OtdGsmApp_MainProcessATCmd(void)
{
	switch(network_state.nw_main_state)
	{
		case GSM_STATE:	
				OtdGsmApp_ProcessInitCommand();
				#if GSM_DEBUG == 1
				OtdUart_DebugSend("GSM_STATE>> GSM Main State Executed\n");
				#endif
				break;
		case TCP_STATE:
				break;
		case FIRMWARE_UPDATE_STATE:
				break;
				
		default :
				#if GSM_DEBUG == 1
				OtdUart_DebugSend("GSM_INIT>> Default\n");
				#endif
				break;
		
	}
}

