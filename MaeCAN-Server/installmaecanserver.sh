#! /bin/bash
sudo apt-get install nodejs apache2 php5
rm /var/www/html/index.html
mv -R ./* /var/www/ 
rm /var/www/installmaecanserver.sh
cd /var/www/node/
npm install websocket ini

