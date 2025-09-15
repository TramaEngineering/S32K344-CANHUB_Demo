/**
*   @file       bsp.c
*
*   @addtogroup bsp Bsp_Module
*
*   @{
*/

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "bsp.h"

/*==================================================================================================
*                         LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*=================================================================================================
*                                       LOCAL MACROS
=================================================================================================*/
/* CPU freq is 160MHz, so systick 160 count is 1usec. It's used for software time delay. */
#define SYSTICK_CNT_PER_USEC	160
/* CPU freq is 160MHz, so systick 160000 count is 1msec. It's used for software time delay. */
#define SYSTICK_CNT_PER_MSEC	160000

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/
uint32 Qspi_Flash_TargetAddr = 0x00000000;
uint32 Qspi_Flash_WriteTestData[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint32 Qspi_Flash_ReadTestData[16];
uint8 Qspi_Flash_DeviceId[4];

volatile uint32 Qspi_Erase_WaitLoopCnt = 0;
volatile uint32 Qspi_Program_WaitLoopCnt = 0;

/*==================================================================================================
*                                      GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/
uint32_t Task_Flag_1mS = 0;
uint32_t Task_Flag_2mS = 0;
uint32_t Task_Flag_10mS = 0;
uint32_t Task_Flag_100mS = 0;
uint32_t Task_Flag_200mS = 0;
uint32_t Task_Flag_1000mS = 0;
uint32_t Task_Flag_Cnt = 0;

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                       External FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                       Interrupts callback FUNCTIONS
==================================================================================================*/

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
/*int fs26SpiTransferFunction(uint8_t *TxBuffer, uint8_t *RxBuffer, uint16_t Length)
{
	Lpspi_Ip_StatusType spiStat;

	spiStat = Lpspi_Ip_SyncTransmit(&Lpspi_Ip_DeviceAttributes_SpiExternalDevice_3_Instance_3_BOARD_InitPeripherals,
			TxBuffer, RxBuffer, Length, 1000);

	return (int)spiStat;
}*/

/*================================================================================================*/
/**
 * @brief          PIT0 CH0 Notification function
 *
 * @details        Notification function that is invoked when PIT0 CH0 timeout occurs.
 *
 * @param[in]      channel, the channel number that triggers the interrupt.
 *
 * @return         none
 */
/*void link_check(uint8 channel){
	(void) channel;

	printf("Hello i'm PIT\r\n");
}*/


uint32 Bsp_Init(void){
	uint32_t retCode = 0;

	/* Initialize Clock */
	OsIf_Init(NULL_PTR);

	/* Initialize all pins using the Port driver */
	Siul2_Port_Ip_PortStatusType Status_Init_Port = SIUL2_PORT_ERROR;
	retCode = Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals, g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);

	if(retCode != SIUL2_PORT_SUCCESS)
	{
		while(1); /* Error during initialization. */
	}

	/* Set RGB to indicate initialization */
	//set_rgb_status(INITIALIZE);

	Clock_Ip_StatusType Status_Init_Clock = CLOCK_IP_ERROR;
	retCode |= Clock_Ip_Init(Clock_Ip_aClockConfig);

	if(retCode != CLOCK_IP_SUCCESS)
	{
		//set_rgb_status(ERROR);
		while(1); /* Error during initialization. */
	}

	/* Initialize the FS26 to stop it from resetting */
	Lpspi_Ip_Init(&Lpspi_Ip_PhyUnitConfig_SpiPhyUnit_3_Instance_3);
	UART_init();
	/*ERROR IN START UP*/
	//fs26_initialize(&fs26SpiTransferFunction);

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
	Pit_Ip_StartChannel(PIT_0_IP_INSTANCE_NUMBER, 0, 40000);/*count 1ms*/
	/*enable channel interrupt*/
	Pit_Ip_EnableChannelInterrupt(PIT_0_IP_INSTANCE_NUMBER, 0);

	/*load the interrupt configuration*/
	retCode |= IntCtrl_Ip_Init(&IntCtrlConfig_0);
	/*route the handler to the interrupt*/
	/*embeded in previuos function*/
	//retCode |= IntCtrl_Ip_ConfigIrqRouting(&intRouteConfig);

	return retCode;
}

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/
/*================================================================================================*/
/**
 * @brief          Minisecond time delay using CPU systick timer, which is a 24bit count-down timer
 * @details        Poll the systick timer to delay a period of time.
 * @param[in]      nMsec, count of mini-second to delay
 * @return         none.
 */
void OsIf_Delay_Ms(uint32_t nMsec){
	uint32 cnt1, cnt2, i;
	for(i=0; i<nMsec; i++){
		cnt1 = OsIf_GetCounter(OSIF_COUNTER_SYSTEM);
		cnt1 = (cnt1<<8);
		do {
			cnt2 = OsIf_GetCounter(OSIF_COUNTER_SYSTEM);
			cnt2 = (cnt2<<8);
		} while(cnt1 - cnt2 < (SYSTICK_CNT_PER_MSEC<<8));
	}
	return;
}

/*================================================================================================*/
/**
 * @brief          Microsecond time delay using CPU systick timer, which is a 24bit count-down timer
 * @details        Poll the systick timer to delay a period of time.
 * @param[in]      nUsec, count of micro-second to delay
 * @return         none.
 */
void OsIf_Delay_Us(uint32_t nUsec){
	uint32 cnt1, cnt2, i;
	for(i=0; i<nUsec; i++){
		cnt1 = OsIf_GetCounter(OSIF_COUNTER_SYSTEM);
		cnt1 = (cnt1<<8);
		do {
			cnt2 = OsIf_GetCounter(OSIF_COUNTER_SYSTEM);
			cnt2 = (cnt2<<8);
		} while(cnt1 - cnt2 < (SYSTICK_CNT_PER_USEC<<8));
	}
	return;
}

#ifdef __cplusplus
}
#endif
