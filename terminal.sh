# !/bin/bash
# terminal face for the project
# resource askubuntu.com/questions/1705/how-can-i-create-a-select-menu-in-a-shell-script

set -e

# dialog options
HEIGHT=15
WIDTH=55
CHOICE_HEIGHT=4
OPTIONS=(1 "Fingerprint Sensor" 2 "RFID Sensor" 3 "GSM Module")

# make dialog box
CHOICE=$(dialog --title "Welcome! How Would you Like to Verify your Access?" --menu "Please Select a Program:" $HEIGHT $WIDTH $CHOICE_HEIGHT "${OPTIONS[@]}" 2>&1 >/dev/tty)

case $CHOICE in
	1)
		echo "Starting Fingerprint Module"
		cd
		cd ..
		cd bin
		./fingerprintModule
		;;
	2)
		echo "Starting RFID Module"
		;;
	3)
		echo "Starting GSM Module"
		;;

esac
