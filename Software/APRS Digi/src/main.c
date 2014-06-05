/**
 * \file
 * <!--
 * This file was part of BeRTOS and modified by Alejandro Santos.
 *
 * Bertos is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 *
 * Copyright 2010 Develer S.r.l. (http://www.develer.com/)
 *
 * Copyright 2013 Alejandro Santos LU4EXT.
 *
 * -->
 *
 * \author Modified by Alejandro Santos, LU4EXT <alejolp@gmail.com>
 *
 * Based on the BeRTOS work by:
 *
 * \author Francesco Sacchi <batt@develer.com>
 * \author Luca Ottaviano <lottaviano@develer.com>
 * \author Daniele Basile <asterix@develer.com>
 *
 * \brief Arduino APRS Digipeater
 *
 */

#include <cpu/irq.h>
#include <cfg/debug.h>

#include <net/afsk.h>
#include <net/ax25.h>

#include <drv/ser.h>
#include <drv/timer.h>

#include <stdio.h>
#include <string.h>

static Afsk afsk;
static AX25Ctx ax25;
static Serial ser;

#define ADC_CH 0

/* Change this two fields to update your Callsign */
static const char MYCALL[] = "LU4EXT";
static uint8_t MYCALL_SSID = 1;

/* The Digi automatic beacon */
#define APRS_BEACON_MSG    ">LU4EXT Arduino Digipeater http://extradio.sf.net/"
#define APRS_BEACON_TIME (20 * 60) /* In seconds */

/* Don't change this unless you have read the APRS Specs. */
#define CALL_BERTOS_APRS "apzbrt"

/*
 * Print on console the message that we have received.
 */
static void message_callback(struct AX25Msg *msg)
{
	int i, k;

	static AX25Call tmp_path[AX25_MAX_RPT + 2];
	static uint8_t tmp_path_size;
	static const char* relay_calls[] = {"RELAY\x0", "WIDE\x0\x0", "TRACE\x0", 0};


#if 1
	kfile_printf(&ser.fd, "\n\nSRC[%.6s-%d], DST[%.6s-%d]\r\n", msg->src.call, msg->src.ssid, msg->dst.call, msg->dst.ssid);

	for (i = 0; i < msg->rpt_cnt; i++)
		kfile_printf(&ser.fd, "via: [%.6s-%d%s]\r\n", msg->rpt_lst[i].call, msg->rpt_lst[i].ssid, msg->rpt_lst[i].h_bit?"*":"");

	kfile_printf(&ser.fd, "DATA: %.*s\r\n", msg->len, msg->info);
#endif

	if (1) {
		uint8_t repeat = 0;
		uint8_t is_wide = 0, is_trace = 0;

		msg->dst.ssid = 0;
		msg->dst.h_bit = 0;
		memcpy(msg->dst.call, CALL_BERTOS_APRS, 6);

		tmp_path[0] = msg->dst;
		tmp_path[1] = msg->src;

		for (i = 0; i < msg->rpt_cnt; ++i)
			tmp_path[i + 2] = msg->rpt_lst[i];
		tmp_path_size = 2 + msg->rpt_cnt;

		/* Should we repeat the packet?
		 * http://wa8lmf.net/DigiPaths/ */

		/*
		 * This works as follows: for every call on the repeaters list from first to last, find the first
		 * call with the H bit (has-been-repeated) set to false. If this call is RELAY, WIDE or TRACE then
		 * we sould repeat the packet.
		 */

		for (i = 2; i < tmp_path_size; ++i) {
			if (!tmp_path[i].h_bit) {
				AX25Call* c = &tmp_path[i];

				if ((memcmp(tmp_path[i].call, MYCALL, 6) == 0) && (tmp_path[i].ssid == MYCALL_SSID)) {
					repeat = 0;
					break;
				}

				for (k=0; relay_calls[k]; ++k) {
					if (memcmp(relay_calls[k], c->call, 6) == 0) {
						repeat = 1;
						tmp_path[i].h_bit = 1;
						break;
					}
				}

				if (repeat)
					break;

				if (tmp_path[i].ssid > 0) {
					is_wide = memcmp("WIDE", c->call, 4) == 0;
					is_trace = memcmp("TRACE", c->call, 5) == 0;

					if (is_wide || is_trace) {
						repeat = 1;
						tmp_path[i].ssid--;
						if (tmp_path[i].ssid == 0)
							tmp_path[i].h_bit = 1;

						/* Add the Digi Call to the path*/

						if (is_trace && (tmp_path_size < (AX25_MAX_RPT + 2))) {
							for (k = tmp_path_size; k > i; --k) {
								tmp_path[k] = tmp_path[k - 1];
							}
							memcpy(tmp_path[i].call, MYCALL, 6);
							tmp_path[i].ssid = MYCALL_SSID;
							tmp_path[i].h_bit = 1;
							tmp_path_size++;
							i++;
						}

						break;
					}
				}
			} else {
				if ((memcmp(tmp_path[i].call, MYCALL, 6) == 0) && (tmp_path[i].ssid == MYCALL_SSID)) {
					repeat = 1;
					tmp_path[i].h_bit = 1;
					break;
				}
			}
		}

		if ((memcmp(tmp_path[1].call, MYCALL, 6) == 0) && (tmp_path[i].ssid == MYCALL_SSID)) {
			repeat = 0;
		}

		if (repeat) {
			ax25_sendVia(&ax25, tmp_path, tmp_path_size, msg->info, msg->len);
			kfile_print(&ser.fd, "REPEATED\n");
		} else {
			kfile_print(&ser.fd, "NOT REPEATED\n");
		}
	}
}

static void init(void)
{
	IRQ_ENABLE;
	kdbg_init();
	timer_init();

	/*
	 * Init afsk demodulator. We need to implement the macros defined in hw_afsk.h, which
	 * is the hardware abstraction layer.
	 * We do not need transmission for now, so we set transmission DAC channel to 0.
	 */
	afsk_init(&afsk, ADC_CH, 0);
	/*
	 * Here we initialize AX25 context, the channel (KFile) we are going to read messages
	 * from and the callback that will be called on incoming messages.
	 */
	ax25_init(&ax25, &afsk.fd, message_callback);

#if 1
	/* Initialize serial port, we are going to use it to show APRS messages*/
	ser_init(&ser, SER_UART0);
	ser_setbaudrate(&ser, 115200L);
#endif
}

static AX25Call path[] = AX25_PATH(AX25_CALL(CALL_BERTOS_APRS, 0), AX25_CALL("", 0), AX25_CALL("wide1", 1), AX25_CALL("wide2", 2));

int main(void)
{
	init();
	ticks_t start = timer_clock();
	unsigned char x = 0;

	// FIXME
	memcpy(path[1].call, MYCALL, 6);
	path[1].ssid = MYCALL_SSID;

	while (1)
	{
		/* As long as CONFIG_AFSK_RXTIMEOUT is set to 0, this function won't block and return immediately. */
		ax25_poll(&ax25);

#if 1
		/* Send out message every 15sec */
		if (timer_clock() - start > ms_to_ticks(APRS_BEACON_TIME * 1000L))
		{
			kfile_printf(&ser.fd, "Beep %d\n", x++);
			start = timer_clock();
			ax25_sendVia(&ax25, path, countof(path), APRS_BEACON_MSG, sizeof(APRS_BEACON_MSG));
		}
#endif

	}
	return 0;
}
