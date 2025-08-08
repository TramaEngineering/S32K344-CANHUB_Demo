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

/**
*   @file      Tspc_Port_Ip_Cfg.h
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
#include "Tspc_Port_Ip_Cfg.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define TSPC_PORT_IP_VENDOR_ID_CFG_C                       43
#define TSPC_PORT_IP_AR_RELEASE_MAJOR_VERSION_CFG_C        4
#define TSPC_PORT_IP_AR_RELEASE_MINOR_VERSION_CFG_C        7
#define TSPC_PORT_IP_AR_RELEASE_REVISION_VERSION_CFG_C     0
#define TSPC_PORT_IP_SW_MAJOR_VERSION_CFG_C                5
#define TSPC_PORT_IP_SW_MINOR_VERSION_CFG_C                0
#define TSPC_PORT_IP_SW_PATCH_VERSION_CFG_C                0

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/
/* Check if Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h are of the same vendor */
#if (TSPC_PORT_IP_VENDOR_ID_CFG_C != TSPC_PORT_IP_VENDOR_ID_CFG_H)
    #error "Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h have different vendor ids"
#endif
/* Check if Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h are of the same Autosar version */
#if ((TSPC_PORT_IP_AR_RELEASE_MAJOR_VERSION_CFG_C    != TSPC_PORT_IP_AR_RELEASE_MAJOR_VERSION_CFG_H) || \
    (TSPC_PORT_IP_AR_RELEASE_MINOR_VERSION_CFG_C    != TSPC_PORT_IP_AR_RELEASE_MINOR_VERSION_CFG_H) || \
    (TSPC_PORT_IP_AR_RELEASE_REVISION_VERSION_CFG_C != TSPC_PORT_IP_AR_RELEASE_REVISION_VERSION_CFG_H) \
    )
    #error "AutoSar Version Numbers of Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h are different"
#endif
/* Check if Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h are of the same Software version */
#if ((TSPC_PORT_IP_SW_MAJOR_VERSION_CFG_C != TSPC_PORT_IP_SW_MAJOR_VERSION_CFG_H) || \
    (TSPC_PORT_IP_SW_MINOR_VERSION_CFG_C != TSPC_PORT_IP_SW_MINOR_VERSION_CFG_H) || \
    (TSPC_PORT_IP_SW_PATCH_VERSION_CFG_C != TSPC_PORT_IP_SW_PATCH_VERSION_CFG_H)    \
    )
    #error "Software Version Numbers of Tspc_Port_Ip_Cfg.c and Tspc_Port_Ip_Cfg.h are different"
#endif

/*==================================================================================================
                             LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
                                             LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
                                            LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                           GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           GLOBAL VARIABLES
==================================================================================================*/

/* clang-format off */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
PortContainer_0_BOARD_InitPeripherals:
- options: {callFromInitBoot: 'true', coreID: M7_0}
- pin_list:
  - {pin_num: '29', peripheral: SIUL2, signal: 'gpio, 140', pin_signal: PTE12, direction: OUTPUT}
  - {pin_num: '26', peripheral: SIUL2, signal: 'gpio, 142', pin_signal: PTE14, direction: OUTPUT}
  - {pin_num: '28', peripheral: SIUL2, signal: 'gpio, 27', pin_signal: PTA27, direction: OUTPUT}
  - {pin_num: '88', peripheral: SIUL2, signal: 'gpio, 88', pin_signal: PTC24, direction: OUTPUT, InitValue: state_1}
  - {pin_num: '82', peripheral: SIUL2, signal: 'gpio, 82', pin_signal: PTC18, direction: OUTPUT, InitValue: state_0}
  - {pin_num: '86', peripheral: SIUL2, signal: 'gpio, 85', pin_signal: PTC21, identifier: CAN0_STB_N, direction: OUTPUT, InitValue: state_1}
  - {pin_num: '121', peripheral: SIUL2, signal: 'gpio, 98', pin_signal: PTD2, direction: OUTPUT, InitValue: state_1}
  - {pin_num: '122', peripheral: SIUL2, signal: 'gpio, 119', pin_signal: PTD23, direction: OUTPUT, InitValue: state_1}
  - {pin_num: '12', peripheral: SIUL2, signal: 'gpio, 133', pin_signal: PTE5, direction: OUTPUT}
  - {pin_num: '118', peripheral: SIUL2, signal: 'gpio, 118', pin_signal: PTD22, direction: OUTPUT, InitValue: state_1}
  - {pin_num: '111', peripheral: SIUL2, signal: 'gpio, 116', pin_signal: PTD20, direction: OUTPUT}
  - {pin_num: '94', peripheral: SIUL2, signal: 'gpio, 33', pin_signal: PTB1, direction: OUTPUT, InitValue: state_1}
  - {pin_num: '95', peripheral: SIUL2, signal: 'gpio, 32', pin_signal: PTB0, direction: OUTPUT, InitValue: state_1}
  - {pin_num: '70', peripheral: SIUL2, signal: 'gpio, 56', pin_signal: PTB24, direction: OUTPUT}
  - {pin_num: '89', peripheral: SIUL2, signal: 'gpio, 89', pin_signal: PTC25, direction: OUTPUT, InitValue: state_0}
  - {pin_num: '91', peripheral: SIUL2, signal: 'gpio, 90', pin_signal: PTC26, direction: OUTPUT, InitValue: state_1}
  - {pin_num: '73', peripheral: SIUL2, signal: 'gpio, 58', pin_signal: PTB26, direction: OUTPUT}
  - {pin_num: '142', peripheral: SIUL2, signal: 'gpio, 145', pin_signal: PTE17, direction: OUTPUT, InitValue: state_0}
  - {pin_num: '138', peripheral: SIUL2, signal: 'gpio, 126', pin_signal: PTD30, direction: OUTPUT, InitValue: state_1}
  - {pin_num: '139', peripheral: SIUL2, signal: 'gpio, 127', pin_signal: PTD31, direction: OUTPUT}
  - {pin_num: '15', peripheral: SIUL2, signal: 'eirq, 5', pin_signal: PTA25}
  - {pin_num: '35', peripheral: SIUL2, signal: 'eirq, 31', pin_signal: PTD15}
  - {pin_num: '46', peripheral: EMAC, signal: emac_mii_rmii_mdc, pin_signal: PTE8}
  - {pin_num: '34', peripheral: EMAC, signal: emac_mii_rmii_mdio, pin_signal: PTD16, direction: INPUT/OUTPUT}
  - {pin_num: '62', peripheral: EMAC, signal: 'emac_mii_rmii_rxd, 0', pin_signal: PTC0}
  - {pin_num: '61', peripheral: EMAC, signal: 'emac_mii_rmii_rxd, 1', pin_signal: PTC1}
  - {pin_num: '47', peripheral: EMAC, signal: 'emac_mii_rmii_txd, 0', pin_signal: PTB5}
  - {pin_num: '48', peripheral: EMAC, signal: 'emac_mii_rmii_txd, 1', pin_signal: PTB4}
  - {pin_num: '68', peripheral: EMAC, signal: emac_mii_rmii_rx_dv, pin_signal: PTC15}
  - {pin_num: '71', peripheral: EMAC, signal: emac_mii_rmii_rx_er, pin_signal: PTC14}
  - {pin_num: '52', peripheral: EMAC, signal: emac_mii_rmii_tx_clk, pin_signal: PTD6}
  - {pin_num: '36', peripheral: EMAC, signal: emac_mii_rmii_tx_en, pin_signal: PTE9}
  - {pin_num: '97', peripheral: CAN1, signal: can1_rx, pin_signal: PTC9}
  - {pin_num: '98', peripheral: CAN1, signal: can1_tx, pin_signal: PTC8}
  - {pin_num: '102', peripheral: CAN0, signal: can0_rx, pin_signal: PTA6}
  - {pin_num: '100', peripheral: CAN0, signal: can0_tx, pin_signal: PTA7}
  - {pin_num: '158', peripheral: CAN2, signal: can2_rx, pin_signal: PTE25}
  - {pin_num: '157', peripheral: CAN2, signal: can2_tx, pin_signal: PTE24}
  - {pin_num: '99', peripheral: CAN3, signal: can3_rx, pin_signal: PTC29}
  - {pin_num: '96', peripheral: CAN3, signal: can3_tx, pin_signal: PTC28}
  - {pin_num: '103', peripheral: CAN4, signal: can4_rx, pin_signal: PTC31}
  - {pin_num: '101', peripheral: CAN4, signal: can4_tx, pin_signal: PTC30}
  - {pin_num: '90', peripheral: CAN5, signal: can5_rx, pin_signal: PTC11}
  - {pin_num: '92', peripheral: CAN5, signal: can5_tx, pin_signal: PTC10}
  - {pin_num: '7', peripheral: LPSPI3, signal: lpspi3_sck, pin_signal: PTD1, direction: OUTPUT}
  - {pin_num: '10', peripheral: LPSPI3, signal: lpspi3_sin, pin_signal: PTE10, direction: OUTPUT}
  - {pin_num: '8', peripheral: LPSPI3, signal: lpspi3_sout, pin_signal: PTD0, direction: INPUT}
  - {pin_num: '172', peripheral: LPUART2, signal: lpuart2_rx, pin_signal: PTA8}
  - {pin_num: '171', peripheral: LPUART2, signal: lpuart2_tx, pin_signal: PTA9, direction: OUTPUT}
  - {pin_num: '119', peripheral: SIUL2, signal: 'gpio, 100', pin_signal: PTD4, direction: OUTPUT}
  - {pin_num: '31', peripheral: LPSPI3, signal: lpspi3_pcs0, pin_signal: PTD17, direction: OUTPUT}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* No registers that support TSPC were configured*/

/*==================================================================================================
                                      LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                                           LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
                                           GLOBAL FUNCTIONS
==================================================================================================*/


#ifdef __cplusplus
}
#endif

/** @} */
