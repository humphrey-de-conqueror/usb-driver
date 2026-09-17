### printk 
```
printk(LOG_LEVEL string);
```
log_level: 
+ KERN_EMERG	system is unsuable 
+ KERN_ERR	something went wrong 
+ KERN_WARNING	something looks suspecious 
+ KERN_INFO	general information 
+ KERN_DEBUG	verbose debug output 

### kmalloc / kfree 
```
void *buf	= kmalloc(BYTE_SIZE, FLAG);

kfree(buf)
```
FLAG
+ GFP_KERNEL	normal context, can sleep, use by default 
+ GFP_ATOMIC	interrupt context, cannot sleep, use in callbacks that fire from interrupt

### struct usb_device / struct usb_interface / interface_to_usbdev / struct usb_host_interface 
```
struct usb_interface *intf;

struct usb_device *dev = interface_to_usbdev(intf);

struct usb_host_interface *interface_detail = intf->cur_altsetting 

struct usb_endpoint_descriptor *endpoint; 
endpoint = &interface_detail->endpoint[i].desc 
```

### URB: usb request blocks 
1. fill in the urb form (where to send, how much data, which endpoint)
2. submit the form to usb core 
3. usb core handles that actual delivery 
4. when done, recieve acknowledgement message 

lifecycle: 
1. allocate	= usb_alloc_urb() 
2. fill		= usb_fill_bulk_urb() or similar 
3. submit	= usb_submit_urb() 
4. free		= usb_free_urb() 

### synchronous usb transfers 
```
usb_bulk_msg(dev, pipe, data, len, &actual_len, timeout);
```
dev	= struct usb_device
piep	= usb_sndbulkpipe/usb_rcvbulkpipe (dev, endpoint_address);
&actual_len = how many byte send or receive, fill in my api

### expose this usb information to userspace (if want): at /sys/ 
```
register: 
static DEVICE_ATTR_RO(my_attribute);	read-only
static DEVICE_ATTR_RW(my_attribute);	read-write

this will run when someone read the usb attribute at /sys/:
static ssize_t my_attribute_show(struct device *dev, struct device_aatribute *attr, char *buf) {}
```

