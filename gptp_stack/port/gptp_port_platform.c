/*
 * Copyright 2023-2024 NXP
 *
 * NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading, installing, activating and/or otherwise
 * using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/**
 * @file gptp_port_platform.c
 *
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "gptp_port_platform.h"
#include "gptp_port.h"
#include "stdbool.h"
#include "enet.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* None */

/*******************************************************************************
 * Data types
 ******************************************************************************/
/* None */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* None */

/*******************************************************************************
 * Variables
 ******************************************************************************/

 static volatile uint64_t svu64FreeRunningGptpTimer;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief           This function processes egress timestamp.
 *
 * @details         This function processes captured egress timestamp.
 *                  Only call this function from EthIf_TxConfirmation.
 *
 * @param[in]       u8CtrlIdx Index of the controller which transmitted
 *                  the frame.
 * @param[in]       u32BufferIndex Index of the transmitted data buffer.
 * @param[in]       eStatus Result.
 *
 * @ requirements 529199
 * @ requirements 529270
*/
void GPTP_PORT_TxConfirmation(uint8_t u8CtrlIdx,
                              Eth_BufIdxType_g u32BufferIndex,
                              Std_ReturnType eStatus,
							  Gmac_Ip_TimestampType Timestamp)
{
    static Eth_TimeStampQualType_g seTimeStampQuality;
    static Gmac_Ip_TimestampType     srEgressTimeStamp;
    static uint8_t               su8GptpPort;
    Std_ReturnType               eStatusEgressTS;

    if ((Std_ReturnType)E_OK == eStatus)
    {
    	/*DONE change*/
        /*eStatusEgressTS = Eth_43_GMAC_GetEgressTimeStamp(u8CtrlIdx,
                                                         u32BufferIndex,
                                                         &seTimeStampQuality,
                                                         &srEgressTimeStamp);*/

    	/*get egress timestamp*/
    	srEgressTimeStamp = Timestamp;
    	eStatusEgressTS = E_OK;

        if (((Std_ReturnType)E_OK == eStatusEgressTS) &&
            (ETH_VAL == seTimeStampQuality))
        {
            /* Get gPTP port ID from map table. */
            if (GPTP_ERR_OK == GPTP_PORT_PortLookup(&su8GptpPort, u8CtrlIdx, 0u))
            {
                /* Timestamp captured correctly, to be used and passed to
                   the gPTP stack. Eth_43 driver not provides secondsHi. */
                GPTP_UpdateTimestampEntry(su8GptpPort, u32BufferIndex,
                                          srEgressTimeStamp.seconds,
                                          srEgressTimeStamp.nanoseconds);
            }
        }
        else
        {
            /* No timestamp available, or error occured during the timestamp
               query. */
        	printf("No timestamp available on Txconfirmation!\r\n");
        }
    }
}

/*!
 * @brief           This function processes the timestamp of a received frames.
 *
 * @details         This function processes the timestamp of a received frames
 *                  and passes the frame to the stack for furthure processing
 *                  to the upper layer of gPTP stack.
 *
 * @note            The passed data buffer is no longer valid after the function
 *                  is exited.
 *
 * @warning         This is an example implementation of function passing
 *                  received frames with timestamp to the gPTP stack.
 *
 * @param[in]       u8CtrlIdx Index of the controller which received the frame.
 * @param[in]       u16FrameType The received frame Ethertype
 *                  (from the frame header).
 * @param[in]       bIsBroadcast The value TRUE indicates that the received
 *                  frame was sent to broadcast address (ff-ff-ff-ff-ff-ff).
 * @param[in]       cpu8PhysAddr Pointer to received frame source MAC address
 *                  (6 bytes).
 * @param[in]       cpu8Data Data buffer containing the received Ethernet frame
 *                  payload.
 * @param[in]       u16LenByte Length of the data in the buffer cpu8Data.
 *
 * @ requirements 529199
 * @ requirements 529269
*/
void GPTP_PORT_RxIndication(uint8_t u8CtrlIdx,
                            Eth_FrameType_g u16FrameType,
                            bool bIsBroadcast,
                            const uint8_t *cpu8PhysAddr,
                            const Eth_DataType_g *cpu8Data,
                            uint16_t u16LenByte,
							Gmac_Ip_TimestampType Timestamp)
{
    (void)bIsBroadcast;
    (void)u16LenByte;

    static Eth_TimeStampQualType_g seTimeStampQuality = ETH_UNCERT;
    static Gmac_Ip_TimestampType     srIngressTimeStamp;
    static uint8_t               su8GptpPort;
    gptp_def_rx_data_t           rRxData;
    //Std_ReturnType               eStatus;
    uint64_t                     u64MacAddress;

    /* DONE: change by passing the timestamp from the MAC*/
    /*eStatus = Eth_43_GMAC_GetIngressTimeStamp(u8CtrlIdx, cpu8Data,
                                              &seTimeStampQuality,
                                              &srIngressTimeStamp);*/
    //get_ts_ingress_data(&srIngressTimeStamp);
    //srIngressTimeStamp = Timestamp;// USED IN WB BECAUSE NOT CABABLE OF HA TIMESTAMPING
    if ((0U == Timestamp.seconds) && (0U == Timestamp.nanoseconds)){
		seTimeStampQuality = ETH_INVAL;
	}
    else{
    	seTimeStampQuality = ETH_VAL;
    }

    rRxData.cpu8RxData = cpu8Data;
    rRxData.u16EthType = (uint16_t)u16FrameType;

    if (ETH_VAL == seTimeStampQuality)
    {
        /* Eth_43 driver not provides secondsHi. */
        rRxData.u32TsSec = Timestamp.seconds;
        rRxData.u32TsNsec = Timestamp.nanoseconds;
    }
    else
    {
        /* No timestamp available, or error occured during the timestamp
           query. */
        rRxData.u32TsSec = 0u;
        rRxData.u32TsNsec = 0u;
    }

    /* Get gPTP port ID from map table. */
    if (GPTP_ERR_OK == GPTP_PORT_PortLookup(&su8GptpPort, u8CtrlIdx, 0u))
    {
        rRxData.u8PtpPort = su8GptpPort;
        u64MacAddress = (uint64_t)((((uint64_t)cpu8PhysAddr[0]) << 40u) | \
                        (((uint64_t)cpu8PhysAddr[1]) << 32u) | \
                        (((uint64_t)cpu8PhysAddr[2]) << 24u) | \
                        (((uint64_t)cpu8PhysAddr[3]) << 16u) | \
                        (((uint64_t)cpu8PhysAddr[4]) << 8u)  | \
                        ((uint64_t)cpu8PhysAddr[5]));
        rRxData.u64SourceMac = u64MacAddress;
        GPTP_MsgReceive(rRxData);
    }
}

/*!
 * @brief           This function increments the internal FreeRunning timer
 *                  value.
 *
 * @param[in]       u64Ellapsed Ellapsed time from previous call.
 *
* @ requirements 529199
* @ requirements 539911
*/
void GPTP_PORT_IncFreeRunningTimer(uint64_t u64Ellapsed)
{
    svu64FreeRunningGptpTimer += u64Ellapsed;
}

/*!
 * @brief           This function returns the internal FreeRunning timer value.
 *
 * @return          FreeRunning timer current value - uint64_t.
 *
* @ requirements 529199
* @ requirements 539912
*/
uint64_t GPTP_PORT_GetFreeRunningTimer(void)
{
    return svu64FreeRunningGptpTimer;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
