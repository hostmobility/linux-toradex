/*
 * linux/flexray.h
 *
 * Definitions for FlexRay network layer (socket addr / FlexRay frame)
 *
 */

#ifndef FLEXRAY_H
#define FLEXRAY_H

#include <linux/types.h>
#include <linux/socket.h>

/* FlexRay kernel definitions */

/* special address description flags for the FLEXRAY_ID */
#define FLEXRAY_PPI_FLAG	0x40 /* Payload preamble indicator */
#define FLEXRAY_NFI_FLAG	0x20 /* Null frame indikator */
#define FLEXRAY_SYN_FLAG	0x10 /* Sync frame indikator */
#define FLEXRAY_SFI_FLAG	0x08 /* Startup frame indikator */

/*
 * FlexRay Frame structure (Note: This is updated below)
 *
 * Bit 0     reserved
 * Bit 1     Payload preamble indicator
 * Bit 2     Null frame indikator
 * Bit 3     Sync frame indikator
 * Bit 4     Startup frame indikator
 * Bit 5-15  Frame ID
 * Bit 16-22 Payload length
 * Bit 23-33 Header CRC
 * Bit 34-39 Cycle count
 * PAYLOAD [0-254 Bytes]
 * 24 Bit CRC
 */

/* define Flexray Header Type for Filters */
typedef __u64 flexray_header_t;

/**
 * struct flexray_frame - basic FlexRay frame structure
 * @head:      Header
 * @data:       the FlexRay frame payload.
 * @crc:       the FlexRay frame CRC
 */

struct flexray_frame {
   __u16    frame_id;   // frame_id/slotid
   __u16    flags;      //
   __u8     len;        // 0-127 words in data[] area
   __u8     cc;         // cycle count
   __u8     offset;     // offset counter
   __u8     repetition; // repetition
   __u64    timestamp;  // Perhaps not needed, but since packet is created on external board, it might...
   __u8     header_crc; //
   __u8     data[254] __attribute__((aligned(8)));  //
   __u8     crc[3];     //
};

#if 0
// Old implementation used wrapper functions to access the bit-fields.
static inline int flexray_get_ppi(__u8 *head)
{
	return !!(head[0] & FLEXRAY_PPI_FLAG);
}

static inline void flexray_set_ppi(int ppi, __u8 *head)
{
	if (ppi)
		head[0] |= FLEXRAY_PPI_FLAG;
	else
		head[0] &= ~FLEXRAY_PPI_FLAG;
}

static inline int flexray_get_nfi(__u8 *head)
{
	return !!(head[0] & FLEXRAY_NFI_FLAG);
}

static inline void flexray_set_nfi(int nfi, __u8 *head)
{
	if (nfi)
		head[0] |= FLEXRAY_NFI_FLAG;
	else
		head[0] &= ~FLEXRAY_NFI_FLAG;
}

static inline int flexray_get_syn(__u8 *head)
{
	return !!(head[0] & FLEXRAY_SYN_FLAG);
}

static inline void flexray_set_syn(int syn, __u8 *head)
{
	if (syn)
		head[0] |= FLEXRAY_SYN_FLAG;
	else
		head[0] &= ~FLEXRAY_SYN_FLAG;
}

static inline int flexray_get_sfi(__u8 *head)
{
	return !!(head[0] & FLEXRAY_SFI_FLAG);
}

static inline void flexray_set_sfi(int sfi, __u8 *head)
{
	if (sfi)
		head[0] |= FLEXRAY_SFI_FLAG;
	else
		head[0] &= ~FLEXRAY_SFI_FLAG;
}

static inline __u16 flexray_get_id(__u8 *head)
{
	return ((head[0] & 0x3) << 8) | head[1];
}

static inline void flexray_set_id(__u16 id, __u8 *head)
{
	head[0] &= ~0x3;
	head[0] |= (id >> 8) & 0x3;
	head[1] = id & 0xff;
}

static inline __u8 flexray_get_pl(__u8 *head)
{
	return head[2] >> 1;
}

static inline void flexray_set_pl(__u8 pl, __u8 *head)
{
	head[2] &= 0x1;
	head[2] |= pl << 1;
}

static inline __u16 flexray_get_hcrc(__u8 *head)
{
	return ((head[2] & 0x1) << 10) | (head[3] << 2) | (head[4] >> 6);
}

static inline void flexray_set_hcrc(__u16 hcrc, __u8 *head)
{
	head[2] &= 0xfe;
	head[4] &= 0x3f;
	head[2] |= (hcrc >> 10) & 0x1;
	head[3] = (hcrc >> 2) & 0xff;
	head[4] |= (hcrc & 0x3) << 6;
}

static inline __u8 flexray_get_cc(__u8 *head)
{
	return head[4] & 0x3f;
}

static inline void flexray_set_cc(__u8 cc, __u8 *head)
{
	head[4] &= ~0x3f;
	head[4] |= cc & 0x3f;
}
#endif

/* particular protocols of the protocol family PF_FLEXRAY */
#define FLEXRAY_RAW      1 /* RAW sockets */
#define FLEXRAY_NPROTO   2
#if 0
#define FLEXRAY_BCM      2 /* Broadcast Manager */
#define FLEXRAY_TP16  3 /* VAG Transport Protocol v1.6 */
#define FLEXRAY_TP20  4 /* VAG Transport Protocol v2.0 */
#define FLEXRAY_MCNET 5 /* Bosch MCNet */
#define FLEXRAY_ISOTP 6 /* ISO 15765-2 Transport Protocol */
#define FLEXRAY_NPROTO   7
#endif
#define SOL_FLEXRAY_BASE 110

struct sockaddr_flexray {
	sa_family_t	flexray_family;
	int		flexray_ifindex;
#if 0
   union {
      /* transport protocol class address information (e.g. ISOTP) */
      struct { flexray_header_t rx_id, tx_id; } tp;

      /* reserved for future CAN protocols address information */
   } flexray_addr;
#endif
};

struct flexray_filter {
	flexray_header_t flexray_id;
	flexray_header_t flexray_mask;
};

#endif /* FLEXRAY_H */
