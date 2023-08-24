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
/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#define DISABLE_INTERRUPT	1U /* disable INTST1 interrupt */
#define ENABLE_INTERRUPT	0U  /* Enable INTSR1 interrupt */

#define TX_INTERRUPT(STATE)     STMK1 = STATE    
#define RX_INTERRUPT(STATE)	SRMK1 = STATE   

volatile uint8_t rx_data;

extern volatile uint8_t * gp_uart1_rx_address;        /* uart1 receive buffer address */
extern volatile uint16_t  g_uart1_rx_count;            /* uart1 receive data number */

extern uint8_t result;
extern Circular_Buffer *debug_rx_ptr;
/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/

/** @brief Uart initialisation
 *  @param void
 *  @return Void.
 */
void OtdUart_Init(void)
{
	R_UART1_Start();
	OtdUart_Recieve(&result,1);	
}

/** @brief Uart Disable
 *  @param void
 *  @return Void.
 */
void OtdUart_Disable(void)
{
	R_UART1_Stop();
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
	buff_store_char(result,debug_rx_ptr);
	OtdUart_Recieve(&result,1);
}