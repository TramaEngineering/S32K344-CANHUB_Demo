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
*   THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
*   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
*   IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
*   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
*   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
*   THE POSSIBILITY OF SUCH DAMAGE.
*
*   This file contains sample code only. It is not part of the production code deliverables.
*/

/**
*   @file    retarget.c
*   @version 1.0.0
*
*   @brief   Provides retargeting of C library printf/scanf functions via ITM / SWO / UART Trace
*   @details output debug print messages to ITM or UART.
*            How to implement this file to achieve UART printf.
*            1. call Console_SerialPort_Init() to initialize the UART. And include "retarget.h" where you call this function.
*            2. Include "stdio.h" where you call printf.
*            Note: Please choose Newlib for library.Float point is not support if you choose Newlib_Nano.
*            Date: 2021/3/15
*
*   This file contains sample code only. It is not part of the production code deliverables.
*
*   @addtogroup DEMO_COMPONENT
*   @{
*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
*                                        INCLUDE FILES
==================================================================================================*/
#include <stdint.h>
#include <stdio.h>
#include "retarget.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/
#if(RETARGRT == ITM)

// ******************************************************************
// Cortex-M SWO Trace / Debug registers used for accessing ITM
// ******************************************************************

// CoreDebug - Debug Exception and Monitor Control Register
#define DEMCR           (*((volatile uint32_t *) (0xE000EDFC)))
// DEMCR Trace Enable Bit
#define TRCENA          (1UL << 24)

// ITM Stimulus Port Access Registers
#define ITM_Port8(n)    (*((volatile uint8_t *) (0xE0000000 + 4 * n)))
#define ITM_Port16(n)   (*((volatile uint16_t *) (0xE0000000 + 4 * n)))
#define ITM_Port32(n)   (*((volatile uint32_t *) (0xE0000000 + 4 * n)))

// ITM Trace Control Register
#define ITM_TCR (*((volatile  uint32_t *) (0xE0000000 + 0xE80)))
// ITM TCR: ITM Enable bit
#define ITM_TCR_ITMENA (1UL << 0)

// ITM Trace Enable Register
#define ITM_TER (*((volatile  uint32_t *) (0xE0000000 + 0xE00)))
// ITM Stimulus Port #0 Enable bit
#define ITM_TER_PORT0ENA (1UL << 0)

// ******************************************************************
// Buffer used for pseudo-ITM reads from the host debugger
// ******************************************************************

// Value identifying ITM_RxBuffer is ready for next character
#define ITM_RXBUFFER_EMPTY    0x5AA55AA5
// variable to receive ITM input characters
volatile int32_t ITM_RxBuffer = ITM_RXBUFFER_EMPTY;

// Privilege register: registers that can be accessed by unprivileged code
#define ITM_TPR   (*((unsigned long *) 0xe0000e40))

// Lock Access register
#define ITM_LAR   (*((unsigned long *) 0xe0000fb0))

// unlock value
#define ITM_LAR_ACCESS  0xc5acce55

void ITM_init(void)
{
    ITM_LAR = ITM_LAR_ACCESS; // unlock
    ITM_TCR = 0x1;            // global enable for ITM
    ITM_TPR = 0x1;            // first 8 stim registers have unpriv access
    ITM_TER = 0xf;            // enable 4 stim ports
    DEMCR = TRCENA;           // global enable DWT and ITM
}

#else

#include "retarget.h"
#include "Lpuart_Uart_Ip.h"

static LPUART_Type * const Lpuart_Uart_Ip_userBases[LPUART_UART_IP_NUMBER_OF_INSTANCES] = IP_LPUART_BASE_PTRS;

#endif

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/

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



/**
 * @brief          Initialize the LPUART instance for debug print
 * @details        Initialize the LPUART instance for debug print
 *
 * @param[in]      none.
 *
 * @return         none.
 */
void Console_SerialPort_Init(void)
{
#if(RETARGRT == ITM)
	/* initialize the ITM as the console serial communication port */
	ITM_init();
#else
	/*init the systick to use in timeout counting in func Lpuart_Uart_Ip_SyncSend if you chose system timer.
	 * If dummy timer is chosen,not necessary to call OsIf_Init.*/
	/* OsIf_Init(NULL_PTR); */
	/* initialize the UART as the console serial communication port */
	Lpuart_Uart_Ip_Init(CONSOLE_UART_INST,CONSOLE_UART_CONFIG);

#endif
}

/**
 * @brief          UART "write" function implementation
 * @details        EWL C Library function : __write_console
 *                 Newlib C library function : _write
 *                 Function called by lower level of printf routine within C library.
 *                 With the default semihosting stub, this would write the character(s) to the
 *                 debugger console window (which acts as stdout. It can write the character(s)
 *                 from the Cortex M3/M4 SWO / ITM interface for display in the ITM Console,
 *                 or write the char to LPUART interface.
 *
 * @param[in]      iFileHandle.
 * @pcBuffer[in]   pointer to the buffer that is to be printed.
 * @iLength[in]    the message length (in bytes) to print.
 *
 * @return         the number of bytes that are written to print target.
 */
#if defined (__NEWLIB__)
int _write(int iFileHandle, char *pcBuffer, int iLength) {
#elif defined (__EWL__)
int	__write_console(__std(__file_handle) iFileHandle, unsigned char *pcBuffer, __std(size_t) *count) {
#endif

#if(RETARGRT == ITM)
    int32_t num = 0;
#endif

#if defined (__EWL__)
    int iLength = *count;
#endif

    //check that iFileHandle == 1 to confirm that write is to stdout
    if (iFileHandle != 1) {
        return 0;
    }

#if(RETARGRT == ITM)
    // check if debugger connected and ITM channel enabled for tracing
    if ((DEMCR & TRCENA) &&
    // ITM enabled
            (ITM_TCR & ITM_TCR_ITMENA) &&
            // ITM Port #0 enabled
            (ITM_TER & ITM_TER_PORT0ENA)) {

        while (num < iLength) {
            while (ITM_Port32(0) == 0) {
            }
            ITM_Port8(0) = pcBuffer[num++];
        }
        return 0;
    } else
        // Function returns number of unwritten bytes if error
        return (iLength);
#else
    //UART_TIMEOUT_US
   if(LPUART_UART_IP_STATUS_SUCCESS == Lpuart_Uart_Ip_SyncSend(CONSOLE_UART_INST, (unsigned char *)pcBuffer, iLength, UART_TIMEOUT_US))
   {
	   /* wait till the UART transmission is completed, otherwise functional reset will cause the last char is not printed correctly. */
	   //LPUART_Type * Base = Lpuart_Uart_Ip_userBases[CONSOLE_UART_INST];
	   	//while((Base->STAT & LPUART_STAT_TC_MASK) == 0) {}
	   return 0;
   }
   else
    return iLength;
#endif
}

#ifdef __cplusplus
}
#endif

/** @} */
