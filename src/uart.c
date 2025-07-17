#include "uart.h"
#include <string.h>
#include <stdlib.h>
#include "Lpuart_Uart_Ip_HwAccess.h"

//extern UART_Common_IRQHnadler(void);
extern LPUART_UART_IP_2_IRQHandler(void);
//extern UART_CH_RX_IRQHandler(void);

void UART_init(){

    /*Lpuart_Uart_Ip_StatusType Status_Init_UART = LPUART_UART_IP_STATUS_ERROR;

    Status_Init_UART = */
	Lpuart_Uart_Ip_Init(LPUART_UART_IP_INSTANCE_USING_2, &Lpuart_Uart_Ip_xHwConfigPB_2_BOARD_INITPERIPHERALS);

    /*if(Status_Init_UART != LPUART_UART_IP_STATUS_SUCCESS){
        return Status_Init_UART;
    }*/


    IntCtrl_Ip_InstallHandler(LPUART2_IRQn, LPUART_UART_IP_2_IRQHandler, NULL_PTR);
    IntCtrl_Ip_SetPriority(LPUART2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    IntCtrl_Ip_EnableIrq(LPUART2_IRQn);

    //return Status_Init_UART;
}
