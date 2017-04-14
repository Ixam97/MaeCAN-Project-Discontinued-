<?php 
	$value = $_GET['value'];
	exec('../python/stopgo '.$value);
 ?>