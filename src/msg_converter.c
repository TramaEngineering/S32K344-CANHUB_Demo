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

#include "msg_converter.h"
#include <stddef.h>
#include <string.h>
#include "FlexCAN_Ip_HwAccess.h"

static uint8_t sequence_number = 0;

#define htobe32 __builtin_bswap32
#define htobe16 __builtin_bswap16
#define htobe8 __builtin_bswap8

#define be32toh __builtin_bswap32
#define be16toh __builtin_bswap16
#define be8toh __builtin_bswap8

void etherswap(uint16* ethertype) {
	uint16 mask = 0XFF00;
	uint16 high = *ethertype & mask;
	*ethertype = *ethertype << 8;
    *ethertype = *ethertype | high >> 8;
}

static uint8_t calculate_padding_to_quadlet(uint8_t data_len)
{
	uint8_t padding = 0;
	uint8_t remainder = (data_len & 0x03);
	if (remainder > 0)
	{
		padding = 4 - remainder;
	}
	return padding;
}

static uint16_t roundup_octets_to_quadlets(uint16_t octets)
{
	uint16_t quadlets;
	quadlets = (octets >> 2) & 0x3FFF;
	if ((octets & 0x0003) > 0)
	{
		quadlets = quadlets + 1;
	}
	return quadlets;
}

int convert_can_to_avtp_no_can_inst(struct ethernet_frame* out, Flexcan_Ip_MsgBuffType* in)
{
	if (out == NULL || in == NULL)
		return -1;

	int size = ETHERNET_FRAME_MAC_HEADER_SIZE + sizeof (out->ether_type);
	uint8_t data_len_padding = calculate_padding_to_quadlet(in->dataLen);
	int ntscf_data_len = AVTP_ACF_HEADER_LEN + AVTP_ACF_CAN_BRIEF_HEADER_LEN +
			in->dataLen + data_len_padding;
	int acf_msg_length = roundup_octets_to_quadlets(ntscf_data_len);

	out->ether_type = htobe16(ETHERNET_ETHERTYPE_AVTP);
	/* NTSCF header */
	size += AVTP_NTSCF_HEADER_LEN;
	struct avtp_ntscf_header* ntscf_header = (struct avtp_ntscf_header*)&out->data;
	ntscf_header->subtype = AVTP_SUBTYPE_ACF_NTSCF;
	ntscf_header->sequence_number = sequence_number;
	++sequence_number;
	ntscf_header->ntscf_data_length_lsb = ntscf_data_len & 0xFF;
	ntscf_header->ntscf_data_length_msb = (ntscf_data_len >> 8) & 0x7;

	/* ACF header */
	size += AVTP_ACF_HEADER_LEN;
	struct avtp_acf_header* acf_header = (struct avtp_acf_header*)&out->data[AVTP_NTSCF_HEADER_LEN];
	acf_header->acf_msg_type = AVTP_ACF_MSG_TYPE_CAN_BRIEF;
	acf_header->acf_msg_length_lsb = acf_msg_length;
	acf_header->acf_msg_length_msb = acf_msg_length >> 8;

	/* ACF message */
	size += AVTP_ACF_CAN_BRIEF_HEADER_LEN;
	struct acf_can_brief_msg* acf_can_brief_msg = (struct acf_can_brief_msg*)&out->data[AVTP_NTSCF_HEADER_LEN + AVTP_ACF_HEADER_LEN];

	acf_can_brief_msg->header.eff = ((in->cs & FLEXCAN_IP_CS_IDE_MASK) != 0);
	acf_can_brief_msg->header.brs = ((in->cs & FLEXCAN_IP_MB_BRS_MASK) != 0);
	acf_can_brief_msg->header.rtr = ((in->cs & FLEXCAN_IP_CS_RTR_MASK) != 0);
	acf_can_brief_msg->header.fdf = ((in->cs & FLEXCAN_IP_MB_EDL_MASK) != 0);
	acf_can_brief_msg->header.can_identifier = htobe32(in->msgId & 0x1FFFFFFF); //29 bits
	acf_can_brief_msg->header.pad = data_len_padding;
	//acf_can_brief_msg->header.can_bus_id = instance;
	size += in->dataLen + data_len_padding;
	memcpy(acf_can_brief_msg->payload, &in->data, in->dataLen);

	return size;
}

int convert_can_to_avtp_ntscf(uint8 instance, struct ethernet_frame* out, Flexcan_Ip_MsgBuffType* in)
{
	if (out == NULL || in == NULL)
		return -1;

	int size = ETHERNET_FRAME_MAC_HEADER_SIZE + sizeof (out->ether_type);
	uint8_t data_len_padding = calculate_padding_to_quadlet(in->dataLen);
	int ntscf_data_len = AVTP_ACF_HEADER_LEN + AVTP_ACF_CAN_BRIEF_HEADER_LEN +
			in->dataLen + data_len_padding;
	int acf_msg_length = roundup_octets_to_quadlets(ntscf_data_len);

	out->ether_type = htobe16(ETHERNET_ETHERTYPE_AVTP);
	/* NTSCF header */
	size += AVTP_NTSCF_HEADER_LEN;
	struct avtp_ntscf_header* ntscf_header = (struct avtp_ntscf_header*)&out->data;
	ntscf_header->subtype = AVTP_SUBTYPE_ACF_NTSCF;
	ntscf_header->sequence_number = sequence_number;
	++sequence_number;
	ntscf_header->ntscf_data_length_lsb = ntscf_data_len & 0xFF;
	ntscf_header->ntscf_data_length_msb = (ntscf_data_len >> 8) & 0x7;

	/* ACF header */
	size += AVTP_ACF_HEADER_LEN;
	struct avtp_acf_header* acf_header = (struct avtp_acf_header*)&out->data[AVTP_NTSCF_HEADER_LEN];
	acf_header->acf_msg_type = AVTP_ACF_MSG_TYPE_CAN_BRIEF;
	acf_header->acf_msg_length_lsb = acf_msg_length;
	acf_header->acf_msg_length_msb = acf_msg_length >> 8;

	/* ACF message */
	size += AVTP_ACF_CAN_BRIEF_HEADER_LEN;
	struct acf_can_brief_msg* acf_can_brief_msg = (struct acf_can_brief_msg*)&out->data[AVTP_NTSCF_HEADER_LEN + AVTP_ACF_HEADER_LEN];

	acf_can_brief_msg->header.eff = ((in->cs & FLEXCAN_IP_CS_IDE_MASK) != 0);
	acf_can_brief_msg->header.brs = ((in->cs & FLEXCAN_IP_MB_BRS_MASK) != 0);
	acf_can_brief_msg->header.rtr = ((in->cs & FLEXCAN_IP_CS_RTR_MASK) != 0);
	acf_can_brief_msg->header.fdf = ((in->cs & FLEXCAN_IP_MB_EDL_MASK) != 0);
	acf_can_brief_msg->header.can_identifier = htobe32(in->msgId & 0x1FFFFFFF); //29 bits
	acf_can_brief_msg->header.pad = data_len_padding;
	acf_can_brief_msg->header.can_bus_id = instance;
	size += in->dataLen + data_len_padding;
	memcpy(acf_can_brief_msg->payload, &in->data, in->dataLen);

	return size;
}

int avtp_ntsf_to_can(Flexcan_Ip_MsgBuffType* out, const uint8_t* ntscf_frame)
{
	int ret = 0;
	// assuming there is only one CAN message in AVTP packet
	const struct avtp_acf_header* acf_header = (const struct avtp_acf_header*)&ntscf_frame[0];
	switch (acf_header->acf_msg_type)
	{
	case AVTP_ACF_MSG_TYPE_CAN_BRIEF:
	{
		uint16_t acf_can_msg_len = ((((acf_header->acf_msg_length_msb << 8) & 0x100) | acf_header->acf_msg_length_lsb) << 2) & 0xFFFC;
		const struct acf_can_brief_msg* acf_can_brief_msg = (const struct acf_can_brief_msg*)&ntscf_frame[AVTP_ACF_HEADER_LEN];
		out->msgId = be32toh(acf_can_brief_msg->header.can_identifier);
		out->dataLen = (uint8_t)(acf_can_msg_len - (AVTP_ACF_HEADER_LEN + AVTP_ACF_CAN_BRIEF_HEADER_LEN) - acf_can_brief_msg->header.pad);

		if(out->dataLen > 64) {
			ret = -1;
		} else {
			memcpy(out->data, &ntscf_frame[AVTP_ACF_HEADER_LEN + AVTP_ACF_CAN_BRIEF_HEADER_LEN], out->dataLen);
			out->cs = 0;

			if(acf_can_brief_msg->header.eff)
				out->cs |= FLEXCAN_IP_CS_IDE_MASK;

			if(acf_can_brief_msg->header.brs)
				out->cs |= FLEXCAN_IP_MB_BRS_MASK;

			if(acf_can_brief_msg->header.rtr)
				out->cs |= FLEXCAN_IP_CS_RTR_MASK;

			if(acf_can_brief_msg->header.fdf)
				out->cs |= FLEXCAN_IP_MB_EDL_MASK;

			ret = acf_can_brief_msg->header.can_bus_id;

			break;
		}
	}
	default:
		ret = -1;
	}
	return ret;
}

int convert_ethernet_to_can(Flexcan_Ip_MsgBuffType* out, const struct ethernet_frame* ether_frame)
{
	if (out == NULL || ether_frame == NULL)
		return -1;

	int ret = -1;
	/*there is some problem on ethertype get on the struct: it jumps a bit and reverse the number*/
	/*buffer is only 128 byte long and it has to reconstruct the message piece by piece */
	etherswap(&ether_frame->ether_type);
	if (ether_frame->ether_type == ETHERNET_ETHERTYPE_AVTP_BE)
	{
		const struct avtp_ntscf_header* header = (const struct avtp_ntscf_header*)&ether_frame->data[0];
		switch (header->subtype)
		{
		case AVTP_SUBTYPE_ACF_NTSCF:
			ret = avtp_ntsf_to_can(out, &ether_frame->data[AVTP_NTSCF_HEADER_LEN]);
			break;
		default:
			ret = -1;
		}
	}
	else {

	}
	return ret;
}

