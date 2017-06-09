i-7565 CAN-to-USB amusing facts and trivia
by GDR! <gdr@go2.pl>

* In order for i7565 to operate, you need to load the following kernel modules:

modprobe usbserial
modprobe pl2303

Both of them are available in standard 2.6 Linux kernel.

* The provided Linux drivers are useless and don't even compile on recent kernels.

* To connect to the device, use /dev/ttyUSB0 (unless you have other usb/serial converters, of course).

* The virtual port parameters are as following:
 921600 bauds
 8 bits per char
 no parity
 1 stop bit
 no flow control, or software (xon/xoff) flow control

 A.K.A. 921600 8n1                        


