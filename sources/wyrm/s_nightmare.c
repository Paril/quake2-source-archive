#include "g_local.h"
#include "s_nightmare.h"

float	nightmareToggleTime;
qboolean	nightmareModeState;
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);

/* _________ *\
** ¯¯¯¯¯¯¯¯¯ **
** Nightmare **
** _________ **
\* ¯¯¯¯¯¯¯¯¯ */
void nightmareModeSetup (void)
{
	if (nightmareModeState)
	{
		gi.configstring(CS_LIGHTS+0, "a");
		gi.configstring(CS_LIGHTS+1, "a");
		gi.configstring(CS_LIGHTS+2, "a");
		gi.configstring(CS_LIGHTS+3, "a");
		gi.configstring(CS_LIGHTS+4, "a");
		gi.configstring(CS_LIGHTS+5, "a");
		gi.configstring(CS_LIGHTS+6, "a");
		gi.configstring(CS_LIGHTS+7, "a");
		gi.configstring(CS_LIGHTS+8, "a");
		gi.configstring(CS_LIGHTS+9, "a");
		gi.configstring(CS_LIGHTS+10, "a");
		gi.configstring(CS_LIGHTS+11, "a");
		gi.configstring(CS_LIGHTS+63, "z");
	}
	else
	{
		gi.configstring(CS_LIGHTS+0, "m");
		gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");
		gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
		gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
		gi.configstring(CS_LIGHTS+4, "mamamamamama");
		gi.configstring(CS_LIGHTS+5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
		gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");
		gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");
		gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
		gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");
		gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");
		gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");
		gi.configstring(CS_LIGHTS+63, "a");
	}
	nightmareToggleTime = level.time;
}
void nightmareResetFlashlight (edict_t *self)
{
	if (self->owner->client && self->owner->client->flashlight)
	{
		float volume = 1.0;

		if (self->owner->client->silencer_shots)
			volume = 0.2;
		gi.sound(self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("misc/lasfly.wav"), volume, ATTN_NORM, 0);
		self->owner->client->flashlight = NULL;
                self->owner->client->flashtype = 0;
	}
	G_FreeEdict(self);
}
void flashlight_think (edict_t *self)
{
	vec3_t	forward, right, up, offset, start, end;
	trace_t	tr;
        int content= (CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
        if (level.intermissiontime || !self->owner->inuse || self->owner->deadflag)
	{
		nightmareResetFlashlight(self);
		return;
	}
	AngleVectors(self->owner->client->v_angle, forward, right, up);
	VectorSet(offset, 0, 8, self->owner->viewheight-8);
        P_ProjectSource(self->owner->client, self->owner->s.origin, offset, forward, right, start);
	VectorMA(start, 8192, forward, end);
        tr = gi.trace(start, NULL, NULL, end, self->owner, content);
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)) && tr.fraction < 1.0)
		self->svflags &= ~SVF_NOCLIENT;
	else
		self->svflags |= SVF_NOCLIENT;
	vectoangles(tr.plane.normal, self->s.angles);
	VectorCopy(tr.endpos, self->s.origin);
	self->nextthink = level.time+NIGHTMARE_THINK;

	gi.linkentity(self);
}
void Use_Flashlight (edict_t *player, gitem_t *item)
{
	if (player->client->flashlight)
        {
                if (player->client->flashtype == 0)
                {
                        player->client->flashtype = 1;
                        player->client->flashlight->s.effects = 0;
                        player->client->flashlight->s.renderfx = RF_TRANSLUCENT;
                        gi.linkentity(player->client->flashlight);
                }
                else if (player->client->flashtype == 1)
                {
                        player->client->flashtype = 2;
                        player->client->flashlight->s.effects = EF_HYPERBLASTER;
                        gi.linkentity(player->client->flashlight);
                }
                else
                {
                        player->client->flashtype = 0;
                        nightmareResetFlashlight(player->client->flashlight);
                }
        }
        else
	{
		float volume = 1.0;

		if (player->client->silencer_shots)
			volume = 0.2;
		gi.sound(player, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("misc/lasfly.wav"), volume, ATTN_NORM, 0);
		player->client->flashlight = G_Spawn();
		VectorCopy(player->s.origin, player->client->flashlight->s.origin);
		player->client->flashlight->movetype = MOVETYPE_NOCLIP;
		player->client->flashlight->clipmask = MASK_SHOT;
		player->client->flashlight->solid = SOLID_NOT;
		player->client->flashlight->owner = player;
		player->client->flashlight->nextthink = level.time+NIGHTMARE_THINK;
		player->client->flashlight->think = flashlight_think;
		player->client->flashlight->classname = "flashlight";
		player->client->flashlight->svflags = SVF_NOCLIENT;
                //Wyrm: invisible model
                player->client->flashlight->s.modelindex = gi.modelindex("models/lsight/tris.md2");
                player->client->flashlight->s.frame      = 0;
                player->client->flashlight->s.skinnum    = 0;

                VectorClear(player->client->flashlight->mins);
                VectorClear(player->client->flashlight->maxs);
                player->client->flashlight->s.effects = EF_HYPERBLASTER;
                player->client->flashlight->s.renderfx |= RF_TRANSLUCENT;
                player->client->flashlight->s.renderfx |= RF_FULLBRIGHT;
                gi.linkentity(player->client->flashlight);
	}
}
void nightmareCheckRules (void)
{
	if (nightmareModeState && (!nightmare->value || level.intermissiontime))
		nightmareModeState = false;
	else if (nightmare->value && !nightmareModeState && lighttime->value > 0 && level.time >= nightmareToggleTime+lighttime->value*60)
		nightmareModeState = true;
	else if (nightmareModeState && nighttime->value > 0 && level.time >= nightmareToggleTime+nighttime->value*60)
		nightmareModeState = false;
	else
		return;
	nightmareModeSetup();
}
void nightmareModeToggle (qboolean cmd)
{
	if (cmd)
	{
		if (gi.argc() > 2 && atoi(gi.argv(2)))
		{
			gi.cvar_forceset("nightmare", gi.argv(2));
			gi.bprintf(PRINT_HIGH, "Nightmare mode TOGGLE\n");
		}
		else if (nightmare->value)
		{
			gi.cvar_forceset("nightmare", "0");
			gi.bprintf(PRINT_HIGH, "Nightmare mode OFF\n");
		}
		else
		{
			gi.cvar_forceset("nightmare", "1");
			gi.bprintf(PRINT_HIGH, "Nightmare mode ON\n");
		}
	}
	else
	{
		nightmare = gi.cvar("nightmare", "0", CVAR_SERVERINFO|CVAR_LATCH);
		nighttime = gi.cvar("nighttime", "0", 0);
		lighttime = gi.cvar("lighttime", "0", 0);
	}
	if (nightmare->value > 0)
		nightmareModeState = true;
	else
		nightmareModeState = false;
	nightmareModeSetup();
}
void nightmarePlayerResetFlashlight (edict_t *player)
{
	if (player->client->flashlight)
		nightmareResetFlashlight(player->client->flashlight);
}

#if 0
void nightmareClientGiveFlashlight (gclient_t *client)
{
	if (nightmare->value)
	{
		gitem_t *item;

		item = FindItem("Flashlight");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
	}
}
#endif

void nightmareEffects (edict_t *player)
{
        if ((player->client->flashlight)&&
            (player->client->flashtype != 1))
                player->s.renderfx |= RF_FULLBRIGHT;
}
//Nightmare
