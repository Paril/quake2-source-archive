//===========================================================================
// g_cmd_scopetoggle.c
//
// Predator gun scope.
// Originally coded by majoon.
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//===========================================================================

#include "g_local.h"

void Cmd_ScopeToggle_f (edict_t *ent)
{
	if (ent->isPredator)
	{
		if (ent->client->ps.fov == 90) {
			gi.cprintf(ent, PRINT_HIGH, "2x Zoom\n");
			ent->client->ps.fov = 60;
		}
		else if (ent->client->ps.fov == 60) {
			gi.cprintf(ent, PRINT_HIGH, "4x Zoom\n");
			ent->client->ps.fov = 40;
		}
		else if (ent->client->ps.fov == 40) {
			gi.cprintf(ent, PRINT_HIGH, "8x Zoom\n");
			ent->client->ps.fov = 20;
		}
		else {
			gi.cprintf(ent, PRINT_HIGH, "1x Zoom\n");
			ent->client->ps.fov = 90;
		}
	}
	else gi.cprintf(ent, PRINT_HIGH, "Space marines cannot use the gun scope.\n");
}
