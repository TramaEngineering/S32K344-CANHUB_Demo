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

#include "enet.h"
#include <string.h>
#include <stdlib.h>
#include "msg_converter.h"
#include "IntCtrl_Ip.h"
#include "Gmac_Ip_Hw_Access.h"
#include "avtp_defs.h"
#include "uart.h"

SemaphoreHandle_t tx_queue_handle;
QueueHandle_t tx_descr_queue;
QueueHandle_t tx_descr_queue_send;
SemaphoreHandle_t tx_send_mutex = NULL;
SemaphoreHandle_t eth_blink;
TaskHandle_t led_blink;
SemaphoreHandle_t eth_blink_send;
TaskHandle_t led_blink_send;

extern void GMAC0_Common_IRQHandler(void);
extern void GMAC0_CH_TX_IRQHandler(void);
extern void GMAC0_CH_RX_IRQHandler(void);

//void etherswap(uint16* ethertype) {
//	uint16 mask = 0XFF00;
//	uint16 high = *ethertype & mask;
//	*ethertype = *ethertype << 8;
//    *ethertype = *ethertype | high >> 8;
//}

QueueHandle_t* eth_can_queue;
int can_count;
TaskHandle_t rx_task;
TaskHandle_t tx_task;
int phyad = 0;

extern uint32_t __UTEST_UID[2];

#define TIMEOUT_MS					(200U)
#define TJA1103_DEV_ID 				(0x001BU)
#define RGMII_SUPPORTED 			(0U)

/* MMDs */
#define PHYAD                       18
#define MMD1						(1U)
#define MMD30						(30U)

#define PMA_STATUS1                 (0x0001U)
#define BT1_PMA_CONTROL_REG_ADR		(0x0834U)
#define DEV_CONTR_REG_ADR			(0x0040U)
#define PHY_CONTR_REG_ADR			(0x8100U)

#define PMA_STATUS_LINK_STATUS		(1 << 2)

#define BT1_PMAPMD_CONFIG_EN		(1 << 15)
#define BT1_PMAPMD_MASTER		    (1 << 14)

#define DEV_GLOBAL_CONF_ENA_FLAG		(0x4000U)
#define DEV_SUPER_CONF_ENA_FLAG			(0x2000U)
#define PHY_CONFIG_EN_FLAG 				(0x4000U)

uint8 Txbuff2[16] = "Hello from board";

const Flexcan_Ip_MsgBuffType CanAvtp = {
		.cs = 0x0,
		.msgId = 0x10,
		.data = "AVTP",
		.dataLen = 4,
		.id_hit = 0,
		.time_stamp = 0
};

const Flexcan_Ip_MsgBuffType CanMdns = {
		.cs = 0x0,
		.msgId = 0x11,
		.data = "MDNS",
		.dataLen = 4,
		.id_hit = 0,
		.time_stamp = 0
};

const Flexcan_Ip_MsgBuffType CanArp = {
		.cs = 0x0,
		.msgId = 0x12,
		.data = "ARP",
		.dataLen = 3,
		.id_hit = 0,
		.time_stamp = 0
};

void eth_activity_led( void ){
	BaseType_t operation_status;

	for(;;){
		operation_status = xSemaphoreTake(eth_blink, portMAX_DELAY);
		configASSERT(operation_status == pdTRUE);

		Siul2_Dio_Ip_SetPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
		Siul2_Dio_Ip_ClearPins(LED_RED_PORT, (1 << LED_RED_PIN));
		Siul2_Dio_Ip_ClearPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));

		vTaskDelay(pdMS_TO_TICKS(90));

		Siul2_Dio_Ip_ClearPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
		Siul2_Dio_Ip_SetPins(LED_RED_PORT, (1 << LED_RED_PIN));
		Siul2_Dio_Ip_SetPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
	}
}

void eth_activity_led_send( void ){
	BaseType_t operation_status;

	for(;;){
		operation_status = xSemaphoreTake(eth_blink_send, portMAX_DELAY);
		configASSERT(operation_status == pdTRUE);

		/*set another task to do the uart shell write and readdsdsdsdsdsd*/
		Lpuart_Uart_Ip_AsyncSend(LPUART_UART_IP_INSTANCE_USING_2, Txbuff2, 16);

		Siul2_Dio_Ip_ClearPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
		Siul2_Dio_Ip_SetPins(LED_RED_PORT, (1 << LED_RED_PIN));
		Siul2_Dio_Ip_ClearPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));

		vTaskDelay(pdMS_TO_TICKS(90));

		Siul2_Dio_Ip_ClearPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
		Siul2_Dio_Ip_SetPins(LED_RED_PORT, (1 << LED_RED_PIN));
		Siul2_Dio_Ip_SetPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
	}
}

uint8_t TJA1103_find_addr(void)
{
	for(uint8_t id = 1; id < 32; id++)
	{
		uint16_t value;
		/* INST, ADDR, MMD, REGADDR, VAL, TIMEOUT */
		Gmac_Ip_MDIOReadMMD(0, id, 1, 0x0002, &value, 200);
		if (value == TJA1103_DEV_ID)
			return id;
	}
	return 0;
}

void tja1103_config_enable(void)
{
	phyad = TJA1103_find_addr();

	Gmac_Ip_MDIOWriteMMD(0, phyad, MMD1, DEV_CONTR_REG_ADR,
			DEV_GLOBAL_CONF_ENA_FLAG | DEV_SUPER_CONF_ENA_FLAG, 100);

	Gmac_Ip_MDIOWriteMMD(0, phyad, MMD1, PHY_CONTR_REG_ADR,
			PHY_CONFIG_EN_FLAG, 100);
}

void tja1103_wait_for_link(void) {
	uint16_t regvalue;

	//pma is a sub group of MDIO registers
	Gmac_Ip_MDIOReadMMD(0, phyad, MMD1, PMA_STATUS1, &regvalue, TIMEOUT_MS);//receive link status on pma status1

	/* Use chip UID for a random seed */
	srand(__UTEST_UID[0]);

	while((regvalue & PMA_STATUS_LINK_STATUS) == 0) {
		int rval = (rand() / (RAND_MAX / 1000)) + 500; /* Random retries between 500 and 1500 ms */

		Gmac_Ip_MDIOReadMMD(0, phyad, MMD1, BT1_PMA_CONTROL_REG_ADR, &regvalue, TIMEOUT_MS);//base_t1_PMA_CONTROL register 1<<14 is master
		if(regvalue & BT1_PMAPMD_MASTER) {
			Gmac_Ip_MDIOWriteMMD(0, phyad, MMD1, BT1_PMA_CONTROL_REG_ADR,
					BT1_PMAPMD_CONFIG_EN, 100);
		} else {
			Gmac_Ip_MDIOWriteMMD(0, phyad, MMD1, BT1_PMA_CONTROL_REG_ADR,
					BT1_PMAPMD_CONFIG_EN | BT1_PMAPMD_MASTER, 100);
		}

		Gmac_Ip_MDIOReadMMD(0, phyad, MMD1, PMA_STATUS1, &regvalue, TIMEOUT_MS);

		/* Wait for completion */
		vTaskDelay(pdMS_TO_TICKS(rval));

		Gmac_Ip_MDIOReadMMD(0, phyad, MMD1, PMA_STATUS1, &regvalue, TIMEOUT_MS);
	}
}


Gmac_Ip_StatusType enet_init(QueueHandle_t* tx_descr_queue_m) {

	//RMII mode
	IP_DCM_GPR->DCMRWF1 = (IP_DCM_GPR->DCMRWF1 & ~DCM_GPR_DCMRWF1_RMII_MII_SEL_MASK) | DCM_GPR_DCMRWF1_RMII_MII_SEL(1U);

	/* Initialize and enable the GMAC module */
	Gmac_Ip_StatusType Status_Init_Gmac = GMAC_STATUS_ERROR;
	Status_Init_Gmac = Gmac_Ip_Init(INST_GMAC_0, &Gmac_0_ConfigPB_BOARD_INITPERIPHERALS);

	if(Status_Init_Gmac != GMAC_STATUS_SUCCESS)
	{
		return Status_Init_Gmac;
	}

	IntCtrl_Ip_EnableIrq(EMAC_0_IRQn);
	IntCtrl_Ip_InstallHandler(EMAC_0_IRQn, GMAC0_Common_IRQHandler, NULL_PTR);
	IntCtrl_Ip_SetPriority(EMAC_0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	IntCtrl_Ip_EnableIrq(EMAC_1_IRQn);
	IntCtrl_Ip_InstallHandler(EMAC_1_IRQn, GMAC0_CH_TX_IRQHandler, NULL_PTR);
	IntCtrl_Ip_SetPriority(EMAC_1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

	/*Create a binary semaphore to do trigger the led blinking on eth working on receive*/
	vSemaphoreCreateBinary(eth_blink);
	xTaskCreate(eth_activity_led, "ETH_LED_RECEIVE", configMINIMAL_STACK_SIZE, led_blink ,eth_TASK_PRIORITY-1, NULL);

	/*Create a binary semaphore to do trigger the led blinking on eth working on send*/
	vSemaphoreCreateBinary(eth_blink_send);
	xTaskCreate(eth_activity_led_send, "ETH_LED_SEND", configMINIMAL_STACK_SIZE, led_blink_send ,eth_TASK_PRIORITY-1, NULL);

	/* Binary semaphore to block send when TX queue is full. */
	vSemaphoreCreateBinary(tx_queue_handle);

	/* Mutex to lock concurrent use for send function. */
	tx_send_mutex = xSemaphoreCreateMutex();

	/* Create a queue TX DMA descriptor to clear them in ISR. */
	tx_descr_queue = xQueueCreate( GMAC_0_MAX_TXBUFF_SUPPORTED,
			sizeof( Gmac_Ip_BufferType ) );

	/*Create a queue TX to be filled with new message to be sent*/
	tx_descr_queue_send = xQueueCreate(GMAC_0_MAX_TXBUFF_SUPPORTED, sizeof(Flexcan_Ip_MsgBuffType));
	*tx_descr_queue_m = tx_descr_queue_send;

	return Status_Init_Gmac;
}



void eth_tx_worker( void * arg) {
	//volatile Gmac_Ip_StatusType Status;
	Flexcan_Ip_MsgBuffType can_message;
	//Gmac_Ip_BufferType TxBuffer = {0};
	//Gmac_Ip_TxOptionsType TxOptions = {FALSE, GMAC_CRC_AND_PAD_INSERTION, GMAC_CHECKSUM_INSERTION_DISABLE};
	//uint32_t ulInterruptStatus;
	//int bus_id;
	(void)arg;

	tja1103_config_enable();
	tja1103_wait_for_link();

	for( ;; )
	{
		/* Wait for new IEEE1722 ACF-CAN packet from Ethernet */
		if(xQueueReceive(tx_descr_queue_send, &can_message, portMAX_DELAY) == pdTRUE) {
//			xSemaphoreTake( tx_send_mutex, portMAX_DELAY );
//
//			/*Ask for a descriptor to load the message to be sent*/
//			TxBuffer.Length = 1536U;
//			while((GMAC_STATUS_SUCCESS != Gmac_Ip_GetTxBuff(INST_GMAC_0, 0U, &TxBuffer, NULL_PTR)) || (TxBuffer.Length < 1536U)){
//				xSemaphoreTake(tx_queue_handle, portMAX_DELAY);
//			}
//
//			while (GMAC_STATUS_TX_QUEUE_FULL == Gmac_Ip_SendFrame(INST_GMAC_0, 0U, &TxBuffer, &TxOptions))
//				{
//					xSemaphoreTake(tx_queue_handle, portMAX_DELAY);
//				}
//
//			if(xQueueReceive(tx_descr_queue, ( void * ) &TxBuffer,( TickType_t ) 10 ) != pdTRUE){
//				/*Error the message has not been correctly sent*/
//			}
//
//			/* Signal activity led */
//			xSemaphoreGive( tx_send_mutex );
			//Flexcan_Ip_MsgBuffType* message = (Flexcan_Ip_MsgBuffType*) TxBuffer.Data;
			send_main_can_frame_on_eth(&can_message);
		}
	}
}


void eth_rx_worker(void *arg) {
	volatile Gmac_Ip_StatusType Status;
	Gmac_Ip_BufferType RxBuffer = {0};
	Gmac_Ip_RxInfoType RxInfo  = {0};
	Flexcan_Ip_MsgBuffType txData;
	uint32_t ulInterruptStatus;
	int bus_id;
	(void)arg;

	/* Make sure that TJA1103 has an ethernet link */
	tja1103_config_enable();
	tja1103_wait_for_link();

	for( ;; )
	{
		Status = Gmac_Ip_ReadFrame(INST_GMAC_0, 0U, &RxBuffer, &RxInfo);

		/* If no packet, Wait for the frame to be received */
		while (Status == GMAC_STATUS_RX_QUEUE_EMPTY) {
			xTaskNotifyWaitIndexed( 0,
					0x00,               /* Don't clear any bits on entry. */
					0xFFFFFFFF,          /* Clear all bits on exit. */
					&ulInterruptStatus, /* Receives the notification value. */
					portMAX_DELAY );    /* Block indefinitely. */
			Status = Gmac_Ip_ReadFrame(INST_GMAC_0, 0U, &RxBuffer, &RxInfo);
		}

		xSemaphoreGive(eth_blink);
		const struct ethernet_frame* ether_frame = (struct ethernet_frame*)RxBuffer.Data;

		Gmac_Ip_ProvideRxBuff(INST_GMAC_0, 0U, &RxBuffer);

		bus_id = convert_ethernet_to_can(&txData, (struct ethernet_frame*)RxBuffer.Data);
		bus_id = 0U;

		Flexcan_Ip_MsgBuffType message;
		message = CanAvtp;
		ether_frame = (struct ethernet_frame*)RxBuffer.Data;
		/*i have to swap the ethertype for a correct read*/
		//etherswap(&ether_frame->ether_type);
		switch(ether_frame->ether_type){
			case ETHERNET_ETHERTYPE_AVTP_BE:
				message = CanAvtp;
			break;
			case ETHERNET_ETHERTYPE_MDNS:
				message = CanMdns;
			break;
			case ETHERNET_ETHERTYPE_ARP:
				message = CanArp;
			break;
		}

		//sprintf("message: %s", (char*)message.data);

		if(bus_id >= 0 && bus_id < can_count) {
			if( xQueueSend( eth_can_queue[bus_id],
					( void * ) &message,
					( TickType_t ) 1000000 != pdPASS ))
			{
				/* Failed queue CAN frame drop packet */
			}

		}


	}
}

void enet_start_rx(QueueHandle_t* eth_can_queues, uint32 count) {
	eth_can_queue = eth_can_queues;
	can_count = count;

	/* Create ethernet RX worker task */
	xTaskCreate( eth_rx_worker, "ETH_RX", 512, NULL, eth_TASK_PRIORITY, &rx_task);

	/* Enable RX IRQ to receive incoming packets */
	IntCtrl_Ip_InstallHandler(EMAC_2_IRQn, GMAC0_CH_RX_IRQHandler, NULL_PTR);
	IntCtrl_Ip_SetPriority(EMAC_2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	IntCtrl_Ip_EnableIrq(EMAC_2_IRQn);
}

void enet_start_tx(void) {
	/*eth_can_queue = eth_can_queues;
	can_count = count;*/

	/* Create ethernet TX worker task */
	xTaskCreate( eth_tx_worker, "ETH_TX", 512, NULL, eth_TASK_PRIORITY, &tx_task);

	/* Enable RX IRQ to receive incoming packets */

	IntCtrl_Ip_InstallHandler(EMAC_2_IRQn, GMAC0_CH_RX_IRQHandler, NULL_PTR);
	IntCtrl_Ip_SetPriority(EMAC_2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	IntCtrl_Ip_EnableIrq(EMAC_2_IRQn);
}

void enet_rx_interrupt(uint32 Instance, uint32 Channel) {
	(void)Instance;
	(void)Channel;

	/* Signal RX worker */
	/*handler on the peripheral that triggers when a a message is detected*/
	xTaskNotifyIndexedFromISR( rx_task,
			0,
			0,
			eSetBits,
			NULL );
}

void enet_tx_interrupt(uint32 Instance, uint32 Channel) {
	Gmac_Ip_TxInfoType TxInfo  = {0};
	Gmac_Ip_BufferType TxBuffer = {0};

	UBaseType_t queue_count = uxQueueMessagesWaitingFromISR(tx_descr_queue);
	for(UBaseType_t i = 0; i < queue_count; i++) {
		if(xQueueReceiveFromISR(tx_descr_queue, &TxBuffer, NULL) == pdTRUE) {
			/* Attempt to free TX descriptor */
			if(Gmac_Ip_GetTransmitStatus(Instance, Channel, &TxBuffer, &TxInfo) == GMAC_STATUS_BUSY) {
				/* Still busy, send back to queue */
				xQueueSendToBackFromISR(tx_descr_queue, &TxBuffer, NULL);
			}
		}
	}

	/* Signal TX queue handle */
	xSemaphoreGiveFromISR(tx_queue_handle, NULL);
}

void send_main_can_frame_on_eth(Flexcan_Ip_MsgBuffType *can_frame){
	xSemaphoreTake( tx_send_mutex, portMAX_DELAY );

		Gmac_Ip_BufferType TxBuffer = {0};
		Gmac_Ip_TxOptionsType TxOptions = {FALSE, GMAC_CRC_AND_PAD_INSERTION, GMAC_CHECKSUM_INSERTION_DISABLE};
		/* Setup the frame with the Mac address and size */
		uint8 MacAddr[6U] = {0U};

		Gmac_Ip_GetMacAddr(INST_GMAC_0, MacAddr);

		/* Request a buffer of at least 64 bytes */
		TxBuffer.Length = 128U;
		while ((GMAC_STATUS_SUCCESS != Gmac_Ip_GetTxBuff(INST_GMAC_0, 0U, &TxBuffer, NULL_PTR)) || (TxBuffer.Length < 128U))
		{
			xSemaphoreTake(tx_queue_handle, portMAX_DELAY);
		}

		struct ethernet_frame* can_eth_frame = (struct ethernet_frame*)TxBuffer.Data;

		memset(can_eth_frame->dst_macaddr, 0xFF, ETH_ALEN); /* Broadcast */
		memcpy(can_eth_frame->src_macaddr, MacAddr, ETH_ALEN); /* Our own MAC addr */

		TxBuffer.Length = convert_can_to_avtp_no_can_inst((struct ethernet_frame*)TxBuffer.Data, can_frame);

		/* Send the ETH frame */
		/*true function that sends data to the transceiver*/
		while (GMAC_STATUS_TX_QUEUE_FULL == Gmac_Ip_SendFrame(INST_GMAC_0, 0U, &TxBuffer, &TxOptions))
		{
			xSemaphoreTake(tx_queue_handle, portMAX_DELAY);
		}

		if( xQueueSend( tx_descr_queue,( void * ) &TxBuffer,( TickType_t ) 10 ) != pdPASS )
		{
			/* Failed to post the message, even after 10 ticks. */
		}
		xSemaphoreGive(eth_blink_send);

		xSemaphoreGive( tx_send_mutex );
}

void enet_ieee1722_acf_can_send(uint8 instance, Flexcan_Ip_MsgBuffType *can_frame) {
	xSemaphoreTake( tx_send_mutex, portMAX_DELAY );

	Gmac_Ip_BufferType TxBuffer = {0};
	Gmac_Ip_TxOptionsType TxOptions = {FALSE, GMAC_CRC_AND_PAD_INSERTION, GMAC_CHECKSUM_INSERTION_DISABLE};
	/* Setup the frame with the Mac address and size */
	uint8 MacAddr[6U] = {0U};

	Gmac_Ip_GetMacAddr(INST_GMAC_0, MacAddr);

	/* Request a buffer of at least 64 bytes */
	TxBuffer.Length = 128U;
	while ((GMAC_STATUS_SUCCESS != Gmac_Ip_GetTxBuff(INST_GMAC_0, 0U, &TxBuffer, NULL_PTR)) || (TxBuffer.Length < 128U))
	{
		xSemaphoreTake(tx_queue_handle, portMAX_DELAY);
	}

	struct ethernet_frame* can_eth_frame = (struct ethernet_frame*)TxBuffer.Data;

	memset(can_eth_frame->dst_macaddr, 0xFF, ETH_ALEN); /* Broadcast */
	memcpy(can_eth_frame->src_macaddr, MacAddr, ETH_ALEN); /* Our own MAC addr */

	TxBuffer.Length = convert_can_to_avtp_ntscf(instance, (struct ethernet_frame*)TxBuffer.Data, can_frame);

	/* Send the ETH frame */
	/*true function that sends data to the transceiver*/
	while (GMAC_STATUS_TX_QUEUE_FULL == Gmac_Ip_SendFrame(INST_GMAC_0, 0U, &TxBuffer, &TxOptions))
	{
		xSemaphoreTake(tx_queue_handle, portMAX_DELAY);
	}

	if( xQueueSend( tx_descr_queue,( void * ) &TxBuffer,( TickType_t ) 10 ) != pdPASS )
	{
		/* Failed to post the message, even after 10 ticks. */
	}
	xSemaphoreGive(eth_blink_send);
	xSemaphoreGive( tx_send_mutex );

}
