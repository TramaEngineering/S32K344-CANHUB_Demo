/*
*   (c) Copyright 2022 NXP
*
*   NXP Confidential. This software is owned or controlled by NXP and may only be used strictly
*   in accordance with the applicable license terms.  By expressly accepting
*   such terms or by downloading, installing, activating and/or otherwise using
*   the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms.  If you do not agree to
*   be bound by the applicable license terms, then you may not retain,
*   install, activate or otherwise use the software.
*
*   THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
*   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
*   IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
*   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
*   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
*   THE POSSIBILITY OF SUCH DAMAGE.
*
*   This file contains sample code only. It is not part of the production code deliverables.
*/

#ifndef BSP_BSP_H_
#define BSP_BSP_H_
/**
*   @file       bsp.h
*
*   @addtogroup bsp Bsp_Module
*
*   @{
*/
#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                         INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
/* Basic RTD modules includes */
#include "Mcal.h"
#include <stdarg.h>
#include <stdio.h>

#include "Clock_Ip.h"
#include "Pit_Ip.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "Siul2_Icu_Ip.h"
#include "Siul2_Icu_Ip_Irq.h"
#include "IntCtrl_Ip.h"
#include "Lpuart_Uart_Ip.h"
#include "Lpspi_Ip.h"
#include "Siul2_Icu_Ip.h"
#include "IntCtrl_Ip.h"
#include "FlexCan_Ip.h"
#include "stdbool.h"

#include "OsIf.h"
#include "FreeRTOS.h"
#include "task.h"

#include "can.h"
#include "enet.h"
#include "fs26.h"
#include "uart.h"
#include "Pit_Ip.h"
#include "IntCtrl_Ip.h"

/* MCU registers definitions includes */
#ifdef S32K344
#include "S32K344_SCB.h"
#include "S32K344_MSCM.h"
#include "S32K344_MC_RGM.h"
#endif
#ifdef S32K324
#include "S32K324_SCB.h"
#include "S32K324_MSCM.h"
#include "S32K324_CMU_FM.h"
#include "S32K324_MC_RGM.h"
#endif
/* Interfaces from external units */
#include "../uart_print/retarget.h"

/*==================================================================================================
*                                           CONSTANT-LIKE DEFINES
==================================================================================================*/
#define		ADC0_INST		0
#define		ADC1_INST		1
#define		ADC2_INST		2

#define		EMIOS0_INST		0
#define		EMIOS1_INST		1
#define		EMIOS2_INST		2

#define 	EMIOS1_CH0		0
#define		EMIOS2_CH8		8
#define		EMIOS2_CH11		11
#define		EMIOS2_CH17		17
#define		EMIOS2_CH18		18

#define 	QSPI_INSTANCE_0		0

#define 	MMA8452Q_IS_WELDED	0

/*==================================================================================================
*                                       FUNCTION-LIKE DEFINES(MACROS)
==================================================================================================*/
//#define LOGI(formatstring, ...) printf(formatstring, ##__VA_ARGS__)

/* just used for test in case ENET is not working
 * call it in a timer ISR to generate the clock
 * for example, call it in a PIT ISR with 1ms period to generate 500Hz clock
 */
#define ENET_PPS_CLK_GEN() Siul2_Dio_Ip_TogglePins(GPIO_EMAC_PPS0_PORT,(1<<GPIO_EMAC_PPS0_PIN))

/*==================================================================================================
*                                             ENUMS
==================================================================================================*/

/*==================================================================================================
*                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
*                                 GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/
extern uint32_t Task_Flag_1mS;
extern uint32_t Task_Flag_2mS;
extern uint32_t Task_Flag_10mS;
extern uint32_t Task_Flag_100mS;
extern uint32_t Task_Flag_200mS;
extern uint32_t Task_Flag_1000mS;
extern uint32_t Task_Flag_Cnt;
/*==================================================================================================
*                                     FUNCTION PROTOTYPES
==================================================================================================*/
/**
 * @brief          Minisecond time delay using CPU systick timer, which is a 24bit count-down timer
 * @details        Poll the systick timer to delay a period of time.
 * @param[in]      nMsec, count of mini-second to delay
 * @return         none.
 */
void OsIf_Delay_Ms(uint32_t nMsec);

/**
 * @brief          Microsecond time delay using CPU systick timer, which is a 24bit count-down timer
 * @details        Poll the systick timer to delay a period of time.
 * @param[in]      nUsec, count of micro-second to delay
 * @return         none.
 */
void OsIf_Delay_Us(uint32_t nUsec);

/**
 * @brief          QSPI Flash test function
 * @details        Initialize QSPI instance, erase flash, read flash, read flash device ID.
 * @param[in]      none.
 * @return         E_OK or E_NOT_OK
 */
Std_ReturnType QSPI_Flash_InitTest(void);

/**
 * @brief          board initialization
 * @details        Initialization for MCU clocks, pins, peripherals and other devices on the board.
 * @param[in]      none.
 * @return         0 means OK.
 */
uint32_t Bsp_Init(void);

void gPTPTimer(uint8 channel);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* BSP_BSP_H_ */
