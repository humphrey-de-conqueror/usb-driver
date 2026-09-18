1. create a device table 
```
#define VID	0x0781
#define PID	0x5591

static const struct usb_device_id my_usb_table[] = {
	{ USB_DEVICE(VID, PID)}, 
	{ }
};
MODULE_DEVICE_TABLE(usb, my_usb_table);
```

2. provide probe function 
```
static int my_usb_probe(struct usb_interface *intf, const struct usb_device_id, *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);

	printk(KERN_INFO "my_usb: device plugged in\n");
	printk(KERN_INFO "my_usb: VID: %04x | PID: %04x", dev->descriptor.idVendor, dev->descriptor.idProduct);

	return 0;
}
```

3. provide disconnect function 
```
static void my_usb_disconnect(struct usb_interface *intf)
{
	printk(KERN_INFO "my_usb: device unplugged\n");
	return; 
}

```

4. register them 
```
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
```

5. find interface and endpoint in probe 
```
static int my_usb_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *iface_desc = intf->cur_altsetting; 
	int i;

	printk(KERN_INFO "my_usb: device plugged in\n");
	printk(KERN_INFO "my_usb: VID: %04x | PID: %04x \n", dev->descriptor.idVendor, dev->descriptor.idProduct);
	printk(KERN_INFO "my_usb: number of endpoints: %d \n", iface_desc->desc.bNumEndpoints);

	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
		struct usb_endpoint_descriptor *endpoint; 
		endpoint = &iface_desc->endpoint[i].desc;

		printk(KERN_INFO "my_usb: endpoint[%d] address: 0x%02x \n", i, endpoint->bEndpointAddress);
		printk(KERN_INFO "my_usb: endpoint[%d] max packet size: %d \n", i, endpoint->wMaxPacketSize);

		if (usb_endpoint_dir_in(endpoint))
			printk(KERN_INFO "my_usb: endpoint[%d] direction: IN\n", i);
		else 
			printk(KERN_INFO "my_usb: endpoint[%d] direction: OUT\n", i);

		if (usb_endpoint_xfer_bulk(endpoint))
			printk(KERN_INFO "my_usb: endpoint[%d] type: BULK \n", i);
	}

	return 0;
}
```

6. io happen at endpoint 
```
struct my_usb_dev {
	struct usb_device *udev;
	__u8 bulk_in_addr;
	__u8 bulk_out_addr;
	__u16 bulk_in_maxpacket;
};

```