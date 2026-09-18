#include <linux/module.h> 
#include <linux/usb.h> 
#include <linux/fs.h> 
#include <linux/uaccess.h> /* copy_to_user, move thing from kernel to userspace */
#include <linux/cdev.h>

#define VID	0x0781
#define PID	0x5591

struct my_usb_dev {
	struct usb_device *udev; 
	__u8 bulk_in_addr; 
	__u8 bulk_out_addr; 
	__u16 bulk_in_maxpacket; 
	struct cdev cdev; 
	dev_t devno;
	__u8 *bulk_in_buf; 
};

static struct class *my_usb_class; 
static dev_t my_usb_major; 

static const struct usb_device_id my_usb_table[] = {
	{ USB_DEVICE(VID, PID)}, 
	{ }
};
MODULE_DEVICE_TABLE(usb, my_usb_table);

static int my_usb_open(struct inode *inode, struct file *file)
{
	struct my_usb_dev *mydev; 

	mydev = container_of(inode->i_cdev, struct my_usb_dev, cdev);
	file->private_data = mydev; 
	return 0; 
}

static ssize_t my_usb_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct my_usb_dev *mydev = file->private_data; 
	int actual_len; 
	int ret; 

	ret = usb_bulk_msg(
		mydev->udev, 
		usb_rcvbulkpipe(mydev->udev, mydev->bulk_in_addr), 
		mydev->bulk_in_buf, 
		min(count, (size_t)mydev->bulk_in_maxpacket), 
		&actual_len, 
		5000);
	
	if (ret) {
		printk(KERN_ERR "my_usb: bulk read failed: %d \n", ret);
		return ret; 
	}

	if (copy_to_user(buf, mydev->bulk_in_buf, actual_len)) {
		printk(KERN_ERR "my_usb: copy to user failed \n");
		return -EFAULT; 
	}

	return actual_len; 
}

static struct file_operations my_usb_fops = {
	.owner = THIS_MODULE, 
	.open = my_usb_open,
	.read = my_usb_read,
};

static int my_usb_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *iface_desc = intf->cur_altsetting; 
	struct usb_endpoint_descriptor *endpoint; 
	struct my_usb_dev *mydev; 
	int ret; 
	int i;

	printk(KERN_INFO "my_usb: device plugged in\n");
	printk(KERN_INFO "my_usb: VID: %04x | PID: %04x \n", dev->descriptor.idVendor, dev->descriptor.idProduct);
	printk(KERN_INFO "my_usb: number of endpoints: %d \n", iface_desc->desc.bNumEndpoints);

	mydev = kmalloc(sizeof(*mydev), GFP_KERNEL);
	if (!mydev) {
		printk(KERN_ERR "my_usb: out of memory\n");
		return -ENOMEM;
	}

	mydev->udev = dev;
	mydev->bulk_in_addr = 0; 
	mydev->bulk_out_addr = 0; 
	mydev->bulk_in_maxpacket = 0;


	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
		endpoint = &iface_desc->endpoint[i].desc;

		printk(KERN_INFO "my_usb: endpoint[%d] address: 0x%02x \n", i, endpoint->bEndpointAddress);
		printk(KERN_INFO "my_usb: endpoint[%d] max packet size: %d \n", i, endpoint->wMaxPacketSize);

		if (usb_endpoint_dir_in(endpoint)) {
			printk(KERN_INFO "my_usb: endpoint[%d] direction: IN\n", i);
		} else {
			printk(KERN_INFO "my_usb: endpoint[%d] direction: OUT\n", i);
		} 
			
		if (usb_endpoint_xfer_bulk(endpoint))
			printk(KERN_INFO "my_usb: endpoint[%d] type: BULK \n", i);

		if (usb_endpoint_dir_in(endpoint) && usb_endpoint_xfer_bulk(endpoint)) {
			mydev->bulk_in_addr = endpoint->bEndpointAddress; 
			mydev->bulk_in_maxpacket = endpoint->wMaxPacketSize;
		}

		if (usb_endpoint_dir_out(endpoint) && usb_endpoint_xfer_bulk(endpoint)) {
			mydev->bulk_out_addr = endpoint->bEndpointAddress; 
		}
	}

	/* mydev attach itself to the interface */
	usb_set_intfdata(intf, mydev);

	/* allocate bulk in buf to character device */
	mydev->bulk_in_buf = kmalloc(mydev->bulk_in_maxpacket, GFP_KERNEL);
	if (!mydev->bulk_in_buf) {
		printk(KERN_ERR "my_usb: failed to allocate bulk in buffer \n");
		kfree(mydev);
		return -ENOMEM; 
	} 

	mydev->devno = MKDEV(MAJOR(my_usb_major), 0);
	cdev_init(&mydev->cdev, &my_usb_fops);
	mydev->cdev.owner = THIS_MODULE; 

	ret = cdev_add(&mydev->cdev, mydev->devno, 1);
	if (ret) {
		printk(KERN_ERR "my_usb: failed to add cdev \n");
		kfree(mydev->bulk_in_buf);
		kfree(mydev);
		
		return ret; 
	}

	device_create(my_usb_class, NULL, mydev->devno, NULL, "my_usb0");
	printk(KERN_INFO "my_usb: device created at /dev/my_usb0 \n");
	return 0;
}

static void my_usb_disconnect(struct usb_interface *intf)
{
	struct my_usb_dev *mydev = usb_get_intfdata(intf);

	usb_set_intfdata(intf, NULL);
	device_destroy(my_usb_class, mydev->devno);
	cdev_del(&mydev->cdev);
	kfree(mydev->bulk_in_buf);
	kfree(mydev);

	printk(KERN_INFO "my_usb: device unplugged\n");

	return; 
}


static struct usb_driver my_usb_driver = {
	.name		= "my_usb", 
	.id_table	= my_usb_table, 
	.probe		= my_usb_probe, 
	.disconnect	= my_usb_disconnect,
};

static int __init my_usb_init(void)
{
	int ret; 

	ret = alloc_chrdev_region(&my_usb_major, 0, 1, "my_usb");
	if (ret < 0) {
		printk(KERN_ERR "my_usb: failed to allocate device number\n");
		return ret; 
	}

	my_usb_class = class_create("my_usb");
	if (IS_ERR(my_usb_class)) {
		unregister_chrdev_region(my_usb_major, 1);
		return PTR_ERR(my_usb_class);
	}

	return usb_register(&my_usb_driver);
}

static void __exit my_usb_exit(void) 
{
	usb_deregister(&my_usb_driver);
	class_destroy(my_usb_class);
	unregister_chrdev_region(my_usb_major, 1);
}

module_init(my_usb_init);
module_exit(my_usb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("humphrey");
MODULE_DESCRIPTION("A simple USB driver");