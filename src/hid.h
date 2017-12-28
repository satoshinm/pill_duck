// vim: tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>

extern struct usb_interface_descriptor hid_iface;
extern void hid_set_config(usbd_device *dev, uint16_t wValue);

// Structure of HID report packets, must match hid_report_descriptor below
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

		uint8_t padding[15];
	};
} __attribute__((packed));

// Not real report IDs, used as pseudocommands when processing the packets array
#define REPORT_ID_NOP		0
#define REPORT_ID_DELAY		254
#define REPORT_ID_END		255


#ifdef INCLUDE_PACKET_DESCRIPTOR // defined here only for proximity to above variables which must match
// For descriptor format see http://eleccelerator.com/tutorial-about-usb-hid-report-descriptors/
// Generated using http://eleccelerator.com/usbdescreqparser/
static const uint8_t hid_report_descriptor[] = {
	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
	0x09, 0x06,        // Usage (Keyboard)
	0xA1, 0x01,        // Collection (Application)
	0x85, 0x01,        //   Report ID (1)
	0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
	0x19, 0xE0,        //   Usage Minimum (0xE0)
	0x29, 0xE7,        //   Usage Maximum (0xE7)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x01,        //   Logical Maximum (1)
	0x75, 0x01,        //   Report Size (1)
	0x95, 0x08,        //   Report Count (8)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x19, 0x00,        //   Usage Minimum (0x00)
	0x29, 0x65,        //   Usage Maximum (0x65)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x65,        //   Logical Maximum (101)
	0x75, 0x08,        //   Report Size (8)
	0x95, 0x06,        //   Report Count (6)
	0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x05, 0x08,        //   Usage Page (LEDs)
	0x19, 0x01,        //   Usage Minimum (Num Lock)
	0x29, 0x05,        //   Usage Maximum (Kana)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x01,        //   Logical Maximum (1)
	0x75, 0x01,        //   Report Size (1)
	0x95, 0x05,        //   Report Count (5)
	0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x95, 0x03,        //   Report Count (3)
	0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0xC0,              // End Collection
	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
	0x09, 0x02,        // Usage (Mouse)
	0xA1, 0x01,        // Collection (Application)
	0x85, 0x02,        //   Report ID (2)
	0x09, 0x01,        //   Usage (Pointer)
	0xA1, 0x00,        //   Collection (Physical)
	0x05, 0x09,        //     Usage Page (Button)
	0x19, 0x01,        //     Usage Minimum (0x01)
	0x29, 0x03,        //     Usage Maximum (0x03)
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x01,        //     Logical Maximum (1)
	0x95, 0x03,        //     Report Count (3)
	0x75, 0x01,        //     Report Size (1)
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x95, 0x01,        //     Report Count (1)
	0x75, 0x05,        //     Report Size (5)
	0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
	0x09, 0x30,        //     Usage (X)
	0x09, 0x31,        //     Usage (Y)
	0x09, 0x38,        //     Usage (Wheel)
	0x15, 0x81,        //     Logical Minimum (-127)
	0x25, 0x7F,        //     Logical Maximum (127)
	0x75, 0x08,        //     Report Size (8)
	0x95, 0x03,        //     Report Count (3)
	0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              //   End Collection
	0x09, 0x3C,        //   Usage (Motion Wakeup)
	0x05, 0xFF,        //   Usage Page (Reserved 0xFF)
	0x09, 0x01,        //   Usage (0x01)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x01,        //   Logical Maximum (1)
	0x75, 0x01,        //   Report Size (1)
	0x95, 0x02,        //   Report Count (2)
	0xB1, 0x22,        //   Feature (Data,Var,Abs,No Wrap,Linear,No Preferred State,No Null Position,Non-volatile)
	0x75, 0x06,        //   Report Size (6)
	0x95, 0x01,        //   Report Count (1)
	0xB1, 0x01,        //   Feature (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0xC0,              // End Collection
};
#endif
