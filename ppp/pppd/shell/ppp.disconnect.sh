#!/bin/sh

IFACE=$1

if [ -f "/var/misc/ppp/pppd-$IFACE.pid" ] ; then
	echo "pppd-disconnect: pppd is being stop!"
	PPPD_PID=`head -n1 /var/misc/ppp/pppd-$IFACE.pid` > /dev/null 2>&1
	echo "kill $PPPD_PID"
	kill -15 $PPPD_PID > /dev/null 2>&1
	MY_PID=$PPPD_PID
	while [ ! -z $MY_PID ]
	do
		MY_PID=`ps | grep $PPPD_PID | grep -v "grep" | cut -c 1-6`
		echo "$MY_PID";
		sleep 0.3
	done
	echo "$MY_PID";
fi

if [ -f "/var/misc/ppp/pppoe-$IFACE.pid" ] ; then
	echo "pppoe-disconnect: pppoe is being stop!"
	PPPOE_PID=`head -n1 /var/misc/ppp/pppoe-$IFACE.pid` > /dev/null 2>&1
	echo "kill $PPPOE_PID"
	kill -15 $PPPOE_PID > /dev/null 2>&1
	MY_PID=$PPPOE_PID
	while [ ! -z $MY_PID ]
	do
		MY_PID=`ps | grep $PPPOE_PID | grep -v "grep" | cut -c 1-6`
		echo "$MY_PID";
		sleep 0.3
	done
	echo "$MY_PID";
fi

rm /var/misc/ppp/pppd-$IFACE.pid
rm /var/misc/ppp/pppoe-$IFACE.pid
