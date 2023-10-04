/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2011, 2021 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_serial_user.c
* Version      : CodeGenerator for RL78/G14 V2.05.06.02 [08 Nov 2021]
* Device(s)    : R5F104ML
* Tool-Chain   : CCRL
* Description  : This file implements device driver for Serial module.
* Creation Date: 15-08-2023
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "OtdUART.h"
#include "OtdCircularBuffer_App.h"
#include "OtdGsm_App.h"
/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#define GSM_UART_INIT()		R_UART1_Start()
#define DEBUG_UART_INIT()	R_UART3_Start()

#define GSM_UART_STOP()		R_UART1_Stop()
#define DEBUG_UART_STOP()	R_UART3_Stop()

extern uint8_t debug_tx_pending;
//TODO: -check
volatile uint8_t GsmRxData = 0; 
extern volatile uint16_t RxIndex;
extern volatile char Gsm_RxBuffer[RX_BUFFER_LENGTH];
/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/

/** @brief Uart initialisation
 *  @param void
 *  @return Void.
 */
void OtdUart_Init(void)
{
	GSM_UART_INIT();
	DEBUG_UART_INIT();
	OtdUart_Recieve((uint8_t * __near)&GsmRxData,1);	
}

/** @brief Uart Disable
 *  @param void
 *  @return Void.
 */
void OtdUart_Disable(void)
{
	GSM_UART_STOP();
	DEBUG_UART_STOP();
}

/** @brief Uart Disable
 *  @param void
 *  @return Void.
 */
void OtdUart_CallbackSend(void)
{

}
/** @brief Uart Disable
 *  @param void
 *  @return Void.
 */
void OtdUart_CallbackRecieve(void)
{
	Gsm_RxBuffer[RxIndex++] = GsmRxData;
	OtdUart_Recieve((uint8_t * __near)&GsmRxData,1);
}

Otd_Uart_Status OtdUart_Send(uint8_t *buf, uint16_t len , uint8_t uart)
{
	Otd_Uart_Status uart_status = OTD_UART_STATUS_PASS;
	
	if(uart == GSM_UART)
	{
		if(R_UART1_Send(buf,len) == MD_OK)
		{
			uart_status = OTD_UART_STATUS_PASS;
		}
		else
		{
			uart_status = OTD_UART_STATUS_FAIL;
		}
	}
	if(uart == DEBUG_UART)
	{
		if(R_UART3_Send(buf,len) == MD_OK)
		{
			uart_status = OTD_UART_STATUS_PASS;
		}
		else
		{
			uart_status = OTD_UART_STATUS_FAIL;
		}		
	}
	

	return uart_status;
}
void OtdUart_DebugSend(const char *s)
{
	while(*s != '\0')
	{
		OtdUart_Send((uint8_t *__near)s++, 1,DEBUG_UART);
		while(!debug_tx_pending);
		debug_tx_pending = 0;
	}
}