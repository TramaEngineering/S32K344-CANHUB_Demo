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

/*gptp libraries*/
#include "Devassert.h"
#include "gptp.h"
#include "gptp_port_platform.h"
#include "s32k344_gptp_config.h"
#include "Clock_Ip.h"

#include "EthTrcv.h"
#include "EthIf_Cbk.h"
#include "gptp_cbk.h"

//define a vector of can queues (RTOS)
#define CAN_COUNT 6
#define MILLISECOND_IN_NS               (1000000U)
#define GMAC_MAX_CTRLIDX_SUPPORTED 		1U
// NO_FRRERTOS defined in bsp.h

QueueHandle_t eth_can_queues[CAN_COUNT];
/*message queue to be filled with the message to be sent when button is pressed*/
QueueHandle_t tx_queue_send;
Gmac_Ip_TimestampType timestamp;
Gmac_Ip_TimestampType old_timestamp;
uint8 polling;

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

rgb_status error_st = RED;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void EEPROM_Init(void);
static void EEPROM_Enqueue_Write(const uint8_t cu8Port,
                                 const uint8_t cu8Offset,
                                 const uint32_t cu32Length,
                                 const uint8_t *cpu8Data);
static void EEPROM_Write_Poll(void);
static uint8_t EEPROM_Read(const uint8_t cu8Port,
                           const uint8_t cu8Offset,
                           const uint32_t cu32Length,
                           uint8_t *pu8Data);
static void EEPROM_Poll(void);
static void Eth_Poll(void);
static void Eth_PollLinkStatus(void);

void HardFault_Handler(void)
{
	/* Hard fault occured */
	set_rgb_status(error_st);
	while(TRUE){};
}

/*******************************************************************************
 * Local functions
 ******************************************************************************/

void button_sw1(void)
{
	/* Send CAN Frame to the CAN0 device */
	//set_rgb_status(INITIALIZE);
	/*if( xQueueSendFromISR( eth_can_queues[0],
			( void * ) &buttonCanFrame,	NULL))
	{
		// Failed queue CAN frame drop packet
	}*/
}

void button_sw2_ethernet(void)
{

	//if(xQueueSendFromISR(tx_queue_send, &pDelayResp, NULL)){
			/*the thread has fail to send the message after 10 tick so there will be some error*/
			/*put led blinking on a certain way*/
		//}
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
	Siul2_Dio_Ip_TogglePins(TP31_PORT, 1<<TP31_PIN);
	//old_timestamp = timestamp;
	//get_current_time(&timestamp);
	/* toggle LED1 */
	Task_Flag_Cnt++;

	//ENET_PPS_CLK_GEN(); /* to generate 500Hz clock output */
	static volatile uint64_t u64PitIsrCountMs = 0;

	/* Increment 1ms = 1000000 ns. */
	//GPTP_PORT_IncFreeRunningTimer(MILLISECOND_IN_NS);

	//	Task_Flag_1mS = 1;
	//	if((Task_Flag_Cnt % 10) == 0)
	//		Task_Flag_10mS = 1;
	//	if((Task_Flag_Cnt % 2) == 0)
	//			Task_Flag_2mS = 1;
	//	if((Task_Flag_Cnt % 100) == 0)
	//		Task_Flag_100mS = 1;
	//	if((Task_Flag_Cnt % 200) == 0)
	//			Task_Flag_200mS = 1;
	//	if((Task_Flag_Cnt % 1000) == 0){
	//		//Siul2_Dio_Ip_TogglePins(LED2_PORT, 1<<LED2_PIN);
	//		Task_Flag_1000mS = 1;
	//		Task_Flag_Cnt = 0;
	//	}
	//	u64PitIsrCountMs++;

		Task_Flag_50uS = 1;
		if((Task_Flag_Cnt % 2) == 0)
			Task_Flag_100uS = 1;
		if((Task_Flag_Cnt % 4) == 0)
				Task_Flag_200uS = 1;
		if((Task_Flag_Cnt % 20) == 0){
			Task_Flag_1mS = 1;
			u64PitIsrCountMs++;

			//ENET_PPS_CLK_GEN(); /* to generate 500Hz clock output */

			/* Increment 1ms = 1000000 ns. */
			GPTP_PORT_IncFreeRunningTimer(MILLISECOND_IN_NS);
		}
		if((Task_Flag_Cnt % 200) == 0)
			Task_Flag_10mS = 1;
		if((Task_Flag_Cnt % 40) == 0)
				Task_Flag_2mS = 1;
		if((Task_Flag_Cnt % 2000) == 0)
			Task_Flag_100mS = 1;
		if((Task_Flag_Cnt % 4000) == 0)
				Task_Flag_200mS = 1;
		if((Task_Flag_Cnt % 20000) == 0){
			//Siul2_Dio_Ip_TogglePins(LED2_PORT, 1<<LED2_PIN);
			Task_Flag_1000mS = 1;
			Task_Flag_Cnt = 0;
		}

	return;
}

///*******************************************************************************
// * EEPROM FUNCTIONS
// ******************************************************************************/
//
///*!
// * @brief           This function initializes all components necessary for
// *                  simulated EEPROM in data flash.
// *
// * @details         This function prepares cache data entries, enables MemmAcc
// *                  and Fee.
//*/
static void EEPROM_Init(void)
{
//    uint32_t u32PortCacheEntry;
//    uint32_t u32PortDataEntry;
//
//    /* Prepare cache default values. */
//    for (u32PortCacheEntry = 0u; u32PortCacheEntry < (PORT_COUNT * PORT_DATA_ENTRY_COUNT); u32PortCacheEntry++)
//    {
//        sarPerPortData[u32PortCacheEntry].eState = WRITE_COMPLETE;
//        sarPerPortData[u32PortCacheEntry].u8Offset = PORT_DATA_ENTRY_SIZE * u32PortCacheEntry;
//
//        for (u32PortDataEntry = 0u; u32PortDataEntry < PORT_DATA_ENTRY_SIZE; u32PortDataEntry++)
//        {
//            sarPerPortData[u32PortCacheEntry].au8Data[u32PortDataEntry] = 0u;
//        }
//    }
//
//    /* Init MemAcc. */
//    MemAcc_Init(NULL_PTR);
//
//    /* Init Fee. */
//    Fee_Init(NULL_PTR);
//
//    /* Perform init Fee driver. */
//    do
//    {
//        EEPROM_Poll();
//    } while (MEMIF_IDLE != seMemStatus);
}
//
///*!
// * @brief           This function queues up data to be written to EEPROM.
// *
// * @param[in]       cu8Port Port ID.
// * @param[in]       cu8Offset EEPROM address offset.
// * @param[in]       cu32Length Data length.
// * @param[in]       cpu8Data Data to be enqueued.
//*/
static void EEPROM_Enqueue_Write(const uint8_t cu8Port,
                                 const uint8_t cu8Offset,
                                 const uint32_t cu32Length,
                                 const uint8_t *cpu8Data)
{
//    const uint32_t cu32Idx = (PORT_DATA_ENTRY_COUNT * cu8Port) + cu8Offset;
//    uint32_t       u32Entry;
//
//    if ((cu32Length <= PORT_DATA_ENTRY_SIZE) &&
//        (cu32Idx < (PORT_COUNT * PORT_DATA_ENTRY_COUNT)))
//    {
//        for (u32Entry = 0u; u32Entry < cu32Length; u32Entry++)
//        {
//            sarPerPortData[cu32Idx].au8Data[u32Entry] = cpu8Data[u32Entry];
//        }
//        sarPerPortData[cu32Idx].eState = WRITE_PENDING;
//    }
}
//
///*!
// * @brief           Executes actual write to EEPROM.
// *
// * @details         This function goes through the cache entry and executes
// *                  write above the first entry which has a WRITE_PENING state.
//*/
static void EEPROM_Write_Poll(void)
{
//    boolean  bIsWriteInProgress = false;
//    uint32_t u32PortCacheEntry;
//    uint16_t u16BlockNumber;
//
//    EEPROM_Poll();
//    if (MEMIF_IDLE == seMemStatus)
//    {
//        /* Clear status of previous write. */
//        for (u32PortCacheEntry = 0u; u32PortCacheEntry < (PORT_COUNT * PORT_DATA_ENTRY_COUNT); u32PortCacheEntry++)
//        {
//            if (WRITE_IN_PROGRESS == sarPerPortData[u32PortCacheEntry].eState)
//            {
//                sarPerPortData[u32PortCacheEntry].eState = WRITE_COMPLETE;
//            }
//        }
//        /* Write data from cache entry. */
//        for (u32PortCacheEntry = 0u; u32PortCacheEntry < (PORT_COUNT * PORT_DATA_ENTRY_COUNT); u32PortCacheEntry++)
//        {
//            if ((WRITE_PENDING == sarPerPortData[u32PortCacheEntry].eState) &&
//                !bIsWriteInProgress)
//            {
//                bIsWriteInProgress = true;
//                /* In case of necessity handle error. */
//                u16BlockNumber = sarPerPortData[u32PortCacheEntry].u8Offset == PDELAY_IDX_OFFSET ? FeeConf_FeeBlockConfiguration_FeeBlockConfiguration_0 : FeeConf_FeeBlockConfiguration_FeeBlockConfiguration_1;
//                Fee_Write(u16BlockNumber, sarPerPortData[u32PortCacheEntry].au8Data);
//                sarPerPortData[u32PortCacheEntry].eState = WRITE_IN_PROGRESS;
//            }
//        }
//    }
}
//
///*!
// * @brief           This function reads data from EEPROM.
// *
// * @param[in]       cu8Port Port ID.
// * @param[in]       cu8Offset EEPROM address offset.
// * @param[in]       cu32Length Data length.
// * @param[out]      pu8Data Data to be read.
// *
// * @return          0 on success.
//*/
static uint8_t EEPROM_Read(const uint8_t cu8Port,
                           const uint8_t cu8Offset,
                           const uint32_t cu32Length,
                           uint8_t *pu8Data)
{
    uint8_t        u8Ret = 0u;
//    const uint32_t cu32Idx = (PORT_DATA_ENTRY_COUNT * cu8Port) + cu8Offset;
//    uint32_t       u32Entry;
//    uint16_t       u16BlockNumber;
//
//    if (cu32Idx >= (PORT_COUNT * PORT_DATA_ENTRY_COUNT))
//    {
//        u8Ret = 1u;
//    }
//    else
//    {
//        /* If data is about to be written or is being written at the moment,
//           get the value from cache. */
//        if ((WRITE_IN_PROGRESS == sarPerPortData[cu32Idx].eState) ||
//            (WRITE_PENDING == sarPerPortData[cu32Idx].eState))
//        {
//            for (u32Entry = 0u; u32Entry < cu32Length; u32Entry++)
//            {
//                pu8Data[u32Entry] = sarPerPortData[cu32Idx].au8Data[u32Entry];
//            }
//        }
//        else
//        {
//            u16BlockNumber = sarPerPortData[cu32Idx].u8Offset == PDELAY_IDX_OFFSET ? FeeConf_FeeBlockConfiguration_FeeBlockConfiguration_0 : FeeConf_FeeBlockConfiguration_FeeBlockConfiguration_1;
//
//            /* Read data block. */
//            Fee_Read(u16BlockNumber, 0u, pu8Data, cu32Length);
//
//            /* Perform read data form Block 0. */
//            do
//            {
//                EEPROM_Poll();
//            } while (MEMIF_IDLE != seMemStatus);
//        }
//    }

    return u8Ret;
}
//
///*!
// * @brief           Executes actual EEPROM command.
// *
// * @details         This function call Fls/Fee main functions to execute
// *                  necessary actions.
//*/
static void EEPROM_Poll(void)
{
//    Fee_MainFunction();
//    MemAcc_MainFunction();
//    seMemStatus = Fee_GetStatus();
}

//
///*!
// * @brief           This function configures PPS output.
// *
// * @details         This function configures PPS output parameters.
// *
// * @param[in]       u8PPSCtrl Configuration of PPSCTRL field of MAC_PPS_CONTROL
// *                  register:
// *                      0 - 1Hz, short 20ns pulse
// *                      1 - 1Hz, 50% duty cycle
// *                      2 - 2Hz, 50% duty cycle
// *                      3 - 4Hz, 50% duty cycle
// *                      4 - 8Hz, 50% duty cycle
// *                      .....
// *                      15 - 32.768 kHz, 50% duty cycle
//*/
//static void Eth_Configure_1PPS_Output(uint8_t u8PPSCtrl)
//{
//    /* Clear the current PPSCTRL setting. */
//    Gmac_apxBases[0]->MAC_PPS_CONTROL &= ~GMAC_MAC_PPS_CONTROL_PPSCTRL_PPSCMD_MASK;
//     /* Set 1PPS output to 1Hz and 50% duty cycle. */
//    Gmac_apxBases[0]->MAC_PPS_CONTROL |= GMAC_MAC_PPS_CONTROL_PPSCTRL_PPSCMD(u8PPSCtrl);
//}
//
///*!
// * @brief           Callback function for EEPROM write.
//*/
uint8_t EEPROM_Write_CB(uint8_t u8PdelayMachine,
                        gptp_def_nvm_data_t eNvmDataType,
                        float64_t f64Value,
                        gptp_def_mem_write_stat *peWriteStat)
{
//    uint8_t u8Offset = eNvmDataType == GPTP_DEF_NVM_PDELAY ? PDELAY_IDX_OFFSET : RRATIO_IDX_OFFSET;
//    uint8_t au8Data[PORT_DATA_ENTRY_SIZE];
//    uint8_t u8Idx;
//
//    if (GPTP_DEF_MEM_WRITE_INIT == *peWriteStat)
//    {
//        for (u8Idx = 0u; u8Idx < PORT_DATA_ENTRY_SIZE; u8Idx++)
//        {
//           au8Data[u8Idx] = ((uint8_t*)&f64Value)[u8Idx];
//        }
//
//        EEPROM_Enqueue_Write(u8PdelayMachine, u8Offset, PORT_DATA_ENTRY_SIZE,
//                             au8Data);
//
//        *peWriteStat = GPTP_DEF_MEM_WRITE_FINISH;
//    }
//
    return 0u;
}
//
///*!
// * @brief           Callback function for EEPROM read.
//*/
uint8_t EEPROM_Read_CB(uint8_t u8PdelayMachine,
                       gptp_def_nvm_data_t eNvmDataType,
                       float64_t *f64Value)
{
//    uint8_t u8Status;
//    uint8_t u8Offset = eNvmDataType == GPTP_DEF_NVM_PDELAY ? PDELAY_IDX_OFFSET : RRATIO_IDX_OFFSET;
//    uint8_t au8Data[PORT_DATA_ENTRY_SIZE];
//    uint8_t u8Idx;
//
//    u8Status = EEPROM_Read(u8PdelayMachine, u8Offset, PORT_DATA_ENTRY_SIZE,
//                           au8Data);
//
//    for (u8Idx = 0u; u8Idx < PORT_DATA_ENTRY_SIZE; u8Idx++)
//    {
//       ((uint8_t*)f64Value)[u8Idx] = au8Data[u8Idx];
//    }
//
//   return u8Status;
}

static void Eth_PollLinkStatus(void)
{
    uint8_t                      u8CtrlIdx;
    /* Current link status. */
    static EthTrcv_LinkStateType_g seLinkState;
    /* Last link status for change detection. */
    static EthTrcv_LinkStateType_g seLastLinkState[GMAC_MAX_CTRLIDX_SUPPORTED] = {ETHTRCV_LINK_ST_DOWN};

    for (u8CtrlIdx = 0u; u8CtrlIdx < GMAC_MAX_CTRLIDX_SUPPORTED; u8CtrlIdx++) {
    				//implemented in GMAC_Task
        if (E_OK == EthTrcv_GetLinkState(u8CtrlIdx, &seLinkState))
        {
            /* Detect link status change. */
            if (seLastLinkState[u8CtrlIdx] != seLinkState)
            {
                seLastLinkState[u8CtrlIdx] = seLinkState;
                /* Notify via EthIf API. */
                EthIf_TrcvLinkStateChg(u8CtrlIdx, seLinkState);
            }
        }
    }
}

static void Eth_Poll(void)
{
    uint8            u8FifoIdx;
    Eth_RxStatusType rRxStatus;

    /*for (u8FifoIdx = 0u; u8FifoIdx < ETH_43_GMAC_MAX_RXFIFO_SUPPORTED; u8FifoIdx++)
    {
        Eth_43_GMAC_Receive(EthConf_EthCtrlConfig_EthCtrlConfig_0, u8FifoIdx,
                            &rRxStatus);
    }

    Eth_43_GMAC_TxConfirmation(EthConf_EthCtrlConfig_EthCtrlConfig_0);*/
    if(polling != 1){
    	polling = 1;
		eth_rx_check();
		enet_tx_free_buffer();
		polling = 0;
    }
    else{
    	printf("interrupt!\r\n");
    }

}

/**
 * @brief        Main function of the example
 * @details      Initializes the used drivers and uses 1 binary Semaphore and
 *               2 tasks to toggle a LED.
 */

int main(void)
{
	uint8 timer0, timer1, annouce = 0;
	Gmac_Ip_TimestampType srEgressTimeStamp;
	uint16 seq_id;
	static uint16 message_index = 0;
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

#ifdef NO_FREERTOS
		IntCtrl_Ip_Init(&IntCtrlConfig_0);
#endif

	/*set handler for interrupt*/
	IntCtrl_Ip_InstallHandler(SIUL_0_IRQn, SIUL2_EXT_IRQ_0_7_ISR, NULL_PTR);
	/*set priority handler for the external input*/
	IntCtrl_Ip_SetPriority(SIUL_0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	/*enable IRQ for switch 1*/
	IntCtrl_Ip_EnableIrq(SIUL_0_IRQn);

	IntCtrl_Ip_InstallHandler(SIUL_3_IRQn, SIUL2_EXT_IRQ_24_31_ISR, NULL_PTR);
	IntCtrl_Ip_SetPriority(SIUL_3_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	IntCtrl_Ip_EnableIrq(SIUL_3_IRQn);

	IntCtrl_Ip_InstallHandler(PIT0_IRQn, PIT_0_ISR, NULL_PTR);
	IntCtrl_Ip_SetPriority(PIT0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	IntCtrl_Ip_EnableIrq(PIT0_IRQn);

	/*PIT initialization*/
	Pit_Ip_Init(PIT_0_IP_INSTANCE_NUMBER, &PIT_0_InitConfig_PB);
	/*PIT channel initialization*/
	Pit_Ip_InitChannel(PIT_0_IP_INSTANCE_NUMBER, &PIT_0_ChannelConfig_PB[0U]);
	//	/* Start PIT0 channel0, PIT0 clocks from AIPS_SLOW_CLK 40MHz, so here the timeout 40000 means 1mS */
	//	Pit_Ip_StartChannel(PIT_0_IP_INSTANCE_NUMBER, 0, 40000);
	/* Start PIT0 channel0, PIT0 clocks from AIPS_SLOW_CLK 40MHz, so here the timeout 2000 means 50uS */
	Pit_Ip_StartChannel(PIT_0_IP_INSTANCE_NUMBER, 0, 2000);
	/*enable channel interrupt*/
	Pit_Ip_EnableChannelInterrupt(PIT_0_IP_INSTANCE_NUMBER, 0);
	/*load the interrupt configuration*/


	/* Initialize ethernet MAC */
	/*set the MAC to slave mode and not to master mode*/
	Gmac_Ip_StatusType Status_Init_Gmac = GMAC_STATUS_ERROR;

#ifndef NO_FREERTOS
	Status_Init_Gmac = enet_init_freertos(&tx_queue_send);
#else
	Status_Init_Gmac = enet_init();
#endif

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
			set_rgb_status(ERROR);
			while(1); /* Error during initialization. */
		}
	}

	gptp_err_type_t err = GPTP_GptpInit(&rGptpCfgParams);
	DevAssert(GPTP_ERR_OK == err);

	set_rgb_status(NOMINAL);

#ifndef NO_FREERTOS
	/*start link check task*/
	start_link_check();

	/* Start Listening to ethernet packets */
	enet_start_rx(eth_can_queues, CAN_COUNT);

	/*create a thread that pools on a message queue and send the message when it receive one*/
	enet_start_tx();

	/* Start FreeRTOS */
	vTaskStartScheduler();

	/* Scheduler returned this an error was encountered */
	set_rgb_status(ERROR);
#endif

	//printf("Error in code!\r\n");

	for( ;; ){
#ifdef NO_FREERTOS
		timer0 = Task_Flag_Cnt;
				if(annouce == 0){
					//send_eth_frame_lld(&pDelayReq);
					send_eth_frame_lld(&arpAnnouce);
					annouce = 1;
				}
				if(Task_Flag_200uS){
					Task_Flag_200uS = 0;
					Eth_Poll();
				}
				if(Task_Flag_2mS){
					Task_Flag_2mS = 0;
					Eth_PollLinkStatus();
					for(int i=0; i<128; i+=2)
						udpFrame128[14+20+8+i] = (uint8_t)(message_index & 0xFFFF);
					message_index++;
					send_eth_frame_lld(&customMessage_UDP_128);
					//Eth_Poll();
					//if(timestamp.seconds < old_timestamp.seconds)
						//printf("old 2ms TS: %u s %u ns, new TS: %u s %u ns\r\n", old_timestamp.seconds,old_timestamp.seconds, timestamp.nanoseconds,timestamp.nanoseconds);
					/* Add task call for 1ms interval */

				}
				if(Task_Flag_10mS){
					Task_Flag_10mS = 0;
					//get_ts_ingress_data(&srEgressTimeStamp, seq_id);
					//printf("difference 1ms %u s %u ns\r\n", timestamp.seconds/*-old_timestamp.seconds*/, timestamp.nanoseconds/*-old_timestamp.nanoseconds*/);
					GPTP_TimerPeriodic();
				}
				if(Task_Flag_1000mS){
					Task_Flag_1000mS = 0;
					//send_eth_frame_lld(&customMessage_ipv4);
				}


				timer1 = Task_Flag_Cnt;

#endif
	}


	return 0;
}

/** @} */
