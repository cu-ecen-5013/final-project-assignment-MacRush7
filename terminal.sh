#!/bin/bash
# terminal face for the project
# resource askubuntu.com/questions/1705/how-can-i-create-a-select-menu-in-a-shell-script

set -e

# dialog options
HEIGHT=15
WIDTH=55
CHOICE_HEIGHT=6
OPTIONS=(1 "System Init" 2 "Fingerprint Sensor" 3 "RFID Sensor" 4 "Buzzer Test" 5 "MySQL")

# make dialog box
CHOICE=$(dialog --title "Welcome! How Would you Like to Verify your Access?" --menu "Please Select a Program:" $HEIGHT $WIDTH $CHOICE_HEIGHT "${OPTIONS[@]}" 2>&1 >/dev/tty)

case $CHOICE in
	1)
		echo "Starting System Init"
		./init.sh
		;;
	2)
		echo "Starting Fingerprint Module"
		./fingerprintModule		
		;;
	3)
		echo "Starting RFID Module"
		./rfidtest.sh
		;;
	4)
		echo "Starting Buzzer Test"
		./buzzerTest
		;;
	5)
		echo "Starting MySQL"
		python3 mysql.py
		;;

esac
