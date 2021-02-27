<?php
// Copyright (C) 2001-2006 Pat www.aftermoon.net - This program is under the terms 
// of the GNU Lesser General Public License, See www.gnu.org for more details


//-----------------------------------------------------------------------------
// called by MapGetXmlRules for specific XmlRules in the map
// id           = id of the quake2 server
// player_list  = server name and all players names (separated by comma)
//-----------------------------------------------------------------------------
function xmlrules_map($id, $player_list)
{
    $text  = "<!-- specific inventory for this mission -->".CR;
    $text .= "<client_set>".CR;

	$i = 0;
	$player_array = explode(',',$player_list);
	foreach($player_array as $player_name)
	{
	    $text .= '<client'.$i.'>'.CR;
	    $text .= '<name>'.$player_name.'</name>'.CR;
	    $text .= "<health>100</health>".CR;
	    
        $text .= "<inventory>7:1,";
	    if ( ($i % 2) == 0 )
	        $text .= "8:1,18:30,12:5";
	    else
	        $text .= "10:1,19:60,12:5";
        $text .= "</inventory>".CR;
	    $text .= '</client'.$i.'>'.CR;
	    $i++;
    }

    $text .= "</client_set>".CR;

    return $text;
}

?>