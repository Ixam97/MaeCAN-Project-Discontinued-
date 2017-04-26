#! /bin/bash

sudo su
echo -e '\033[0;34m Updating ... \033[0m'
apt-get update
echo -e '\033[0;34m Setting up Node lts Boron ...\033[0m'
curl -sL https://deb.nodesource.com/setup_6.x | sudo -E bash -
apt-get install -y nodejs
apt-get install apache2 php libapache2-mod-php7.0
echo -e '\033[0;34m Setting up node server ...\033[0m'
cd /tmp/
mkdir server/
cd server/
wget https://github.com/Ixam97/MaeCAN-Project/raw/master/MaeCAN-Server/MaeCAN-Server.zip
unzip MaeCAN-Server.zip
rm /var/www/html/index.html
cp -R html/ /var/www/ 
cp -R node/ /var/www/
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
echo -e '\033[0;32m Done!\033[0m'

exit 0
