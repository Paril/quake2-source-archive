<?php
// Copyright (C) 2001-2002 Pat www.aftermoon.net - This program is under the terms 
// of the GNU Lesser General Public License, See www.gnu.org for more details

/*
 * Internal function that add or remove entities related to a teleporter
 */
function AddEntity_TeleportLevel( $num, $x, $y, $z, $map, $msg, $present, $available )
{
    $x1 = $x - 60;
    $y1 = $y - 60;
    $z1 = $z;
    $x2 = $x + 60;
    $y2 = $y + 60;
    $z2 = $z + 40;
    $z3 = $z - 500;
    $z4 = $z3 + 40;

    $text  = "<entity_change>".CR;

    // Disable the unused teleporter light (red or green)
    $keyvalue = $present ? 'tl'.$num : 'tlr'.$num;
	$text .= '<if keyname="targetname" keyvalue="'.$keyvalue.'"><set_key keyname="spawnflags" value="0"/></if>'.CR;
    
	// Disable the field force if the teleporter is available and present
	$value = ( $available && $present ) ? 3 : 7;
	$text .= '<if keyname="targetname" keyvalue="tff'.$num.'"><set_key keyname="spawnflags" value="'.$value.'"/></if>'.CR;
    if ( !$available && $present )
    {
		$msg .= CR.'locked for challenge mode players';
    }

    $text .= '</entity_change>'.CR;

    $text .= '<entity_add>'.CR.
             '<entity>{ "classname" "misc_teleporter" "origin" "'.$x.' '.$y.' '.$z.'" "target" "t_dest_'.$map.'" }</entity>'.CR.
             '<entity>{ "classname" "trigger_multiple" "sounds" "2" "message" "'.$msg.'" "wait" "5" "mins" "'.$x1.' '.$y1.' '.$z1.'" "maxs" "'.$x2.' '.$y2.' '.$z2.'" }</entity>'.CR.
             '<entity>{ "classname" "misc_teleporter_dest" "targetname" "t_dest_'.$map.'" "origin" "'.$x.' '.$y.' '.$z3.'" }</entity>'.CR.
             '<entity>{ "classname" "trigger_multiple" "target" "t_'.$map.'" "mins" "'.$x1.' '.$y1.' '.$z3.'" "maxs" "'.$x2.' '.$y2.' '.$z4.'" }</entity>'.CR.
             '<entity>{ "classname" "target_changelevel" "map" "'.$map.'" "targetname" "t_'.$map.'" "origin" "'.$x.' '.$y.' '.$z3.'" }</entity>'.CR.
             '</entity_add>'.CR;
    return $text;
}

//-----------------------------------------------------------------------------
// Internal function that add dynamic entities for Button that trigger a Goal.
//-----------------------------------------------------------------------------
function AddEntity_ButtonGoal( $num, $x, $y, $z, $available, $msg_info, $msg_help )
{
    $x1 = $x;
    $y1 = $y - 32;
    $z1 = $z;
    $x2 = $x + 40;
    $y2 = $y + 32;
    $z2 = $z + 64;

	$text  = CR.'<!-- special button goal '.$num.' -->'.CR;

    if ( $available )
    {
    	// Disable the masking wall
        $text  .= 
            '<entity_change>'.CR.
    	    '<if keyname="targetname" keyvalue="t_wall_btn'.$num.'"><set_key keyname="spawnflags" value="3"/></if>'.CR.
            '</entity_change>'.CR;

    	$text .=
    		'<entity_add>'.CR.
    			'<entity>{ "classname" "trigger_multiple" "sounds" "2" "message" "'.$msg_info.'" "wait" "5" "mins" "'.$x1.' '.$y1.' '.$z1.'" "maxs" "'.$x2.' '.$y2.' '.$z2.'" }</entity>'.CR.
    			'<entity>{ "classname" "target_goal" "targetname" "t_btn'.$num.'" "origin" "'.$x.' '.$y.' '.$z.'" }</entity>'.CR.
    			'<entity>{ "classname" "target_help" "targetname" "t_btn'.$num.'" "message" "'.$msg_help.'" "origin" "'.$x.' '.$y.' '.$z.'" }</entity>'.CR.
    		'</entity_add>'.CR;
    }
    
	return $text;
}

//-----------------------------------------------------------------------------
// called by MapGetXmlRules for specific XmlRules in the map "startmap"
// id           = id of the quake2 server
// player_list  = server name and all players names (separated by comma)
//-----------------------------------------------------------------------------
function xmlrules_map($id, $player_list)
{
    global $session_data, $map_list;

    $text = "";
    $player_array = explode(",", $player_list);

    // Initialize inventory (dataset 0)
    $text .= "<!-- specific inventory for the startmap -->".CR;
    $text .= "<client_set>".CR;
	$i = 0;
	foreach($player_array as $player_name)
	{
	    $text .= '<client'.$i.'>'.CR;
	    $text .= '<name>'.$player_name.'</name>'.CR;
	    $text .= '<health>100</health>'.CR;
        $text .= '<inventory>7:1</inventory>'.CR;
	    $text .= '</client'.$i.'>'.CR;
	    $i++;
    }
    $text .= "</client_set>".CR;

	// create "virtual session" for each dataset to access the right maplist
	$session_dataset1 = new csession_data($session_data->id);
	$session_dataset1->dataset = 1;
	$session_dataset2 = new csession_data($session_data->id);
	$session_dataset2->dataset = 2;
	
	// First, load all players, for complete team
    foreach ( $player_array as $player_name )
    {
		$session_dataset1->load_player_info($player_name); 
		$session_dataset2->load_player_info($player_name); 
	}

	// Second: load all player data
    foreach ( $player_array as $player_name )
    {
		$session_dataset1->load_player_data($player_name);
		$session_dataset2->load_player_data($player_name);
    }
	
    // management of dataset1 : the classic idsoftware missions
    foreach ( $player_array as $player_name )
    {
    	$player_data =& $session_dataset1->get_player_data($player_name);

        $player_maplist = $player_data->maplist;
        if ( !strstr($player_maplist, "base1") )
            $player_maplist .= " base1";
        $player_maplist = explode(" ", $player_maplist);
        
        if ( !isset($maplist) )
        {
            // First player (and server)
            $maplist = $player_maplist;
            $maplist_available = $player_maplist;

            $mapspot = $player_data->map;
            $mapspotname = strtok($mapspot, "$");
        }
        else
        {
            // other players
            $maplist_available = array_intersect($maplist_available, $player_maplist);
            $maplist = array_merge($maplist, $player_maplist);
        }
    }

    // hack for coop spawn target different of empty string
    $unitstart_target["bunk1"]   = "start";     // mission 2
    $unitstart_target["mintro"]  = "start";     // mission 4
    $unitstart_target["fact1"]   = "start";     // mission 5
    $unitstart_target["power1"]  = "pstart";    // mission 6
    $unitstart_target["hangar1"] = "unitstart"; // mission 8
    $unitstart_target["city1"]   = "unitstart"; // mission 9
    $unitstart_target["boss1"]   = "bosstart";  // mission 10
    
    // All the teleporters for the dataset1 : the classic idsoftware missions
    $origin_y = -1504;
    $num=1;
    foreach ( $map_list as $mapname => $map )
    {
        if ( $map->dataset==1 && $map->mission != "" )
        {
            $mapname_target = $mapname;
            if ( array_key_exists($mapname, $unitstart_target) )
                $mapname_target .= "\$".$unitstart_target[$mapname];

            $text .= AddEntity_TeleportLevel($num, -160, $origin_y, 144, $mapname_target, 
                                             $map->name."\n".$map->mission,
                                             in_array($mapname, $maplist),
                                             $session_dataset1->gametype==0 ? 1 : in_array($mapname, $maplist_available));
            $origin_y += 128;
            $num++;
        }
    }

    // Teleporter for the last played map on the dataset1 : the classic idsoftware missions
    if ( isset($map_list[$mapspotname]) )
    {
    	$map = $map_list[$mapspotname];
    	$text .= AddEntity_TeleportLevel("0", 210, -1504, 144, $mapspot, 
                                         $map->name."\ncurrent game",
                                         in_array($mapspotname, $maplist),
                                         $session_dataset1->gametype==0 ? 1 : in_array($mapspotname, $maplist_available));
    }
    else
        $text .= '<entity_change>'.CR.
                 '<if keyname="targetname" keyvalue="tff0"><set_key keyname="spawnflags" value="3"/></if>'.CR.
                 '</entity_change>'.CR;

	// Teleporter for the punishment
	$mapspot = "q2dm3\$punish";
    $map = $map_list[strtok($mapspot,"\$")];
    $text .= AddEntity_TeleportLevel("0", 210, -1300, 126, $mapspot, $map->name."\n".$map->mission, 1, 1);

	// Teleporter for "Rush for the Head"
	$mapspot = "q2dm7";
    $map = $map_list[strtok($mapspot,"\$")];
    $text .= AddEntity_TeleportLevel("0", 210, -1200, 126, $mapspot, $map->name."\n".$map->mission, 1, 1);

	// Teleporter for "Crash n Massacre"
	$mapspot = "q2dm4";
    $map = $map_list[strtok($mapspot,"\$")];
    $text .= AddEntity_TeleportLevel("0", 210, -1100, 126, $mapspot, $map->name."\n".$map->mission, 1, 1);

	// Teleporter for Q2DM1, randomly "The Makron twins" by ANDREI / "" by xRDVx
	$mapspot = "q2dm1";
    $map = $map_list[strtok($mapspot,"\$")];
    $text .= AddEntity_TeleportLevel("0", 210, -1000, 126, $mapspot, $map->name."\n".$map->mission, 1, 1);

	// Teleporter for "The cave people" by ANDREI
	$mapspot = "q2dm6";
    $map = $map_list[strtok($mapspot,"\$")];
    $text .= AddEntity_TeleportLevel("0", 210, -900, 126, $mapspot, $map->name."\n".$map->mission, 1, 1);

	// Teleporter for the stench serie 1st map
	$mapspot = "zomboid";
    $map = $map_list[strtok($mapspot,"\$")];
    $text .= AddEntity_TeleportLevel("0", 210, -608, 126, $mapspot, $map->name."\n".$map->mission, 1, 1);

	// Teleporter for the stench serie last played map
	$player_data =& $session_dataset2->get_player_data($player_name);
	if ( !empty($player_data->map) )
	{
		$mapspot = $player_data->map;
	    $map = $map_list[strtok($mapspot,"\$")];
	    $text .= AddEntity_TeleportLevel("0", 210, -480, 126, $mapspot, $map->name."\ncurrent game for the\nStench serie", 1, 1);
	}

	// Teleporter for the "Premonition of Angron" 1st map
	$mapspot = "angron2a";
    $map = $map_list[strtok($mapspot,"\$")];
    $text .= AddEntity_TeleportLevel("0", 210, -224, 126, $mapspot, $map->name."\n".$map->mission, 1, 1);
	
	// NOTE: direct jump for debuging
	if ( $session_dataset1->betatest )
	{
        $test_mapspots = array( 'boss2', 'ware1', 'ware2' );

        $i = 0;
        foreach( $test_mapspots as $test_mapspot )
        {
            $test_map = $map_list[$test_mapspot];
            $text .= AddEntity_TeleportLevel("0", 20, -1504+(128*$i), 126, $test_mapspot, 
                                             $test_map->name."\nbetatest direct access", 1, 1);
            $i++;
        }
	}

	//-------------------------------------------------------------------------
	// Manage special goal buttons
	//-------------------------------------------------------------------------
	
    // Buttons: Y positions
    $y_1_2 = -2400;
    $y_3_4 = -2272;
    $y_5_6 = -2144;
    $y_7_8 = -2016;
	
	$text .= AddEntity_ButtonGoal( 1, -272, $y_1_2, 24, true,
		"Push to select ANDREI additional\n".
		"monsters.                       ",
		"ANDREI additional monsters\n".
		"selected.                 " );

//	$text .= AddEntity_ButtonGoal( 3, -272, $y_3_4, 24, true, "Push for goal 3", "Goal 3 triggered" );
	
	if ( $session_data->get_nb_players() == 1 )
	{
		$text .= AddEntity_ButtonGoal( 5, -272, $y_5_6, 24, true,
			"Push to vote for more ammo\n".
			"(only your last vote count)", 
			"Voted for more ammo" );

		$text .= AddEntity_ButtonGoal( 7, -272, $y_7_8, 24, true, 
			"Push to vote for more health\n".
			"(only your last vote count)", 
			"Voted for more health" );
	}

//	$text .= AddEntity_ButtonGoal( 2, 280, $y_1_2, 24, true, "Push for goal 2", "Goal 2 triggered" );
//	$text .= AddEntity_ButtonGoal( 4, 280, $y_3_4, 24, true, "Push for goal 4", "Goal 4 triggered" );
//	$text .= AddEntity_ButtonGoal( 6, 280, $y_5_6, 24, true, "Push for goal 6", "Goal 6 triggered" );
//	$text .= AddEntity_ButtonGoal( 8, 280, $y_7_8, 24, true, "Push for goal 8", "Goal 8 triggered" );

	// Primary objective message
	$msg1 = 'Welcome to COOP or DIE!'.CR;
	$msg1 .= $session_dataset1->gametype ?
		'Don\'t forget you have only one life, be cautious ;-)' :
		'Try the challenge mode if you dare ;-)' ;
	$msg1 = wordwrap_fixed($msg1, 25, 4);
	
	// Secondary objective message
	$host_name = $player_array[0];
	$gametype = $session_dataset1->gametype ? 'challenge' : 'regular';
	$betatest = $session_dataset1->betatest ? ' (betatest)' : '';
	$msg2 = 'This game is hosted by '.$host_name.', and run in '.$gametype.' mode.'.$betatest;
	$msg2 = wordwrap_fixed($msg2, 25, 4);
	
	$text .=  
		'<entity_add>'.CR.
			'<entity>{ '.
				'"classname" "target_help" '.
				'"targetname" "t_help_msg" '.
				'"message" "'.$msg1.'" '.
				'"spawnflags" "1" '.			// Flag for primary objective message
				'"origin" "0 -1800 150" '.
			'}</entity>'.CR.
			'<entity>{ '.
				'"classname" "target_help" '.
				'"targetname" "t_help_msg" '.
				'"message" "'.$msg2.'" '.
				'"origin" "0 -1800 150" '.
			'}</entity>'.CR.
			'<entity>{ '.
				'"classname" "trigger_always" '.
				'"mins" "-300 -1800 88" '.
				'"maxs" "360 -1500 200" '.
				'"target" "t_help_msg" '.
				'"sounds" "2" '.
			'}</entity>'.CR.
		'</entity_add>'.CR;

	$text .= 
		'<cvar_forceset>'.CR.
		'<coop>1</coop>'.CR.
		'<skill>2</skill>'.CR.
		'<deathmatch>0</deathmatch>'.CR.
		'<ctf>0</ctf>'.CR.
		'</cvar_forceset>'.CR;

	return $text ;
}

?>