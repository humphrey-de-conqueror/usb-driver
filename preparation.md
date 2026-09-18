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

---

### communication protocol of storage device 
+ storage device 
	+ SATA SSD	-> SATA/AHCI
	+ NVMe SSD	-> NVMe
	+ USB flash	-> USB MSC

### understand usb msc protocol 
| _step_ | Host | Device | 
| :--: | :--: | :--: | 
| p1 | cbw command | -> |
| p2 | <- | data | 
| p3 | csw pass/failed status | -> |

#### p1: command block wrapper 
| Bytes | Field | Description |
|---|---|---|
| 0–3 | `Signature` | Always `0x43425355` (`"USBC"`) |
| 4–7 | `Tag` | Any number, echoed back in CSW so you can match them |
| 8–11 | `DataLength` | How many bytes you expect back |
| 12 | `Flags` | `0x80` = device → host (read), `0x00` = host → device |
| 13 | `LUN` | Logical Unit Number, usually `0` |
| 14 | `CBLength` | Length of the SCSI command, `6` for `INQUIRY` |
| 15–30 | `CB` | The actual SCSI command bytes |

#### p2: data
the device sends back the data you requested if p1 pass 

#### p3: command status wrapper 
the device sends back 13 bytes status if command pass/failed 

