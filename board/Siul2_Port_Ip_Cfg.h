/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7 
*   Platform             : CORTEXM
*   Peripheral           : SIUL2
*   Dependencies         : none
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 5.0.0
*   Build Version        : S32K3_S32M27x_AUTOSAR_R21-11_RTD_5_0_0_D2410_ASR_REL_4_7_REV_0000_20241002
*
*   Copyright 2020 - 2024 NXP
*
*   NXP Confidential. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/

#ifndef SIUL2_PORT_IP_CFG_H
#define SIUL2_PORT_IP_CFG_H

/**
*   @file      Siul2_Port_Ip_Cfg.h
*
*   @addtogroup Port_CFG
*   @{
*/

#ifdef __cplusplus
extern "C"{
#endif


/*==================================================================================================
                                         INCLUDE FILES
 1) system and project includes
 2) needed interfaces from external units
 3) internal and external interfaces from this unit
==================================================================================================*/
#include "S32K344_SIUL2.h"
#include "Siul2_Port_Ip_Types.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define SIUL2_PORT_IP_VENDOR_ID_CFG_H                       43
#define SIUL2_PORT_IP_AR_RELEASE_MAJOR_VERSION_CFG_H        4
#define SIUL2_PORT_IP_AR_RELEASE_MINOR_VERSION_CFG_H        7
#define SIUL2_PORT_IP_AR_RELEASE_REVISION_VERSION_CFG_H     0
#define SIUL2_PORT_IP_SW_MAJOR_VERSION_CFG_H                5
#define SIUL2_PORT_IP_SW_MINOR_VERSION_CFG_H                0
#define SIUL2_PORT_IP_SW_PATCH_VERSION_CFG_H                0

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/
/* Check if the files Siul2_Port_Ip_Cfg.h and Siul2_Port_Ip_Types.h are of the same version */
#if (SIUL2_PORT_IP_VENDOR_ID_CFG_H != SIUL2_PORT_IP_TYPES_VENDOR_ID_H)
    #error "Siul2_Port_Ip_Cfg.h and Siul2_Port_Ip_Types.h have different vendor ids"
#endif
/* Check if Siul2_Port_Ip_Cfg.h and Siul2_Port_Ip_Types.h are of the same Autosar version */
#if ((SIUL2_PORT_IP_AR_RELEASE_MAJOR_VERSION_CFG_H    != SIUL2_PORT_IP_TYPES_AR_RELEASE_MAJOR_VERSION_H) || \
     (SIUL2_PORT_IP_AR_RELEASE_MINOR_VERSION_CFG_H    != SIUL2_PORT_IP_TYPES_AR_RELEASE_MINOR_VERSION_H) || \
     (SIUL2_PORT_IP_AR_RELEASE_REVISION_VERSION_CFG_H != SIUL2_PORT_IP_TYPES_AR_RELEASE_REVISION_VERSION_H) \
    )
    #error "AutoSar Version Numbers of Siul2_Port_Ip_Cfg.h and Siul2_Port_Ip_Types.h are different"
#endif
/* Check if Siul2_Port_Ip_Cfg.h and Siul2_Port_Ip_Types.h are of the same Software version */
#if ((SIUL2_PORT_IP_SW_MAJOR_VERSION_CFG_H != SIUL2_PORT_IP_TYPES_SW_MAJOR_VERSION_H) || \
     (SIUL2_PORT_IP_SW_MINOR_VERSION_CFG_H != SIUL2_PORT_IP_TYPES_SW_MINOR_VERSION_H) || \
     (SIUL2_PORT_IP_SW_PATCH_VERSION_CFG_H != SIUL2_PORT_IP_TYPES_SW_PATCH_VERSION_H)    \
    )
    #error "Software Version Numbers of Siul2_Port_Ip_Cfg.h and Siul2_Port_Ip_Types.h are different"
#endif
/*==================================================================================================
                                           CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                      DEFINES AND MACROS
==================================================================================================*/
#define SIUL2_MSCR_SSS_MASK                      (0xFU)
#define SIUL2_MSCR_SSS_SHIFT                     (0U)
#define SIUL2_MSCR_SSS_WIDTH                     (4U)
#define SIUL2_MSCR_SSS(x)                        (((uint32)(((uint32)(x)) << SIUL2_MSCR_SSS_SHIFT)) & SIUL2_MSCR_SSS_MASK)

#define SIUL2_MSCR_SRE_MASK                      (0x4000U)
#define SIUL2_MSCR_SRE_SHIFT                     (14U)
#define SIUL2_MSCR_SRE_WIDTH                     (1U)
#define SIUL2_MSCR_SRE(x)                        (((uint32)(((uint32)(x)) << SIUL2_MSCR_SRE_SHIFT)) & SIUL2_MSCR_SRE_MASK)


/*! @brief Definitions for PortContainer_0_BOARD_InitPeripherals Functional Group */

/*! @brief User number of configured pins */
#define NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals 52

#define PORT_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Port_MemMap.h"

/*! @brief User configuration structure */
extern const Siul2_Port_Ip_PinSettingsConfig g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals[NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals];

#define PORT_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Port_MemMap.h"

/*! @brief Defines for user pin and port configurations */
#define LED_BLUE_PIN            12u
#define LED_BLUE_PORT           PTE_L_HALF
#define LED_RED_PIN             14u
#define LED_RED_PORT            PTE_L_HALF
#define LED_GREEN_PIN           11u
#define LED_GREEN_PORT          PTA_H_HALF
#define CAN0_EN_PIN             8u
#define CAN0_EN_PORT            PTC_H_HALF
#define LED_CAN0_PIN            2u
#define LED_CAN0_PORT           PTC_H_HALF
#define CAN0_STB_N_PIN          5u
#define CAN0_STB_N_PORT         PTC_H_HALF
#define CAN1_STB_N_PIN          2u
#define CAN1_STB_N_PORT         PTD_L_HALF
#define CAN1_EN_PIN             7u
#define CAN1_EN_PORT            PTD_H_HALF
#define LED_CAN1_PIN            5u
#define LED_CAN1_PORT           PTE_L_HALF
#define CAN2_STB_N_PIN          6u
#define CAN2_STB_N_PORT         PTD_H_HALF
#define LED_CAN2_PIN            4u
#define LED_CAN2_PORT           PTD_H_HALF
#define CAN3_STB_N_PIN          1u
#define CAN3_STB_N_PORT         PTB_L_HALF
#define CAN3_EN_PIN             0u
#define CAN3_EN_PORT            PTB_L_HALF
#define LED_CAN3_PIN            8u
#define LED_CAN3_PORT           PTB_H_HALF
#define CAN4_STB_N_PIN          9u
#define CAN4_STB_N_PORT         PTC_H_HALF
#define CAN4_EN_PIN             10u
#define CAN4_EN_PORT            PTC_H_HALF
#define LED_CAN4_PIN            10u
#define LED_CAN4_PORT           PTB_H_HALF
#define CAN5_STB_N_PIN          1u
#define CAN5_STB_N_PORT         PTE_H_HALF
#define CAN5_EN_PIN             14u
#define CAN5_EN_PORT            PTD_H_HALF
#define LED_CAN5_PIN            15u
#define LED_CAN5_PORT           PTD_H_HALF
#define CAN2_EN_PIN             4u
#define CAN2_EN_PORT            PTD_L_HALF
#define TP31_PIN                15u
#define TP31_PORT               PTA_H_HALF

/*==================================================================================================
                                           ENUMS
==================================================================================================*/

/*==================================================================================================
                               STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
                               GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
                               FUNCTION PROTOTYPES
==================================================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* SIUL2_PORT_IP_CFG_H */

