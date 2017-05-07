mv /var/www/html/config/ /tmp/config/
cd /var/www/
rm -R ./node/
rm -R ./html/
rm MaeCAN-Server.zip
wget https://github.com/Ixam97/MaeCAN-Project/raw/master/MaeCAN-Server/MaeCAN-Server.zip
unzip MaeCAN-Server.zip
rm updatemaecanserver.sh
wget https://raw.githubusercontent.com/Ixam97/MaeCAN-Project/master/MaeCAN-Server/updatemaecanserver.sh
mv /tmp/config/ /var/www/html/config/
cd ./node/
npm install ini websocket
cd /etc/init.d/
wget https://raw.githubusercontent.com/Ixam97/MaeCAN-Project/master/MaeCAN-Server/maecanserver.sh
chmod +x maecanserver.sh
update-rc.d maecanserver.sh defaults
