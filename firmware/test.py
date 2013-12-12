import usb
import iox32a4u as x

dev = usb.core.find(idVendor=0x59e3, idProduct=0xf000)
y = dev.read(0x81, 100000)
print "Characters\n" + "".join([chr(x) for x in y])
print "Binary\n" + "\n".join(['{:08b}'.format(x) for x in y])
print "Hex\n" + "".join([hex(x) for x in y])