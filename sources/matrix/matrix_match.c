#include "g_local.h"

// Have a couple of states

// Warmup readying
// Warmup timed??? maybe
// Match timed

// Need Ready System
// rules checking
// start checking
void CheckWhosReady ()
{
}

qboolean MatrixMatchWin ()
{
	if ((matchtimelimit->value) &&
		(matrix.start_tank_time >= matchtimelimit->value*60))
		return true;
	else if ((teamfraglimit->value) &&
			((matrix.team_blue_score >= teamfraglimit->value) ||
			(matrix.team_red_score >= teamfraglimit->value)))
		return true;
	else
		return false;
}

void MatrixMatchThink ()
{
	int		i, numplayers = 0, total = 0;
	edict_t *player;

	if (!matchmode->value || 
		!tankmode->value)
		return;

	for(i=1; i <= maxclients->value; i++)
	{
		if ((player=&g_edicts[i]) && player->inuse)
		{
			total++;

			if (player->client->resp.spectator)
				continue;
			if (!player->client->resp.ready)
				continue;

			numplayers++;
		}
	}
	if ((numplayers > 1) && (numplayers == total)
		&& (!matrix.count_started) && (!matrix.started))
	{
		matrix.start_tank_time = level.time + 3;
		matrix.count_started = true;
		gi.dprintf ("DEBUG: Starting in 3 Sec\n");
	}

	if (matrix.start_tank_time == level.time)
	{
		matrix.count_started = false;
		matrix.started = true;
		// RESPAWNALLPEEPS
	}

	if (matrix.started && MatrixMatchWin())
		EndDMLevel();

}
void MatrixRespawn (edict_t *self, edict_t *attacker)
{
        //SKULL
        if (self->client->oldplayer)
                G_FreeEdict (self->client->oldplayer);
        if (self->client->chasecam)
                G_FreeEdict (self->client->chasecam);
        //END
	
		
	
	
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
	/*	
	if (IsNeutral(self))
	{
		ThrowGib (self, "players/cyborg/limb_rleg.md2", 150, GIB_ORGANIC);
		ThrowGib (self, "players/cyborg/limb_lleg.md2", 150, GIB_ORGANIC);		
		ThrowGib (self, "players/cyborg/limb_rarm.md2", 150, GIB_ORGANIC);		
		ThrowGib (self, "players/cyborg/limb_larm.md2", 150, GIB_ORGANIC);		
			
	}
	else if (IsFemale(self))
	{
	
		ThrowGib (self, "players/female/limb_rleg.md2", 150, GIB_ORGANIC);
		ThrowGib (self, "players/female/limb_lleg.md2", 150, GIB_ORGANIC);		
		ThrowGib (self, "players/female/limb_rarm.md2", 150, GIB_ORGANIC);		
		ThrowGib (self, "players/female/limb_larm.md2", 150, GIB_ORGANIC);		
	
	}
	else
	{
		ThrowGib (self, "players/male/limb_rleg.md2", 150, GIB_ORGANIC);
		ThrowGib (self, "players/male/limb_lleg.md2", 150, GIB_ORGANIC);		
		ThrowGib (self, "players/male/limb_rarm.md2", 150, GIB_ORGANIC);		
		ThrowGib (self, "players/male/limb_larm.md2", 150, GIB_ORGANIC);		
	}
	   */
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", 150, GIB_ORGANIC);

			//matrix PROPER gibs
		ThrowClientHead (self, 150);
		self->takedamage = DAMAGE_NO;
	
	self->s.modelindex2 = 0;	// remove linked weapon model
	self->s.modelindex3 = 0;
	self->s.modelindex4 = 0;

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;
	
	TossClientWeapon (self);
	ClientObituary (self, attacker, attacker);
	

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;
		//if (deathmatch->value || coop->value)
	//{
		self->movetype = MOVETYPE_TOSS;
		// spectator's don't leave bodies
		if (self->movetype != MOVETYPE_NOCLIP)
			CopyToBodyQue (self);

	//	self->movetype = MOVETYPE_NOCLIP;
		self->svflags &= ~SVF_NOCLIENT;
		PutClientInServer (self);

		// add a teleportation effect
//		self->s.event = EV_PLAYER_TELEPORT;


		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (self-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (self->s.origin, MULTICAST_PVS);

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;
		
		self->client->respawn_time = level.time;
		SV_AddBlend (0, 0.4, 0, 0.08, self->client->ps.blend);
		return;


}