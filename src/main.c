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
#include <string.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/usb/cdc.h>

#include "cdcacm.h"
#include "hid.h"
#include "hex_utils.h"
#include "version.h"
#include "flash.h"

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
	"satoshinm",
	"Pill Duck",
	"ABC",
	"Pill Duck UART Port",
};

static uint32_t report_index = 0;

// Section of flash memory for storing the user payload data - this should match the
// size defined in the .ld linker script file. Points directly to flash, see below for writing.
__attribute__((__section__(".user_data"))) const struct composite_report
	user_data[sizeof(struct composite_report) / (128 - 8) * 1024];


// RAM to temporarily store composite reports when converting, before writing to flash above
// This fits one flash page (1 KB)
static struct composite_report packet_buffer[1024 / sizeof(struct composite_report)] = {0};


/*
void reset_packet_buffer(void)
{
	packets[0].report_id = REPORT_ID_END;
	report_index = 0;
}

void add_keyboard_spammer(int scancode)
{
	int j = report_index;

	packets[j].report_id = REPORT_ID_KEYBOARD;
	packets[j].keyboard.modifiers = 0;
	packets[j].keyboard.reserved = 0;
	packets[j].keyboard.keys_down[0] = scancode;
	packets[j].keyboard.keys_down[1] = 0;
	packets[j].keyboard.keys_down[2] = 0;
	packets[j].keyboard.keys_down[3] = 0;
	packets[j].keyboard.keys_down[4] = 0;
	packets[j].keyboard.keys_down[5] = 0;
	packets[j].keyboard.leds = 0;
	++j;

	packets[j].report_id = REPORT_ID_END;

	report_index = j;
}
*/

// Convert a compiled DuckyScript code to USB HID reports
// see: https://github.com/hak5darren/USB-Rubber-Ducky/blob/33a834b0e19f9d4f995432eb9dbcccb247c2e4df/Firmware/Source/Ducky_HID/src/main.c#L143
int convert_ducky_binary(uint8_t *buf, int len, struct composite_report *out)
{
	int j = 0;

	// 16-bit words, must be even
	if ((len % 2) != 0) len -= 1;

	for (int i = 0; i < len; i += 2) {
		uint16_t word = buf[i] | (buf[i + 1] << 8);

		if ((word & 0xff) == 0) {
			// TODO: wait
			continue;
		}

		// Press key and modifier
		out[j].report_id = REPORT_ID_KEYBOARD;
		out[j].keyboard.modifiers = word >> 8;
		out[j].keyboard.reserved = 1;
		out[j].keyboard.keys_down[0] = word & 0xff;
		out[j].keyboard.keys_down[1] = 0;
		out[j].keyboard.keys_down[2] = 0;
		out[j].keyboard.keys_down[3] = 0;
		out[j].keyboard.keys_down[4] = 0;
		out[j].keyboard.keys_down[5] = 0;
		out[j].keyboard.leds = 0;
		++j;

		// Release key
		out[j].report_id = REPORT_ID_KEYBOARD;
		out[j].keyboard.modifiers = 0;
		out[j].keyboard.reserved = 1;
		out[j].keyboard.keys_down[0] = 0;
		out[j].keyboard.keys_down[1] = 0;
		out[j].keyboard.keys_down[2] = 0;
		out[j].keyboard.keys_down[3] = 0;
		out[j].keyboard.keys_down[4] = 0;
		out[j].keyboard.keys_down[5] = 0;
		out[j].keyboard.leds = 0;
		++j;
	}

	out[j].report_id = REPORT_ID_END;
	++j;

	return j;
}

static bool paused = true;
static bool single_step = false;
void sys_tick_handler(void)
{
	if (paused && !single_step) return;

	struct composite_report report = user_data[report_index];
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

	uint16_t bytes_written = 0;
	do {
		bytes_written = usbd_ep_write_packet(usbd_dev, 0x81, &report, len);
	} while (bytes_written == 0);
	gpio_toggle(GPIOC, GPIO13);

	if (single_step) {
		single_step = false;
		paused = true;
	}

	report_index += 1;
}


static void usb_set_config(usbd_device *dev, uint16_t wValue)
{
	hid_set_config(dev, wValue);
	cdcacm_set_config(dev, wValue);
}

int add_mouse_jiggler(int width)
{
	int j = 0;
	for (int i = 0; i < width; ++i) {
		packet_buffer[j].report_id = REPORT_ID_MOUSE;
		packet_buffer[j].mouse.buttons = 0;
		packet_buffer[j].mouse.x = 1;
		packet_buffer[j].mouse.y = 0;
		packet_buffer[j].mouse.wheel = 0;
		++j;
	}

	for (int i = 0; i < width; ++i) {
		packet_buffer[j].report_id = REPORT_ID_MOUSE;
		packet_buffer[j].mouse.buttons = 0;
		packet_buffer[j].mouse.x = -1;
		packet_buffer[j].mouse.y = 0;
		packet_buffer[j].mouse.wheel = 0;
		++j;
	}

	packet_buffer[j].report_id = REPORT_ID_END;
	++j;

	return j;
}

char *process_serial_command(char *buf, int len) {
	(void) len;

	if (buf[0] == 'v') {
		return "Pill Duck version " FIRMWARE_VERSION;
	} else if (buf[0] == '?') {
		return "see source code for help";
	/* TODO: help, but too big for one packet
		return "help:\r\n"
			"?\tshow this help\r\n"
			"v\tshow firmware version\r\n"
			"w<hex>\twrite flash data\r\n"
			"d<hex>\twrite compiled DuckyScript flash data\r\n"
			"j\twrite mouse jiggler to flash data\r\n"
			"r\tread flash data\r\n"
			"@\tshow current report index\r\n"
			"p\tpause/resume execution\r\n"
			"s\tsingle step execution\r\n"
			"z\treset report index to zero\r\n"
			;
	*/
	} else if (buf[0] == 'w' || buf[0] == 'd') {
		char binary[128] = {0};
		int binary_len = len / 2;
		uint8_t *to_write = (uint8_t *)&binary;

		unhexify(binary, &buf[1], len);

		if (buf[0] == 'd') {
			int records = convert_ducky_binary((uint8_t *)binary, binary_len, packet_buffer);
			binary_len = records * sizeof(struct composite_report);
			to_write = (uint8_t *)&packet_buffer;
		}

		int result = flash_program_data((uint32_t)&user_data, to_write, binary_len);
		if (result == RESULT_OK) {
			return "wrote flash";
		} else if (result == FLASH_WRONG_DATA_WRITTEN) {
			return "wrong data written";
		} else {
			return "error writing flash";
		}
	} else if (buf[0] == 'j') {
		int records = add_mouse_jiggler(30);
		int binary_len = records * sizeof(struct composite_report);

		int result = flash_program_data((uint32_t)&user_data, (uint8_t *)&packet_buffer, binary_len);
		if (result == RESULT_OK) {
			return "wrote flash";
		} else if (result == FLASH_WRONG_DATA_WRITTEN) {
			return "wrong data written";
		} else {
			return "error writing flash";
		}
	} else if (buf[0] == 'r') {
		char binary[16] = {0};
		memset(binary, 0, sizeof(binary));
		flash_read_data((uint32_t)&user_data, sizeof(binary), (uint8_t *)&binary);

		static char hex[32] = {0};
		hexify(hex, (const char *)binary, sizeof(binary));
		return hex;
	} else if (buf[0] == '@') {
		static char hex[16] = {0};
		// TODO: show in decimal and correct endian
		hexify(hex, (const char *)&report_index, sizeof(report_index));
		return hex;
	} else if (buf[0] == 'p') {
		paused = !paused;
		if (paused) return "paused";
		else return "resumed";
	} else if (buf[0] == 's') {
		single_step = true;
		return "step";
	} else if (buf[0] == 'z') {
		report_index = 0;
	} else {
		return "invalid command, try ? for help";
	}

	return "";
}

static void setup_clock(void) {
	rcc_clock_setup_in_hsi_out_48mhz();
	rcc_periph_clock_enable(RCC_GPIOC);

	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	/* SysTick interrupt every N clock pulses: set reload to N-1
	 * Period: N / (72 MHz / 8 )
	 * */
	systick_set_reload(89999); // 10 ms
	//systick_set_reload(899999); // 100 ms
	systick_interrupt_enable();
	systick_counter_enable();
}

static void setup_gpio(void) {
	// Built-in LED on blue pill board, PC13
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
	gpio_set(GPIOC, GPIO13);

}

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

int main(void)
{
	setup_clock();
	setup_gpio();

	//add_mouse_jiggler(30);
	//add_keyboard_spammer(6); // 'c'

	// Ddde
	//add_ducky_binary((uint8_t *)"\x07\x02\x07\x00\x07\x00\x08\x00", 8);

	// Hello, world!
	/*
	convert_ducky_binary((uint8_t *)
		"\x00\xff\x00\xff\x00\xff\x00\xeb\x0b\x02\x08\x00\x0f\x00\x0f\x00"
		"\x12\x00\x36\x00\x2c\x00\x1a\x00\x12\x00\x15\x00\x0f\x00\x07\x00"
		"\x1e\x02\x00\xff\x00\xf5\x28\x00", 36);
	*/

	if (user_data[0].report_id != REPORT_ID_END) {
		paused = false;
	}

	usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config, usb_strings,
		sizeof(usb_strings)/sizeof(char *),
		usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, usb_set_config);

	while (1)
		usbd_poll(usbd_dev);
}

