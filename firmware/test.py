import usb
import iox32a4u as x

dev = usb.core.find(idVendor=0x59e3, idProduct=0xf000)
print "".join([chr(x) for x in dev.read(0x81, 100000)])
