<?php

function xmlrules_map($id, $player_list)
{
    $text  = "<!-- specific inventory for this mission -->".CR;
    $text .= "<client_set>".CR;

	$i = "0";
    $player_array = explode(",", $player_list);
    foreach ( $player_array as $player_name )
	{
		$text .= 
			'<client'.$i.'>'.CR.
			'<name>'.$player_name.'</name>'.CR.
			'<health>100</health>'.CR.
			'<inventory>1:100,7:1</inventory>'.CR.
			'</client'.$i.'>'.CR;
		$i++;
	}

	$text .= "</client_set>".CR;

    return $text ;
}

?>

