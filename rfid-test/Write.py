#!/usr/bin/env python

# Source - https://pimylifeup.com/raspberry-pi-rfid-rc522/

import RPi.GPIO as GPIO
from mfrc522 import SimpleMFRC522

reader = SimpleMFRC522()
echo "RFID Sensor Connected\n"

try:
        text = input('Enter user name:')
        print("Please place your tag for registration")
        reader.write(text)
        print("User successfully registered")
finally:
        GPIO.cleanup()
		echo "Stopped interaction with RFID Sensor\n"