<?php 
	$page = $_GET['page'];
	$side = $_GET['side']; 
?>

<!DOCTYPE html>
<html id="root" style="font-size: 10px;">
	<head>
		<title>Integrator</title>
		<link rel="stylesheet" type="text/css" href="styles/rangeslider.custom.css">
		<link rel="stylesheet" type="text/css" href="styles/main.css?14789">
		<link rel="stylesheet" type="text/css" href="styles/mobile.360x640.css?4">
		<link id="size-stylesheet" rel="stylesheet" type="text/css" href="">
		<meta charset="utf-8">
		<meta name="mobile-web-app-capable" content="yes">
		<meta name="viewport" content="width=device-width, initial-scale=1">

		<script type="text/javascript" src="js/jquery.min.js"></script>
		<script type="text/javascript" src="js/rangeslider.min.js"></script>
		<noscript>Javascript ist erforderlich!</noscript>

	</head>
	<body id="body" style="background-color: #e6e6e6;" onload="setRem(); return false;">

		<?php include 'php/'.$page.'.php'; ?>

		<script type="text/javascript" src="js/main.js?41452857"></script>
	</body>
</html>
