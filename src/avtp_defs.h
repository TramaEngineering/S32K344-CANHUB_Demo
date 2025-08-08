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
#ifndef AVTP_DEFS_H
#define AVTP_DEFS_H

#include <PlatformTypes.h>
#include "Gmac_Ip_Sa_PBcfg.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Ethernet specific definitions  */
#define ETHERNET_FRAME_MAC_HEADER_SIZE 			12
#define	ETHERNET_VLAN_TAG_SIZE					4
#define ETHERNET_VLAN_TAG_ID					0x8100
#define ETHERNET_ETHERTYPE_SIZE					2

#define ETHERNET_ETHERTYPE_IPV4					0x0800
#define ETHERNET_ETHERTYPE_IPV6					0x86DD
#define ETHERNET_ETHERTYPE_AVTP					0x22F0
#define ETHERNET_ETHERTYPE_AVTP_BE				0xF022
#define ETHERNET_ETHERTYPE_MDNS					0x86DD
#define ETHERNET_ETHERTYPE_ARP					0x0806
#define ETHERNET_ETHERTYPE_MAC_SEC				0x88E5
#define ETHERNET_ETHERTYPE_PTP					0x88F7
#define ETHERNET_ETHERTYPE_SOME_IP				0x22EA
#define ETHERNET_ETHERTYP_DOIP					0xA0ED


#define AVTP_SUBTYPE_ACF_TSCF             		0x05
#define AVTP_SUBTYPE_ACF_NTSCF            		0x82
#define AVTP_ACF_MSG_TYPE_CAN					0x01
#define AVTP_ACF_MSG_TYPE_CAN_BRIEF				0x02

#define AVTP_ACF_CAN_PAYLOAD_LEN				64

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

struct ethernet_frame
{
	uint8_t dst_macaddr[ETH_ALEN];
	uint8_t src_macaddr[ETH_ALEN];
	uint16_t ether_type;
	uint8_t data[GMAC_0_MAX_TXBUFFLEN_SUPPORTED - 2 - 2*ETH_ALEN];
} __attribute__ ((__packed__));

struct avtp_ntscf_header
{
	uint32_t subtype:8;            /* subtype */
	uint32_t ntscf_data_length_msb:3; /* NTSCF data length: 11 bits - length of acf_payload_data in octets */
	uint32_t sv:1;                 /* stream_id validation */
	uint32_t version:3;	           /* version - default 000 */
	uint32_t r:1;                  /* reserved */
	uint32_t ntscf_data_length_lsb:8; /* NTSCF data length: 11 bits - length of acf_payload_data in octets */
	uint32_t sequence_number:8;    /* sequence number */
	uint32_t stream_id_low:32;     /* stream id */
	uint32_t stream_id_high:32;    /* stream id */
} __attribute__ ((__packed__));

struct avtp_tscf_header
{
	uint32_t subtype:8;                /* subtype */
	uint32_t sv:1;                     /* stream_id validation */
	uint32_t version:3;                /* version - default 000 */
	uint32_t mr:1;                     /* media clock restart */
	uint32_t rsv:2;                    /* reserved */
	uint32_t tv:1;                     /* avtp timestamp validation */
	uint32_t sequence_number:8;        /* sequence number */
	uint32_t reserved:7;               /* reserved */
	uint32_t tu:1;                     /* timestamp uncertain */
	uint32_t stream_id_low;            /* stream_id */
	uint32_t stream_id_high;           /* stream_id */
	uint32_t avtp_timestamp;           /* avtp timestamp */
	uint32_t reserved1;                /* reserved */
	uint32_t stream_data_length:16;    /* stream data length, in octets */
	uint32_t reserved2:16;             /* reserved */
} __attribute__ ((__packed__));

struct avtp_acf_can_header_timestamps
{
	uint32_t timestamp_left;           /* CAN acquisition time, 8B */
	uint32_t timestamp_right;
} __attribute__ ((__packed__));

struct avtp_acf_header
{
	uint16_t acf_msg_length_msb:1;   /* length of the acf_msg in quadlets including the acf header */
	uint16_t acf_msg_type:7;         /* 0x01 for CAN/CAN FD messages, 0x02 for abbreviated CAN/CAN FD messages (no timestamp field) */
	uint16_t acf_msg_length_lsb:8;   /* length of the acf_msg in quadlets including the acf header */
} __attribute__ ((__packed__));

struct avtp_acf_can_brief_msg_header
{
	uint8_t esi:1;              /* error_state_indicator field. Contains the CAN msg error state indicator bit. */
	uint8_t fdf:1;              /* CAN Flexible Data-rate (FD) format field. Contains the CAN msg Flexible Data-rate format bit. */
	uint8_t brs:1;              /* bit_rate_switch field. Contains the CAN msg bit rate switch bit */
	uint8_t eff:1;              /* extended_frame_format field. Contains the CAN msg extended frame format bit */
	uint8_t rtr:1;              /* remote_transmission_request field. Contains the CAN msg remote transmission req. bit */
	uint8_t mtv:1;              /* message_timestamp_valid field. */
	uint8_t pad:2;              /* padding length at the end of the message in octets to fill a quadlet (32-bits) */

	uint8_t can_bus_id:5;       /* Can_bus_id field. Provides an identifier for the CAN bus on which this msg originated. */
	uint8_t rsv:3;              /* reserved, set to 0. */

	uint32_t can_identifier;	/* can_identifier field 29 bits, bit 30,31,32 are reserved*/

} __attribute__ ((__packed__));

struct acf_can_brief_msg
{
	struct avtp_acf_can_brief_msg_header header;
	uint8_t payload[AVTP_ACF_CAN_PAYLOAD_LEN];
} __attribute__ ((__packed__));


#define AVTP_TSCF_HEADER_LEN                (sizeof(struct avtp_tscf_header))
#define AVTP_NTSCF_HEADER_LEN               (sizeof(struct avtp_ntscf_header))
#define AVTP_ACF_HEADER_LEN                 (sizeof(struct avtp_acf_header))
#define AVTP_ACF_CAN_BRIEF_HEADER_LEN       (sizeof(struct avtp_acf_can_brief_msg_header))
#define AVTP_ACF_CAN_HEADER_TIMESTAMPS_LEN  (sizeof(struct avtp_acf_can_header_timestamps))
#ifdef __cplusplus
}
#endif
#endif // AVTP_DEFS_H
