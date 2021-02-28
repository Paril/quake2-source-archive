#include "g_local.h"
#include "m_player.h"
// Nodes
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <malloc.h>

// Paril
void Bot_Respawn(edict_t *ent);
// Paril
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ChasecamRemove(edict_t *ent);
qboolean Started_Grappling(gclient_t *client);
qboolean Ended_Grappling(gclient_t *client);
qboolean Is_Grappling(gclient_t *client);
void Throw_Grapple(edict_t *ent);
void Release_Grapple(edict_t *ent);
void Think_Grapple(edict_t *ent);
void Pull_Grapple(edict_t *ent);

void SP_misc_teleporter_dest (edict_t *ent);

void SV_SetClientAnimation(edict_t *ent,int startframe,int endframe,int priority) {
  if (!ent->client) return;
  ent->client->anim_priority=priority;
  ent->s.frame=startframe;
  ent->client->anim_end=endframe;
  gi.linkentity(ent);
}

void G_Spawn_Sparks(int type, vec3_t start, vec3_t movdir, vec3_t origin ) {
gi.WriteByte(svc_temp_entity);
gi.WriteByte(type);
gi.WritePosition(start);
gi.WriteDir(movdir);
gi.multicast(origin, MULTICAST_PVS);
}

//======================================================
// True if Ent is valid, has client, and edict_t inuse.
//======================================================
qboolean G_EntExists(edict_t *ent) {
return ((ent) && (ent->client) && (ent->inuse));
}

//======================================================
// True if ent is not DEAD or DEAD or DEAD (and BURIED!)
//======================================================
qboolean G_ClientNotDead(edict_t *ent) {
qboolean buried=true;
qboolean b1=ent->client->ps.pmove.pm_type!=PM_DEAD;
qboolean b2=ent->deadflag != DEAD_DEAD;
qboolean b3=ent->health > 0;
return (b3||b2||b1)&&(buried);
}


//======================================================
// True if ent is not DEAD and not just did a Respawn.
//======================================================
qboolean G_ClientInGame(edict_t *ent) {
if (!G_EntExists(ent)) return false;
if (!G_ClientNotDead(ent)) return false;
return (ent->client->respawn_time + 5.0 < level.time);
}

void CTFAssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];
	
	if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
		ent->client->resp.bloody = 1;
	else
		ent->client->resp.bloody = 0;
	
	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)
		p[1] = 0;
	else if (ent->client->resp.class == 1)
		strcpy(t, "soldier/");
	else if (ent->client->resp.class == 3)
		strcpy(t, "tank/");
	else
		strcpy(t, "male/");

	if (ctf->value)
	{
			// ===========================================
			// ===========================================
			// Start CTF switch
			// ===========================================
			// ===========================================
		switch (ent->client->resp.class) 
		{
		case 0:
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
				ent->client->pers.netname, t, "grunt") );
			break;
		case 1:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "soldier/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "soldier/", "ctf_b") );
			}
			break;
		case 36:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "soldier/", "scctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "soldier/", "scctf_b") );
			}
			break;
		case 2:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "infantry/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "infantry/", "ctf_b") );
			}
			break;
		case 37:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "traitor/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "traitor/", "ctf_b") );
			}
			break;
		case 38:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "traitf/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "traitf/", "ctf_b") );
			}
			break;
		case 3:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "ctf_r") );
			}
			else	
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "ctf_b") );
			}
			break;
		case 33:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "dtctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "dtctf_b") );
			}
			break;
		case 4:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "medic/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "medic/", "ctf_b") );
			}
			break;
		case 5:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gunner/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "gunner/", "ctf_b") );
			}
			break;
		case 6:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gladiator/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "gladiator/", "ctf_b") );
			}
			break;
		case 34:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gladiator/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "gladiator/", "ctf_b") );
			}
			break;
		case 7:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stank/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "stank/", "ctf_b") );
			}
			break;
		case 39:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stank/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "stank/", "ctf_b") );
			}
			break;
		case 8:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "flyer/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "flyer/", "ctf_b") );
			}
			break;
		case 35:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "flyer/", "hornctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "flyer/", "hornctf_b") );
			}
			break;
		case 9:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "berserk/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "berserk/", "ctf_b") );
			}
			break;
		case 10:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "maiden/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "maiden/", "ctf_b") );
			}
			break;
		case 11:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "boss2/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "boss2/", "ctf_b") );
			}
			break;
		case 12:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "brain/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "brain/", "ctf_b") );
			}
			break;
		case 13:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "mutant/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "mutant/", "ctf_b") );
			}
			break;
		case 14:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "cctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "tank/", "cctf_b") );
			}
			break;
		case 41:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "cctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "tank/", "cctf_b") );
			}
			break;
		case 40:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "parasite/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "parasite/", "ctf_b") );
			}
			break;
		case 15:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "hover/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "hover/", "ctf_b") );
			}
			break;
		case 16:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "float/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "float/", "ctf_b") );
			}
			break;
		case 17:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "flipper/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "flipper/", "ctf_b") );
			}
			break;
		case 18:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "jorg/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "jorg/", "ctf_b") );
			}
			break;
		case 19:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "rider/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "rider/", "ctf_b") );
			}
			break;
			// Mission Pack 1
		case 20:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "soldierh/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "soldierh/", "ctf_b") );
			}
			break;
		case 21:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gladb/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "gladb/", "ctf_b") );
			}
			break;
		case 22:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "brain/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "brain/", "ctf_b") );
			}
			break;
		case 23:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "maiden/", "betactf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "maiden/", "betactf_b") );
			}
			break;
		case 24:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stank/", "betactf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "stank/", "betactf_b") );
			}
			break;
		case 25:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gekk/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "gekk/", "ctf_b") );
			}
			break;
		case 32:
			// Fixxy bot has no pain skinzzz yet
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "fixbot/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "fixbot/", "ctf_b") );
			}
			break;
			// Mission Pack 2
		case 26:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "hover/", "rctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "hover/", "rctf_b") );
			}
			break;
		case 27:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stalker/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "stalker/", "ctf_b") );
			}
			break;
		case 28:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "medic/", "rctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "medic/", "rctf_b") );
			}
			break;
		case 29:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "carrier/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "carrier/", "ctf_b") );
			}
			break;
		case 30:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "blackwidow/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "blackwidow/", "ctf_b") );
			}
			break;
		case 31:
			if (ent->client->resp.ctf_team == CTF_TEAM1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "blackwidow2/", "ctf_r") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "blackwidow2/", "ctf_b") );
			}
			break;
		default:
			gi.configstring (CS_PLAYERSKINS+playernum, 
				va("%s\\%s", ent->client->pers.netname, s) );
			break;
			// ===========================================
			// ===========================================
			// End CTF switch
			// ===========================================
			// ===========================================
		}
	}
	else
	{
		switch (ent->client->resp.class) 
		{
		case 0:
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
				ent->client->pers.netname, t, "grunt") );
			break;
		case 1:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "soldier/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "soldier/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "soldier/", "skin") );
			}
			break;
		case 36:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "soldier/", "skin_scp") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "soldier/", "skin_sc") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "soldier/", "skin_sc") );
			}
			break;
		case 2:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "infantry/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "infantry/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "infantry/", "skin") );
			}
			break;
		case 37:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "traitor/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "traitor/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "traitor/", "skin") );
			}
			break;
		case 38:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "traitf/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "traitf/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "traitf/", "skin") );
			}
			break;
		case 3:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "skin") );
			}
			else	
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "skin") );
			}
			break;
		case 33:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "dtpain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "dtskin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "dtskin") );
			}
			break;
		case 4:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "medic/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "medic/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "medic/", "skin") );
			}
			break;
		case 5:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gunner/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gunner/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "gunner/", "skin") );
			}
			break;
		case 6:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gladiator/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gladiator/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "gladiator/", "skin") );
			}
			break;
		case 34:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gladiator/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gladiator/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "gladiator/", "skin") );
			}
			break;
		case 7:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stank/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stank/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "stank/", "skin") );
			}
			break;
		case 39:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stank/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stank/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "stank/", "skin") );
			}
			break;
		case 8:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "flyer/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "flyer/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "flyer/", "skin") );
			}
			break;
		case 35:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "flyer/", "horn_p") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "flyer/", "hornet") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "flyer/", "hornet") );
			}
			break;
		case 9:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "berserk/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "berserk/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "berserk/", "skin") );
			}
			break;
		case 10:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "maiden/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "maiden/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "maiden/", "skin") );
			}
			break;
		case 11:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "boss2/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "boss2/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "boss2/", "skin") );
			}
			break;
		case 12:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "brain/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "brain/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "brain/", "skin") );
			}
			break;
		case 13:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "mutant/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "mutant/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "mutant/", "skin") );
			}
			break;
		case 14:
		/*
		if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
		{
		gi.configstring (CS_PLAYERSKINS+playernum,
		va("%s\\%s%s", ent->client->pers.netname, "parasite/", "pain") );
		}
		else if (ent->health < 1)
		{
		gi.configstring (CS_PLAYERSKINS+playernum,
		va("%s\\%s%s", ent->client->pers.netname, "parasite/", "skin") );
		}
		else
		{
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
		ent->client->pers.netname, "parasite/", "skin") );
		}
			break;*/
			// Why bother, he's being ded anyway
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "cpain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "cskin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "tank/", "cskin") );
			}
			break;
		case 41:
			// Why bother, he's being ded anyway
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "cpain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "tank/", "cskin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "tank/", "cskin") );
			}
			break;
		case 40:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "parasite/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "parasite/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "parasite/", "skin") );
			}
			break;		case 15:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "hover/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "hover/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "hover/", "skin") );\
			}
			break;
		case 16:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "float/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "float/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "float/", "skin") );
			}
			break;
		case 17:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "flipper/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "flipper/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "flipper/", "skin") );
			}
			break;
		case 18:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "jorg/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "jorg/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "jorg/", "skin") );
			}
			break;
		case 19:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "rider/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "rider/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "rider/", "skin") );
			}
			break;
			// Mission Pack 1
		case 20:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "soldierh/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "soldierh/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "soldierh/", "skin") );
			}
			break;
		case 21:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gladb/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gladb/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "gladb/", "skin") );
			}
			break;
		case 22:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "brain/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "brain/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s%s", ent->client->pers.netname, "brain/", "skin") );
			}
			break;
		case 23:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "maiden/", "bpain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "maiden/", "beta") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "maiden/", "beta") );
			}
			break;
		case 24:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stank/", "pbeta") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stank/", "beta") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "stank/", "beta") );
			}
			break;
		case 25:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gekk/", "gekpain2") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "gekk/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "gekk/", "skin") );
			}
			break;
		case 32:
			// Fixxy bot has no pain skinzzz yet
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "fixbot/", "skin") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "fixbot/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "fixbot/", "skin") );
			}
			break;
			// Mission Pack 2
		case 26:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "hover/", "rpain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "hover/", "rskin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "hover/", "rskin") );
			}
			break;
		case 27:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stalker/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "stalker/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "stalker/", "skin") );
			}
			break;
		case 28:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "medic/", "rpain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "medic/", "rskin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "medic/", "rskin") );
			}
			break;
		case 29:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "carrier/", "pain2") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "carrier/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "carrier/", "skin") );
			}
			break;
		case 30:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "blackwidow/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "blackwidow/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "blackwidow/", "skin") );
			}
			break;
		case 31:
			if ((ent->health < (ent->max_health / 2)) && (ent->health > 0))
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "blackwidow2/", "pain") );
			}
			else if (ent->health < 1)
			{
				gi.configstring (CS_PLAYERSKINS+playernum,
					va("%s\\%s%s", ent->client->pers.netname, "blackwidow2/", "skin") );
			}
			else
			{
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
					ent->client->pers.netname, "blackwidow2/", "skin") );
			}
			break;
		default:
			gi.configstring (CS_PLAYERSKINS+playernum, 
				va("%s\\%s", ent->client->pers.netname, s) );
			break;
		}
	}
}

//Bot
#define OAK_FIND_RANGE 700.00

#define OAK_WALK	16
#define OAK_RUN		36

/************************************************
*
* Prototypes
*
************************************************/

// oak.c 
void OAK_Check_SP(edict_t *ent);
void SP_Oak(void);
void OAK_Respawn(edict_t *self);

  

// oakai.c
void oak_stand(edict_t *self);
void oak_run(edict_t *self);
void oak_standclose(edict_t *self);

void oak_pain(edict_t *self, edict_t *other, float kick, int damage);
void oak_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void oak_painthink(edict_t *self);

void OakAI_FaceEnemy(edict_t *self);
void OakAI_MoveToEnemy(edict_t *self, float dist);
void OakAI_MoveToGoal (edict_t *ent, float dist);

void OakAI_RunFrames(edict_t *self, int start, int end);
void OakAI_Point(edict_t *self);
void OakAI_Finger(edict_t *self);
void OakAI_Salute(edict_t *self);
void OakAI_Taunt(edict_t *self);
void OakAI_Wave(edict_t *self);

// from id's code
void TossClientWeapon (edict_t *self);
qboolean SV_CloseEnough (edict_t *ent, edict_t *goal, float dist);
qboolean SV_StepDirection (edict_t *ent, float yaw, float dist);









//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

/*we get silly velocity-effects when we are on ground and try to
  accelerate, so lift us a little bit if possible*/
qboolean Jet_AvoidGround( edict_t *ent )
{
  vec3_t         new_origin;
  trace_t trace;
  qboolean       success;

	if (ent->health < 1)
		return false;
 
  /*Check if there is enough room above us before we change origin[2]*/
  new_origin[0] = ent->s.origin[0];
  new_origin[1] = ent->s.origin[1];
  new_origin[2] = ent->s.origin[2] + 0.5;
  trace = gi.trace( ent->s.origin, ent->mins, ent->maxs, new_origin, ent, MASK_MONSTERSOLID );
 
  if ( success=(trace.plane.normal[2]==0) )    /*no ceiling?*/
    ent->s.origin[2] += 0.5;                   /*then make sure off ground*/
 
  return success;
}
 
 
/*This function returns true if the jet is activated
  (surprise, surprise)*/
qboolean Jet_Active( edict_t *ent )
{
	if (ent->health < 1)
		return false;
  return ( ent->client->Jet_framenum >= level.framenum );
}

/*If a player dies with activated jetpack this function will be called
  and produces a little explosion*/
void Jet_BecomeExplosion( edict_t *ent, int damage )
{
  int    n;
 
  gi.WriteByte( svc_temp_entity );
  gi.WriteByte( TE_EXPLOSION1 );   /*TE_EXPLOSION2 is possible too*/
  gi.WritePosition( ent->s.origin );
  gi.multicast( ent->s.origin, MULTICAST_PVS );
  gi.sound( ent, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0 );
 
  /*throw some gib*/
  for ( n=0; n<4; n++ )     ThrowGib( ent, "models/objects/gibs/sm_meat/tris.md2");   ThrowClientHead( ent);   ent->takedamage = DAMAGE_NO;
 
}
 
 
/*The lifting effect is done through changing the origin, it
  gives the best results. Of course its a little dangerous because
  if we dont take care, we can move into solid*/
void Jet_ApplyLifting( edict_t *ent )
{
  float          delta;
  vec3_t new_origin;
  trace_t trace;
  int            time = 24;     /*must be >0, time/10 = time in sec for a
                                 complete cycle (up/down)*/
  float          amplitude = 2.0;

	if (ent->health < 1)
		return;
 
  /*calculate the z-distance to lift in this step*/
  delta = sin( (float)((level.framenum%time)*(360/time))/180*M_PI ) * amplitude;
  delta = (float)((int)(delta*8))/8; /*round to multiples of 0.125*/
 
  VectorCopy( ent->s.origin, new_origin );
  new_origin[2] += delta;
 
  if( VectorLength(ent->velocity) == 0 )
  {
     /*i dont know the reason yet, but there is some floating so we
       have to compensate that here (only if there is no velocity left)*/
     new_origin[0] -= 0.125;
     new_origin[1] -= 0.125;
     new_origin[2] -= 0.125;
  }
 
  /*before we change origin, its important to check that we dont go
    into solid*/
  trace = gi.trace( ent->s.origin, ent->mins, ent->maxs, new_origin, ent, MASK_MONSTERSOLID );
  if ( trace.plane.normal[2] == 0 )
    VectorCopy( new_origin, ent->s.origin );
}
 
 
/*This function applys some sparks to your jetpack, this part is
  exactly copied from Muce's and SumFuka's JetPack-tutorial and does a
  very nice effect.*/
void Jet_ApplySparks ( edict_t *ent )
{
  vec3_t  forward, right;
  vec3_t  pack_pos, jet_vector;

	if (ent->health < 1)
		return;
 
  AngleVectors(ent->client->v_angle, forward, right, NULL);
  VectorScale (forward, -7, pack_pos);
  VectorAdd (pack_pos, ent->s.origin, pack_pos);
  pack_pos[2] += (ent->viewheight);
  VectorScale (forward, -50, jet_vector);
 
  gi.WriteByte (svc_temp_entity);
  gi.WriteByte (TE_SPARKS);
  gi.WritePosition (pack_pos);
  gi.WriteDir (jet_vector);
  gi.multicast (pack_pos, MULTICAST_PVS);
}
 
 
/*if the angle of the velocity vector is different to the viewing
  angle (flying curves or stepping left/right) we get a dotproduct
  which is here used for rolling*/
void Jet_ApplyRolling( edict_t *ent, vec3_t right )
{
  float roll,
        value = 0.05,
        sign = -1;    /*set this to +1 if you want to roll contrariwise*/

	if (ent->health < 1)
		return;
 
  roll = DotProduct( ent->velocity, right ) * value * sign;
  ent->client->kick_angles[ROLL] = roll;
}
 
 
/*Now for the main movement code. The steering is a lot like in water, that
  means your viewing direction is your moving direction. You have three
  direction Boosters: the big Main Booster and the smaller up-down and
  left-right Boosters.
  There are only 2 adds to the code of the first tutorial: the Jet_next_think
  and the rolling.
  The other modifications results in the use of the built-in quake functions,
  there is no change in moving behavior (reinventing the wheel is a lot of
  "fun" and a BIG waste of time ;-))*/
void Jet_ApplyJet( edict_t *ent, usercmd_t *ucmd )
{
  float  direction;
  vec3_t acc;
  vec3_t forward, right;
  int    i;
 
	if (ent->health < 1)
		return;

  /*clear gravity so we dont have to compensate it with the Boosters*/
  ent->client->ps.pmove.gravity = 0;
 
  /*calculate the direction vectors dependent on viewing direction
    (length of the vectors forward/right is always 1, the coordinates of
    the vectors are values of how much youre looking in a specific direction
    [if youre looking up to the top, the x/y values are nearly 0 the
    z value is nearly 1])*/
  AngleVectors( ent->client->v_angle, forward, right, NULL );
 
  /*Run jet only 10 times a second so movement dont depends on fps
    because ClientThink is called as often as possible
    (fps<10 still is a problem ?)*/   if ( ent->client->Jet_next_think <= level.framenum )
  {
    ent->client->Jet_next_think = level.framenum + 1;
 
    /*clear acceleration-vector*/
    VectorClear( acc );
 
    /*if we are moving forward or backward add MainBooster acceleration
      (60)*/
    if ( ucmd->forwardmove )
    {
      /*are we accelerating backward or forward?*/
      direction = (ucmd->forwardmove<0) ? -1.0 : 1.0;        /*add the acceleration for each direction*/       acc[0] += direction * forward[0] * 60;       acc[1] += direction * forward[1] * 60;       acc[2] += direction * forward[2] * 60;     }      /*if we sidestep add Left-Right-Booster acceleration (40)*/     if ( ucmd->sidemove )
    {
      /*are we accelerating left or right*/
      direction = (ucmd->sidemove<0) ? -1.0 : 1.0;        /*add only to x and y acceleration*/       acc[0] += right[0] * direction * 40;       acc[1] += right[1] * direction * 40;     }      /*if we crouch or jump add Up-Down-Booster acceleration (30)*/     if ( ucmd->upmove )
      acc[2] += ucmd->upmove > 0 ? 30 : -30;
 
    /*now apply some friction dependent on velocity (higher velocity results
      in higher friction), without acceleration this will reduce the velocity
      to 0 in a few steps*/
    ent->velocity[0] += -(ent->velocity[0]/6.0);
    ent->velocity[1] += -(ent->velocity[1]/6.0);
    ent->velocity[2] += -(ent->velocity[2]/7.0);
 
    /*then accelerate with the calculated values. If the new acceleration for
      a direction is smaller than an earlier, the friction will reduce the speed
      in that direction to the new value in a few steps, so if youre flying
      curves or around corners youre floating a little bit in the old direction*/
    VectorAdd( ent->velocity, acc, ent->velocity );
 
    /*round velocitys (is this necessary?)*/
    ent->velocity[0] = (float)((int)(ent->velocity[0]*8))/8;
    ent->velocity[1] = (float)((int)(ent->velocity[1]*8))/8;
    ent->velocity[2] = (float)((int)(ent->velocity[2]*8))/8;
 
    /*Bound velocitys so that friction and acceleration dont need to be
      synced on maxvelocitys*/
    for ( i=0 ; i<2 ; i++) /*allow z-velocity to be greater*/     {       if (ent->velocity[i] > 300)
        ent->velocity[i] = 300;
      else if (ent->velocity[i] < -300)
        ent->velocity[i] = -300;
    }
 
    /*add some gentle up and down when idle (not accelerating)*/
    if( VectorLength(acc) == 0 )
      Jet_ApplyLifting( ent );
 
  }//if ( ent->client->Jet_next_think...
 
  /*add rolling when we fly curves or boost left/right*/
  Jet_ApplyRolling( ent, right );

  if (ent->client->resp.class == 8)
  {
	if (ent->client->resp.next_thrust_sound < level.time)
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("flyer/Flyidle1.wav"), 1, ATTN_NORM, 0);
		ent->client->resp.next_thrust_sound=level.time+1.29;
	}
  }
  if (ent->client->resp.class == 35)
  {
	if (ent->client->resp.next_thrust_sound < level.time)
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("flyer/Flyidle1.wav"), 1, ATTN_NORM, 0);
		ent->client->resp.next_thrust_sound=level.time+1.29;
	}
  }
  if (ent->client->resp.class == 32)
  {
	if (ent->client->resp.next_thrust_sound < level.time)
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("flyer/Flyidle1.wav"), 1, ATTN_NORM, 0);
		ent->client->resp.next_thrust_sound=level.time+1.29;
	}
  }
  else if (ent->client->resp.class == 15)
  {
	if (ent->client->resp.next_thrust_sound < level.time)
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("hover/hovidle1.wav"), 1, ATTN_NORM, 0);
		ent->client->resp.next_thrust_sound=level.time+1.6;
	}
  }
  else if (ent->client->resp.class == 26)
  {
	if (ent->client->resp.next_thrust_sound < level.time)
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("daedalus/daedidle1.wav"), 1, ATTN_NORM, 0);
		ent->client->resp.next_thrust_sound=level.time+1.6;
	}
  }
  else if (ent->client->resp.class == 11)
  {
	if (ent->client->resp.next_thrust_sound < level.time)
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("bosshovr/BHVENGN1.WAV"), 1, ATTN_NORM, 0);
		ent->client->resp.next_thrust_sound=level.time+1.6;
	}
  } 
  else if (ent->client->resp.class == 29)
  {
	if (ent->client->resp.next_thrust_sound < level.time)
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("bosshovr/BHVENGN1.WAV"), 1, ATTN_NORM, 0);
		ent->client->resp.next_thrust_sound=level.time+1.6;
	}
  } 
  else if (ent->client->resp.class == 16)
  {
	if (ent->client->resp.next_thrust_sound < level.time)
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("floater/Fltsrch1.wav"), 1, ATTN_NORM, 0);
		ent->client->resp.next_thrust_sound=level.time+2.4;
	}
  } 
}
 
/*end of jet.c*/


void ApplySupertankSound ( edict_t *ent )
{
	gi.sound (ent, CHAN_AUTO, gi.soundindex ("bosstank/btkengn1.wav"), 1, ATTN_NORM, 0);
	ent->client->resp.next_thrust_sound=level.time+7.1;
}

/*
=================
ApplyThrust

MUCE:
To add thrusting velocity to player
=================
*/

void ApplyThrust (edict_t *ent)
{

	vec3_t forward, right;
	vec3_t pack_pos, jet_vector;

	//MUCE:  add thrust to character

	if (ent->velocity[2] < -500)
		ent->velocity[2]+=((ent->velocity[2])/(-5));
	else if (ent->velocity[2] < 0)
		ent->velocity[2] += 100; 
	else
		ent->velocity[2]+=((1000-ent->velocity[2])/8);

	//MUCE:  add sparks

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -7, pack_pos);
	VectorAdd (pack_pos, ent->s.origin, pack_pos);
	pack_pos[2] += (ent->viewheight);

	VectorScale (forward, -50, jet_vector);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (pack_pos);
	gi.WriteDir (jet_vector);
	gi.multicast (pack_pos, MULTICAST_PVS);

	//MUCE: add sound 

	if (ent->client->resp.next_thrust_sound < level.time)
	{
		gi.sound (ent, CHAN_BODY, gi.soundindex("flyer/Flyidle1.wav"), 1, ATTN_NORM, 0);
		ent->client->resp.next_thrust_sound=level.time+1.3;
	}
}

static void SP_FixCoopSpots (edict_t *self)
{
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384)
		{
			if ((!self->targetname) || Q_stricmp(self->targetname, spot->targetname) != 0)
			{
//				gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
				self->targetname = spot->targetname;
			}
			return;
		}
	}
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
	edict_t	*spot;

	if(Q_stricmp(level.mapname, "security") == 0)
	{
		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 128;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		return;
	}
}


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
	if (!coop->value)
		return;
	if(Q_stricmp(level.mapname, "security") == 0)
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_CreateCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}

	if((Q_stricmp(level.mapname, "jail2") == 0)   ||
	   (Q_stricmp(level.mapname, "jail4") == 0)   ||
	   (Q_stricmp(level.mapname, "mine1") == 0)   ||
	   (Q_stricmp(level.mapname, "mine2") == 0)   ||
	   (Q_stricmp(level.mapname, "mine3") == 0)   ||
	   (Q_stricmp(level.mapname, "mine4") == 0)   ||
	   (Q_stricmp(level.mapname, "lab") == 0)     ||
	   (Q_stricmp(level.mapname, "boss1") == 0)   ||
	   (Q_stricmp(level.mapname, "fact3") == 0)   ||
	   (Q_stricmp(level.mapname, "biggun") == 0)  ||
	   (Q_stricmp(level.mapname, "space") == 0)   ||
	   (Q_stricmp(level.mapname, "command") == 0) ||
	   (Q_stricmp(level.mapname, "power2") == 0) ||
	   (Q_stricmp(level.mapname, "strike") == 0))
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_FixCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(edict_t *self)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}

qboolean IsNeutral (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
		return true;
	return false;
}

// TYPE OF MONSTER ---- HEALTH
#define M_SOLDIERLT 0 // 20
#define M_SOLDIER 1 // 30
#define M_SOLDIERSS 2 // 40
#define M_FLIPPER 3 // 50
#define M_FLYER 4 // 50
#define M_INFANTRY 5 // 100
#define M_INSANE 6 // 100 - Crazy Marine
#define M_GUNNER 7 // 175
#define M_CHICK 8 // 175
#define M_PARASITE 9 // 175
#define M_FLOATER 10 // 200
#define M_HOVER 11 // 240
#define M_BERSERK 12 // 240
#define M_MEDIC 13 // 300
#define M_MUTANT 14 // 300
#define M_BRAIN 15 // 300
#define M_GLADIATOR 16 // 400
#define M_TANK 17 // 750
#define M_SUPERTANK 18 // 1500
#define M_BOSS2 19 // 2000
#define M_JORG 20 // 3000
#define M_MAKRON 21 // 3000
#define M_STALKER 22
//======================================================
// Frag award and Obits for Victims of Monster attacks!
// Attacker=Monster
// Attacker->activator=Monster's Owner
//======================================================
qboolean Monster_Obits(edict_t *victim, edict_t *attacker) {
char *message1="";
char *message2="";

  // No message if killed by your own Monster!
  // Paril 7.0 - This just confuses people..
  if (victim==attacker->activator)
  {
	  message1 = "was killed by his own";
	 // What type of monster was this?
	 switch (attacker->mtype) {
		case M_BERSERK:
		 message2=" Berserker";
		  break;
		case M_BOSS2:
	    message2=" Boss";
	    break;
		case M_SOLDIERSS:
		  message2=" SoldierSS";
		  break;
		case M_JORG:
		  message2=" Jorg";
		  break;
		case M_BRAIN:
		  message2=" Brain";
		  break;
		case M_CHICK:
		  message2=" Chick";
		  break;
		case M_FLIPPER:
		  message2=" Shark";
		  break;
		case M_FLOATER:
		  message2=" Floater";
		  break;
		case M_FLYER:
		  message2=" Flyer";
		  break;
		case M_INSANE:
		  message2=" Insane";
		  break;
		case M_GLADIATOR:
		  message2=" Gladiator";
		  break;
		case M_HOVER:
		  message2=" Icarus";
		  break;
		case M_INFANTRY:
		  message2=" Infantry";
		  break;
		case M_SOLDIERLT:
		  message2=" SoldierLT";
		  break;
		case M_SOLDIER:
		  message2=" Soldier";
		  break;
		case M_MEDIC:
		  message2=" Medic";
		  break;
		case M_MUTANT:
		  message2=" Mutant";
		  break;
		case M_PARASITE:
		  message2=" Parasite";
		  break;
		case M_TANK:
		  message2=" Tank";
		  break;
		case M_MAKRON:
		  message2=" Makron";
		  break;
		case M_GUNNER:
		  message2=" Gunner";
		  break;
		case M_SUPERTANK:
		  message2=" Supertank";
		  break;
		case M_STALKER:
		  message2=" Stalker";
		  break;
		default:
		  return false; 
	  }

	// Print the obituary message..
	 if (victim->client && attacker->activator->client) 
	 {
		 safe_bprintf(PRINT_MEDIUM,"%s %s %s!\n", victim->client->pers.netname, message1, message2);
		 if (logging->value)
			 fprintf (logged, "%s %s %s!\n", victim->client->pers.netname, message1, message2);
	 }

  return true;
  }
  message1="was killed by";

  // What type of monster was this?
  switch (attacker->mtype) {
    case M_BERSERK:
      message2="'s Berserker";
      break;
    case M_BOSS2:
      message2="'s Boss";
      break;
    case M_SOLDIERSS:
      message2="'s SoldierSS";
      break;
    case M_JORG:
      message2="'s Jorg";
      break;
    case M_BRAIN:
      message2="'s Brain";
      break;
    case M_CHICK:
      message2="'s Chick";
      break;
    case M_FLIPPER:
      message2="'s Shark";
      break;
    case M_FLOATER:
      message2="'s Floater";
      break;
    case M_FLYER:
      message2="'s Flyer";
      break;
    case M_INSANE:
      message2="'s Insane";
      break;
    case M_GLADIATOR:
      message2="'s Gladiator";
      break;
    case M_HOVER:
      message2="'s Icarus";
      break;
    case M_INFANTRY:
      message2="'s Infantry";
      break;
    case M_SOLDIERLT:
      message2="'s SoldierLT";
      break;
    case M_SOLDIER:
      message2="'s Soldier";
      break;
    case M_MEDIC:
      message2="'s Medic";
      break;
    case M_MUTANT:
      message2="'s Mutant";
      break;
    case M_PARASITE:
      message2="'s Parasite";
      break;
    case M_TANK:
      message2="'s Tank";
      break;
    case M_MAKRON:
      message2="'s Makron";
      break;
    case M_GUNNER:
      message2="'s Gunner";
      break;
    case M_SUPERTANK:
      message2="'s Supertank";
      break;
	case M_STALKER:
      message2="'s Stalker";
      break;
    default:
      return false; }

  // Print the obituary message..
  if (victim->client && attacker->activator->client) 
  {
    safe_bprintf(PRINT_MEDIUM,"%s %s %s%s!\n",
                                 victim->client->pers.netname,
                                 message1,
                                 attacker->activator->client->pers.netname,
                                 message2);
    // Give Monster's activator/owner this frag!! - CHANGE THIS TO FIT YOUR MOD
    attacker->activator->client->resp.score++; 
	if (logging->value)
		fprintf (logged, "%s %s %s%s!\n", victim->client->pers.netname, message1, attacker->activator->client->pers.netname, message2);
  }

  return true;
}


// RPG Mode functions
int Find_Kill_Experience (edict_t *attacker, edict_t *self)
{
	int yourexp;
	int enemyexp;
	int yourkills;
	int enemykills;
	int yourstreak;
	int enemystreak;
	int yourlevel;
	int enemylevel;
	int exp;
	int mul;

	yourexp = attacker->client->resp.exp;
	enemyexp = self->client->resp.exp;
	yourkills = attacker->client->pers.score;
	enemykills = self->client->pers.score;
	yourstreak  = attacker->client->pers.streak;
	enemystreak  = self->client->pers.streak;
	yourlevel     = self->client->resp.lvl;
	enemylevel     = self->client->resp.lvl;

	if (!yourexp || !enemyexp)
		mul = 100;
	else if ((yourexp + enemyexp) / 2 < 0)
		mul = (yourexp + enemyexp);
	else
		mul = ((yourexp + enemyexp) / 2);

	exp = mul - ((yourkills + enemykills) - (yourstreak - enemystreak)) - ((yourlevel - enemylevel) * 2);

	if (exp < 0)
		exp = 0;
	safe_cprintf (attacker, PRINT_HIGH, "%i\n", exp);
	return exp;
}

char *LevelUpMonsters (edict_t *ent, int lvl)
{
	unsigned int nolvlmon[] = {1, 3, 4, 19, 21, 23, 24, 25, 27, 29};

	unsigned int yeslvlmon[] = {2, 5, 6, 7, 8, 9, 10, 11, 
								12, 13, 14, 15, 16, 17, 18,
								20, 22, 26, 28, 30};

	unsigned char *yeslvlmonc[] = {"the Enforcer", 
								"the Berserk and the Barracuda Shark",
								"the Gunner and the Technician", 
								"the Brain", 
								"the Medic, Iron Maiden, Icarus and the BETA Soldier",
								"the Gladiator, Mutant, Stalker and Parasite", 
								"the Repair Bot, Male Traitor and Female Traitor", 
								"the Tank, BETA Brain and Daedalus", 
								"the Tank Commander, Gekk and BETA Gladiator", 
								"the BETA Iron Maiden, Laser Gladiator and the Flyer Hornet",
								"the Supertank and the Medic Commander", 
								"the Biggun", 
								"the BETA Supertank", 
								"the Carrier and the Stormtrooper Soldier", 
								"the Death Tank",
								"the JORG", 
								"the Makron", 
								"the Black Widow (walk)", 
								"the Black Widow (spider)", 
								"the Hypertank"};
	char start[] = "You now have access to ";
	char middle[90];
	char *end = "";
	char *congrats = "You have unlocked all of the monsters, congratulations!\n";
	char finish [300] = "";
	int n;
	int i;

	if (sizeof(yeslvlmon) != sizeof(yeslvlmonc))
	{	
		gi.cprintf(ent, PRINT_HIGH, "Error: Arrays don't have same count.\n");
		return "";
	}


	if (lvl > 30)
		 strcpy(middle,"");

	for (n = 0; nolvlmon[n]; n++)
	{
		if (lvl == nolvlmon[n])
		{
			strcpy(middle,"");
		}
	}

	for (i = 0; i < sizeof(yeslvlmon)/sizeof(int); i++)
	{
		if (lvl == yeslvlmon[i])
		{
			strcpy(middle, yeslvlmonc[i]);
		}
	}

	if (Q_stricmp(middle, "") == 0)
	{
		// Nothing.
		return "";
	}
	else
	{
		gi.cprintf (ent, PRINT_HIGH, " %s%s%s.\n", start, middle, end);
		if (lvl == 30)
			gi.cprintf (ent, PRINT_HIGH, "%s", congrats);
	}

	return "";
}


void Check_Levelup (edict_t *ent)
{
	int current_level, needed;
//	char *monsters_you_get;
	
	current_level = ent->client->resp.lvl;
	needed = 250 * (current_level) * (current_level);
	if (ent->client->resp.exp >= needed)
	{
		ent->client->resp.lvl++;
		safe_bprintf (PRINT_HIGH, "-----=====%s is now level %d!=====-----\n", ent->client->pers.netname, ent->client->resp.lvl); // Tell everyone you have leveled, and what level you are.
		gi.sound (ent, CHAN_AUTO, gi.soundindex ("world/xian1.wav"), 1, ATTN_NORM, 0);
		LevelUpMonsters(ent, ent->client->resp.lvl);
		//if (monsters_you_get)
			//gi.cprintf (ent, PRINT_HIGH, "%s", monsters_you_get);
		WRITE_PLAYER_STATS (ent);//[QBS] 
	}
}

int FindClassPower (edict_t *player, int class)
{
	int power;

	if (class == 0)
	{
		gi.dprintf ("Uh oh, somebody died with no class.\n");
		power = 0;
	}
	else if (class == 1)
		power = 3;
	else if (class == 2)
		power = 5;
	else if (class == 3)
		power = 12;
	else if (class == 4)
		power = 6;
	else if (class == 5)
		power = 8;
	else if (class == 6)
		power = 14;
	else if (class == 7)
		power = 21;
	else if (class == 39)
		power = 49;
	else if (class == 8)
		power = 9;
	else if (class == 9)
		power = 7;
	else if (class == 10)
		power = 9;
	else if (class == 11)
		power = 28;
	else if (class == 12)
		power = 11;
	else if (class == 13)
		power = 14;
	else if (class == 14)
		power = 6;
	else if (class == 41)
		power = 6;
	else if (class == 15)
		power = 9;
	else if (class == 16)
		power = 11;
	else if (class == 17)
		power = 8;
	else if (class == 18)
		power = 38;
	else if (class == 19)
		power = 43;
	else if (class == 20)
		power = 7;
	else if (class == 21)
		power = 12;
	else if (class == 22)
		power = 9;
	else if (class == 23)
		power = 19;
	else if (class == 24)
		power = 23;
	else if (class == 25)
		power = 15;
	else if (class == 26)
		power = 12;
	else if (class == 27)
		power = 17;
	else if (class == 28)
		power = 12;
	else if (class == 29)
		power = 34;
	else if (class == 30)
		power = 48;
	else if (class == 31)
		power = 52;
	else if (class == 32)
		power = 13;
	else if (class == 33)
		power = 28;
	else if (class == 34)
		power = 16;
	else if (class == 35)
		power = 17;
	else if (class == 36)
		power = 13;
	else if (class == 37)
		power = 17;
	else if (class == 38)
		power = 17;
	else if (class == 40)
		power = 9;

	return power;
}

qboolean IsBoss (int class)
{
	if (class == 7 || class == 24 || class == 18 || class == 19 || class == 11 || class == 30 || class == 31 || class == 39)
		return true;
	else
		return false;
}

// Returns the gclient_t of the player found by name
gclient_t *FindPlayer (char *name)
{
	edict_t *en;
	int l;

	if (!name || name == NULL)
		return NULL;

	for (l = 0; l < maxclients->value; l++)
	{
		en = g_edicts + 1 + l;

		// Found a match!
		if (Q_stricmp(en->client->pers.netname, name) == 0)
			return en->client;
	}
	return NULL;
}
gclient_t *InParty (edict_t *ent);

void AddExp (edict_t *ent, float amount)
{
	gclient_t *partyowner = NULL;
	gclient_t *tempmember = NULL;
	ent->client->resp.exp += (int)amount;

	// Check if he's in a party
	partyowner = InParty(ent);
	// Man, I regret coding stuff the way I coded stuff.
	// Memory addresses are my friend and I turned my back on them.
	if (partyowner) // We have one! Split this exp amount up for each member of the party
	{
		int i;
		for (i = 0; i < 9; i++)
		{
			if (!partyowner->resp.party.member[i])
				break;

			tempmember = FindPlayer (partyowner->resp.party.member[i]->client->pers.netname);
			if (tempmember)
				tempmember->resp.exp += amount / 2; // Give them / 2 amount. Gotta be kinda even here.
		}
	}

	// Check if we OWN a party.

	if (ent->client->resp.party.has_party)
	{
		int i;
		for (i = 0; i < 9; i++)
		{
			if (!ent->client->resp.party.member[i])
				break;

			tempmember = FindPlayer (ent->client->resp.party.member[i]->client->pers.netname);
			if (tempmember)
				tempmember->resp.exp += amount / 2; // Give them / 2 amount. Gotta be kinda even here.
		}
	}
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;
	int lvl_diff, exp, targ_lvl, attacker_lvl, attacker_class, targ_class, class_diff, attacker_team, i; // NewB exp 

	// Was this a monster that did the killing?
	if (attacker->ismonster) 
	{ 
		if (attacker->activator)
		{
			Monster_Obits(self, attacker); 
			return; 
		}
	}

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;


	if ((int)(stroggflags->value) & SF_RPG_MODE)
	{
		if (attacker == world)
		{
			self->client->resp.exp -= (5 * (self->client->resp.lvl));
			return;
		}
		if (attacker->client)
		{
			if (attacker == self || !attacker->client->resp.lvl) // NewB Checks for suiside of mod from a levelless attacker.
			{
				self->client->resp.exp -= (5 * (self->client->resp.lvl));
		//		self->client->resp.save_exp -= (2 * (self->client->resp.lvl));
			}
			else
			{
				targ_lvl = self->client->resp.lvl;
				attacker_lvl = attacker->client->resp.lvl;
				lvl_diff = targ_lvl - attacker_lvl;
				attacker_class = FindClassPower (attacker, attacker->client->resp.class);
				targ_class = FindClassPower (self, self->client->resp.class);
				class_diff = targ_class - attacker_class;
				attacker_team = attacker->client->resp.ctf_team;

				// calc the player views now that all pushing
				// and damage has been added
				if (IsBoss (self->client->resp.class))
				{
					edict_t	*ent;
	//				int rnd;
					safe_bprintf (PRINT_HIGH, "%s killed a boss!\n", attacker->client->pers.netname);
					for (i=0 ; i<maxclients->value ; i++)
					{
						ent = g_edicts + 1 + i;
						if (!ent->inuse || !ent->client)
							continue;
						if (ctf->value)
						{
							if (attacker_team == 1)
							{
								if (ent->client->resp.ctf_team == 1)
									ent->client->resp.exp += rndnum (45, 60);
								else
									ent->client->resp.exp += rndnum (10, 20);
							}
							else
							{
								if (ent->client->resp.ctf_team == 2)
									ent->client->resp.exp += rndnum (45, 60);
								else
									ent->client->resp.exp += rndnum (10, 20);
							}
						}
						else
							ent->client->resp.exp += rndnum (45, 60);
					}
				}

				if (class_diff < 0)
					class_diff = 0;
				
				if (lvl_diff < -10)
					lvl_diff = -10;
				exp = ((40 + lvl_diff) + (class_diff)) + rand()%10;
				attacker->client->resp.exp += exp;
				Check_Levelup(attacker);
			}
		}
	}


	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			message = "cratered";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		}
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsNeutral(self))
					message = "tripped on its own grenade";
				else if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsNeutral(self))
					message = "blew itself up";
				else if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			default:
				if (IsNeutral(self))
					message = "killed itself";
				else if (IsFemale(self))
					message = "killed herself";
				else
					message = "killed himself";
				break;
			}
		}
		if (message)
		{
			safe_bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
				self->client->resp.score--;
			self->enemy = NULL;
			if (logging->value)
				fprintf (logged, "%s %s.\n", self->client->pers.netname, message);
			self->client->pers.streak = 0;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				break;
			case MOD_SHOTGUN:
				message = "was gunned down by";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				break;
			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				break;
			case MOD_LASER:
				message = "saw the light from";
				message2 = "'s laser";
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG";
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade";
				break;
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
			case MOD_GLADRAIL:
				message = "was owned by";
				message2 = "'s shoulder railgun";
				break;
			case MOD_MEDICHYPER:
				message = "was melted to death by";
				message2 = "'s hyperblaster";
				break;
			case MOD_TANKBLAST:
				message = "took blaster bolts to the head from";
				message2 = "'s blaster";
				break;
			case MOD_SOLDBLAST:
				message = "was humiliated by";
				message2 = "'s blaster";
				break;
			case MOD_GUNNERCHAIN:
				message = "took bullets from";
				message2 = "'s armored chaingun";
				break;
			case MOD_INFMELEE:
				message = "was beaten to death from";
				message2 = "'s chaingun";
				break;
			case MOD_FLYSPIKES:
				message = "was slashed by";
				message2 = "'s blades";
				break;
			case MOD_GLADSPIKE:
				message = "was impaled by";
				message2 = "'s spike";
				break;
			case MOD_BRAINTENTACLE:
				message = "was eaten to death by";
				message2 = "'s live tentacles";
				break;
			case MOD_BRAINCLAWS:
				message = "took";
				message2 = "'s claws to the chest";
				break;
			case MOD_MUTSLASH:
				message = "has scars everywhere, thanks to";
				message2 = "'s claws";
				break;
			case MOD_BERCLUB:
				message = "was clubbed by";
				break;
			case MOD_BERSPIKE:
				message = "was spiked by";
				break;
			case MOD_IRONHAND:
				message = "was slapped and scarred from";
				message2 = "'s sharp hand";
				break;
			case MOD_STANKCHAIN:
				message = "took 12x12 bullets to the body from";
				message2 = "'s huge chaingun";
				break;
			case MOD_INFCHAIN:
				message = "was shot to death by";
				break;
			case MOD_BOSS2CH1:
				message = "took bullets from";
				message2 = "'s first chaingun";
				break;
			case MOD_BOSS2CH2:
				message = "took bullets from";
				message2 = "'s second chaingun";
				break;
			case MOD_TANKMACHINEGUN:
				message = "was pulverized from";
				message2 = "'s machinegun";
				break;
			case MOD_SOLDMACHINEGUN:
				message = "was shot by";
				message2 = "'s machinegun";
				break;
			case MOD_SOLDSHOTGUN:
				message = "was pelted from";
				message2 = "'s shotgun";
				break;
//ZOID
			case MOD_GRAPPLE:
				message = "was caught by";
				message2 = "'s grapple";
				break;
//ZOID
			}
			if (message)
			{
				safe_bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (logging->value)
					fprintf (logged, "%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
				{
					if (ff)
						attacker->client->resp.score--;
					else
					{
						if (attacker->client->pers.abilities.floater_fragmulti)
						{
							attacker->client->pers.streak++;
							attacker->client->resp.score++;
						}
						attacker->client->resp.score++;

						attacker->client->pers.streak++;
						self->client->pers.streak = 0;
						if (!((int)stroggflags->value & SF_NO_STREAKS))
						{
							int streak_bonus;

							if (attacker->client->pers.streak == 4 || attacker->client->pers.streak == 5 || attacker->client->pers.streak == 6 || attacker->client->pers.streak == 7 || attacker->client->pers.streak == 8)
							{
								safe_bprintf (PRINT_MEDIUM, "%s has a %i kill streak, and gets 2 frags for the kill.\n", attacker->client->pers.netname, attacker->client->pers.streak);
								streak_bonus = 1;
							}
							else if (attacker->client->pers.streak == 9 || attacker->client->pers.streak == 10 || attacker->client->pers.streak == 11 || attacker->client->pers.streak == 12 || attacker->client->pers.streak == 13 )
							{
								safe_bprintf (PRINT_MEDIUM, "%s has a %i kill streak, and gets 4 frags for the kill.\n", attacker->client->pers.netname, attacker->client->pers.streak);
								streak_bonus = 3;
							}
							else if (attacker->client->pers.streak == 14 || attacker->client->pers.streak == 15 || attacker->client->pers.streak == 16 || attacker->client->pers.streak == 17 || attacker->client->pers.streak == 18 )
							{
								safe_bprintf (PRINT_MEDIUM, "%s has a %i kill streak, and gets 6 frags for the kill.\n", attacker->client->pers.netname, attacker->client->pers.streak);
								streak_bonus = 5;
							}
							else if (attacker->client->pers.streak == 19 || attacker->client->pers.streak == 20)
							{
								safe_bprintf (PRINT_MEDIUM, "%s has a %i kill streak, and gets 8 frags for the kill.\n", attacker->client->pers.netname, attacker->client->pers.streak);
								streak_bonus = 7;
							}
							else if (attacker->client->pers.streak > 20)
							{
								safe_bprintf (PRINT_MEDIUM, "%s has a %i kill streak, and gets 10 frags for the kill.\n", attacker->client->pers.netname, attacker->client->pers.streak);
								streak_bonus = 9;
							}
							else
								streak_bonus = 0;

							attacker->client->resp.score += streak_bonus;
						}
					}

				}
				return;
			}
		}
	}

	safe_bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (logging->value)
	{
		fprintf (logged, "%s died.\n", self->client->pers.netname);
		fprintf (logged, "NOTE: %s died with an unknown message, please report this to Paril!\n", self->client->pers.netname);
	}
	if (deathmatch->value)
		self->client->resp.score--;
}


void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	float		spread;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Soldier Blaster") == 0))
		item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}

	if (quad)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	if (dir[0])
		self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
	else {
		self->client->killer_yaw = 0;
		if (dir[1] > 0)
			self->client->killer_yaw = 90;
		else if (dir[1] < 0)
			self->client->killer_yaw = -90;
	}
	if (self->client->killer_yaw < 0)
		self->client->killer_yaw += 360;
	

}


//#define rndnum(y,z) ((random()*((z)-((y)+1)))+(y))
/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
	int		r;
//	int temp;
//	cplane_t	plane;		// surface normal at impact

	if (attacker->client)// yes attacker was client
	{
		WRITE_PLAYER_STATS(attacker);//[QBS]
	}
	else//[QBS] must have been the world that killed them
	{
	} 

	WRITE_PLAYER_STATS(self);//[QBS] dead dude !! he's dead Jim !!! hehehe
	//[QBS]1end 


	if (self->client->pers.monster1)
	{
		G_FreeEdict (self->client->pers.monster1);
		self->client->resp.spawned_monsters--;
	}

	if (self->client->pers.monster2)
	{
		G_FreeEdict (self->client->pers.monster2);
		self->client->resp.spawned_monsters--;
	}

	if (self->client->pers.monster3)
	{
		G_FreeEdict (self->client->pers.monster3);
		self->client->resp.spawned_monsters--;
	}

	if (self->client->pers.monster4)
	{
		G_FreeEdict (self->client->pers.monster4);
		self->client->resp.spawned_monsters--;
	}

	if (self->client->pers.monster5)
	{
		G_FreeEdict (self->client->pers.monster5);
		self->client->resp.spawned_monsters--;
	}

	if (self->client->pers.monster6)
	{
		G_FreeEdict (self->client->pers.monster6);
		self->client->resp.spawned_monsters--;
	}

    if (self->client->hook)
        Release_Grapple(self->client->hook);

	self->svflags &= ~SVF_NOCLIENT;

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
//ZOID
	self->s.modelindex3 = 0;	// remove linked ctf flag
//ZOID

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

  /* WonderSlug */

  Kamikaze_Cancel(self);              /* No Kamikaze Now!!*/

  /* end wonderlsug */

 // CCH: Call off the airstrike
 self->client->airstrike_called = 0;

                //Wyrm: chasecam
                ChasecamRemove(self);



//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
//ZOID
		// if at start and same team, clear
		if (ctf->value && meansOfDeath == MOD_TELEFRAG &&
			self->client->resp.ctf_state < 2 &&
			self->client->resp.ctf_team == attacker->client->resp.ctf_team) {
			attacker->client->resp.score--;
			self->client->resp.ctf_state = 0;
		}

		CTFFragBonuses(self, inflictor, attacker);
//ZOID
		//TossClientWeapon (self);
//ZOID
		CTFPlayerResetGrapple(self);
		CTFDeadDropFlag(self);
		CTFDeadDropTech(self);
//ZOID
		if (deathmatch->value && !self->client->showscores)
			Cmd_Help_f (self);		// show scores
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;

	if (self->client->resp.class == 8)
	{
		BecomeExplosion1(self);
		gi.sound (self, CHAN_BODY, gi.soundindex ("flyer/flydeth1.wav"), 1, ATTN_NORM, 0);
		self->deadflag = DEAD_DEAD;
		gi.linkentity (self);
		ThrowClientHead (self);

		self->takedamage = DAMAGE_NO;
		return;
	}
	if (self->client->resp.class == 35)
	{
		BecomeExplosion1(self);
		gi.sound (self, CHAN_BODY, gi.soundindex ("flyer/flydeth1.wav"), 1, ATTN_NORM, 0);
		self->deadflag = DEAD_DEAD;
		gi.linkentity (self);
		ThrowClientHead (self);

		self->takedamage = DAMAGE_NO;
		return;
	}
	if (self->client->resp.class == 32)
	{
		BecomeExplosion1(self);
		gi.sound (self, CHAN_BODY, gi.soundindex ("flyer/flydeth1.wav"), 1, ATTN_NORM, 0);
		self->deadflag = DEAD_DEAD;
		gi.linkentity (self);
		ThrowClientHead (self);

		self->takedamage = DAMAGE_NO;
		return;
	}
	if (self->client->resp.class == 16)
	{
		BecomeExplosion1(self);
		gi.sound (self, CHAN_BODY, gi.soundindex ("floater/fltdeth1.wav"), 1, ATTN_NORM, 0);
		self->deadflag = DEAD_DEAD;
		gi.linkentity (self);
		ThrowClientHead (self);

		self->takedamage = DAMAGE_NO;
		return;
	}
	/*if (self->client->resp.class == 30)
	{
		BecomeExplosion1(self);
		gi.sound (self, CHAN_BODY, gi.soundindex ("widow/death.wav"), 1, ATTN_NORM, 0);
		self->deadflag = DEAD_DEAD;
		gi.linkentity (self);
		ThrowClientHead (self);

		self->takedamage = DAMAGE_NO;
		return;
	}*/
	if (self->health < -40)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2");
		ThrowClientHead (self);

		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i+1)%3;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			/*if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			}
			else */switch (self->client->resp.class)
			{
			case 0:
				self->s.frame = FRAME_death101-1;
				self->client->anim_end = FRAME_death106;
				break;
			case 1:
				self->s.frame = 441;
				self->client->anim_end = 464;
				break;
			case 36:
				self->s.frame = 441;
				self->client->anim_end = 464;
				break;
			case 2:
				self->s.frame = 125;
				self->client->anim_end = 144;
				break;
			case 3:
				self->s.frame = 222;
				self->client->anim_end = 253;
				break;
			case 4:
				self->s.frame = 147;
				self->client->anim_end = 176;
				break;
			case 5:
				self->s.frame = 190;
				self->client->anim_end = 200;
				break;
			case 6:
				self->s.frame = 61;
				self->client->anim_end = 82;
				break;
			case 34:
				self->s.frame = 61;
				self->client->anim_end = 82;
				break;
			case 7:
				self->s.frame = 98;
				self->client->anim_end = 127;
				break;
			case 39:
				self->s.frame = 39;
				self->client->anim_end = 39;
				break;
			case 8:
				break;
			case 9:
				self->s.frame = 236;
				self->client->anim_end = 243;
				break;
			case 10:
				self->s.frame = 48;
				self->client->anim_end = 59;
				break;
			case 11:
				self->s.frame = 132;
				self->client->anim_end = 180;
				break;
			case 12:
				self->s.frame = 123;
				self->client->anim_end = 140;
				break;
			case 13:
				self->s.frame = 15;
				self->client->anim_end = 23;
				break;
			case 14:
				self->s.frame = 222;
				self->client->anim_end = 253;
				break;
			case 41:
				self->s.frame = 222;
				self->client->anim_end = 253;
				break;
			case 15:
				self->s.frame = 162;
				self->client->anim_end = 172;
				break;
			case 16:
				break;
			case 17:
				self->s.frame = 104;
				self->client->anim_end = 159;
				break;
			case 18:
				self->s.frame = 31;
				self->client->anim_end = 80;
				break;
			case 19:
				self->s.frame = 50;
				self->client->anim_end = 144;
				break;
			case 20:
				self->s.frame = 441;
				self->client->anim_end = 464;
				break;
			case 21:
				self->s.frame = 61;
				self->client->anim_end = 82;
				break;
			case 22:
				self->s.frame = 123;
				self->client->anim_end = 140;
				break;
			case 23:
				self->s.frame = 48;
				self->client->anim_end = 59;
				break;
			case 24:
				self->s.frame = 98;
				self->client->anim_end = 127;
				break;
			case 25:
				self->s.frame = 114;
				self->client->anim_end = 123;
				break;
			case 26:
				self->s.frame = 162;
				self->client->anim_end = 172;
				break;
			case 27:
				self->s.frame = 70;
				self->client->anim_end = 88;
				break;
			case 28:
				self->s.frame = 147;
				self->client->anim_end = 176;
				break;
			case 29:
				self->s.frame = 62;
				self->client->anim_end = 77;
				break;
			case 30:
				self->s.frame = 130;
				self->client->anim_end = 160;
				break;
			case 31:
				self->s.frame = 60;
				self->client->anim_end = 125;
				break;
			case 33:
				self->s.frame = 222;
				self->client->anim_end = 253;
				break;
			case 37:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			case 38:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			default:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			}
			if (self->client->resp.class == 1)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("soldier/soldeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 36)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("soldier/soldeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 20)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("soldier/soldeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 30)
			{
				gi.sound (self, CHAN_BODY, gi.soundindex ("widow/death.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 31)
			{
				gi.sound (self, CHAN_BODY, gi.soundindex ("widow/death.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 2)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("infantry/infdeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 3)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("tank/death.wav"), 1, ATTN_NORM, 0);
				if (self->s.frame == 249)
					gi.sound (self, CHAN_AUTO, gi.soundindex("tank/thud.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 33)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("tank/death.wav"), 1, ATTN_NORM, 0);
				if (self->s.frame == 249)
					gi.sound (self, CHAN_AUTO, gi.soundindex("tank/thud.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 4)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("medic/meddeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 28)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("medic_commander/meddeth.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 5)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("gunner/death1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 6)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("gladiator/glddeth2.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 34)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("gladiator/glddeth2.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 21)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("gladiator/glddeth2.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 25)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("gek/gk_deth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 27)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("stalker/death.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 7)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("bosstank/btkdeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 39)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("bosstank/btkdeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 24)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("bosstank/btkdeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 9)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("berserk/Berdeth2.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 10)
			{
				r = rndnum (1, 3);
				gi.sound (self, CHAN_VOICE, gi.soundindex(va("chick/Chkdeth%d.wav", r)), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 23)
			{
				r = rndnum (1, 3);
				gi.sound (self, CHAN_VOICE, gi.soundindex(va("chick/Chkdeth%d.wav", r)), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 11)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("bosshovr/BHVDETH1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 29)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("carrier/death.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 12)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("brain/brndeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 22)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("brain/brndeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 13)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("mutant/mutdeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 18)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("boss3/BS3DETH1.WAV"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 17)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("flipper/Flpdeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 19)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("makron/death.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 15)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("hover/hovdeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 26)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("daedalus/daeddeth2.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 14)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("tank/death.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 41)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("tank/death.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 40)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex("parasite/Pardeth1.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 37)
			{
				float rnddeath = rndnum (0, 3);

				if (rnddeath == 0)
					gi.sound (self, CHAN_VOICE, gi.soundindex("../players/male/death1.wav"), 1, ATTN_NORM, 0);
				else if (rnddeath == 1)
					gi.sound (self, CHAN_VOICE, gi.soundindex("../players/male/death2.wav"), 1, ATTN_NORM, 0);
				else if (rnddeath == 2)
					gi.sound (self, CHAN_VOICE, gi.soundindex("../players/male/death3.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (self, CHAN_VOICE, gi.soundindex("../players/male/death4.wav"), 1, ATTN_NORM, 0);
			}
			else if (self->client->resp.class == 38)
			{
				float rnddeath = rndnum (0, 3);

				if (rnddeath == 0)
					gi.sound (self, CHAN_VOICE, gi.soundindex("../players/female/death1.wav"), 1, ATTN_NORM, 0);
				else if (rnddeath == 1)
					gi.sound (self, CHAN_VOICE, gi.soundindex("../players/female/death2.wav"), 1, ATTN_NORM, 0);
				else if (rnddeath == 2)
					gi.sound (self, CHAN_VOICE, gi.soundindex("../players/female/death3.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (self, CHAN_VOICE, gi.soundindex("../players/female/death4.wav"), 1, ATTN_NORM, 0);
			}

		}
	}


	self->deadflag = DEAD_DEAD;

	gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
   if (client->resp.class == 1) 
    {
        //Class 1
        gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= soldier_health->value;
	client->pers.max_health		= soldier_health->value;
        item = FindItem("Shells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 25;

        item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 150;

        item = FindItem("Soldier Machinegun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        item = FindItem("Soldier Shotgun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        item = FindItem("Soldier Blaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 36) 
    {
        //Class 1
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= stormt_health->value;
	client->pers.max_health		= stormt_health->value;


		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 560;
        item = FindItem("Power Screen");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 800;

        item = FindItem("Soldier Machinegun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 2)
    {
        //Class 2
        gitem_t         *item;


        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


	client->pers.health			= enforcer_health->value;
	client->pers.max_health		= enforcer_health->value;
        item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 220;
        item = FindItem("Infantry Melee");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Infantry Chaingun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 3)
    {
        //Class 3
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= tank_health->value;
	client->pers.max_health		= tank_health->value;

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 50;
		item = FindItem("Rockets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 20;
		item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;
        item = FindItem("Tank Shockwave");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Tank Rocket Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Tank Machinegun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Tank Blaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 33)
    {
        //Class 3
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= deathtank_health->value;
	client->pers.max_health		= deathtank_health->value;

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 560;
        item = FindItem("Power Screen");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Rockets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 20;
        item = FindItem("Death Tank Plasma Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Death Tank Laser Gun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
	 else if (client->resp.class == 4)
    {
        //Class 4
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= medic_health->value;
	client->pers.max_health		= medic_health->value;

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 500;
        item = FindItem("Medic Healer");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Medic Hyperblaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 28)
    {
        //Class 4
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= mediccommander_health->value;
	client->pers.max_health		= mediccommander_health->value; // Medic Com

		item = FindItem("Slugs");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 300;
		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 650;
        item = FindItem("Medic Commander Healer");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Medic Commander Spawner");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Medic Commander Hyperblaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 5)
    {
        //Class 5
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= gunner_health->value;
	client->pers.max_health		= gunner_health->value; //Gun

		item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 300;
		item = FindItem("Grenades");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 40;
        item = FindItem("Gunner Grenade Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Gunner Chaingun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 6)
    {
        //Class 6
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= gladiator_health->value;
	client->pers.max_health		= gladiator_health->value; //Glad

		item = FindItem("Slugs");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 100;
        item = FindItem("Gladiator Spike");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Gladiator Railgun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 34)
    {
        //Class 6
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= lasergladiator_health->value;
	client->pers.max_health		= lasergladiator_health->value; /// FIXME 7.0

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 560;
        item = FindItem("Power Screen");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

		item = FindItem("Slugs");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 100;
        item = FindItem("Gladiator Spike");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Gladiator Railgun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 21)
    {
        //Class 6
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= betagladiator_health->value;
	client->pers.max_health		= betagladiator_health->value; //Glad2

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 150;
        item = FindItem("Gladiator Spike");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Gladiator Plasma");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 7)
    {
        //Class 7
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


	client->pers.health			= supertank_health->value;
	client->pers.max_health		= supertank_health->value; //Supert

		item = FindItem("Rockets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 35;
		item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 400;
        item = FindItem("SuperTank Rocket Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("SuperTank Chaingun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
     else if (client->resp.class == 39)
    {
        //Class 7
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


	client->pers.health			= hypertank_health->value;
	client->pers.max_health		= hypertank_health->value; //Supert

		item = FindItem("Rockets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 35;
		item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 400;
 		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 600;
		item = FindItem("Power Shield");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 400;
		item = FindItem("Hypertank Rocket Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Hypertank Chaingun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
	 else if (client->resp.class == 24)
    {
        //Class 7
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


	client->pers.health			= betasupertank_health->value;
	client->pers.max_health		= betasupertank_health->value; //Bsupert

		item = FindItem("Power Shield");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 350;
		item = FindItem("Rockets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 35;
		item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 400;
        item = FindItem("SuperTank Rocket Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("SuperTank Chaingun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 8)
    {
        //Class 8
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


	client->pers.health			= flyer_health->value;
	client->pers.max_health		= flyer_health->value; // Flyer

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;
		item = FindItem("Flyer Kamikaze");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Flyer Melee");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Flyer Hyperblaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 35)
    {
        //Class 8
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


	client->pers.health			= flyerhornet_health->value;
	client->pers.max_health		= flyerhornet_health->value;


		item = FindItem("Power Shield");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 350;
		item = FindItem("Slugs");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;
		item = FindItem("Flyer Melee");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Flyer Hornet Railgun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 32)
    {
        //Class 8
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


	client->pers.health			= fixbot_health->value;
	client->pers.max_health		= fixbot_health->value;//fix

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 500;
        item = FindItem("Fixbot Blaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Fixbot Welder");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 16)
    {
        //Class 16
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


	client->pers.health			= floater_health->value;
	client->pers.max_health		= floater_health->value;//float

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 350;
        item = FindItem("Floater Clamp");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1; 
		item = FindItem("Floater Melee");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Floater HyperBlaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 17)
    {
        //Class 17
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


	client->pers.health			= shark_health->value;
	client->pers.max_health		= shark_health->value;//shrk

		item = FindItem("Shark Tail");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Shark Bite");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 9)
    {
        //Class 9
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= berserk_health->value;
	client->pers.max_health		= berserk_health->value;//bers

		item = FindItem("Berserker Spike");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Berserker Club");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 10)
    {
        //Class 10
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= ironmaiden_health->value;
	client->pers.max_health		= ironmaiden_health->value;//ironm

		item = FindItem("Rockets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 150;
        item = FindItem("Iron Maiden Slash");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Iron Maiden Rocket Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 23)
    {
        //Class 10
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= betaironmaiden_health->value;
	client->pers.max_health		= betaironmaiden_health->value;//biron

		item = FindItem("Rockets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 100;
        item = FindItem("Iron Maiden Slash");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Beta Iron Maiden Rocket Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 11)
    {
        //Class 11
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= boss2_health->value;
	client->pers.max_health		= boss2_health->value;//boss2

		item = FindItem("Rockets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 75;
		item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 400;
        item = FindItem("Boss2 Rocket Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Boss2 Chainguns");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 29)
    {
        //Class 11
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= carrier_health->value;
	client->pers.max_health		= carrier_health->value;//carr

		item = FindItem("Slugs");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;
		item = FindItem("Grenades");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;
		item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 400;
		item = FindItem("Carrier Spawner");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Carrier Railgun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Carrier Grenade Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
	    item = FindItem("Carrier Chainguns");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 12)
    {
        //Class 12
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= 550;
	client->pers.max_health		= 550;//brain

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;
        item = FindItem("Power Screen");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Brain Claws");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Brain Tentacles");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 22)
    {
        //Class 12
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= betabrain_health->value;
	client->pers.max_health		= betabrain_health->value;//bbrain

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 400;
        item = FindItem("Power Screen");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Brain Eye Lasers");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Brain Claws");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("BETA Brain Tentacles");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 13)
    {
        //Class 13
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= mutant_health->value;
	client->pers.max_health		= mutant_health->value;//mutant

		item = FindItem("Mutant Claws");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 14)
    {
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= ctank_health->value;
	client->pers.max_health		= ctank_health->value;

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 50;
		item = FindItem("Rockets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 20;
		item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;
        //item = FindItem("Tank Shockwave");
        //client->pers.selected_item = ITEM_INDEX(item);
        //client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Tank Rocket Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Tank Machinegun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Tank Blaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
/*        //Class 14
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= parasite_health->value;
	client->pers.max_health		= parasite_health->value;//para

		item = FindItem("Parasite Drain Attack");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Parasite Suck Attack");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon=item;*/
    }
    else if (client->resp.class == 41)
    {
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= ctank_health->value;
	client->pers.max_health		= ctank_health->value;

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 50;
		item = FindItem("Rockets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 20;
		item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;
        //item = FindItem("Tank Shockwave");
        //client->pers.selected_item = ITEM_INDEX(item);
        //client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Tank Rocket Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Tank Machinegun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Tank Blaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; 
		client->pers.lastweapon = item;
    }
    else if (client->resp.class == 18)
    {
        //Class 18
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= jorg_health->value;
	client->pers.max_health		= jorg_health->value;//jorg

		item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 700;
        item = FindItem("Jorg Chainguns");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 19)
    {
        //Class 19
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= makron_health->value;
	client->pers.max_health		= makron_health->value;//bfg


        item = FindItem("Makron Railgun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Makron BFG");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Slugs");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 230;
		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 450;
        item = FindItem("Makron Blaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 30)
    {
        //Class 19
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= widow1_health->value;
	client->pers.max_health		= widow1_health->value;//wid


        item = FindItem("Widow Kick");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Widow Railgun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Widow Spawner");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Slugs");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 230;
		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 450;
        item = FindItem("Widow Blaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 31)
    {
        //Class 31
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= widow2_health->value;
	client->pers.max_health		= widow2_health->value;//wid2


        item = FindItem("Black Widow Tongs");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Black Widow Disruptor");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Widow Spawner");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Slugs");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;
		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 500;
        item = FindItem("Black Widow Heat Beam");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 15)
    {
        //Class 15
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= icarus_health->value;
	client->pers.max_health		= icarus_health->value;//icar

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 540;
        item = FindItem("Icarus HyperBlaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 26)
    {
        //Class 15
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= daedalus_health->value;
	client->pers.max_health		= daedalus_health->value;//daedalus

		item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 300;
        item = FindItem("Daedalus HyperBlaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 20) 
    {
        //Class 1
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= betasoldier_health->value;
	client->pers.max_health		= betasoldier_health->value;//bsoldier

        item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 700;
        item = FindItem("Soldier LaserGun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Soldier HyperGun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Soldier RipperGun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 25) 
    {
        //Class 1
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= gekk_health->value;
	client->pers.max_health		= gekk_health->value;//gek

        item = FindItem("Gekk Spit");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Gekk Claws");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 27) 
    {
        //Class 1
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= stalker_health->value;
	client->pers.max_health		= stalker_health->value;//stalk

        item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;
        item = FindItem("Stalker Blaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        item = FindItem("Stalker Melee");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 37) 
    {
        //Class 1
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= traitorm_health->value;
	client->pers.max_health		= traitorm_health->value;


		item = FindItem("Body Armor");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 400;
        item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 800;
        item = FindItem("Slugs");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;
        item = FindItem("Shells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 450;

  		item = FindItem("Traitor Chaingun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
   		item = FindItem("Traitor Railgun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
   		item = FindItem("Traitor Super Shotgun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 38) 
    {
        //Class 1
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= traitorf_health->value;
	client->pers.max_health		= traitorf_health->value;


		item = FindItem("Body Armor");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 400;
        item = FindItem("Bullets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 800;
        item = FindItem("Cells");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 500;
        item = FindItem("Rockets");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 200;

  		item = FindItem("Traitor Chaingun");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
   		item = FindItem("Traitor HyperBlaster");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
      	item = FindItem("Traitor Rocket Launcher");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }
    else if (client->resp.class == 40)
    {
        //Class 40
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= parasite_health->value;
	client->pers.max_health		= parasite_health->value;//para

		item = FindItem("Parasite Drain Attack");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
		item = FindItem("Parasite Suck Attack");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }

    else 
    {
        //Observer mode, doesn't really matter what they have
        gitem_t         *item;

        if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));
    
	client->pers.health			= 1;
	client->pers.max_health		= 1;

        item = FindItem("Combat Armor");
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.weapon = item; client->pers.lastweapon = item;
    }

	client->pers.max_bullets	= 1000;
	client->pers.max_shells		= 1000;
	client->pers.max_rockets	= 1000;
	client->pers.max_grenades	= 1000;
	client->pers.max_cells		= 1000;
	client->pers.max_slugs		= 1000;

	if (deathmatch->value)
		client->pers.connected = true;
}


void InitClientResp (gclient_t *client)
{
//ZOID
	int ctf_team = client->resp.ctf_team;
	qboolean id_state = client->resp.id_state;
//ZOID

	memset (&client->resp, 0, sizeof(client->resp));
	
//ZOID
	client->resp.ctf_team = ctf_team;
	client->resp.id_state = id_state;
//ZOID

	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
 
//ZOID
	if (ctf->value && client->resp.ctf_team < CTF_TEAM1)
		CTFAssignTeam(client);
//ZOID
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE|FL_NOTARGET|FL_POWER_ARMOR));
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->flags |= ent->client->pers.savedFlags;
	if (coop->value)
		ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint (void)
{
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint ();
	else
		return SelectRandomDeathmatchSpawnPoint ();
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

	if (deathmatch->value)
//ZOID
		if (ctf->value)
			spot = SelectCTFSpawnPoint(ent);
		else
//ZOID
			spot = SelectDeathmatchSpawnPoint ();
	else if (coop->value)
		spot = SelectCoopSpawnPoint (ent);

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
				gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}

//======================================================================


void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void body_free(edict_t *body) { // MIGHT NOT EXIST

  if (!(body->svflags & SVF_NOCLIENT)) // if player back in game
    G_MuzzleFlash(body-g_edicts,body->s.origin,11); // do sparkle effect when body is removed from game

  G_FreeEdict(body);
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < -40)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2");
		self->s.origin[2] -= 48;
		ThrowClientHead (self);
		self->takedamage = DAMAGE_NO;
	}
}

void body_think(edict_t *body) { // MIGHT NOT EXIST

  if ((body->wait<=level.time) || (body->owner && !(body->owner->svflags & SVF_NOCLIENT))) {
    body_free(body); // free body at moment player respawn back into game
    return; }

  body->nextthink=level.time + 0.1;
}

void body_touch(edict_t *body,edict_t *other,cplane_t *plane,csurface_t *surf) {
  if (other==world) return;
  G_FreeEdict(body); // disappear if touched by other
}


void CopyToBodyQue(edict_t *ent) {
edict_t *body;

  gi.unlinkentity(ent); // must unlink player's edict_t struct first! - don't unlink to see what happens.. pretty interesting...

  body=G_Spawn();
  body->owner=ent;
  body->s=ent->s;
  body->takedamage=DAMAGE_NO;
  body->s.number=body-g_edicts;
  body->svflags|=ent->svflags;
  VectorCopy(ent->mins,body->mins);
  VectorCopy(ent->maxs,body->maxs);
  VectorCopy(ent->absmin,body->absmin);
  VectorCopy(ent->absmax,body->absmax);
  VectorCopy(ent->size,body->size);
  body->solid=ent->solid;
  body->clipmask=ent->clipmask;
  body->movetype=ent->movetype;

  body->wait=level.time + 10.0; // auto-free in 10 secs if player doesn't rejoin game first

  body->touch=body_touch; // free body

  body->think=body_think;
  body->nextthink=level.time + 0.1;

  gi.linkentity(body);
}


void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
		// Paril
		if (Q_stricmp(self->classname, "bot") == 0)
		{
			Bot_Respawn(self); // If bot, respawn him seperately
			return;
		} 
		// Paril
		// spectator's don't leave bodies
		if (self->movetype != MOVETYPE_NOCLIP)
			CopyToBodyQue (self);
		self->svflags &= ~SVF_NOCLIENT;
		PutClientInServer (self);

		// add a teleportation effect
		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		self->client->respawn_time = level.time;

		return;
	}

	// restart the entire server
	gi.AddCommandString ("menu_loadgame\n");
}

/* 
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
void spectator_respawn (edict_t *ent)
{
	int i, numspec;

	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators

	if (ent->client->pers.spectator) {
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			safe_cprintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");
			ent->client->pers.spectator = false;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
		}

		// count spectators
		for (i = 1, numspec = 0; i <= maxclients->value; i++)
			if (g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			safe_cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
			ent->client->pers.spectator = false;
			// reset his spectator var
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
		}
	} else {
		// he was a spectator and wants to join the game
		// he must have the right password
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value)) {
			safe_cprintf(ent, PRINT_HIGH, "Password incorrect.\n");
			ent->client->pers.spectator = true;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 1\n");
			gi.unicast(ent, true);
			return;
		}
	}

	// clear client on respawn
	ent->client->resp.score = ent->client->pers.score = 0;

	ent->svflags &= ~SVF_NOCLIENT;
	PutClientInServer (ent);

	// add a teleportation effect
	if (!ent->client->pers.spectator)  {
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	}

	ent->client->respawn_time = level.time;

	if (ent->client->pers.spectator) 
	{
		if (logging->value)
			fprintf (logged, "%s has moved to the sidelines\n", ent->client->pers.netname);
		safe_bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
	}
	else
	{
		safe_bprintf (PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
		if (logging->value)
			fprintf (logged, "%s joined back into the game from spectator mode\n", ent->client->pers.netname);
	}
}

//==============================================================

void stuffcmd(edict_t *ent, char *s, int number) 
{	
	if (ent->is_bot)
	{
		//gi.cvar_forceset (s, number);
		return;
	}
	gi.WriteByte (11); 
	gi.WriteString (s);
	gi.unicast (ent, true); 
} 

int AssignClassSpeed(edict_t *ent)
{
	switch( ent->client->resp.class )
	{
		case 0: return 400;
		break;
		case 1: return 400;
		break;
		case 36: return 400;
		break;
		case 37: return 400;
		break;
		case 38: return 400;
		break;
		case 2: return 400;
		break;
		case 3: return 200;
		break;
		case 33: return 200;
		break;
		case 4: return 400;
		break;
		case 28: return 400;
		break;
		case 5: return 400;
		break;
		case 6: return 300;
		break;
		case 34: return 300;
		break;
		case 7: return 250;
		break;
		case 39: return 250;
		break;
		case 8: return 400;
		break;
		case 35: return 400;
		break;
		case 32: return 200;
		break;
		case 16: return 200;
		break;
		case 9: return 800;
		break;
		case 10: return 250;
		break;
		case 11: return 400;
		break;
		case 29: return 400;
		break;
		case 12: return 150;
		break;
		case 13: return 400;
		break;
		case 14: return 200;
		break;
		case 41: return 200;
		break;
		case 40: return 400;
		break;
		case 17: return 250;
		break;
		case 18: return 350;
		break;
		case 19: return 320;
		break;
		case 20: return 400;
		break;
		case 21: return 300;
		break;
		case 22: return 150;
		break;
		case 23: return 250;
		break;
		case 24: return 200;
		break;
		case 25: return 400;
		break;
		case 15: return 400;
		break;
		case 26: return 400;
		break;
		case 27: return 320;
		break;
		case 30: return 350;
		break;
		case 31: return 320;
		break;
	}
	return ent->client->resp.class;
} 

void StuffSpeedByClass( edict_t *ent )
{
	switch ( ent->client->resp.class )
	{
		case 0: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 1: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 36: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 37: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 38: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 2: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 3: stuffcmd( ent, "cl_forwardspeed 100\n" , 100);
		stuffcmd( ent, "cl_sidespeed 100\n", 100 );
		break;
		case 33: stuffcmd( ent, "cl_forwardspeed 100\n" , 100);
		stuffcmd( ent, "cl_sidespeed 100\n" , 100);
		break;
		case 4: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 28: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 5: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 6: stuffcmd( ent, "cl_forwardspeed 150\n" , 150);
		stuffcmd( ent, "cl_sidespeed 150\n" , 150);
		break;
		case 34: stuffcmd( ent, "cl_forwardspeed 150\n" , 150);
		stuffcmd( ent, "cl_sidespeed 150\n" , 150);
		break;
		case 7: stuffcmd( ent, "cl_forwardspeed 125\n" , 125);
		stuffcmd( ent, "cl_sidespeed 125\n" , 125);
		break;
		case 39: stuffcmd( ent, "cl_forwardspeed 125\n" , 125);
		stuffcmd( ent, "cl_sidespeed 125\n" , 125);
		break;
		case 8: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 35: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 32: stuffcmd( ent, "cl_forwardspeed 100\n" , 100);
		stuffcmd( ent, "cl_sidespeed 100\n" , 100);
		break;
		case 16: stuffcmd( ent, "cl_forwardspeed 100\n" , 100);
		stuffcmd( ent, "cl_sidespeed 100\n" , 100);
		break;
		case 9: stuffcmd( ent, "cl_forwardspeed 400\n" , 400);
		stuffcmd( ent, "cl_sidespeed 400\n" , 400);
		break;
		case 10: stuffcmd( ent, "cl_forwardspeed 125\n" , 125);
		stuffcmd( ent, "cl_sidespeed 125\n" , 125);
		break;
		case 17: stuffcmd( ent, "cl_forwardspeed 125\n" , 125);
		stuffcmd( ent, "cl_sidespeed 125\n", 125 );
		break;
		case 11: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 29: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 12: stuffcmd( ent, "cl_forwardspeed 75\n" , 75);
		stuffcmd( ent, "cl_sidespeed 75\n" , 75);
		break;
		case 13: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 14: stuffcmd( ent, "cl_forwardspeed 100\n" , 100);
		stuffcmd( ent, "cl_sidespeed 100\n", 200 );
		break;
		case 41: stuffcmd( ent, "cl_forwardspeed 100\n" , 100);
		stuffcmd( ent, "cl_sidespeed 100\n", 200 );
		break;
		case 40: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n", 200 );
		break;
		case 18: stuffcmd( ent, "cl_forwardspeed 175\n" , 175);
		stuffcmd( ent, "cl_sidespeed 175\n" , 175);
		break;
		case 19: stuffcmd( ent, "cl_forwardspeed 160\n" , 160);
		stuffcmd( ent, "cl_sidespeed 160\n", 160 );
		break;
		case 20: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n" , 200);
		break;
		case 21: stuffcmd( ent, "cl_forwardspeed 150\n" , 150);
		stuffcmd( ent, "cl_sidespeed 150\n" , 150);
		break;
		case 22: stuffcmd( ent, "cl_forwardspeed 75\n" , 75);
		stuffcmd( ent, "cl_sidespeed 75\n" , 75);
		break;
		case 23: stuffcmd( ent, "cl_forwardspeed 125\n" , 125);
		stuffcmd( ent, "cl_sidespeed 125\n" , 125);
		break;
		case 24: stuffcmd( ent, "cl_forwardspeed 100\n" , 100);
		stuffcmd( ent, "cl_sidespeed 100\n" , 100);
		break;
		case 15: stuffcmd( ent, "cl_forwardspeed 200\n", 200 );
		stuffcmd( ent, "cl_sidespeed 200\n", 200 );
		break;
		case 25: stuffcmd( ent, "cl_forwardspeed 200\n" , 200);
		stuffcmd( ent, "cl_sidespeed 200\n", 200 );
		break;
		case 26: stuffcmd( ent, "cl_forwardspeed 150\n" , 150);
		stuffcmd( ent, "cl_sidespeed 150\n", 150 );
		break;
		case 27: stuffcmd( ent, "cl_forwardspeed 160\n" , 160);
		stuffcmd( ent, "cl_sidespeed 160\n" , 160);
		break;
		case 30: stuffcmd( ent, "cl_forwardspeed 175\n" , 175);
		stuffcmd( ent, "cl_sidespeed 175\n", 175 );
		break;
		case 31: stuffcmd( ent, "cl_forwardspeed 160\n" , 160);
		stuffcmd( ent, "cl_sidespeed 160\n" , 160);
		break;
	}
} 

void InitCoopPersistant (edict_t *ent)
{
	gclient_t *client = ent->client;
		// Reset inventory every time.
		// This prevents weapons from other classes merging with your current class.
		memset (ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));
		if (ent->client->resp.class == 1) 
		{
			//Class 1
			gitem_t         *item;

				if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= soldier_health->value;
		ent->client->pers.max_health		= soldier_health->value;

			item = FindItem("Shells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 25;

			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 150;

			item = FindItem("Soldier Machinegun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			item = FindItem("Soldier Shotgun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			item = FindItem("Soldier Blaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 36) 
		{
			//Class 1
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= stormt_health->value;
		ent->client->pers.max_health		= stormt_health->value;


			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 560;
			item = FindItem("Power Screen");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 800;

			item = FindItem("Soldier Machinegun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 2)
		{
			//Class 2
			gitem_t         *item;


			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


		ent->client->pers.health			= enforcer_health->value;
		ent->client->pers.max_health		= enforcer_health->value;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 220;
			item = FindItem("Infantry Melee");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Infantry Chaingun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 3)
		{
			//Class 3
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= tank_health->value;
		ent->client->pers.max_health		= tank_health->value;

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 50;
			item = FindItem("Rockets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 20;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;
			item = FindItem("Tank Shockwave");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Tank Rocket Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Tank Machinegun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Tank Blaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 33)
		{
			//Class 3
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= deathtank_health->value;
		ent->client->pers.max_health		= deathtank_health->value;

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 560;
			item = FindItem("Power Screen");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Rockets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 20;
			item = FindItem("Death Tank Plasma Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Death Tank Laser Gun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		 else if (ent->client->resp.class == 4)
		{
			//Class 4
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= medic_health->value;
		ent->client->pers.max_health		= medic_health->value;

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 500;
			item = FindItem("Medic Healer");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Medic Hyperblaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 28)
		{
			//Class 4
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= mediccommander_health->value;
		ent->client->pers.max_health		= mediccommander_health->value; // Medic Com

			item = FindItem("Slugs");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 300;
			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 650;
			item = FindItem("Medic Commander Healer");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Medic Commander Spawner");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Medic Commander Hyperblaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 5)
		{
			//Class 5
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= gunner_health->value;
		ent->client->pers.max_health		= gunner_health->value; //Gun

			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 300;
			item = FindItem("Grenades");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 40;
			item = FindItem("Gunner Grenade Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Gunner Chaingun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 6)
		{
			//Class 6
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= gladiator_health->value;
		ent->client->pers.max_health		= gladiator_health->value; //Glad

			item = FindItem("Slugs");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 100;
			item = FindItem("Gladiator Spike");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Gladiator Railgun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 34)
		{
			//Class 6
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= lasergladiator_health->value;
		ent->client->pers.max_health		= lasergladiator_health->value; /// FIXME 7.0

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 560;
			item = FindItem("Power Screen");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			item = FindItem("Slugs");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 100;
			item = FindItem("Gladiator Spike");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Gladiator Railgun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 21)
		{
			//Class 6
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= betagladiator_health->value;
		ent->client->pers.max_health		= betagladiator_health->value; //Glad2

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 150;
			item = FindItem("Gladiator Spike");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Gladiator Plasma");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 7)
		{
			//Class 7
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


		ent->client->pers.health			= supertank_health->value;
		ent->client->pers.max_health		= supertank_health->value; //Supert

			item = FindItem("Rockets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 35;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 400;
			item = FindItem("SuperTank Rocket Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("SuperTank Chaingun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		 else if (ent->client->resp.class == 39)
		{
			//Class 7
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


		ent->client->pers.health			= hypertank_health->value;
		ent->client->pers.max_health		= hypertank_health->value; //Supert

			item = FindItem("Rockets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 35;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 400;
 			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 600;
			item = FindItem("Power Shield");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 400;
			item = FindItem("Hypertank Rocket Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Hypertank Chaingun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		 else if (ent->client->resp.class == 24)
		{
			//Class 7
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


		ent->client->pers.health			= betasupertank_health->value;
		ent->client->pers.max_health		= betasupertank_health->value; //Bsupert

			item = FindItem("Power Shield");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 350;
			item = FindItem("Rockets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 35;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 400;
			item = FindItem("SuperTank Rocket Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("SuperTank Chaingun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 8)
		{
			//Class 8
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


		ent->client->pers.health			= flyer_health->value;
		ent->client->pers.max_health		= flyer_health->value; // Flyer

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;
			item = FindItem("Flyer Kamikaze");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Flyer Melee");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Flyer Hyperblaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 35)
		{
			//Class 8
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


		ent->client->pers.health			= flyerhornet_health->value;
		ent->client->pers.max_health		= flyerhornet_health->value;


			item = FindItem("Power Shield");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 350;
			item = FindItem("Slugs");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;
			item = FindItem("Flyer Melee");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Flyer Hornet Railgun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 32)
		{
			//Class 8
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


		ent->client->pers.health			= fixbot_health->value;
		ent->client->pers.max_health		= fixbot_health->value;//fix

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 500;
			item = FindItem("Fixbot Blaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Fixbot Welder");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 16)
		{
			//Class 16
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


		ent->client->pers.health			= floater_health->value;
		ent->client->pers.max_health		= floater_health->value;//float

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 350;
			item = FindItem("Floater Clamp");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1; 
			item = FindItem("Floater Melee");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Floater HyperBlaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 17)
		{
			//Class 17
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));


		ent->client->pers.health			= shark_health->value;
		ent->client->pers.max_health		= shark_health->value;//shrk

			item = FindItem("Shark Tail");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Shark Bite");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 9)
		{
			//Class 9
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= berserk_health->value;
		ent->client->pers.max_health		= berserk_health->value;//bers

			item = FindItem("Berserker Spike");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Berserker Club");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 10)
		{
			//Class 10
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= ironmaiden_health->value;
		ent->client->pers.max_health		= ironmaiden_health->value;//ironm

			item = FindItem("Rockets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 150;
			item = FindItem("Iron Maiden Slash");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Iron Maiden Rocket Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 23)
		{
			//Class 10
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= betaironmaiden_health->value;
		ent->client->pers.max_health		= betaironmaiden_health->value;//biron

			item = FindItem("Rockets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 100;
			item = FindItem("Iron Maiden Slash");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Beta Iron Maiden Rocket Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 11)
		{
			//Class 11
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= boss2_health->value;
		ent->client->pers.max_health		= boss2_health->value;//boss2

			item = FindItem("Rockets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 75;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 400;
			item = FindItem("Boss2 Rocket Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Boss2 Chainguns");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 29)
		{
			//Class 11
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= carrier_health->value;
		ent->client->pers.max_health		= carrier_health->value;//carr

			item = FindItem("Slugs");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;
			item = FindItem("Grenades");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 400;
			item = FindItem("Carrier Spawner");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Carrier Railgun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Carrier Grenade Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Carrier Chainguns");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 12)
		{
			//Class 12
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= 550;
		ent->client->pers.max_health		= 550;//brain

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;
			item = FindItem("Power Screen");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Brain Claws");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Brain Tentacles");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 22)
		{
			//Class 12
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= betabrain_health->value;
		ent->client->pers.max_health		= betabrain_health->value;//bbrain

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 400;
			item = FindItem("Power Screen");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Brain Eye Lasers");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Brain Claws");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("BETA Brain Tentacles");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 13)
		{
			//Class 13
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= mutant_health->value;
		ent->client->pers.max_health		= mutant_health->value;//mutant

			item = FindItem("Mutant Claws");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 14)
		{
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= ctank_health->value;
		ent->client->pers.max_health		= ctank_health->value;

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 50;
			item = FindItem("Rockets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 20;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;
			//item = FindItem("Tank Shockwave");
			//ent->client->pers.selected_item = ITEM_INDEX(item);
			//ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Tank Rocket Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Tank Machinegun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Tank Blaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
	/*        //Class 14
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= parasite_health->value;
		ent->client->pers.max_health		= parasite_health->value;//para

			item = FindItem("Parasite Drain Attack");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Parasite Suck Attack");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon=item;*/
		}
		else if (ent->client->resp.class == 41)
		{
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= ctank_health->value;
		ent->client->pers.max_health		= ctank_health->value;

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 50;
			item = FindItem("Rockets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 20;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;
			//item = FindItem("Tank Shockwave");
			//ent->client->pers.selected_item = ITEM_INDEX(item);
			//ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Tank Rocket Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Tank Machinegun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Tank Blaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; 
			ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 18)
		{
			//Class 18
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= jorg_health->value;
		ent->client->pers.max_health		= jorg_health->value;//jorg

			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 700;
			item = FindItem("Jorg Chainguns");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 19)
		{
			//Class 19
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= makron_health->value;
		ent->client->pers.max_health		= makron_health->value;//bfg


			item = FindItem("Makron Railgun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Makron BFG");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Slugs");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 230;
			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 450;
			item = FindItem("Makron Blaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 30)
		{
			//Class 19
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= widow1_health->value;
		ent->client->pers.max_health		= widow1_health->value;//wid


			item = FindItem("Widow Kick");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Widow Railgun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Widow Spawner");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Slugs");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 230;
			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 450;
			item = FindItem("Widow Blaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 31)
		{
			//Class 31
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= widow2_health->value;
		ent->client->pers.max_health		= widow2_health->value;//wid2


			item = FindItem("Black Widow Tongs");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Black Widow Disruptor");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Widow Spawner");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Slugs");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;
			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 500;
			item = FindItem("Black Widow Heat Beam");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 15)
		{
			//Class 15
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= icarus_health->value;
		ent->client->pers.max_health		= icarus_health->value;//icar

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 540;
			item = FindItem("Icarus HyperBlaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 26)
		{
			//Class 15
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= daedalus_health->value;
		ent->client->pers.max_health		= daedalus_health->value;//daedalus

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 300;
			item = FindItem("Daedalus HyperBlaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 20) 
		{
			//Class 1
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= betasoldier_health->value;
		ent->client->pers.max_health		= betasoldier_health->value;//bsoldier

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 700;
			item = FindItem("Soldier LaserGun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Soldier HyperGun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Soldier RipperGun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 25) 
		{
			//Class 1
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= gekk_health->value;
		ent->client->pers.max_health		= gekk_health->value;//gek

			item = FindItem("Gekk Spit");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Gekk Claws");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 27) 
		{
			//Class 1
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= stalker_health->value;
		ent->client->pers.max_health		= stalker_health->value;//stalk

			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;
			item = FindItem("Stalker Blaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
			item = FindItem("Stalker Melee");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 37) 
		{
			//Class 1
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= traitorm_health->value;
		ent->client->pers.max_health		= traitorm_health->value;


			item = FindItem("Body Armor");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 400;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 800;
			item = FindItem("Slugs");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;
			item = FindItem("Shells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 450;

  			item = FindItem("Traitor Chaingun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
   			item = FindItem("Traitor Railgun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
   			item = FindItem("Traitor Super Shotgun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (ent->client->resp.class == 38) 
		{
			//Class 1
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));

		ent->client->pers.health			= traitorf_health->value;
		ent->client->pers.max_health		= traitorf_health->value;


			item = FindItem("Body Armor");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 400;
			item = FindItem("Bullets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 800;
			item = FindItem("Cells");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 500;
			item = FindItem("Rockets");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 200;

  			item = FindItem("Traitor Chaingun");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
   			item = FindItem("Traitor HyperBlaster");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;
      		item = FindItem("Traitor Rocket Launcher");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
		else if (client->resp.class == 40)
		{
			//Class 40
			gitem_t         *item;
			
			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));
			
			client->pers.health			= parasite_health->value;
			client->pers.max_health		= parasite_health->value;//para
			
			item = FindItem("Parasite Drain Attack");
			client->pers.selected_item = ITEM_INDEX(item);
			client->pers.inventory[client->pers.selected_item] = 1;
			item = FindItem("Parasite Suck Attack");
			client->pers.selected_item = ITEM_INDEX(item);
			client->pers.inventory[client->pers.selected_item] = 1;
			
			client->pers.weapon = item; client->pers.lastweapon = item;
		}
		else 
		{
			//Observer mode, doesn't really matter what they have
			gitem_t         *item;

			if (deathmatch->value) memset (&client->pers, 0, sizeof(client->pers));
    
		ent->client->pers.health			= 1;
		ent->client->pers.max_health		= 1;

			item = FindItem("Combat Armor");
			ent->client->pers.selected_item = ITEM_INDEX(item);
			ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

			ent->client->pers.weapon = item; ent->client->pers.lastweapon = item;
		}
}

void GetClientBboxes (edict_t *ent)
{
	if (ent->client->resp.class == 1)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 36)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 37)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 38)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 2)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 3)
	{
		VectorSet (ent->mins, -32, -32, -16);
		VectorSet (ent->maxs, 32, 32, 72);
	}
	else if (ent->client->resp.class == 33)
	{
		VectorSet (ent->mins, -32, -32, -16);
		VectorSet (ent->maxs, 32, 32, 72);
	}
	else if (ent->client->resp.class == 4)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 28)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 5)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 6)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 34)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 7)
	{
		VectorSet (ent->mins, -32, -32, -24);
		VectorSet (ent->maxs, 32, 32, 64);
	}
	else if (ent->client->resp.class == 39)
	{
		VectorSet (ent->mins, -32, -32, -24);
		VectorSet (ent->maxs, 32, 32, 64);
	}
	else if (ent->client->resp.class == 39)
	{
		VectorSet (ent->mins, -32, -32, -24);
		VectorSet (ent->maxs, 32, 32, 64);
	}
	else if (ent->client->resp.class == 8)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 16); // 24, too big
	}
	else if (ent->client->resp.class == 35)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 16); // 24, too big
	}
	else if (ent->client->resp.class == 32)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 9)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 10)
	{
		VectorSet (ent->mins, -16, -16, 0);
		VectorSet (ent->maxs, 16, 16, 56);
	}
	else if (ent->client->resp.class == 11)
	{
		VectorSet (ent->mins, -56, -56, 0);
		VectorSet (ent->maxs, 56, 56, 80);
	}
	else if (ent->client->resp.class == 29)
	{
		VectorSet (ent->mins, -56, -56, 0);
		VectorSet (ent->maxs, 56, 56, 80);
	}
	else if (ent->client->resp.class == 12)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 13)
	{
		VectorSet (ent->mins, -32, -32, -24);
		VectorSet (ent->maxs, 32, 32, 48);
	}
	else if (ent->client->resp.class == 14)
	{
		VectorSet (ent->mins, -32, -32, -16);
		VectorSet (ent->maxs, 32, 32, 72);
	}
	else if (ent->client->resp.class == 41)
	{
		VectorSet (ent->mins, -32, -32, -16);
		VectorSet (ent->maxs, 32, 32, 72);
	}
	else if (ent->client->resp.class == 15)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 26)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 16)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 17)
	{
		VectorSet (ent->mins, -16, -16, 0);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 18)
	{
		VectorSet (ent->mins, -80, -80, 0);
		VectorSet (ent->maxs, 80, 80, 140);
	}
	else if (ent->client->resp.class == 31)
	{
		VectorSet (ent->mins, -80, -80, 0);
		VectorSet (ent->maxs, 80, 80, 140);
	}
	else if (ent->client->resp.class == 19)
	{
		VectorSet (ent->mins, -30, -30, 0);
		VectorSet (ent->maxs, 30, 30, 90);
	}
	else if (ent->client->resp.class == 30)
	{
		VectorSet (ent->mins, -30, -30, 0);
		VectorSet (ent->maxs, 30, 30, 140);
	}
	else if (ent->client->resp.class == 20)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 21)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 22)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 23)
	{
		VectorSet (ent->mins, -16, -16, 0);
		VectorSet (ent->maxs, 16, 16, 56);
	}
	else if (ent->client->resp.class == 24)
	{
		VectorSet (ent->mins, -64, -64, 0);
		VectorSet (ent->maxs, 64, 64, 112);
	}
	else if (ent->client->resp.class == 25)
	{
		VectorSet (ent->mins, -32, -32, -24);
		VectorSet (ent->maxs, 32, 32, 48);
	}
	else if (ent->client->resp.class == 26)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 27)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 16);
	}
	else if (ent->client->resp.class == 37)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 38)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 40)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	// Odd, no class.
	// No bbox.
	// Prevents crash
	else
	{
		VectorSet (ent->mins, 0, 0, 0);
		VectorSet (ent->maxs, 0, 0, 0);
	}

}

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
int happenonce;
void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-64, -64, 0};
	vec3_t	maxs = {64, 64, 112};
	// FUBAR!
//	vec3_t  min;
//	vec3_t  max;
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	int theclass;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if (ent->client->resp.class == 17)
	{
/*--------------------------------*/
/*     new code starts here       */
/*--------------------------------*/
        VectorCopy (mins, ent->mins);
        VectorCopy (maxs, ent->maxs);
        if (findspawnpoint(ent)) // new line
        {
               VectorCopy(ent->s.origin, spawn_origin);
               VectorClear (spawn_angles);
               spawn_angles[YAW] = rand() % 360 - 180; // face a random direction
        }     
        else // couldn't find a good spot, so...
               SelectSpawnPoint (ent, spawn_origin, spawn_angles);
	}
/*--------------------------------*/
/*     new code ends here         */
/*--------------------------------*/
	else
		SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		
		ent->client->pers.skin_pain = 0;
		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
		//gi.bprintf (PRINT_HIGH, "Client in server.\n");
	}
	else if (coop->value)
	{
		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		// save class for level changing
		theclass = client->resp.class;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < game.num_items; n++)
		{
			if (itemlist[n].flags & IT_KEY)
				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
		}
		resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
		resp.coop_respawn.helpchanged = client->pers.helpchanged;
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	if (client->resp.exp)
		ent->exp = client->resp.exp;
	if (client->resp.lvl)
		ent->lvl = client->resp.lvl;
	if (client->resp.score)
		ent->score = client->resp.score;

	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;
	// Reset saved class.
	// Makes sure a client can respawn correctly.
	if (coop->value)
		client->resp.class = theclass;

	if (coop->value)
	{
		// InitClientPersistant seems to not like me, so this goes here.
		// Weapons for the class.
		InitCoopPersistant (ent);
	}

	// Need to set the player's new data!
	//if (ent->need_to_set)
	//{
		client->resp.exp = ent->exp;
		client->resp.lvl = ent->lvl;
		client->resp.score = ent->score;
	//}
	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// Make sure the client has the right hit boxes.
	GetClientBboxes(ent);


	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;
	ent->client->resp.nextfootstep=0;
	ent->ismonster = 0;
	// acrid 3/99 start unfrozen
	ent->frozen = 0;
	ent->client->pers.stalker_ceiling = 0;
	ent->client->pers.abilities.powerpoints = 301;
	ent->s.effects |= 0;
	ent->s.renderfx |= RF_IR_VISIBLE;
	ent->client->pers.abilities.brain_proxies = 0;
    gi.cvar_forceset("gl_saturatelighting","0"); 
    gi.cvar_forceset("r_fullbright","0"); 
	ent->team_owner = ent;
	//ent->client->ps.rdflags |= RDF_IRGOGGLES;
	if (ent->client->resp.lvl == 0)
		ent->client->resp.lvl = 1; // Paril, initiation for RPG :P

        //Wyrm: chasecam
        ent->svflags &= ~SVF_NOCLIENT;
        ent->owner = NULL;
        ent->chasedist2 = ent->client->cammaxdistance = 90;

	if (!happenonce)
	{
		happenonce = 1;
	}

	// Abilities
	ent->client->pers.abilities.soldier_haste = 0;
	ent->client->pers.abilities.tank_dualrockets = 0;
	ent->client->pers.abilities.gunner_forcegrenades = 0;
	ent->client->pers.abilities.gladiator_wallpierce = 0;
	ent->client->pers.abilities.floater_fragmulti = 0;
	ent->client->pers.abilities.berserk_shield = 0;
	ent->client->pers.abilities.chick_triplespread = 0;
	ent->client->pers.abilities.icarus_explobolts = 0;
	ent->client->resp.spawned_monsters = 0;
	ent->client->pers.selected = NULL;
	ent->s.renderfx |= RF_FRAMELERP;

	if (ent->client->resp.bloody) // Check for blood on respawn
	{
//		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));

		ClientUserinfoChanged (ent, userinfo);
	}

	StuffSpeedByClass( ent );
	ent->client->resp.class_speed = AssignClassSpeed( ent );

	if (deathmatch->value || coop->value)
	{
		if (!ent->client->resp.class)
		{
			if ((int)(stroggflags->value) & SF_RPG_MODE)
				OpenLoginMenu(ent, NULL);
			else
				OpenMainMenus (ent); 
		}
	}

	//VectorCopy (mins, ent->mins);
	//VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;
//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
	{
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
	}

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

//ZOID
	if (CTFStartClient(ent))
		return;
//ZOID

	// spawn a spectator
	if (client->pers.spectator) {
		client->chase_target = NULL;

		client->resp.spectator = true;

		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);
		return;
	} else
		client->resp.spectator = false;

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);
}

void EndObserverMode_Walking(edict_t* ent) 
{ 

	ChasecamRemove(ent);
    ent->movetype &= ~MOVETYPE_NOCLIP; 
    ent->solid &= ~SOLID_NOT; 
    ent->svflags &= ~SVF_NOCLIENT; 

	CTFDeadDropTech (ent);

    PutClientInServer (ent);

	WRITE_PLAYER_STATS(ent);

    if (level.intermissiontime)
    {
        MoveClientToIntermission (ent);
    }
    else
    {
        // send effect
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_LOGIN);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
    }

		ent->client->resp.thrusting = 0;
		ent->client->resp.next_thrust_sound=0;
		ent->client->resp.tread = 0;

		if (ent->client->resp.class == 17)
			ent->client->resp.landtype = 0;
		else
			ent->client->resp.landtype = 1;

		if (teamplay_landtype->value)
		{
			if (ent->client->resp.class == 17)
			{
				safe_bprintf (PRINT_HIGH, "%s has entered as a flying monster!\n", ent->client->pers.netname); 
				if (logging->value)
					fprintf (logged, "%s has entered as a flying monster!\n", ent->client->pers.netname);
			}
			else
			{
				safe_bprintf (PRINT_HIGH, "%s has entered as a ground monster!\n", ent->client->pers.netname); 
				if (logging->value)
					fprintf (logged, "%s has entered as a ground monster!\n", ent->client->pers.netname);
			}
		}
		else
		{
			safe_bprintf (PRINT_HIGH, "%s has entered!\n", ent->client->pers.netname); 
			if (logging->value)
				fprintf (logged, "%s has entered! (class %i)\n", ent->client->pers.netname, ent->client->resp.class);
		}
}

void EndObserverMode_Tread(edict_t* ent) 
{ 
	ChasecamRemove(ent);
    ent->movetype &= ~MOVETYPE_NOCLIP; 
    ent->solid &= ~SOLID_NOT; 
    ent->svflags &= ~SVF_NOCLIENT;

	CTFDeadDropTech (ent);

    PutClientInServer (ent);

	WRITE_PLAYER_STATS(ent);

    if (level.intermissiontime)
    {
        MoveClientToIntermission (ent);
    }
    else
    {
        // send effect
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_LOGIN);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
    }

		ent->client->resp.thrusting = 0;
		ent->client->resp.next_thrust_sound=0;
		ent->client->resp.tread = 1;
		ent->client->resp.landtype = 1;

		if (teamplay_landtype->value)
		{
			safe_bprintf (PRINT_HIGH, "%s has entered as a ground monster!\n", ent->client->pers.netname); 
			if (logging->value)
				fprintf (logged, "%s has entered as a ground monster!\n", ent->client->pers.netname);
		}
		else
		{
			safe_bprintf (PRINT_HIGH, "%s has entered!\n", ent->client->pers.netname); 
			if (logging->value)
				fprintf (logged, "%s has entered! (class %i)\n", ent->client->pers.netname, ent->client->resp.class);
		}
//    gi.bprintf (PRINT_HIGH, "%s has entered!\n", ent->client->pers.netname); 

} 


void EndObserverMode_Flying(edict_t* ent) 
{ 
	ChasecamRemove(ent);
    ent->movetype &= ~MOVETYPE_NOCLIP; 
    ent->solid &= ~SOLID_NOT; 
    ent->svflags &= ~SVF_NOCLIENT; 

	CTFDeadDropTech (ent);

    PutClientInServer (ent);

	WRITE_PLAYER_STATS(ent);

    if (level.intermissiontime)
    {
        MoveClientToIntermission (ent);
    }
    else
    {
        // send effect
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_LOGIN);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
    }

		ent->client->resp.thrusting=1;
		ent->client->resp.next_thrust_sound=0;
		ent->client->resp.tread = 0;
		ent->client->resp.next_weld_sound=0;

		ent->client->resp.landtype = 0;

		if (teamplay_landtype->value)
		{
			safe_bprintf (PRINT_HIGH, "%s has entered as a flying monster!\n", ent->client->pers.netname); 
			if (logging->value)
				fprintf (logged, "%s has entered as a flying monster\n", ent->client->pers.netname);
		}
		else
		{
			safe_bprintf (PRINT_HIGH, "%s has entered!\n", ent->client->pers.netname); 
			if (logging->value)
				fprintf (logged, "%s has entered! (class %i)\n", ent->client->pers.netname, ent->client->resp.class);
		}
} 



/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	G_InitEdict (ent);

	InitClientResp (ent->client);

	// locate ent at a spawn point
	PutClientInServer (ent);

    ent->client->ps.gunindex = 0; 
    gi.linkentity (ent); 

	safe_bprintf (PRINT_HIGH, "%s entered the world\n", ent->client->pers.netname);
	if (logging->value)
		fprintf (logged, "%s entered the world\n", ent->client->pers.netname);

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int		i;

	ent->client = game.clients + (ent - g_edicts - 1);

	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGIN);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			safe_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
			if (logging->value)
				fprintf (logged, "%s entered the world\n", ent->client->pers.netname);
		}
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s;
	int		playernum;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set spectator
	s = Info_ValueForKey (userinfo, "spectator");
	// spectators are only supported in deathmatch
	if (deathmatch->value && *s && strcmp(s, "0"))
		ent->client->pers.spectator = true;
	else
		ent->client->pers.spectator = false;

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
//ZOID
	if (ent->client->resp.class != 0)
	{
//		gi.bprintf (PRINT_HIGH, "Assigned skin\n");
		//ZOID
		//if (ctf->value)
			//AssignCTFSkins(ent, s);
		//else
			CTFAssignSkin(ent, s);
//ZOID
	}
	else
//ZOID
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

//ZOID
	// set player name field (used in id_state view)
	gi.configstring (CS_GENERAL+playernum, ent->client->pers.netname);
//ZOID

	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char	*value;
	char tmpbuf[20];
	_strtime (tmpbuf);

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}

	// check for a spectator
	value = Info_ValueForKey (userinfo, "spectator");
	if (deathmatch->value && *value && strcmp(value, "0")) {
		int i, numspec;

		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
			return false;
		}

		// count spectators
		for (i = numspec = 0; i < maxclients->value; i++)
			if (g_edicts[i+1].inuse && g_edicts[i+1].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
			return false;
		}
	} else {
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
			return false;
		}
	}


	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
//ZOID -- force team join
		ent->client->resp.ctf_team = -1;
		ent->client->resp.id_state = true; 

//ZOID
		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
	{
		if (logging->value)
			fprintf (logged, "%s connected to the server	(%s)\n", ent->client->pers.netname, tmpbuf);
		gi.dprintf ("%s connected\n", ent->client->pers.netname);
	}

	ent->svflags = 0; // make sure we start with known default
	ent->client->pers.connected = true;
	return true;

}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;
    char tmpbuf[20];
    _strtime(tmpbuf);

	if (!ent->client)
		return;

	safe_bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);
	if (logging->value)
	{
		fprintf (logged, "%s left the server at %s\n", ent->client->pers.netname, tmpbuf);
	}

	WRITE_PLAYER_STATS (ent);//[QBS] 

//ZOID
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
//ZOID

        //Wyrm: chasecam
        ChasecamRemove(ent);

	if (ent->client->pers.monster1)
	{
		G_FreeEdict (ent->client->pers.monster1);
		ent->client->pers.monster1 = NULL;
		ent->client->resp.spawned_monsters --;
	}
		//T_Damage (self->client->pers.monster1, self, self, vec3_origin, self->client->pers.monster1->s.origin, vec3_origin, 1000000, 0, 0, 0);

	if (ent->client->pers.monster2)
	{
		G_FreeEdict (ent->client->pers.monster2);
		ent->client->pers.monster2 = NULL;
		ent->client->resp.spawned_monsters --;
	}
		//T_Damage (self->client->pers.monster2, self, self, vec3_origin, self->client->pers.monster2->s.origin, vec3_origin, 1000000, 0, 0, 0);

	if (ent->client->pers.monster3)
	{
		G_FreeEdict (ent->client->pers.monster3);
		ent->client->pers.monster3 = NULL;
		ent->client->resp.spawned_monsters --;
	}
		//T_Damage (self->client->pers.monster3, self, self, vec3_origin, self->client->pers.monster3->s.origin, vec3_origin, 1000000, 0, 0, 0);

	if (ent->client->pers.monster4)
	{
		G_FreeEdict (ent->client->pers.monster4);
		ent->client->pers.monster4 = NULL;
		ent->client->resp.spawned_monsters --;
	}
		//T_Damage (self->client->pers.monster4, self, self, vec3_origin, self->client->pers.monster4->s.origin, vec3_origin, 1000000, 0, 0, 0);

	if (ent->client->pers.monster5)
	{
		G_FreeEdict (ent->client->pers.monster5);
		ent->client->pers.monster5 = NULL;
		ent->client->resp.spawned_monsters --;
	}
		//T_Damage (self->client->pers.monster5, self, self, vec3_origin, self->client->pers.monster5->s.origin, vec3_origin, 1000000, 0, 0, 0);

	if (ent->client->pers.monster6)
	{
		G_FreeEdict (ent->client->pers.monster6);
		ent->client->pers.monster6 = NULL;
		ent->client->resp.spawned_monsters --;
	}
		//T_Damage (self->client->pers.monster6, self, self, vec3_origin, self->client->pers.monster6->s.origin, vec3_origin, 1000000, 0, 0, 0);


	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
}


//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

//========================================================
void Cmd_Catapult_f(edict_t *ent) {
vec3_t forward,up;

  // Not available to dead or respawning players!
  //if (!G_ClientInGame(ent)) return;

  // Is ent currently airborne?
  if (ent->groundentity==NULL) return;

  else {
    // Physically throw the player up and forward!
    AngleVectors(ent->s.angles, forward, NULL, up);
    VectorCopy(forward, ent->movedir);
    VectorClear(ent->velocity);
    VectorMA(ent->velocity,450,forward,ent->velocity); // forward
    VectorMA(ent->velocity,400,up,ent->velocity); }    // upward
}

void Cmd_Stalk_f(edict_t *ent) {
vec3_t forward,up;

  // Not available to dead or respawning players!
  //if (!G_ClientInGame(ent)) return;

  // Is ent currently airborne?
  if (ent->groundentity==NULL) return;

  else {
    // Physically throw the player up and forward!
    AngleVectors(ent->s.angles, forward, NULL, up);
    VectorCopy(forward, ent->movedir);
    VectorClear(ent->velocity);
    //VectorMA(ent->velocity,450,forward,ent->velocity); // forward
    VectorMA(ent->velocity,400,up,ent->velocity); }    // upward
}

void MutantJumpAttack (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 40 + 10 * random();
	int			kick = 40;
	vec3_t		offset;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Middle_Source (ent->client, ent->s.origin, offset, forward, right, start);

	fire_player_melee_mutant (ent, start, forward, 80, damage, kick, 1, MOD_BLASTER);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

static vec3_t beameffects[] = {
	{12.58, -43.71, 68.88},
	{3.43, 58.72, 68.41}
};

void gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void gib_touch (edict_t *gib, edict_t *other, cplane_t *plane, csurface_t *surf);
void ThrowWidowGibReal (edict_t *self, char *gibname, int damage, int type, vec3_t startpos, qboolean sized, int hitsound, qboolean fade);

#define	MAX_LEGSFRAME	23
#define	LEG_WAIT_TIME	1

void ThrowWidowGibLoc (edict_t *self, char *gibname, int damage, int type, vec3_t startpos, qboolean fade)
{
	ThrowWidowGibReal (self, gibname, damage, type, startpos, false, 0, fade);
}

void ThrowSmallStuff (edict_t *self, vec3_t point)
{
	int n;

	for (n= 0; n < 1; n++)
		ThrowWidowGibLoc (self, "models/objects/gibs/sm_meat/tris.md2", 300, GIB_ORGANIC, point, false);
	ThrowWidowGibLoc (self, "models/objects/gibs/sm_metal/tris.md2", 300, GIB_METALLIC, point, false);
	ThrowWidowGibLoc (self, "models/objects/gibs/sm_metal/tris.md2", 100, GIB_METALLIC, point, false);

}

void WidowVelocityForDamage (int damage, vec3_t v)
{
	v[0] = damage * crandom();
	v[1] = damage * crandom();
	v[2] = damage * crandom() + 200.0;
}

void widow_gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{

	self->solid = SOLID_NOT;
	self->touch = NULL;
	self->s.angles[PITCH] = 0;
	self->s.angles[ROLL] = 0;
	VectorClear (self->avelocity);

	if (self->plat2flags)
		gi.sound (self, CHAN_VOICE, self->plat2flags, 1, ATTN_NORM, 0);
/*
	if (plane)
	{
		if (plane->normal[2] < -0.8)
		{
			gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/fhit3.wav"), 1, ATTN_NORM, 0);
		}
		
		//vectoangles (plane->normal, normal_angles);
		//AngleVectors (normal_angles, NULL, right, NULL);
		//vectoangles (right, self->s.angles);
		//VectorClear (self->avelocity);
	}
*/
}

void ThrowWidowGib (edict_t *self, char *gibname, int damage, int type)
{
	ThrowWidowGibReal (self, gibname, damage, type, NULL, false, 0, true);
}

void ThrowWidowGibReal (edict_t *self, char *gibname, int damage, int type, vec3_t startpos, qboolean sized, int hitsound, qboolean fade)
{
	edict_t *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;

	if (!gibname)
		return;

	gib = G_Spawn();

	if (startpos)
		VectorCopy (startpos, gib->s.origin);
	else
	{
		VectorScale (self->size, 0.5, size);
		VectorAdd (self->absmin, size, origin);
		gib->s.origin[0] = origin[0] + crandom() * size[0];
		gib->s.origin[1] = origin[1] + crandom() * size[1];
		gib->s.origin[2] = origin[2] + crandom() * size[2];
	}

	gib->solid = SOLID_NOT;
	gib->s.effects |= EF_GIB;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;
	gib->s.renderfx |= RF_IR_VISIBLE;

	if (fade)
	{
		gib->think = G_FreeEdict;
		// sized gibs last longer
		if (sized)
			gib->nextthink = level.time + 5 + random()*5;
		else
			gib->nextthink = level.time + 5 + random()*5;
	}
	else
	{
		gib->think = G_FreeEdict;
		// sized gibs last longer
		if (sized)
			gib->nextthink = level.time + 5 + random()*5;
		else
			gib->nextthink = level.time + 5 + random()*5;
	}

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_TOSS;
		gib->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	WidowVelocityForDamage (damage, vd);
	VectorMA (self->velocity, vscale, vd, gib->velocity);
	ClipGibVelocity (gib);

	gi.setmodel (gib, gibname);

	if (sized)
	{
		gib->plat2flags = hitsound;
		gib->solid = SOLID_BBOX;
		gib->avelocity[0] = random()*400;
		gib->avelocity[1] = random()*400;
		gib->avelocity[2] = random()*200;
		if (gib->velocity[2] < 0)
			gib->velocity[2] *= -1;
		gib->velocity[0] *= 2;
		gib->velocity[1] *= 2;
		ClipGibVelocity (gib);
		gib->velocity[2] = fmaxf((350 + (random()*100.0)), gib->velocity[2]);
		gib->gravity = 0.25;
		gib->touch = widow_gib_touch;
		gib->owner = self;
		if (gib->s.modelindex == gi.modelindex ("models/monsters/blackwidow2/gib2/tris.md2"))
		{
			VectorSet (gib->mins, -10, -10, 0);
			VectorSet (gib->maxs, 10, 10, 10);
		}
		else
		{
			VectorSet (gib->mins, -5, -5, 0);
			VectorSet (gib->maxs, 5, 5, 5);
		}
	}
	else
	{
		gib->velocity[0] *= 2;
		gib->velocity[1] *= 2;
		gib->avelocity[0] = random()*600;
		gib->avelocity[1] = random()*600;
		gib->avelocity[2] = random()*600;
	}

//	gib->think = G_FreeEdict;
//	gib->nextthink = level.time + 10 + random()*10;

	gi.linkentity (gib);
}

void ThrowWidowGibSized (edict_t *self, char *gibname, int damage, int type, vec3_t startpos, int hitsound, qboolean fade)
{
	ThrowWidowGibReal (self, gibname, damage, type, startpos, true, hitsound, fade);
}

void widowlegs_think (edict_t *self)
{
	vec3_t	offset;
	vec3_t	point;
	vec3_t	f,r,u;

	if (self->s.frame == 17)
	{
		VectorSet (offset, 11.77, -7.24, 23.31);
		AngleVectors (self->s.angles, f, r, u);
		G_ProjectSource2 (self->s.origin, offset, f, r, u, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (point);
		gi.multicast (point, MULTICAST_ALL);
		ThrowSmallStuff (self, point);
	}

	if (self->s.frame < MAX_LEGSFRAME)
	{
		self->s.frame++;
		self->nextthink = level.time + FRAMETIME;
		return;
	}
	else if (self->wait == 0)
	{
		self->wait = level.time + LEG_WAIT_TIME;
	}
	if (level.time > self->wait)
	{
		AngleVectors (self->s.angles, f, r, u);

		VectorSet (offset, -65.6, -8.44, 28.59);
		G_ProjectSource2 (self->s.origin, offset, f, r, u, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (point);
		gi.multicast (point, MULTICAST_ALL);
		ThrowSmallStuff (self, point);

		ThrowWidowGibSized (self, "models/monsters/blackwidow/gib1/tris.md2", 80 + (int)(random()*20.0), GIB_METALLIC, point, 0, true);
		ThrowWidowGibSized (self, "models/monsters/blackwidow/gib2/tris.md2", 80 + (int)(random()*20.0), GIB_METALLIC, point, 0, true);

		VectorSet (offset, -1.04, -51.18, 7.04);
		G_ProjectSource2 (self->s.origin, offset, f, r, u, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (point);
		gi.multicast (point, MULTICAST_ALL);
		ThrowSmallStuff (self, point);

		ThrowWidowGibSized (self, "models/monsters/blackwidow/gib1/tris.md2", 80 + (int)(random()*20.0), GIB_METALLIC, point, 0, true);
		ThrowWidowGibSized (self, "models/monsters/blackwidow/gib2/tris.md2", 80 + (int)(random()*20.0), GIB_METALLIC, point, 0, true);
		ThrowWidowGibSized (self, "models/monsters/blackwidow/gib3/tris.md2", 80 + (int)(random()*20.0), GIB_METALLIC, point, 0, true);

		G_FreeEdict (self);
		return;
	}
	if ((level.time > (self->wait - 0.5)) && (self->count == 0))
	{
		self->count = 1;
		AngleVectors (self->s.angles, f, r, u);

		VectorSet (offset, 31, -88.7, 10.96);
		G_ProjectSource2 (self->s.origin, offset, f, r, u, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (point);
		gi.multicast (point, MULTICAST_ALL);
//		ThrowSmallStuff (self, point);

		VectorSet (offset, -12.67, -4.39, 15.68);
		G_ProjectSource2 (self->s.origin, offset, f, r, u, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (point);
		gi.multicast (point, MULTICAST_ALL);
//		ThrowSmallStuff (self, point);

		self->nextthink = level.time + FRAMETIME;
		return;
	}
	self->nextthink = level.time + FRAMETIME;
}

void Widowlegs_Spawn (vec3_t startpos, vec3_t angles)
{
	edict_t *ent;

	ent = G_Spawn();
	VectorCopy(startpos, ent->s.origin);
	VectorCopy(angles, ent->s.angles);
	ent->solid = SOLID_NOT;
	ent->s.renderfx = RF_IR_VISIBLE;
	ent->movetype = MOVETYPE_NONE;
	ent->classname = "widowlegs";

	ent->s.modelindex = gi.modelindex("models/monsters/legs/tris.md2");
	ent->think = widowlegs_think;

	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity (ent);
}


void WidowExplosion1 (edict_t *self)
{
//	int		n;
	vec3_t	f,r,u, startpoint;
	vec3_t	offset = {23.74, -37.67, 76.96};

//	gi.dprintf ("1\n");
	AngleVectors (self->s.angles, f, r, u);
	G_ProjectSource2 (self->s.origin, offset, f, r, u, startpoint);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (startpoint);
	gi.multicast (self->s.origin, MULTICAST_ALL);
}

void WidowExplosion2 (edict_t *self)
{
//	int		n;
	vec3_t	f,r,u, startpoint;
	vec3_t	offset = {-20.49, 36.92, 73.52};

//	gi.dprintf ("2\n");

	AngleVectors (self->s.angles, f, r, u);
	G_ProjectSource2 (self->s.origin, offset, f, r, u, startpoint);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (startpoint);
	gi.multicast (self->s.origin, MULTICAST_ALL);
}

void WidowExplosion3 (edict_t *self)
{
	int		n;
	vec3_t	f,r,u, startpoint;
	vec3_t	offset = {2.11, 0.05, 92.20};

//	gi.dprintf ("3\n");

	AngleVectors (self->s.angles, f, r, u);
	G_ProjectSource2 (self->s.origin, offset, f, r, u, startpoint);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (startpoint);
	gi.multicast (self->s.origin, MULTICAST_ALL);
	
	for (n= 0; n < 1; n++)
		ThrowWidowGibLoc (self, "models/objects/gibs/sm_meat/tris.md2", 300, GIB_ORGANIC, startpoint, false);
	for (n= 0; n < 1; n++)
		ThrowWidowGibLoc (self, "models/objects/gibs/sm_metal/tris.md2", 100, GIB_METALLIC, startpoint, false);
	for (n= 0; n < 1; n++)
		ThrowWidowGibLoc (self, "models/objects/gibs/sm_metal/tris.md2", 300, GIB_METALLIC, startpoint, false);
}

void WidowExplosion4 (edict_t *self)
{
	int		n;
	vec3_t	f,r,u, startpoint;
	vec3_t	offset = {-28.04, -35.57, -77.56};

//	gi.dprintf ("4\n");

	AngleVectors (self->s.angles, f, r, u);
	G_ProjectSource2 (self->s.origin, offset, f, r, u, startpoint);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (startpoint);
	gi.multicast (self->s.origin, MULTICAST_ALL);
	
	for (n= 0; n < 1; n++)
		ThrowWidowGibLoc (self, "models/objects/gibs/sm_meat/tris.md2", 300, GIB_ORGANIC, startpoint, false);
	for (n= 0; n < 1; n++)
		ThrowWidowGibLoc (self, "models/objects/gibs/sm_metal/tris.md2", 100, GIB_METALLIC, startpoint, false);
	for (n= 0; n < 1; n++)
		ThrowWidowGibLoc (self, "models/objects/gibs/sm_metal/tris.md2", 300, GIB_METALLIC, startpoint, false);
}

void WidowExplosion5 (edict_t *self)
{
//	int		n;
	vec3_t	f,r,u, startpoint;
	vec3_t	offset = {-20.11, -1.11, 40.76};

//	gi.dprintf ("5\n");

	AngleVectors (self->s.angles, f, r, u);
	G_ProjectSource2 (self->s.origin, offset, f, r, u, startpoint);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (startpoint);
	gi.multicast (self->s.origin, MULTICAST_ALL);
	
}

void WidowExplosion6 (edict_t *self)
{
//	int		n;
	vec3_t	f,r,u, startpoint;
	vec3_t	offset = {-20.11, -1.11, 40.76};

	//gi.dprintf ("6\n");

	AngleVectors (self->s.angles, f, r, u);
	G_ProjectSource2 (self->s.origin, offset, f, r, u, startpoint);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (startpoint);
	gi.multicast (self->s.origin, MULTICAST_ALL);
	
}

void WidowExplosion7 (edict_t *self)
{
	int		n;
	vec3_t	f,r,u, startpoint;
	vec3_t	offset = {-20.11, -1.11, 40.76};

	//gi.dprintf ("7\n");

	AngleVectors (self->s.angles, f, r, u);
	G_ProjectSource2 (self->s.origin, offset, f, r, u, startpoint);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (startpoint);
	gi.multicast (self->s.origin, MULTICAST_ALL);
	
	for (n= 0; n < 1; n++)
		ThrowWidowGibLoc (self, "models/objects/gibs/sm_meat/tris.md2", 300, GIB_ORGANIC, startpoint, false);
	for (n= 0; n < 1; n++)
		ThrowWidowGibLoc (self, "models/objects/gibs/sm_metal/tris.md2", 100, GIB_METALLIC, startpoint, false);
	for (n= 0; n < 1; n++)
		ThrowWidowGibLoc (self, "models/objects/gibs/sm_metal/tris.md2", 300, GIB_METALLIC, startpoint, false);
}

void WidowExplosionLeg (edict_t *self)
{
//	int		n;
	vec3_t	f,r,u, startpoint;
	vec3_t	offset1 = {-31.89, -47.86, 67.02};
	vec3_t	offset2 = {-44.9, -82.14, 54.72};

	//gi.dprintf ("Leg\n");

	AngleVectors (self->s.angles, f, r, u);
	G_ProjectSource2 (self->s.origin, offset1, f, r, u, startpoint);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1_BIG);
	gi.WritePosition (startpoint);
	gi.multicast (self->s.origin, MULTICAST_ALL);

	G_ProjectSource2 (self->s.origin, offset2, f, r, u, startpoint);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (startpoint);
	gi.multicast (self->s.origin, MULTICAST_ALL);

	ThrowWidowGibSized (self, "models/monsters/blackwidow2/gib1/tris.md2", 300, GIB_METALLIC, startpoint,
		gi.soundindex ("misc/fhit3.wav"), false);
	ThrowWidowGibLoc (self, "models/objects/gibs/sm_meat/tris.md2", 300, GIB_ORGANIC, startpoint, false);
	ThrowWidowGibLoc (self, "models/objects/gibs/sm_metal/tris.md2", 100, GIB_METALLIC, startpoint, false);
}

void ThrowArm1 (edict_t *self)
{
	int		n;
	vec3_t	f,r,u, startpoint;
	vec3_t	offset1 = {65.76, 17.52, 7.56};

	AngleVectors (self->s.angles, f, r, u);
	G_ProjectSource2 (self->s.origin, offset1, f, r, u, startpoint);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1_BIG);
	gi.WritePosition (startpoint);
	gi.multicast (self->s.origin, MULTICAST_ALL);

	for (n= 0; n < 1; n++)
		ThrowWidowGibLoc (self, "models/objects/gibs/sm_metal/tris.md2", 100, GIB_METALLIC, startpoint, false);
}

void ThrowArm2 (edict_t *self)
{
//	int		n;
	vec3_t	f,r,u, startpoint;
	vec3_t	offset1 = {65.76, 17.52, 7.56};

	AngleVectors (self->s.angles, f, r, u);
	G_ProjectSource2 (self->s.origin, offset1, f, r, u, startpoint);

	ThrowWidowGibSized (self, "models/monsters/blackwidow2/gib4/tris.md2", 200, GIB_METALLIC, startpoint, 
		gi.soundindex ("misc/fhit3.wav"), false);
	ThrowWidowGibLoc (self, "models/objects/gibs/sm_meat/tris.md2", 300, GIB_ORGANIC, startpoint, false);
}

void BloodFountain (edict_t *self, int number, vec3_t startpos, int damage)
{
	int n;
	vec3_t	vd;
	vec3_t	origin, size, velocity;

	return;

	for (n= 0; n < number; n++)
	{
		if (startpos)
			VectorCopy (startpos, origin);
		else
		{
			VectorScale (self->size, 0.5, size);
			VectorAdd (self->absmin, size, origin);
			origin[0] = origin[0] + crandom() * size[0];
			origin[1] = origin[1] + crandom() * size[1];
			origin[2] = origin[2] + crandom() * size[2];
		}

		WidowVelocityForDamage (damage, vd);
		VectorMA (self->velocity, 1.0, vd, velocity);
		velocity[0] *= 2;
		velocity[1] *= 2;

//		gi.WriteByte (svc_temp_entity);
//		gi.WriteByte (TE_BLOOD_FOUNTAIN);
//		gi.WritePosition (origin);
//		gi.WritePosition (velocity);
//		gi.WriteShort (50);
//		gi.multicast (self->s.origin, MULTICAST_ALL);
	}
}


void ThrowMoreStuff (edict_t *self, vec3_t point)
{
//	int n;

	if (coop && coop->value)
	{
		ThrowSmallStuff (self, point);
		return;
	}

	ThrowWidowGibLoc (self, "models/objects/gibs/sm_metal/tris.md2", 100, GIB_METALLIC, point, false);

}

void WidowExplode (edict_t *self)
{
	vec3_t	org;
	int		n;

	self->think = WidowExplode;
//	gi.dprintf ("count = %d\n");

//redo:
	VectorCopy (self->s.origin, org);
	org[2] += 24 + (rand()&15);
	if (self->count < 8)
		org[2] += 24 + (rand()&31);
	switch (self->count)
	{
	case 0:
		org[0] -= 24;
		org[1] -= 24;
		break;
	case 1:
		org[0] += 24;
		org[1] += 24;
		ThrowSmallStuff(self, org);
		break;
	case 2:
		org[0] += 24;
		org[1] -= 24;
		break;
	case 3:
		org[0] -= 24;
		org[1] += 24;
		ThrowMoreStuff(self, org);
		break;
	case 4:
		org[0] -= 48;
		org[1] -= 48;
		break;
	case 5:
		org[0] += 48;
		org[1] += 48;
		ThrowArm1 (self);
		break;
	case 6:
		org[0] -= 48;
		org[1] += 48;
		ThrowArm2 (self);
		break;
	case 7:
		org[0] += 48;
		org[1] -= 48;
		ThrowSmallStuff(self, org);
		break;
	case 8:
		org[0] += 18;
		org[1] += 18;
		org[2] = self->s.origin[2] + 48;
		ThrowMoreStuff(self, org);
		break;
	case 9:
		org[0] -= 18;
		org[1] += 18;
		org[2] = self->s.origin[2] + 48;
		break;
	case 10:
		org[0] += 18;
		org[1] -= 18;
		org[2] = self->s.origin[2] + 48;
		break;
	case 11:
		org[0] -= 18;
		org[1] -= 18;
		org[2] = self->s.origin[2] + 48;
		break;
	case 12:
		self->s.sound = 0;
		for (n= 0; n < 1; n++)
			ThrowWidowGib (self, "models/objects/gibs/sm_meat/tris.md2", 400, GIB_ORGANIC);
		for (n= 0; n < 1; n++)
			ThrowWidowGib (self, "models/objects/gibs/sm_metal/tris.md2", 100, GIB_METALLIC);
		for (n= 0; n < 1; n++)
			ThrowWidowGib (self, "models/objects/gibs/sm_metal/tris.md2", 400, GIB_METALLIC);
//		ThrowGib (self, "models/objects/gibs/chest/tris.md2", 1000, GIB_ORGANIC);
//		ThrowHead (self, "models/objects/gibs/gear/tris.md2", 1000, GIB_METALLIC);
		self->deadflag = DEAD_DEAD;
		//self->think = monster_think;
		//self->nextthink = level.time + 0.1;
//		self->monsterinfo.currentmove = &widow2_move_dead;
		return;
	}

	self->count++;
	if (self->count >=9 && self->count <=12)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1_BIG);
		gi.WritePosition (org);
		gi.multicast (self->s.origin, MULTICAST_ALL);
//		goto redo;
	} 
	else
	{
		// else
		gi.WriteByte (svc_temp_entity);
		if (self->count %2)
			gi.WriteByte (TE_EXPLOSION1);
		else
			gi.WriteByte (TE_EXPLOSION1_NP);
		gi.WritePosition (org);
		gi.multicast (self->s.origin, MULTICAST_ALL);
	}

	self->nextthink = level.time + 0.1;
}

void BossExplode (edict_t *self);

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
// I made this shorter for myself
// Whee.
void StroggDM_SetClientEffects (edict_t *ent, usercmd_t *ucmd)
{
		if (ent->client->resp.class == 3 || ent->client->resp.class == 14 || ent->client->resp.class == 33 || ent->client->resp.class == 41)
		{
			if (ent->s.frame == 249)
				gi.sound (ent, CHAN_VOICE, gi.soundindex("tank/Tnkdeth2.wav"), 1, ATTN_NORM, 0);
		}

		if (ent->client->resp.class == 30)
		{
			if (ent->s.frame == 140)
			{
				vec3_t startpoint,f,r,u;

			//	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
				AngleVectors (ent->s.angles, f, r, u);

				G_ProjectSource2 (ent->s.origin, beameffects[0], f, r, u, startpoint);
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_WIDOWBEAMOUT);
				gi.WriteShort (20001);
				gi.WritePosition (startpoint);
				gi.multicast (startpoint, MULTICAST_ALL);

				G_ProjectSource2 (ent->s.origin, beameffects[1], f, r, u, startpoint);
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_WIDOWBEAMOUT);
				gi.WriteShort (20002);
				gi.WritePosition (startpoint);
				gi.multicast (startpoint, MULTICAST_ALL);

				gi.sound (ent, CHAN_VOICE, gi.soundindex ("misc/bwidowbeamout.wav"), 1, ATTN_NORM, 0);
			}
			if (ent->s.frame == 160)
			{
				//BecomeExplosion1(ent);
				vec3_t startpoint,f,r,u;

				AngleVectors (ent->s.angles, f, r, u);
				G_ProjectSource2 (ent->s.origin, beameffects[0], f, r, u, startpoint);
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_WIDOWSPLASH);
				gi.WritePosition (startpoint);
				gi.multicast (startpoint, MULTICAST_ALL);

				G_ProjectSource2 (ent->s.origin, beameffects[1], f, r, u, startpoint);
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_WIDOWSPLASH);
				gi.WritePosition (startpoint);
				gi.multicast (startpoint, MULTICAST_ALL);

				VectorCopy (ent->s.origin, startpoint);
				startpoint[2] += 36;
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BOSSTPORT);
				gi.WritePosition (startpoint);
				gi.multicast (startpoint, MULTICAST_PVS);

				Widowlegs_Spawn (ent->s.origin, ent->s.angles);
				ent->deadflag = DEAD_DEAD;
				gi.linkentity (ent);
				ThrowClientHead (ent);

				ent->takedamage = DAMAGE_NO;
			}
		}
		if (ent->client->resp.class == 31)
		{
			if (ent->s.frame == 62)
				WidowExplosion1 (ent);
			if (ent->s.frame == 65)
				WidowExplosion2 (ent);
			if (ent->s.frame == 77)
				WidowExplosion3 (ent);
			if (ent->s.frame == 84)
				WidowExplosion4 (ent);
			if (ent->s.frame == 88)
				WidowExplosion5 (ent);
			if (ent->s.frame == 89)
				WidowExplosionLeg (ent);
			if (ent->s.frame == 93)
				WidowExplosion6 (ent);
			if (ent->s.frame == 97)
				WidowExplosion7 (ent);
			if (ent->s.frame == 103)
				WidowExplode (ent);
		}

		if (ent->client->resp.class == 26 || ent->client->resp.class == 15)
		{
			if (ent->s.frame == 172)
			{
				BecomeExplosion1(ent);
				ent->deadflag = DEAD_DEAD;
				gi.linkentity (ent);
				ThrowClientHead (ent);

				ent->takedamage = DAMAGE_NO;
			}
		}
		if (ent->client->resp.class == 7)
		{
			if (ent->s.frame == 122)
			{
				BossExplode (ent);
			}
		}
		if (ent->client->resp.class == 39)
		{
			if (ent->s.frame == 122)
			{
				BossExplode (ent);
			}
		}

}

void Think_Lightn (edict_t *ent);
void Think_Airstrike (edict_t *ent);
void Kamikaze_Explode(edict_t *the_doomed_one);

void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;
	int backspeed;
	int sidespeed;
	int walkspeed;
//	vec3_t up;

	level.current_entity = ent;
	client = ent->client;

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

    if (client->on_hook == true)
    {
        Pull_Grapple(ent);
        client->ps.pmove.gravity = 0;
    }
    else
    {
        client->ps.pmove.gravity = sv_gravity->value;
    }

	Check_Levelup(ent);

	pm_passent = ent;

	//safe_cprintf (ent, PRINT_HIGH, "%i\n", ent->client->resp.class_speed);



//ZOID
	if (ent->client->chase_target) {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		return;
	}
//ZOID

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

// speed cap code 

	if (ucmd->forwardmove == (ent->client->resp.class_speed / 2))
		ent->client->resp.walking = 1;
	else
		ent->client->resp.walking = 0;

	if ( (ucmd->forwardmove > ent->client->resp.class_speed) || (ucmd->sidemove > ent->client->resp.class_speed) )
	{
		ucmd->forwardmove = ent->client->resp.class_speed;
		ucmd->sidemove = ent->client->resp.class_speed;
		if (!ent->is_bot)
			StuffSpeedByClass( ent );
	} 

	walkspeed = ent->client->resp.class_speed / 2;
	// negative speed (left/back movement) check
	if (ucmd->forwardmove < 0)
	{
		backspeed = ucmd->forwardmove + ent->client->resp.class_speed; 

		if ( backspeed != 0 )
		{
			if ( backspeed != walkspeed )
			{
			ucmd->forwardmove = 0 - ent->client->resp.class_speed;
			if (!ent->is_bot)
				StuffSpeedByClass( ent );
			}
		}
	}

	if (ucmd->sidemove < 0)
	{
		sidespeed = ucmd->sidemove + ent->client->resp.class_speed; 

		if ( sidespeed != 0 )
		{
			if (sidespeed != walkspeed)
			{
				ucmd->sidemove = 0 - ent->client->resp.class_speed;
				if (!ent->is_bot)
					StuffSpeedByClass( ent );
			}
		}
	} 


//ERASER END
//Acrid freeze 3/99
	if (ent->frozen)
	{
		if( level.time < ent->frozentime )
		{  
			ent->client->ps.pmove.pm_type = PM_DEAD;

	       if (ent->client->buttons & BUTTON_ATTACK)//4/99 acrid
		   {
		       ent->client->buttons &= ~BUTTON_ATTACK;
		   }

			return;
		}
		else
		{
			ent->frozen = 0;
		}
	}


		if (ent->movetype == MOVETYPE_NOCLIP)
			client->ps.pmove.pm_type = PM_SPECTATOR;
		else if (ent->s.modelindex != 255)
			client->ps.pmove.pm_type = PM_GIB;
		else if (ent->deadflag)
			client->ps.pmove.pm_type = PM_DEAD;
		else if (level.time < ent->frozentime)//botfreeze 3/99 needed still?
			client->ps.pmove.pm_type = PM_DEAD;//3/99
		else
			client->ps.pmove.pm_type = PM_NORMAL;


  /*ATTILA begin*/
		if (ent->health > 1)
		{
			if ( ent->client->resp.thrusting==1 )
			Jet_ApplyJet( ent, ucmd );

		if (ent->client->resp.tread == 1)
		{
			if (ent->client->resp.next_thrust_sound < level.time)
			{
				gi.sound (ent, CHAN_AUTO, gi.soundindex("bosstank/btkengn1.wav"), 1, ATTN_NORM, 0);
				ent->client->resp.next_thrust_sound=level.time+6.6;
			}
		}
  /*ATTILA end*/
  /*ATTILA begin*/
		if ( ent->client->tread==1 )
			ApplySupertankSound (ent);
  /*ATTILA end*/
		}

  if (deathmatch->value || coop->value)
  {
    if (ent->client->resp.class < 1)
    {
        ent->solid = SOLID_NOT;
        ent->movetype = MOVETYPE_NOCLIP;
        ent->svflags |= SVF_NOCLIENT;
		ent->client->newweapon = NULL;
		//ChangeWeapon (ent);
    }
  }

		pm.s = client->ps.pmove;

		StroggDM_SetClientEffects (ent, ucmd);


		for (i=0 ; i<3 ; i++)
		{
			pm.s.origin[i] = ent->s.origin[i]*8;
			pm.s.velocity[i] = ent->velocity[i]*8;
		}

		if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
		{
			pm.snapinitial = true;
	//		gi.dprintf ("pmove changed!\n");
		}

		pm.cmd = *ucmd;

		pm.trace = PM_trace;	// adds default parms
		pm.pointcontents = gi.pointcontents;

		// perform a pmove
		gi.Pmove (&pm);

		// save results of pmove
		client->ps.pmove = pm.s;
		client->old_pmove = pm.s;

		for (i=0 ; i<3 ; i++)
		{
			ent->s.origin[i] = pm.s.origin[i]*0.125;
  /*ATTILA begin*/
  if ( !ent->client->resp.thrusting==1 || (ent->client->resp.thrusting==1&&(fabs((float)pm.s.velocity[i]*0.125) < fabs(ent->velocity[i]))) )
  /*ATTILA end*/

			ent->velocity[i] = pm.s.velocity[i]*0.125;
		}

		//VectorCopy (pm.mins, ent->mins);
		//VectorCopy (pm.maxs, ent->maxs);

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

  /*ATTILA begin*/
  //if ( ent->client->resp.class == 7 )
    //if( pm.groundentity )               /*are we on ground*/
      //if ( Jet_AvoidGround(ent) )       /*then lift us if possible*/
        //pm.groundentity = NULL;         /*now we are no longer on ground*/
  /*ATTILA end*/
  /*ATTILA begin*/
  /*else */if ( ent->client->resp.class == 8 )
    if( pm.groundentity )               /*are we on ground*/
      if ( Jet_AvoidGround(ent) )       /*then lift us if possible*/
        pm.groundentity = NULL;         /*now we are no longer on ground*/
if ( ent->client->resp.class == 35 )
    if( pm.groundentity )               /*are we on ground*/
      if ( Jet_AvoidGround(ent) )       /*then lift us if possible*/
        pm.groundentity = NULL;         /*now we are no longer on ground*/
  /*else */if ( ent->client->resp.class == 32 )
    if( pm.groundentity )               /*are we on ground*/
      if ( Jet_AvoidGround(ent) )       /*then lift us if possible*/
        pm.groundentity = NULL;         /*now we are no longer on ground*/
  /*ATTILA end*/
  /*ATTILA begin*/
  else if ( ent->client->resp.class == 15 )
    if( pm.groundentity )               /*are we on ground*/
      if ( Jet_AvoidGround(ent) )       /*then lift us if possible*/
        pm.groundentity = NULL;         /*now we are no longer on ground*/
  /*ATTILA end*/
  /*ATTILA begin*/
  else if ( ent->client->resp.class == 26 )
    if( pm.groundentity )               /*are we on ground*/
      if ( Jet_AvoidGround(ent) )       /*then lift us if possible*/
        pm.groundentity = NULL;         /*now we are no longer on ground*/
  /*ATTILA end*/
  /*ATTILA begin*/
  else if ( ent->client->resp.class == 16 )
    if( pm.groundentity )               /*are we on ground*/
      if ( Jet_AvoidGround(ent) )       /*then lift us if possible*/
        pm.groundentity = NULL;         /*now we are no longer on ground*/
  /*ATTILA end*/
  /*ATTILA begin*/
  else if ( ent->client->resp.class == 11 )
    if( pm.groundentity )               /*are we on ground*/
      if ( Jet_AvoidGround(ent) )       /*then lift us if possible*/
        pm.groundentity = NULL;         /*now we are no longer on ground*/
  /*ATTILA end*/
  /*ATTILA begin*/
  else if ( ent->client->resp.class == 29 )
    if( pm.groundentity )               /*are we on ground*/
      if ( Jet_AvoidGround(ent) )       /*then lift us if possible*/
        pm.groundentity = NULL;         /*now we are no longer on ground*/
  /*ATTILA end*/

		if (ent->groundentity)
			ent->client->resp.mutant_jumped = 0;

		if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
		{
			if (ent->client->resp.class == 27)
				Cmd_Stalk_f (ent);
			//gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
			//PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
			if (ent->client->resp.class == 13)
			{
				Cmd_Catapult_f (ent);
				gi.sound(ent, CHAN_VOICE, gi.soundindex("mutant/Mutsght1.wav"), 1, ATTN_NORM, 0);
			}
		}
		if (ent->groundentity && (pm.waterlevel == 0) && (pm.cmd.upmove >= 0))
		{
			if (ent->client->resp.class == 40)
			{
				if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
				{
					ent->client->resp.nextidlesound = level.time +0.6;
				}
				if (ent->client->resp.nextidlesound < level.time)
				{
					gi.sound(ent, CHAN_AUTO, gi.soundindex ("parasite/idlesound.wav"), 1, ATTN_NORM, 0);
					ent->client->resp.nextidlesound = level.time +0.6;
				}
			}
		}
		if (!ent->groundentity)
		{
			if (ent->client->resp.class == 13)
			{
				MutantJumpAttack (ent);
			}
		}

		ent->viewheight = pm.viewheight;
		ent->waterlevel = pm.waterlevel;
		ent->watertype = pm.watertype;
		ent->groundentity = pm.groundentity;
		if (pm.groundentity)
			ent->groundentity_linkcount = pm.groundentity->linkcount;

		if (ent->deadflag)
		{
			client->ps.viewangles[ROLL] = 40;
			client->ps.viewangles[PITCH] = -15;
			client->ps.viewangles[YAW] = client->killer_yaw;
		}
		else
		{
			VectorCopy (pm.viewangles, client->v_angle);
			VectorCopy (pm.viewangles, client->ps.viewangles);
		}

//ZOID
	if (client->ctf_grapple)
		CTFGrapplePull(client->ctf_grapple);
//ZOID

		gi.linkentity (ent);

		if (ent->movetype != MOVETYPE_NOCLIP)
			G_TouchTriggers (ent);

		// touch other objects
		for (i=0 ; i<pm.numtouch ; i++)
		{
			other = pm.touchents[i];
			for (j=0 ; j<i ; j++)
				if (pm.touchents[j] == other)
					break;
			if (j != i)
				continue;	// duplicated
			if (!other->touch)
				continue;
			other->touch (other, ent, NULL, NULL);
		}

	//}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
		)
	{
		if (client->resp.spectator) {

			client->latched_buttons = 0;

			if (client->chase_target) {
				client->chase_target = NULL;
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			} else
				GetChaseTarget(ent);

		} else if (!client->weapon_thunk) {
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

  /* WonderSlug ----Code to go boom */

  if ((ent->client->kamikaze_framenum <= level.framenum) && (ent->client->kamikaze_mode & 1))

          Kamikaze_Explode(ent);

  /* WonderSlug End */

//ZOID
//regen tech
	CTFApplyRegeneration(ent);
//ZOID

   // Check to see if player pressing the "use" key
    if (ent->client->buttons & BUTTON_USE && !ent->deadflag && client->hook_frame <= level.framenum)
    {     
		Throw_Grapple (ent);     
    }
    if    (Ended_Grappling (client) && !ent->deadflag && client->hook)
    {
        Release_Grapple (client->hook);
    }

	if (client->resp.spectator) {
		if (ucmd->upmove >= 10) {
			if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) {
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
				if (client->chase_target)
					ChaseNext(ent);
				else
					GetChaseTarget(ent);
			}
		} else
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}

 // CCH: Check to see if an airstrike has arrived
 if ( client->airstrike_called && level.time > client->airstrike_time )
 {
        client->airstrike_called = 0;
        Think_Airstrike (ent);
 }

 // CCH: Check to see if an airstrike has arrived
 if ( client->lightn_called && level.time > client->lightn_time )
 {
        client->lightn_called = 0;
        Think_Lightn (ent);
 }


//ZOID
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}

	if (client->menudirty && client->menutime <= level.time) {
		PMenuDoUpdate(ent);
		gi.unicast (ent, true);
		client->menutime = level.time;
		client->menudirty = false;
	}
//ZOID
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;
//	vec3_t up;

	if (level.intermissiontime)
		return;

	client = ent->client;

	if (deathmatch->value &&
		client->pers.spectator != client->resp.spectator &&
		(level.time - client->respawn_time) >= 5) {
		spectator_respawn(ent);
		return;
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
		&& !client->resp.spectator)
//ZOID
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			// in deathmatch, only wait for attack button
			if (deathmatch->value)
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;

			if ( ( client->latched_buttons & buttonMask ) ||
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) )
			{
				if (ent->client->resp.class == 30)
				{
					BecomeExplosion1(ent);
					//gi.sound (ent, CHAN_BODY, gi.soundindex ("widow/death.wav"), 1, ATTN_NORM, 0);
					ThrowClientHead (ent);
				}
				respawn(ent);
				client->latched_buttons = 0;
			}
		}
		return;
	}

	// add player trail so monsters can follow
//	if (!deathmatch->value)
		if (!visible (ent, PlayerTrail_LastSpot() ) )
			PlayerTrail_Add (ent->s.old_origin);

	client->latched_buttons = 0;
}

// Offhand Grapple!
#define HOOK_TIME        5000    
#define HOOK_SPEED        1200    
#define THINK_TIME        0.3         
#define HOOK_DAMAGE        5         
#define GRAPPLE_REFIRE    2     
#define PULL_SPEED    500 

qboolean Ended_Grappling (gclient_t *client)
{
    return (!(client->buttons & BUTTON_USE) && client->oldbuttons & BUTTON_USE);
}

qboolean Is_Grappling (gclient_t *client)
{
    return (client->hook == NULL) ? false : true;
}

void Grapple_Touch(edict_t *hook, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    // Release if hitting its owner
    if (other == hook->owner)
        return;
    if (!Is_Grappling(hook->owner->client) && hook->health == 0) {
        return;
    }

    hook->health = 0;
    if (surf && surf->flags & SURF_SKY)
        {
            Release_Grapple(hook);
            return;
    }

    if (other != g_edicts && other->clipmask == MASK_SHOT)
        return;
    //gi.sound(hook, CHAN_ITEM, gi.soundindex("hook/hit.wav"), 1, ATTN_NORM, 0);//uncomment this line to make it play a sound         //when your hook hits a wall

   if (other != NULL) 
   {
        T_Damage(other, hook, hook->owner, hook->velocity, hook->s.origin, plane->normal, HOOK_DAMAGE, 0, 0, MOD_SUICIDE);
		gi.sound(hook, CHAN_ITEM, gi.soundindex("flyer/Flyatck1.wav"), 1, ATTN_NORM, 0);//uncomment this line to make it play a sound         //when your hook hits a wall
   }
   else
   {
	    gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (hook->s.origin);
		gi.WriteDir (plane->normal);
		gi.multicast (hook->s.origin, MULTICAST_PVS);
   }
    if (other != g_edicts && other->health && other->solid == SOLID_BBOX) {
        Release_Grapple(hook);
        return;
    }

    if (other != g_edicts && other->inuse &&
        (other->movetype == MOVETYPE_PUSH || other->movetype == MOVETYPE_STOP))
    {
        other->mynoise2 = hook;
        hook->owner->client->hook_touch = other;
        hook->enemy = other;
        hook->groundentity = NULL;
        hook->flags |= FL_TEAMSLAVE;
    }

    VectorClear(hook->velocity);
    VectorClear(hook->avelocity);
    hook->solid = SOLID_NOT;
    hook->touch = NULL;
    hook->movetype = MOVETYPE_NONE;
    hook->delay = level.time + HOOK_TIME;
    hook->owner->client->on_hook = true;
    hook->owner->groundentity = NULL;
    Pull_Grapple(hook->owner);

}

void Think_Grapple(edict_t *hook)
{
    if (level.time > hook->delay)
        hook->prethink = Release_Grapple;
    else
    {
        if (hook->owner->client->hook_touch) {
            edict_t *obj = hook->owner->client->hook_touch;

            if (obj == g_edicts)
            {
                Release_Grapple(hook);
                return;
            }

            if (obj->inuse == false) {
                Release_Grapple(hook);
                return;
            }

            if (obj->deadflag == DEAD_DEAD)
            {
                Release_Grapple(hook);
                return;
            }

            // Movement code is handled with the MOVETYPE_PUSH stuff in g_phys.c

            T_Damage(obj, hook, hook->owner, hook->velocity, hook->s.origin, vec3_origin, HOOK_DAMAGE, 0, 0, MOD_SUICIDE);
        }

        hook->nextthink += THINK_TIME;
    }
}

static void DrawBeam (edict_t *ent)
{
    gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_BFG_LASER);
    gi.WritePosition (ent->owner->s.origin);
    gi.WritePosition (ent->s.origin);
    gi.multicast (ent->s.origin, MULTICAST_PHS);
}
void Make_Hook(edict_t *ent)
{
    edict_t *hook;
    vec3_t forward, right, start, offset;

    hook = G_Spawn();
    AngleVectors(ent->client->v_angle, forward, right, NULL);
    VectorScale(forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;
    VectorSet(offset, 8, 0, ent->viewheight);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    VectorCopy(start, hook->s.origin);
    VectorCopy(forward, hook->movedir);
    vectoangles(forward, hook->s.angles);
    VectorScale(forward, HOOK_SPEED, hook->velocity);
    VectorSet(hook->avelocity, 0, 0, 500);

    hook->classname = "hook";
    hook->movetype = MOVETYPE_FLYMISSILE;
    hook->clipmask = MASK_SHOT;
    hook->solid = SOLID_BBOX;
    hook->svflags |= SVF_DEADMONSTER;
    hook->s.renderfx = RF_FULLBRIGHT;
    VectorClear (hook->mins);
    VectorClear (hook->maxs);
    hook->s.effects |= EF_COLOR_SHELL;
    hook->s.renderfx |= RF_SHELL_RED;    
    hook->s.modelindex = gi.modelindex ("models/monsters/parasite/tip/tris.md2");    
    hook->owner = ent;
    hook->touch = Grapple_Touch;
    hook->delay = level.time + HOOK_TIME;
    hook->nextthink = level.time;

    hook->prethink = DrawBeam;
    hook->think = Think_Grapple;
    hook->health = 100;
    hook->svflags = SVF_MONSTER;
    
    ent->client->hook = hook;
    gi.linkentity(hook);
}

void Throw_Grapple (edict_t *player)
{
	// Paril - Flying monsters can't.
	// Revision 8.0: Shark can. Complaining gets on my nerves, so this is
	// the answer to all the bitching.
	if (player->client->resp.landtype == 0 && player->client->resp.class != 17)
	{
		safe_cprintf (player, PRINT_HIGH, "You don't need a hook!\n");
		return;
	}

	// Paril - Nohook on? Nohook!
	if (nohook->value)
	{
		safe_cprintf (player, PRINT_HIGH, "Sorry, the hook is disabled.\n");
		return;
	}
    
    if (player->client->hook) {
        return;
    }

    gi.sound(player, CHAN_AUTO, gi.soundindex("medic/Medatck2.wav"), 0.5, ATTN_NORM, 0);

    player->client->hook_touch = NULL;
    
    Make_Hook(player);
}

void Release_Grapple (edict_t *hook)
{
    edict_t *owner = hook->owner;
    gclient_t *client = hook->owner->client;
    edict_t *link = hook->teamchain;

    client->on_hook = false;
    client->hook_touch = NULL;

    if (client->hook != NULL) {
        client->hook = NULL;
        VectorClear(client->oldvelocity);

        hook->think = NULL;

        if (hook->enemy) {
            hook->enemy->mynoise2 = NULL;
        }

        G_FreeEdict(hook);
    }
}

void Pull_Grapple (edict_t *player)
{
    vec3_t hookDir;
    vec_t length;

    VectorSubtract(player->client->hook->s.origin, player->s.origin, hookDir);
    length = VectorNormalize(hookDir);

    VectorScale(hookDir, /*player->scale * */ PULL_SPEED, player->velocity);
    VectorCopy(hookDir, player->movedir);

//To move the player off the ground just a bit so he doesn't stay stuck (version 3.17 bug)
    if (player->velocity[2] > 0) {

        vec3_t traceTo;
        trace_t trace;

        // find the point immediately above the player's origin
        VectorCopy(player->s.origin, traceTo);
        traceTo[2] += 1;

        // trace to it
        trace = gi.trace(traceTo, player->mins, player->maxs, traceTo, player, MASK_PLAYERSOLID);

        // if there isn't a solid immediately above the player
        if (!trace.startsolid) {
            player->s.origin[2] += 1;    // make sure player off ground
        }
    }

}



// constants
#define MIN_CHAIN_LEN		40		// minimum chain length
#define MAX_CHAIN_LEN		1000	// maximum chain length
#define CHAIN_LINK_LEN		55		// length between chain link origins
#define GROW_SHRINK_RATE	40		// units of lengthen/shrink chain in 0.1 sec
#define MAX_HOOKS			10		// max number of multihooks

// edict->hookstate constants
#define HOOK_ON		0x00000001		// set if hook command is active
#define HOOK_IN		0x00000002		// set if hook has attached
#define SHRINK_ON	0x00000004		// set if shrink chain is active 
#define GROW_ON		0x00000008		// set if grow chain is active



void DropHook (edict_t *ent)
{
	// remove all hook flags
	ent->owner->client->hookstate = 0;
	ent->owner->client->num_hooks = 0;

	gi.sound (ent->owner, CHAN_AUTO, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_IDLE, 0);
	
	// removes hook
	G_FreeEdict (ent);
}


void MaintainLinks (edict_t *ent)
{
	vec3_t pred_hookpos;	// predicted future hook origin
	float multiplier;		// prediction multiplier
	vec3_t norm_hookvel;	// normalized hook velocity

	vec3_t	offset, start;
	vec3_t	forward, right;

// FIXME: add this and use it to make chain not clip in players view
//	vec3_t chainvec;		// vector of the chain 
//	vec3_t chainunit; 		// vector of chain with distance of 1
//	float chainlen;			// length of chain

	// predicts hook's future position since chain links fall behind
	VectorClear (norm_hookvel);
	multiplier = VectorLength(ent->velocity) / 22;
	VectorNormalize2 (ent->velocity, norm_hookvel); 
	VectorMA (ent->s.origin, multiplier, norm_hookvel, pred_hookpos);

	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet (offset, 8, 8, ent->owner->viewheight - 8.0f);
	P_ProjectSource_Reverse (ent->owner->client, ent->owner->s.origin, offset, forward, right, start);

// FIXME: add this and use it to make chain not clip in players view
	// get info about chain
//	_VectorSubtract (pred_hookpos,start,chainvec);
//	VectorNormalize2 (chainvec, chainunit);
//	VectorMA (chainvec, -18, chainunit, chainvec);
//	chainlen = VectorLength (chainvec);

	// create temp entity chain
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (ent - g_edicts);
	gi.WritePosition (pred_hookpos);
	gi.WritePosition (start);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}


void HookBehavior(edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	vec3_t	chainvec;		// chain's vector
	float chainlen;			// length of extended chain
	vec3_t velpart;			// player's velocity component moving to or away from hook
	float f1, f2;			// restrainment forces
	float framestep;		// grow or shrink step per frame
		
	// decide when to disconnect hook
	if ( (!(ent->owner->client->hookstate & HOOK_ON)) ||// if hook has been retracted
	     (ent->enemy->solid == SOLID_NOT) ||			// if target is no longer solid (i.e. hook broke glass; exploded barrels, gibs) 
	     (ent->owner->deadflag) ||	// if player died 
		 (!ent->owner->inuse) ||	// or disconnected unexpectedly //QW//
	     (ent->owner->s.event == EV_PLAYER_TELEPORT) )	// if player goes through teleport
	{
		DropHook(ent);
		return;
	}

	// gives hook same velocity as the entity it is stuck in
	VectorCopy (ent->enemy->velocity,ent->velocity);

// chain sizing 

	// grow the length of the chain
	if ((ent->owner->client->hookstate & GROW_ON) && (ent->angle < MAX_CHAIN_LEN))
	{
		if (level.time - ent->wait > 0.1f) ent->wait = level.time - 0.1f;
		framestep = 10 * (level.time - ent->wait) * GROW_SHRINK_RATE;
		ent->angle += framestep;
		if (ent->angle > MAX_CHAIN_LEN) ent->angle = MAX_CHAIN_LEN;
		ent->wait = level.time;

		// trigger climb sound
		if (level.time - ent->delay >= 0.1f)
		{
			gi.sound (ent->owner, CHAN_AUTO, gi.soundindex("world/turbine1.wav"), 0.8f, ATTN_IDLE, 0);
			ent->delay = level.time;
		}
	}

    if ((ent->owner->client->hookstate & SHRINK_ON) && (ent->angle > MIN_CHAIN_LEN))
	{
		if (level.time - ent->wait > 0.1f) ent->wait = level.time - 0.1f;
		framestep = 10 * (level.time - ent->wait) * GROW_SHRINK_RATE;
		ent->angle -= framestep;
		if (ent->angle < MIN_CHAIN_LEN) ent->angle = MIN_CHAIN_LEN;
		ent->wait = level.time;

		// trigger slide sound
		if (level.time - ent->delay >= 0.1f)		
		{
			gi.sound (ent->owner, CHAN_AUTO, gi.soundindex("world/turbine1.wav"), 0.8f, ATTN_IDLE, 0);
			ent->delay = level.time;
		}
	}

// chain physics

	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->owner->viewheight - 8.0f);
	P_ProjectSource_Reverse (ent->owner->client, ent->owner->s.origin, offset, forward, right, start);

	// get info about chain
	_VectorSubtract (ent->s.origin, start, chainvec);
	chainlen = VectorLength (chainvec);

	// if player's location is beyond the chain's reach
	if (chainlen > ent->angle)	
	{	 
		// determine player's velocity component of chain vector
		VectorScale (chainvec, _DotProduct (ent->owner->velocity, chainvec) / _DotProduct (chainvec, chainvec), velpart);
		
		// restrainment default force 
		f2 = (chainlen - ent->angle) * 5;

		// if player's velocity heading is away from the hook
		if (_DotProduct (ent->owner->velocity, chainvec) < 0)
		{
			// if chain has streched for 25 units
			if (chainlen > ent->angle + 25)
				// remove player's velocity component moving away from hook
				_VectorSubtract(ent->owner->velocity, velpart, ent->owner->velocity);
			f1 = f2;
		}
		else  // if player's velocity heading is towards the hook
		{
			if (VectorLength (velpart) < f2)
				f1 = f2 - VectorLength (velpart);
			else		
				f1 = 0;
		}
	}
	else
		f1 = 0;
	
    // applys chain restrainment 
	VectorNormalize (chainvec);
	VectorMA (ent->owner->velocity, f1, chainvec, ent->owner->velocity);
	
	MaintainLinks (ent);

	// prep for next think
	ent->nextthink = level.time + FRAMETIME;
}


void HookTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	vec3_t	chainvec;		// chain's vector

	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->owner->viewheight - 8.0f);
	P_ProjectSource_Reverse (ent->owner->client, ent->owner->s.origin, offset, forward, right, start);

	// member angle is used to store the length of the chain
	_VectorSubtract(ent->s.origin,start,chainvec);
	ent->angle = VectorLength (chainvec);	

	// don't attach hook to sky
	if (surf && (surf->flags & SURF_SKY))
	{
		DropHook (ent);
		return;
	}

	// inflict damage on damageable items
	if (other->takedamage)
	{
		int mod;

		// Set up the means of death.
		mod = MOD_BLASTER;

		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin,
			plane->normal, ent->dmg, 100, 0, mod);
	}

	if (other->solid == SOLID_BBOX)
	{
		/*if ((other->svflags & SVF_MONSTER) || (other->client))
			gi.sound (ent, CHAN_VOICE, gi.soundindex("flyer/flyatck2.wav"), 1, ATTN_IDLE, 0);

		DropHook(ent);
		return;*/
		ent->other = other;
	}
	
	if (other->solid == SOLID_BSP)
	{
		// create puff of smoke
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SHOTGUN);
		gi.WritePosition (ent->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		gi.sound (ent, CHAN_VOICE, gi.soundindex("flyer/flyatck1.wav"), 1, ATTN_IDLE, 0);
		VectorClear (ent->avelocity);
	}
	else if (other->solid == SOLID_TRIGGER)
	{
		// debugging line; don't know if this will ever happen 
		safe_cprintf (ent->owner, PRINT_HIGH, "Hook touched a SOLID_TRIGGER\n");
	}
	
	// hook gets the same velocity as the item it attached to
	VectorCopy (other->velocity,ent->velocity);

	// flags hook as being attached to something
	ent->owner->client->hookstate |= HOOK_IN;

	ent->enemy = other;
	ent->touch = NULL;
	ent->think = HookBehavior;
	ent->nextthink = level.time + FRAMETIME;
}


void HookAirborne (edict_t *ent)
{
    vec3_t chainvec;		// chain's vector
	float chainlen;			// length of extended chain
	
	// get info about chain
	_VectorSubtract (ent->s.origin, ent->owner->s.origin, chainvec);
	chainlen = VectorLength (chainvec);
	
	if ( (!(ent->owner->client->hookstate & HOOK_ON)) || (chainlen > MAX_CHAIN_LEN) )
	{
		DropHook(ent);
		return;
	}
	
	MaintainLinks (ent);	

	ent->nextthink = level.time + FRAMETIME;
}


void FireHook (edict_t *ent)
{
	edict_t *newhook;
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;

	//QW// limit the number of multi-hooks deployed
	if (ent->client->num_hooks >= MAX_HOOKS){
		safe_cprintf(ent,PRINT_HIGH, "Too many hooks activated\n");
		return;
	}

	// determine the damage the hook will inflict
	damage = 10;
	if (ent->client->quad_framenum > level.framenum)
		damage *= 4;
	
	// derive point of hook origin
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->viewheight - 8.0f);
	P_ProjectSource_Reverse (ent->client, ent->s.origin, offset, forward, right, start);

	// spawn hook
	newhook = G_Spawn();
	VectorCopy (start, newhook->s.origin);
	VectorCopy (forward, newhook->movedir);
	vectoangles (forward, newhook->s.angles);
	VectorScale (forward, 1000, newhook->velocity);
	VectorSet(newhook->avelocity,0,0,-800);
	newhook->movetype = MOVETYPE_FLYMISSILE;
	newhook->clipmask = MASK_SHOT;
	newhook->solid = SOLID_BBOX;
	VectorClear (newhook->mins);
	VectorClear (newhook->maxs);
//	if (ctf->value)
//		newhook->s.modelindex = gi.modelindex ("models/weapons/grapple/hook/tris.md2");
//	else
//		newhook->s.modelindex = gi.modelindex ("models/weapons/grapple/hook/tris.md2");
		newhook->s.modelindex = gi.modelindex ("models/objects/debris2/tris.md2");
	newhook->owner = ent;
	newhook->dmg = damage;

	// wait used to regulate climb and slide rates; tracks time between frames 
	newhook->wait = level.time;  

	// delay used to keep track of how frequent chain noise should occur 
	newhook->delay = level.time;   
    
	// play hook launching sound
	gi.sound (ent, CHAN_AUTO, gi.soundindex ("medic/medatck2.wav"), 1, ATTN_IDLE, 0);
	
	// specify actions to follow 
	newhook->touch = HookTouch;
	newhook->think = HookAirborne;
	newhook->nextthink = level.time + FRAMETIME;
	
	gi.linkentity (newhook);
	ent->client->num_hooks++;

}


void Cmd_Hook_f (edict_t *ent)
{
	char *s;
	int	*hookstate;

	// No grappling hook when you're dead.
	if (ent->deadflag) 
		return;

	// Or when you are a flying monster
	// Paril - Flying monsters can't.
	if (ent->client->resp.landtype == 0 && ent->client->resp.class != 17)
	{
		safe_cprintf (ent, PRINT_HIGH, "You don't need a hook!\n");
		return;
	}

	// Paril - Nohook on? Nohook!
	if (nohook->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, the hook is disabled.\n");
		return;
	}

	// get the first hook argument
	s = gi.argv(1);

	// create intermediate value
	hookstate = &ent->client->hookstate;

	if ((!(*hookstate & HOOK_ON)) && (Q_stricmp(s, "action") == 0))
	{
		// flags hook as being active 
		*hookstate = HOOK_ON;   

		FireHook (ent);
		return;
	}

	if  (*hookstate & HOOK_ON)
	{
		// release hook	
		if (Q_stricmp(s, "action") == 0)
		{
			*hookstate = 0;
			return;
		}

// FIXME: put this in when I figure out where the jump key is handled
		// hop of chain and release hook when the following conditions apply
//		if (	(self.button2) && 					// jump is pressed
//				(self.flags & FL_JUMPRELEASED) &&	// previous jump cycle has finished
//				(self.hook & HOOK_IN) &&			// hook is attached
//				(!(self.flags & FL_ONGROUND)) &&	// player not on ground
//				(!(self.flags & FL_INWATER))	)	// player not in water
//		{
//			self.hook = self.hook - (self.hook & HOOK_ON);
//			self.velocity_z = self.velocity_z + 200;
//			sound (self, CHAN_BODY, "player/plyrjmp8.wav", 1, ATTN_NORM);
//			return;
//		}

		// deactivate chain growth or shrink
		if (Q_stricmp(s, "stop") == 0)
		{
			*hookstate -= *hookstate & (GROW_ON | SHRINK_ON);
			return;
		}

		// activate chain growth
		if (Q_stricmp(s, "grow") == 0)
		{
			*hookstate |= GROW_ON;
			*hookstate -= *hookstate & SHRINK_ON;
			return;
		}

		// activate chain shrinking
		if (Q_stricmp(s, "shrink") == 0)
		{
			*hookstate |= SHRINK_ON;		
			*hookstate -= *hookstate & GROW_ON;	
		}
	}
}




void ChasecamTrack (edict_t *ent);

/* The ent is the owner of the chasecam */

void ChasecamStart (edict_t *ent)
 {
 /* This creates a tempory entity we can manipulate within this
 * function */

 edict_t *chasecam;

 	if (ent->is_bot)
		return; // None! Paril
/* Tell everything that looks at the toggle that our chasecam is on
 * and working */

 ent->client->chasetoggle = 1;

 /* Make out gun model "non-existent" so it's more realistic to the
 * player using the chasecam */

 ent->client->ps.gunindex = 0;

 chasecam = G_Spawn ();
 chasecam->owner = ent;
 chasecam->solid = SOLID_NOT;

// chasecam->movetype = MOVETYPE_FLYMISSILE;
 chasecam->movetype = MOVETYPE_NOCLIP;

// chasecam->clipmask = MASK_OPAQUE;

 chasecam->chasedist1 = 20;

     // Added by WarZone - Begin
     ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION; // this turns off Quake2's inclination to predict where the camera is going,
     // making a much smoother ride
     chasecam->svflags |= SVF_NOCLIENT; // this line tells Quake2 not to send the unnecessary info about the camera to other players
     // Added by WarZone - End

 /* Now, make the angles of the player model, (!NOT THE HUMAN VIEW!) be
 * copied to the same angle of the chasecam entity */

 VectorCopy (ent->s.angles, chasecam->s.angles);

 /* Clear the size of the entity, so it DOES technically have a size,
 * but that of '0 0 0'-'0 0 0'. (xyz, xyz). mins = Minimum size,

 * maxs = Maximum size */

 VectorClear (chasecam->mins);
 VectorClear (chasecam->maxs);


 /* Make the chasecam's origin (position) be the same as the player
 * entity's because as the camera starts, it will force itself out
 * slowly backwards from the player model */

 VectorCopy (ent->s.origin, chasecam->s.origin);

 chasecam->classname = "chasecam";
 chasecam->prethink = ChasecamTrack;
// chasecam->think = ChasecamTrack;

 ent->client->chasecam = chasecam;
 ent->client->oldplayer = G_Spawn();

 //execute the routine only once...
 chasecam->prethink(chasecam);
}


void ChasecamRestart (edict_t *ent)
{
	if (ent->is_bot)
		return; // None! Paril
        /*Keep thinking this function to check all the time whether the player is out of the water */
        /* if the player is dead, the camera is not wanted... Kill me and stop the funcion */

        if (ent->owner->health <= 0)
        {
                G_FreeEdict (ent);
                return;
        }

        /* If the player is still underwater, break the routine */

        if (ent->owner->waterlevel)
                return;

        /*If the player is NOT under water, and not dead, the he is going to
        want his camera back. Create a new camera, then remove the old one
        that's not doing anything. We could quite easily 're-instate' the
        old camera, but I'm lazy :) */
        ChasecamStart (ent->owner);
        G_FreeEdict(ent);
}

void ChasecamRemove(edict_t *ent)
{
	if (ent->is_bot)
		return; // None! Paril
//        if (ent->client->missile) return;

        if (!ent->client->chasetoggle)
                return;

        VectorClear(ent->client->chasecam->velocity);

//        if (!ent->client->onturret && !ent->client->missile)
//        {
                ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
                ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
 //       }

        ent->s.modelindex = ent->client->oldplayer->s.modelindex;

//        ent->svflags &= ~SVF_NOCLIENT;

        if (ent->client->chasetoggle)
        {
                free(ent->client->oldplayer->client); //needed?
                G_FreeEdict(ent->client->oldplayer);
        }
        ent->client->chasetoggle = 0;
        G_FreeEdict(ent->client->chasecam);

}


void ChasecamTrack (edict_t *ent)
{
       
   trace_t      tr;
   vec3_t       spot1, spot2, spot3, spot4, spot5, dir;
   vec3_t       forward, right, up;
   vec3_t       size1, size2;
   float distance, tot;
//   vec3_t       owner_origin;
   
	if (ent->is_bot)
		return; // None! Paril
 
   VectorSet(size1, -4, -4, -4);
   VectorSet(size2, 4, 4, 4);

   ent->nextthink = level.time + 0.1;

   AngleVectors (ent->owner->client->v_angle, forward, right, up);

   VectorCopy(up, dir);
   VectorNegate(dir,dir);
   if (!(ent->owner->client->ps.pmove.pm_flags & PMF_DUCKED))
           VectorMA(dir, 2, forward, dir);
   else
           VectorMA(dir, 4, forward, dir);

   VectorNormalize(dir);

//spot1 is the owner origin...
   VectorCopy (ent->owner->s.origin, spot1);
//   spot1[2] += ent->owner->viewheight;

   VectorMA (spot1, -ent->chasedist1, dir, spot2);

   tr = gi.trace (spot1, size1, size2, spot2, ent->owner, MASK_OPAQUE);

   VectorCopy(tr.endpos, spot3);

   tr = gi.trace (spot1, NULL, NULL, spot2, ent->owner, MASK_OPAQUE);

   VectorCopy(tr.endpos, spot2);

   VectorSubtract(spot3, spot1, spot4);
   VectorSubtract(spot2, spot1, spot5);

   if (VectorLength(spot4) > VectorLength(spot5))
   {
//           VectorMA(spot2, 4, dir, spot2);
           VectorMA(spot2, 4, tr.plane.normal, spot2);
   }
   else
           VectorCopy(spot3, spot2);

/*
   if (tr.fraction != 1)
   {
           VectorSubtract(spot2, ent->owner->s.origin, spot1);
           ent->chasedist1 = VectorLength(spot1);
   }
*/
   tr = gi.trace (ent->s.origin, size1, size2, spot2, ent->owner,  MASK_OPAQUE);

   if ((tr.fraction != 1)|| !visible(ent, ent->owner))
   {
           VectorCopy(spot2, ent->s.origin);
           VectorClear(ent->velocity);
   }
   else
   {
           VectorSubtract(spot2, ent->s.origin, spot1);
           distance = VectorLength(spot1);
           VectorNormalize(spot1);
           VectorCopy(spot1, dir);

           tot = 0.4 * distance;

           /* if we're going too fast, make us top speed */

           if (tot > 5.2)
                VectorScale(dir, distance * 5.2, ent->velocity);
           else
           {
                 /* if we're NOT going top speed, but we're going faster than
                  * 1, relative to the total, make us as fast as we're going */

                 if (tot > 1)
                        VectorScale(dir, distance * tot, ent->velocity);
                 else
                 {
                         /* if we're not going faster than one, don't accelerate our
                          * speed at all, make us go slow to our destination */
                         VectorScale(dir, distance, ent->velocity);
                 }
           }

           /* subtract endpos;player position, from chasecam position to get
            * a length to determine whether we should accelerate faster from
            * the player or not */

           if (distance < 20)
                VectorScale (ent->velocity, 2, ent->velocity);

   }

   /* add to the distance between the player and the camera */
   ent->chasedist1 += 2;

   /* if we're too far away, give us a maximum distance */
   if (ent->chasedist1 > ent->owner->client->cammaxdistance)
        ent->chasedist1 = ent->owner->client->cammaxdistance;

}

void Cmd_Chasecam_Toggle (edict_t *ent)
{
	if (ent->is_bot)
		return; // None! Paril
        if (ent->movetype == MOVETYPE_NOCLIP)
        {
                safe_cprintf (ent, PRINT_HIGH, "You must join the game before using the Chasecam.\n");
                return;
        }

/*        if (cam_force->value)
        {
                safe_cprintf (ent, PRINT_HIGH, "Sorry, you must play with the chasecam (to disable this, set 'cam_force' to 0).\n");
                return;
        }*/

        if (!ent->deadflag || ent->client->playing_dead)
        {
                if (ent->client->chasetoggle)
                        ChasecamRemove(ent);
                else
                        ChasecamStart(ent);
        }
}

void CheckChasecam_Viewent(edict_t *ent)
{
        gclient_t *cl;

		if (ent->is_bot)
			return; // None! Paril
        if (!ent->client->oldplayer->client)
        {
                cl = (gclient_t *)malloc(sizeof(gclient_t));
                ent->client->oldplayer->client = cl;
        }
        if /*(*/(ent->client->chasetoggle == 1/* || ent->client->missile)*/&&(ent->client->oldplayer))
        {
                ent->client->oldplayer->s.frame = ent->s.frame;
                VectorCopy(ent->s.origin, ent->client->oldplayer->s.origin);
                VectorCopy(ent->velocity, ent->client->oldplayer->velocity);
                VectorCopy(ent->s.angles, ent->client->oldplayer->s.angles);

                ent->client->oldplayer->s = ent->s;
                ent->client->oldplayer->mass = ent->mass;
                if (ent->svflags & SVF_NOCLIENT)
                        ent->client->oldplayer->svflags |= SVF_NOCLIENT;
                else
                        ent->client->oldplayer->svflags &= ~SVF_NOCLIENT;

                gi.linkentity(ent->client->oldplayer);
        }

}

void Cmd_CamMaxDistance (edict_t *ent)
{
		char		*name;
        int                     num;

	if (ent->is_bot)
		return; // None! Paril

        if (!strlen(gi.args()))
                {
                safe_cprintf (ent, PRINT_HIGH, "The actual cam_maxdistance is: %d\n", ent->client->cammaxdistance);
                return;
        }

	name = gi.args();

        num = atoi(name);

        if (num > 0 && num < 500)
        {
                ent->client->cammaxdistance = num;
                safe_cprintf (ent, PRINT_HIGH, "cam_maxdistance set to %d\n", ent->client->cammaxdistance);
        }
}

// Node stuff









#define INVALID -1

FILE *fp;

#define CONTENTS_NODE 0x00

typedef struct {
int nodenum; // node[]
int type; // type of node this is (pointcontents)
vec3_t origin; // location in [x,y,z]
} node_t;

node_t *node;

int numnodes;

// Q2 maps are -4096,+4096 units along each axis
// 8192/256 = 32 units between nodes
// 8192/128 = 64 units between nodes
// 8192/64 =128 units between nodes.

// NOTE: you can vary these number to 128, 64, etc
// to change the granularity of your slices of the world.

// HINT: Keep the zsize 256 else the nodes will be 
// too high off the floor... (Your choice)

#define xsize 256 // 32 units
#define ysize 256 // 32 units
#define zsize 256 // 32 units

int grid[xsize][ysize][zsize]; // Temp storage..

#define gridpt grid[x][y][z]

float xevery = 8192/xsize;
float yevery = 8192/ysize;
float zevery = 8192/zsize;

#define MaxOf(x,y) ((x) > (y)?(x):(y))
#define MinOf(x,y) ((x) < (y)?(x):(y))


void *xmalloc(size_t *size) 
{
	void *mem;

  mem = malloc((size_t) size);
  if (!mem) exit(1);
  return mem;
}

#define xmalloc malloc
 


//==================================================
// Converts a q2 vector into a position in the grid
//==================================================
void vec2grid(vec3_t in, int *x, int *y, int *z) {
*x=(int)MinOf(MaxOf((in[0]+4096)/xevery,0),xsize-1);
*y=(int)MinOf(MaxOf((in[1]+4096)/yevery,0),ysize-1);
*z=(int)MinOf(MaxOf((in[2]+4096)/zevery,0),zsize-1);
}

//==================================================
// Converts a position in the grid into a q2 vector
//==================================================
void grid2vec(int x, int y, int z, vec3_t out) {
out[0]=(float)MinOf(MaxOf(x*xevery-4096+(xevery/2),-4096),4096);
out[1]=(float)MinOf(MaxOf(y*yevery-4096+(yevery/2),-4096),4096);
out[2]=(float)MinOf(MaxOf(z*zevery-4096+(zevery/2),-4096),4096);
}

//===========================================================
void WriteGrid(void) {
int i=0,x,y,z;
char mappath[64];

fprintf(stderr,"Saving Node File...");

sprintf(mappath, "stroggdm/maps/%s.grd",level.mapname);
fp=fopen(mappath,"wb");
if (!fp) {
fprintf(stderr,"ERROR WRITING: %s\n",mappath);
return; }

// How many valid nodes do we have?
numnodes=0;
for (z=0; z < zsize; z++)
for (x=0; x < xsize; x++)
for (y=0; y < ysize; y++)
if (gridpt != INVALID)
numnodes++;

// Write to file the number of nodes
fwrite(&numnodes,sizeof(int),1,fp);

// Allocate a single node for copying purposes
node=(node_t *)xmalloc(sizeof(node_t));

// Assign valid nodes to node_t and write to file
for (z=0; z < zsize; z++)
for (x=0; x < xsize; x++)
for (y=0; y < ysize; y++)
if (gridpt != INVALID) {
node->type = gridpt; // pointcontents
node->nodenum = i++; // nodenum
grid2vec(x,y,z,node->origin); // node origin
fwrite(node,sizeof(node_t),1,fp); }

free(node);

fclose(fp);

fprintf(stderr,"Done\n%d Nodes written to %s\n",numnodes,mappath);
}

//===========================================================
void ReadGrid(void) {
int i;
char mappath[64];

sprintf(mappath, "c:/quake2/baseq2/maps/%s.grd",level.mapname);
fp=fopen(mappath,"rb");
if (!fp) {
fprintf(stderr,"ERROR READING: %s\n",mappath);
return; }

// How many nodes are in this file?
fread(&numnodes,sizeof(int),1,fp);

// Allocate ALL the node_t structs we'll need
node=(node_t *)xmalloc(numnodes*sizeof(node_t));

// Read these nodes from file..
for (i=0;i < numnodes;i++)
fread(&node[i],sizeof(node_t),1,fp);

fclose(fp);

fprintf(stderr,"%d Nodes Loaded\n",numnodes);
}


//========================================================
// Initialize all the cells in grid and delete any extras
//========================================================
void CreateGrid(void) {
	int i,x,y,z;
	int haslava=0;
	vec3_t origin;
	
	numnodes=xsize*ysize*zsize;
	
	memset(&grid,0,xsize*ysize*zsize*sizeof(int));
	
	fprintf(stderr, "Wait.. Partitioning map into %d horizontal planes\n",zsize);
	
	// Here, I'm getting the point contents of EVERY node in the grid...
	
	for (x=0; x < xsize; x++)
		for (y=0; y < ysize; y++)
			for (z=0; z < zsize; z++) {
				grid2vec(x,y,z,origin);
				gridpt=gi.pointcontents(origin);
				if (!haslava && gridpt & (CONTENTS_LAVA|CONTENTS_SLIME))
					haslava=1; }
			
			fprintf(stderr,"Deleting nodes in Lava/Slime...");
			
			// If map doesn't have lava nodes then we can skip this part..
			
			if (haslava)
				for (z=0; z < zsize; z++)
					for (x=0; x < xsize; x++)
						for (y=0; y < ysize; y++)
							for (i=z; i < zsize; i++)
								if (grid[x][y][i] & (CONTENTS_LAVA|CONTENTS_SLIME))
									while ((i < zsize) && !(grid[x][y][i] & (CONTENTS_SOLID|CONTENTS_LADDER)))
										grid[x][y][i++] = INVALID;
									
									fprintf(stderr,"Done\n");
									
									fprintf(stderr,"Deleting Nodes in mid-air...");
									
									// What I'm trying to do here is to delete any mid-air nodes (CONTENTS_NONE)
									// while leaving the very first node directly above the ground. I'm also
									// trying to leave ladder nodes intact. 
									
									for (z=0; z < zsize; z++)
										for (x=0; x < xsize; x++)
											for (y=0; y < ysize; y++)
												for (i=z; i < zsize; i++)
													if (grid[x][y][i] == 0 || grid[x][y][i] & CONTENTS_WATER) {
														while ((++i < zsize) && !(grid[x][y][i] & (CONTENTS_SOLID|CONTENTS_LADDER)))
															grid[x][y][i] = INVALID;
														
														
														fprintf(stderr,"Done\n");
														
														fprintf(stderr,"Deleting Nodes in Solids...");
														
														
														// Lastly, I want to step thru all the nodes in the grid and remove any of those
														// nodes which are in solid but NOT ladder nodes.
														
														//============================
														// Now delete ALL solid nodes
														//============================
														for (z=0; z < zsize; z++)
															for (x=0; x < xsize; x++)
																for (y=0; y < ysize; y++)
																	if (gridpt & CONTENTS_SOLID && !(gridpt & CONTENTS_LADDER))
																		gridpt = INVALID;
																	
																	fprintf(stderr,"Done\n\n");
																	
																	// In the end, I have a layer of nodes covering ALL horizontal surfaces in the 
																	// map with each node being zevery units above the floor or ground (any solid) 
																	// while keeping ladder nodes intact... Understand that?
																	
																	WriteGrid();
													}
													
													
}

#define but else if


void Bot_Spawn (edict_t *ent);	// Define it
void Bot_Create_AssignTeam (void);
void Bot_Create_CTF (int team);
void Bot_Create (void);			// Needs header file!
void Bot_Attack(edict_t *ent, usercmd_t *cmd, vec3_t angles);
void Bot_Aim(edict_t *ent, edict_t *target, vec3_t angles);
void Bot_Pain(edict_t *ent, edict_t *other, float kickback, int damage);
void Bot_Attack(edict_t *ent, usercmd_t *cmd, vec3_t angles);
void Bot_Move(edict_t *ent, edict_t *goal, usercmd_t *cmd, vec3_t angles);
edict_t *Bot_FindEnemy(edict_t *ent);
qboolean ACEIT_CanUseArmor (gitem_t *item, edict_t *other);
float ACEIT_ItemNeed(edict_t *self, int item);
int ACEIT_ClassnameToIndex(char *classname);
qboolean ACEIT_IsReachable(edict_t *self, vec3_t goal);
void ACEAI_PickShortRangeGoal(edict_t *self, usercmd_t *cmd);
qboolean Bot_CanMove (edict_t *self);
void Bot_ChangeWeapon_DontUseBlaster (edict_t *ent);
void Bot_ChangeWeapon_Accordingly (edict_t *ent, int change_event);

qboolean ClientConnect (edict_t *ent, char *userinfo); // Define aswell
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles); // Define
void ClientThink (edict_t *ent, usercmd_t *ucmd);
void CopyToBodyQue (edict_t *ent);
void ClientDisconnect (edict_t *ent);
void Svcmd_Bot_f(void);

#define WEAP_MISSILE 0
#define WEAP_INSTANT 1

void Svcmd_Bot_f(void)
{
	char *arg = gi.argv(2); // arg is the third char after "sv bot"

	// Disconnect a bot this way.
	// Disconnects the bot you named.
	if (Q_stricmp(arg, "kill") == 0)
	{
		int i;
		char *name = gi.argv(3);
		edict_t *bot; 

		for (i = maxclients->value; i > 0; i--)
		{
			bot = g_edicts + i + 1; 

			if (Q_stricmp(name, bot->client->pers.netname))
				break;

			if (bot->client && (Q_stricmp("bot", bot->classname) == 0) && (Q_stricmp(name, bot->client->pers.netname) == 0))
			{
				ClientDisconnect(bot);
				break;
			}
		}
	}
	else if (Q_stricmp(arg, "spawn") == 0) // If it was spawn..
	{
		int team = atoi(gi.argv(3));

		Bot_Create(); // Voo.
	}
	// Disconnect a bot this way.
	// Disconnects the bot you named.
}

// Skins
// Male
#define SKIN_CIPHER				"cipher"
#define SKIN_CLAYMORE			"claymore"
#define SKIN_DISGUISE			"male/disguise"
#define SKIN_FLAK				"flak"
#define SKIN_GRUNT				"grunt"
#define SKIN_HOWITZER			"howitzer"
#define SKIN_MAJOR				"major"
#define SKIN_NIGHTOPS			"nightops"
#define SKIN_POINTMAN			"pointman"
#define SKIN_PHYCHO				"phycho"
#define SKIN_RAMPAGE			"rampage"
#define SKIN_RAZOR				"razor"
#define SKIN_RECON				"recon"
#define SKIN_ROGUEB				"rogue_b"
#define SKIN_ROGUER				"rogue_r"
#define SKIN_SCOUT				"scout"
#define SKIN_SNIPER				"sniper"
#define SKIN_VIPER				"viper"
// Female
#define SKIN_ATHENA				"athena"
#define SKIN_BRIANNA			"brianna"
#define SKIN_COBALT				"cobalt"
#define SKIN_DISGUISEF			"disguise"
#define SKIN_ENSIGN				"ensign"
#define SKIN_JEZEBEL			"jezebel"
#define SKIN_JUNGLE				"jungle"
#define SKIN_LOTUS				"lotus"
#define SKIN_ROGUEBF			"rogue_b"
#define SKIN_ROGUERF			"rogue_r"
#define SKIN_STILETTO			"stiletto"
#define SKIN_VENUS				"venus"
#define SKIN_VOODOO				"voodoo"

// Cyborg
// Skins
#define SKIN_ONI911				"oni911"
#define SKIN_PS9000				"ps9000"
#define SKIN_TYR574				"tyr574"

const char *c_szNames[] = { "Parilbot", "Grievebot", "Wolf", "God", "Satan", "Chuck Norris",
							"Will", "Haxxo", "SnIpEr", "Hajaz", "Chris", "Santa", 
							"Asploder", "Whoa", "Willium", "Sephy", "TheOwner", "l337", "Mike",
							"Radud", "EViL" };


const char *c_szClans[] = { "[OPFD]", "(GGFG)", "[PDP]", "{WRTC}", "[ELO]", 
							"_MNN_", "~TILL~", "_MUL_", "-KLD-", "__CL__", 
							"(-SIX-)", "[HL]", "" };


const char *gender[] = { "male", "female", "cyborg" };

const char *male_skins[] = { SKIN_CIPHER, SKIN_CLAYMORE, SKIN_DISGUISE, 
							 SKIN_FLAK, SKIN_GRUNT, SKIN_HOWITZER,
							 SKIN_MAJOR, SKIN_NIGHTOPS, SKIN_POINTMAN,
							 SKIN_PHYCHO, SKIN_RAMPAGE, SKIN_RAZOR, SKIN_RECON,
							 SKIN_ROGUEB, SKIN_ROGUER, SKIN_SCOUT, SKIN_SNIPER, 
							 SKIN_VIPER };

const char *female_skins[] = { SKIN_ATHENA, SKIN_BRIANNA, SKIN_COBALT, 
							 SKIN_DISGUISEF, SKIN_ENSIGN, SKIN_JEZEBEL,
							 SKIN_JUNGLE, SKIN_LOTUS, SKIN_ROGUEBF,
							 SKIN_ROGUERF, SKIN_STILETTO, SKIN_VENUS, 
							 SKIN_VOODOO };

const char *cyborg_skins[] = { SKIN_ONI911, SKIN_PS9000, SKIN_TYR574 };

unsigned int nNameCount = sizeof( c_szNames ) / sizeof( c_szNames[0] );
unsigned int nClanCount = sizeof( c_szClans ) / sizeof( c_szClans[0] );

unsigned int gendercount = sizeof( gender ) / sizeof( gender[0] );

unsigned int male_skinscount = sizeof( male_skins ) / sizeof( male_skins[0] );
unsigned int female_skinscount = sizeof( female_skins ) / sizeof( female_skins[0] );
unsigned int cyborg_skinscount = sizeof( cyborg_skins ) / sizeof( cyborg_skins[0] );



void Bot_Create (void)
{
	int i;
	char userinfo[MAX_INFO_STRING];
	edict_t *bot; 
	float r;
	char *name;

	char *gender_;
	char *skin_;

	int nName = ( ( float ) rand( ) ) / RAND_MAX * nNameCount;
	int nClan = ( ( float ) rand( ) ) / RAND_MAX *  nClanCount;

	int gender_n = ( ( float ) rand( ) ) / RAND_MAX * gendercount;

	int malesk_n = ( ( float ) rand( ) ) / RAND_MAX *  male_skinscount;
	int femalesk_n = ( ( float ) rand( ) ) / RAND_MAX *  female_skinscount;
	int cyborgsk_n = ( ( float ) rand( ) ) / RAND_MAX *  cyborg_skinscount;
	int skin;

	switch( gender_n )
	{
		case 0:
			skin = ( ( float ) rand( ) ) / RAND_MAX *  male_skinscount;

			gender_ = "male";

			skin_ = malloc(33);
			sprintf(skin_, "%s/%s", gender_, male_skins [malesk_n]);

			break;

		case 1:
			skin = ( ( float ) rand( ) ) / RAND_MAX *  female_skinscount;

			gender_ = "female";

			skin_ = malloc(33);
			sprintf(skin_, "%s/%s", gender_, female_skins [femalesk_n]);

			break;

		case 2:
			skin = ( ( float ) rand( ) ) / RAND_MAX *  cyborg_skinscount;

			gender_ = "cyborg";

			skin_ = malloc(33);
			sprintf(skin_, "%s/%s", gender_, cyborg_skins [cyborgsk_n]);

			break;
	}

	r = random();

	for (i = maxclients->value; i > 0; i--)
	{
		bot = g_edicts + i + 1;
		if (!bot->inuse)
			break; // Check to see if he can be put in.
	} 

	if (bot->inuse)
		bot = NULL; // Make sure he is not inuse if there is no place to put him
					// Not enough players, then.

	if (bot)
	{
		memset(userinfo, 0, MAX_INFO_STRING); 

/*		// Start name
		if (r < 0.1)
			name = "Will";
		else if (r < 0.2)
			name = "Haxxor";
		else if (r < 0.3)
			name = "[666]Jullie";
		else if (r < 0.4)
			name = "W|-|ee";
		else if (r < 0.5)
			name = "[RUN]Whetev";
		else if (r < 0.6)
			name = "ParilBot";
		else if (r < 0.7)
			name = "GrieveBot";
		else if (r < 0.8)
			name = "Pix";
		else if (r < 0.9)
			name = "Harold";
		else
			name = "Name";
		// End names*/

		name = malloc(33);
		sprintf(name, "%s%s", c_szClans [nClan], c_szNames [nName]);
		Info_SetValueForKey(userinfo, "name", name); // Random names
		free(name);


		Info_SetValueForKey(userinfo, "skin", skin_);
		free(skin_);

		// Randomize hand.. full client emulation, you know!
		if (r <= 0.3)
			Info_SetValueForKey(userinfo, "hand", "2");
		else if (r <= 0.5)
			Info_SetValueForKey(userinfo, "hand", "0");
		else
			Info_SetValueForKey(userinfo, "hand", "1");

		ClientConnect(bot, userinfo);	// Connect the client
		G_InitEdict(bot);				// Initalize him as an edict
		InitClientResp(bot->client);	// Get his respawn information

		Bot_Spawn(bot);					// Spawn him

		gi.WriteByte(svc_muzzleflash);	// Muzzle flash
		gi.WriteShort(bot - g_edicts);
		gi.WriteByte(MZ_LOGIN);
		gi.multicast(bot->s.origin, MULTICAST_PVS); 
		
		// Print
		safe_bprintf(PRINT_HIGH, "%s entered the game\n", bot->client->pers.netname);
		ClientEndServerFrame(bot); // End his server frame (See the function)


	// Say a random message
		r = rndnum (0, 8);
		if (r < 3)
		{
			char *msg;

			if (random() < 0.1)
				msg = "Hey everyone.";
			else if (random() < 0.2)
				msg = "I'm gonna own you all.";
			else if (random() < 0.3)
				msg = "LOL lamezer.";
			else if (random() < 0.4)
				msg = "Time to kick ass and chew bubblegum.. and I'm all out of gum..";
			else if (random() < 0.5)
				msg = "How's everybody?";
			else if (random() < 0.7)
				msg = "Hello";
			else if (random() < 0.8)
				msg = "Sup?";
			else
				msg = "Yo.";

			safe_bprintf (PRINT_CHAT, "%s: %s\n", bot->client->pers.netname, msg);
		}
	}
	else
		gi.dprintf("Bot cannot connect - server is full!\n"); // Can't connect
}

void Bot_MoveToGoal(edict_t *ent, usercmd_t *cmd)
{
	vec3_t angles = { 0, 0, 0 }; 

	VectorCopy(ent->client->v_angle, angles);
	cmd->forwardmove = 200;
}


void Bot_Think(edict_t *ent)
{
	usercmd_t cmd;
	vec3_t angles = { 0, 0, 0 }; 

	VectorCopy(ent->client->v_angle, angles);
	VectorSet(ent->client->ps.pmove.delta_angles, 0, 0, 0);
	memset(&cmd, 0, sizeof(usercmd_t));  // Get his angles and cmd stuff (nothing to do with commands)

	if (ent->deadflag == DEAD_DEAD)
	{
		ent->client->buttons = 0;
		cmd.buttons = BUTTON_ATTACK; // If dead, press the button automatically
	} 

	else if (ent->enemy)
	{
		if (!visible(ent, ent->enemy))
			ent->enemy = NULL;
		Bot_Attack(ent, &cmd, angles); // Attack enemy!
		ent->s.angles[0] = 90;
	}
	else if (ent->movetarget)
	{
		if (ent->movetarget->health < 1)
			ent->movetarget = NULL;
		ent->s.angles[0] = 90;
		Bot_Move (ent, ent->movetarget, &cmd, angles);
	}
	else
	{
		cmd.forwardmove = 200;
		ent->enemy = Bot_FindEnemy(ent);
	}

	cmd.msec = rndnum(70,180); // Ping

	cmd.angles[PITCH] = ANGLE2SHORT(angles[PITCH]);
	cmd.angles[YAW] = ANGLE2SHORT(angles[YAW]);
	cmd.angles[ROLL] = ANGLE2SHORT(angles[ROLL]);  // Angles

	ClientThink(ent, &cmd);  // Use ClientThink. Now, his frames are nice.

	//Bot_ChangeWeapon_DontUseBlaster (ent);

	if (ent->health > 0)
		ACEAI_PickShortRangeGoal (ent, &cmd);




		if (ent->wait > level.time)
		{
			//cmd->upmove = -10;
			ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
			//gi.dprintf ("%i\n", ent->client->ps.pmove.pm_flags);
		}
		else
		{
			//gi.dprintf ("Not Crouching");
			ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
		}

		if (!ent->enemy)
		{
			// A different approach.
			if (ent->client->resp.nextturn < level.time)
			{
				int r = rndnum(0, 3);
				int best_angle;
				// We hit something.. change our angles
				// [Start Bad Way To Calculate Angles]
				// FIXME: Do traces in each direction detecting which way
				// is best to go!
				if (r == 0)
					best_angle = 90;
				else if (r == 1)
					best_angle = 180;
				else if (r == 2)
					best_angle = 270;
				else
					best_angle = 90; // 90 is best choice
				// [End Bad Way To Calculate Angles]
				ent->s.angles[1] += best_angle;
				VectorCopy(ent->s.angles, ent->client->ps.viewangles);
				VectorCopy(ent->s.angles, ent->client->v_angle);  // Angles

				//gi.bprintf (PRINT_HIGH, "Called ent->client->resp.nextturn thing, best angle is %i", best_angle);
				ent->client->resp.nextturn = level.time + 4;
			}
		}


	ent->nextthink = level.time + FRAMETIME; // Do this every frame
}

void Bot_Spawn(edict_t *ent)
{
	vec3_t origin, angles;
	vec3_t mins = {-16, -16, -24};
	vec3_t maxs = {16, 16, 32};
	int i, index;
	client_persistant_t pers;
	client_respawn_t resp; 
	int r = rndnum (0, 8);
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;

	if (!deathmatch->value)
	{
		gi.dprintf("Bots only work in Deathmatch!\n");
		return; // Note that we will never get here, CTF forces deathmatch, but just incase.
	} 

	// Put this here
	ent->client->resp.class = rndnum (1, 37);

	if (ctf->value && !ent->client->resp.ctf_team)
		CTFAssignTeam(ent->client);

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if (ent->client->resp.class == 17)
	{
/*--------------------------------*/
/*     new code starts here       */
/*--------------------------------*/
        VectorCopy (mins, ent->mins);
        VectorCopy (maxs, ent->maxs);
        if (findspawnpoint(ent)) // new line
        {
               VectorCopy(ent->s.origin, spawn_origin);
               VectorClear (spawn_angles);
               spawn_angles[YAW] = rand() % 360 - 180; // face a random direction
        }     
        else // couldn't find a good spot, so...
               SelectSpawnPoint (ent, spawn_origin, spawn_angles);
	}
/*--------------------------------*/
/*     new code ends here         */
/*--------------------------------*/

	SelectSpawnPoint(ent, origin, angles);  // Get his spawn point

	index = ent - g_edicts - 1; 
	client = ent->client;

	if (deathmatch->value)
	{
		char userinfo[MAX_INFO_STRING]; 

		resp = ent->client->resp;
		memcpy(userinfo, ent->client->pers.userinfo, MAX_INFO_STRING);
		InitClientPersistant(ent->client); // Get the persistant stuff, happens on respawn
		ClientUserinfoChanged(ent, userinfo); // Make sure his information is correct
	}
	else
		memset(&resp, 0, sizeof(client_respawn_t));

	pers = ent->client->pers;
	memset(ent->client, 0, sizeof(gclient_t));
	ent->client->pers = pers;
	ent->client->resp = resp; 

	FetchClientEntData(ent);  //Get ent data

	if (ent->client->resp.class == 1)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 36)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 2)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 3)
	{
		VectorSet (ent->mins, -32, -32, -16);
		VectorSet (ent->maxs, 32, 32, 72);
	}
	else if (ent->client->resp.class == 33)
	{
		VectorSet (ent->mins, -32, -32, -16);
		VectorSet (ent->maxs, 32, 32, 72);
	}
	else if (ent->client->resp.class == 4)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 28)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 5)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 6)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 34)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 7)
	{
		VectorSet (ent->mins, -32, -32, -24);
		VectorSet (ent->maxs, 32, 32, 64);
	}
	else if (ent->client->resp.class == 39)
	{
		VectorSet (ent->mins, -32, -32, -24);
		VectorSet (ent->maxs, 32, 32, 64);
	}
	else if (ent->client->resp.class == 8)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 16); // 24, too big
	}
	else if (ent->client->resp.class == 35)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 16); // 24, too big
	}
	else if (ent->client->resp.class == 32)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 9)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 10)
	{
		VectorSet (ent->mins, -16, -16, 0);
		VectorSet (ent->maxs, 16, 16, 56);
	}
	else if (ent->client->resp.class == 11)
	{
		VectorSet (ent->mins, -56, -56, 0);
		VectorSet (ent->maxs, 56, 56, 80);
	}
	else if (ent->client->resp.class == 29)
	{
		VectorSet (ent->mins, -56, -56, 0);
		VectorSet (ent->maxs, 56, 56, 80);
	}
	else if (ent->client->resp.class == 12)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 13)
	{
		VectorSet (ent->mins, -32, -32, -24);
		VectorSet (ent->maxs, 32, 32, 48);
	}
	else if (ent->client->resp.class == 14)
	{
		VectorSet (ent->mins, -32, -32, -16);
		VectorSet (ent->maxs, 32, 32, 72);
	}
	else if (ent->client->resp.class == 41)
	{
		VectorSet (ent->mins, -32, -32, -16);
		VectorSet (ent->maxs, 32, 32, 72);
	}
	else if (ent->client->resp.class == 15)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 26)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 16)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 17)
	{
		VectorSet (ent->mins, -16, -16, 0);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 18)
	{
		VectorSet (ent->mins, -80, -80, 0);
		VectorSet (ent->maxs, 80, 80, 140);
	}
	else if (ent->client->resp.class == 31)
	{
		VectorSet (ent->mins, -80, -80, 0);
		VectorSet (ent->maxs, 80, 80, 140);
	}
	else if (ent->client->resp.class == 19)
	{
		VectorSet (ent->mins, -30, -30, 0);
		VectorSet (ent->maxs, 30, 30, 90);
	}
	else if (ent->client->resp.class == 30)
	{
		VectorSet (ent->mins, -30, -30, 0);
		VectorSet (ent->maxs, 30, 30, 140);
	}
	else if (ent->client->resp.class == 20)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 21)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 22)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	}
	else if (ent->client->resp.class == 23)
	{
		VectorSet (ent->mins, -16, -16, 0);
		VectorSet (ent->maxs, 16, 16, 56);
	}
	else if (ent->client->resp.class == 24)
	{
		VectorSet (ent->mins, -64, -64, 0);
		VectorSet (ent->maxs, 64, 64, 112);
	}
	else if (ent->client->resp.class == 25)
	{
		VectorSet (ent->mins, -32, -32, -24);
		VectorSet (ent->maxs, 32, 32, 48);
	}
	else if (ent->client->resp.class == 26)
	{
		VectorSet (ent->mins, -24, -24, -24);
		VectorSet (ent->maxs, 24, 24, 32);
	}
	else if (ent->client->resp.class == 27)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 16);
	}
	else if (ent->client->resp.class == 37)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	} 
	else if (ent->client->resp.class == 38)
	{
		VectorSet (ent->mins, -16, -16, -24);
		VectorSet (ent->maxs, 16, 16, 32);
	} 

	// Main spawn functions start
	ent->groundentity = NULL;				// Not on ground yet
	ent->client = &game.clients[index];		// He's a client
	ent->takedamage = DAMAGE_AIM;			// Damage type, damage_aim is best for clients (prediction)
	ent->movetype = MOVETYPE_WALK;			// He walks, yes.
	ent->viewheight = 22;					// 22 viewheight, for weapons.
	ent->inuse = true;						// He is in use
	ent->classname = "bot";					// bot is his classname
	ent->mass = 200;						// 200 mass, same as clients
	ent->solid = SOLID_BBOX;				// A solid bbox
	ent->deadflag = DEAD_NO;				// Not dead
	ent->air_finished = level.time + 12;	// How long for drowning
	ent->clipmask = MASK_PLAYERSOLID;		// He's a player solid (things that hits players will hit him)
	ent->think = Bot_Think;						// Think like a bot
	ent->touch = NULL;						// No touch yet (handled elsewhere)
	ent->pain = Bot_Pain;					// New pain
	ent->die = player_die;					// Uses player die
	ent->waterlevel = 0;					// Not in water
	ent->watertype = 0;						// Ditto
	ent->flags &= ~FL_NO_KNOCKBACK;			// Remove no knockback flag
	ent->enemy = NULL;						// No enemy (we'll use this to fight)
	ent->movetarget = NULL;					// No target to move to (nodes?)
	ent->is_bot = true;						// He is a bot
	// Main spawn functions end

	ent->client->resp.nextfootstep=0;
	ent->ismonster = 0;
	ent->client->pers.stalker_ceiling = 0;
	ent->client->pers.abilities.powerpoints = 301;
	ent->s.effects |= 0;
	ent->s.renderfx |= RF_IR_VISIBLE;
	ent->client->pers.abilities.brain_proxies = 0;
	if (!ent->is_bot)
	{
		gi.cvar_forceset("gl_saturatelighting","0"); 
		gi.cvar_forceset("r_fullbright","0"); 
	}
	ent->team_owner = ent;
	//ent->client->ps.rdflags |= RDF_IRGOGGLES;

        //Wyrm: chasecam
        ent->svflags &= ~SVF_NOCLIENT;
        ent->owner = NULL;
        ent->chasedist2 = ent->client->cammaxdistance = 90;

	// Abilities
	ent->client->pers.abilities.soldier_haste = 0;
	ent->client->pers.abilities.tank_dualrockets = 0;
	ent->client->pers.abilities.gunner_forcegrenades = 0;
	ent->client->pers.abilities.gladiator_wallpierce = 0;
	ent->client->pers.abilities.floater_fragmulti = 0;
	ent->client->pers.abilities.berserk_shield = 0;
	ent->client->pers.abilities.chick_triplespread = 0;
	ent->client->pers.abilities.icarus_explobolts = 0;
	ent->client->resp.spawned_monsters = 0;
	ent->client->pers.selected = NULL;
	ent->s.renderfx |= RF_FRAMELERP;



	if (ent->client->resp.bloody) // Check for blood on respawn
	{
//		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));

		ClientUserinfoChanged (ent, userinfo);
	}

	if (!ent->is_bot)
		StuffSpeedByClass( ent );
	ent->client->resp.class_speed = AssignClassSpeed( ent );

	//VectorCopy(mins, ent->mins);
	//VectorCopy(maxs, ent->maxs);
	VectorClear(ent->velocity);  // Get his bbox and velocity

	memset(&ent->client->ps, 0, sizeof(player_state_t)); // Set this stuff

	for (i = 0; i < 3; i++)
	{
		ent->client->ps.pmove.origin[i] = origin[i] * 8;
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(angles[i] - ent->client->resp.cmd_angles[i]);
	}  
	// ^^^^ Pmove stuff

	ent->client->ps.fov = 90; // FOV is 90 (not sure why we would need this, just incase..)
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model); // Gunindex (1st person)

	ent->s.effects = 0;			// No effects
	ent->s.skinnum = index;		// Number of client (I think?)
	ent->s.modelindex = 255;	// Model
	ent->s.modelindex2 = 255;	// Vwep
	ent->s.frame = 0;			// Frame is 0

	VectorCopy(origin, ent->s.origin); // Origin
	ent->s.origin[2]++; 

	ent->s.angles[PITCH] = 0; // Angles
	ent->s.angles[YAW] = angles[YAW];
	ent->s.angles[ROLL] = 0; 

	VectorCopy(ent->s.angles, ent->client->ps.viewangles);
	VectorCopy(ent->s.angles, ent->client->v_angle);  // Angles

	gi.unlinkentity(ent); // Unlink him, so that he telefrags without killing himself aswell
	KillBox(ent); // Telefrag!
	gi.linkentity(ent);  // Link again

	ent->client->newweapon = ent->client->pers.weapon; // Weapon
	ChangeWeapon(ent);  // Change to that weapon.

	ent->nextthink = level.time + FRAMETIME; // Will think again later.

}


void Bot_Respawn(edict_t *ent)
{
	CopyToBodyQue(ent); // Bodyque stuff

	Bot_Spawn(ent);  // Respawn.. VOO
	ent->s.event = EV_PLAYER_TELEPORT;  // He's teleporting
	// Hold here for a few seconds
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 50;
}

void Bot_Attack(edict_t *ent, usercmd_t *cmd, vec3_t angles)
{
	if (ent->health < 1)
		return;
	if (!ent->enemy)
	{
		ent->oldenemy = NULL;
		ent->enemy = NULL; // Self-explainatory
		ent->client->pers.bot_state &= BOT_ENEMY;
		return;
	}
	if (ent->enemy->health < 1)
	{
		ent->oldenemy = NULL;
		ent->enemy = NULL; // Self-explainatory
		ent->client->pers.bot_state &= BOT_ENEMY;
	}
	else
	{
		if (!visible(ent, ent->enemy))
		{
			ent->oldenemy = NULL;
			ent->enemy = NULL; // Self-explainatory
			ent->client->pers.bot_state &= BOT_ENEMY;
			return;
		}
		if (infront(ent, ent->enemy))
		{
			Bot_Move(ent, ent->enemy, cmd, angles);
			Bot_Aim(ent, ent->enemy, angles); 
			if (!Q_stricmp(ent->client->pers.weapon->pickup_name, "Chaingun") == 0 && !Q_stricmp(ent->client->pers.weapon->pickup_name, "Hyperblaster") == 0 && !Q_stricmp(ent->client->pers.weapon->pickup_name, "Machinegun") == 0)
			{
				if (random() < 0.8) // Don't fire too often with certain weapons.
					cmd->buttons = BUTTON_ATTACK;
			}
			else
				cmd->buttons = BUTTON_ATTACK;
			ent->client->pers.bot_state |= BOT_ENEMY;
		}
	}
	// Attack enemy
}

void Bot_Aim(edict_t *ent, edict_t *target, vec3_t angles)
{
	vec3_t dir, start, end; 
	int r = rndnum (0, 10); // Will be re-used, for random aims

	if (!target)
		return;

	if (target->health < 1)
		return;
	if (ent->health < 1)
		return;

	VectorCopy(target->s.origin, start);
	VectorCopy(ent->s.origin, end);
	/*if (target)
	{
		if (ent->enemy)
		{
			if(target->client != NULL && target->client->ps.pmove.pm_flags & PMF_DUCKED)
				start[0] -= 22;
		}
	}*/

	VectorSubtract(start, end, dir);
	vectoangles(dir, angles); // This will aim at our enemy

}

void Bot_Pain(edict_t *ent, edict_t *other, float kickback, int damage)
{
	vec3_t angles = { 0, 0, 0 }; 

	VectorCopy(ent->client->v_angle, angles);

	if (ent != other)
	{
		vec3_t start, end, dir;
		vec3_t angles = {0, 0, 0};
		ent->oldenemy = ent->enemy;
		ent->enemy = other;
		//Bot_Aim (ent, ent->enemy, angles);
		VectorCopy(ent->enemy->s.origin, start);
		VectorCopy(ent->s.origin, end);

		VectorSubtract(start, end, dir);
		vectoangles(dir, angles); // This will aim at our enemy
	} 

	player_pain(ent, other, kickback, damage);
}

void Bot_Move(edict_t *ent, edict_t *goal, usercmd_t *cmd, vec3_t angles)
{
	int r = rndnum (0, 60);
	int sidemove_here;

	Bot_Aim(ent, goal, angles);

	if (Bot_CanMove(ent) == true)
	{
		if (random() < 0.4)
			sidemove_here = -200;
		else
			sidemove_here = 200;

		if (range(ent, goal) >= RANGE_NEAR)
			cmd->forwardmove = 200; // Run speed
		else
			cmd->forwardmove = -200; // Go backwards, don't get too close

		if (r > 25)
		{
			cmd->sidemove = sidemove_here;
		}
		if (r > 45)
		{
			if (ent->wait < level.time)
				cmd->upmove = 10;
		}
		if (r > 45 && ent->wait < level.time)
		{
			ent->wait = level.time + 4.0;
		}
		
	}
	else if (ent->movetarget)
	{
		Bot_Aim(ent, ent->movetarget, angles);
		cmd->forwardmove = 200;
	}
	else
	{
		cmd->sidemove = 0;
		cmd->forwardmove = 0;
		cmd->upmove = 0;
	}
}

edict_t *Bot_FindEnemy(edict_t *ent)
{
	int range = 4780; // They can see anything.. ANYZING
					  // Paril revision: Not everything. Only so far.
	char *classname;
	edict_t *enemy = ent->enemy, *newenemy = NULL; 

	if (enemy == NULL)
	{
		classname = "player"; 

		while ((newenemy = findradius(newenemy, ent->s.origin, range)) != NULL)
		{
			if (!newenemy->client)
				continue; 
			if (newenemy->flags & FL_NOTARGET)
				continue;
			
			if (ctf->value)
			{
				if (newenemy->client->resp.ctf_team == ent->client->resp.ctf_team)
				{
					/*if (Q_stricmp(newenemy->classname, "player") == 0)
					{
						if (!ent->movetarget)
						{
							safe_cprintf (newenemy, PRINT_CHAT, "%s: Hello, %s! Lead the way!\n", ent->client->pers.netname, newenemy->client->pers.netname);
							ent->movetarget = newenemy;
						}
					}*/
					continue;
				}
			}

			// Don't fight self or mess with the dead or invisible
			if ((newenemy != ent) && (newenemy->deadflag != DEAD_DEAD))// && (pxE->light_level > 5)) 
			{ 
				if ((Q_stricmp(newenemy->classname, classname) == 0) || (Q_stricmp(newenemy->classname, "bot") == 0) &&
				visible(ent, newenemy))
				{
					if (infront(ent, newenemy))
					{
						enemy = newenemy;
						break;
					}
				}
			}
		}
	} 

	return enemy;
}

// Based on ACE
// Bot will stop on ledges
qboolean Bot_CanMove (edict_t *self)
{
	vec3_t forward, right;
	vec3_t offset,start,end;
	vec3_t angles;
	trace_t tr;

	// Now check to see if move will move us off an edge
	VectorCopy(self->s.angles,angles);

	// Set up the vectors
	AngleVectors (angles, forward, right, NULL);
	
	VectorSet(offset, 36, 0, 24);
	G_ProjectSource (self->s.origin, offset, forward, right, start);
		
	VectorSet(offset, 36, 0, -450);
	G_ProjectSource (self->s.origin, offset, forward, right, end);


	tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID|MASK_OPAQUE);
	
	if(tr.fraction == 1.0 || tr.contents & (CONTENTS_LAVA|CONTENTS_SLIME))
	{
		return false;	
	}
	
	return true; // yup, can move
}


///////////////////////////////////////////////////////////////////////
// These routines are bot safe print routines, all id code needs to be 
// changed to these so the bots do not blow up on messages sent to them. 
// Do a find and replace on all code that matches the below criteria. 
//
// (Got the basic idea from Ridah)
//	
//  change: safe_cprintf to safe_cprintf
//  change: gi.bprintf to safe_bprintf
//  change: gi.centerprintf to safe_centerprintf
// 
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Debug print, could add a "logging" feature to print to a file
///////////////////////////////////////////////////////////////////////
void debug_printf(char *fmt, ...)
{
	int     i;
	char	bigbuffer[0x10000];
	int		len;
	va_list	argptr;
	edict_t	*cl_ent;
	
	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	if (dedicated->value)
		safe_cprintf(NULL, PRINT_MEDIUM, bigbuffer);

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || (Q_stricmp(cl_ent->classname, "bot") == 0))
			continue;

		safe_cprintf(cl_ent,  PRINT_MEDIUM, bigbuffer);
	}

}

///////////////////////////////////////////////////////////////////////
// botsafe cprintf
///////////////////////////////////////////////////////////////////////
void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...)
{
	char	bigbuffer[0x10000];
	va_list		argptr;
	int len;

	if (ent && (!ent->inuse || (Q_stricmp(ent->classname, "bot") == 0)))
		return;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	gi.cprintf(ent, printlevel, bigbuffer);
	
}

///////////////////////////////////////////////////////////////////////
// botsafe centerprintf
///////////////////////////////////////////////////////////////////////
void safe_centerprintf (edict_t *ent, char *fmt, ...)
{
	char	bigbuffer[0x10000];
	va_list		argptr;
	int len;

	if (!ent->inuse || (Q_stricmp(ent->classname, "bot") == 0))
		return;
	
	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);
	
	gi.centerprintf(ent, bigbuffer);
	
}

///////////////////////////////////////////////////////////////////////
// botsafe bprintf
///////////////////////////////////////////////////////////////////////
void safe_bprintf (int printlevel, char *fmt, ...)
{
	int i;
	char	bigbuffer[0x10000];
	int		len;
	va_list		argptr;
	edict_t	*cl_ent;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	if (dedicated->value)
		gi.cprintf(NULL, printlevel, bigbuffer);

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || (Q_stricmp(cl_ent->classname, "bot") == 0))
			continue;

		gi.cprintf(cl_ent, printlevel, bigbuffer);
	}
}

///////////////////////////////////////////////////////////////////////
// Pick best goal based on importance and range. This function
// overrides the long range goal selection for items that
// are very close to the bot and are reachable.
///////////////////////////////////////////////////////////////////////

void ACEAI_PickShortRangeGoal(edict_t *self, usercmd_t *cmd)
{
	edict_t *target;
	
	// look for a target (should make more efficient later)
	target = findradius(NULL, self->s.origin, 900);
	
	while(target)
	{
		if(target->classname == NULL)
			return;
		
		// Missle avoidance code
		// Set our movetarget to be the rocket or grenade fired at us. 
		if(strcmp(target->classname,"rocket")==0)
		{
			//safe_bprintf (PRINT_HIGH, "Rocket detected\n");
			//self->movetarget = target;
			self->client->ps.pmove.pm_flags |= PMF_DUCKED;
			if (random() > 0.5)
				cmd->sidemove = -200;
			else
				cmd->sidemove = 200;
		}
		// Missle avoidance code
		// Set our movetarget to be the rocket or grenade fired at us. 
		if(strcmp(target->classname,"grenade")==0 || strcmp(target->classname,"hgrenade")==0 )
		{
			//safe_bprintf (PRINT_HIGH, "Grenade detected\n");
			//self->movetarget = target;
			if (random() > 0.5)
				cmd->sidemove = -200;
			else
				cmd->sidemove = 200;
		}

		// next target
		target = findradius(target, self->s.origin, 200);
	}
}

///////////////////////////////////////////////////////////////////////
// Can we get there?
///////////////////////////////////////////////////////////////////////

qboolean ACEIT_IsReachable(edict_t *self, vec3_t goal)
{
	trace_t trace;
	vec3_t v;

	VectorCopy(self->mins,v);
	v[2] += 18; // Stepsize

//AQ2	trace = gi.trace (self->s.origin, v, self->maxs, goal, self, MASK_OPAQUE);
	trace = gi.trace (self->s.origin, v, self->maxs, goal, self, MASK_SOLID|MASK_OPAQUE);
	
	// Yes we can see it
	if (trace.fraction == 1.0)
		return true;
	else
		return false;

}

///////////////////////////////////////////////////////////////////////
// Convert a classname to its index value
//
// I prefer to use integers/defines for simplicity sake. This routine
// can lead to some slowdowns I guess, but makes the rest of the code
// easier to deal with.
///////////////////////////////////////////////////////////////////////
// Item defines (got this list from somewhere??....so thanks to whoever created it)
#define INVALID -1
#define ITEMLIST_NULLINDEX			0
#define ITEMLIST_BODYARMOR			1
#define ITEMLIST_COMBATARMOR		2
#define ITEMLIST_JACKETARMOR		3
#define ITEMLIST_ARMORSHARD			4
#define ITEMLIST_POWERSCREEN		5
#define ITEMLIST_POWERSHIELD		6

#define ITEMLIST_GRAPPLE            7

#define ITEMLIST_BLASTER			8
#define ITEMLIST_SHOTGUN			9
#define ITEMLIST_SUPERSHOTGUN		10
#define ITEMLIST_MACHINEGUN			11
#define ITEMLIST_CHAINGUN			12
#define ITEMLIST_GRENADES			13
#define ITEMLIST_GRENADELAUNCHER	14
#define ITEMLIST_ROCKETLAUNCHER		15
#define ITEMLIST_HYPERBLASTER		16
#define ITEMLIST_RAILGUN			17
#define ITEMLIST_BFG10K				18

#define ITEMLIST_SHELLS				19
#define ITEMLIST_BULLETS			20
#define ITEMLIST_CELLS				21
#define ITEMLIST_ROCKETS			22
#define ITEMLIST_SLUGS				23
#define ITEMLIST_QUADDAMAGE			24
#define ITEMLIST_INVULNERABILITY	25
#define ITEMLIST_SILENCER			26
#define ITEMLIST_REBREATHER			27
#define ITEMLIST_ENVIRONMENTSUIT	28
#define ITEMLIST_ANCIENTHEAD		29
#define ITEMLIST_ADRENALINE			30
#define ITEMLIST_BANDOLIER			31
#define ITEMLIST_AMMOPACK			32
#define ITEMLIST_DATACD				33
#define ITEMLIST_POWERCUBE			34
#define ITEMLIST_PYRAMIDKEY			35
#define ITEMLIST_DATASPINNER		36
#define ITEMLIST_SECURITYPASS		37
#define ITEMLIST_BLUEKEY			38
#define ITEMLIST_REDKEY				39
#define ITEMLIST_COMMANDERSHEAD		40
#define ITEMLIST_AIRSTRIKEMARKER	41
#define ITEMLIST_HEALTH				42

// new for ctf
#define ITEMLIST_FLAG1              43
#define ITEMLIST_FLAG2              44
#define ITEMLIST_RESISTANCETECH     45
#define ITEMLIST_STRENGTHTECH       46
#define ITEMLIST_HASTETECH          47
#define ITEMLIST_REGENERATIONTECH   48

// my additions
#define ITEMLIST_HEALTH_SMALL		49
#define ITEMLIST_HEALTH_MEDIUM		50
#define ITEMLIST_HEALTH_LARGE		51
#define ITEMLIST_BOT				52
#define ITEMLIST_PLAYER				53
#define ITEMLIST_HEALTH_MEGA        54

int ACEIT_ClassnameToIndex(char *classname)
{
	// Normal quake stuff:
	if(strcmp(classname,"item_armor_body")==0) 
		return ITEMLIST_BODYARMOR;
	
	if(strcmp(classname,"item_armor_combat")==0)
		return ITEMLIST_COMBATARMOR;

	if(strcmp(classname,"item_armor_jacket")==0)
		return ITEMLIST_JACKETARMOR;
	
	if(strcmp(classname,"item_armor_shard")==0)
		return ITEMLIST_ARMORSHARD;

	if(strcmp(classname,"item_power_screen")==0)
		return ITEMLIST_POWERSCREEN;

	if(strcmp(classname,"item_power_shield")==0)
		return ITEMLIST_POWERSHIELD;

	if(strcmp(classname,"weapon_grapple")==0)
		return ITEMLIST_GRAPPLE;

	if(strcmp(classname,"weapon_blaster")==0)
		return ITEMLIST_BLASTER;

	if(strcmp(classname,"weapon_shotgun")==0)
		return ITEMLIST_SHOTGUN;
	
	if(strcmp(classname,"weapon_supershotgun")==0)
		return ITEMLIST_SUPERSHOTGUN;
	
	if(strcmp(classname,"weapon_machinegun")==0)
		return ITEMLIST_MACHINEGUN;
	
	if(strcmp(classname,"weapon_chaingun")==0)
		return ITEMLIST_CHAINGUN;

	if(strcmp(classname,"weapon_chaingun")==0)
		return ITEMLIST_CHAINGUN;
	
	if(strcmp(classname,"ammo_grenades")==0)
		return ITEMLIST_GRENADES;

	if(strcmp(classname,"weapon_grenadelauncher")==0)
		return ITEMLIST_GRENADELAUNCHER;

	if(strcmp(classname,"weapon_rocketlauncher")==0)
		return ITEMLIST_ROCKETLAUNCHER;

	if(strcmp(classname,"weapon_hyperblaster")==0)
		return ITEMLIST_HYPERBLASTER;

	if(strcmp(classname,"weapon_railgun")==0)
		return ITEMLIST_RAILGUN;

	if(strcmp(classname,"weapon_bfg10k")==0)
		return ITEMLIST_BFG10K;

	if(strcmp(classname,"ammo_shells")==0)
		return ITEMLIST_SHELLS;
	
	if(strcmp(classname,"ammo_bullets")==0)
		return ITEMLIST_BULLETS;

	if(strcmp(classname,"ammo_cells")==0)
		return ITEMLIST_CELLS;

	if(strcmp(classname,"ammo_rockets")==0)
		return ITEMLIST_ROCKETS;

	if(strcmp(classname,"ammo_slugs")==0)
		return ITEMLIST_SLUGS;
	
	if(strcmp(classname,"item_quad")==0)
		return ITEMLIST_QUADDAMAGE;

	if(strcmp(classname,"item_invunerability")==0)
		return ITEMLIST_INVULNERABILITY;

	if(strcmp(classname,"item_silencer")==0)
		return ITEMLIST_SILENCER;

	if(strcmp(classname,"item_rebreather")==0)
		return ITEMLIST_REBREATHER;

	if(strcmp(classname,"item_enviornmentsuit")==0)
		return ITEMLIST_ENVIRONMENTSUIT;

	if(strcmp(classname,"item_ancienthead")==0)
		return ITEMLIST_ANCIENTHEAD;

	if(strcmp(classname,"item_adrenaline")==0)
		return ITEMLIST_ADRENALINE;

	if(strcmp(classname,"item_bandolier")==0)
		return ITEMLIST_BANDOLIER;

	if(strcmp(classname,"item_pack")==0)
		return ITEMLIST_AMMOPACK;

	if(strcmp(classname,"item_datacd")==0)
		return ITEMLIST_DATACD;

	if(strcmp(classname,"item_powercube")==0)
		return ITEMLIST_POWERCUBE;

	if(strcmp(classname,"item_pyramidkey")==0)
		return ITEMLIST_PYRAMIDKEY;

	if(strcmp(classname,"item_dataspinner")==0)
		return ITEMLIST_DATASPINNER;

	if(strcmp(classname,"item_securitypass")==0)
		return ITEMLIST_SECURITYPASS;

	if(strcmp(classname,"item_bluekey")==0)
		return ITEMLIST_BLUEKEY;

	if(strcmp(classname,"item_redkey")==0)
		return ITEMLIST_REDKEY;

	if(strcmp(classname,"item_commandershead")==0)
		return ITEMLIST_COMMANDERSHEAD;

	if(strcmp(classname,"item_airstrikemarker")==0)
		return ITEMLIST_AIRSTRIKEMARKER;

	if(strcmp(classname,"item_health")==0) // ??
		return ITEMLIST_HEALTH;

	if(strcmp(classname,"item_flag_team1")==0)
		return ITEMLIST_FLAG1;

	if(strcmp(classname,"item_flag_team2")==0)
		return ITEMLIST_FLAG2;

	if(strcmp(classname,"item_tech1")==0)
		return ITEMLIST_RESISTANCETECH;

	if(strcmp(classname,"item_tech2")==0)
		return ITEMLIST_STRENGTHTECH;

	if(strcmp(classname,"item_tech3")==0)
		return ITEMLIST_HASTETECH;

	if(strcmp(classname,"item_tech4")==0)
		return ITEMLIST_REGENERATIONTECH;

	if(strcmp(classname,"item_health_small")==0)
		return ITEMLIST_HEALTH_SMALL;

	if(strcmp(classname,"item_health_medium")==0)
		return ITEMLIST_HEALTH_MEDIUM;

	if(strcmp(classname,"item_health_large")==0)
		return ITEMLIST_HEALTH_LARGE;
	
	if(strcmp(classname,"item_health_mega")==0)
		return ITEMLIST_HEALTH_MEGA;

	return INVALID;
}

///////////////////////////////////////////////////////////////////////
// Determines the NEED for an item
//
// This function can be modified to support new items to pick up
// Any other logic that needs to be added for custom decision making
// can be added here. For now it is very simple.
///////////////////////////////////////////////////////////////////////

float ACEIT_ItemNeed(edict_t *self, int item)
{
	
	// Make sure item is at least close to being valid
	if(item < 0 || item > 100)
		return 0.0;
        

	switch(item)
	{
		// Health
		case ITEMLIST_HEALTH_SMALL:	
		case ITEMLIST_HEALTH_MEDIUM:
		case ITEMLIST_HEALTH_LARGE:	
		case ITEMLIST_HEALTH_MEGA:	
			if(self->health < 100)
				return 1.0 - (float)self->health/100.0f; // worse off, higher priority
			else
				return 0.0;

		case ITEMLIST_AMMOPACK:
		case ITEMLIST_QUADDAMAGE:
		case ITEMLIST_INVULNERABILITY:
		case ITEMLIST_SILENCER:			
	//	case ITEMLIST_REBREATHER
	//	case ITEMLIST_ENVIRONMENTSUIT
		case ITEMLIST_ADRENALINE:		
		case ITEMLIST_BANDOLIER:			
			return 0.5;
		
		// Weapons
		case ITEMLIST_ROCKETLAUNCHER:
		case ITEMLIST_RAILGUN:
		case ITEMLIST_MACHINEGUN:
		case ITEMLIST_CHAINGUN:
		case ITEMLIST_SHOTGUN:
		case ITEMLIST_SUPERSHOTGUN:
		case ITEMLIST_BFG10K:
		case ITEMLIST_GRENADELAUNCHER:
		case ITEMLIST_HYPERBLASTER:
			if(!self->client->pers.inventory[item])
				return 0.7;
			else
				return 0.0;

		// Ammo
		case ITEMLIST_SLUGS:			
			if(self->client->pers.inventory[ITEMLIST_SLUGS] < self->client->pers.max_slugs)
				return 0.3;  
			else
				return 0.0;
	
		case ITEMLIST_BULLETS:
			if(self->client->pers.inventory[ITEMLIST_BULLETS] < self->client->pers.max_bullets)
				return 0.3;  
			else
				return 0.0;
	
		case ITEMLIST_SHELLS:
		   if(self->client->pers.inventory[ITEMLIST_SHELLS] < self->client->pers.max_shells)
				return 0.3;  
			else
				return 0.0;
	
		case ITEMLIST_CELLS:
			if(self->client->pers.inventory[ITEMLIST_CELLS] <	self->client->pers.max_cells)
				return 0.3;  
			else
				return 0.0;
	
		case ITEMLIST_ROCKETS:
			if(self->client->pers.inventory[ITEMLIST_ROCKETS] < self->client->pers.max_rockets)
				return 0.3;  
			else
				return 0.0;
	
		case ITEMLIST_GRENADES:
			if(self->client->pers.inventory[ITEMLIST_GRENADES] < self->client->pers.max_grenades)
				return 0.3;  
			else
				return 0.0;
	
		case ITEMLIST_BODYARMOR:
			if(ACEIT_CanUseArmor (FindItem("Body Armor"), self))
				return 0.6;  
			else
				return 0.0;
	
		case ITEMLIST_COMBATARMOR:
			if(ACEIT_CanUseArmor (FindItem("Combat Armor"), self))
				return 0.6;  
			else
				return 0.0;
	
		case ITEMLIST_JACKETARMOR:
			if(ACEIT_CanUseArmor (FindItem("Jacket Armor"), self))
				return 0.6;  
			else
				return 0.0;
	
		case ITEMLIST_POWERSCREEN:
		case ITEMLIST_POWERSHIELD:
			return 0.5;  

/*		case ITEMLIST_FLAG1:
			// If I am on team one, I want team two's flag
			if(!self->client->pers.inventory[item] && self->client->resp.ctf_team == CTF_TEAM2)
				return 10.0;  
			else 
				return 0.0;

		case ITEMLIST_FLAG2:
			if(!self->client->pers.inventory[item] && self->client->resp.ctf_team == CTF_TEAM1)
				return 10.0;  
			else
				return 0.0;*/
		
		case ITEMLIST_RESISTANCETECH:
		case ITEMLIST_STRENGTHTECH:
		case ITEMLIST_HASTETECH:			
		case ITEMLIST_REGENERATIONTECH:
			// Check for other tech
			if(!self->client->pers.inventory[ITEMLIST_RESISTANCETECH] &&
			   !self->client->pers.inventory[ITEMLIST_STRENGTHTECH] &&
			   !self->client->pers.inventory[ITEMLIST_HASTETECH] &&
			   !self->client->pers.inventory[ITEMLIST_REGENERATIONTECH])
			    return 0.4;  
			else
				return 0.0;
				
		default:
			return 0.0;	
	}	
}

extern gitem_armor_t jacketarmor_info;
extern gitem_armor_t combatarmor_info;
extern gitem_armor_t bodyarmor_info;

///////////////////////////////////////////////////////////////////////
// Check if we can use the armor
///////////////////////////////////////////////////////////////////////
qboolean ACEIT_CanUseArmor (gitem_t *item, edict_t *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;

	// get info on new armor
	newinfo = (gitem_armor_t *)item->info;

	old_armor_index = ArmorIndex (other);

	// handle armor shards specially
	if (item->tag == ARMOR_SHARD)
		return true;
	
	// get info on old armor
	if (old_armor_index == ITEM_INDEX(FindItem("Jacket Armor")))
		oldinfo = &jacketarmor_info;
	else if (old_armor_index == ITEM_INDEX(FindItem("Combat Armor")))
		oldinfo = &combatarmor_info;
	else // (old_armor_index == body_armor_index)
		oldinfo = &bodyarmor_info;

	if (newinfo->normal_protection <= oldinfo->normal_protection)
	{
		// calc new armor values
		salvage = newinfo->normal_protection / oldinfo->normal_protection;
		salvagecount = salvage * newinfo->base_count;
		newcount = other->client->pers.inventory[old_armor_index] + salvagecount;

		if (newcount > oldinfo->max_count)
			newcount = oldinfo->max_count;

		// if we're already maxed out then we don't need the new armor
		if (other->client->pers.inventory[old_armor_index] >= newcount)
			return false;

	}

	return true;
}


void Bot_ChangeWeapon_DontUseBlaster (edict_t *ent)
{
	if (ent->health < 1)
		return;
	if (!ent->enemy)
		return;

	if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Blaster") == 0)
	{ // Switch to anything you have with blaster.. NOW!
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("bfg10k"))] && range(ent, ent->enemy) <= RANGE_NEAR)
		{
			ent->client->newweapon = FindItem ("bfg10k");
			return;
		}
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))] )
		{
			ent->client->newweapon = FindItem ("railgun");
			return;
		}
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] )
		{
			ent->client->newweapon = FindItem ("hyperblaster");
			return;
		}
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("rocket launcher"))] && range(ent, ent->enemy) <= RANGE_MID)
		{
			ent->client->newweapon = FindItem ("rocket launcher");
			return;
		}
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("grenade launcher"))] && range(ent, ent->enemy) <= RANGE_NEAR)
		{
			ent->client->newweapon = FindItem ("grenade launcher");
			return;
		}
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] &&  !ent->client->pers.inventory[ITEM_INDEX(FindItem("grenade launcher"))] )
		{
			ent->client->newweapon = FindItem ("grenades");
			return;
		}
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] )
		{
			ent->client->newweapon = FindItem ("chaingun");
			return;
		}
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] )
		{
			ent->client->newweapon = FindItem ("machinegun");
			return;
		}
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 1 &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))] )
		{
			ent->client->newweapon = FindItem ("super shotgun");
			return;
		}
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("shotgun"))] )
		{
			ent->client->newweapon = FindItem ("shotgun");
			return;
		}
	}
}

void Bot_ChangeWeapon_Accordingly (edict_t *ent, int change_event)
{
	if (ent->health < 1)
		return;
	if (!ent->enemy)
		return;

	if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Shotgun") == 0)
	{
		if (range(ent, ent->enemy) < RANGE_NEAR)
		{
			if (random() > 0.75)
			{
				if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 1 &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))] )
				{
					ent->client->newweapon = FindItem ("super shotgun");
					return;
				}
			}
		}
		if (range(ent, ent->enemy) >= RANGE_NEAR)
		{
			if (random() > 0.9 && ent->health >= 100)
			{
				if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("bfg10k"))] )
				{
					ent->client->newweapon = FindItem ("bfg10k");
					return;
				}
			}
			if (ent->health < 80 && random() > 0.7 && range(ent, ent->enemy) >= RANGE_MID)
			{
				if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))] )
				{
					ent->client->newweapon = FindItem ("railgun");
					return;
				}
			}
			if (random() > 0.55)
			{
				if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] )
				{
					ent->client->newweapon = FindItem ("hyperblaster");
					return;
				}
			}
			if (random() > 0.55 && range(ent, ent->enemy) >= RANGE_NEAR)
			{
				if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("rocket launcher"))] && range(ent, ent->enemy) <= RANGE_MID)
				{
					ent->client->newweapon = FindItem ("rocket launcher");
					return;
				}
			}
			if (random() > 0.30 && range(ent, ent->enemy) >= RANGE_MID)
			{
				if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("grenade launcher"))] && range(ent, ent->enemy) <= RANGE_NEAR)
				{
					ent->client->newweapon = FindItem ("grenade launcher");
					return;
				}
			}
			if (random() < 0.10 && range(ent, ent->enemy) <= RANGE_MID)
			{ // Handgrenades kinda suck
				if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] &&  !ent->client->pers.inventory[ITEM_INDEX(FindItem("grenade launcher"))] )
				{
					ent->client->newweapon = FindItem ("grenades");
					return;
				}
			}

			if (random() > 0.90) // Not often
			{
				if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] )
				{
					ent->client->newweapon = FindItem ("chaingun");
					return;
				}
			}
			if (random() > 0.82) // Not often
			{
				if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] &&  ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] )
				{
					ent->client->newweapon = FindItem ("machinegun");
					return;
				}
			}

			ent->client->newweapon = FindItem ("blaster");
		}
	}
}
