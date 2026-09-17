#include <linux/module.h> 
#include <linux/usb.h> 

#define VID	0x0781
#define PID	0x5591

static const struct usb_device_id my_usb_table[] = {
	{ USB_DEVICE(VID, PID)}, 
	{ }
};
MODULE_DEVICE_TABLE(usb, my_usb_table);

static int my_usb_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);

	printk(KERN_INFO "my_usb: device plugged in\n");
	printk(KERN_INFO "my_usb: VID: %04x | PID: %04x \n", dev->descriptor.idVendor, dev->descriptor.idProduct);

	return 0;
}

static void my_usb_disconnect(struct usb_interface *intf)
{
	printk(KERN_INFO "my_usb: device unplugged\n");
	return; 
}


static struct usb_driver my_usb_driver = {
	.name		= "my_usb", 
	.id_table	= my_usb_table, 
	.probe		= my_usb_probe, 
	.disconnect	= my_usb_disconnect,
};

module_usb_driver(my_usb_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("humphrey");
MODULE_DESCRIPTION("A simple USB driver");