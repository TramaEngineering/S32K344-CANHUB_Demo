
/*==================================================================================================
*   Project              : RTD AUTOSAR 4.4
*   Platform             : CORTEXM
*   Peripheral           : Emios Siul2 Wkpu LpCmp
*   Dependencies         : none
*
*   Autosar Version      : 4.4.0
*   Autosar Revision     : ASR_REL_4_4_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 2.0.0
*   Build Version        : S32K3_RTD_2_0_0_D2203_ASR_REL_4_4_REV_0000_20220331
*
*   (c) Copyright 2020 - 2022 NXP Semiconductors
*   All Rights Reserved.
*
*   NXP Confidential. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/


#ifndef SIUL2_ICU_IP_SA_BOARD_INITPERIPHERALS_PBCFG_H
#define SIUL2_ICU_IP_SA_BOARD_INITPERIPHERALS_PBCFG_H

/**
 *   @file         Siul2_Icu_Ip_PBCfg.h
 *   @version 2.0.0
 *
 *   @brief   AUTOSAR Icu - contains the data exported by the Icu module
 *   @details Contains the information that will be exported by the module, as requested by Autosar.
 *
 *   @addtogroup siul2_icu_ip SIUL2 IPL
 *   @{
 */
 
#ifdef __cplusplus
extern "C"{
#endif

 /*==================================================================================================
 *                                         INCLUDE FILES
 * 1) system and project includes
 * 2) needed interfaces from external units
 * 3) internal and external interfaces from this unit
 *================================================================================================*/

/*==================================================================================================
 *                              SOURCE FILE VERSION INFORMATION
 *================================================================================================*/
#define SIUL2_ICU_IP_SA_BOARD_INITPERIPHERALS_PBCFG_VENDOR_ID                     43
#define SIUL2_ICU_IP_SA_BOARD_INITPERIPHERALS_PBCFG_AR_RELEASE_MAJOR_VERSION      4
#define SIUL2_ICU_IP_SA_BOARD_INITPERIPHERALS_PBCFG_AR_RELEASE_MINOR_VERSION      4
#define SIUL2_ICU_IP_SA_BOARD_INITPERIPHERALS_PBCFG_AR_RELEASE_REVISION_VERSION   0
#define SIUL2_ICU_IP_SA_BOARD_INITPERIPHERALS_PBCFG_SW_MAJOR_VERSION              2
#define SIUL2_ICU_IP_SA_BOARD_INITPERIPHERALS_PBCFG_SW_MINOR_VERSION              0
#define SIUL2_ICU_IP_SA_BOARD_INITPERIPHERALS_PBCFG_SW_PATCH_VERSION              0

/*==================================================================================================
 *                                      FILE VERSION CHECKS
 *================================================================================================*/

/*==================================================================================================
 *                                       GLOBAL CONSTANTS
 *================================================================================================*/
#define SIUL2_ICU_CONFIG_SA_BOARD_INITPERIPHERALS_PB  \
    extern const Siul2_Icu_Ip_ChannelConfigType Siul2_Icu_Ip_0_ChannelConfig_PB_BOARD_InitPeripherals[2U]; \
    extern const Siul2_Icu_Ip_InstanceConfigType Siul2_Icu_Ip_0_InstanceConfig_PB_BOARD_InitPeripherals; \
    extern const Siul2_Icu_Ip_ConfigType Siul2_Icu_Ip_0_Config_PB_BOARD_InitPeripherals; \

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* SIUL2_ICU_IP_PBCFG_SA_BOARD_INITPERIPHERALS_H */

