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
 *   This file contains sample code only. It is not part of the production code deliverables.
 */

#ifndef CAN_H
#define CAN_H

#ifdef __cplusplus
extern "C"{
#endif

#include "FreeRTOS.h"
#include "semphr.h"

#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "FlexCAN_Ip.h"
#include "FlexCAN_Ip_Sa_BOARD_InitPeripherals_PBcfg.h"

/*==================================================================================================
 *                                        INCLUDE FILES
 * 1) system and project includes
 * 2) needed interfaces from external units
 * 3) internal and external interfaces from this unit
==================================================================================================*/

/*==================================================================================================
 *                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/


typedef struct CAN{
	uint8 instance;
	Flexcan_Ip_StateType* state;
	const Flexcan_Ip_ConfigType* config;
	Siul2_Dio_Ip_GpioType * const led_port;
	Siul2_Dio_Ip_PinsChannelType led_pin;
	TaskHandle_t rx_task;
	TaskHandle_t tx_task;
	SemaphoreHandle_t led_sem;
	QueueHandle_t eth_can_queue;
	char rx_task_name[9];
	char tx_task_name[9];
	char led_task_name[9];
	bool isTJA1153;
	Siul2_Dio_Ip_GpioType * const stb_port;
	Siul2_Dio_Ip_PinsChannelType stb_pin;
} CAN;

/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/


/*==================================================================================================
 *                                      LOCAL CONSTANTS
==================================================================================================*/

#define can_TASK_PRIORITY                ( tskIDLE_PRIORITY + 2 )
#define ETH_CAN_QUEUE_SIZE               32

/*==================================================================================================
 *                                      LOCAL VARIABLES
==================================================================================================*/


/*==================================================================================================
 *                                      GLOBAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
 *                                      GLOBAL VARIABLES
==================================================================================================*/



/*==================================================================================================
 *                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/


/*==================================================================================================
 *                                       LOCAL FUNCTIONS
==================================================================================================*/


/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
==================================================================================================*/

CAN* can_init(uint8 inst);


#ifdef __cplusplus
}
#endif

#endif

/** @} */
