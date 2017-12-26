// vim: tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>

extern struct usb_interface_descriptor hid_iface;
extern void hid_set_config(usbd_device *dev, uint16_t wValue);
