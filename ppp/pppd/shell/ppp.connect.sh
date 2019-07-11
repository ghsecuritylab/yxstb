#!/bin/sh

if [ ! -f "/var/misc" ] ; then
	echo "could not find /var/misc"
	mkdir /var/misc
fi

if [ ! -f "/var/misc/ppp" ] ; then
	echo "could not find /var/misc/ppp"
	mkdir /var/misc/ppp
fi

USER=$1
PASSWORD=$2
IFACE=$3

ifconfig $IFACE up

if [  -f "/var/misc/ppp/pppd-$IFACE.pid" ] ; then
	echo "pppd-status: pppd is running! I will stop it frist"
	ppp.disconnect.sh $IFACE
fi

if [  -f "/var/misc/ppp/pppoe-$IFACE.pid" ] ; then
	echo "pppoe-status: pppoe is running! I will stop it frist"
	ppp.disconnect.sh $IFACE
fi

pppd pty "pppoe -p /var/misc/ppp/pppoe-$IFACE.pid -I $IFACE -T 80 -U -m 1412" file /home/hybroad/share/ppp.conf phycardname $IFACE user "$USER" password "$PASSWORD" &

echo "$!" > /var/misc/ppp/pppd-$IFACE.pid
