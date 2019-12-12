#!/bin/bash

# Source - https://blog.stigok.com/2017/10/12/setting-up-a-pn532-nfc-module-on-a-raspberry-pi-using-i2c.html

echo "Testing if the sensor is connected at this time..."
i2cdetect -y 1
echo "Showing data for sensor if connected..."
nfc-scan-device -v
echo "Please place your NFC sensor tag on the sensor for testing sensor response..."
nfc-poll
echo "Exiting NFC sensor test script"
