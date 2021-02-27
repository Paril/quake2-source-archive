/*
Copyright (C) 2001-2009 Pat AfterMoon (www.aftermoon.net)

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
// wsrv_trap.c -- WorldServer function trap 
// typical syntax is : trap_WSRV_functionName 
// where functionName is the initial Quake2 function

#include "../g_local.h"
#include "wsrv_trap.h"

//-----------------------------------------------------------------------------
// Call to WorldServer client console commands
//-----------------------------------------------------------------------------
qboolean trap_WSRV_ClientCommand(edict_t *ent, char *cmd, char *parm)
{
	if (Q_stricmp(cmd, "wsrv_login") == 0)
		Cmd_WSrv_Login_f(ent);
	else if (Q_stricmp(cmd, "wsrv_logout") == 0)
		Cmd_WSrv_Logout_f(ent);
	else
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Report every kills to the WorldServer
//-----------------------------------------------------------------------------
void trap_WSRV_Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if ( worldserver->value )
		WSrv_Killed(targ, inflictor, attacker);
}

//-----------------------------------------------------------------------------
// Report every goals reached to the WorldServer
//-----------------------------------------------------------------------------
void trap_WSRV_UseTargetGoal(edict_t *ent, edict_t *other, edict_t *activator)
{
    if ( worldserver->value )
	{
        WSRV_UseTarget(ent, other, activator);
		WSrv_BypassCheckWorldServer();	// Send it to the worldserver ASAP
	}
}

//-----------------------------------------------------------------------------
// Report every secret found to the WorldServer
//-----------------------------------------------------------------------------
void trap_WSRV_UseTargetSecret(edict_t *ent, edict_t *other, edict_t *activator)
{
    if ( worldserver->value )
        WSRV_UseTarget(ent, other, activator);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void trap_WSRV_ShutdownGame()
{
	int	i;

	if ( worldserver->value )
	{
		char    szCmd[32];

		for ( i=0 ; i<maxclients->value ; i++ )
		{
			edict_t *cl_ent = g_edicts + 1 + i;
			if ( game.clients[i].pers.connected || cl_ent->inuse )
			{
				Com_sprintf(szCmd, sizeof(szCmd), "SetInventory,%s", game.clients[i].pers.netname);
				WSrv_AddCmd(szCmd);
			}
		}

		WSrv_AddCmd("Logout");
		WSrv_CheckWorldServer(true);    // Flush the command buffer
	}

	if ( worldserver->value || (worldserver->latched_string && atoi(worldserver->latched_string)) )
		WSrv_SerializeWsrvData("tag_worldserver");

}


//-----------------------------------------------------------------------------
// ExitLevel : save data of all clients (not required but safe)
//-----------------------------------------------------------------------------
void trap_WSRV_ExitLevel()
{
	int		i;
	char	*pcChar;


	if ( worldserver->value )
	{
		char    szCmd[32];

		for ( i=0 ; i<maxclients->value ; i++ )
		{
			edict_t *cl_ent = g_edicts + 1 + i;
			if ( game.clients[i].pers.connected || cl_ent->inuse )
			{
				Com_sprintf(szCmd, sizeof(szCmd), "SetInventory,%s", game.clients[i].pers.netname);
				//$$WSrv_SendCmdImmediate(szCmd);
				WSrv_AddCmd(szCmd);
			}
		}
		// <worldserver> Unlock the map command n time for manage the '+' sign in changemap
		i = 1;
		for ( pcChar = level.changemap ; *pcChar ; pcChar++ )
			if ( *pcChar == '+' )
				i++ ;
		WSrv_SetMapLock(i);
	}
}


//-----------------------------------------------------------------------------
// Check if a command is queued
//-----------------------------------------------------------------------------
void trap_WSRV_RunFrame()
{
	if ( worldserver->value )
		WSrv_CheckWorldServer(false);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void trap_WSRV_InitGame()
{
	// Existing cvar overlaped for the flag CVAR_NOSET
	port = gi.cvar ("port", "0", CVAR_NOSET);
	gi.cvar ("name", "0", CVAR_NOSET);      // Just for write protect this cvar
	gi.cvar ("hostname", "0", CVAR_NOSET);  // Just for write protect this cvar

	// new specifics cvar
	worldserver   = gi.cvar ("worldserver",  "0", CVAR_LATCH);
	wsrv_url      = gi.cvar ("wsrv_url",     "0", CVAR_NOSET);
	wsrv_debug    = gi.cvar ("wsrv_debug",   "0", CVAR_ARCHIVE);
	xmlrules      = gi.cvar ("xmlrules",      "", CVAR_NOSET);
	challengemode = gi.cvar ("challengemode","0", CVAR_NOSET);

	// For preserving worldserver data throught a dll reload, it is serialized and
	// stored as text in the cvar "tag_worldserver". If the "worldserver" cvar is set,
	// so "tag_worldserver" must contain data.
	if ( worldserver->value )
	{
		cvar_t	*tag_worldserver = gi.cvar ("tag_worldserver",  "", CVAR_NOSET);
		if ( tag_worldserver->string[0] != '\0' )
			WSrv_DeserializeWsrvData(tag_worldserver->string);
		else
		{
			gi.error("tag_worldserver not found\n");
		}
	}
}

//-----------------------------------------------------------------------------
// Call to WorldServer server console commands
//-----------------------------------------------------------------------------
qboolean trap_WSRV_ServerCommand(char *cmd)
{
	if ( dedicated->value )
	{
		if (Q_stricmp(cmd, "wsrv_login") == 0)
			Cmd_WSrv_Login_f(NULL);
		else if (Q_stricmp(cmd, "wsrv_logout") == 0)
			Cmd_WSrv_Logout_f(NULL);
		else
			return false;
	}
	else
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Create and begin a new intermission with a given map and coordinates
// changemap		= name of the map to go
// origin, angles	= origin and angles for the intermission view
//-----------------------------------------------------------------------------
void BeginNewIntermission (char *changemap, vec3_t origin, vec3_t angles)
{
	int		i;
	edict_t	*client;

	if (level.intermissiontime)
		return;		// already activated

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		if (client->health <= 0)
			respawn(client);
	}

	level.intermissiontime = level.time;
	level.changemap = changemap;

	level.exitintermission = 0;

	VectorCopy (origin, level.intermission_origin);
	VectorCopy (angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}
}

//-----------------------------------------------------------------------------
// Reset a player after he die
//-----------------------------------------------------------------------------
qboolean trap_WSRV_player_die1(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if ( worldserver->value )
	{
		gitem_t		*item;
	
		// clear inventory
		memset (&self->client->resp.coop_respawn, 0, sizeof(self->client->resp.coop_respawn));
		item = FindItem("Blaster");

		self->client->resp.coop_respawn.inventory[ITEM_INDEX(item)] = 1;
		self->client->resp.coop_respawn.weapon = item;

		self->client->resp.coop_respawn.health			= 100;
		self->client->resp.coop_respawn.max_health		= 100;

		self->client->resp.coop_respawn.max_bullets		= 200;
		self->client->resp.coop_respawn.max_shells		= 100;
		self->client->resp.coop_respawn.max_rockets		= 50;
		self->client->resp.coop_respawn.max_grenades	= 50;
		self->client->resp.coop_respawn.max_cells		= 200;
		self->client->resp.coop_respawn.max_slugs		= 50;

		self->client->resp.coop_respawn.connected		= true;

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Begin an intermission with the view of the dead player
//-----------------------------------------------------------------------------
void trap_WSRV_player_die2(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t	intermission_angle, intermission_origin;
	edict_t	*client;
	int		i;

	if (coop->value)
	{
		VectorCopy (self->s.origin, intermission_origin);
		VectorSet (intermission_angle, -15, self->client->killer_yaw, 40);
		BeginNewIntermission ("*startmap", intermission_origin, intermission_angle);

		for (i=0 ; i<maxclients->value ; i++)
		{
			client = g_edicts + 1 + i;
			if (client->inuse)
				gi.centerprintf(client, "%s has died.\nHis deeds of valor will be remembered", self->client->pers.netname);
		}
	}
}

//-----------------------------------------------------------------------------
// ClientConnect: inform the WorldServer + manage the regular/challenge mode
//-----------------------------------------------------------------------------
qboolean trap_WSRV_ClientConnect(edict_t *ent, char *userinfo)
{
	vec3_t	intermission_angle, intermission_origin;
	edict_t	*entSpot, *client;
	int		i;

	if ( worldserver->value )
	{
		WSrv_ClientConnect(ent);  
	}

	// if the client is not the server and we are in challenge mode
	if ( ent != (g_edicts+1) && challengemode->value )
	{
		if ( strcmp(level.mapname,"startmap")!=0 )
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (client->inuse)
				{
					gi.centerprintf(client, "%s can't join outside StartMap", ent->client->pers.netname);
				}
			}

			Info_SetValueForKey(userinfo, "rejmsg", "Games in challenge mode can only be reached in the StartMap");
			return false;
		}
	}

	// If the client is not the server and we are in the startmap, we reload 
	// the map in both modes (challenge or regular)
	if ( ent != (g_edicts+1) && strcmp(level.mapname,"startmap")==0 )
	{
		// find an intermission spot
		entSpot = G_Find (NULL, FOFS(classname), "info_player_intermission");
		if (!entSpot)	// the map creator forgot to put in an intermission point...
		{
			entSpot = G_Find (NULL, FOFS(classname), "info_player_start");
			if (!entSpot)
			{
				entSpot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
			}
		}

		VectorCopy (entSpot->s.origin, intermission_origin);
		VectorCopy (entSpot->s.angles, intermission_angle);

		BeginNewIntermission ("*startmap", intermission_origin, intermission_angle);

		for (i=0 ; i<maxclients->value ; i++)
		{
			client = g_edicts + 1 + i;
			if (client->inuse)
			{
				gi.centerprintf(client, "%s entered the game.\nReload the startmap", ent->client->pers.netname);
			}
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// ClientDisconnect : save data of the client
//-----------------------------------------------------------------------------
void trap_WSRV_ClientDisconnect(edict_t *ent)
{
	if ( worldserver->value )
	{
		char    szCmd[32];
		Com_sprintf(szCmd, sizeof(szCmd), "SetInventory,%s", ent->client->pers.netname);
		WSrv_SendCmdImmediate(szCmd);
		
		WSrv_ClientDisconnect(ent->client->pers.netname);
	}
}

