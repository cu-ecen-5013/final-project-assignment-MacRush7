# !/bin/bash
# buzzer module startup

set -e

case "$1" in
	start)
		echo "Starting Buzzer Program"
		start-stop-daemon -S -n simpleserver -a /usr/bin/buzzerModule -- -d
		;;
	stop)
		echo "Stopping Buzzer Program"
		start-stop-daemon -K -n buzzermodule
		;;
	*)
		echo "default start/stop"
	exit 1

esac

exit
