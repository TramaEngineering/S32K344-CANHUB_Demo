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
*   @file    retarget.h
*   @version 1.0.0
*
*   @brief   it's for uart print or ITM print.
*
*   This file contains sample code only. It is not part of the production code deliverables.
*
*   @addtogroup DEMO_COMPONENT
*   @{
*/

#ifndef RETARGET__H_
#define RETARGET_H_

/*==================================================================================================
*                                        INCLUDE FILES
==================================================================================================*/
#include <stdarg.h>
#include <stdio.h>

/*==================================================================================================
*                                          CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                      DEFINES AND MACROS
==================================================================================================*/
#define ITM   0
#define UART  1

#define RETARGRT   UART  /* select the re-target peripheral */
//#define RETARGRT   ITM  /* select the re-target peripheral */

#define S32K3_WB

#define UART_TIMEOUT_US		20000

/*select the UART PAL instance and configuration for the console re-target implementation */
#ifdef S32K3_WB
#define CONSOLE_UART_INST  		2U
#define CONSOLE_UART_CONFIG   	&Lpuart_Uart_Ip_xHwConfigPB_2
#endif

#ifndef LOGI
#define LOGI(tagString, ...) printf(__VA_ARGS__)
#endif

/*==================================================================================================
*                                             ENUMS
==================================================================================================*/

/*==================================================================================================
*                                STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
*                                GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
*                                    FUNCTION PROTOTYPES
==================================================================================================*/
void Console_SerialPort_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* RETARGET_H_ */

/** @} */
