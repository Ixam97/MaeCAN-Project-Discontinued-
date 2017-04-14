<?php
$lokliste = str_replace('[lokomotive]', '', file_get_contents("../data/lokliste.cs2"));


//echo nl2br($lokliste);

$lok_array = explode('lokomotive', $lokliste);
$lok_array_len = sizeof($lok_array);

for ($i=1; $i < $lok_array_len; $i++) {

	${'lok_'.$i} 				= explode('.funktionen', $lok_array[$i],2);
	${'lok_'.$i.'_funktionen'} 	= explode('.funktionen', ${'lok_'.$i}[1]);
	${'lok_'.$i.'_info'} 		= explode('.', ${'lok_'.$i}[0]);

	for ($j=1; $j < sizeof(${'lok_'.$i.'_info'}); $j++) { 
		${'lok_'.$i.'_infoarray'} = explode("=", ${'lok_'.$i.'_info'}[$j]);
		${'lok_'.$i.'_'.${'lok_'.$i.'_infoarray'}[0]} = rtrim(${'lok_'.$i.'_infoarray'}[1]);
	}
}

for ($i=1; $i < $lok_array_len; $i++) { 
	echo ${'lok_'.$i.'_name'}.'$'.${'lok_'.$i.'_uid'};
	if ($i < $lok_array_len - 1) {
		echo '§';
	}
}
	




?>