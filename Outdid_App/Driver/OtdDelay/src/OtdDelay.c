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
* File Name    : OtdDelay.c
* Device(s)    : Outdid version 1.0
* Tool-Chain   : CCRL
* Description  : This file implements device driver for Delay module.
* Creation Date: 15-08-2023
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "OtdDelay.h"
#include "r_cg_timer.h"
#include "OtdGsm_App.h"
#include "OtdGsm_TcpApp.h"
/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
extern OtdGsmApp_SubState_tst GsmSubState_st;
extern OtdGsmTcpApp_SubState_tst TcpSubState_st;

volatile uint16_t time_out = 0;
volatile static uint16_t delay_cnt = 0;
/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/

/** @brief Start timer TAU0 Channel 0 @1ms
 *  @param void
 *  @return Void.
 */
void OtdDelay_Start(void)
{
	R_TAU0_Channel0_Start();//Start timer to trigger for every 1ms	
}

/** @brief Uart Disable
 *  @param void
 *  @return Void.
 */
void OtdDelay_Stop(void)
{
	R_TAU0_Channel0_Stop();
}
void OtdDelay_ms(uint16_t delay_ms)
{
	delay_cnt = 0;
	while(delay_cnt < delay_ms){};
}
/** @brief Uart Disable
 *  @param void
 *  @return Void.
 */
void OtdDelay_Callback1ms(void)
{
	delay_cnt++;
	//time_out++;
	GsmSubState_st.TimeOut--;
	TcpSubState_st.TcpTimeOut--;
}

uint16_t OtdDelay_GetTicks(void)
{
	return time_out;
}