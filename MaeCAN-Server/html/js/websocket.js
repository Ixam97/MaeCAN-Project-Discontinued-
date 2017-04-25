var url = window.location.origin;
console.log('Current Domain: ' + url);

var ws = new WebSocket(url.replace('http', 'ws') + ':8080', 'echo-protocol');

ws.onerror = function(error){
	console.log(error);
	document.getElementById('loksteuerung').innerHTML = '<center><h1>WebSocket-Fehler!</p></center>';
};

ws.onclose = function(){
	console.log('WebSocket connection closed!')
	document.getElementById('loksteuerung').innerHTML = '<center><h1>WebSocket-Verbindung wurde getrennt!</p></center>';
};