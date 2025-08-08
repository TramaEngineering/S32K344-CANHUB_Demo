/*==================================================================================================
*   Project              : RTD AUTOSAR 4.4
*   Platform             : CORTEXM
*   Peripheral           : GMAC
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
/*   @implements Gmac_Ip_PBcfg.c_Artifact */
/**
*   @file
*
*   @addtogroup GMAC_DRIVER_CONFIGURATION GMAC Driver Configuration
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
#include "Gmac_Ip_Types.h"
#include "Gmac_Ip_Cfg.h"

/*==================================================================================================
*                              SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_VENDOR_ID_C                     43
#define GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_AR_RELEASE_MAJOR_VERSION_C      4
#define GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_AR_RELEASE_MINOR_VERSION_C      4
#define GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_AR_RELEASE_REVISION_VERSION_C   0
#define GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_SW_MAJOR_VERSION_C              2
#define GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_SW_MINOR_VERSION_C              0
#define GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_SW_PATCH_VERSION_C              0

/*==================================================================================================
*                                     FILE VERSION CHECKS
==================================================================================================*/
/* Checks against Gmac_Ip_Types.h */
#if (GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_VENDOR_ID_C != GMAC_IP_TYPES_VENDOR_ID)
    #error "Gmac_Ip_Sa_BOARD_INITPERIPHERALS_PBcfg.c and Gmac_Ip_Types.h have different vendor ids"
#endif
#if ((GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_AR_RELEASE_MAJOR_VERSION_C    != GMAC_IP_TYPES_AR_RELEASE_MAJOR_VERSION) || \
     (GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_AR_RELEASE_MINOR_VERSION_C    != GMAC_IP_TYPES_AR_RELEASE_MINOR_VERSION) || \
     (GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_AR_RELEASE_REVISION_VERSION_C != GMAC_IP_TYPES_AR_RELEASE_REVISION_VERSION))
     #error "AUTOSAR Version Numbers of Gmac_Ip_Sa_BOARD_INITPERIPHERALS_PBcfg.c and Gmac_Ip_Types.h are different"
#endif
#if ((GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_SW_MAJOR_VERSION_C != GMAC_IP_TYPES_SW_MAJOR_VERSION) || \
     (GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_SW_MINOR_VERSION_C != GMAC_IP_TYPES_SW_MINOR_VERSION) || \
     (GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_SW_PATCH_VERSION_C != GMAC_IP_TYPES_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of Gmac_Ip_Sa_BOARD_INITPERIPHERALS_PBcfg.c and Gmac_Ip_Types.h are different"
#endif

/* Checks against Gmac_Ip_Cfg.h */
#if (GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_VENDOR_ID_C != GMAC_IP_CFG_VENDOR_ID)
    #error "Gmac_Ip_Sa_BOARD_INITPERIPHERALS_PBcfg.c and Gmac_Ip_Cfg.h have different vendor ids"
#endif
#if ((GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_AR_RELEASE_MAJOR_VERSION_C    != GMAC_IP_CFG_AR_RELEASE_MAJOR_VERSION) || \
     (GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_AR_RELEASE_MINOR_VERSION_C    != GMAC_IP_CFG_AR_RELEASE_MINOR_VERSION) || \
     (GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_AR_RELEASE_REVISION_VERSION_C != GMAC_IP_CFG_AR_RELEASE_REVISION_VERSION))
     #error "AUTOSAR Version Numbers of Gmac_Ip_Sa_BOARD_INITPERIPHERALS_PBcfg.c and Gmac_Ip_Cfg.h are different"
#endif
#if ((GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_SW_MAJOR_VERSION_C != GMAC_IP_CFG_SW_MAJOR_VERSION) || \
     (GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_SW_MINOR_VERSION_C != GMAC_IP_CFG_SW_MINOR_VERSION) || \
     (GMAC_IP_SA_PBCFG_BOARD_INITPERIPHERALS_SW_PATCH_VERSION_C != GMAC_IP_CFG_SW_PATCH_VERSION) \
    )
    #error "Software Version Numbers of Gmac_Ip_Sa_BOARD_INITPERIPHERALS_PBcfg.c and Gmac_Ip_Cfg.h are different"
#endif

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/
#define GMAC_MAC_ADDR_LENGTH        (6U)

/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/
#define ETH_START_SEC_CODE
#include "Eth_MemMap.h"

/*! @brief Channel callbacks external declarations */
extern void enet_rx_interrupt(uint8 instance, uint8 channel);
extern void enet_tx_interrupt(uint8 instance, uint8 channel);

#define ETH_STOP_SEC_CODE
#include "Eth_MemMap.h"

/*==================================================================================================
*                                      BUFFER DECLARATIONS
==================================================================================================*/
#define ETH_START_SEC_VAR_CLEARED_UNSPECIFIED_NO_CACHEABLE
#include "Eth_MemMap.h"

extern Gmac_Ip_BufferDescriptorType GMAC_0_RxRing_0_DescBuffer[];
extern uint8 GMAC_0_RxRing_0_DataBuffer[];
extern Gmac_Ip_BufferDescriptorType GMAC_0_TxRing_0_DescBuffer[];
extern uint8 GMAC_0_TxRing_0_DataBuffer[];

#define ETH_STOP_SEC_VAR_CLEARED_UNSPECIFIED_NO_CACHEABLE
#include "Eth_MemMap.h"

/*==================================================================================================
*                                   STATE STRUCTURE DECLARATIONS
==================================================================================================*/
#define ETH_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "Eth_MemMap.h"

extern Gmac_Ip_StateType GMAC_0_StateStructure;

#define ETH_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "Eth_MemMap.h"

/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/
#define ETH_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Eth_MemMap.h"


/*! @brief The MAC address(es) of the configured controller(s) */
static const uint8 GMAC_0_au8MacAddrPB_BOARD_INITPERIPHERALS[GMAC_MAC_ADDR_LENGTH] = { 0x66U, 0x55U, 0x44U, 0x33U, 0x22U, 0x11U };

/*! @brief Reception ring configuration structures */
static const Gmac_Ip_RxRingConfigType GMAC_0_aRxRingConfigPB_BOARD_INITPERIPHERALS[1U] =
{
    /* The configuration structure for Rx Ring 0 */
    {
        /*.ringDesc = */GMAC_0_RxRing_0_DescBuffer,
        /*.callback = */enet_rx_interrupt,
        /*.buffer = */GMAC_0_RxRing_0_DataBuffer,
        /*.interrupts = */(uint32)GMAC_CH_INTERRUPT_RI,
        /*.bufferLen = */128U,
        /*.ringSize = */16U,
        /*.priorityMask = */0U,
        /*.dmaBurstLength = */32U
    }
};

/*! @brief Transmission ring configuration structures */
static const Gmac_Ip_TxRingConfigType GMAC_0_aTxRingConfigPB_BOARD_INITPERIPHERALS[1U] =
{
    /* The configuration structure for Tx Ring 0 */
    {
        /*.weight = */0U,
        /*.idleSlopeCredit = */0U,
        /*.sendSlopeCredit = */0U,
        /*.hiCredit = */0U,
        /*.loCredit = */0,
        /*.ringDesc = */GMAC_0_TxRing_0_DescBuffer,
        /*.callback = */enet_tx_interrupt,
        /*.buffer = */GMAC_0_TxRing_0_DataBuffer,
        /*.interrupts = */(uint32)GMAC_CH_INTERRUPT_TI,
        /*.bufferLen = */128U,
        /*.ringSize = */16U,
        /*.priorityMask = */0U,
        /*.dmaBurstLength = */32U,
        /*.queueOpMode = */GMAC_OP_MODE_DCB_GEN
    }
};

/*! @brief Module configuration structures */
static const Gmac_Ip_ConfigType GMAC_0_InitConfigPB_BOARD_INITPERIPHERALS =
{
    /*.rxRingCount = */1U,
    /*.txRingCount = */1U,
#if (FEATURE_GMAC_ASP_ALL || FEATURE_GMAC_ASP_ECC)
    /*.safetyInterrupts = */0U,
    /*.safetyCallback = */NULL_PTR,
#endif
    /*.interrupts = */0U,
    /*.callback = */NULL_PTR,
    /*.miiMode = */GMAC_RMII_MODE,
    /*.txSchedAlgo = */GMAC_SCHED_ALGO_SP,
    /*.speed = */GMAC_SPEED_100M,
    /*.duplex = */GMAC_FULL_DUPLEX,
    /*.macConfig = */0U,
    /*.macPktFilterConfig = */0U | (uint32)GMAC_PKT_FILTER_RECV_ALL | (uint32)GMAC_PKT_FILTER_PROMISCUOUS_MODE,
    /*.enableCtrl = */TRUE
};

static const Gmac_Ip_TxGateControl GMAC_0_GateControlListPB_BOARD_INITPERIPHERALS[1U]  =
{
    {
        /*.timeInterval = */0U,
        /*.gateControlFifo = */0U
    }
};
static const Gmac_Ip_TxTimeAwareShaper GMAC_0_pTxTimeShaperPB_BOARD_INITPERIPHERALS =
{
    /*.baseTimeSecond = */0U,
    /*.baseTimenanoSecond = */0U,
    /*.cycleTimeSecond = */0U,
    /*.cycleTimeNanoSecond = */0U,
    /*.extendedTime = */0U,
    /*.gateControlListDepth = */0U,
    /*.releaseAdvanceTime = */0U,
    /*.holdAdvanceTime = */0U,
    /*.preemptionClassification = */0U,
    /*.GateControlList = */GMAC_0_GateControlListPB_BOARD_INITPERIPHERALS
};

#define ETH_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Eth_MemMap.h"
/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/
#define ETH_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Eth_MemMap.h"

const Gmac_CtrlConfigType Gmac_0_ConfigPB_BOARD_INITPERIPHERALS =
{
    &GMAC_0_StateStructure,
    &GMAC_0_InitConfigPB_BOARD_INITPERIPHERALS,
    &GMAC_0_aRxRingConfigPB_BOARD_INITPERIPHERALS[0U],
    &GMAC_0_aTxRingConfigPB_BOARD_INITPERIPHERALS[0U],
    &GMAC_0_au8MacAddrPB_BOARD_INITPERIPHERALS[0U],
    &GMAC_0_pTxTimeShaperPB_BOARD_INITPERIPHERALS
};

#define ETH_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Eth_MemMap.h"

/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/


/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/


/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

#ifdef __cplusplus
}
#endif

/** @} */

