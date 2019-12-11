#!/bin/bashecho "Initializing system...\n"

modprobe i2c-dev
modprobe i2c-bcm2835
modprobe spi-bcm2835
modprobe spidev

echo "System is initialized...\n"
