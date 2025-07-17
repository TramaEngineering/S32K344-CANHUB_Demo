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

#include "fs26.h"
#include "fs26_defines.h"

#include <sys/types.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include "Devassert.h"
#include "./uart_print/retarget.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* CRC polynomial used for SPI communication. */
#define FS26_CRC_TBL_SIZE 256U /* Size of CRC table. */
#define FS26_COM_CRC_POLYNOM 0x1DU /* CRC polynom. */
#define FS26_COM_CRC_INIT 0xFFU /* CRC initial value. */

// this will enable debug printf() statements, which makes it too slow to stop WD
//#define DEBUG_FS26_OUTPUT

#ifdef DEBUG_FS26_OUTPUT
#	define DEBUG_FS26 1
#else
# 	define DEBUG_FS26 0
#endif

#define debug_printf(...) \
		do { if(DEBUG_FS26) printf(__VA_ARGS__); } while (0) // ("%s:%d:%s(): ",__FILE__, __LINE__, __func__);

#ifndef OK
# 	define OK 0
#endif
/****************************************************************************
 * Private Types
 ****************************************************************************/
fs26SpiTransferCallbackFunction p_fs26SpiTransferCallbackFunctionfp;

/* CRC lookup table. */
static const uint8_t FS26_CRC_TABLE[FS26_CRC_TBL_SIZE] = {
		0x00U, 0x1DU, 0x3AU, 0x27U, 0x74U, 0x69U, 0x4EU, 0x53U, 0xE8U, 0xF5U, 0xD2U, 0xCFU, 0x9CU,
		0x81U, 0xA6U, 0xBBU, 0xCDU, 0xD0U, 0xF7U, 0xEAU, 0xB9U, 0xA4U, 0x83U, 0x9EU, 0x25U, 0x38U,
		0x1FU, 0x02U, 0x51U, 0x4CU, 0x6BU, 0x76U, 0x87U, 0x9AU, 0xBDU, 0xA0U, 0xF3U, 0xEEU, 0xC9U,
		0xD4U, 0x6FU, 0x72U, 0x55U, 0x48U, 0x1BU, 0x06U, 0x21U, 0x3CU, 0x4AU, 0x57U, 0x70U, 0x6DU,
		0x3EU, 0x23U, 0x04U, 0x19U, 0xA2U, 0xBFU, 0x98U, 0x85U, 0xD6U, 0xCBU, 0xECU, 0xF1U, 0x13U,
		0x0EU, 0x29U, 0x34U, 0x67U, 0x7AU, 0x5DU, 0x40U, 0xFBU, 0xE6U, 0xC1U, 0xDCU, 0x8FU, 0x92U,
		0xB5U, 0xA8U, 0xDEU, 0xC3U, 0xE4U, 0xF9U, 0xAAU, 0xB7U, 0x90U, 0x8DU, 0x36U, 0x2BU, 0x0CU,
		0x11U, 0x42U, 0x5FU, 0x78U, 0x65U, 0x94U, 0x89U, 0xAEU, 0xB3U, 0xE0U, 0xFDU, 0xDAU, 0xC7U,
		0x7CU, 0x61U, 0x46U, 0x5BU, 0x08U, 0x15U, 0x32U, 0x2FU, 0x59U, 0x44U, 0x63U, 0x7EU, 0x2DU,
		0x30U, 0x17U, 0x0AU, 0xB1U, 0xACU, 0x8BU, 0x96U, 0xC5U, 0xD8U, 0xFFU, 0xE2U, 0x26U, 0x3BU,
		0x1CU, 0x01U, 0x52U, 0x4FU, 0x68U, 0x75U, 0xCEU, 0xD3U, 0xF4U, 0xE9U, 0xBAU, 0xA7U, 0x80U,
		0x9DU, 0xEBU, 0xF6U, 0xD1U, 0xCCU, 0x9FU, 0x82U, 0xA5U, 0xB8U, 0x03U, 0x1EU, 0x39U, 0x24U,
		0x77U, 0x6AU, 0x4DU, 0x50U, 0xA1U, 0xBCU, 0x9BU, 0x86U, 0xD5U, 0xC8U, 0xEFU, 0xF2U, 0x49U,
		0x54U, 0x73U, 0x6EU, 0x3DU, 0x20U, 0x07U, 0x1AU, 0x6CU, 0x71U, 0x56U, 0x4BU, 0x18U, 0x05U,
		0x22U, 0x3FU, 0x84U, 0x99U, 0xBEU, 0xA3U, 0xF0U, 0xEDU, 0xCAU, 0xD7U, 0x35U, 0x28U, 0x0FU,
		0x12U, 0x41U, 0x5CU, 0x7BU, 0x66U, 0xDDU, 0xC0U, 0xE7U, 0xFAU, 0xA9U, 0xB4U, 0x93U, 0x8EU,
		0xF8U, 0xE5U, 0xC2U, 0xDFU, 0x8CU, 0x91U, 0xB6U, 0xABU, 0x10U, 0x0DU, 0x2AU, 0x37U, 0x64U,
		0x79U, 0x5EU, 0x43U, 0xB2U, 0xAFU, 0x88U, 0x95U, 0xC6U, 0xDBU, 0xFCU, 0xE1U, 0x5AU, 0x47U,
		0x60U, 0x7DU, 0x2EU, 0x33U, 0x14U, 0x09U, 0x7FU, 0x62U, 0x45U, 0x58U, 0x0BU, 0x16U, 0x31U,
		0x2CU, 0x97U, 0x8AU, 0xADU, 0xB0U, 0xE3U, 0xFEU, 0xD9U, 0xC4U
};

fs26_watchdog_type wd_type = FS26_WD_CHALLENGER;
static uint16_t watchdog_token = 0x5AB2;

// This will print on the debug console
static inline void fs26_print_general_device_status(uint32_t retval)
{
	if(retval & FS26_M_AVAL)
	{
		debug_printf("Main State machine availability\n");
	}
	if(retval & FS26_FS_EN)
	{
		debug_printf("Fail Safe State machine status enabled\n");
	}
	debug_printf("IRQ:\n");
	if(retval & FS26_FS_G)
	{
		debug_printf("FS26_FS_G\n");
	}
	if(retval & FS26_COM_G)
	{
		debug_printf("FS26_COM_G\n");
	}
	if(retval & FS26_WIO_G)
	{
		debug_printf("FS26_WIO_G\n");
	}
	if(retval & FS26_VSUP_G)
	{
		debug_printf("FS26_VSUP_G\n");
	}
	if(retval & FS26_REG_G)
	{
		debug_printf("FS26_REG_G\n");
	}
	if(retval & FS26_TSD_G)
	{
		debug_printf("FS26_TSD_G\n");
	}
}

static inline void fs26_print_crc(uint8_t crc, uint8_t crc_received)
{
	debug_printf("%02x<->%02x\n", crc, crc_received);
}

/* Computes Challenger Watchdog answer. */
static inline uint16_t fs26_wdcomputeanswer(uint16_t token)
{
	uint32_t u32_mr = token; /* Monitoring result. */

	/* Simulates ALU Checker on the MCU side. */
	u32_mr *= 4U;
	u32_mr += 6U;
	u32_mr -= 4U;
	u32_mr = ~u32_mr;
	u32_mr /= 4U;

	return (uint16_t)u32_mr;
}

static uint8_t fs26_calcrc(const uint8_t* data, uint8_t dataLen)
{
	uint8_t crc; /* Result. */
	uint8_t tableIdx; /* Index to the CRC table. */
	uint8_t dataIdx; /* Index to the data array (memory). */

	DevAssert(data != NULL);
	DevAssert(dataLen > 0);

	/* Set CRC token value. */

	crc = FS26_COM_CRC_INIT;

	for (dataIdx = dataLen; dataIdx > 0; dataIdx--)
	{
		tableIdx = crc ^ data[dataIdx];
		crc = FS26_CRC_TABLE[tableIdx];
	}

	return crc;
}

uint32_t fs26_setreg(uint8_t regaddr, uint16_t regval) {
	uint32_t spidata, retval, spiStatus;

	/* Send register address and set the value */
	spidata = FS26_REG_ADDR(regaddr) | FS26_SET_DATA(regval) | FS26_RW;
	spidata |= (uint32_t)fs26_calcrc((uint8_t*)&spidata, 3);

	// do SPI read (transfer)
	spiStatus = p_fs26SpiTransferCallbackFunctionfp((uint8_t*)&spidata, (uint8_t*)&retval, 4);

	//debug_printf("SPI transfer done (set), error check...\n");
	// check for errrors
	if(spiStatus)
	{
		printf("SPI Error: could not transfer: %d\n", (int)spiStatus);

		printf("looping\n");
		while(1);
	}

	fs26_print_general_device_status(retval);
	debug_printf("SPI info: Received %08lx\n", retval);

	if(fs26_calcrc((uint8_t*)&retval, 3) != ((uint8_t*)&retval)[0])
	{
		printf("CRC error expected %02x got %02x\n",
				fs26_calcrc((uint8_t*)&retval, 3),
				((uint8_t*)&retval)[0]);
		retval = 0x0;
	}

	return retval;
}

uint32_t fs26_getreg(uint8_t regaddr) {
	uint32_t spiDataTx, retVal, spiStatus;

	/* Send register address and calc CRC */
	spiDataTx  = FS26_REG_ADDR(regaddr);
	spiDataTx |= (uint32_t)fs26_calcrc((uint8_t*)&spiDataTx, 3);

	debug_printf("fs26 SPI transfer get BeEn, sending: %x\n", (unsigned int)spiDataTx);

	// do SPI read (transfer)
	spiStatus = p_fs26SpiTransferCallbackFunctionfp((uint8_t*)&spiDataTx, (uint8_t*)&retVal, 4);

	debug_printf("SPI transfer done (get), error check...\n");
	// check for errrors
	if(spiStatus)
	{
		printf("SPI Error: could not transfer: %d\n", (int)spiStatus);

		printf("looping\n");
		while(1);
	}

	debug_printf("SPI got BeEn: %x\n", (unsigned int)retVal);

	if(fs26_calcrc((uint8_t*)&retVal, 3) != ((uint8_t*)&retVal)[0])
	{
		printf("CRC error expected %02x got %02x\n",
				fs26_calcrc((uint8_t*)&retVal, 3),
				((uint8_t*)&retVal)[0]);
		retVal = 0x0;
	}

	/* DEBUG print */
	debug_printf("SPI info: %02x->%04lx\n", regaddr, retVal);

	return retVal;
}


static uint32_t fs26_wdreadchallengetoken(uint16_t* token_ptr)
{
	uint32_t retval;

	retval = fs26_getreg(FS26_FS_WD_TOKEN);
	if (retval != 0)
	{
		*token_ptr = FS26_GET_DATA(retval);
		retval = 0;
	}

	return retval;
}

int32_t fs26_wdrefresh(void)
{
	int32_t retval = OK;
	uint16_t u16_answer; /* Calculated monitoring result. */

	if (FS26_WD_DISABLED == wd_type)
	{
		/* No need to refresh watchdog. */
		retval = OK;
	}
	else if(FS26_WD_SIMPLE == wd_type)
	{
		if(fs26_setreg(FS26_FS_WD_ANSWER, watchdog_token) != 0)
		{
			retval = OK;
		}
		else
		{
			retval = -1;
		}
	}
	else if(FS26_WD_CHALLENGER == wd_type)
	{
		if(fs26_wdreadchallengetoken(&watchdog_token) == OK)
		{
			u16_answer = fs26_wdcomputeanswer(watchdog_token);

			if(fs26_setreg(FS26_FS_WD_ANSWER, u16_answer) != 0)
			{
				retval = OK;
			}
			else
			{
				retval = -1;
			}
		}
		else
		{
			retval = -1;
		}

	}

	/* Check if watchdog refresh was successful. */
	if (((FS26_WD_SIMPLE == wd_type) || (FS26_WD_CHALLENGER == wd_type)) && (OK == retval))
	{
		if((FS26_GET_DATA(fs26_getreg(FS26_FS_GRL_FLAGS)) & FS_WD_G_MASK) == FS_WD_G)
		{
			retval = -1;
		}
		else
		{
			retval = OK;
		}
	}
	return retval;
}


/* Initialization */
/*!
 * @brief 		Function used to make sure the FS26 will not continues reset the S32K3 via the watchdog
 * @note 		Make sure the correct SPI is initialized
 *
 * @param[in]	p_fs26SpiTransferCallbackBatFuntion - pointer to the callback function to perform SPI transfer
 *
 * @return 		None
 */
void fs26_initialize(fs26SpiTransferCallbackFunction p_fs26SpiTransferCallbackFunction)
{
	uint32_t retval;
	uint16_t regval;

	// connect the callback function
	p_fs26SpiTransferCallbackFunctionfp = p_fs26SpiTransferCallbackFunction;

	/* Check FS diag */

	retval = fs26_getreg(FS26_FS_DIAG_SAFETY1);

	if((FS26_GET_DATA(retval) & (ABIST1_PASS_MASK | LBIST_STATUS_MASK)) != (ABIST1_PASS | LBIST_STATUS_OK))
	{
		//debug_printf("FS26 DIAG failed %08lx\n", retval);

		//printf("FS26 DIAG failed %08lx\r\n", retval);

		while(1);
	}

	/* Get state machine state */
	retval = fs26_getreg(FS26_FS_STATES);

	if((FS26_GET_DATA(retval) & DBG_MODE_MASK) == DBG_MODE)
	{
		//debug_printf("FS26 in DEBUG mode\n");
		//printf("FS26 in DEBUG mode\r\n");
	}

	/* INIT_FS */
	/*error in reset*/
	if((FS26_GET_DATA(retval) & FS_STATES_MASK) == FS_STATES_INIT_FS)
	{

		/* Set all FS26_FS_I_XXX registers
		 * Note write data to both normal and not registers
		 */

		regval = VMON_PRE_OV_FS_REACTION_NO_EFFECT |
				VMON_PRE_UV_FS_REACTION_NO_EFFECT |
				VMON_CORE_OV_FS_REACTION_NO_EFFECT |
				VMON_CORE_UV_FS_REACTION_NO_EFFECT |
				VMON_LDO1_OV_FS_REACTION_NO_EFFECT |
				VMON_LDO1_UV_FS_REACTION_NO_EFFECT |
				VMON_LDO2_OV_FS_REACTION_NO_EFFECT |
				VMON_LDO2_UV_FS_REACTION_NO_EFFECT;

		fs26_setreg(FS26_FS_I_OVUV_SAFE_REACTION1, regval);
		fs26_setreg(FS26_FS_I_NOT_OVUV_SAFE_REACTION1, ~regval);


		regval = VMON_EXT_OV_FS_REACTION_NO_EFFECT |
				VMON_EXT_UV_FS_REACTION_NO_EFFECT |
				VMON_REF_OV_FS_REACTION_NO_EFFECT |
				VMON_REF_UV_FS_REACTION_NO_EFFECT |
				VMON_TRK2_OV_FS_REACTION_NO_EFFECT |
				VMON_TRK2_UV_FS_REACTION_NO_EFFECT |
				VMON_TRK1_OV_FS_REACTION_NO_EFFECT |
				VMON_TRK1_UV_FS_REACTION_NO_EFFECT;

		fs26_setreg(FS26_FS_I_OVUV_SAFE_REACTION2, regval);


		fs26_setreg(FS26_FS_I_NOT_OVUV_SAFE_REACTION2, ~regval);


		regval = WD_ERR_LIMIT_8 | WD_RFR_LIMIT_6 | WD_FS_REACTION_NO_ACTION;


		fs26_setreg(FS26_FS_I_WD_CFG, regval);

		fs26_setreg(FS26_FS_I_NOT_WD_CFG, ~regval);


		regval = FCCU_CFG_NO_MONITORING | ERRMON_ACK_TIME_32MS;

		fs26_setreg(FS26_FS_I_SAFE_INPUTS, regval);
		fs26_setreg(FS26_FS_I_NOT_SAFE_INPUTS, ~regval);

		regval = FLT_ERR_REACTION_NO_EFFECT | CLK_MON_DIS | DIS8S;

		fs26_setreg(FS26_FS_I_FSSM, regval);
		fs26_setreg(FS26_FS_I_NOT_FSSM, ~regval);

		/* Disable watchdog */

		regval = WDW_PERIOD_DISABLE | WDW_DC_62_37 | WDW_RECOVERY_DISABLE;

		fs26_setreg(FS26_FS_WDW_DURATION, regval);
		fs26_setreg(FS26_FS_NOT_WDW_DURATION, ~regval);


		fs26_wdrefresh();


		//debug_printf("FS26 in INIT_FS mode\n");
		//printf("FS26 in INIT_FS mode\r\n");
	}
	else if((FS26_GET_DATA(retval) & FS_STATES_MASK) == FS_STATES_DEBUG_ENTRY)
	{
		//debug_printf("FS26 in DEBUG_ENTRY mode\n");
		//printf("FS26 in DEBUG_ENTRY mode\r\n");
	}
	else if((FS26_GET_DATA(retval) & FS_STATES_MASK) == FS_STATES_NORMAL)
	{
		//debug_printf("FS26 in NORMAL mode\n");
		//printf("FS26 in NORMAL mode\r\n");
	}
	else if((FS26_GET_DATA(retval) & FS_STATES_MASK) == FS_STATES_SAFETY_OUT_NOT)
	{
		//debug_printf("FS26 in Safety Outputs not released\n");
		//printf("FS26 in Safety Outputs not released\r\n");
	}

}


