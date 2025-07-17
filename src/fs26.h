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
 *   This file contains sample code only. It is not part of the production code deliverables.
 */

#ifndef FS26_H_
#define FS26_H_
#include "stdio.h"
#include "stdint.h"
/****************************************************************************
 * Data Types
 ****************************************************************************/

/*! @brief this callback function is used to do SPI transfers to the FS26 */
typedef int (*fs26SpiTransferCallbackFunction)(uint8_t *TxBuffer, uint8_t *RxBuffer, uint16_t Length);

typedef enum
{
	FS26_WD_DISABLED   = 0U, /* Watchdog refresh disabled */
	FS26_WD_SIMPLE     = 1U, /* Simple watchdog refresh */
	FS26_WD_CHALLENGER = 2U  /* Challenger watchdog refresh */
} fs26_watchdog_type;

enum STATUS{
	INITIALIZE,
	ERROR,
	NOMINAL,
	ETH_ACTIVITY,
	RED,
	GREEN,
	BLUE,
	CYAN,/*BLUE AND GREEN*/
	YELLOW, /*GREEN AND RED*/
	MAGENTA, /*RED AND BLUE*/
	WHITE, /*ALL*/
};
typedef enum STATUS rgb_status;

extern rgb_status error_st;

/****************************************************************************
 * Public functions
 ****************************************************************************/

/*!
 * @brief 		Function used to make sure the FS26 will not continues reset the S32K3 via the watchdog
 * @note 		Make sure the correct SPI is initialized
 *
 * @param[in]	p_fs26SpiTransferCallbackFunction - pointer to the callback function to perform SPI transfer
 *
 * @return 		None
 */
void fs26_initialize(fs26SpiTransferCallbackFunction p_fs26SpiTransferCallbackFunction);

#endif /* FS26_H_ */
