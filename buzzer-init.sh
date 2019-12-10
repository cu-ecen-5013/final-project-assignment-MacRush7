# !/bin/bash
# buzzer module startup

set -e

case "$1" in
	start)
		echo "Starting Buzzer Program"
		./buzzerModule
		;;
	stop)
		echo "Stopping Buzzer Program"
		trap "kill 0" EXIT
		;;
	*)
		echo "default start/stop"
	exit 1

esac

exit
