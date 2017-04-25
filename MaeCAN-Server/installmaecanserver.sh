#! /bin/bash

echo 'Installing needed packages ...'
apt-get install apache2 php5 nodejs nodejs-legacy npm
echo 'Setting up node server ...'
rm /var/www/html/index.html
mv -R html/ /var/www/ 
mv -R node/ /var/www/
cd /var/www/node/
npm install websocket ini
echo 'Updating device tree for CAN ...'
cd /boot/dtb
mv sun7i-a20-bananapi.dtb sun7i-a20-bananapi.dtb_org
wget lnxpps.de/bpi/bin/sun7i-a20-bananapi.dtb
echo 'Creating can2lan and can2udp ...'
cd /tmp/
git clone --depth 1 https://github.com/GBert/railroad.git
cd railroad/can2udp/src/
make
cp can2udp /usr/bin/
cp can2lan /usr/bin/
echo 'Done'

