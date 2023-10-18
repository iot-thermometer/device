flash:
	idf.py build && idf.py -p /dev/cu.usbserial-0001 flash

monitor:
	idf.py -p /dev/cu.usbserial-0001 monitor