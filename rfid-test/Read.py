#!/usr/bin/env python

# Source - https://pimylifeup.com/raspberry-pi-rfid-rc522/

import RPi.GPIO as GPIO
from mfrc522 import SimpleMFRC522

reader = SimpleMFRC522()
echo "RFID Sensor Connected\n"
echo "RFID Sensor waiting for user registration...\n"

try:
        id, text = reader.read()
		echo "Printing user unique ID\n"
        print(id)
		echo "Printing user name\n" 
        print(text)
		
finally:
        GPIO.cleanup()
		echo "Stopped interaction with RFID Sensor\n"