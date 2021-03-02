#include "g_local.h"

void sp_respawn (edict_t *self);

// Class_Set - set's clients class, respawns
void Class_Set (edict_t *ent, int desired_class)
{
	switch (desired_class)
	{
		case INFANTRY: // infantry
			if((level.time - ent->client->respawn_time) < 1)
				return;
			ent->flags &= ~FL_GODMODE;
			ent->flags &= ~FL_NOTARGET;
			ent->health = 0;
			player_die (ent, ent, ent, 100000, vec3_origin);
			// don't even bother waiting for death frames
			ent->deadflag = DEAD_DEAD;
		    ent->client->resp.class = INFANTRY;
			ent->client->play_state = STATE_NEWCLASS;
			if (deathmatch->value)
				respawn (ent);
			else
				sp_respawn (ent);
			ent->svflags &= ~SVF_NOCLIENT;
			break;
		case RECON: // recon
			if((level.time - ent->client->respawn_time) < 1)
				return;
			ent->flags &= ~FL_GODMODE;
			ent->flags &= ~FL_NOTARGET;
			ent->health = 0;
			player_die (ent, ent, ent, 100000, vec3_origin);
			// don't even bother waiting for death frames
			ent->deadflag = DEAD_DEAD;
		    ent->client->resp.class = RECON;
			ent->client->play_state = STATE_NEWCLASS;
			if (deathmatch->value)
				respawn (ent);
			else
				sp_respawn (ent);
			ent->svflags &= ~SVF_NOCLIENT;
			break;
		case LIGHT_ASSAULT: // light assault
			if((level.time - ent->client->respawn_time) < 1)
				return;
			ent->flags &= ~FL_GODMODE;
			ent->flags &= ~FL_NOTARGET;
			ent->health = 0;
			player_die (ent, ent, ent, 100000, vec3_origin);
			// don't even bother waiting for death frames
			ent->deadflag = DEAD_DEAD;
		    ent->client->resp.class = LIGHT_ASSAULT;
			ent->client->play_state = STATE_NEWCLASS;
			if (deathmatch->value)
				respawn (ent);
			else
				sp_respawn (ent);
			ent->svflags &= ~SVF_NOCLIENT;
			break;
		case HEAVY_DEFENSE: // heavy defense
			if((level.time - ent->client->respawn_time) < 1)
				return;
			ent->flags &= ~FL_GODMODE;
			ent->flags &= ~FL_NOTARGET;
			ent->health = 0;
			player_die (ent, ent, ent, 100000, vec3_origin);
			// don't even bother waiting for death frames
			ent->deadflag = DEAD_DEAD;
			ent->client->play_state = STATE_NEWCLASS;
		    ent->client->resp.class = HEAVY_DEFENSE;
			if (deathmatch->value)
				respawn (ent);
			else
				sp_respawn (ent);
			ent->svflags &= ~SVF_NOCLIENT;
			break;
		case SEAL: // seal
			if((level.time - ent->client->respawn_time) < 1)
				return;
			ent->flags &= ~FL_GODMODE;
			ent->flags &= ~FL_NOTARGET;
			ent->health = 0;
			player_die (ent, ent, ent, 100000, vec3_origin);
			// don't even bother waiting for death frames
			ent->deadflag = DEAD_DEAD;
			ent->client->play_state = STATE_NEWCLASS;
		    ent->client->resp.class = SEAL;
			if (deathmatch->value)
				respawn (ent);
			else
				sp_respawn (ent);
			ent->svflags &= ~SVF_NOCLIENT;
			break;
		case ENGINEER: // engineer
			if((level.time - ent->client->respawn_time) < 1)
				return;
			ent->flags &= ~FL_GODMODE;
			ent->flags &= ~FL_NOTARGET;
			ent->health = 0;
			player_die (ent, ent, ent, 100000, vec3_origin);
			// don't even bother waiting for death frames
			ent->deadflag = DEAD_DEAD;
			ent->client->play_state = STATE_NEWCLASS;
		    ent->client->resp.class = ENGINEER;
			if (deathmatch->value)
				respawn (ent);
			else
				sp_respawn (ent);
			ent->svflags &= ~SVF_NOCLIENT;
			break;
		case SNIPER: // sniper
			if((level.time - ent->client->respawn_time) < 1)
				return;
			ent->flags &= ~FL_GODMODE;
			ent->flags &= ~FL_NOTARGET;
			ent->health = 0;
			player_die (ent, ent, ent, 100000, vec3_origin);
			// don't even bother waiting for death frames
			ent->deadflag = DEAD_DEAD;
			ent->client->play_state = STATE_NEWCLASS;
		    ent->client->resp.class = SNIPER;
			if (deathmatch->value)
				respawn (ent);
			else
				sp_respawn (ent);
			ent->svflags &= ~SVF_NOCLIENT;
			break;
		case DEMOLITIONS: // demolitions
			if((level.time - ent->client->respawn_time) < 1)
				return;
			ent->flags &= ~FL_GODMODE;
			ent->flags &= ~FL_NOTARGET;
			ent->health = 0;
			player_die (ent, ent, ent, 100000, vec3_origin);
			// don't even bother waiting for death frames
			ent->deadflag = DEAD_DEAD;
			ent->client->play_state = STATE_NEWCLASS;
		    ent->client->resp.class = DEMOLITIONS;
			if (deathmatch->value)
				respawn (ent);
			else
				sp_respawn (ent);
			ent->svflags &= ~SVF_NOCLIENT;
			break;
		case MEDIC: // medic
			if((level.time - ent->client->respawn_time) < 1)
				return;
			ent->flags &= ~FL_GODMODE;
			ent->flags &= ~FL_NOTARGET;
			ent->health = 0;
			player_die (ent, ent, ent, 100000, vec3_origin);
			// don't even bother waiting for death frames
			ent->deadflag = DEAD_DEAD;
		    ent->client->resp.class = MEDIC;
			ent->client->play_state = STATE_NEWCLASS;
			if (deathmatch->value)
				respawn (ent);
			else
				sp_respawn (ent);
			ent->svflags &= ~SVF_NOCLIENT;
			break;
		default:
			ent->client->resp.class = 0;
			break;
	}
}
