#! /bin/bash

sudo su
echo -e '\033[0;34m Setting up Node server ...\033[0m'
cd /var/www/
wget https://raw.github.com/Ixam97/MaeCAN-Project/master/MaeCAN-Server/updatemaecanserver.sh
wget https://raw.github.com/Ixam97/MaeCAN-Project/master/MaeCAN-Server/MaeCAN-Server.zip
unzip MaeCAN-Server.zip
cd /var/www/node/
npm install websocket ini
echo -e '\033[0;34m Updating device tree for CAN ...\033[0m'
cd /boot/dtb
mv sun7i-a20-bananapi.dtb sun7i-a20-bananapi.dtb_org
wget lnxpps.de/bpi/bin/sun7i-a20-bananapi.dtb
echo -e '\033[0;34m Creating can2lan and can2udp ...\033[0m'
cd /tmp/
git clone --depth 1 https://github.com/GBert/railroad.git
cd railroad/can2udp/src/
make
cp can2udp /usr/bin/
cp can2lan /usr/bin/
echo -e '\033[0;32m Setting up startup script...\033[0m'
cd /etc/init.d/
wget https://raw.githubusercontent.com/Ixam97/MaeCAN-Project/master/MaeCAN-Server/maecanserver.sh
chmod +x maecanserver.sh
update-rc.d maecanserver.sh defaults
echo -e '\033[0;32m Done!\033[0m'

exit 0
