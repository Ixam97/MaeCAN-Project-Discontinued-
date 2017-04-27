#!/bin/sh

ip link set can0 up type can bitrate 250000 restart-ms 10
can2udp -d 15733 -l 15734 -b 127.0.0.1
cd /var/www/node
nodejs maecanserver.js
