<!DOCTYPE html>
<html style="font-size: 10px;">
	<head>
		<title>MäCAN-Webserver</title>
		<link rel="manifest" href="/manifest.json?1">
		<link rel="shortcut icon" type="image/x-icon" href="/favicon.png">
		<link rel="apple-touch-icon" sizes="180x180" href="/apple-touch-icon.png">
		<link rel="stylesheet" type="text/css" href="styles/rangeslider.custom.css">
		<link rel="stylesheet" type="text/css" href="styles/main.css?3">
		<link rel="stylesheet" type="text/css" href="styles/mobile.360x640.css?1">
		<link id="size-stylesheet" rel="stylesheet" type="text/css" href="">
		<meta charset="utf-8">
		<meta name="mobile-web-app-capable" content="yes">
		<meta name="viewport" content="width=device-width, initial-scale=1">

		<script type="text/javascript" src="js/jquery.min.js"></script>
		<script type="text/javascript" src="js/rangeslider.min.js"></script>
		<noscript>Javascript ist erforderlich!</noscript>

	</head>
	<body id="body">
		<div id="dummy">
			<div class="container" id="container" style="margin: 0;">
				<div class="containerheader" id="header"><div>MäCAN-Server</div><a href=""><div id="options" onclick="$('#iframe_1').attr('src',''); hideIframe('iframe_1', 'selector_1'); return false;">
					<div style="width: 3rem; height: .4rem; border-radius: .2rem; background-color: black; margin: .8rem .8rem .5rem .5rem;"></div>
					<div style="width: 3rem; height: .4rem; border-radius: .2rem; background-color: black; margin: .5rem;"></div>
					<div style="width: 3rem; height: .4rem; border-radius: .2rem; background-color: black; margin: .5rem .8rem .8rem .5rem;"></div>


				</div></a></div>
				<div class="containercontent" id="content">
<!--
					<p>Bitte über 'Optionen' eine Funktion auswählen!</p>
					<center><img src="media/logo.png" alt="MäCAN" style="max-width: 100%;"></center>
-->					<div class="subcontainer" id="selector_1">
						<a href=""><div class="button" onclick="toggleFullscreen(); return false;">Vollbildmodus</div></a>
						<a href=""><div class="button" onclick="$('#iframe_1').attr('src','integrator.php?page=loksteuerung'); showIframe('iframe_1', 'selector_1'); return false;">Loksteuerung (rechts)</div></a>
						<a href=""><div class="button" onclick="$('#iframe_1').attr('src','integrator.php?page=loksteuerung&side=left'); showIframe('iframe_1', 'selector_1'); return false;">Loksteuerung (links)</div></a>
						<a href=""><div class="button" onclick="$('#iframe_1').attr('src','integrator.php?page=memory'); showIframe('iframe_1', 'selector_1'); return false;">Memory</div></a>
					</div>
					<iframe src="integrator.php?page=selektor" name="iframe_1" id="iframe_1" scrolling="no" style="border: none; display: none; width: 100%; height: calc(100vh - 7rem); margin: auto;"></iframe>

				</div>
			</div>
		</div>
		<script type="text/javascript" src="js/main.js?9"></script>
	</body>
</html>
