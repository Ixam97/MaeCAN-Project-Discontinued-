<?php 
	$locid = $_GET['locid'];

	$value = file_get_contents('../python/'.$locid.'.txt');

	if ($value == null) {
		$value = 'error';	
	}

	echo $value;
 ?>