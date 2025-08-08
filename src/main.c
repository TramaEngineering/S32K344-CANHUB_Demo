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
 *
 *   FreeRTOS IEEE1772 ACF-CAN application for MR-CANHUBK3
 */

/* Including necessary configuration files. */
#include "Clock_Ip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "Siul2_Icu_Ip.h"
#include "Siul2_Icu_Ip_Irq.h"
#include "IntCtrl_Ip.h"
#include "Gmac_Ip.h"
#include "FlexCAN_Ip.h"
#include "Lpspi_Ip.h"
#include <stdio.h>
#include <string.h>
//#include "avtp_defs.h"
#include "ethernet_type.h"

#include "can.h"
#include "enet.h"
#include "fs26.h"
#include "uart.h"
#include "./uart_print/retarget.h"
#include "Pit_Ip.h"
#include "IntCtrl_Ip.h"
#include "bsp.h"

//define a vector of can queues (RTOS)
#define CAN_COUNT 6
QueueHandle_t eth_can_queues[CAN_COUNT];
/*message queue to be filled with the message to be sent when button is pressed*/
QueueHandle_t tx_queue_send;

const char* buttonMsg = "CANHUBK3";

const Flexcan_Ip_MsgBuffType buttonCanFrame = {
		.cs = 0x0,
		.msgId = 0x12,
		.data = "CANHUBK3",
		.dataLen = 8,
		.id_hit = 0,
		.time_stamp = 0
};


Gmac_Ip_BufferType buttonEthFrame = { .Data = pDelayResp_frame, .Length=68};


void set_rgb_status(rgb_status status) {
	switch(status) {
		case INITIALIZE:
			Siul2_Dio_Ip_SetPins(LED_RED_PORT, (1 << LED_RED_PIN));
			Siul2_Dio_Ip_SetPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
			Siul2_Dio_Ip_ClearPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
			break;

		case ERROR:
			Siul2_Dio_Ip_ClearPins(LED_RED_PORT, (1 << LED_RED_PIN));
			Siul2_Dio_Ip_SetPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
			Siul2_Dio_Ip_SetPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
			break;

		case NOMINAL:
			Siul2_Dio_Ip_SetPins(LED_RED_PORT, (1 << LED_RED_PIN));
			Siul2_Dio_Ip_ClearPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
			Siul2_Dio_Ip_SetPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
			break;

		case ETH_ACTIVITY:
			Siul2_Dio_Ip_ClearPins(LED_RED_PORT, (1 << LED_RED_PIN));
			Siul2_Dio_Ip_SetPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
			Siul2_Dio_Ip_ClearPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
			break;

		case RED:
			Siul2_Dio_Ip_ClearPins(LED_RED_PORT, (1 << LED_RED_PIN));
			Siul2_Dio_Ip_SetPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
			Siul2_Dio_Ip_SetPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
			break;

		case GREEN:
			Siul2_Dio_Ip_SetPins(LED_RED_PORT, (1 << LED_RED_PIN));
			Siul2_Dio_Ip_ClearPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
			Siul2_Dio_Ip_SetPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
			break;

		case BLUE:
			Siul2_Dio_Ip_SetPins(LED_RED_PORT, (1 << LED_RED_PIN));
			Siul2_Dio_Ip_SetPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
			Siul2_Dio_Ip_ClearPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
			break;

		case CYAN:
			Siul2_Dio_Ip_SetPins(LED_RED_PORT, (1 << LED_RED_PIN));
			Siul2_Dio_Ip_ClearPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
			Siul2_Dio_Ip_ClearPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
			break;

		case YELLOW:
			Siul2_Dio_Ip_ClearPins(LED_RED_PORT, (1 << LED_RED_PIN));
			Siul2_Dio_Ip_ClearPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
			Siul2_Dio_Ip_SetPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
			break;

		case MAGENTA:
			Siul2_Dio_Ip_ClearPins(LED_RED_PORT, (1 << LED_RED_PIN));
			Siul2_Dio_Ip_SetPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
			Siul2_Dio_Ip_ClearPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
			break;

		case WHITE:
			Siul2_Dio_Ip_SetPins(LED_RED_PORT, (1 << LED_RED_PIN));
			Siul2_Dio_Ip_SetPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
			Siul2_Dio_Ip_SetPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
			break;
	}
}

rgb_status error_st = BLUE;

void HardFault_Handler(void)
{
	/* Hard fault occured */
	set_rgb_status(error_st);
	while(TRUE){};
}

void button_sw1(void)
{
	/* Send CAN Frame to the CAN0 device */
	//set_rgb_status(INITIALIZE);
	if( xQueueSendFromISR( eth_can_queues[0],
			( void * ) &buttonCanFrame,	NULL))
	{
		/* Failed queue CAN frame drop packet */
	}
}

void button_sw2_ethernet(void)
{

	if(xQueueSendFromISR(tx_queue_send, &pDelayResp, NULL)){
			/*the thread has fail to send the message after 10 tick so there will be some error*/
			/*put led blinking on a certain way*/
		}
}

//void button_sw2(void)
//{
//	/* Send CAN Frame to the CAN1 device */
//	if( xQueueSendFromISR( eth_can_queues[1],
//			( void * ) &buttonCanFrame,	NULL))
//	{
//		/* Failed queue CAN frame drop packet */
//	}
//}

/**
 * @brief            Function the fs26 source file will call to do SPI communication with the FS26.
 *
 * @param[in]        TxBuffer - pointer to transmit buffer.
 * @param[in-out]    RxBuffer - pointer to receive buffer.
 * @param[in]        Length - number of bytes to be sent.
 *
 * @return           0 = LPSPI_IP_STATUS_SUCCESS: Transmission command has been accepted.
 *                   1 = LPSPI_IP_FIFO_ERROR: Overflow or underflow error occurred.
 *                   2 = LPSPI_IP_STATUS_FAIL: Transmission command has not been accepted.
 *                   3 = LPSPI_IP_TIMEOUT: Timeout error occurred.
 */
int fs26SpiTransferFunction(uint8_t *TxBuffer, uint8_t *RxBuffer, uint16_t Length)
{
	Lpspi_Ip_StatusType spiStat;

	spiStat = Lpspi_Ip_SyncTransmit(&Lpspi_Ip_DeviceAttributes_SpiExternalDevice_3_Instance_3,
			TxBuffer, RxBuffer, Length, 1000);

	return (int)spiStat;
}
void link_check(uint8 channel){
	(void) channel;

	//printf("Hello i'm PIT\r\n");
}

/**
 * @brief        Main function of the example
 * @details      Initializes the used drivers and uses 1 binary Semaphore and
 *               2 tasks to toggle a LED.
 */

int main(void)
{
	/* Initialize Clock */
	OsIf_Init(NULL_PTR);

	/* Initialize all pins using the Port driver */
	Siul2_Port_Ip_PortStatusType Status_Init_Port = SIUL2_PORT_ERROR;
	Status_Init_Port = Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals, g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);

	if(Status_Init_Port != SIUL2_PORT_SUCCESS)
	{
		while(1); /* Error during initialization. */
	}

	/* Set RGB to indicate initialization */
	//set_rgb_status(INITIALIZE);

	Clock_Ip_StatusType Status_Init_Clock = CLOCK_IP_ERROR;
	Status_Init_Clock = Clock_Ip_Init(Clock_Ip_aClockConfig);

	if(Status_Init_Clock != CLOCK_IP_SUCCESS)
	{
		//set_rgb_status(ERROR);
		while(1); /* Error during initialization. */
	}

	/* Initialize the FS26 to stop it from resetting */
	Lpspi_Ip_Init(&Lpspi_Ip_PhyUnitConfig_SpiPhyUnit_3_Instance_3);
	Console_SerialPort_Init();
	/*ERROR IN START UP*/
	fs26_initialize(&fs26SpiTransferFunction);

	/* Intialize for SIUL ICU for external interrupts from the buttons */
	Siul2_Icu_Ip_Init(0, &Siul2_Icu_Ip_0_Config_PB);
	Siul2_Icu_Ip_EnableInterrupt(0, 5);  /* EIRQ5  PTA25 */
	Siul2_Icu_Ip_EnableNotification(0, 5);
	Siul2_Icu_Ip_EnableInterrupt(0, 31); /* EIRQ31 PTD15 */
	Siul2_Icu_Ip_EnableNotification(0, 31);

	/*enable IRQ for switch 1*/
	IntCtrl_Ip_EnableIrq(SIUL_0_IRQn);
	/*set handler for interrupt*/
	IntCtrl_Ip_InstallHandler(SIUL_0_IRQn, SIUL2_EXT_IRQ_0_7_ISR, NULL_PTR);
	/*set priority handler for the external input*/
	IntCtrl_Ip_SetPriority(SIUL_0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

	IntCtrl_Ip_EnableIrq(SIUL_3_IRQn);
	IntCtrl_Ip_InstallHandler(SIUL_3_IRQn, SIUL2_EXT_IRQ_24_31_ISR, NULL_PTR);
	IntCtrl_Ip_SetPriority(SIUL_3_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

	/*PIT initialization*/
	Pit_Ip_Init(PIT_0_IP_INSTANCE_NUMBER, &PIT_0_InitConfig_PB);
	/*PIT channel initialization*/
	Pit_Ip_InitChannel(PIT_0_IP_INSTANCE_NUMBER, &PIT_0_ChannelConfig_PB[0]);
	/*Start pit 0 channel 0*/
	Pit_Ip_StartChannel(PIT_0_IP_INSTANCE_NUMBER, 0, 16000000);/*400ms*/
	/*enable channel interrupt*/
	Pit_Ip_EnableChannelInterrupt(PIT_0_IP_INSTANCE_NUMBER, 0);

	/* Initialize ethernet MAC */
	/*set the MAC to slave mode and not to master mode*/
	Gmac_Ip_StatusType Status_Init_Gmac = GMAC_STATUS_ERROR;
	Status_Init_Gmac = enet_init(&tx_queue_send);

	if(Status_Init_Gmac != GMAC_STATUS_SUCCESS)
	{
		//set_rgb_status(ERROR);
		while(1); /* Error during initialization. */
	}

	/* Initialize CAN0 .. CAN5 */
	for(int i = 0; i < CAN_COUNT; i++) {
		CAN* can;
		can = can_init(i);
		if(can != NULL) {
			eth_can_queues[i] = can->eth_can_queue;
		} else {
			//set_rgb_status(ERROR);
			while(1); /* Error during initialization. */
		}
	}

	/*have to init on another way the interrupts because at this moment all the other interrupt are misconfigured*/
	/*route the handler to the interrupt*/
	//IntCtrl_Ip_ConfigIrqRouting(&intRouteConfig);
	/*load the interrupt configuration*/
	//IntCtrl_Ip_Init(&IntCtrlConfig_0);

	/*start link check task*/
	start_link_check();

	/* Start Listening to ethernet packets */
	enet_start_rx(eth_can_queues, CAN_COUNT);

	/*create a thread that pools on a message queue and send the message when it receive one*/
	enet_start_tx();

	//set_rgb_status(NOMINAL);

	/* Start FreeRTOS */
	vTaskStartScheduler();

	/* Scheduler returned this an error was encountered */
	set_rgb_status(ERROR);

	printf("Error in code!\r\n");

	for( ;; );

	return 0;
}

/** @} */
