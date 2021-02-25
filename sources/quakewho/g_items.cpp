/*
Copyright (C) 1997-2001 Id Software, Inc.

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

#include "g_local.h"

gitem_t g_weapons[WEAP_TOTAL] =
{
	{
		Use_Weapon,
		Weapon_Blaster,
		"models/weapons/v_blast/tris.md2",
		"#w_blaster.md2",
		"w_blaster",
		"Blaster",
		0,
		AMMO_NONE,
		WEAP_BLASTER,
		"weapons/blastf1a.wav misc/lasfly.wav"
	},

	{
		Use_Weapon,
		Weapon_Shotgun,
		"models/weapons/v_shotg/tris.md2",
		"#w_shotgun.md2",
		"w_shotgun",
		"Shotgun",
		1,
		AMMO_SHELLS,
		WEAP_SHOTGUN,
		"weapons/shotgf1b.wav weapons/shotgr1b.wav"
	},

	/*{
		Use_Weapon,
		Weapon_Machinegun,
		"models/weapons/v_machn/tris.md2",
		"#w_machinegun.md2",
		"w_machinegun",
		"Machinegun",
		1,
		AMMO_BULLETS,
		WEAP_MACHINEGUN,
		"weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	},*/

	{
		Use_Weapon,
		Weapon_GrenadeLauncher,
		"models/weapons/v_launch/tris.md2",
		"#w_glauncher.md2",
		"w_glauncher",
		"Grenade Launcher",
		1,
		AMMO_GRENADES,
		WEAP_GRENADELAUNCHER,
		"models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	}
};

/*
===============
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
===============
*/
static void PrecacheItem (const gitem_t &it)
{
	if (it.vwep_model)
		gi.modelindex (it.vwep_model);
	if (it.view_model)
		gi.modelindex (it.view_model);
	if (it.icon)
		gi.imageindex (it.icon);

	// parse the space seperated precache string for other items
	const char *s = it.precaches;

	if (!s || !s[0])
		return;

	while (*s)
	{
		const char *start = s;

		while (*s && *s != ' ')
			s++;

		const size_t len = s - start;

		if (len >= MAX_QPATH || len < 5)
			gi.error ("%s: %s has bad precache string", __FUNCTION__, it.pickup_name);

		char data[MAX_QPATH];
		strncpy(data, start, len);

		if (*s)
			s++;

		// determine type based on extension
		if (!strcmp(data+len-3, "md2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "sp2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "wav"))
			gi.soundindex (data);
		if (!strcmp(data+len-3, "pcx"))
			gi.imageindex (data);
	}
}

void InitItems ()
{
	for (auto &weapon : g_weapons)
		PrecacheItem(weapon);
}
