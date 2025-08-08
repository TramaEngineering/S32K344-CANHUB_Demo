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

#ifndef SIUL2_ICU_IP_DEFINES_H
#define SIUL2_ICU_IP_DEFINES_H

/**
 *   @file    Siul2_Icu_Ip_Defines.h
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
#include "StandardTypes.h"
#include "S32K344_SIUL2.h"
/*==================================================================================================
*                                 SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define SIUL2_ICU_IP_DEFINES_VENDOR_ID                    43
#define SIUL2_ICU_IP_DEFINES_MODULE_ID                    122
#define SIUL2_ICU_IP_DEFINES_AR_RELEASE_MAJOR_VERSION     4
#define SIUL2_ICU_IP_DEFINES_AR_RELEASE_MINOR_VERSION     4
#define SIUL2_ICU_IP_DEFINES_AR_RELEASE_REVISION_VERSION  0
#define SIUL2_ICU_IP_DEFINES_SW_MAJOR_VERSION             2
#define SIUL2_ICU_IP_DEFINES_SW_MINOR_VERSION             0
#define SIUL2_ICU_IP_DEFINES_SW_PATCH_VERSION             0

/*==================================================================================================
 *                                      FILE VERSION CHECKS
 *================================================================================================*/
#ifndef DISABLE_MCAL_INTERMODULE_ASR_CHECK
    /* Check if header file and StandardTypes.h file are of the same Autosar version */
    #if ((SIUL2_ICU_IP_DEFINES_AR_RELEASE_MAJOR_VERSION != STD_AR_RELEASE_MAJOR_VERSION) || \
         (SIUL2_ICU_IP_DEFINES_AR_RELEASE_MINOR_VERSION != STD_AR_RELEASE_MINOR_VERSION))
        #error "AutoSar Version Numbers of Siul2_Icu_Ip_Defines.h and StandardTypes.h are different"
    #endif
#endif
/*==================================================================================================
                                       DEFINES AND MACROS
==================================================================================================*/
/** @brief Switches the Development Error Detection and Notification on or off.  */
#define SIUL2_ICU_IP_DEV_ERROR_DETECT (STD_OFF)

/** @brief The number of channels available on each instance */
#define SIUL2_ICU_IP_NUM_OF_CHANNELS  (32)
/** @brief Adds or removes all services related to the deinitialization functionality. */
#define SIUL2_ICU_IP_DEINIT_API                 (STD_ON)

/** @brief Adds or removes all services related to mode set functionality. */
#define SIUL2_ICU_IP_SET_MODE_API               (STD_ON)

/** @brief Adds or removes all services related to dual clock edge functionality. */
#define SIUL2_ICU_IP_DUAL_CLOCK_MODE_API        (STD_ON)

/** @brief Add or remove all functions related to input state. */
#define SIUL2_ICU_IP_GET_INPUT_STATE_API        (STD_ON)
/** @brief Instance of SIUL2 used by current platform. */
#define SIUL2_ICU_IP_INSTANCE                (0U)

/** @brief Support for User mode. If this parameter has been configured to STD_ON, the FTM driver 
 *         code can be executed from both supervisor and user mode. */
#define SIUL2_ICU_IP_ENABLE_USER_MODE_SUPPORT   (STD_OFF)

/* Verification for user mode support. */
#ifndef MCAL_ENABLE_USER_MODE_SUPPORT
 #if (defined (SIUL2_ICU_IP_ENABLE_USER_MODE_SUPPORT) && (STD_ON == SIUL2_ICU_IP_ENABLE_USER_MODE_SUPPORT))
    #error MCAL_ENABLE_USER_MODE_SUPPORT is not enabled. For running Icu in user mode the MCAL_ENABLE_USER_MODE_SUPPORT needs to be defined
 #endif
#endif


#ifdef __cplusplus
}
#endif

/** @} */

#endif  /* SIUL2_ICU_IP_DEFINES_H */

