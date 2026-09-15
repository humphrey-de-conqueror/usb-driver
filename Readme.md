### Preparation 
1. lsusb to identity vid:pid
```
lsusb
```
Vendor Id: 0781
Product Id: 5591

2. identify the existing driver 
```
lsusb -t
``` 

### unbind standard driver: usb-storage if found 
1. find path of usb 
```
ls /sys/bus/usb/drivers/usb-storage
```

2. identifies the interfaces, for example 4-1:1.0 (bus-port:config.interface)

3. unbind existing driver 
```
echo -n "4-1:1.0" | sudo tee /sys/bus/usb/dirvers/usb-storage/unbind 
```