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

#include "can.h"
#include "enet.h"
#include <stdio.h>
#include "task.h"
#include "IntCtrl_Ip.h"
#include "FlexCAN_Ip_HwAccess.h"
#include "FlexCAN_Ip_Wrapper.h"

#define MSG_ID 20u
#define RX_MB_IDX 1U
#define TX_MB_IDX 0U

extern void CAN0_ORED_0_31_MB_IRQHandler(void);
extern void CAN1_ORED_0_31_MB_IRQHandler(void);
extern void CAN2_ORED_0_31_MB_IRQHandler(void);
extern void CAN3_ORED_0_31_MB_IRQHandler(void);
extern void CAN4_ORED_0_31_MB_IRQHandler(void);
extern void CAN5_ORED_0_31_MB_IRQHandler(void);

void eth_can_tx_worker( void * arg);
void can_rx_worker( void *arg );
void can_rxfifo_worker( void *arg );

typedef struct Flexcan_irq_mapping{
	IRQn_Type irq;
	IntCtrl_Ip_IrqHandlerType irq_handler;
} Flexcan_irq_mapping;

const Flexcan_irq_mapping irq_mapping[] = {
		{FlexCAN0_1_IRQn, CAN0_ORED_0_31_MB_IRQHandler},
		{FlexCAN1_1_IRQn, CAN1_ORED_0_31_MB_IRQHandler},
		{FlexCAN2_1_IRQn, CAN2_ORED_0_31_MB_IRQHandler},
		{FlexCAN3_1_IRQn, CAN3_ORED_0_31_MB_IRQHandler},
		{FlexCAN4_1_IRQn, CAN4_ORED_0_31_MB_IRQHandler},
		{FlexCAN5_1_IRQn, CAN5_ORED_0_31_MB_IRQHandler}
};


const Flexcan_Ip_EnhancedIdTableType CAN0_EnhanceFIFO_IdFilterTable[3] =
{
		/* Enhanced FIFO filter table element 0-2 */
		{
				.filterType = FLEXCAN_IP_ENHANCED_RX_FIFO_RANGE_ID_FILTER,  // Filter element with filter + mask scheme
				.isExtendedFrame = true,
				.id1 = 0x0, 	// EXT ID filter
				.id2 = 0x1FFFFFFF ,	// EXT ID mask
				.rtr2 = false,	// RTR filter
				.rtr1 = true,	// RTR mask
		},
		{
				.filterType = FLEXCAN_IP_ENHANCED_RX_FIFO_RANGE_ID_FILTER,
				.isExtendedFrame = false,
				.id1 = 0x0, 	// STD ID filter
				.id2 = 0x7FF ,	// STD ID mask
				.rtr2 = false,	// RTR filter
				.rtr1 = true,	// RTR mask
		},
		{
				.filterType = FLEXCAN_IP_ENHANCED_RX_FIFO_ONE_ID_FILTER,
				.isExtendedFrame = false,
				.id2 = 0x456, 	// STD ID filter
				.id1 = 0x7FF ,	// STD ID mask
				.rtr2 = false,	// RTR filter
				.rtr1 = true,	// RTR mask
		}
};


CAN can_instances[] = {
		{0, &FlexCAN_State0, &FlexCAN_Config0, LED_CAN0_PORT, LED_CAN0_PIN, NULL, NULL, NULL, NULL, {0}, {0}, {0}, false, 0, 0},
		{1, &FlexCAN_State1, &FlexCAN_Config1, LED_CAN1_PORT, LED_CAN1_PIN, NULL, NULL, NULL, NULL, {0}, {0}, {0}, false, 0, 0},
		{2, &FlexCAN_State2, &FlexCAN_Config2, LED_CAN2_PORT, LED_CAN2_PIN, NULL, NULL, NULL, NULL, {0}, {0}, {0}, false, 0, 0},
		{3, &FlexCAN_State3, &FlexCAN_Config3, LED_CAN3_PORT, LED_CAN3_PIN, NULL, NULL, NULL, NULL, {0}, {0}, {0}, false, 0, 0},
		{4, &FlexCAN_State4, &FlexCAN_Config4, LED_CAN4_PORT, LED_CAN4_PIN, NULL, NULL, NULL, NULL, {0}, {0}, {0}, true, CAN4_STB_N_PORT, CAN4_STB_N_PIN},
		{5, &FlexCAN_State5, &FlexCAN_Config5, LED_CAN5_PORT, LED_CAN5_PIN, NULL, NULL, NULL, NULL, {0}, {0}, {0}, true, CAN5_STB_N_PORT, CAN5_STB_N_PIN},
};

static inline uint32_t arm_lsb(unsigned int value)
{
	uint32_t ret;
	volatile uint32_t rvalue = value;
	__asm__ __volatile__ ("rbit %1,%0" : "=r" (rvalue) : "r" (rvalue));
	__asm__ __volatile__ ("clz %0, %1" : "=r"(ret) : "r"(rvalue));
	return ret;
}

void setupCanTJA1153(int instance, Siul2_Dio_Ip_GpioType * const stb_port,
		Siul2_Dio_Ip_PinsChannelType stb_pin)
{
	Flexcan_Ip_DataInfoType tx_info = {
			.msg_id_type = FLEXCAN_MSG_ID_STD,
			.data_length = 8u,
			.fd_enable = FALSE,
			.fd_padding = FALSE,
			.enable_brs = FALSE,
			.is_polling = FALSE,
			.is_remote = FALSE
	};
	/**
	 * The TJA1153 transceiver does not come ready to use as most CAN transceivers
	 * It enters configuration mode initially if it has not been configured before,
	 * in this initial state, it is ready for receiving additional security
	 * setups such as an ID blocklist for example.
	 *
	 * In this example, the ID being transmitted is added to the CAN0 transceiver's
	 * passlist, other configurations are left as default.
	 *
	 * Refer to the NXP's TJA1153 data sheet for further detail.
	 *
	 */
#define TJA1153_START_ID   (uint32_t)(0x555u)
#define TJA1153_CONFIG_ID  (uint32_t)(0x18DA00F1u)

	uint8 sendData[8]={0,0,0,0,0,0,0,0};


	/* Allow configuration from local host via TXD pin for CAN transceiver */
	Siul2_Dio_Ip_WritePin(stb_port, stb_pin, 0U);

	/* Auto bit rate detection initial CAN Classic frame with ID 0x555 for CAN0 */
	tx_info.is_polling = TRUE;
	tx_info.msg_id_type = FLEXCAN_MSG_ID_STD;
	tx_info.data_length=0;
	FlexCAN_Ip_SendBlocking(instance, TX_MB_IDX, &tx_info, TJA1153_START_ID, sendData, 100);

	sendData[0]= 0x10;
	sendData[1]= 0x00;
	sendData[2]= 0x50;
	sendData[3]= 0x00;
	sendData[4]= 0x07;
	sendData[5]= 0xFF;

	tx_info.msg_id_type = FLEXCAN_MSG_ID_EXT;
	tx_info.data_length = 6;
	FlexCAN_Ip_SendBlocking(instance, TX_MB_IDX, &tx_info, TJA1153_CONFIG_ID, sendData, 100);

	sendData[0]= 0x10;
	sendData[1]= 0x01;
	sendData[2]= 0x9f;
	sendData[3]= 0xff;
	sendData[4]= 0xff;
	sendData[5]= 0xff;

	tx_info.msg_id_type = FLEXCAN_MSG_ID_EXT;
	tx_info.data_length = 6;

	FlexCAN_Ip_SendBlocking(instance, TX_MB_IDX, &tx_info, TJA1153_CONFIG_ID, sendData, 100);

	sendData[0]= 0x10;
	sendData[1]= 0x02;
	sendData[2]= 0xc0;
	sendData[3]= 0x00;
	sendData[4]= 0x00;
	sendData[5]= 0x00;

	tx_info.msg_id_type = FLEXCAN_MSG_ID_EXT;
	tx_info.data_length = 6;

	FlexCAN_Ip_SendBlocking(instance, TX_MB_IDX, &tx_info, TJA1153_CONFIG_ID, sendData, 100);

	sendData[0]= 0x71;
	sendData[1]= 0x02;
	sendData[2]= 0x03;
	sendData[3]= 0x04;
	sendData[4]= 0x05;
	sendData[5]= 0x06;
	sendData[6]= 0x07;
	sendData[7]= 0x08;

	tx_info.msg_id_type = FLEXCAN_MSG_ID_EXT;
	tx_info.data_length=8;

	FlexCAN_Ip_SendBlocking(instance, TX_MB_IDX, &tx_info, TJA1153_CONFIG_ID, sendData, 100);

	vTaskDelay(pdMS_TO_TICKS(10));

	/* After the last frame, the transceiver exits configuration mode and goes to standby mode, exit
	 * to normal operation mode is done by setting the STB pin of CAN transceiver to HIGH (pin is negated) */
	Siul2_Dio_Ip_WritePin(stb_port, stb_pin, 1U);
}

void can_activity_led( void *arg )
{
	CAN* can = (CAN*)arg;
	BaseType_t operation_status;

	for( ;; )
	{
		operation_status = xSemaphoreTake(can->led_sem, portMAX_DELAY);
		configASSERT(operation_status == pdPASS);
		Siul2_Dio_Ip_ClearPins(can->led_port, (1 << can->led_pin));
		vTaskDelay(pdMS_TO_TICKS(90));
		Siul2_Dio_Ip_SetPins(can->led_port, (1 << can->led_pin));
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

CAN* can_init(uint8 inst) {

	if(inst > sizeof(can_instances) / sizeof(can_instances[0]))
		return NULL;

	/*can instances already defined above at start of file with all the configuration options*/
	CAN* can = &can_instances[inst];

	/* Enable FlexCAN IRQ */
	IntCtrl_Ip_EnableIrq(irq_mapping[inst].irq);
	IntCtrl_Ip_InstallHandler(irq_mapping[inst].irq, irq_mapping[inst].irq_handler, NULL_PTR);
	IntCtrl_Ip_SetPriority(irq_mapping[inst].irq, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

	/* Initialize and enable the FlexCAN module and see if the intialization has been successfull*/
	Flexcan_Ip_StatusType Status_Init_FlexCAN = FLEXCAN_STATUS_ERROR;
	Status_Init_FlexCAN = FlexCAN_Ip_Init(can->instance, can->state, can->config);

	if(Status_Init_FlexCAN != FLEXCAN_STATUS_SUCCESS)
	{
		return NULL;
	}

	/* FlexCAN Mailbox intialization */
	if(can->config->is_enhanced_rx_fifo_needed) {
		FlexCAN_Ip_ConfigEnhancedRxFifo_Privileged(can->instance, &CAN0_EnhanceFIFO_IdFilterTable[0]);
		FlexCAN_Ip_SetRxMaskType_Privileged(can->instance, FLEXCAN_RX_MASK_INDIVIDUAL);
		FlexCAN_Ip_SetRxIndividualMask_Privileged(can->instance, 1, 0x1FFFFFFF);
		FlexCAN_Ip_SetRxIndividualMask(can->instance, 2, 0x7FF << 18);
	} else {
		Flexcan_Ip_DataInfoType rx_info = {
				.msg_id_type = FLEXCAN_MSG_ID_EXT,
				.data_length = 8u,
				.fd_enable = TRUE,
				.fd_padding = FALSE,
				.enable_brs = TRUE,
				.is_polling = FALSE,
				.is_remote = FALSE
		};
		FlexCAN_Ip_ConfigRxMb(can->instance, RX_MB_IDX, &rx_info, 0x0);
		rx_info.msg_id_type = FLEXCAN_MSG_ID_STD;
		FlexCAN_Ip_ConfigRxMb(can->instance, RX_MB_IDX+1, &rx_info, 0x0);
		FlexCAN_Ip_SetRxMaskType_Privileged(can->instance, FLEXCAN_RX_MASK_INDIVIDUAL);
		FlexCAN_Ip_SetRxMbGlobalMask_Privileged(can->instance, 0x3fffffff);
		FlexCAN_Ip_SetRxMb14Mask_Privileged(can->instance, 0x3fffffff);
		FlexCAN_Ip_SetRxMb15Mask_Privileged(can->instance, 0x3fffffff);
		FlexCAN_Ip_SetRxFifoGlobalMask_Privileged(can->instance, 0x0);
	}

	FlexCAN_Ip_SetStartMode(can->instance);

	sprintf(can->rx_task_name, "CANRX%i", can->instance);
	sprintf(can->tx_task_name, "CANTX%i", can->instance);
	sprintf(can->led_task_name, "CANLED%i", can->instance);
	printf("CANLED%i", can->instance);

	/* Create Tasks for FlexCAN TX & RX handling */
	vSemaphoreCreateBinary(can->led_sem);
	if(can->config->is_enhanced_rx_fifo_needed) {
		xTaskCreate( can_rxfifo_worker, ( const char * const )can->rx_task_name, 1024, (void*)can, can_TASK_PRIORITY, &can->rx_task);
	} else {
		xTaskCreate( can_rx_worker, ( const char * const )can->rx_task_name, 1024, (void*)can, can_TASK_PRIORITY, &can->rx_task);
	}
	xTaskCreate( eth_can_tx_worker, can->tx_task_name, configMINIMAL_STACK_SIZE, (void*)can, can_TASK_PRIORITY, &can->tx_task);
	xTaskCreate( can_activity_led, ( const char * const )can->led_task_name, configMINIMAL_STACK_SIZE, (void*)can, can_TASK_PRIORITY-1, NULL);
	can->eth_can_queue = xQueueCreate( ETH_CAN_QUEUE_SIZE, sizeof( Flexcan_Ip_MsgBuffType ) );

	return can;
}


void can_interrupt(uint8 instance,
		Flexcan_Ip_EventType eventType,
		uint32 buffIdx,
		const struct FlexCANState *driverState) {
	(void)buffIdx;
	(void)driverState;

	CAN* can = &can_instances[instance];

	switch(eventType)
	{
	case FLEXCAN_EVENT_TX_COMPLETE:
		/* Signal TX that mailbox is available */
		xTaskNotifyIndexedFromISR( can->tx_task,
				0,
				buffIdx,
				eSetBits,
				NULL );
		break;
	case FLEXCAN_EVENT_RX_COMPLETE:
		/* Signal RX worker */
		xTaskNotifyIndexedFromISR( can->rx_task,
				0,
				buffIdx,
				eSetBits,
				NULL );
		break;
	case FLEXCAN_EVENT_ENHANCED_RXFIFO_COMPLETE:
		/* Signal RX FIFO worker */
		xTaskNotifyIndexedFromISR( can->rx_task,
				0,
				eventType,
				eSetBits,
				NULL );

		break;
	case FLEXCAN_EVENT_ENHANCED_RXFIFO_WATERMARK:
		break;
	default:
		break;
	}
}

void can_rx_worker( void *arg )
{
	CAN* can = (CAN*)arg;

	Flexcan_Ip_MsgBuffType rxData[2];
	uint32_t buffIdx;
	uint32_t mb_index;
	BaseType_t operation_status;

	/* CAN4 & CAN5 have a TJA1153 transceiver that needs to be initialized */
	if(can->isTJA1153) {
		setupCanTJA1153(can->instance, can->stb_port, can->stb_pin);
	}

	/* Initiate receive */
	FlexCAN_Ip_Receive(can->instance, RX_MB_IDX, &rxData[0], FALSE);
	FlexCAN_Ip_Receive(can->instance, RX_MB_IDX+1, &rxData[1], FALSE);

	for( ;; )
	{
		/* Wait for receive */
		operation_status = xTaskNotifyWaitIndexed( 0,                  /* Wait for 0th Notificaition */
				0x00,               /* Don't clear any bits on entry. */
				0xFFFFFFFF,          /* Clear all bits on exit. */
				&buffIdx, /* Receives the notification value. */
				portMAX_DELAY );    /* Block indefinitely. */


		while ((mb_index = arm_lsb(buffIdx)) != 32 && operation_status == pdPASS) {

			/* Signal activity led */
			xSemaphoreGive(can->led_sem);

			/* Send CAN frame over Ehernet using IEEE1772 ACF-CAN */
			enet_ieee1722_acf_can_send(can->instance, &rxData[mb_index]);

			/* Re-initiate mailbox */
			FlexCAN_Ip_Receive(can->instance, RX_MB_IDX + mb_index, &rxData[mb_index], FALSE);

			/* Unmask mailbox */
			buffIdx &= ~(1 << mb_index);
		}

	}
}

void can_rxfifo_worker( void *arg )
{
	CAN* can = (CAN*)arg;

	Flexcan_Ip_MsgBuffType rxFifoData;
	uint32_t ulInterruptStatus;

	for( ;; )
	{
		/* Initiate receive */
		FlexCAN_Ip_RxFifo(can->instance, &rxFifoData);

		/* Wait for receive */
		xTaskNotifyWaitIndexed( 0,                  /* Wait for 0th Notificaition */
				0x00,               /* Don't clear any bits on entry. */
				0xFFFFFFFF,          /* Clear all bits on exit. */
				&ulInterruptStatus, /* Receives the notification value. */
				portMAX_DELAY );    /* Block indefinitely. */

		/* Signal activity led */
		xSemaphoreGive(can->led_sem);

		/* Send CAN frame over Ehernet using IEEE1772 ACF-CAN */
		enet_ieee1722_acf_can_send(can->instance, &rxFifoData);
	}
}

void eth_can_tx_worker( void * arg) {
	CAN* can = (CAN*)arg;

	uint32_t ulInterruptStatus;
	Flexcan_Ip_MsgBuffType txData;
	Flexcan_Ip_DataInfoType tx_info;

	tx_info.is_polling = FALSE;

	for( ;; )
	{
		/* Wait for new IEEE1722 ACF-CAN packet from Ethernet */
		if(xQueueReceive(can->eth_can_queue, &txData, portMAX_DELAY) == pdTRUE) {

			/* Set TX information */
			tx_info.data_length = txData.dataLen;
			tx_info.enable_brs = ((txData.cs & FLEXCAN_IP_MB_BRS_MASK) != 0);
			tx_info.msg_id_type = ((txData.cs & FLEXCAN_IP_CS_IDE_MASK) != 0);
			tx_info.is_remote = ((txData.cs & FLEXCAN_IP_CS_RTR_MASK) != 0);
			tx_info.fd_enable = ((txData.cs & FLEXCAN_IP_MB_EDL_MASK) != 0);

			while(FlexCAN_Ip_Send(can->instance, TX_MB_IDX, &tx_info, txData.msgId, (uint8 *)&txData.data)
					!= FLEXCAN_STATUS_SUCCESS) {
				/* TX is full, wait for TX to unblock */
				xTaskNotifyWaitIndexed( 0,
						0x00,               /* Don't clear any bits on entry. */
						0xFFFFFFFF,          /* Clear all bits on exit. */
						&ulInterruptStatus, /* Receives the notification value. */
						portMAX_DELAY );    /* Block indefinitely. */
			}

			/* Signal activity led */
			xSemaphoreGive(can->led_sem);
		}
	}
}

