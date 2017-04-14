<?php 
	$value = $_GET['value'];
	$locid = $_GET['locid'];
	exec('../python/throttle '.$locid.' '.$value);
 ?>