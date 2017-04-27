#!/bin/sh

mkdir /var/log/apache2
ip link set can0 up type can bitrate 250000 restart-ms 10
can2udp -d 15733 -l 15734 -b 127.0.0.1
can2lan -c /var/www/html/config/ -m
cd /var/www/node
node maecanserver.js
