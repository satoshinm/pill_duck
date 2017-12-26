// vim: tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab
/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/usb/cdc.h>

#include "cdcacm.h"
#include "hid.h"

static usbd_device *usbd_dev;

const struct usb_device_descriptor dev_descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x05ac,
	.idProduct = 0x2227,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};


const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = &hid_iface,
}, {
	.num_altsetting = 1,
	.iface_assoc = &uart_assoc,
	.altsetting = uart_comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = uart_data_iface,
}};

const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = sizeof(ifaces)/sizeof(ifaces[0]),
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0xC0,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

static const char *usb_strings[] = {
	"Black Sphere Technologies",
	"HID Demo",
	"DEMO",
	"Pill Duck UART Port",
};

// Structure of HID report packets, must match hid_report_descriptor
#define REPORT_ID_KEYBOARD	1
#define REPORT_ID_MOUSE		2
struct composite_report {
	uint8_t report_id;
	union {
		struct {
			uint8_t buttons;
			uint8_t x;
			uint8_t y;
			uint8_t wheel;
		} __attribute__((packed)) mouse;

		struct {
			uint8_t modifiers;
			uint8_t reserved;
			uint8_t keys_down[6];
			uint8_t leds;
		} __attribute__((packed)) keyboard;
	};
} __attribute__((packed));

#define REPORT_ID_NOP		0
#define REPORT_ID_END		255

static struct composite_report packets[1024] = {
	{
		.report_id = REPORT_ID_KEYBOARD,
		.keyboard.modifiers = 0,
		.keyboard.reserved = 0,
		.keyboard.keys_down = { 6 /* 'c' */, 0, 0, 0, 0, 0 },
		.keyboard.leds = 0,
	},
	{
		.report_id = REPORT_ID_END,
	},
};

static int report_index = 0;

void sys_tick_handler(void)
{
	struct composite_report report = packets[report_index];
	uint16_t len = 0;
	uint8_t id = report.report_id;

	if (id == REPORT_ID_NOP) {
		return;
	} else if (id == REPORT_ID_KEYBOARD) {
		len = 9;
	} else if (id == REPORT_ID_MOUSE) {
		len = 5;
	} else {
		report_index = 0;
		return;
	}

	usbd_ep_write_packet(usbd_dev, 0x81, &report, len);

	report_index += 1;
}


#if 0
int dir = 1;
bool jiggler = true;
bool spam_keyboard = true;
void sys_tick_handler(void)
{
	static bool toggle = false;

	if (jiggler && toggle) {
		static int x = 0;
		/*
		uint8_t buf[5] = {0, 0, 0, 0, 0};

		buf[0] = 2; // mouse
		buf[2] = dir;
		*/

		struct composite_report report = {
			.report_id = 2, // mouse
			.mouse.buttons = 0,
			.mouse.x = dir,
			.mouse.y = 0,
			.mouse.wheel = 0,
		};

		x += dir;
		if (x > 30)
			dir = -dir;
		if (x < -30)
			dir = -dir;
		usbd_ep_write_packet(usbd_dev, 0x81, &report, 5);
	}

	if (spam_keyboard && !toggle) {
		/*
		uint8_t report[9] = {0};
		report[0] = 1; // keyboard
		report[1] = 0; // no modifiers down
		report[2] = 0;
		report[3] = 0x06; // 'c'
		*/

		struct composite_report report = {
			.report_id = 1, // keyboard
			.keyboard.modifiers = 0,
			.keyboard.reserved = 0,
			.keyboard.keys_down = {
				0x06, // 'c'
				0x00, 0x00, 0x00, 0x00, 0x00 },
			.keyboard.leds = 0,
		};

		usbd_ep_write_packet(usbd_dev, 0x81, &report, 9);
	}

	toggle = !toggle;
}
#endif

static void usb_set_config(usbd_device *dev, uint16_t wValue)
{
	hid_set_config(dev, wValue);
	cdcacm_set_config(dev, wValue);
}

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];


int main(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();

	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
	gpio_set(GPIOC, GPIO13);


	usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config, usb_strings,
		sizeof(usb_strings)/sizeof(char *),
		usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, usb_set_config);

	while (1)
		usbd_poll(usbd_dev);
}

