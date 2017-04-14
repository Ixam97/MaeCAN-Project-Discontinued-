var throttle_slider = $('input[type="range"]');
var throttle_slider_handle;

var throttle_sliding = false;
var speedupdate = false;
var direct_adress = true;
var list_visible = false;
var is_fullscreen = false;

var timer;
var throttle_watcher;
var throttle_manager;
var old_speed;

//Wichtige Elemente:
var content		      = document.getElementById('body');
var options           = document.getElementById('options');
var stop_button_1     = document.getElementById('stop_button_1');
var go_button_1       = document.getElementById('go_button_1');
var function_buttons  = document.getElementsByClassName('function');
var change_direction  = document.getElementById('change_direction');
var confirm_adress    = document.getElementById('confirm_adress');
var adress_input      = document.getElementById('adress_input');
var motorola          = document.getElementById('radio01');
var dcc               = document.getElementById('radio02');
var rangeslider01     = document.getElementById('rangeslider01');
var locoselector_list = document.getElementById('locoselector_list');
var locoselector      = document.getElementById('locoselector');

//regelm��ig genutzte Farben:
var cs_red = '#ff0025';
var cs_green = '#2fa938';

//Standard LocID:
var locid = 1;
var names = [];
var uids = [];
var lokliste = [];
var str_lokliste;

function adjustStyle(width) {
  if (window.screen.width == 360 && window.screen.height == 640){
    $('#size-stylesheet').attr('href', 'styles/mobile.360x640.css');
  }else{
    $('#size-stylesheet').attr('href','');
  }
}

$(function() {
  adjustStyle();
  $(window).resize(function() {
    adjustStyle();
  });
});

throttle_slider.rangeslider({
  polyfill: false
});

throttle_slider.on('input', function(){

  // �berpr�ft, ob der Input vom Nutzer oder durch ein regelm��iges Geschwindikeitsupdate ausgel�st wurde
  if (speedupdate){ 
    speedupdate = false;    
  }else{
    // �berpr�ft, ob erster Input und startet entsprechend Throttlemanager
    if (!throttle_sliding) {
      throttle_manager = setInterval(function(){setSpeed(rangeslider01.value);}, 150);
    }

    throttle_sliding = true;
    throttle_slider_handle.textContent = Math.ceil(this.value / 10);

    clearTimeout(timer);
    clearInterval(throttle_watcher);

    timer = setTimeout(function(){
      throttle_sliding = false;
      clearInterval(throttle_manager);
      throttle_watcher = setInterval(getSpeed, 1000);
      console.log('restarting');
    },500);

  }
});

throttle_slider_handle = document.getElementById('js-rangeslider-0').childNodes[1];
throttle_slider_handle.textContent = Math.ceil(throttle_slider[0].value / 10);


// allgemeine Funktionen:

// Status der Stop und Go Tasten abrufen.
function getButtonStatus(){
  $.get("php/buttonUpdater.php", {}, function(data){
    if (data == '0'){
      stop_button_1.style.backgroundColor = cs_red;
      go_button_1.style.backgroundColor = 'white';
      stop_button_1.style.color = 'white';
      go_button_1.style.color = cs_green;
    }else if (data == '1'){
      stop_button_1.style.backgroundColor = 'white';
      go_button_1.style.backgroundColor = cs_green;
      stop_button_1.style.color = cs_red;
      go_button_1.style.color = 'white';
    }
  });
}

// Geschwindigkeit der aktuellen LocID anfordern:
function getSpeed(){ 
  $.get("php/getSpeed.php", {locid: locid}, function(data){
    if (data == 'error') {
      console.log('speederror');
    }else if (data != rangeslider01.value) {
      speedupdate = true;
      old_speed = data;
      console.log('getting speed...');
      throttle_slider.val(data).change();
      throttle_slider_handle.textContent = Math.ceil(data / 10);
    }
  });
}

//Geschwindigkeit der aktuellen LocID setzen:
function setSpeed(value){
  if (value != old_speed) {
    $.get("php/setSpeed.php",{locid: locid, value: value});
    console.log('throtteling...');
    old_speed = value;
  }
}

//Elementgr��e in anh�ngigkeit der Bildschirmaufl�sung �ndern
function displayScreenSize(){ 
  var factor;
  var displayScreenWidth = window.screen.width;
  var displayScreenHeight = window.screen.height;
  if (displayScreenWidth < 400) {
    factor = 156.25;
  }else if (displayScreenWidth < 700) {
    factor = 125;
  }else{
    factor = 62.5;
  }
  document.getElementById('root').style.fontSize = factor + '%';
}

function createDropdown(){
  for ( var i = 0; i < lokliste.length; i++) (function(i){
    var obj = document.createElement('a');
    var obj_text = document.createTextNode(names[i]);
    var obj_div = document.createElement('div');
    obj.setAttribute('href','');
    obj_div.setAttribute('class', 'dropdown_point')
    obj_div.appendChild(obj_text);
    obj.appendChild(obj_div);
    locoselector_list.appendChild(obj);

    obj.onclick = function(){
      locid = parseInt(uids[i], 16);
      getSpeed();
      locoselector.childNodes[0].textContent = names[i];
      hideDropdown();
      return false;
    };
  })(i);
}

function hideDropdown(){
  var listlength = locoselector_list.childNodes.length;
  for (var i = 0; i < listlength ; i++){
    locoselector_list.removeChild(locoselector_list.childNodes[0]);
  }
  list_visible = false;
}

//Intervallfuntionen:
var button_watcher = setInterval(function(){
  getButtonStatus();
}, 1000);
console.log('button_watcher gestartet...');

throttle_watcher = setInterval(function() {
  getSpeed();
}, 1000);
console.log('throttle_watcher gestartet...');

/*
header.onclick = function(){
		content.webkitRequestFullScreen();
};*/

//onClick-Events f�r Funktionstasten:
for (var i = 0; i < function_buttons.length; i++) (function(i){
  function_buttons[i].onclick = function(){
    console.log('Schalte Funktion ' + i);

    return false;
  }
})(i);

options.onclick = function(){
  if (direct_adress) {
    document.getElementById('directadress').style.display = 'none';
    document.getElementById('locoselector').style.display = 'block';
    direct_adress = false;
    //options.textContent = 'Adresse';
  }else{
    document.getElementById('directadress').style.display = 'block';
    document.getElementById('locoselector').style.display = 'none';
    direct_adress = true;
    //options.textContent = 'Lokliste';
  }
  return false;
}

confirm_adress.onclick = function(){
  if (motorola.checked) {
    locid = parseInt(adress_input.value, 10);
  }else if (dcc.checked) {
    locid = 0xc000 + parseInt(adress_input.value, 10);
  }
  getSpeed();
  return false;
};

locoselector.onclick = function(){
  if (!list_visible) {
    getLokliste();
    var timer = setTimeout(function(){
      createDropdown();
    },100);
    list_visible = true
  }else{
    hideDropdown();
  }

  return false;

};

stop_button_1.onclick = function(){
  $.get('/php/StopGo.php',{value: 'stop'});
  return false;
};

go_button_1.onclick = function(){
  $.get('/php/StopGo.php',{value: 'go'});
  return false;
};

change_direction.onclick = function(){
  setSpeed(0);
  return false;
}

