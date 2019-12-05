#!/usr/bin/env python

# Source - https://pimylifeup.com/raspberry-pi-rfid-rc522/
# For RPI test connections, please see the source document referenced above. Thanks.

import RPi.GPIO as GPIO
from mfrc522 import SimpleMFRC522

reader = SimpleMFRC522()
print("RFID Sensor Connected\n")

try:
	text = input('Enter user name:')
	print("Please place your tag for registration\n")
	reader.write(text)
	print("\n")
	print("User successfully registered\n")

finally:
	GPIO.cleanup()
	print("Stopped interaction with RFID Sensor\n")
