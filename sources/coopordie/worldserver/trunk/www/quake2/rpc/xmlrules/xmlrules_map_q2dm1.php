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
	if ( mt_rand(1,10) > 5 )
	{
	    $text = file_get_contents( 'xmlrules/xmlrules_map_q2dm1_andrei.xml' ) ;
		$msg = '(The Makron twins by ANDREI)';
	}
	else
	{
	    $text = file_get_contents( 'xmlrules/xmlrules_map_q2dm1_xrdvx.xml' ) ;
		$msg = '(The unnamed map by xRDVx)';
	}

    rpc_chat( rpc_player_name($id), $msg, MSG_FLAG_PUB_WSRV_LVL2);

    return $text;
}

?>