<!DOCTYPE html>
<html id="root" style="font-size: 10px;">
	<head>
		<title>MäCAN-Webserver</title>
		<link rel="stylesheet" type="text/css" href="styles/rangeslider.custom.css">
		<link rel="stylesheet" type="text/css" href="styles/main.css">
		<link rel="stylesheet" type="text/css" href="styles/mobile.360x640.css">
		<link id="size-stylesheet" rel="stylesheet" type="text/css" href="">
		<meta charset="utf-8">
		<meta name="mobile-web-app-capable" content="yes">
		<meta name="viewport" content="width=device-width, initial-scale=1">

		<script type="text/javascript" src="js/jquery.min.js"></script>
		<script type="text/javascript" src="js/rangeslider.min.js"></script>
		<script type="text/javascript">

			function getLokliste(){
			  $.get("php/lokliste.php",{}, function(data){
			    //console.log(data);
			    str_lokliste = data;
			    lokliste = str_lokliste.split("§");
			    for (var i = lokliste.length - 1; i >= 0; i--) {
			      names[i] = lokliste[i].split("$")[0];
			      uids[i] = lokliste[i].split("$")[1];
			    }
			  });
			}

		</script>
		<noscript>Javascript ist erforderlich!</noscript>

	</head>
	<body id="body">
	<div id="dummy">
		<div class="container" id="container" style="margin: 0;">
			<div class="containerheader" id="header"><div>MäCAN-Server</div><a href><div id="options">Optionen</div></a></div>
			<div class="containercontent" id="content">
				<div class="subcontainer">

					<div class="locoselect" id="directadress">
						<div class="locoselect_left">
							<div class="radiodiv">
								<input id="radio01" type="radio" name="protocol" checked/>
			  					<label for="radio01"><span></span>Motorola</label>
			  				</div>
			  				<div class="radiodiv">
								<input id="radio02" type="radio" name="protocol"/>
			  					<label for="radio02"><span></span>DCC</label>
			  				</div>
			  			</div><!--
			  		 --><div class="locoselect_left">
			  				<div class="textdiv">
				  				<p>Adresse:</p>
								<input id="adress_input" class="textfield" type="text" name="adress" value="1"><br>
							</div>
						</div><!--
					 --><div class="locoselect_left">
							<a href=""><div id="confirm_adress" >OK</div></a>
						</div>
					</div>

					<div>
						<a href="" id="locoselector" style="display: none;"><div>Lokauswahl</div></a>
					
						<div id="locoselector_list">
								
						</div>
					</div>

					<div class="controlscontainer">
						
						<div class="slider">
							<input type="range" id="rangeslider01" data-orientation="vertical" min="0" max="1000" value="0" step="1">
							<a id="changedirection" href=""><div class="button" id="change_direction">
								<div id="left" style="width: 100%; display: none;">
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-right: 2rem solid #2FA938; float: left;"></div>
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-left: 2rem solid #adadad; float: right;"></div>
								</div>
								<div id="right" style="width: 100%; display: inline-block;">
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-right: 2rem solid #adadad; float: left;"></div>
									<div style="width: 0; height: 0; border-bottom: 2rem solid transparent; border-top: 2rem solid transparent; border-left: 2rem solid #2fa938; float: right;"></div>
								</div>
							</div></a>
						</div><!--
					 --><div class="functions" style="margin: 0 .5rem;">
							<a href=""><div class="function">F0</div></a>
							<a href=""><div class="function">F1</div></a>
							<a href=""><div class="function">F2</div></a>
							<a href=""><div class="function">F3</div></a>
							<a href=""><div class="function">F4</div></a>
							<a href=""><div class="function">F5</div></a>
							<a href=""><div class="function">F6</div></a>
							<a href=""><div class="function">F7</div></a>
						</div><!--
					 --><div class="functions">
							<a href=""><div class="function">F8</div></a>
							<a href=""><div class="function">F9</div></a>
							<a href=""><div class="function">F10</div></a>
							<a href=""><div class="function">F11</div></a>
							<a href=""><div class="function">F12</div></a>
							<a href=""><div class="function">F13</div></a>
							<a href=""><div class="function">F14</div></a>
							<a href=""><div class="function">F15</div></a>
						</div>
					</div>
					<div class="stopgocontainer">
						<center>
							<a href=""><div class="stopgobutton" id="stop_button_1" style="color: red;">STOP</div></a>
							<a href=""><div class="stopgobutton" id="go_button_1" style="color: green;">GO</div></a>
						</center>
					</div>
				</div>
			</div>
		</div>
   
   
		<script type="text/javascript" src="js/main.js"></script>
	</div>
	</body>
</html>
