/*
 * Copyright (c) 2014 David Jander, Protonic Holland
 * Copyright (C) 2014-2016 Pengutronix, Marc Kleine-Budde <kernel@pengutronix.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/circ_buf.h>
#include <linux/can/dev.h>
#include <linux/can/rx-fifo.h>

static int can_rx_fifo_napi_read_frame(struct can_rx_fifo *fifo, int index)
{
	struct net_device *dev = fifo->dev;
	struct net_device_stats *stats = &dev->stats;
	struct sk_buff *skb;
	struct can_frame *cf;

	skb = alloc_can_skb(dev, &cf);
	if (unlikely(!skb)) {
		stats->rx_dropped++;
		return 0;
	}

	memcpy(cf, &fifo->ring[index], sizeof(*cf));
	memset(&fifo->ring[index], 0x0, sizeof(*cf));

	stats->rx_packets++;
	stats->rx_bytes += cf->can_dlc;
	netif_receive_skb(skb);

	return 1;
}

static int can_rx_fifo_napi_poll(struct napi_struct *napi, int quota)
{
	struct can_rx_fifo *fifo = container_of(napi, struct can_rx_fifo, napi);
	unsigned int tail;
	int work_done = 0;

	if (fifo->poll_pre_read && work_done < quota)
		work_done += fifo->poll_pre_read(fifo);

	/* handle mailboxes */
	tail = fifo->ring_tail;
	while ((CIRC_CNT(smp_load_acquire(&fifo->ring_head), tail, fifo->ring_size)) &&
	       (work_done < quota)) {
		work_done += can_rx_fifo_napi_read_frame(fifo, tail);
		tail++;
		tail &= fifo->ring_size -1;
		smp_store_release(&fifo->ring_tail, tail);
	}

	if (fifo->poll_post_read && work_done < quota)
		work_done += fifo->poll_post_read(fifo);

	if (work_done < quota) {
		unsigned int head;

		napi_complete(napi);

		/* Check if there was another interrupt */
		head = smp_load_acquire(&fifo->ring_head);
		if (((CIRC_CNT(head, tail, fifo->ring_size)) || fifo->poll_errors) &&
		    napi_reschedule(&fifo->napi)) {
			fifo->poll_errors = false;
		}

		if (fifo->poll_error_interrupts_enable)
			fifo->poll_error_interrupts_enable(fifo);
	}

	can_led_event(fifo->dev, CAN_LED_EVENT_RX);

	return work_done;
}

static unsigned int can_rx_fifo_offload_one(struct can_rx_fifo *fifo, unsigned int n)
{
	unsigned int head, tail;
	unsigned int ret;

	head = fifo->ring_head;
	tail = ACCESS_ONCE(fifo->ring_tail);
	if (CIRC_SPACE(head, tail, fifo->ring_size)) {
		ret = fifo->mailbox_read(fifo, &fifo->ring[head], n);
		if (ret) {
			head++;
			head &= fifo->ring_size - 1;
			smp_store_release(&fifo->ring_head, head);
		}
	} else {
		/* Circular buffer is fill, read to discard mailbox */
		ret = fifo->mailbox_read(fifo, &fifo->overflow, n);
		if (ret)
			fifo->dev->stats.rx_dropped++;
	}

	return ret;
}

int can_rx_fifo_irq_offload_simple(struct can_rx_fifo *fifo)
{
	unsigned int received = 0;
	unsigned int ret;

	do {
		ret = can_rx_fifo_offload_one(fifo, 0);
		received += ret;
	} while (ret);

	if (received)
		can_rx_fifo_schedule(fifo);

	return received;
}
EXPORT_SYMBOL_GPL(can_rx_fifo_irq_offload_simple);

static int can_rx_fifo_init_ring(struct net_device *dev,
				 struct can_rx_fifo *fifo, unsigned int weight)
{
	fifo->dev = dev;

	/* Make ring-buffer a sensible size that is a power of 2 */
	fifo->ring_size = 2 << fls(weight);
	fifo->ring = kzalloc(sizeof(struct can_frame) * fifo->ring_size,
			     GFP_KERNEL);
	if (!fifo->ring)
		return -ENOMEM;

	fifo->ring_head = fifo->ring_tail = 0;
	netif_napi_add(dev, &fifo->napi, can_rx_fifo_napi_poll, weight);

	return 0;
}

int can_rx_fifo_add_simple(struct net_device *dev, struct can_rx_fifo *fifo, unsigned int weight)
{
	if (!fifo->mailbox_read)
		return -EINVAL;

	return can_rx_fifo_init_ring(dev, fifo, weight);
}
EXPORT_SYMBOL_GPL(can_rx_fifo_add_simple);

void can_rx_fifo_enable(struct can_rx_fifo *fifo)
{
	napi_enable(&fifo->napi);
}
EXPORT_SYMBOL_GPL(can_rx_fifo_enable);

void can_rx_fifo_irq_error(struct can_rx_fifo *fifo)
{
	fifo->poll_errors = true;
	can_rx_fifo_schedule(fifo);
}
EXPORT_SYMBOL_GPL(can_rx_fifo_irq_error);

void can_rx_fifo_del(struct can_rx_fifo *fifo)
{
	netif_napi_del(&fifo->napi);
	kfree(fifo->ring);
}
EXPORT_SYMBOL_GPL(can_rx_fifo_del);
