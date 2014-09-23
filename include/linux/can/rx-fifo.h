/*
 * linux/can/rx-fifo.h
 *
 * Copyright (c) 2014 David Jander, Protonic Holland
 * Copyright (c) 2014-2016 Pengutronix, Marc Kleine-Budde <kernel@pengutronix.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _CAN_RX_FIFO_H
#define _CAN_RX_FIFO_H

#include <linux/netdevice.h>
#include <linux/can.h>

struct can_rx_fifo {
	struct net_device *dev;

	void (*mailbox_enable_mask)(struct can_rx_fifo *fifo, u64 mask);
	void (*mailbox_enable)(struct can_rx_fifo *fifo, unsigned int mb);
	void (*poll_error_interrupts_enable)(struct can_rx_fifo *fifo);
	unsigned int (*mailbox_read)(struct can_rx_fifo *fifo, struct can_frame *cf, unsigned int mb);
	unsigned int (*poll_pre_read)(struct can_rx_fifo *fifo);
	unsigned int (*poll_post_read)(struct can_rx_fifo *fifo);

	u64 mask_low;
	u64 mask_high;
	u64 active;

	unsigned int ring_size;
	unsigned int ring_head;
	unsigned int ring_tail;
	unsigned int low_first;
	unsigned int high_first;
	unsigned int high_last;

	struct can_frame *ring;
	struct can_frame overflow;
	struct napi_struct napi;

	bool inc;
	bool scan_high_first;
	bool poll_errors;
};

int can_rx_fifo_add(struct net_device *dev, struct can_rx_fifo *fifo);
int can_rx_fifo_add_simple(struct net_device *dev, struct can_rx_fifo *fifo, unsigned int weight);
int can_rx_fifo_irq_offload(struct can_rx_fifo *fifo, u64 reg);
int can_rx_fifo_irq_offload_simple(struct can_rx_fifo *fifo);
void can_rx_fifo_irq_error(struct can_rx_fifo *fifo);
void can_rx_fifo_reset(struct can_rx_fifo *fifo);
void can_rx_fifo_del(struct can_rx_fifo *fifo);
void can_rx_fifo_enable(struct can_rx_fifo *fifo);

static inline void can_rx_fifo_schedule(struct can_rx_fifo *fifo)
{
	napi_schedule(&fifo->napi);
}

static inline void can_rx_fifo_disable(struct can_rx_fifo *fifo)
{
	napi_disable(&fifo->napi);
}

#endif /* !_CAN_RX_FIFO_H */
