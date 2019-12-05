#!/usr/bin/env python

# Source - https://pimylifeup.com/raspberry-pi-rfid-rc522/
# For RPI test connections, please see the source document referenced above. Thanks.

import RPi.GPIO as GPIO
from mfrc522 import SimpleMFRC522

reader = SimpleMFRC522()
print("RFID Sensor Connected\n")
print("RFID Sensor waiting for user registration...\n")
print("Please place your RFID card on the sensor sense-pad...\n")

try:
	id, text = reader.read()
	print("Printing user unique ID\n")
	print(id)
	print("\n")
	print("Printing user name\n")
	print(text)
	print("\n")
		
finally:
	GPIO.cleanup()
	print("Stopped interaction with RFID Sensor\n")
