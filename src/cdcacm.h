// vim: tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab

#include <libopencm3/usb/cdc.h>

extern const struct usb_interface_descriptor uart_comm_iface[];
extern const struct usb_interface_descriptor uart_data_iface[];
extern const struct usb_iface_assoc_descriptor uart_assoc;

void cdcacm_set_config(usbd_device *dev, uint16_t wValue);
