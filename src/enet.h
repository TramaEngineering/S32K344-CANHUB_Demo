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

#ifndef ENET_H
#define ENET_H

#ifdef __cplusplus
extern "C"{
#endif


#include "FreeRTOS.h"
#include "semphr.h"
#include "Gmac_Ip.h"
#include "FlexCAN_Ip.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"


/*==================================================================================================
 *                                        INCLUDE FILES
 * 1) system and project includes
 * 2) needed interfaces from external units
 * 3) internal and external interfaces from this unit
==================================================================================================*/

/*==================================================================================================
 *                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/


/*==================================================================================================
 *                                       LOCAL MACROS
==================================================================================================*/


/*==================================================================================================
 *                                      LOCAL CONSTANTS
==================================================================================================*/

#define eth_TASK_PRIORITY                ( tskIDLE_PRIORITY + 2 )

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
void eth_activity_led( void *arg );

void eth_activity_led_send( void *arg );

Gmac_Ip_StatusType enet_init(QueueHandle_t* tx_descr_queue);

void enet_ieee1722_acf_can_send(uint8 instance, Flexcan_Ip_MsgBuffType *can_frame);

void enet_start_rx(QueueHandle_t* eth_can_queues, uint32 count);

void enet_start_tx(void);

void send_main_can_frame_on_eth(Flexcan_Ip_MsgBuffType *can_frame);

void send_eth_frame(Gmac_Ip_BufferType* eth_message);

void init_annouce(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */
