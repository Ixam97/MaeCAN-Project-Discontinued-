const ip = 'localhost';
const dest_port = 15734;
const frame_stop = new Buffer([0x00, 0x00, 0x03, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]);
const frame_go   = new Buffer([0x00, 0x00, 0x03, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00]);

var power = false;

var count = 0;
var clients = {};

var config_data = [];


var http = require('http');
var server = http.createServer(function(request, response) {});
var dgram = require('dgram');
var udpServer = dgram.createSocket('udp4');
var udpClient = dgram.createSocket('udp4');

var fs = require('fs');
var ini = require('ini');

server.listen(8080, function() {
    console.log('WebSocet-Server is listening on port 8080');
});

var WebSocketServer = require('websocket').server;
wsServer = new WebSocketServer({
    httpServer: server
});


//WebSocket-Clients aufnehmen:
wsServer.on('request', function(request){
	var connection = request.accept('echo-protocol', request.origin);
	var id = count++;
	clients[id] = connection;
	console.log(connection.remoteAddress + ' connected.');

	connection.send(power ? 'go' : 'stop');

	connection.on('close', function(reasonCode, description){
		delete clients[id];
		console.log(connection.remoteAddress + ' disconnected.');
	});


	//Befehle vom WebSocet-Client verarbeiten:
	connection.on('message', function(dgram){
		var msg = dgram.utf8Data.split(':');
		var cmd = msg[0];
		var uid = parseInt(msg[1]);
		var value = parseInt(msg[2]);

		var uid_high = (uid & 0xFF00) >> 8;
		var uid_low = (uid & 0x00FF);
		var value_high = (value & 0xFF00) >> 8;
		var value_low = (value & 0x00FF);

		if (cmd == 'stop') {			// STOP-Taste
			udpClient.send(frame_stop, dest_port, ip);
		} else if (cmd == 'go') {		// GO-Taste
			udpClient.send(frame_go, dest_port, ip);
		} else if (cmd == 'setSpeed') {
			udpClient.send(new Buffer([0, 8, 3, 0, 6, 0, 0, uid_high, uid_low, value_high, value_low, 0, 0]), dest_port, ip);
		} else if (cmd == 'getSpeed') {
			udpClient.send(new Buffer([0, 8, 3, 0, 4, 0, 0, uid_high, uid_low, 0, 0, 0, 0]), dest_port, ip);
		} else if (cmd == 'lokNothalt') {
			udpClient.send(new Buffer([0, 0, 3, 0, 5, 0, 0, uid_high, uid_low, 3, 0, 0, 0]), dest_port, ip);
		} else if (cmd == 'setFn') {
			udpClient.send(new Buffer([0, 0x0c, 3, 0, 6, 0, 0, uid_high, uid_low, value_high, value_low, 0, 0]), dest_port, ip);
		} else if (cmd == 'getFn') {
			udpClient.send(new Buffer([0, 0x0c, 3, 0, 5, 0, 0, uid_high, uid_low, value, 0, 0, 0]), dest_port, ip);
		} else if (cmd == 'toggleDir') {
			udpClient.send(new Buffer([0, 0x0a, 3, 0, 5, 0, 0, uid_high, uid_low, 3, 0, 0, 0]), dest_port, ip);
			udpClient.send(new Buffer([0, 0x0a, 3, 0, 4, 0, 0, uid_high, uid_low, 0, 0, 0, 0]), dest_port, ip);
		} else if (cmd == 'getDir') {
			udpClient.send(new Buffer([0, 0x0a, 3, 0, 4, 0, 0, uid_high, uid_low, 0, 0, 0, 0]), dest_port, ip);
		}
	});
});


udpServer.on('error', (err) => {
  console.log(`server error:\n${err.stack}`);
  udpServer.close();
});


// Befehle vom CAN verarbeiten:
udpServer.on('message', (udp_msg, rinfo) => {
  
  var ws_msg = '';
  var cmd = parseInt(udp_msg[1]);
  var hash = (parseInt(udp_msg[2]) << 8) + parseInt(udp_msg[3]);
  var dlc = parseInt(udp_msg[4]);
  var data = [parseInt(udp_msg[5]), parseInt(udp_msg[6]), parseInt(udp_msg[7]), parseInt(udp_msg[8]), parseInt(udp_msg[9]), parseInt(udp_msg[10]), parseInt(udp_msg[11]), parseInt(udp_msg[12])]
  var uid = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data [3];
  var value = (parseInt(udp_msg[9]) << 8) + parseInt(udp_msg[10]);

  if (cmd == 0x01) {
  	var sub_cmd = parseInt(udp_msg[9]);
  	if (sub_cmd == 0x00) {
  		ws_msg = 'stop';
  		power = false;
  	} else if (sub_cmd == 0x01) {
  		ws_msg = 'go';
  		power = true;
  	} else if (sub_cmd == 0x03){
  		ws_msg = `updateSpeed:${uid}:${0}`;
  	}
  } else if (cmd == 0x09) {
  	ws_msg = `updateSpeed:${uid}:${value}`;
  } else if (cmd == 0x0d) {
  	ws_msg = `updateFn:${uid}:${value}`;
  } else if (cmd == 0x0b) {
  	ws_msg = `updateDir:${uid}:${parseInt(udp_msg[9])}`;
  } else if (cmd == 0x3B) {
  	if (dlc == 8) {
  		console.log(data);
  	}
  } else if (cmd == 0x31) {
  	var str_uid = toUnsignedString(uid);
  	var str_ver = `${data[4]}.${data[5]}`
  	var str_typ = ((data[6] << 8) + data[7]).toString(16);
  	console.log(`Ping response from 0x${str_uid}, Version ${str_ver}, Type 0x${str_typ}`);

  	var geraete = ini.parse(fs.readFileSync('./geraete.ini', 'utf-8'));

  	if (!geraete[str_uid]) {
  		console.log('Creating geraete.ini entry for ' + str_uid);
  		geraete[str_uid] = {};
  		geraete[str_uid].type = str_typ;
  		geraete[str_uid].version = str_ver;
  		fs.writeFileSync('./geraete.ini', ini.stringify(geraete));
  	}
  }

  if (ws_msg != '') {
	  for (var i in clients){
	  	clients[i].sendUTF(ws_msg);
	  }
  }
});

udpServer.on('listening', () => {
  var address = udpServer.address();
  console.log(`UDP-Server listening on Port ${address.port}.`);
  udpClient.send(frame_stop, dest_port, ip);
});

udpServer.bind(15733);
udpClient.send(new Buffer([0,0x30,3,0,0,0,0,0,0,0,0,0,0]), dest_port, ip);
console.log('ping sent.');

var pinger = setInterval(function(){
	udpClient.send(new Buffer([0,0x30,3,0,0,0,0,0,0,0,0,0,0]), dest_port, ip);
	console.log('ping sent.');
}, 10000);

function toUnsignedString(number){
	if (number < 0) {
		number = 0xffffffff + number  + 1;
	}

	return number.toString(16);
}

