#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C"{
#endif


#include "FreeRTOS.h"
#include "semphr.h"
#include "Gmac_Ip.h"
#include "FlexCAN_Ip.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "Lpuart_Uart_Ip.h"
#include "IntCtrl_Ip.h"

void UART_init();

#ifdef __cplusplus
}
#endif

#endif
