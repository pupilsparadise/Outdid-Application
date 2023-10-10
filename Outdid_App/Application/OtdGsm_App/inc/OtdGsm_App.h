#ifndef OTDGSMAPP_H
#define OTDGSMAPP_H

#include "r_cg_macrodriver.h"
#define RX_BUFFER_LENGTH 256

typedef enum
{
	Gsm_Busy =0,
	Gsm_Ok,
	Gsm_Nok,
	Gsm_RxTimeOut
}OtdGsmApp_Status_ten;
//GSM Main State
typedef enum
{
	Gsm_Init = 0,
	Tcp_Init,
	Tcp_Send,
	FirmwareUpdate,
	MaxMainState
}OtdGsmApp_MainState_ten;

//SubState Processing
typedef enum
{
	/**< Reinitializing the GSM module */
	GSM_AT = 0,			/**< ----GSM state 1	(Send AT for checking) 			*/
	GSM_ATI,			/**< ----GSM state 2	(Product identification)		*/
	GSM_GSN,			/**< ----GSM state 3	(Request IMEI)				*/
	GSM_STATE_CHECK_PIN,		/**< ----GSM state 4	(Check if PIN) 				*/
	//GSM_STATE_ENTER_PIN,		/**< ----GSM state 5	(Enter PIN code) 			*/
	GSM_STATE_ICCID,		/**< ----GSM state 6	(SIM Serial Number) 			*/
	//GSM_STATE_SET_SLEEP_MODE,	/**< ----GSM state 7	(Config sleep mode(Slow clock) 		*/
	//GSM_STATE_WAIT_PROVIDER,	/**< ----GSM state 8	(network provider) 			*/
	//GSM_STATE_CGREG,		/**< ----GSM state 9	(network registration status)		*/
	GSM_STATE_SIGNAL_QUALITY,	/**< ----GSM state 10 	(signal quality) 			*/
	//GSM_STATE_CGATT,		/**< ----GSM state 11 	(Attach to GPRS) 			*/
	//GSM_STATE_APN,			/**< ----GSM state 12 	(Select CSD or GPRS(1) as the Bearer	*/
	//GSM_QISTAT,			/**< ----GSM state 13 	(Query current connection status) 	*/
	//GSM_QIREGAPP,			/**< ----GSM state 14 	(Start TCP task)			*/
	//GSM_QIACT,			/**< ----GSM state 15 	(Activate GPRS context) 		*/
	//GSM_QIDEACT,			/**< ----GSM state 16 	(Deactivate GPRS context) 		*/
	//GSM_STATE_READY,		/**< ----GSM state 17 	(GSM ready) 				*/
	//GSM_FGCNT,			/**< ----GSM state 18 	(Foreground context) 			*/
	//GSM_CSGP,			/**< ----GSM state 19 	(Select CSD or GPRS(1) as the Bearer 	*/
	//GSM_SHUTDOWN,			/**< ----GSM state 20 	(For Shutting down GSM module) 		*/
	
	GSM_CNMI,
	GSM_CGQMIN,
	
	/**< Commands for sending an SMS */
	GSM_CMGF,											/**< For setting SMS text mode 								*/
	GSM_CSMP,											/**< For setting the parameter to send SMS text mode 		*/
	GSM_CMGS,											/**< For sending it to SMS mode 								*/
	GSM_CSCS,											/**< Set as GSM mode 										*/
	//GSM_CNMI,											/**< For setting status receiving management 				*/
	GSM_CMGD,
	Gsm_MaxSubState
}OtdGsmApp_SubState_ten;

typedef struct
{
	OtdGsmApp_MainState_ten Current_en; //stores the current state
	OtdGsmApp_MainState_ten Previous_en;//stores the previous state
	//add array of function pointers for processing the command frame
}OtdGsmApp_MainState_tst;

typedef struct
{
	OtdGsmApp_SubState_ten Current_en; //stores the current state
	OtdGsmApp_SubState_ten Previous_en;//stores the previous state
	volatile uint8_t CmdSendFlag;
	volatile uint8_t Trials;//tracks the number of times the frame executed
	volatile int16_t TimeOut; // tracks the time
}OtdGsmApp_SubState_tst;

void OtdGsmApp_GsmStateInit(void);
void OtdGsmApp_MainStateMachine(void);

#endif
