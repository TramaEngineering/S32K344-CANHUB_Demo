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

#ifndef MSG_CONVERTER_H
#define MSG_CONVERTER_H

//#include "avtp_defs.h"
#include "ethernet_type.h"
#include "Gmac_Ip.h"
#include "FlexCAN_Ip.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
extern int convert_can_to_avtp_ntscf(uint8 instance, struct ethernet_frame* out, Flexcan_Ip_MsgBuffType* in);
extern int convert_ethernet_to_can(Flexcan_Ip_MsgBuffType* out, const struct ethernet_frame* ether_frame);
extern int convert_can_to_avtp_no_can_inst(struct ethernet_frame* out, Flexcan_Ip_MsgBuffType* in);
void etherswap(uint16* ethertype);
#ifdef __cplusplus
}
#endif
#endif // MSG_CONVERTER_H
