/*
Copyright (C) 2004 Nathan 'jitspoe' Wulf / Digital Paint

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "../client/client.h"
#include "xfiregameclient.h"

void CL_Xfire (void)
{
	// xfiretest
	char *keys[3], *values[3];
	int pic;
	char *picname;
	int i;
	static char szLastValue[64] = "Not Playing";

	keys[0] = "Team";
	values[0] = "Not Playing";

	// Hack to get player's team color from the HUD
	for (i = STAT_PB_TEAM1; i <= STAT_PB_TEAM4; i++)
	{
		if (pic = cl.frame.playerstate.stats[i])
		{
			picname = cl.configstrings[CS_IMAGES + pic];

			if (picname)
			{
				if (strncmp(picname, "i_red", 5) == 0 && strchr(picname, '2'))
					values[0] = "Red";
				else if (strncmp(picname, "i_blue", 5) == 0 && strchr(picname, '2'))
					values[0] = "Blue";
				else if (strncmp(picname, "i_yellow", 5) == 0 && strchr(picname, '2'))
					values[0] = "Yellow";
				else if (strncmp(picname, "i_purple", 5) == 0 && strchr(picname, '2'))
					values[0] = "Purple";
			}
		}
	}

	if (!Q_streq(szLastValue, values[0]))
	{
		XfireSetCustomGameData(1, keys, values);
		Q_strncpyz(szLastValue, values[0], sizeof(szLastValue));
	}
}
