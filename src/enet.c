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
//#include "avtp_defs.h"
#include "ethernet_type.h"
#include "uart.h"
#include "./uart_print/retarget.h"
#include "fs26.h"
#include "bsp.h"
//#include "S32K311_DCM_GPR.h"

//gPTP libraries
#include "EthIf_Cbk.h"
#include "gptp_port_platform.h"
#include "gptp_frame.h"
#include <float.h>

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/

#define TIMEOUT_MS					(200U)
#define TJA1103_DEV_ID 				(0x001BU)
#define RGMII_SUPPORTED 			(0U)
#define CFG_PHY_CTRL_IDX        	(0U)
#define VLAN_ACTIVE					(1U)

/* MMDs */
#define PHYAD                       18
#define MMD1						(1U)
#define MMD30						(30U)

#define PMA_STATUS1                 (0x0001U)
#define BT1_PMA_CONTROL_REG_ADR		(0x0834U)
#define DEV_CONTR_REG_ADR			(0x0040U)
#define PHY_CONTR_REG_ADR			(0x8100U)
#define DEVICE_CONTROL				(0x40U)
#define PHY_STATUS_REGISTER			0x8102

#define PMA_STATUS_LINK_STATUS		(1 << 2)

#define BT1_PMAPMD_CONFIG_EN		(1 << 15)
#define BT1_PMAPMD_MASTER		    (1 << 14)
#define BT1_PMAPMD_SLAVE		    0XFFFF & ~(1 << 14)

#define DEV_GLOBAL_CONF_ENA_FLAG		(0x4000U)
#define DEV_SUPER_CONFIG_ENA_FLAG		(1 << 13)
#define DEV_SUPER_CONFIG_DIS_FLAG		0XFFFF & ~(1 << 13)
#define PHY_CONFIG_EN_FLAG 				(0x4000U)

/*TS register macro*/
#define SUPER_CONFIG_ENABLE			(0x2000U) //enable reconfiguration regiters to enable ptp timestamping
#define PORT_FUNC_ENABLE			(0X8048U)
#define PTP_CLK_PERIOD				(0x1104U) //set ptp clk period to 15ns because of 33.3MHz clock
#define PKT_FILT_CTRL				(0x1140U) //set filter to get timestamp
#define TX_PIPE_DLY_NS				(0x1149U) //set ts delay in tx
#define RX_PIPE_DLY_NS				(0x114BU) //set ts delay in rx
#define LTC_LOAD_CTRL				(0x1105U) //to capture the LTC
#define LTC_RD_DATA_0				(0x110AU) //get LCT timnestamp ns 0_15
#define LTC_RD_DATA_1				(0x110BU) //get LCT timnestamp ns 16_29
#define LTC_RD_DATA_2				(0x110CU) //get LCT timnestamp ns 0_15
#define LTC_RD_DATA_3				(0x110DU) //get LCT timnestamp ns 16_31
//ingress data registers
#define INGR_TS_0					(0X1155U)
#define INGR_TS_1					(0X1156U)
#define INGR_TS_2					(0x1157U)
#define INGR_TS_3					(0X1158U)
#define INGR_TS_4					(0X1159U)
#define INGR_TS_5					(0X115AU)
#define ING_RING_DONE				(0x115BU) //after read ts in order to flush ring position
//egress data registers
#define EGR_TS_0					(0X114EU)
#define EGR_TS_1					(0X114FU)
#define EGR_TS_2					(0x1150U)
#define EGR_TS_3					(0X1151U)
#define EGR_TS_4					(0X1152U)
#define EGR_TS_5					(0X1153U)
#define EGR_RING_DONE				(0X1154U) //after read ts in order to flush ring position
#define PTP_IRQ_SOURCE				(0x1130U)
#define PTP_IRQ_ENABLE				(0x1131U)
#define MASK_ING_LOST(x)			((x & 0x0040U)>>6)
#define MASK_EGR_LOST(x)			((x & 0x0020U)>>5)

#define RX_TS_INSRT_CTRL			(0x114DU)

#define SWAP16(x)  (((x) >> 8) | ((x) << 8))

/*==================================================================================================
*                                       LOCAL VARIABLES
==================================================================================================*/

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

QueueHandle_t* eth_can_queue;
int can_count;
TaskHandle_t rx_task;
TaskHandle_t tx_task;
TaskHandle_t link_check_task;
int phyad = 0;

extern uint32_t __UTEST_UID[2];

/*==================================================================================================
*                                 LOCAL STRUCTURES AND TYPES
==================================================================================================*/

DescrBuffer bufferQueue[MAX_TX_PENDING];

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

uint8 annouce_frame[48] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // MAC Dest (TRM PC)
		0x66, 0x55, 0x44, 0x33, 0x22, 0x11,  // MAC Src (esempio)
		0x08, 0x06,                          // EtherType: ARP
		// Inizio payload ARP (dummy data per esempio)
		0x00, 0x01, //HW ethertype
		0x08, 0x00, // IPV4
		0x06, //HW size
		0x04, //PROTOCOL size
		0x00, 0x01,	//opcode: request
		0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //sender mac address
		0xc0, 0xa8, 0x00, 0x01, //sender ip address
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	//target mac address
		0x00, 0x00, 0x00, 0x00, //target ip address
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

uint8 residence_frame[48] = {
		0x10, 0x11, 0x22, 0x88, 0x88, 0x88,  // MAC Dest (TRM PC)
		0x66, 0x55, 0x44, 0x33, 0x22, 0x11,  // MAC Src (esempio)
		0x08, 0x06,                          // EtherType: ARP
		// Inizio payload ARP (dummy data per esempio)
		0x00, 0x01, //HW ethertype
		0x08, 0x00, // IPV4
		0x06, //HW size
		0x04, //PROTOCOL size
		0x00, 0x01,	//opcode: request
		0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //sender mac address
		0xc0, 0xa8, 0x00, 0x01, //sender ip address
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	//target mac address
		0x00, 0x00, 0x00, 0x00, //target ip address
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

uint8 button_eth_frame[80] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // MAC Dest (TRM PC)
		0x66, 0x55, 0x44, 0x33, 0x22, 0x11,  // MAC Src (esempio)
		0x08, 0x06,                          // EtherType: ARP
		// Inizio payload ARP (dummy data per esempio)
		0x00, 0x01, //HW ethertype
		0x08, 0x00, // IPV4
		0x06, //HW size
		0x04, //PROTOCOL size
		0x00, 0x01,	//opcode: request
		0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //sender mac address
		0xc0, 0xa8, 0x00, 0x01, //sender ip address
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	//target mac address
		0x00, 0x00, 0x00, 0x00, //target ip address
		0x48, 0x69, 0x20, 0x74,
		0x68, 0x69,
		0x73, 0x20, 0x69, 0x73,
		0x20, 0x62, 0x75, 0x74,
		0x74, 0x6F, 0x6E, 0x20,
		0x65, 0x74, 0x68, 0x20,
		0x66, 0x72, 0x61, 0x6D, 0x65 //71
	};

uint8 pDelayResp_frame[68] = {
	    0x1, 0x80, 0xc2, 0x0, 0x0, 0xe,
	    0x66, 0x55, 0x44, 0x33, 0x22, 0x11,
	    0x88, 0xf7,
	    0x13,//domain  and message type
	    0x2,//version PTPv2 = gPTP
	    0x0, 0x36, //MESSAGE LENGHT
	    0x0,
	    0x0,
	    0x0, 0x8, //FLAGS
	    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, //CORRECTION FIELD (NS)
	    0x0, 0x0, 0x0, 0x0, //MESSAGE SPECIFIC
	    0x38, 0x2a, 0x19, 0xff, 0xfe, 0x0, 0x4c, 0x3e, //CLOCK IDENTITY
	    0x0, 0x0, //SOURCE PORT ID
	    0x2, 0x9b, //SEQUENCE ID
	    0x5, //CONTROL FIELD
	    0x7f, //LONG MESSAGE PERIOD
	    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	};// NO RESPONSE FOR THIS DELAY REQ

uint8_t customIPv4Frame[] = {
    // --- Ethernet header ---
		//0x66,0x55,0x44,0x33,0x22,0x11,    // Src MAC
		//0xFC, 0xC2, 0x3D, 0x5E, 0x0F, 0x8B, //fibrecode
		0x10,0x11,0x22,0x88,0x88,0x88,    // Dest MAC
		0x66,0x55,0x44,0x33,0x22,0x11,
		0x08,0x00,                        // EtherType = IPv4

		// --- IPv4 header (20B) ---
		0x45,       // Version=4, IHL=5
		0x00,       // DSCP/ECN
		0x00,0x24,  // Total length = 36 bytes (20+8+8)
		0x00,0x01,  // Identification
		0x40,0x00,  // Flags + Fragment offset
		0x40,       // TTL = 64
		0x11,       // Protocol = UDP (0x11)
		0xB8,0x53,  // Header checksum (calcolato)
		0xC0,0xA8,0x00,0x01,  // Src IP = 192.168.0.2
		0xC0,0xA8,0x00,0x02,  // Dst IP = 192.168.0.1

		// --- UDP header (8B) ---
		0x12,0x34,  // Src port = 0x1234
		0x56,0x78,  // Dst port = 0x5678
		0x00,0x10,  // Length = 16 (8B UDP hdr + 8B payload)
		0x00,0x00,  // Checksum = 0 (disabilitato)

		// --- Payload (8B) ---
		0xDE,0xAD,0xBE,0xEF,0xCA,0xFE,0xBA,0xBE
};

// --- Frame UDP con 128B payload ---
uint8_t udpFrame128[14 + 20 + 8 + 128] = {
    // Ethernet header (14B)
    0x10,0x11,0x22,0x88,0x88,0x88,  // Dest MAC
    0x66,0x55,0x44,0x33,0x22,0x11,  // Src MAC
	0x81,0x00,                        // EtherType = VLAN
	0x40,0x02,						//priotity-DEI-ID
	0x08,0x00,						//Ethertype = IPv4

    // IPv4 header (20B)
    0x45, 0x00, 0x00, 0x9C,  // Total length = 20+8+128 = 156 = 0x009C
    0x00,0x01, 0x40,0x00, 0x40,0x11, 0xB8,0x53,
    0xC0,0xA8,0x00,0x01,  // Src IP
    0xC0,0xA8,0x00,0x02,  // Dst IP

    // UDP header (8B)
    0x12,0x34,0x34,0x12,  // Src/Dst port
    0x00,0x88,0x00,0x00,  // Length = 8 + 128 = 136 = 0x0088
                           // Checksum 0

    // Payload 128B
    // esempio con pattern incrementale
};
Gmac_Ip_BufferType customMessage_UDP_128 = { .Data = udpFrame128, .Length = 18 + 20 + 8 + 128 };

Gmac_Ip_BufferType customMessage_ipv4 = { .Data = customIPv4Frame, .Length = 60/*54*/ };

Gmac_Ip_BufferType pDelayResp = { .Data = pDelayResp_frame, .Length = 68 };
Gmac_Ip_BufferType arpAnnouce = { .Data = annouce_frame, .Length = 48 };

/*==================================================================================================
*                                  External FUNCTIONS FreeRTOS
==================================================================================================*/
/*================================================================================================*/

Gmac_Ip_StatusType enet_init_freertos(QueueHandle_t* tx_descr_queue_m) {

	//RMII mode
	IP_DCM_GPR->DCMRWF1 = (IP_DCM_GPR->DCMRWF1 & ~DCM_GPR_DCMRWF1_MAC_CONF_SEL_MASK) | DCM_GPR_DCMRWF1_MAC_CONF_SEL(2U);

	/* Initialize and enable the GMAC module */
	Gmac_Ip_StatusType Status_Init_Gmac = GMAC_STATUS_ERROR;
	Status_Init_Gmac = Gmac_Ip_Init(INST_GMAC_0, &Gmac_0_ConfigPB);

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
	tx_descr_queue_send = xQueueCreate(GMAC_0_MAX_TXBUFF_SUPPORTED, sizeof(Gmac_Ip_BufferType));
	*tx_descr_queue_m = tx_descr_queue_send;

	return Status_Init_Gmac;
}

void eth_activity_led( void *arg ){
	(void) arg;
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

void eth_activity_led_send( void *arg ){
	(void) arg;
	BaseType_t operation_status;

	for(;;){
		operation_status = xSemaphoreTake(eth_blink_send, portMAX_DELAY);
		configASSERT(operation_status == pdTRUE);

		/*set another task to do the uart shell write and readdsdsdsdsdsd*/
		//Lpuart_Uart_Ip_AsyncSend(LPUART_UART_IP_INSTANCE_USING_2, Txbuff2, 16);

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
	uint16_t device_control_reg;
	//phyad = TJA1103_find_addr();

	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEV_CONTR_REG_ADR, &device_control_reg, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEV_CONTR_REG_ADR, &device_control_reg, 100);
#ifdef DEBUG_PRINT
	printf("device control register mmd30 reg 40 before: ");
	print_16(&device_control_reg);
#endif

	Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, DEV_CONTR_REG_ADR,
			device_control_reg | DEV_SUPER_CONFIG_ENA_FLAG, 100);

	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEV_CONTR_REG_ADR, &device_control_reg, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEV_CONTR_REG_ADR, &device_control_reg, 100);
#ifdef DEBUG_PRINT
	printf("device control register mmd30 reg 40: ");
	print_16(&device_control_reg);
#endif

}

void tja1103_config_disable(void)
{
	uint16_t device_control_reg;
	//phyad = TJA1103_find_addr();

	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEV_CONTR_REG_ADR, &device_control_reg, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEV_CONTR_REG_ADR, &device_control_reg, 100);
#ifdef DEBUG_PRINT
	printf("device control register mmd30 reg 40 before: ");
	print_16(&device_control_reg);
#endif

	Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, DEV_CONTR_REG_ADR,
			device_control_reg & DEV_SUPER_CONFIG_DIS_FLAG, 100);

	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEV_CONTR_REG_ADR, &device_control_reg, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEV_CONTR_REG_ADR, &device_control_reg, 100);
#ifdef DEBUG_PRINT
	printf("device control register mmd30 reg 40: ");
	print_16(&device_control_reg);
#endif
}

void tja1103_wait_for_link(void) {
	uint16_t regvalue;
	Gmac_Ip_StatusType read_result;

	//pma is a sub group of MDIO registers
	read_result = Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD1, PHY_STATUS_REGISTER, &regvalue, TIMEOUT_MS);//receive link status on pma status1
	read_result |= Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD1, PHY_STATUS_REGISTER, &regvalue, TIMEOUT_MS);//receive link status on pma status1

	if(E_OK != read_result){
		printf("Read gone wrong!!\r\n");
	}
	/* Use chip UID for a random seed */
	vTaskDelay(500);

	if((regvalue & PMA_STATUS_LINK_STATUS) == 0) {
		tja1103_config_enable();

		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD1, BT1_PMA_CONTROL_REG_ADR, &regvalue, TIMEOUT_MS);//base_t1_PMA_CONTROL register 1<<14 is master
		if(regvalue & BT1_PMAPMD_MASTER) {
			Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD1, BT1_PMA_CONTROL_REG_ADR,
					regvalue & BT1_PMAPMD_SLAVE, 100);
		} else {
			Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD1, BT1_PMA_CONTROL_REG_ADR,
					regvalue | BT1_PMAPMD_MASTER, 100);
		}

		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD1, BT1_PMA_CONTROL_REG_ADR, &regvalue, 100);
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD1, BT1_PMA_CONTROL_REG_ADR, &regvalue, 100);

		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD1, PMA_STATUS1, &regvalue, TIMEOUT_MS);
		tja1103_config_disable();

		/* Wait for completion */
		vTaskDelay(500);

		/*if a new link is up so we have to send a new annouce*/
		read_result = Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD1, PHY_STATUS_REGISTER, &regvalue, TIMEOUT_MS);//receive link status on pma status1
		read_result = Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD1, PHY_STATUS_REGISTER, &regvalue, TIMEOUT_MS);//receive link status on pma status1
		if(E_OK != read_result){
			#ifdef DEBUG_PRINT
				printf("Read gone wrong!!\r\n");
			#endif
			}
		if((regvalue & PMA_STATUS_LINK_STATUS) != 0) {
			//init_annouce();
			xQueueSend(tx_descr_queue, &arpAnnouce, TIMEOUT_MS);
		}
	}
}

void link_check_worker(void *args){
	(void) args;

	for(;;){
		tja1103_wait_for_link();
		vTaskDelay(500);
	}
}

void eth_tx_worker( void * arg) {
	Gmac_Ip_BufferType buffer_frame;
	(void)arg;

	for( ;; )
	{
		/*wait for message in the in the queue and sends it*/
		if(xQueueReceive(tx_descr_queue_send, &buffer_frame, portMAX_DELAY) == pdTRUE) {
			//buffer frame
			send_eth_frame(&buffer_frame);
		}
	}
}


void eth_rx_worker(void *arg) {
	(void)arg;
	volatile Gmac_Ip_StatusType Status;
	Gmac_Ip_BufferType RxBuffer = {0};
	Gmac_Ip_RxInfoType RxInfo  = {0};
	uint32_t ulInterruptStatus;
	BaseType_t notified = 0L;
	boolean IsBroadcast;
	uint16 PayloadLength;
	Gmac_Ip_TimestampType srIngressTimeStamp;


	for( ;; )
	{
		Status = Gmac_Ip_ReadFrame(INST_GMAC_0, 0U, &RxBuffer, &RxInfo);


		/* If no packet, Wait for the frame to be received */
		while (Status == GMAC_STATUS_RX_QUEUE_EMPTY) {
			notified = xTaskNotifyWaitIndexed( 0,
					0x00,               /* Don't clear any bits on entry. */
					0xFFFFFFFF,          /* Clear all bits on exit. */
					&ulInterruptStatus, /* Receives the notification value. */
					portMAX_DELAY );    /* Block indefinitely. */
			if(notified != 0)
				Status = Gmac_Ip_ReadFrame(INST_GMAC_0, 0U, &RxBuffer, &RxInfo);
		}


#ifdef DEBUG_PRINT
		UBaseType_t stack_left = uxTaskGetStackHighWaterMark(NULL);
		printf("RxTask stack left: %lu\n", stack_left);
		char buffer[1024];
		vTaskList(buffer);
		printf("%s\n", buffer);
		printf("=================\r\n");
#endif

		if(notified != 0){
			xSemaphoreGive(eth_blink);
			const struct ethernet_frame* ether_frame = (struct ethernet_frame*)RxBuffer.Data;
			Gmac_Ip_ProvideRxBuff(INST_GMAC_0, 0U, &RxBuffer);

			IsBroadcast = (ether_frame->dst_macaddr[0] == 0xFF) && (ether_frame->dst_macaddr[1] == 0xFF) && (ether_frame->dst_macaddr[2] == 0xFF) && (ether_frame->dst_macaddr[3] == 0xFF) && (ether_frame->dst_macaddr[4] == 0xFF) && (ether_frame->dst_macaddr[5] == 0xFF);
			PayloadLength = RxInfo.PktLen-((2*ETH_ALEN)+2);

			if((ether_frame->dst_macaddr[0] == 0x01) && (ether_frame->dst_macaddr[1] == 0x80) && (ether_frame->dst_macaddr[2] == 0xC2) && (ether_frame->dst_macaddr[3] ==0x00) && (ether_frame->dst_macaddr[4] == 0x00) && (ether_frame->dst_macaddr[5] == 0x0E)){
				//get_ts_ingress_data(&srIngressTimeStamp);
				//printf("send resp delay\r\n");
				xQueueSend(tx_descr_queue_send, &pDelayResp, portMAX_DELAY);
			}
			else{
				xQueueSend(tx_descr_queue_send, &customMessage_ipv4, portMAX_DELAY);
			}



            //EthIf_RxIndication(CFG_PHY_CTRL_IDX, ether_frame->ether_type, IsBroadcast, &ether_frame->dst_macaddr, &ether_frame->data, PayloadLength, RxInfo.Timestamp);

		}


		/*
		bus_id = convert_ethernet_to_can(&txData, (struct ethernet_frame*)RxBuffer.Data);
		bus_id = 0U;

		Flexcan_Ip_MsgBuffType message;
		message = CanAvtp;
		ether_frame = (struct ethernet_frame*)RxBuffer.Data;
		//i have to swap the ethertype for a correct read
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

		if(bus_id >= 0 && bus_id < can_count) {
			if( xQueueSend( eth_can_queue[bus_id],
					( void * ) &message,
					( TickType_t ) 1000000 != pdPASS ))
			{
				// Failed queue CAN frame drop packet
			}

		}*/


	}
}

void start_link_check(void){
	xTaskCreate(link_check_worker, "PHY_LINK_CHECK", 512, NULL, eth_TASK_PRIORITY_1, &link_check_task);
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


Gmac_Ip_StatusType send_eth_frame(Gmac_Ip_BufferType* eth_message){
	#ifdef DEBUG_PRINT
		printf("Im about to send! \r\n");
	#endif
	Gmac_Ip_StatusType eERROR = GMAC_STATUS_SUCCESS;
	xSemaphoreTake(tx_send_mutex, portMAX_DELAY);

	Gmac_Ip_BufferType TxBuffer = {0};
	Gmac_Ip_TxOptionsType TxOptions = {FALSE, GMAC_CRC_AND_PAD_INSERTION, GMAC_CHECKSUM_INSERTION_DISABLE};

	uint8 MacAddr[6U] = {0U};

	Gmac_Ip_GetMacAddr(INST_GMAC_0, MacAddr);

	/*request a buffer of at least 64 bytes*/
	TxBuffer.Length = 128U;

	eERROR = Gmac_Ip_GetTxBuff(INST_GMAC_0, 0u, &TxBuffer, NULL_PTR);
	while(GMAC_STATUS_SUCCESS != eERROR || (TxBuffer.Length < 128U)){
		xSemaphoreTake(tx_queue_handle, portMAX_DELAY);
	}

	TxBuffer = *eth_message;
	struct ethernet_frame * eth_frame = (struct ethernet_frame*)TxBuffer.Data;

	//memset(eth_frame->dst_macaddr, 0xFF, ETH_ALEN); /* Broadcast */
	memcpy(eth_frame->src_macaddr, MacAddr, ETH_ALEN); /* Our own MAC addr */

	//TxBuffer.Length = 64U - 4U;     /* Don't count FCS, because it is automatically inserted by the controller in this example */

	/* Send the ETH frame */
	/*true function that sends data to the transceiver*/
	eERROR = Gmac_Ip_SendFrame(INST_GMAC_0, 0U, &TxBuffer, &TxOptions);
	while (GMAC_STATUS_TX_QUEUE_FULL == eERROR)
	{
		xSemaphoreTake(tx_queue_handle, portMAX_DELAY);
	}

	if( xQueueSend( tx_descr_queue,( void * ) &TxBuffer,( TickType_t ) 10 ) != pdPASS )
	{
		/* Failed to post the message, even after 10 ticks. */
		printf("Message send fail after 10 ticks!\r\n");
	}
	xSemaphoreGive(eth_blink_send);

	xSemaphoreGive( tx_send_mutex );
	#ifdef DEBUG_PRINT
		printf("Sent message!\r\n");
	#endif

	return eERROR;

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

/*==================================================================================================
*                               External FUNCTIONS Loop gPTP
==================================================================================================*/
/*================================================================================================*/
bool get_device_link_status(void){
	uint16 Tja1103_Base_Status;
	Gmac_Ip_StatusType ePHY_Status;

	ePHY_Status = Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD1, PHY_STATUS_REGISTER, &Tja1103_Base_Status, TIMEOUT_MS);//receive link status on pma status1
	ePHY_Status = Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD1, PHY_STATUS_REGISTER, &Tja1103_Base_Status, TIMEOUT_MS);//receive link status on pma status1

	DevAssert((Gmac_Ip_StatusType)GMAC_STATUS_SUCCESS == ePHY_Status);

	return (0 != (Tja1103_Base_Status & PMA_STATUS_LINK_STATUS));
}

Gmac_Ip_StatusType enet_init(void) {

	//RMII mode
	IP_DCM_GPR->DCMRWF1 = (IP_DCM_GPR->DCMRWF1 & ~DCM_GPR_DCMRWF1_MAC_CONF_SEL_MASK) | DCM_GPR_DCMRWF1_MAC_CONF_SEL(2U);
	uint16_t port_func, phy_control, ptp_clk_period, tx_pipe_dly_ns, rx_pipe_dly_ns, irq_en, reg_1, embed_ingress_ts, error_counter, ctrl_filter;

	/* Initialize and enable the GMAC module */
	Gmac_Ip_StatusType Status_Init_Gmac = GMAC_STATUS_ERROR;
	Status_Init_Gmac = Gmac_Ip_Init(INST_GMAC_0, &Gmac_0_ConfigPB);

	if(Status_Init_Gmac != GMAC_STATUS_SUCCESS)
	{
		return Status_Init_Gmac;
	}

	/*clear all the ring buffer slots*/
	Gmac_Ip_MDIOWriteMMD(0,	PHYAD, MMD30, ING_RING_DONE, 0X0001, 100);
	Gmac_Ip_MDIOWriteMMD(0,	PHYAD, MMD30, ING_RING_DONE, 0X0001, 100);
	Gmac_Ip_MDIOWriteMMD(0,	PHYAD, MMD30, ING_RING_DONE, 0X0001, 100);
	Gmac_Ip_MDIOWriteMMD(0,	PHYAD, MMD30, ING_RING_DONE, 0X0001, 100);
	Gmac_Ip_MDIOWriteMMD(0,	PHYAD, MMD30, ING_RING_DONE, 0X0001, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, INGR_TS_1, &reg_1, 100);/*sequence id of ring buffer*/

	//read port enable to allow registers configuration
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, PORT_FUNC_ENABLE, &port_func, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, PORT_FUNC_ENABLE, &port_func, 100);
	//check clk period for 100base t1 connection
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, PTP_CLK_PERIOD, &ptp_clk_period, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, PTP_CLK_PERIOD, &ptp_clk_period, 100);
	//check tx pipe delay to be added to eggress ts due to circuirty loss
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, TX_PIPE_DLY_NS, &tx_pipe_dly_ns, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, TX_PIPE_DLY_NS, &tx_pipe_dly_ns, 100);
	//check rx pipe delay to be added to ingress ts due to circuirty loss
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, RX_PIPE_DLY_NS, &rx_pipe_dly_ns, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, RX_PIPE_DLY_NS, &rx_pipe_dly_ns, 100);

	while((rx_pipe_dly_ns & 0x019D) != 0x019D){
		rx_pipe_dly_ns |= 0x019D;
		Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, RX_PIPE_DLY_NS, rx_pipe_dly_ns, 100);

		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, RX_PIPE_DLY_NS, &rx_pipe_dly_ns, 100);
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, RX_PIPE_DLY_NS, &rx_pipe_dly_ns, 100);

		if((rx_pipe_dly_ns & 0x019D) == 0x019D)
			printf("TS Tx delay set to 413 ns!\r\n");
	}
	while((tx_pipe_dly_ns & 0x004D) != 0x004D){
		tx_pipe_dly_ns |= 0x004D;
		Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, TX_PIPE_DLY_NS, tx_pipe_dly_ns, 100);

		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, TX_PIPE_DLY_NS, &tx_pipe_dly_ns, 100);
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, TX_PIPE_DLY_NS, &tx_pipe_dly_ns, 100);

		if((tx_pipe_dly_ns & 0x004D) == 0x004D)
			printf("TS Tx delay set to 77 ns!\r\n");
	}

	while((ptp_clk_period & 0x000F) != 0x000F){
		/*8ns*/
		ptp_clk_period |= 0x000F;
		Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, 0x1104, ptp_clk_period, 100);

		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, 0x1104, &ptp_clk_period, 100);
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, 0x1104, &ptp_clk_period, 100);

		if((ptp_clk_period & 0x000F) == 0x000F)
			printf("Ptp period set to 15ns!\r\n");
	}

//	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, RX_TS_INSRT_CTRL, &embed_ingress_ts, 100);
//	//if((embed_ingress_ts & 0x0100) >> 8 == 1){
//		embed_ingress_ts = 0x0090;
//		Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, RX_TS_INSRT_CTRL, embed_ingress_ts, 100);
//		printf("HW timestamp embed in gPTP message!\r\n");
//	//}
//	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, RX_TS_INSRT_CTRL, &embed_ingress_ts, 100);
	/*Read EVENT_MSG_FILT*/

	if(VLAN_ACTIVE){
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, PKT_FILT_CTRL, &ctrl_filter, 100);
		if((ctrl_filter & 0x4000) != 0x4000){ //check if vlan filter is active
			ctrl_filter |= 0x4000;
			Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, PKT_FILT_CTRL, ctrl_filter, 100);
		}
	}
	else{
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, 0x1148, &ctrl_filter, 100);
				if((ctrl_filter & 0x4000) == 0x4000){ //check if vlan filter is active
					ctrl_filter = 0x0000;
					Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, PKT_FILT_CTRL, ctrl_filter, 100);
				}
	}

	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, 0x1148, &embed_ingress_ts, 100);


	while((port_func & 0x0008) != 0x0008 ){
		printf("HW timestamp disabled!\r\n");
		/*set PHY_control to config_enable in order to be able to write on registers*/
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEVICE_CONTROL, &phy_control, 100);//bit 13 need to be at 1
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEVICE_CONTROL, &phy_control, 100);
		phy_control |= SUPER_CONFIG_ENABLE;
		Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, DEVICE_CONTROL, phy_control, 100);
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEVICE_CONTROL, &phy_control, 100);
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEVICE_CONTROL, &phy_control, 100);

		/*enable the PTP HW timestamping*/
		port_func |= 0x0008;
		Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, PORT_FUNC_ENABLE, port_func, 100);

		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, PORT_FUNC_ENABLE, &port_func, 100);
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, PORT_FUNC_ENABLE, &port_func, 100);

		if((port_func & 0x0008) == 0x0008){
			printf("PTP enabled! \r\n");
			Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEVICE_CONTROL, &phy_control, 100);//bit 13 need to be at 1
			Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEVICE_CONTROL, &phy_control, 100);
			phy_control &= ~SUPER_CONFIG_ENABLE;
			Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, DEVICE_CONTROL, phy_control, 100);
			Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEVICE_CONTROL, &phy_control, 100);
			Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, DEVICE_CONTROL, &phy_control, 100);
		}
	}

	/*enable IRQ interrupt for ing/egr TS overflow*/
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, PTP_IRQ_ENABLE, &irq_en, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, PTP_IRQ_ENABLE, &irq_en, 100);
	Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, PTP_IRQ_ENABLE, 0x0060, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, PTP_IRQ_ENABLE, &irq_en, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, PTP_IRQ_ENABLE, &irq_en, 100);

	/*Check and enable ERROR_COUNTER_MISC*/
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, 0x8352, &error_counter, 100);
	while(error_counter>>15 == 0){
		error_counter |= 0x8000;
		Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, 0x8352, error_counter, 100);
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, 0x8352, &error_counter, 100);
	}
	printf("Error counter enabled!\r\n");


	return Status_Init_Gmac;
}

void eth_rx_check(void){
		volatile Gmac_Ip_StatusType Status;
		Gmac_Ip_BufferType RxBuffer = {0};
		Gmac_Ip_BufferType RxBuffer_cpy = {0};
		Gmac_Ip_RxInfoType RxInfo  = {0};
		boolean IsBroadcast;
		uint16 PayloadLength, etherType, seq_id;
		//Gmac_Ip_TimestampType currentTime;
		const Eth_DataType *frame_data;
		Gmac_Ip_TimestampType srIngressTimeStamp;

		Status = Gmac_Ip_ReadFrame(INST_GMAC_0, 0U, &RxBuffer, &RxInfo);
		/* If no packet, Wait for the frame to be received */
		if (Status != GMAC_STATUS_RX_QUEUE_EMPTY) {
				Gmac_Ip_ProvideRxBuff(INST_GMAC_0, 0U, &RxBuffer);

				memcpy(RxBuffer_cpy.Data, RxBuffer.Data, RxBuffer.Length);
				struct ethernet_frame* ether_frame = (struct ethernet_frame*)RxBuffer_cpy.Data;
				IsBroadcast = (ether_frame->dst_macaddr[0] == 0xFF) && (ether_frame->dst_macaddr[1] == 0xFF) && (ether_frame->dst_macaddr[2] == 0xFF) && (ether_frame->dst_macaddr[3] == 0xFF) && (ether_frame->dst_macaddr[4] == 0xFF) && (ether_frame->dst_macaddr[5] == 0xFF);
				PayloadLength = RxInfo.PktLen-((2*ETH_ALEN)+2);
				etherType = SWAP16(ether_frame->ether_type);
				if((etherType == 0x8100 && ether_frame->data[2] == 0x88 && ether_frame->data[3] == 0xf7) || etherType == 0x88f7){
					//etherType = 0x88f7;
					frame_data = ether_frame->data;
					get_ltc_counter(&srIngressTimeStamp);
					if(ether_frame->data[VLAN_ACTIVE*4] == 0x12){/*Pdelay req*/
						seq_id = ether_frame->data[30+VLAN_ACTIVE*4]<<8 | ether_frame->data[31+VLAN_ACTIVE*4];
						get_ts_ingress_data(&srIngressTimeStamp, seq_id);
					}
					EthIf_RxIndication(CFG_PHY_CTRL_IDX, etherType, IsBroadcast, (const uint8*)ether_frame->dst_macaddr, frame_data, PayloadLength, srIngressTimeStamp);
				}
		}
}

void enet_tx_free_buffer(void){
	Gmac_Ip_TxInfoType TxInfo  = {0};
	Gmac_Ip_BufferType TxBuffer = {0};
	Gmac_Ip_StatusType trasmit_status = GMAC_STATUS_SUCCESS;
	uint16 etherType, seq_id;
	struct ethernet_frame* ether_frame;
	Gmac_Ip_TimestampType srEgressTimeStamp, currentTime;
	uint8 offset_vlan = 0;

	for(uint8 index = 0; index < MAX_TX_PENDING && bufferQueue[index].inUse ; index++){
			TxBuffer.Data = bufferQueue[index].Data;
			TxBuffer.Length = bufferQueue[index].Length;
			trasmit_status = Gmac_Ip_GetTransmitStatus(CFG_PHY_CTRL_IDX, 0U, &TxBuffer, &TxInfo);

			if(trasmit_status == GMAC_STATUS_BUSY){
					/*descriptor still busy so send back the message in the queue*/
			}
			else if( trasmit_status == GMAC_STATUS_BUFF_NOT_FOUND ){
				printf("Buffer not found!\r\n");
			}
			else if(trasmit_status == GMAC_STATUS_SUCCESS){
				/*I have to call EthIf function to pass timestamp to the state machine*/
				/*second parameter has to be the BufIdx*/
				/*DONE: Get the timestamp TX from the HW on exit to be passed to EthIf_TxConfirmation*/

				ether_frame = (struct ethernet_frame*)TxBuffer.Data;
				etherType = SWAP16(ether_frame->ether_type);
				if(etherType == 0x8100){//If gPTP frame
					offset_vlan = 4;
				}
				else{
					offset_vlan = 0;
				}
				if(*(&ether_frame->ether_type+(offset_vlan/2)) == 0xf788){//If gPTP frame
					seq_id = ether_frame->data[30+offset_vlan]<<8 | ether_frame->data[31+offset_vlan];
					if(ether_frame->data[offset_vlan] == 0x13 || ether_frame->data[offset_vlan] == 0x10 || ether_frame->data[offset_vlan] == 0x12){//sending Pdelay resp or sync
						//Magenta
						Siul2_Dio_Ip_ClearPins(LED_RED_PORT, (1 << LED_RED_PIN));
						Siul2_Dio_Ip_SetPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
						Siul2_Dio_Ip_ClearPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
						get_ts_egress_data(&srEgressTimeStamp, seq_id);
					}
					else if(ether_frame->data[offset_vlan] == 0x1A){
						/*Respdelay follow up*/
						Siul2_Dio_Ip_ClearPins(LED_RED_PORT, (1 << LED_RED_PIN));
						Siul2_Dio_Ip_ClearPins(LED_GREEN_PORT, (1 << LED_GREEN_PIN));
						Siul2_Dio_Ip_SetPins(LED_BLUE_PORT, (1 << LED_BLUE_PIN));
					}
					EthIf_TxConfirmation(CFG_PHY_CTRL_IDX, index, trasmit_status, srEgressTimeStamp);
				}
				bufferQueue[index].inUse = FALSE;
			}

	}

}

Gmac_Ip_StatusType send_eth_frame_lld(Gmac_Ip_BufferType* eth_message){
	#ifdef DEBUG_PRINT
		printf("Im about to send! \r\n");
	#endif
	Gmac_Ip_StatusType eERROR = GMAC_STATUS_SUCCESS;

	Gmac_Ip_BufferType TxBuffer = {0};
	Gmac_Ip_TxOptionsType TxOptions = {FALSE, GMAC_CRC_AND_PAD_INSERTION, GMAC_CHECKSUM_INSERTION_DISABLE};

	uint8 MacAddr[6U] = {0U};

	Gmac_Ip_GetMacAddr(INST_GMAC_0, MacAddr);

	/*request a buffer of at least 64 bytes*/
	TxBuffer.Length = eth_message->Length;

	eERROR = Gmac_Ip_GetTxBuff(INST_GMAC_0, 0u, &TxBuffer, NULL_PTR);
	if(GMAC_STATUS_SUCCESS == eERROR && TxBuffer.Data != NULL && TxBuffer.Length >= eth_message->Length){

	memcpy(TxBuffer.Data, eth_message->Data, eth_message->Length);
	TxBuffer.Length = eth_message->Length;

	/* Send the ETH frame */
	/*true function that sends data to the transceiver*/
	eERROR = Gmac_Ip_SendFrame(INST_GMAC_0, 0U, &TxBuffer, &TxOptions);

	while (GMAC_STATUS_TX_QUEUE_FULL == eERROR)
		{
			eERROR = Gmac_Ip_SendFrame(INST_GMAC_0, 0U, &TxBuffer, &TxOptions);
		}
		if(GMAC_STATUS_SUCCESS == eERROR){
			/*add buffer in bufferQueue to be free after sending completion*/
			DescrBuffer newBuffItem = { .Data = TxBuffer.Data, .Length = TxBuffer.Length, .ring = 0U, .inUse = TRUE};
			for(uint8 index = 0; index < MAX_TX_PENDING; index++){
				if(!bufferQueue[index].inUse){
					bufferQueue[index] = newBuffItem;
					break;
				}
			}
		}
	}
	else{
		//printf("send operation failed! NO empty buffer are available! \r\n");
	}

	return eERROR;

}
void get_ltc_counter(Gmac_Ip_TimestampType* TimeStamp){
	uint16_t reg_0_15_ns, reg_16_29_ns, reg_0_15_s, reg_16_31_s, read_ltc;

	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, LTC_LOAD_CTRL, &read_ltc, 100);
	read_ltc |= 0x0004;
	Gmac_Ip_MDIOWriteMMD(0, PHYAD, MMD30, LTC_LOAD_CTRL, read_ltc, 100);

	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, LTC_RD_DATA_0, &reg_0_15_ns, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, LTC_RD_DATA_1, &reg_16_29_ns, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, LTC_RD_DATA_2, &reg_0_15_s, 100);
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, LTC_RD_DATA_3, &reg_16_31_s, 100);

	TimeStamp->nanoseconds = 0x00000000 | ((uint32) reg_0_15_ns) | ((uint32)reg_16_29_ns << 16);
	TimeStamp->seconds = 0x00000000 | ((uint32)reg_16_31_s << 16) | ((uint32)reg_0_15_s);

}

void get_ts_ingress_data(Gmac_Ip_TimestampType* srIngressTimeStamp, uint16 seq_id){

	uint16_t ingr_seq_id;
	uint16_t reg_0, reg_1, reg_2, reg_3, reg_4, reg_5, reg_interrupt;
	uint8_t time = 0;

	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, INGR_TS_1, &reg_1, 100);/*sequence id of ring buffer*/
	/*check if TS has been overwritten*/
	Gmac_Ip_MDIOReadMMD(0,	PHYAD, MMD30, PTP_IRQ_SOURCE, &reg_interrupt, 100);
	if(MASK_ING_LOST(reg_interrupt) == 1){
		printf("Ing TS overwritten! \r\n");
		//Gmac_Ip_MDIOWriteMMD(0,	PHYAD, MMD30, ING_RING_DONE, 0X0001, 100);
	}
	//to clear the ring buffer position
	while(reg_1 != seq_id && time < 4){
				//clear the buffer until message is the correct one
				Gmac_Ip_MDIOWriteMMD(0,	PHYAD, MMD30, ING_RING_DONE, 0X0001, 100);
				Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, INGR_TS_1, &reg_1, 100);//sequence id of ring buffer
				printf("clearing RX\r\n");
				time++;
	}
	if(reg_1 != seq_id){
		printf("Timestamping loss! \r\n");
	}
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, INGR_TS_0, &reg_0, 100);/*[7:0] domain number of ts ring buffer + [11:8] message type + [14:12]-[4:2]s*/
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, INGR_TS_2, &reg_2, 100);/*[15:0] ns*/
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, INGR_TS_3, &reg_3, 100);/*[13:0]-[29:16]ns + [15:14][1:0] s*/
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, INGR_TS_4, &reg_4, 100);/*[31:16] subns*/
	Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, INGR_TS_5, &reg_5, 100);/*[3:0] - [15:12] subns + [15:4] - [16:5] s*/
	Gmac_Ip_MDIOWriteMMD(0,	PHYAD, MMD30, ING_RING_DONE, 0X0001, 100);
	/*check for valid timestamp*/
	if((reg_0 & 0x8000)>>15 == 1){
		srIngressTimeStamp->nanoseconds = 0x00000000 | ((uint32) (reg_3 & 0x3FFF)<<16) | ((uint32)reg_2);
		srIngressTimeStamp->seconds = 0x00000000 | (reg_5 & 0xFFF0)<<1 | ((reg_0 & 0x7000)>>10) | ((reg_3 & 0xC000)>>14);
		//printf("T2: %u s \r\n", srIngressTimeStamp->seconds, srIngressTimeStamp->nanoseconds);
	}
	else{
			printf("not valid RX TS!\r\n");
		}
	//printf("seq_id in :%u \r\n", reg_1);


}

void get_ts_egress_data(Gmac_Ip_TimestampType* srEgressTimeStamp, uint16 seq_id){

	uint16_t egr_seq_id;
		uint16_t reg_0, reg_1, reg_2, reg_3, reg_4, reg_5, reg_interrupt;
		uint8_t time = 0;

		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, EGR_TS_1, &reg_1, 100);/*sequence id of ring buffer*/
		/*check if TS has been overwritten*/
		Gmac_Ip_MDIOReadMMD(0,	PHYAD, MMD30, PTP_IRQ_SOURCE, &reg_interrupt, 100);
		if(MASK_EGR_LOST(reg_interrupt) == 1){
			printf("Egr TS overwritten! \r\n");
			//Gmac_Ip_MDIOWriteMMD(0,	PHYAD, MMD30, ING_RING_DONE, 0X0001, 100);
		}
		//to clear the ring buffer position
		while(reg_1 != seq_id && time < 4){
			//clear the buffer until message is the correct one
			Gmac_Ip_MDIOWriteMMD(0,	PHYAD, MMD30, EGR_RING_DONE, 0X0001, 100);
			Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, EGR_TS_1, &reg_1, 100);//sequence id of ring buffer
			printf("clearing TX\r\n");
			time++;
		}
		if(reg_1 != seq_id){
			printf("Timestamping loss! \r\n");
		}
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, EGR_TS_0, &reg_0, 100);/*[7:0] domain number of ts ring buffer + [11:8] message type + [14:12]-[4:2]s*/
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, EGR_TS_2, &reg_2, 100);/*[15:0] ns*/
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, EGR_TS_3, &reg_3, 100);/*[13:0]-[29:16]ns + [15:14][1:0] s*/
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, EGR_TS_4, &reg_4, 100);/*[31:16] subns*/
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, EGR_TS_5, &reg_5, 100);/*[3:0] - [15:12] subns + [15:4] - [16:5] s*/
		Gmac_Ip_MDIOWriteMMD(0,	PHYAD, MMD30, EGR_RING_DONE, 0X0001, 100);
		/*check for valid timestamp*/
		if((reg_0 & 0x8000)>>15 == 1){
			srEgressTimeStamp->nanoseconds = 0x00000000 | ((uint32) (reg_3 & 0x3FFF)<<16) | ((uint32)reg_2);
			srEgressTimeStamp->seconds = 0x00000000 | (reg_5 & 0xFFF0)<<1 | ((reg_0 & 0x7000)>>10) | ((reg_3 & 0xC000)>>14);
			//printf("T3: %u s \r\n", srEgressTimeStamp->seconds, srEgressTimeStamp->nanoseconds);
		}
		else{
				printf("not valid TX TS!\r\n");
			}
		Gmac_Ip_MDIOReadMMD(0, PHYAD, MMD30, 0x1148U, &reg_0, 100);/*read timestmap filter*/
		//print_16(&reg_0);
		//printf("seq_id in :%u \r\n", reg_1);

}

void print_16(uint16_t *data){
	uint16_t shift = *data;
	for(int i = sizeof(uint16_t)*8-1; i>=0; i--){
		printf("%"PRIu16,((shift>>i) & 0x1));
	}
	printf("\r\n");
}

void print_32(uint32_t *data){
	uint32_t shift = *data;
	for(int i = sizeof(uint32_t)*8-1; i>=0; i--){
		printf("%"PRIu32,((shift>>i) & 0x1));
	}
	printf("\r\n");
}

void print_64(uint64_t *data){
	uint64_t shift = *data;
	for(int i = sizeof(uint64_t)*8-1; i>=0; i--){
		printf("%"PRIu64,((shift>>i) & 0x1));
	}
	printf("\r\n");
}
