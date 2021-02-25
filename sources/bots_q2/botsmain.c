// CUSTOM SOURCE FILE

#include "g_local.h"
#include "botsmain.h"
#include "botsteam.h"
#include "botsqdev.h"
#include "botsutil.h"
#include "botshook.h"
#include "botsini.h"
#include "p_light.h"

void gsmod_Tracker (edict_t *ent);
void gsmod_Suicide (edict_t *ent);
void gsmod_Scout (edict_t *ent, qboolean showall);
void weapon_grenade_fire (edict_t *ent, qboolean held);
void weapon_grenadelauncher_fire (edict_t *ent);
void Use_Quad (edict_t *ent, gitem_t *item);
void Use_Invulnerability (edict_t *ent, gitem_t *item);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
void Weapon_RocketLauncher_Fire (edict_t *ent);
void Weapon_Blaster_Fire (edict_t *ent);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker);


void gsmod_ClearVotes(void)
{
	edict_t		*player;
	int			i;

	startvote = 0.0;
	votetotal = 0;

	for (i=1; i < nextmap; i++)
		votes[i].numvotes = 0;

	for (i=0 ; i < game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->client)
			continue; 
		player->client->pers.castvote = 0;
	}
}

void gsmod_QueueCommand(edict_t *ent, char *cmdstring)
{
	if (!ent->client)
		return;

	sprintf(ent->cmdstring, "%s\n", cmdstring);
	ent->stuffcmd = true;
}


void gsmod_Skill (edict_t *ent, int index)
{
	if (!ent->client)
		return;

	if (ent->client->pers.player_class == 1)
	{
		if (index == 1)
			gsmod_QueueCommand(ent, "promote");
		else if (index == 2)
			gsmod_QueueCommand(ent, "create");
		else if (index == 3)
			gsmod_QueueCommand(ent, "tracker");
	}
	else if (ent->client->pers.player_class == 2)
	{
		if (index == 1)
			gsmod_QueueCommand(ent, "laseron");
		else if (index == 2)
			gsmod_QueueCommand(ent, "laseroff");
		else if (index == 3)
			gsmod_QueueCommand(ent, "decoy");
	}
	else if (ent->client->pers.player_class == 3)
	{
		if (index == 1)
			gsmod_QueueCommand(ent, "sight");
		else if (index == 2)
			gsmod_QueueCommand(ent, "sendalarm");
		else if (index == 3)
			gsmod_QueueCommand(ent, "tracker");
	}
	else if (ent->client->pers.player_class == 4)
	{
		if (index == 1)
			gsmod_QueueCommand(ent, "splitmode");
		else if (index == 2)
			gsmod_QueueCommand(ent, "use quad damage");
//		else if (index == 3)
//			gsmod_QueueCommand(ent, "use rocket launcher");
	}
	else if (ent->client->pers.player_class == 5)
	{
		if (index == 1)
			gsmod_QueueCommand(ent, "rage");
		else if (index == 2)
			gsmod_QueueCommand(ent, "use quad damage");
//		else if (index == 3)
//			gsmod_QueueCommand(ent, "use chaingun");
	}
	else if (ent->client->pers.player_class == 6)
	{
		if (index == 1)
			gsmod_QueueCommand(ent, "steal");
		else if (index == 2)
			gsmod_QueueCommand(ent, "disguise");
//		else if (index == 3)
//			gsmod_QueueCommand(ent, "use hyperblaster");
	}
	else if (ent->client->pers.player_class == 7)
	{
		if (index == 1)
			gsmod_QueueCommand(ent, "suicide");
		else if (index == 2)
			gsmod_QueueCommand(ent, "detpipe");
		else if (index == 3)
			gsmod_QueueCommand(ent, "detmode");
	}
	else if (ent->client->pers.player_class == 8)
	{
		if (index == 1)
			gsmod_QueueCommand(ent, "poison");
		else if (index == 2)
			gsmod_QueueCommand(ent, "heal");
//		else if (index == 3)
//			gsmod_QueueCommand(ent, "");
	}
}


void ShowGun(edict_t *ent)
{
	char heldmodel[128];
	int len;

	if (!ent->client)
		return;

	if(!ent->client->pers.weapon)
	{
		ent->s.modelindex2 = 0;
		return;
	}

	strcpy(heldmodel, "players/");
	strcat(heldmodel, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	for(len = 8; heldmodel[len]; len++)
	{
		if(heldmodel[len] == '/')
			heldmodel[++len] = '\0';
	}
	strcat(heldmodel, ent->client->pers.weapon->icon);	
	strcat(heldmodel, ".md2");
	//gi.dprintf ("%s\n", heldmodel);
	ent->s.modelindex2 = gi.modelindex(heldmodel);	// Hentai's custom gun models
}

//
// Add include: 
// #include "\quake2\gsmod\gsmod.h"
// to the following files
//
// p_client.c
// p_weapon.c
// g_items.c
//
void Grenade_Explode (edict_t *ent);
void Flare_Explode (edict_t *ent);

void gsmod_BlowGrenades (edict_t *ent)
{
	edict_t *grenade;

	if (ent->client)
	{
		ent->client->proxdet = 0;
		ent->client->decoypromo = false;
	}

	for (grenade=g_edicts; grenade < &g_edicts[globals.num_edicts]; grenade++)
	{
		if (!grenade->inuse)
			continue;

		if (Q_stricmp (grenade->classname, "gib") == 0)
		{
			G_FreeEdict(grenade);
		}
		else
		{
			if (grenade->owner == ent)
			{
				if ((Q_stricmp (grenade->classname, "proximity_grenade") == 0) || 
					(Q_stricmp (grenade->classname, "detpipe") == 0) || 
					(Q_stricmp (grenade->classname, "decoy_grenade") == 0) ||
					(Q_stricmp (grenade->classname, "decoypromo") == 0) ||
					(Q_stricmp (grenade->classname, "hgrenade") == 0))
				{
					grenade->think = Grenade_Explode;
					grenade->nextthink = level.time + .1;
				}

				if (Q_stricmp (grenade->classname, "flare") == 0)
				{
					grenade->think = Flare_Explode;
					grenade->nextthink = level.time + .1;
				}
				
				if (Q_stricmp (grenade->classname, "item_health") == 0)
				{
					if (grenade->count < 0)
					{
						grenade->count = (grenade->count * -1) / 2;
						grenade->owner = NULL;
					}
				}

				if (Q_stricmp (grenade->classname, "nurse_health_large") == 0)
					grenade->owner = NULL;
				if (Q_stricmp (grenade->classname, "nurse_health_mega") == 0)
					grenade->owner = NULL;
			}
		}
	}
}


char *gsmod_TeamColor (int team)
{
	if (team == 1)
		return "b";
	else if (team == 2)
		return "r";
	else
		return "g";
}


//
// Change function in p_client.c to the following:
//
//void ClientUserinfoChanged (edict_t *ent, char *userinfo)
//{
//	gsmod_ClientUserinfoChanged(ent, userinfo, false, false);
//}
//
void gsmod_ClientUserinfoChanged (edict_t *ent, char *userinfo, qboolean spawned, qboolean newskin)
{
	char	*s;
	int		playernum;
	char	str[MAX_INFO_KEY], femalepic_name[12], malepic_name[12];

	if (!ent->client)
		return;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}


	// set name
	s = Info_ValueForKey (userinfo, "name");

	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;
	
	if ((ent->client->special) && (ent->client->pers.player_class == 6) && (ent->client->pers.classlevel > 1))
		s = ent->client->pers.oldskin;
	else if ((teamplay == 1) && (ent->health < 35))
		s = ent->client->pers.oldskin;
	else if ((teamplay == 1) && (ent->client->pers.player_class) && (ent->client->pers.team))
	{
		// set the correct skin
		if (ent->client->pers.player_class == 1)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "capt", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "capt", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "athena");
				sprintf(malepic_name, "%s", "major");
			}
		}
		else if (ent->client->pers.player_class == 2)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "body", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "body", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "stiletto");
				sprintf(malepic_name, "%s", "flak");
			}
		}
		else if (ent->client->pers.player_class == 3)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "snip", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "snip", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "jungle");
				sprintf(malepic_name, "%s", "sniper");
			}
		}
		else if (ent->client->pers.player_class == 4)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "sol", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "sol", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "ensign");
				sprintf(malepic_name, "%s", "grunt");
			}
		}
		else if (ent->client->pers.player_class == 5)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "berz", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "berz", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "voodoo");
				sprintf(malepic_name, "%s", "howitzer");
			}
		}
		else if (ent->client->pers.player_class == 6)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "enf", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "enf", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "brianna");
				sprintf(malepic_name, "%s", "recon");
			}
		}
		else if (ent->client->pers.player_class == 7)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "kami", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "kami", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "jezebel");
				sprintf(malepic_name, "%s", "psycho");
			}
		}
		else
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "nurs", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "nurs", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "venus");
				sprintf(malepic_name, "%s", "viper");
			}
		}
		
		//sprintf(str, "%s/t%i%s", gender, ent->client->pers.team, pic_name);
	//	if (ent->client->pers.team == 1)
	//		sprintf(str, "%s/%s", gender, "blue");
	//	else if (ent->client->pers.team == 2)
	//		sprintf(str, "%s/%s", gender, "red");
	//	else if (ent->client->pers.team == 3)
	//		sprintf(str, "%s/%s", gender, "green");

		if ((teamplay == 1) && (numberteams == 2))
		{
			if (ent->client->pers.team == 1)
			{
				if (ent->health < 35)
					sprintf(str, "female/damage/%s", femalepic_name);
				else
					sprintf(str, "female/%s", femalepic_name);
			}
			else
			{
				if (ent->health < 35)
					sprintf(str, "male/damage/%s", malepic_name);
				else
					sprintf(str, "male/%s", malepic_name);
			}
		}
		else
		{
			if (IsFemale(ent))
				sprintf(str, "female/%s", femalepic_name);
			else
				sprintf(str, "male/%s", malepic_name);
		}

		strcpy(ent->client->pers.oldskin, str);
		if (!ent->stuffskin) 
		{
			Info_SetValueForKey (userinfo, "skin", ent->client->pers.oldskin);
			Info_SetValueForKey (ent->client->pers.userinfo, "skin", ent->client->pers.oldskin);
			ent->stuffskin = true; 
		}
		// ent->client->pers.oldskin is now confirmed valid
		s = ent->client->pers.oldskin;
	}
	
	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

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

	if (newskin == false)
		gsmod_InitClientMod(ent, spawned);

	ShowGun(ent);
}


void gsmod_SetStats (edict_t *ent)
{
	int		modifier;	

	if (!ent->client)
		return;

	modifier = ent->client->pers.classlevel;
	if (modifier == 0)
		modifier = 1;

	ent->client->pers.max_health   = 0;
	ent->client->pers.max_grenades = 5;

	if (ent->client->pers.playmode == 0)
	{
		ent->client->pers.max_bullets	= 50;
		ent->client->pers.max_shells	= 50;
		ent->client->pers.max_rockets	= 50;
		ent->client->pers.max_cells		= 50;
		ent->client->pers.max_slugs		= 50;
	}
	else
	{
		ent->client->pers.max_bullets	= 0;
		ent->client->pers.max_shells	= 0;
		ent->client->pers.max_rockets	= 0;
		ent->client->pers.max_cells		= 0;
		ent->client->pers.max_slugs		= 0;
		if (ent->client->pers.team == 2)  // male
			ent->client->pers.max_shells = 50;
	}

	switch (ent->client->pers.player_class) 
	{
		case 1:
				if (ent->client->pers.team == 1)  // female
				{
					if (ent->client->pers.playmode == 0) // on defense
					{
						ent->client->pers.max_health	= 225;
						ent->client->pers.max_cells		= 200 * modifier;
						ent->client->pers.max_grenades	= 35;
					}
					else
					{
						ent->client->pers.max_health	= 250;
						ent->client->pers.max_cells		= 200 * modifier;
						ent->client->pers.max_grenades	= 10;
					}
				}
				else
				{
					if (ent->client->pers.playmode == 0)
					{
						ent->client->pers.max_health	= 125;
						ent->client->pers.max_cells		= 200 * modifier;
						ent->client->pers.max_grenades	= 50;
					}
					else
					{
						ent->client->pers.max_health	= 150;
						ent->client->pers.max_cells		= 200 * modifier;
						ent->client->pers.max_grenades	= 15;
					}
				}
				break;
		case 2:
				if (ent->client->pers.team == 1)  // female
				{
					if (ent->client->pers.playmode == 0) // on defense
					{
						ent->client->pers.max_health	= 225;
						ent->client->pers.max_shells	= 50 * modifier;
						ent->client->pers.max_grenades	= 40;
						ent->client->pers.max_cells		= 150 * modifier;
					}
					else
					{
						ent->client->pers.max_health	= 250;
						ent->client->pers.max_shells	= 70 * modifier;
						ent->client->pers.max_grenades	= 16;
						ent->client->pers.max_cells		= 75 * modifier;
					}
				}
				else
				{
					if (ent->client->pers.playmode == 0)
					{
						ent->client->pers.max_health	= 125;
						ent->client->pers.max_shells	= 65 * modifier;
						ent->client->pers.max_grenades	= 50;
						ent->client->pers.max_cells		= 200 * modifier;
					}
					else
					{
						ent->client->pers.max_health	= 150;
						ent->client->pers.max_shells	= 90 * modifier;
						ent->client->pers.max_grenades	= 20;
						ent->client->pers.max_cells		= 100 * modifier;
					}
				}
				break;
		case 3:
				if (ent->client->pers.team == 1)  // female
				{
					if (ent->client->pers.playmode == 0) // on defense
					{
						ent->client->pers.max_health	= 225;
						ent->client->pers.max_slugs		= 40 * modifier;
						ent->client->pers.max_cells		= 80 * modifier;
					}
					else
					{
						ent->client->pers.max_health	= 250;
						ent->client->pers.max_slugs		= 55 * modifier;
						ent->client->pers.max_cells		= 100 * modifier;
					}
				}
				else
				{
					if (ent->client->pers.playmode == 0)
					{
						ent->client->pers.max_health	= 125;
						ent->client->pers.max_slugs		= 55 * modifier;
						ent->client->pers.max_cells		= 120 * modifier;
					}
					else
					{
						ent->client->pers.max_health	= 150;
						ent->client->pers.max_slugs		= 75 * modifier;
						ent->client->pers.max_cells		= 150 * modifier;
					}
				}
				break;
		case 4:
				if (ent->client->pers.team == 1)  // female
				{
					if (ent->client->pers.playmode == 0) // on defense
					{
						ent->client->pers.max_health	= 225;
						ent->client->pers.max_rockets	= 25 * modifier;
						ent->client->pers.max_grenades	= 20;
					}
					else
					{
						ent->client->pers.max_health	= 250;
						ent->client->pers.max_rockets	= 35 * modifier;
						ent->client->pers.max_grenades	= 2;
					}
				}
				else
				{
					if (ent->client->pers.playmode == 0)
					{
						ent->client->pers.max_health	= 125;
						ent->client->pers.max_rockets	= 40 * modifier;
						ent->client->pers.max_grenades	= 30;
					}
					else
					{
						ent->client->pers.max_health	= 150;
						ent->client->pers.max_rockets	= 60 * modifier;
						ent->client->pers.max_grenades	= 5;
					}
				}
				break;
		case 5:
				if (ent->client->pers.team == 1)  // female
				{
					if (ent->client->pers.playmode == 0) // on defense
					{
						ent->client->pers.max_health	= 225;
						ent->client->pers.max_bullets	= 200 * modifier;
						ent->client->pers.max_grenades	= 20;
					}
					else
					{
						ent->client->pers.max_health	= 275;
						ent->client->pers.max_bullets	= 250 * modifier;
						ent->client->pers.max_grenades	= 6;
					}
				}
				else
				{
					if (ent->client->pers.playmode == 0)
					{
						ent->client->pers.max_health	= 125;
						ent->client->pers.max_bullets	= 275 * modifier;
						ent->client->pers.max_grenades	= 30;
					}
					else
					{
						ent->client->pers.max_health	= 175;
						ent->client->pers.max_bullets	= 325 * modifier;
						ent->client->pers.max_grenades	= 10;
					}
				}

				if (ent->client->pers.max_bullets > 999)
					ent->client->pers.max_bullets = 999;

				break;
		case 6:
				if (ent->client->pers.team == 1)  // female
				{
					if (ent->client->pers.playmode == 0) // on defense
					{
						ent->client->pers.max_health	= 225;
						ent->client->pers.max_cells		= 100 * modifier;
						ent->client->pers.max_grenades	= 2;
					}
					else
					{
						ent->client->pers.max_health	= 250;
						ent->client->pers.max_cells		= 150 * modifier;
						ent->client->pers.max_grenades	= 4;
					}
				}
				else
				{
					if (ent->client->pers.playmode == 0)
					{
						ent->client->pers.max_health	= 125;
						ent->client->pers.max_cells		= 150 * modifier;
						ent->client->pers.max_grenades	= 2;
					}
					else
					{
						ent->client->pers.max_health	= 150;
						ent->client->pers.max_cells		= 200 * modifier;
						ent->client->pers.max_grenades	= 8;
					}
				}
				break;
		case 7:
				if (ent->client->pers.team == 1)  // female
				{
					if (ent->client->pers.playmode == 0) // on defense
					{
						ent->client->pers.max_health	= 225;
						ent->client->pers.max_grenades	= 30 * modifier;
						ent->client->pers.max_cells		= 80 * modifier;
					}
					else
					{
						ent->client->pers.max_health	= 275;
						ent->client->pers.max_grenades	= 45 * modifier;
						ent->client->pers.max_cells		= 120 * modifier;
					}
				}
				else
				{
					if (ent->client->pers.playmode == 0)
					{
						ent->client->pers.max_health	= 125;
						ent->client->pers.max_grenades	= 45 * modifier;
						ent->client->pers.max_cells		= 120 * modifier;
					}
					else
					{
						ent->client->pers.max_health	= 175;
						ent->client->pers.max_grenades	= 60 * modifier;
						ent->client->pers.max_cells		= 160 * modifier;
					}
				}
				break;
		case 8:
				if (ent->client->pers.team == 1)  // female
				{
					if (ent->client->pers.playmode == 0) // on defense
					{
						ent->client->pers.max_health	= 225;
						ent->client->pers.max_bullets	= 150 * modifier;
						ent->client->pers.max_cells		= 60 * modifier;
						ent->client->pers.max_grenades	= 35;
					}
					else
					{
						ent->client->pers.max_health	= 250;
						ent->client->pers.max_bullets	= 200 * modifier;
						ent->client->pers.max_cells		= 120 * modifier;
						ent->client->pers.max_grenades	= 18;
					}
				}
				else
				{
					if (ent->client->pers.playmode == 0)
					{
						ent->client->pers.max_health	= 125;
						ent->client->pers.max_bullets	= 200 * modifier;
						ent->client->pers.max_cells		= 120 * modifier;
						ent->client->pers.max_grenades	= 50;
					}
					else
					{
						ent->client->pers.max_health	= 150;
						ent->client->pers.max_bullets	= 250 * modifier;
						ent->client->pers.max_cells		= 180 * modifier;
						ent->client->pers.max_grenades	= 25;
					}
				}
				if (ent->client->pers.max_bullets > 999)
					ent->client->pers.max_bullets = 999;

				break;
	}
}

//
// Add function in p_client.c to the following:
//
//void PutClientInServer (edict_t *ent)
//{
//	...
//	// deathmatch wipes most client data every spawn
//	if (deathmatch->value)
//	{
//		char		userinfo[MAX_INFO_STRING];
//
//		resp = ent->client->resp;
//		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
//		InitClientPersistant (client);
//		gsmod_ClientUserinfoChanged (ent, userinfo, true);	<---
//	}
//	else
//	...
//}
//
void gsmod_InitClientMod (edict_t *ent, qboolean spawned)
{
	gitem_t	*it;

	if (!ent->client)
		return;

	ent->client->quad_framenum       = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum   = 0;
	ent->client->enviro_framenum     = 0;

	gsmod_SetStats(ent);
	
	if (spawned == true)
	{
		ent->client->pers.health = ent->client->pers.max_health - 50;
		ent->health	= ent->client->pers.health;

		if (ent->client->pers.team == 2)
		{
			it = FindItem ("Shotgun");
			ent->client->pers.inventory[ITEM_INDEX(it)] = 1;

			it = FindItem ("Shells");
			ent->client->pers.inventory[ITEM_INDEX(it)] += it->quantity;
		}
	}

	if (teamplay == 1)
		teamplay_InitClass(ent, spawned);
}


void gsmod_DropArmor (edict_t *ent, char *armorname)
{
	int				index;
	gitem_t			*it;
	gitem_armor_t	*armorinfo;

	if (!ent->client)
		return;

	it = FindItem (armorname);
	if (!it)
		return;

	armorinfo = (gitem_armor_t *)it->info;

	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
		return;

	// zero count of old armor so it goes away
	ent->client->pers.inventory[index] = 0;
}


void gsmod_Drop_item (edict_t *ent, char *itemname)
{
	int			index;
	gitem_t		*it;

	if (!ent->client)
		return;

	it = FindItem (itemname);
	if (!it)
		return;

	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
		return;

	it->drop (ent, it);
}


// Added a function call to the Pickup_Weapon function in the 
// p_weapon.c file:
//
//	...
//	if ( ((int)(dmflags->value) & DF_WEAPONS_STAY) && other->client->pers.inventory[index])
//	{
//		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
//			return false;	// leave the weapon for others to pickup
//	}
//
//	// Check the mod								<---
//	if gsmod_Pickup_Weapon(ent, other) == false		<---
//		return false;								<---
//	...
//
qboolean gsmod_Pickup_Weapon (edict_t *ent, edict_t *other)
{
	if (!other->client)
		return false;

	if (teamplay == 1)
		return teamplay_Pickup_Weapon(ent, other);

	// Never get to this code if teamplay is on
	if (IsFemale(other))
	{
		if ( (!strcmp (ent->item->pickup_name, "Rocket Launcher"))
		|| (!strcmp (ent->item->pickup_name, "Chaingun"))
		|| (!strcmp (ent->item->pickup_name, "Railgun"))
		|| (!strcmp (ent->item->pickup_name, "BFG10K")) )
		{
			//gsutil_centerprint(other, "The %s is too heavy for you to pick up :(\n", ent->item->pickup_name);
			return false;
		}
		else
			other->client->newweapon = ent->item;
	}
	else
	{
		if ( (!strcmp (ent->item->pickup_name, "Grenade Launcher"))
		|| (!strcmp (ent->item->pickup_name, "Machinegun"))
		|| (!strcmp (ent->item->pickup_name, "HyperBlaster")) )
		{
			//gsutil_centerprint(other, "The %s is too heavy for you to pick up :(\n", ent->item->pickup_name);
			return false;
		}
		else
			other->client->newweapon = ent->item;
	}

	return true;
}


//
// Add function in p_client.c to the following:
//
//void ClientThink (edict_t *ent, usercmd_t *ucmd)
//{
//	...
//	if (ent->movetype == MOVETYPE_NOCLIP)
//		client->ps.pmove.pm_type = PM_SPECTATOR;
//	else if (ent->s.modelindex != 255)
//		client->ps.pmove.pm_type = PM_GIB;
//	else if (ent->deadflag)
//		client->ps.pmove.pm_type = PM_DEAD;
//	else
//		client->ps.pmove.pm_type = PM_NORMAL;
//
//	gsmod_Gravity(client);		<--- NEW!
//	...
//}
//
void gsmod_JumpRating (edict_t *ent)
{
	float modifier;
	
	if (!ent->client)
		return;

	sv_gravity->value = newbots_gravity;

	modifier = 1.0;

	switch (ent->client->pers.player_class) 
	{
		case 1:
				if (ent->client->pers.team == 1)  // female
				{
					if (ent->client->pers.playmode == 0) // on defense
						modifier = modifier * 1.02;
					else
						modifier = 1.0;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.1;
					else
						modifier = modifier * 1.07;
				}
				break;
		case 2:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.02;
					else
						modifier = 1.0;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.1;
					else
						modifier = modifier * 1.07;
				}
				break;
		case 3:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.85;
					else
						modifier = modifier * 0.82;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.93;
					else
						modifier = modifier * 0.9;
				}
				break;
		case 4:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.05;
					else
						modifier = modifier * 1.02;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.13;
					else
						modifier = modifier * 1.1;
				}
				break;
		case 5:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.07;
					else
						modifier = modifier * 1.05;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.15;
					else
						modifier = modifier * 1.12;
				}
				break;
		case 6:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.85;
					else
						modifier = modifier * 0.82;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.93;
					else
						modifier = modifier * 0.9;
				}
				break;
		case 7:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.95;
					else
						modifier = modifier * 0.8;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = 1.0;
					else
						modifier = modifier * 0.9;
				}
				break;
		case 8:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.83;
					else
						modifier = modifier * 0.8;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.9;
					else
						modifier = modifier * 0.88;
				}
				break;
		default :
				modifier = 1.5;
				break;
	}

	if (ent->client->pers.team == 1)  // female
		modifier = modifier * 0.85;

	ent->client->pers.gravity_modifier = modifier;

	ent->client->ps.pmove.gravity = sv_gravity->value * modifier;
}


void gsmod_SpeedRating (usercmd_t *ucmd, edict_t *ent)
{
	float modifier;

	modifier = 0.90;
	
	if (!ent->client)
		return;

	switch (ent->client->pers.player_class) 
	{
		case 1:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.94;
					else
						modifier = modifier * 0.97;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.92;
					else
						modifier = modifier * 0.95;
				}
				break;
		case 2:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.94;
					else
						modifier = modifier * 0.97;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.92;
					else
						modifier = modifier * 0.93;
				}
				break;
		case 3:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.07;
					else
						modifier = 1.0;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.05;
					else
						modifier = modifier * 1.08;
				}
				break;
		case 4:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.96;
					else
						modifier = modifier * 0.97;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.94;
					else
						modifier = modifier * 0.96;
				}
				break;
		case 5:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.92;
					else
						modifier = modifier * 0.94;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 0.90;
					else
						modifier = modifier * 0.92;
				}
				break;
		case 6:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.09;
					else
						modifier = 1.0;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.06;
					else
						modifier = modifier * 1.08;
				}
				break;
		case 7:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 1)
						modifier = 1.0;
				}
				else
				{
					if (ent->client->pers.playmode == 1)
						modifier = modifier * 1.07;
				}
				break;
		case 8:
				if (ent->client->pers.team == 1)
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.08;
					else
						modifier = 1.0;
				}
				else
				{
					if (ent->client->pers.playmode == 0)
						modifier = modifier * 1.05;
					else
						modifier = modifier * 1.07;
				}
				break;
		default :
				modifier = 0.8;
				break;
	}

	ent->client->pers.speed_modifier = modifier;

	ucmd->forwardmove = (int)ucmd->forwardmove * modifier; 
	ucmd->sidemove = (int)ucmd->sidemove * modifier; 
	ucmd->upmove = (int)ucmd->upmove * modifier; 
}


void gsmod_Gravity (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;

	if (!ent->client)
		return;

	client = ent->client;

	if (ent->client->pers.gravity_modifier == 0)
	{
		gsmod_JumpRating(ent);
		gsmod_SpeedRating(ucmd, ent);
	}
	else
	{
		ucmd->forwardmove = ucmd->forwardmove * ent->client->pers.speed_modifier; 
		ucmd->sidemove = ucmd->sidemove * ent->client->pers.speed_modifier; 
		ucmd->upmove = ucmd->upmove * ent->client->pers.speed_modifier; 

		ent->client->ps.pmove.gravity = sv_gravity->value * ent->client->pers.gravity_modifier;
	}
}


//
// Add function in g_items.c to the following:
//
//qboolean Pickup_Armor (edict_t *ent, edict_t *other)
//{
//	...
//	// get info on new armor
//	newinfo = (gitem_armor_t *)ent->item->info;
//
//	if (gsmod_Pickup_Armor(other, ent->item->pickup_name) == false)
//		return false;
//	...
//}
//
qboolean gsmod_Pickup_Armor(edict_t *ent, char *pickup_name, int armorindex)
{
	int rating=0;
	int playmode;
	int modifier;
	
	if (!ent->client)
		return false;

	playmode = ent->client->pers.playmode;

	if (IsFemale(ent))
	{
		if ((!strcmp (pickup_name, "Combat Armor")) || (!strcmp (pickup_name, "Body Armor")))
			return false;
	}

	modifier = ent->client->pers.classlevel;
	if (modifier == 0)
		modifier = 1;

	if (armorindex)
	{
		switch (ent->client->pers.player_class) 
		{
			case 1:
					if (ent->client->pers.team == 1)
					{
						if (ent->client->pers.playmode == 0)
							rating = 210;
						else
							rating = 175;
					}
					else
					{
						if (ent->client->pers.playmode == 0)
							rating = 275;
						else
							rating = 225;
					}
					break;
			case 2:
					if (ent->client->pers.team == 1)
					{
						if (ent->client->pers.playmode == 0)
							rating = 175;
						else
							rating = 135;
					}
					else
					{
						if (ent->client->pers.playmode == 0)
							rating = 225;
						else
							rating = 175;
					}
					break;
			case 3:
					if (ent->client->pers.team == 1)
					{
						if (ent->client->pers.playmode == 0)
							rating = 55;
						else
							rating = 35;
					}
					else
					{
						if (ent->client->pers.playmode == 0)
							rating = 85;
						else
							rating = 50;
					}
					break;
			case 4:
					if (ent->client->pers.team == 1)
					{
						if (ent->client->pers.playmode == 0)
							rating = 210;
						else
							rating = 175;
					}
					else
					{
						if (ent->client->pers.playmode == 0)
							rating = 275;
						else
							rating = 225;
					}
					break;
			case 5:
					if (ent->client->pers.team == 1)
					{
						if (ent->client->pers.playmode == 0)
							rating = 250 + (modifier * 100);
						else
							rating = 200 + (modifier * 100);
					}
					else
					{
						if (ent->client->pers.playmode == 0)
							rating = 300 + (modifier * 100);
						else
							rating = 250 + (modifier * 100);
					}
					break;
			case 6:
					if (ent->client->pers.team == 1)
					{
						if (ent->client->pers.playmode == 0)
							rating = 55;
						else
							rating = 35;
					}
					else
					{
						if (ent->client->pers.playmode == 0)
							rating = 85;
						else
							rating = 50;
					}
					break;
			case 7:
					if (ent->client->pers.team == 1)
					{
						if (ent->client->pers.playmode == 0)
							rating = 45;
						else
							rating = 20;
					}
					else
					{
						if (ent->client->pers.playmode == 0)
							rating = 65;
						else
							rating = 40;
					}
					break;
			case 8:
					if (ent->client->pers.team == 1)
					{
						if (ent->client->pers.playmode == 0)
							rating = 15;
						else
							rating = 5;
					}
					else
					{
						if (ent->client->pers.playmode == 0)
							rating = 35;
						else
							rating = 15;
					}
					break;
			default :
					rating = 5;
					break;
		}

		if (ent->client->pers.inventory[armorindex] > rating)
		{
			ent->client->pers.inventory[armorindex] = rating;
			return false;
		}
	}

	return true;
}


//
// Add function in p_client.c to do the following:
//
//void ClientBeginDeathmatch (edict_t *ent)
//{
//	gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
//
//	// display text message to player	<---
//	gsmod_DisplayMsg(ent, "welcome.txt");	<---
//}
//
void gsmod_DisplayMsg (edict_t *ent, char *filename)
{
	FILE	*text_file;	
	char	text[500];	
	char	line[80];
	char	newfilename[MAX_QPATH] = "";
	int		i, islefn;

	if (!ent->client)
		return;

#ifdef _WIN32
	strcpy(newfilename, "bots\\");
#else
	strcpy(newfilename, "bots/");
#endif
	islefn = strlen(newfilename);
	for (i=0; filename[i]; i++)
		newfilename[i + islefn] = tolower(filename[i]);
	newfilename[i + islefn] = '\0';

	if (text_file = fopen(newfilename, "r"))	
	{
		// we successfully opened the file "text.txt"
		if ( fgets(text, 500, text_file) )		
		{
			// we successfully read a line from "text.txt" into text
			// ... read the remaining lines now			
			while ( fgets(line, 80, text_file) )
			{	// add each new line to text, to create a BIG message string.
				// we are using strcat: STRing conCATenation function here.
				strcat(text, line);			
			}			// print our message.
			gsutil_centerprint (ent, "%s", text);		
		}		// be good now ! ... close the file
		fclose(text_file);	
	}
}


int gsmod_NumPlayers(void)
{
	edict_t		*player;
	int			n, numplayers;

	numplayers = 0;
	for (n=0 ; n<game.maxclients ; n++)
	{
		player = g_edicts + 1 + n;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->observer)
			continue;
		if (!player->client->pers.player_class)
			continue;
		numplayers++;
	}

	return numplayers;
}


void gsmod_Play(edict_t *ent, int mode)
{
	edict_t	*player;
	int		n;
	char	*s, wavcmd[50];

	if (!ent->client)
		return;

	if (!ent->client->pers.customwav)
		return;
	
	s = gi.args();
	sprintf(wavcmd, "play %s\n", s);

	if (mode == 10)
	{
		if (teamkeys[ent->client->keyindex-1].lastcapturer)
		{
			if(teamkeys[ent->client->keyindex-1].lastcapturer->client)
				gsmod_QueueCommand(teamkeys[ent->client->keyindex-1].lastcapturer, wavcmd);
		}

		return;
	}
	else if (mode == 11)
	{
		if (ent->health <= 0)
		{
			if (ent->enemy)
			{
				if (ent->enemy->client)
					gsmod_QueueCommand(ent->enemy, wavcmd);
			}
		}

		return;
	}
	else if (mode == 12)
	{
		if (ent->inview)
		{
			if (ent->inview->client)
				gsmod_QueueCommand(ent->inview, wavcmd);
		}
		return;
	}

	for (n=0 ; n<game.maxclients ; n++)
	{
		player = g_edicts + 1 + n;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->observer)
			continue;
		if (!player->client->pers.player_class)
			continue;

		if (player->client->pers.team == ent->client->pers.team)
		{
			if (mode == 1)
			{
				if (player->client->pers.player_class == 1)
					gsmod_QueueCommand(player, wavcmd);
			}
			else if (mode == 2)
			{
				if (player->client->pers.player_class == 2)
					gsmod_QueueCommand(player, wavcmd);
			}
			else if (mode == 3)
			{
				if (player->client->pers.player_class == 3)
					gsmod_QueueCommand(player, wavcmd);
			}
			else if (mode == 4)
			{
				if (player->client->pers.player_class == 4)
					gsmod_QueueCommand(player, wavcmd);
			}
			else if (mode == 5)
			{
				if (player->client->pers.player_class == 5)
					gsmod_QueueCommand(player, wavcmd);
			}
			else if (mode == 6)
			{
				if (player->client->pers.player_class == 6)
					gsmod_QueueCommand(player, wavcmd);
			}
			else if (mode == 7)
			{
				if (player->client->pers.player_class == 7)
					gsmod_QueueCommand(player, wavcmd);
			}
			else if (mode == 8)
			{
				if (player->client->pers.player_class == 8)
					gsmod_QueueCommand(player, wavcmd);
			}
			else if (mode == 9)
			{
				gsmod_QueueCommand(player, wavcmd);
			}
		}
	}

	return;
}


void gsmod_InitVoting(void)
{
	char	*ls_value;
	int		elimvalue, randomvalue, groupvalue;

	if((int)(botsflags->value) & BOTS_CLASSIC_VOTE)
	{
		// old voting style
		mapsread = false;
		randomx_count = 0;
		elimx_count   = 0;
		groupx_count  = 0;
		currentmap    = 0;
		votemode      = 1;
		gsmod_ReadMaps("vote.lst");
	}
	else
	{
		// new style of voting using vote.ini
/*
		ls_value = ini_key_value ("vote.ini", "System", "LogVotes", "Yes");
		if (Q_stricmp (ls_value, "No") == 0)
			logvotes = false;
		else
			logvotes = true;
*/		
		ls_value = ini_key_value ("vote.ini", "System", "MapMode", "Voting");
		if (Q_stricmp (ls_value, "Voting") == 0)
		{
			mapsread = false;
			randomx_count = 0;
			elimx_count   = 0;
			groupx_count  = 0;
			votemode = 2;
			ls_value = ini_key_value ("vote.ini", "System", "MapList", "vote.lst");
			gsmod_ReadMaps(ls_value);
		}
		else
		{
			ls_value = ini_key_value ("vote.ini", "Rotation", "RotationType", "Sequential");
			if (Q_stricmp (ls_value, "Groups") == 0)
			{
				mapsread = false;
				randomx_count = 0;
				elimx_count   = 0;
				votemode = 4;

				ls_value = ini_key_value ("vote.ini", "Rotation", "GroupList", "groups.lst");
				gsmod_ReadMaps(ls_value);

				ls_value = ini_key_value ("vote.ini", "Rotation", "GroupX", "1");
				groupvalue = atoi(ls_value);
				
				if ((groupx_count >= groupvalue) || (groupx == 0))
				{
					currentmap   = 0;
					nextinseq    = 0;
					groupx_count = 1;
					groupx       = 0;
				}
				else
					groupx_count++;

				return;
			}
			else
			{
				votemode = 3;

				if (!mapsread)
				{
					mapsread = true;
					ls_value = ini_key_value ("vote.ini", "System", "MapList", "vote.lst");
					gsmod_ReadMaps(ls_value);
				}

				if (!currentmap)
				{
					currentmap = 1;
					nextinseq = 1;
				}

				if ((votemode == 3) && (currentmap))
				{
					ls_value = ini_key_value ("vote.ini", "Rotation", "RotationMode", "Sequential");

					if (Q_stricmp (ls_value, "Elimination") == 0)
					{
						ls_value = ini_key_value ("vote.ini", "Rotation", "ElimX", "1");
						elimvalue = atoi(ls_value);
						if (elimvalue <= elimx_count)
						{
							currentmap = nextinseq;
							nextinseq++;
							if (nextinseq >= nextmap)
								nextinseq = 1;
							elimx_count = 1;
						}
						else
						{
							nextinseq = currentmap;
							elimx_count++;
						}
						
						randomx_count = 0;
						groupx_count  = 0;
					}
					else if (Q_stricmp (ls_value, "Random") == 0)
					{
						ls_value = ini_key_value ("vote.ini", "Rotation", "RandomX", "1");
						randomvalue = atoi(ls_value);
						if (randomvalue <= randomx_count)
						{
							randomx_count = 1;
							randomvalue = (int)(rndnum(1, nextmap - 1));
							nextinseq = randomvalue;
						}
						else
						{
							currentmap = nextinseq;
							nextinseq++;
							if (nextinseq >= nextmap)
								nextinseq = 1;
							randomx_count++;
						}

						elimx_count = 0;
					}
					else
					{
						randomx_count = 0;
						elimx_count   = 0;
						groupx_count  = 0;

						currentmap = nextinseq;
						nextinseq++;
						if (nextinseq >= nextmap)
							nextinseq = 1;
					}
				}
			}
		}
	}

/*
	if ((!currentmap) && (votemode))
	{
		currentmap = 1;
		nextinseq = 1;

		ls_value = ini_key_value ("vote.ini", "System", "FirstMap", "First");
		if (Q_stricmp (ls_value, "Popular") == 0)
		{
			gsmod_CastVote(NULL, "$$Sequence$$");
		}
		else if (Q_stricmp (ls_value, "Random") == 0)
		{
			randomvalue = (int)(rndnum(1, nextmap - 1));
			nextinseq = randomvalue;
			gsmod_CastVote(NULL, "$$Sequence$$");
		}
		else
			gsmod_CastVote(NULL, "$$Sequence$$");
	}
*/
}

void gsmod_ReadMaps (char *filename)
{
	FILE	*text_file;	
	char	line1[80];
	char	line2[80];
	int		map=1;
	char	newfilename[MAX_QPATH] = "";
	int		i, islefn;

#ifdef _WIN32
	strcpy(newfilename, "bots\\");
#else
	strcpy(newfilename, "bots/");
#endif
	islefn = strlen(newfilename);
	for (i=0; filename[i]; i++)
		newfilename[i + islefn] = tolower(filename[i]);
	newfilename[i + islefn] = '\0';

	if (text_file = fopen(newfilename, "r"))	
	{
		// we successfully read a line from filename into text
		// ... read the remaining lines now			
		while ( fgets(line1, 80, text_file) )
		{	
			if (strlen(line1) > 19)
				line1[19] = 0;
			else
				line1[strlen(line1)-1] = 0;
			
			if(fgets(line2, 80, text_file))
			{
				if (strlen(line2) > 19)
					line2[19] = 0;
				else
					line2[strlen(line2)-1] = 0;

				sprintf(votes[map].levelmap, "%s", line1);
				sprintf(votes[map].levelname, "%s", line2);

				if (Q_stricmp (votes[map].levelmap, level.mapname) == 0)
				{
					if (votemode < 4)
					{
						currentmap = map;
						nextinseq = map;
					}
				}
			}
			else
				break;

			if (votemode > 2)
			{
				if (map > 50)
					break;
			}
			else
			{
				if (map > 17)
					break;
			}

			map++;
		}		// be good now ! ... close the file
		fclose(text_file);	
	}
	else
		votemode = 0;

	if (map == 0)
		nextmap = 0;
	else
		nextmap = map + 1;
}


qboolean gsmod_BannedIP (char *ip, char *filename)
{
	FILE		*text_file;	
	char		line1[80];
	qboolean	isbanned=false;
	char	newfilename[MAX_QPATH] = "";
	int		i, islefn;

#ifdef _WIN32
	strcpy(newfilename, "bots\\");
#else
	strcpy(newfilename, "bots/");
#endif
	islefn = strlen(newfilename);
	for (i=0; filename[i]; i++)
		newfilename[i + islefn] = tolower(filename[i]);
	newfilename[i + islefn] = '\0';

	if (text_file = fopen(newfilename, "r"))	
	{
		// we successfully read a line from "text.txt" into text
		// ... read the remaining lines now			
		while ( fgets(line1, 80, text_file) )
		{	
			line1[strlen(line1)-1] = 0;
			ip[strlen(line1)] = 0;
			if (Q_stricmp (ip, line1) == 0)
			{
				isbanned = true;
				break;
			}
		}		// be good now ! ... close the file
		fclose(text_file);	
	}

	return isbanned;
}

void ExitLevel (void);

void gsmod_CastVote (edict_t *ent, char *votemap)
{
	edict_t		*newmap;
	qboolean	isauto=false, isseq=false;
	int			n, maxvote, numplayers, minvote, vote=0, randomvalue;

	numplayers = gsmod_NumPlayers();

	if (Q_stricmp ("$$Sequence$$", votemap) == 0)
	{
		isseq  = true;
		isauto = true;
		vote   = nextinseq;
	}
	else
	{
		if (!ent->client)
			return;

		if (Q_stricmp ("Cancel Vote", votemap) == 0)
			return;

		if (Q_stricmp ("$$Automatic$$", votemap) == 0)
			isauto = true;

		if (votemode == 3)
			vote = nextinseq;
		else
		{
			if ((groupx) && (votemode == 4))
				vote = nextinseq;
			else
			{
				for (n=1; n < nextmap-1; n++)
				{
					if (Q_stricmp (votes[n].levelname, votemap) == 0)
						vote = n;
				}
			}
		}
	}

	if (isauto == false)
	{
		if (vote == 0)
		{
			gi.cprintf (ent, PRINT_HIGH, "Error selecting a map\n");
			return;
		}

		if (ent->client->pers.castvote > 0)
			votes[ent->client->pers.castvote].numvotes--;
		else
			votetotal++;

		if ((startvote == 0) && (numplayers > 1))
		{
			minvote = (int)(numplayers / 2) + 1;
			if (votetotal >= minvote)
			{
				startvote = level.time;
				nextwarn = startvote + 10.0;
			}
		}

		votes[vote].numvotes++;
		
		ent->client->pers.castvote = vote;
		ent->client->isvoting = false;
	}

	if ((votetotal >= numplayers) || (isauto == true))
	{
		if (votemode == 3)
			vote = nextinseq;
		else if ((votemode == 4) && (groupx > 0))
			vote = groupx;
		else
		{
			maxvote = 0;
			for (n=1; n < nextmap; n++)
			{
				if(votes[n].numvotes > maxvote)
				{
					maxvote = votes[n].numvotes;
					vote = n;
				}
			}
		}

		for (n=1; n < nextmap; n++)
			votes[n].numvotes = 0;

		if ((vote >= MAX_MAPS) || (vote < 1))
		{
			newmap = G_Find (NULL, FOFS(classname), "target_changelevel");
			if (!newmap)
			{	// the map designer didn't include a changelevel,
				// so create a fake ent that goes back to the same level
				newmap = G_Spawn ();
				newmap->classname = "target_changelevel";
				newmap->map = level.mapname;
			}
		}
		else
		{
			if (votemode == 4)
			{
				groupx = vote;
				gsmod_ReadMaps(votes[groupx].levelmap);

				if (currentmap == 0)
				{
					randomvalue = (int)(rndnum(1, nextmap - 1));
					nextinseq = randomvalue;
					currentmap = nextinseq;
				}
				else
				{
					currentmap = nextinseq;
					nextinseq++;
					if (nextinseq >= nextmap)
						nextinseq = 1;
				}
				
				vote = nextinseq;
			}

			newmap = G_Spawn ();
			newmap->classname = "target_changelevel";
			newmap->map = votes[vote].levelmap;
		}

		if ((strlen(newmap->map) < 1) || (!newmap->map))
			newmap->map = level.mapname;

		if ((votemode > 2) || (isseq))
		{
			gi.bprintf (PRINT_MEDIUM,"Now loading map %s\n", newmap->map);
			currentmap = vote;
		}
		else
			gi.bprintf (PRINT_MEDIUM,"Map %s won with %i vote(s)\n", newmap->map, maxvote);
		
		if (isseq)
		{
			level.intermissiontime = level.time;
			level.changemap = newmap->map;
			level.exitintermission = 1;		// go immediately to the next level
			ExitLevel ();
		}
		else
			BeginIntermission (newmap);
	}
	else
		gi.bprintf (PRINT_MEDIUM,"%i people have voted. Type 'vote' in console to cast a vote.\n", votetotal);
}


int vote_MenuCallback(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if (!ent->client)
		return 0;

	gsmod_CastVote (ent, ((menu_item_t *)selected->it)->itemtext);
	
	return 0;
}


void gsmod_Killmenu (edict_t *ent)
{
	if (!ent->client)
		return;

	if (ent->client->showmenu)
		UseMenu(ent, 0, true);
}


void gsmod_Vote (edict_t *ent, qboolean redisplay, char *parms)
{
	int		n, numplayers;
	arena_link_t *menulink;

	if (!ent->client)
		return;

	if (!redisplay)
	{
		gsmod_Killmenu(ent);
	}

	numplayers = gsmod_NumPlayers();

	if (numplayers > 4) 
	{
		if ((teams.team_score[0] < 10) && (teams.team_score[1] < 10))
		{
			gsutil_centerprint(ent, "%s", "One team must have at least\n10 points before a new map can\ncan be voted on.\n");
			return;
		}
	}

	if (!votemode)
	{
		gsutil_centerprint(ent, "%s", "Error reading map list.\nVoting has been disabled.\n");
		return;
	}

	if (votemode == 3)
	{
		gsmod_CastVote(ent, "$$Sequential$$");
		return;
	}
	else if (votemode == 4)
	{
		if (groupx)
		{
			gsmod_CastVote(ent, "$$Sequential$$");
			return;
		}
		menulink = CreateMenu(ent, "Vote for the next group");
	}
	else
		menulink = CreateMenu(ent, "Vote for the next map");

	for (n=1; n < nextmap-1; n++)
		AddMenuItem(menulink, votes[n].levelname, " ", votes[n].numvotes, &vote_MenuCallback);

	if (votemode < 4)
		AddMenuItem(menulink, "Automatic Choice", " ", votes[18].numvotes, &vote_MenuCallback);
	AddMenuItem(menulink, "Cancel Vote", NULL, -1, &vote_MenuCallback);
	FinishMenu(ent, menulink);

	ent->client->isvoting = true;
	ent->client->pers.startpause = level.time;
}

	
void gsmod_Rebind (edict_t *ent)
{
	if (!ent->client)
		return;

	stuffcmd(ent, "bind [ \"invprev\"\nbind ] \"invnext\"\n");
	stuffcmd(ent, "bind ENTER \"invuse\"\nbind BACKSPACE \"killmenu\"\n");

//	if (teamplay == 1)
//	{
//		stuffcmd(ent, "alias captain \"cmd class1\"\nalias nurse \"cmd class8\"\n");
//		stuffcmd(ent, "alias bodyguard \"cmd class2\"\nalias sniper \"cmd class3\"\n");
//		stuffcmd(ent, "alias soldier \"cmd class4\"\nalias berzerker \"cmd class5\"\n");
//		stuffcmd(ent, "alias infiltrator \"cmd class6\"\nalias kamikazee \"cmd class7\"\n");
//	}

	ent->client->pers.keysbound = true;

	return;
}


void gsmod_Help (edict_t *ent)
{
	char alias_list[500];
	int  j;

	if (!ent->client)
		return;

	j = sprintf (alias_list,      "To access the BotS main menu \n");
	j += sprintf (alias_list + j, "type 'menu' in the console.  \n");
	
	j += sprintf (alias_list + j, "Use the '[' and ']' to scroll\n");
	j += sprintf (alias_list + j, "through the menu options. Use\n");
	j += sprintf (alias_list + j, "the 'ENTER' key to select a  \n");
	j += sprintf (alias_list + j, "menu option.                 \n\n");

	j += sprintf (alias_list + j, "If for some reason a menu is \n");
	j += sprintf (alias_list + j, "stuck on the screen just type\n");
	j += sprintf (alias_list + j, "'menu' in the console and    \n");
	j += sprintf (alias_list + j, "select 'cancel'.             \n");

	gsutil_centerprint(ent, "%s", alias_list);
}


qboolean gsmod_HasArmor (edict_t *ent, char *armorname)
{
	int				index;
	gitem_t			*it;
	gitem_armor_t	*armorinfo;

	if (!ent->client)
		return false;

	it = FindItem (armorname);
	if (!it)
		return false;

	armorinfo = (gitem_armor_t *)it->info;

	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
		return false;

	return true;
}


void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);

void gsmod_LaserSightThink (edict_t *self)
{   
	vec3_t start, end, endp, offset;
	vec3_t forward,right,up;   
	trace_t tr;
   
	if (!self->owner)
		return;

	if (!self->owner->client)
		return;

	AngleVectors (self->owner->client->v_angle, forward, right, up);
	VectorSet(offset, 24 , 6, self->owner->viewheight-7);
	P_ProjectSource (self->owner->client, self->owner->s.origin, offset, forward, right, start);

	VectorMA(start,8192,forward,end);
	tr = gi.trace (start,NULL,NULL, end,self->owner,CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
	if (tr.fraction != 1) 
	{      
		VectorMA(tr.endpos,-4,forward,endp);
		VectorCopy(endp,tr.endpos);   
	}
	if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
	{
		if ((tr.ent->takedamage) && (tr.ent != self->owner)) 
		{
			self->s.skinnum = 1;      
		}   
	}   
	else      
		self->s.skinnum = 0;
	vectoangles(tr.plane.normal,self->s.angles);
	VectorCopy(tr.endpos,self->s.origin);   
	gi.linkentity (self);
	self->nextthink = level.time + 0.2;
}

void gsmod_LaserSight (edict_t *self)
{
	vec3_t  start,forward,right,end;   

	if (!self->client)
		return;
	
	if (teamplay == 1)
	{
		if (self->client->pers.player_class != 3)
		{
			gsutil_centerprint(self, "%s", "Only Sniper can use laser sight");
			return;
		}
	}
	else
		return;

	if (self->lasersight) 
	{      
		G_FreeEdict(self->lasersight);
		self->lasersight = NULL;      
		self->client->special = false;
		self->client->ps.fov = 90;
		return;
	}   

	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorSet(end,100 , 0, 0);
	G_ProjectSource (self->s.origin, end, forward, right, start);
	self->lasersight = G_Spawn ();   
	self->lasersight->owner = self;   
//	self->lasersight->movetype = MOVETYPE_NOCLIP;
	self->lasersight->solid = SOLID_NOT;   
	self->lasersight->classname = "lasersight";
	self->lasersight->s.modelindex = gi.modelindex ("models/objects/flash/tris.md2");
//	self->lasersight->s.skinnum = 0;   
//	self->lasersight->s.renderfx |= RF_FULLBRIGHT;
//	self->lasersight->s.renderfx |= RF_GLOW;
	self->lasersight->think = gsmod_LaserSightThink;   
	self->lasersight->nextthink = level.time + 0.2;

//	self->lasersight->s.effects |= EF_COLOR_SHELL;
//	self->lasersight->s.renderfx |= teamkeys[self->client->pers.team-1].keyeffect;

	self->client->special = true;
	self->client->specialtimer = level.time + 0.1;
}


void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void gsmod_Decoy (edict_t *ent)
{
	gitem_t	*it;
	edict_t	*dropped;

	if (!ent->client)
		return;

	if (ent->client->special)
		return;
	
	if (teamplay == 1)
	{
		if (ent->client->pers.player_class != 2)
		{
			gsutil_centerprint(ent, "%s", "Only Bodyguard can deploy a decoy");
			return;
		}
	}
	else
		return;

	if (ent->client->decoypromo)
	{
		gsutil_centerprint(ent, "%s", "You already have a decoy deployed!\n");
		return;
	}

	it = FindItemByClassname("key_promotion_decoy");
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "Error: Could not find a proper promotion key");
		return;
	}

	dropped = Drop_Item(ent, it);
	
	dropped->s.effects |= EF_COLOR_SHELL;
	dropped->s.renderfx |= teamkeys[ent->client->pers.team-1].keyeffect;
	dropped->s.modelindex = gi.modelindex ("models/monsters/commandr/head/tris.md2");

	dropped->owner = ent;
	dropped->touch = Grenade_Touch;
	dropped->dmg = 200;
	dropped->dmg_radius = 200;
	dropped->classname = "decoypromo";

	dropped->nextthink = level.time + 60;
	dropped->think = Grenade_Explode;

	ent->client->decoypromo = true;

//	ent->client->special = true;
//	ent->client->specialtimer = level.time + 20.0;
}

	
void gsmod_Rage (edict_t *ent)
{
	gitem_t  *it;

	if (!ent->client)
		return;

	if (ent->client->special)
		return;
	
	if (teamplay == 1)
	{
		if (ent->client->pers.player_class != 5)
		{
			gsutil_centerprint(ent, "%s", "Only Berzerker can go into a rage");
			return;
		}

		if (IsFemale(ent))
		{
			if (ent->health > 99)
			{
				gsutil_centerprint(ent, "%s", "You must have less than\n100 health for rage");
				return;
			}
		}
		else
		{
			if (ent->health > 74)
			{
				gsutil_centerprint(ent, "%s", "You must have less than\n75 health for rage");
				return;
			}
		}
	}
	else
		return;

	if (ent->client->keyindex > 0)
	{
		gsutil_centerprint(ent, "%s", "You cannot rage while\ncarrying the key!");
		return;
	}
	
	ent->client->special = true;
	ent->client->specialtimer = level.time + (ent->health / 7);
	it = FindItem("Invulnerability");
	ent->client->pers.inventory[ITEM_INDEX(it)]++;
	Use_Invulnerability (ent, it);
}
	

void gsmod_Scout (edict_t *ent, qboolean showall)
{
	int		 i, j;
	int		 teamdata[3][8];
	edict_t	 *player;
	char	 stats[500];

	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if (ent->client->pers.player_class != 1)
		{
			gsutil_centerprint(ent, "%s", "Only Captain can scout");
			return;
		}
	}
	else
		return;

	for (i=0 ; i<8 ; i++)
	{
		teamdata[0][i] = 0;
		teamdata[1][i] = 0;
		teamdata[2][i] = 0;
	}

	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client)
			continue; 

		if (player->client->observer)
			continue;

		teamdata[player->client->pers.team-1][player->client->pers.player_class-1]++;
	}

	if (numberteams == 2)
		j = sprintf(stats, "Class                   Female Male\n\n");
	else if (numberteams == 3)
		j = sprintf(stats, "Class                Blue Red Green\n\n");
	else
		return;

    // List classes
	if (numberteams == 2)
		j += sprintf(stats + j, "Captain                 %3i    %3i \n", teamdata[0][0], teamdata[1][0]);
	else if (numberteams == 3)
		j += sprintf(stats + j, "Captain              %3i  %3i  %3i \n", teamdata[0][0], teamdata[1][0], teamdata[2][0]);

	if (numberteams == 2)
		j += sprintf(stats + j, "Bodyguard               %3i    %3i \n", teamdata[0][1], teamdata[1][1]);
	else if (numberteams == 3)
		j += sprintf(stats + j, "Bodyguard            %3i  %3i  %3i \n", teamdata[0][1], teamdata[1][1], teamdata[2][1]);

	if (numberteams == 2)
		j += sprintf(stats + j, "Sniper                  %3i    %3i \n", teamdata[0][2], teamdata[1][2]);
	else if (numberteams == 3)
		j += sprintf(stats + j, "Sniper               %3i  %3i  %3i \n", teamdata[0][2], teamdata[1][2], teamdata[2][2]);

	if (numberteams == 2)
		j += sprintf(stats + j, "Soldier                 %3i    %3i \n", teamdata[0][3], teamdata[1][3]);
	else if (numberteams == 3)
		j += sprintf(stats + j, "Soldier              %3i  %3i  %3i \n", teamdata[0][3], teamdata[1][3], teamdata[2][3]);

	if (numberteams == 2)
		j += sprintf(stats + j, "Berzerker               %3i    %3i \n", teamdata[0][4], teamdata[1][4]);
	else if (numberteams == 3)
		j += sprintf(stats + j, "Berzerker            %3i  %3i  %3i \n", teamdata[0][4], teamdata[1][4], teamdata[2][4]);

	if (numberteams == 2)
		j += sprintf(stats + j, "Infiltrator             %3i    %3i \n", teamdata[0][5], teamdata[1][5]);
	else if (numberteams == 3)
		j += sprintf(stats + j, "Infiltrator          %3i  %3i  %3i \n", teamdata[0][5], teamdata[1][5], teamdata[2][5]);

	if (numberteams == 2)
		j += sprintf(stats + j, "Kamikazee               %3i    %3i \n", teamdata[0][6], teamdata[1][6]);
	else if (numberteams == 3)
		j += sprintf(stats + j, "Kamikazee            %3i  %3i  %3i \n", teamdata[0][6], teamdata[1][6], teamdata[2][6]);

	if (numberteams == 2)
		j += sprintf(stats + j, "Nurse                   %3i    %3i \n", teamdata[0][7], teamdata[1][7]);
	else if (numberteams == 3)
		j += sprintf(stats + j, "Nurse                %3i  %3i  %3i \n", teamdata[0][7], teamdata[1][7], teamdata[2][7]);

	gsutil_centerprint(ent, "%s", stats);

	if (showall)
	{
		for (i=0 ; i<game.maxclients ; i++)
		{
			player = g_edicts + 1 + i;
			if (!player->inuse || !player->client || (player == ent))
				continue; 
			if (player->client->observer)
				continue;
			if (player->client->pers.team != ent->client->pers.team)
				continue; 

			if (player->client->pers.scoutmsg)
				gsutil_centerprint(player, "%s", stats);
		}
	}
}


void gsmod_Tracker (edict_t *ent)
{
	int		 i, j;
	edict_t	 *player;
	char	 class_name[20];
	char	 stats[500];
	vec3_t	 v;
	float	 len;
	qboolean notalone=false;

	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if ((ent->client->pers.player_class != 1) && (ent->client->pers.player_class != 3))
		{
			gsutil_centerprint(ent, "%s", "Only Captain and Sniper can use tracker");
			return;
		}
	}

	
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] >= 1)
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 1;
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "No cells for tracking device.\n");
		return;
	}

	j = sprintf(stats, "Team Name Gender Class Health Range\n\n");
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client || (player == ent))
			continue; 
		if (player->client->observer)
			continue;
		if (ent->client->pers.team == player->client->pers.team)
			continue;

		// Get the class of player
		if (player->client->pers.player_class == 1)
			strcpy (class_name, CLASS1);
		else if (player->client->pers.player_class == 2)
			strcpy (class_name, CLASS2);
		else if (player->client->pers.player_class == 3)
			strcpy (class_name, CLASS3);
		else if (player->client->pers.player_class == 4)
			strcpy (class_name, CLASS4);
		else if (player->client->pers.player_class == 5)
			strcpy (class_name, CLASS5);
		else if (player->client->pers.player_class == 6)
			strcpy (class_name, CLASS6);
		else if (player->client->pers.player_class == 7)
			strcpy (class_name, CLASS7);
		else if (player->client->pers.player_class == 8)
			strcpy (class_name, CLASS8);
		else
			strcpy (class_name, "Error");
		// end class code

		notalone = true;
		VectorSubtract (ent->s.origin, player->s.origin, v);
		len = VectorLength (v);
		if (len < 2000)
		{
			if (IsFemale(player))
				j += sprintf(stats + j, "%1i) %14s %1s %9s %3d %5.0f\n", player->client->pers.team, player->client->pers.netname, "F", class_name, player->health, len);
			else
				j += sprintf(stats + j, "%1i) %14s %1s %9s %3d %5.0f\n", player->client->pers.team, player->client->pers.netname, "M", class_name, player->health, len);
			if (j > 450)
				break;
		}
	}
	if (notalone == true)
		gsutil_centerprint(ent, "%s", stats);
	else
		gsutil_centerprint(ent, "%s", "You're playing by yourself silly!");
}


void gsmod_Coordinates (edict_t *ent)
{
	if (!ent->client)
		return;

	gi.centerprintf(ent, "{%5.6f, %5.6f, %5.6f}", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
}

void gsmod_Earthquake (edict_t *self)
{
	int		i;
	edict_t	*e;

	if (self->last_move_time < level.time)
	{
		gi.positioned_sound (self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NONE, 0);
		self->last_move_time = level.time + 0.5;
	}

	for (i=0 ; i<game.maxclients ; i++)
	{
		e = g_edicts + 1 + i;
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (!e->groundentity)
			continue;
		if (e->client->observer)
			continue;

		e->groundentity = NULL;
		e->velocity[0] += crandom()* 150;
		e->velocity[1] += crandom()* 150;
		e->velocity[2] = self->speed * (100.0 / e->mass);
	}

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
}


void gsmod_Steal (edict_t *ent)
{
	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if (ent->client->pers.player_class != 6)
		{
			gsutil_centerprint(ent, "%s", "Only an Infiltrator can steal");
			return;
		}
	}

	if (ent->client->special2 == true)
	{
		ent->client->special2 = false;
		gsutil_centerprint(ent, "%s", "You will no longer steal\n");
	}
	else
	{
		ent->client->special2 = true;
		gsutil_centerprint(ent, "%s", "Stealing has been enabled\n");
	}
}
	
void gsmod_AutoRewire (edict_t *ent)
{
	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if (ent->client->pers.player_class != 7)
		{
			gsutil_centerprint(ent, "%s", "Only Kamikazee can autorewire");
			return;
		}
	}

	if (ent->client->special2 == true)
	{
		ent->client->special2 = false;
		gsutil_centerprint(ent, "%s", "Autorewire has been turned off\n");
	}
	else
	{
		ent->client->special2 = true;
		gsutil_centerprint(ent, "%s", "Autorewire has been activated\n");
	}
}

void gsmod_Suicide (edict_t *ent)
{
	gitem_t  *it;
	int		 grenades, damage, radius, numsaved, mod, modifier;
	edict_t	 *player = NULL;
	edict_t	 *bodyguard = NULL;
	qboolean isquad, foundbodyguard, isapoc;
//	float	 len;
//	vec3_t   v;

	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if (ent->client->pers.player_class != 7)
		{
			gsutil_centerprint(ent, "%s", "Only Kamikazee can suicide");
			return;
		}
	}

	if (ent->health < 50)
	{
		gsutil_centerprint(ent, "%s", "You need at least 50 health to suicide!");
		return;
	}

	gi.sound(ent, CHAN_VOICE, gi.soundindex("insane/insane6.wav"), 1, ATTN_NORM, 0);

	isquad = false;
	isapoc = false;
	
	grenades = ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))];

	modifier = ent->client->pers.classlevel;
	if (modifier == 0)
		modifier = 1;

	damage = grenades * modifier;
	radius = grenades * modifier + 100;

	if (ent->client->resp.score > 0)
	{
		if (grenades > 10)
		{
			if (ent->health > 74)
			{
				it = FindItem("Quad Damage");
				ent->client->pers.inventory[ITEM_INDEX(it)]++;
				Use_Quad (ent, it);
				isquad = true;
				damage = damage * 4;
			}

			if ((ent->health > 99) && (ent->client->pers.classlevel > 2) && (grenades > 50))
			{
				isapoc = true;
				damage = damage * 2;
				radius = radius * 8;
			}
		}
	}
	else
	{
		gsutil_centerprint(ent, "%s", "You need a positive score for\ndecent sucide damage");
		damage = 100;
		radius = 100;
	}

	if (teamplay_IsOffense(ent))
		radius = (int)(radius * 0.66);
	else
		radius = (int)(radius * 1.25);
	
	foundbodyguard = false;
	numsaved = 0;
	while ((player = findradius(player, ent->s.origin, radius)) != NULL)
	{
		if (!(player->svflags & SVF_MONSTER) && !player->client)
			continue;
		if (player->client->observer)
			continue;
		if (!player->takedamage)
			continue;
		if (player->health <= 0)
			continue;
		if (ent->client->pers.team == player->client->pers.team)
			continue;
		if (!visible(ent, player))
			continue;
		if (player->client->pers.player_class != 2)
		{
			numsaved++;
			continue;
		}

		if (foundbodyguard == false)
		{
			foundbodyguard = true;
			bodyguard = player;
		}
		else
			numsaved++;
	}

	if (foundbodyguard == true)
	{
		if (!numsaved)
			T_RadiusDamage (bodyguard, ent, damage, ent, radius, MOD_KAMISUICIDE);
		else
			teamplay_ClientKill(bodyguard, MOD_KAMISUICIDE);

		bodyguard->client->resp.score += numsaved;
		if (numsaved == 1)
			gi.bprintf (PRINT_MEDIUM,"Bodyguard %s saved %i player from Kamikazee %s!\n", bodyguard->client->pers.netname, numsaved, ent->client->pers.netname);
		else
			gi.bprintf (PRINT_MEDIUM,"Bodyguard %s saved %i players from Kamikazee %s!\n", bodyguard->client->pers.netname, numsaved, ent->client->pers.netname);
	}
	else
	{
		if (isapoc)
			mod = MOD_APOCSUICIDE;
		else
			mod = MOD_KAMISUICIDE;

		while ((player = findradius(player, ent->s.origin, radius)) != NULL)
		{
			if (!(player->svflags & SVF_MONSTER) && !player->client)
				continue;
			if (!player->takedamage)
				continue;
			if (player->client->observer)
				continue;
			if (player->health <= 0)
				continue;
			if (mod == MOD_KAMISUICIDE)
			{
				if (ent->client->pers.team == player->client->pers.team)
					continue;
			}
			if (!visible(ent, player))
				continue;

//			if (isquad == true)
//			{
//				VectorSubtract (ent->s.origin, player->s.origin, v);
//				len = VectorLength (v);
//				if (len < 101)
//					T_RadiusDamage (player, ent, damage * 4, ent, radius, mod);
//				else if (len < 201)
//					T_RadiusDamage (player, ent, damage * 2, ent, radius, mod);
//				else
//					T_RadiusDamage (player, ent, damage, ent, radius, mod);
//			}
//			else
				T_RadiusDamage (player, ent, damage, ent, radius, mod);
		}
	}

	// suicide
	if (isapoc)
	{
		if ((rand() & 20) >= 2)
			centerprint_all("Apocalypse!!!!");
		else
		{
			centerprint_all("Earthquake!!!!");

			if (!earthquake)
				earthquake = G_Spawn();
			earthquake->count = 5;
			earthquake->speed = 200;
			earthquake->svflags |= SVF_NOCLIENT;
			earthquake->think = gsmod_Earthquake;
			earthquake->noise_index = gi.soundindex ("world/quake.wav");
			earthquake->timestamp = level.time + earthquake->count;
			earthquake->nextthink = level.time + FRAMETIME;
			earthquake->activator = ent;
			earthquake->last_move_time = 0;
		}
	}

	ent->client->pers.kills++;
	teamplay_ClientKill(ent, MOD_KAMISUICIDE);
	weapon_grenade_fire (ent, true);
	ent->client->grenade_blew_up = true;
}



// Added function call in p_weapon.c to
//
//void Weapon_RocketLauncher (edict_t *ent)
//{
//	gsmod_Weapon_RocketLauncher(ent);
//}
//
void gsmod_Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	if (!ent->client)
		return;

	if (ent->client->pers.classlevel > 1)
	{
		if (ent->client->weapontoggle)
			Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
		else
		{
			if (ent->client->pers.classlevel > 2)
				Weapon_Generic (ent, 4, 8, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
			else
				Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
		}
	}
	else
		Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}


// Added function call in p_weapon.c to
//
//void Weapon_Blaster (edict_t *ent)
//{
//	gsmod_Weapon_Blaster(ent);
//}
//
void gsmod_Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	if (!ent->client)
		return;

	if (IsFemale(ent))
		Weapon_Generic (ent, 4, 5, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
	else
		Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}


// Added function call to g_cmds.c:
//
//void Cmd_Wave_f (edict_t *ent)
//{
//	..
//	case 2:
//		//gi.cprintf (ent, PRINT_HIGH, "taunt\n");
//		ent->s.frame = FRAME_taunt01-1;
//		ent->client->anim_end = FRAME_taunt17;
//		gsmod_Taunt(ent);
//		break;
//	...		
//}
//
void gsmod_Taunt (edict_t *ent)
{
	int		 i;
	edict_t	 *player;
	vec3_t	 v;
	float	 len;
	qboolean notalone=false;

	if (!ent->client)
		return;

	if (ent->client->tauntcount == 0)
	{
		ent->client->tauntcount++;
		ent->client->taunttimer = level.time;
	}
	else
	{
		ent->client->tauntcount++;
		if ((ent->client->tauntcount > 5) && (level.time < ent->client->taunttimer + 60.0))
		{
			if (ent->client->tauntcount > 6)
				return;
			else
			{
				gsmod_QueueCommand(ent, "kill");
				if (IsFemale(ent))
					gi.bprintf (PRINT_MEDIUM,"%s nagged herself to death\n", ent->client->pers.netname);
				else
					gi.bprintf (PRINT_MEDIUM,"%s scratched himself to death\n", ent->client->pers.netname);

				return;
			}
		}
		else if (level.time > ent->client->taunttimer + 60.0)
		{
			ent->client->tauntcount = 1;
			ent->client->taunttimer = level.time;
		}
	}

	if (IsFemale(ent))
		gi.bprintf (PRINT_MEDIUM,"%s nags: Guys are a bunch of losers!\n", ent->client->pers.netname);
	else
		gi.bprintf (PRINT_MEDIUM,"%s scratches: Man that feels good!\n", ent->client->pers.netname);

	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client || (player == ent))
			continue; 
		if (player->client->observer)
			continue;
		if ((teamplay == 1) && (player->client->pers.team == ent->client->pers.team))
			continue;

		VectorSubtract (ent->s.origin, player->s.origin, v);
		len = VectorLength (v);
		if (len <= 200)
		{
			notalone = true;
			if (IsFemale(ent))
			{
				if (!IsFemale(player))
				{
					gi.bprintf (PRINT_MEDIUM,"%s was nagged by %s!\n", player->client->pers.netname, ent->client->pers.netname);
					T_RadiusDamage (player, ent, 20, ent, 200, MOD_TAUNT);
				}
			}
			else
			{
				if (IsFemale(player))
				{
					gi.bprintf (PRINT_MEDIUM,"%s was disgusted by %s!\n", player->client->pers.netname, ent->client->pers.netname);
					T_RadiusDamage (player, ent, 20, ent, 200, MOD_TAUNT);
				}
			}
		}
	}
	if (notalone == false)
		gsutil_centerprint(ent, "%s", "Nobody saw you...\n");
}


//
// Change function in p_client.c to the following:
//
//void InitClientPersistant (gclient_t *client)
//{
//	teamplay_InitClientPersistant (client);	<---
//	gsmod_InitClientPersistant (client);
//}
//
void gsmod_InitClientPersistant (gclient_t *client)
{
	if (!client)
		return;

	if (teamplay == 1)
	{
		if ((client->pers.player_class != 4) && (client->pers.player_class != 1) && (client->pers.player_class != 5))
			client->pers.inventory[ITEM_INDEX(FindItem("Hook"))] = 1;
	}
	else
		client->pers.inventory[ITEM_INDEX(FindItem("Hook"))] = 1;
}


// gsmod_GetPlayerID: Finds the player closest to the viewer's viewpoint
edict_t *gsmod_GetPlayerID(edict_t *viewer)
{
	edict_t	*target = NULL,	*bestTarget = NULL;
	vec3_t targetDir, viewDir, diffDir;
	// note that setting bestAngleOffset to 0.5 initially means we will
	// return null and not display a name if no players are visible 
	// within about 60 degrees of the viewpoint
	float angleOffset, bestAngleOffset = 0.5;
	trace_t tr;

	if (!viewer->client)
		return NULL;

	while ((target = G_Find (target, FOFS(classname), "player")) != NULL) {
		// get a vector from viewer to target
		VectorSubtract(target->s.origin, viewer->s.origin, targetDir);

		// normalize the vector to the target and change to angular coordinates
		VectorNormalize(targetDir);
	
		// get a vector for the player's view angle
		AngleVectors(viewer->client->v_angle, viewDir, NULL, NULL);

		// angle difference from viewpoint to target
		_VectorSubtract(targetDir, viewDir, diffDir);

		// since both angles were normalized, cLayoutLengthgth of difference vector represents difference in angle.
		angleOffset = VectorLength(diffDir);

		if (angleOffset < bestAngleOffset) {
			// angle to target is smallest so far. check LOS between target and viewer
			tr = gi.trace (viewer->s.origin, NULL, NULL, target->s.origin, 
					viewer, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);
			if (tr.ent->client != NULL)	{
				// best so far
			 	bestAngleOffset = angleOffset;
				bestTarget = target;
			}
		}
	}
	return bestTarget;
}

// Version of GetPlayerID that works with non-players
edict_t *gsmod_GetEdictID(edict_t *viewer)
{
	vec3_t forward, endvec;
	trace_t tr;

	if (!viewer->client)
		return NULL;

	AngleVectors(viewer->client->v_angle, forward, NULL, NULL);
	VectorMA(viewer->s.origin, 8192, forward, endvec);

	tr = gi.trace (viewer->s.origin, NULL, NULL, endvec, viewer,
		MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

	if (tr.ent != g_edicts)	return tr.ent;
	return NULL;
}


//Appends the statusbar to a person's overlay
void gsmod_ShowPlayerID (edict_t *ent)
{
	edict_t	*player;

	if (!ent->client)
		return;

	player = gsmod_GetPlayerID(ent);

	if (player == NULL) 
		return; 

	teamplay_DisplayStatus (player, ent);
}


void gsmod_pre_target_laser_think (edict_t *self)
{
	target_laser_on (self);

	self->think = target_laser_think;
}



void gsmod_AlarmKill (edict_t *ent)
{
	qboolean hadalarm=false;
	
	if (!ent->client)
		return;

	if (ent->alarm1)
	{
		G_FreeEdict(ent->alarm1);
		hadalarm = true;
	}

	if (ent->alarm2)
	{
		G_FreeEdict(ent->alarm2);
		hadalarm = true;
	}

	if (ent->alarm3)
	{
		G_FreeEdict(ent->alarm3);
		hadalarm = true;
	}

	ent->alarm1 = NULL;
	ent->alarm2 = NULL;
	ent->alarm3 = NULL;

	if (hadalarm == true)
		gsutil_centerprint(ent, "%s", "All alarms have been removed\n");
	else
		gsutil_centerprint(ent, "%s", "You have to place an alarm first!\n");
}


void gsmod_LaserKill (edict_t *ent)
{
	if (!ent->client)
		return;

	if (ent->laserbeam) 
	{      
		ent->laserbeam->delay		= level.time;
		ent->laserbase->nextthink	= level.time;
		gsmod_pre_target_laser_think(ent->laserbeam);
		ent->laserbeam = NULL;
		ent->laserbase = NULL;

		if (ent->laserbeam2) 
		{
			ent->laserbeam2->delay		= level.time;
			ent->laserbase2->nextthink	= level.time;
			gsmod_pre_target_laser_think(ent->laserbeam2);
			ent->laserbeam2 = NULL;
			ent->laserbase2 = NULL;
		}
	}
	else
		gsutil_centerprint(ent, "%s", "You have to place a laser first!\n");
}


void gsmod_LaserToggle (edict_t *ent, qboolean toggle)
{
	int modifier = 1;
	vec3_t	 v;
	float	 len;
	qboolean lb_toofar;
	
	if (!ent->client)
		return;

	if (!ent->laserbeam)
	{
		gsutil_centerprint(ent, "%s", "You have to place a laser first\n");
		return;
	}
	
	if (toggle)
	{
		if (level.time < ent->laserdelay)
		{
			gsutil_centerprint(ent, "%s", "Your laser must cool off first!\n");
			return;
		}
		
		if (ent->client->pers.classlevel > 2)
			modifier = 2;

		lb_toofar = false;
		VectorSubtract (ent->s.origin, ent->laserbeam->s.origin, v);
		len = VectorLength (v);
		if (len < 1500)
		{
			if (!ent->laserbeam->delay)
			{
				ent->laserbeam->delay		= level.time + LASER_TIME * modifier;
				ent->laserbase->nextthink	= level.time + LASER_TIME * modifier;
				gsmod_pre_target_laser_think(ent->laserbeam);
				ent->laseron = level.time;
			}
		}
		else
			lb_toofar = true;

		if (ent->laserbeam2) 
		{
			VectorSubtract (ent->s.origin, ent->laserbeam2->s.origin, v);
			len = VectorLength (v);
			if (len < 1500)
			{
				if (!ent->laserbeam2->delay)
				{
					ent->laserbeam2->delay		= level.time + LASER_TIME * modifier;
					ent->laserbase2->nextthink	= level.time + LASER_TIME * modifier;
					gsmod_pre_target_laser_think(ent->laserbeam2);
					if (lb_toofar)
					{
						ent->laseron = level.time;
						gsutil_centerprint(ent, "%s", "Your second laser is fine, but you\nare too far from your first\nlaser to activate it!");
					}
				}
			}
			else
			{
				if (lb_toofar)
					gsutil_centerprint(ent, "%s", "Your are too far from both\nof your lasers to activate them!");
				else
					gsutil_centerprint(ent, "%s", "Your first laser is fine, but you\nare too far from your second\nlaser to activate it!");
			}
		}
		else
		{
			if (lb_toofar)
				gsutil_centerprint(ent, "%s", "You are too far from your laser\nto activate it!");
		}
	}
	else
	{
		ent->laserbeam->delay		= 0;
		ent->laserbase->nextthink	= 0;
		target_laser_off(ent->laserbeam);

		if (ent->laserbeam2)
		{
			ent->laserbeam2->delay		= 0;
			ent->laserbase2->nextthink	= 0;
			target_laser_off(ent->laserbeam2);
		}

		if (ent->laseron)
		{
			if ((level.time - ent->laseron) < 2.0)
				ent->laserdelay = level.time + 2.0;
			else
				ent->laserdelay = level.time + (level.time - ent->laseron);
			ent->laseron = 0;
		}
	}
}


void gsmod_PlaceLaser (edict_t *ent)
{
	edict_t		*self, *grenade;	
	vec3_t		forward, wallp;	
	trace_t		tr;
	qboolean	secondlaser=false;
	int			laser_colour[] = {	0xf3f3f1f1,		// blue								
									0xf2f2f0f0,		// red								
									0xd0d1d2d3,		// green
									0xdcdddedf,		// yellow
									0xe0e1e2e3		// bitty yellow strobe	
								};	
									
	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if (ent->client->pers.player_class != 2)
		{
			gsutil_centerprint(ent, "%s", "Only the Bodyguard can place a laser\n");
			return;
		}
	}

	if (ent->laserbeam)
	{
		if (ent->client->pers.classlevel > 1)
		{
			if (ent->laserbeam2)
			{
				gsutil_centerprint(ent, "%s", "You have already built two lasers\n");
				return;
			}
			else
				secondlaser = true;
		}
		else
		{
			gsutil_centerprint(ent, "%s", "You have already built a laser\n");
			return;
		}
	}
	
	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))	   
		return;	
	
	// cells for laser ?
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < CELLS_FOR_LASER)
	{ 		
		gsutil_centerprint(ent, "%s", "Not enough cells for laser.\n");
		return;	
	}

	// Setup "little look" to close wall	
	VectorCopy(ent->s.origin,wallp);

	// Cast along view angle
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);	// Setup end point
	wallp[0]=ent->s.origin[0]+forward[0]*50;
	wallp[1]=ent->s.origin[1]+forward[1]*50;
	wallp[2]=ent->s.origin[2]+forward[2]*50;  	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);
	
	if (Q_stricmp (tr.ent->classname, "worldspawn") != 0)
	{
	 	gi.cprintf (ent, PRINT_HIGH, "You cannot attach a laser here.\n");
		return;	
	}

	
	// Line complete ? (ie. no collision)	
	if (tr.fraction == 1.0)	
	{
	 	gi.cprintf (ent, PRINT_HIGH, "Too far from wall.\n");		
		return;	
	}	
	
	// Hit sky ?
	if (tr.surface)		
		if (tr.surface->flags & SURF_SKY)			
			return;
	
	// Ok, lets stick one on then ...
	gi.cprintf (ent, PRINT_HIGH, "Laser attached.\n");
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= CELLS_FOR_LASER;

	// -----------
	// Setup laser
	// -----------
	self = G_Spawn();

	self -> movetype		= MOVETYPE_NONE;
	self -> solid			= SOLID_NOT;
	self -> s.renderfx		= RF_BEAM|RF_TRANSLUCENT;
	self -> s.modelindex	= 1;			// must be non-zero
	self -> s.sound			= gi.soundindex ("world/laser.wav");
	self -> classname		= "laser_yaya";
	self -> s.frame			= 2;	// beam diameter
  	self -> owner			= ent;
	self -> s.skinnum		= laser_colour[ent->client->pers.team-1];
  	self -> dmg				= LASER_DAMAGE;
	self -> think			= gsmod_pre_target_laser_think;

	self -> delay		= 0;
	self -> nextthink	= 0;

	if (secondlaser)
		ent->laserbeam2				= self;
	else
	{
		ent->laserbeam		= self;
		ent->laserdelay		= 0;
		ent->laseron		= 0;
	}
	
	// Set orgin of laser to point of contact with wall
	VectorCopy(tr.endpos, self->s.origin);

	// convert normal at point of contact to laser angles
	vectoangles(tr.plane.normal, self -> s.angles);

	// setup laser movedir (projection of laser)
	G_SetMovedir (self->s.angles, self->movedir);

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	// link to world
	gi.linkentity (self);

	// start off ...
	target_laser_off (self);

	// ... but make automatically come on
//	self -> nextthink = level.time + 2;

	// Add a mount for laser
	grenade = G_Spawn();	
	VectorClear (grenade->mins);	
	VectorClear (grenade->maxs);
	VectorCopy (tr.endpos, grenade->s.origin);
	vectoangles(tr.plane.normal,grenade -> s.angles);
	//grenade -> movetype		= MOVETYPE_NONE;	
	//grenade -> clipmask		= MASK_SHOT;
	grenade -> solid		= SOLID_BBOX;
	grenade -> s.modelindex	= gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade -> owner		= self;	
//	grenade -> nextthink	= level.time + LASER_TIME;
	grenade -> think		= G_FreeEdict;	
	grenade -> classname	= "laser_base";
	grenade -> health		= 100;
	gi.linkentity (grenade);

	if (secondlaser)
		ent->laserbase2 = grenade;
	else
		ent->laserbase = grenade;

	ent->client->pausetimer = level.time;
}

	
void gsmod_LevelReset (void)
{
	int		i, numplayers;
	edict_t	*newmap, *player;
	
	numplayers = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client)
			continue; 

		numplayers++;
	}

	if (numplayers == 0)
	{
		// reset the level
		newmap = G_Spawn ();
		newmap->classname = "target_changelevel";
		newmap->map = level.mapname;

		level.intermissiontime = level.time;
		level.changemap = newmap->map;
		level.exitintermission = 1;
	}
}
	

void gsmod_AlarmThink (edict_t *self)
{
	int		i, j=0;
	edict_t *player=NULL;
	char	class_name[20];
	char	alarmname[30];
	char	stats[500];
	char	message[500];
	qboolean	hasflag;
	
	if (!self->owner)
		return;
	
	if (!self->owner->client)
		return;

	hasflag = false;
	while ((player = findradius(player, self->s.origin, 300)) != NULL)
	{
		if (!(player->svflags & SVF_MONSTER) && !player->client)
			continue;
		if (!player->client)
			continue;
		if (player->client->observer)
			continue;
		if (!player->takedamage)
			continue;
		if (player->health <= 0)
			continue;
		if (!visible(self, player))
			continue;       // The alarm can't see the player
		if (self->owner->client->pers.team == player->client->pers.team)
			continue;
		if (player->client->pers.player_class == 6)
			continue;

		// Get the class of player
		if (player->client->pers.player_class == 1)
			strcpy (class_name, CLASS1);
		else if (player->client->pers.player_class == 2)
			strcpy (class_name, CLASS2);
		else if (player->client->pers.player_class == 3)
			strcpy (class_name, CLASS3);
		else if (player->client->pers.player_class == 4)
			strcpy (class_name, CLASS4);
		else if (player->client->pers.player_class == 5)
			strcpy (class_name, CLASS5);
		else if (player->client->pers.player_class == 6)
			strcpy (class_name, CLASS6);
		else if (player->client->pers.player_class == 7)
			strcpy (class_name, CLASS7);
		else if (player->client->pers.player_class == 8)
			strcpy (class_name, CLASS8);
		else
			strcpy (class_name, "Error");
		// end class code

		j += sprintf(stats + j, "%s %s\n", class_name, player->client->pers.netname);

		if (player->client->keyindex > 0)
			hasflag = true;
	}

	if (j)
	{
		if (strcmp(self->classname,"alarm1") == 0)
		{
			if (hasflag)
			{
				if (self->owner->flagalarm1name[0])
					sprintf(alarmname, "%s", self->owner->flagalarm1name);
				else
					sprintf(alarmname, "%s", "Flag Alarm #1");
			}
			else
			{
				if (self->owner->alarm1name[0])
					sprintf(alarmname, "%s", self->owner->alarm1name);
				else
					sprintf(alarmname, "%s", "Alarm #1");
			}
		}
		else if (strcmp(self->classname,"alarm2") == 0)
		{
			if (hasflag)
			{
				if (self->owner->flagalarm2name[0])
					sprintf(alarmname, "%s", self->owner->flagalarm2name);
				else
					sprintf(alarmname, "%s", "Flag Alarm #2");
			}
			else
			{
				if (self->owner->alarm2name[0])
					sprintf(alarmname, "%s", self->owner->alarm2name);
				else
					sprintf(alarmname, "%s", "Alarm #2");
			}
		}
		else if (strcmp(self->classname,"alarm3") == 0)
		{
			if (hasflag)
			{
				if (self->owner->flagalarm3name[0])
					sprintf(alarmname, "%s", self->owner->flagalarm3name);
				else
					sprintf(alarmname, "%s", "Flag Alarm #3");
			}
			else
			{
				if (self->owner->alarm3name[0])
					sprintf(alarmname, "%s", self->owner->alarm3name);
				else
					sprintf(alarmname, "%s", "Alarm #3");
			}
		}

		sprintf(message, "%s\nalarm was tripped by:\n%s\n", alarmname, stats);

		if (Q_stricmp (message, self->owner->alarmmsg) != 0)
		{
			sprintf(self->owner->alarmmsg, "%s", message);
			gsutil_centerprint(self->owner, "%s", message);
			self->owner->alarmclear = level.time + 5.0;
		}
		else
		{
			if (level.time > self->owner->alarmclear)
			{
				gsutil_centerprint(self->owner, "%s", message);
				self->owner->alarmclear = level.time + 5.0;
			}
			else
			{
				self->nextthink = level.time + .2;
				return;
			}
		}

		for (i=0 ; i<game.maxclients ; i++)
		{
			player = g_edicts + 1 + i;
			if (!player->inuse || !player->client || (player == self->owner))
				continue; 
			if (player->client->observer)
				continue;
			if (player->client->pers.team != self->owner->client->pers.team)
				continue; 
			if ((player->client->pers.player_class == 1) || (player->client->pers.playmode == 0))
			{
				gsutil_centerprint(player, "%s", message);
				sprintf(player->alarmmsg, "%s", message);
			}
		}
	}

	self->nextthink = level.time + .2;
}


void gsmod_NameAlarm (edict_t *ent, int alarmnumber, qboolean withflag)
{
	char	*s;

	if (!ent->client)
		return;

	s = gi.args();

	s[29] = 0;
	if (withflag)
	{
		if (alarmnumber == 1)
			sprintf(ent->flagalarm1name, "%s", s);
		else if (alarmnumber == 2)
			sprintf(ent->flagalarm2name, "%s", s);
		else if (alarmnumber == 3)
			sprintf(ent->flagalarm3name, "%s", s);

		gsutil_centerprint(ent, "%s", "Flag alarm has been named.\n");
	}
	else
	{
		if (alarmnumber == 1)
			sprintf(ent->alarm1name, "%s", s);
		else if (alarmnumber == 2)
			sprintf(ent->alarm2name, "%s", s);
		else if (alarmnumber == 3)
			sprintf(ent->alarm3name, "%s", s);

		gsutil_centerprint(ent, "%s", "Alarm has been named.\n");
	}
}


void gsmod_SendAlarm (edict_t *ent)
{
	int		i;
	edict_t	*player;

	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if ((ent->client->pers.player_class != 3) && (ent->client->pers.player_class != 1))
		{
			gsutil_centerprint(ent, "%s", "Only the Captain and Sniper can send\nan alarm\n");
			return;
		}
	}

	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue;
		if (player->client->observer)
			continue;

		if ((!player->client->showmenu) && (player->client->pers.alarmmsg))
		{
			if (ent->alarmmsg)
				gi.centerprintf(player, "Alarm from %s:\n\n%s\n", ent->client->pers.netname, ent->alarmmsg);
			else
				gi.centerprintf(player, "Alarm from %s:\n\nAll clear!\n", ent->client->pers.netname);
		}
	}
}



void gsmod_PlaceAlarm (edict_t *ent)
{
	edict_t		*grenade;	
	vec3_t		forward, wallp;	
	trace_t		tr;
	int			alarmnumber;
									
	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if (ent->client->pers.player_class != 3)
		{
			gsutil_centerprint(ent, "%s", "Only the Sniper can place an alarm\n");
			return;
		}
	}

	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))	   
		return;	
	
	// cells for alarm ?
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < CELLS_FOR_LASER)
	{ 		
		gsutil_centerprint(ent, "%s", "Not enough cells for alarm.\n");
		return;	
	}

	alarmnumber = 1;
	if (ent->alarm1)
	{
		alarmnumber = 2;
		if (ent->alarm2)
		{
			alarmnumber = 3;
			if (ent->alarm3)
			{
				gsutil_centerprint(ent, "%s", "You have set all 3 alarms\n");
				return;
			}
		}
	}

	// Setup "little look" to close wall	
	VectorCopy(ent->s.origin,wallp);

	// Cast along view angle
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);	// Setup end point
	wallp[0]=ent->s.origin[0]+forward[0]*50;
	wallp[1]=ent->s.origin[1]+forward[1]*50;
	wallp[2]=ent->s.origin[2]+forward[2]*50;  	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);
	
	if (Q_stricmp (tr.ent->classname, "worldspawn") != 0)
	{
	 	gi.cprintf (ent, PRINT_HIGH, "You cannot attach an alarm here.\n");
		return;	
	}

	// Line complete ? (ie. no collision)	
	if (tr.fraction == 1.0)	
	{
	 	gi.cprintf (ent, PRINT_HIGH, "Too far from wall.\n");		
		return;	
	}	
	
	// Hit sky ?
	if (tr.surface)		
		if (tr.surface->flags & SURF_SKY)			
			return;
	
	// Ok, lets stick one on then ...
	gi.cprintf (ent, PRINT_HIGH, "Alarm #%i attached.\n", alarmnumber);
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= CELLS_FOR_LASER;

	// Add a mount for alarm
	grenade = G_Spawn();	
	VectorClear (grenade->mins);	
	VectorClear (grenade->maxs);
	VectorCopy (tr.endpos, grenade->s.origin);
	vectoangles(tr.plane.normal,grenade -> s.angles);
	//grenade -> movetype		= MOVETYPE_NONE;	
	//grenade -> clipmask		= MASK_SHOT;
	grenade -> solid		= SOLID_BBOX;
	grenade -> s.modelindex	= gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade -> owner		= ent;	
	grenade -> nextthink	= level.time + .3;
	grenade -> think		= gsmod_AlarmThink;	
	grenade -> health		= 100;

	if (alarmnumber == 1)
		grenade->classname = "alarm1";
	else if (alarmnumber == 2)
		grenade->classname = "alarm2";
	else if (alarmnumber == 3)
		grenade->classname = "alarm3";

	gi.linkentity (grenade);

	if (alarmnumber == 1)
		ent->alarm1 = grenade;
	else if (alarmnumber == 2)
		ent->alarm2 = grenade;
	else if (alarmnumber == 3)
		ent->alarm3 = grenade;

	ent->client->pausetimer = level.time;
}



qboolean gsmod_PlayerReset (edict_t *ent, qboolean respawn)
{
	if (!ent->client)
	{
		if (teamplay == 0)
			return true;
		else
			return false;
	}

	if (!respawn)
	{
		if (teamplay == 0)
			ent->client->pers.startlevel = true;
		else
			ent->client->pers.startlevel = false;

		if (ent->client->pers.castvote > 0)
		{
			votes[ent->client->pers.castvote].numvotes--;
			votetotal--;

			if (votetotal <= 0)
			{
				gsmod_ClearVotes();
				gi.bprintf (PRINT_MEDIUM, "All voting has been reset.\n");
			}
		}

		if (ent->client->pers.impeachvote)
		{
			ent->client->pers.impeachvote = false;
			teams.team_impeachvotes[ent->client->pers.team-1]--;

			if (teams.team_impeachvotes[ent->client->pers.team-1] <= 0)
				teamplay_ResetImpeach(ent->client->pers.team);
		}

		ent->client->pers.team = 0;
		ent->client->pers.player_class = 0;
		ent->client->pers.player_nextclass = 0;
		ent->client->pers.classlevel = 0;
		ent->client->pers.showhelp = false;
		ent->client->pers.keysbound = false;
		ent->client->pers.hasskins = true;
		ent->client->pers.castvote = 0;
		ent->client->isvoting = false;
		ent->client->pers.impeachvote = false;
		ent->client->pers.badsuicide = 0;
		ent->client->pers.respawnpause = 0;
		ent->client->pers.warning = true;
		ent->client->pers.customwav = true;
		ent->client->pers.speed_modifier = 0;
		ent->client->pers.gravity_modifier = 0;

		teamplay_ResetPlayer(ent, true);

		ent->stuffskin = false;
		ent->client->observer = true;
	}
	else
	{
		if ((teamplay == 1) && (ent->client->pers.player_class == 1))
			teamplay_DropPromote(ent);

		ent->client->observer = false;
	}

	ent->client->chasecam = NULL;

	ent->client->special = false;
	ent->client->special2 = false;
	ent->client->poisontimer = 0;
	ent->client->voicetimer = 0;
	ent->client->killtimer = 0;
	ent->client->clustermode = false;
	ent->client->lifelink = false;
	ent->client->quadcloak = false;
	ent->createpause = 0;

	if (ent->flashlight)
		FL_make(ent);

	if (ent->client->hook)
	{
		hook_reset(ent->client->hook);
		ent->client->hook = NULL;
	}
	
	if (ent->lasersight) 
	{      
		G_FreeEdict(ent->lasersight);
		ent->lasersight = NULL;      
	}   

	if (ent->laserbeam) 
	{      
		ent->laserbeam->delay		= level.time;
		ent->laserbase->nextthink	= level.time;
		gsmod_pre_target_laser_think(ent->laserbeam);
		ent->laserbeam = NULL;
		ent->laserbase = NULL;
	}   

	if (ent->laserbeam2) 
	{      
		ent->laserbeam2->delay		= level.time;
		ent->laserbase2->nextthink	= level.time;
		gsmod_pre_target_laser_think(ent->laserbeam2);
		ent->laserbeam2 = NULL;
		ent->laserbase2 = NULL;
	}   

	ent->laseron = 0;
	ent->laserdelay = 0;
	ent->menudisplay = 0;

	if (ent->conquer)
	{
		gi.bprintf (PRINT_MEDIUM,"Capture pad has been released!\n");
		ent->conquer->conquer = NULL;
	}
	ent->conquer = NULL;

	gsmod_BlowGrenades(ent);

	return (!ent->client->pers.startlevel);
}

void gsmod_ModStatus(edict_t *ent)
{
	char	 stats[500];
	int		 j;

	if (!ent->client)
		return;

	j=0;

	if (votemode == 0)
		j += sprintf(stats + j, "Invalid Map List\n");
	else if (votemode == 1)
		j += sprintf(stats + j, "Classic Voting\n");
	else if (votemode == 2)
		j += sprintf(stats + j, "Standard Voting\n");
	else if (votemode == 3)
		j += sprintf(stats + j, "Sequential Map Rotation\n");
	else if (votemode == 4)
		j += sprintf(stats + j, "Group Map Rotation\n");

	if ((votemode == 2) || (votemode == 3))
	{
		if (randomx_count)
			j += sprintf(stats + j, "Random Mode\n\n");
		else if (elimx_count)
			j += sprintf(stats + j, "Elimination Mode\n\n");

		if (Q_stricmp (votes[currentmap].levelmap, level.mapname) == 0)
			j += sprintf(stats + j, "Current map: %s\n", votes[currentmap].levelname);
		else
			j += sprintf(stats + j, "Current map: %s\n", level.mapname);
			
		j += sprintf(stats + j, "Next map: %s\n", votes[nextinseq].levelname);
	}

	gsutil_centerprint(ent, "%s", stats);
}

qboolean gsmod_CheckWeapon(int daweapon)
{
	if (daweapon == MOD_BLASTER)
		return true;
	if (daweapon == MOD_SHOTGUN)
		return true;
	if (daweapon == MOD_SSHOTGUN)
		return true;
	if (daweapon == MOD_MACHINEGUN)
		return true;
	if (daweapon == MOD_CHAINGUN)
		return true;
	if (daweapon == MOD_ROCKET)
		return true;
	if (daweapon == MOD_R_SPLASH)
		return true;
	if (daweapon == MOD_HYPERBLASTER)
		return true;
	if (daweapon == MOD_RAILGUN)
		return true;
	if (daweapon == MOD_GRENADE)
		return true;
	if (daweapon == MOD_G_SPLASH)
		return true;
	if (daweapon == MOD_HANDGRENADE)
		return true;
	if (daweapon == MOD_HG_SPLASH)
		return true;

	return false;
}

void gsmod_LightsToggle(qboolean ab_toggle)
{
    if (ab_toggle)
    {
		// 0 normal
		gi.configstring(CS_LIGHTS+0, "m");

		// 1 FLICKER (first variety)
		gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");

		// 2 SLOW STRONG PULSE
		gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

		// 3 CANDLE (first variety)
		gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

		// 4 FAST STROBE
		gi.configstring(CS_LIGHTS+4, "mamamamamama");

		// 5 GENTLE PULSE 1
		gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");

		// 6 FLICKER (second variety)
		gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");

		// 7 CANDLE (second variety)
		gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");

		// 8 CANDLE (third variety)
		gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

		// 9 SLOW STROBE (fourth variety)
		gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");

		// 10 FLUORESCENT FLICKER
		gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");

		// 11 SLOW PULSE NOT FADE TO BLACK
		gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

		//JR 3/26/98
		//12 Wierd flashing
		gi.configstring(CS_LIGHTS+12, "ahsbexcbkxerswaibldcwersxa");

		lights = true;
    }
    else
    {
        // 0 normal
        gi.configstring(CS_LIGHTS+0, "a");

        // 1 FLICKER (first variety)
        gi.configstring(CS_LIGHTS+1, "a");

        // 2 SLOW STRONG PULSE
        gi.configstring(CS_LIGHTS+2, "a");

        // 3 CANDLE (first variety)
        gi.configstring(CS_LIGHTS+3, "a");

        // 4 FAST STROBE
        gi.configstring(CS_LIGHTS+4, "a");

        // 5 GENTLE PULSE 1
        gi.configstring(CS_LIGHTS+5,"a");

        // 6 FLICKER (second variety)
        gi.configstring(CS_LIGHTS+6, "a");

        // 7 CANDLE (second variety)
        gi.configstring(CS_LIGHTS+7, "a");

        // 8 CANDLE (third variety)
        gi.configstring(CS_LIGHTS+8, "a");

        // 9 SLOW STROBE (fourth variety)
        gi.configstring(CS_LIGHTS+9, "a");

        // 10 FLUORESCENT FLICKER
        gi.configstring(CS_LIGHTS+10, "a");

        // 11 SLOW PULSE NOT FADE TO BLACK
        gi.configstring(CS_LIGHTS+11, "a");

        //JR 3/26/98
        //12 Wierd flashing
        gi.configstring(CS_LIGHTS+12, "a");

   		lights = false;
    }
}

//
// Change function in p_client.c to the following:
//
//void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
//{
//	gsmod_ClientObituary(self, inflictor, attacker);
//}
//
void gsmod_ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod, li_random;
	char		*message;
	char		*message2;
	qboolean	ff, lostflag, wasnear;
	edict_t		*nearflag=NULL;
	vec3_t		origin;

	if (!self->client)
		return;

	li_random = rand() & 4;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	lostflag = false;
	if (deathmatch->value || coop->value)
	{
		if (self->client->pers.startlevel == false)
			return;

		if ((self->client->keyindex > 0) && (teamplay == 1))
		{
			gi.bprintf (PRINT_MEDIUM,"%s Lost the %s!\n", self->client->pers.netname, teamkeys[self->client->keyindex-1].keyname);
			self->s.modelindex3 = 0;
			teamplay_SpawnKey(self, self->client->keyindex, false, true);
			if (self->client->keyindex == 1)
				self->client->pers.inventory[ITEM_INDEX(FindItemByClassname("key_blueteam"))] = 0;
			else
				self->client->pers.inventory[ITEM_INDEX(FindItemByClassname("key_redteam"))] = 0;
			self->client->keyindex = 0;
			lostflag = true;

		}
		
		gsmod_PlayerReset(self, true);

		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "!";

		switch (mod)
		{
			case MOD_SUICIDE:
				message = "suicides";
				break;
			case MOD_FALLING:
				if (IsFemale(self))
					message = "broke a nail, and her neck";
				else
				{
					if (li_random == 4)
						message = "validated Newton's Theory";
					else if (li_random == 3)
						message = "has fallen and can't get up!";
					else if (li_random == 2)
						message = "fell and forgot to bounce";
					else
						message = "took a flying leap";
				}
				break;
			case MOD_CRUSH:
				message = "was squished";
				break;
			case MOD_WATER:
				if (li_random == 4)
					message = "sank like a rock";
				else if (li_random == 3)
					message = "gargled and forgot to spit";
				else if (li_random == 2)
					message = "visited Davey Jones'locker";
				else
					message = "was lost at sea";
				break;
			case MOD_SLIME:
				message = "melted";
				break;
			case MOD_LAVA:
				if (IsFemale(self))
					message = "took a hot bath";
				else
					message = "screamed like a little girl";
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
					if (IsFemale(self))
						message = "tripped on her own grenade";
					else
						message = "tripped on his own grenade";
					break;
				case MOD_R_SPLASH:
					if (IsFemale(self))
						message = "blew herself up";
					else
						message = "blew himself up";
					break;
				case MOD_POISON:
					message = "was fatally poisoned";
					break;
				case MOD_BFG_BLAST:
					message = "should have used a smaller gun";
					break;
				case MOD_RAGE:
					if (IsFemale(self))
						if (li_random > 2)
							message = "sacrificed herself for her team";
						else
							message = "was so mad she died";
					else
						if (li_random > 2)
							message = "sacrificed himself for his team";
						else
							message = "was so mad he died";
					break;
				default:
					if (teamplay == 1)
					{
						if (IsFemale(self))
							message = "did her team a favor";
						else
							message = "did his team a favor";
					}
					else
					{
						if (IsFemale(self))
							message = "killed herself";
						else
							message = "killed himself";
					}
					break;
			}
		}

		if (message)
		{
			if (mod != MOD_POISON)
			{
				gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
				if (deathmatch->value)
				{
					self->client->pers.kills--;
					self->client->forcekill = true;
				}
			}

			if (self->client->resp.score < 0)
			{
				if (self->client->pers.classlevel > 0)
					gi.bprintf (PRINT_MEDIUM,"%s was demoted to Level 0 for a negative frag count!\n", self->client->pers.netname);
				gsutil_centerprint(self, "%s", "You will need to get back into\npositive frags before you can\nbe promoted!\n");
				self->client->pers.classlevel = 0;
			}
			self->enemy = NULL;
			return;
		}
		
		self->enemy = attacker;
		self->client->forcekill = false;

		if (strcmp(inflictor->classname,"laser_yaya") == 0)
		{
			if (inflictor->owner)
			{
				if (inflictor->owner->client)
				{
					inflictor->owner->client->pers.kills++;

					if(self->client->pers.player_class == 1)
					{
						gi.bprintf (PRINT_MEDIUM,"%s's laser sliced Team Captain %s!\n", inflictor->owner->client->pers.netname, self->client->pers.netname);

						if (inflictor->owner->client->pers.team != self->client->pers.team)
						{
							if (teamplay_HasLeader(inflictor->owner->client->pers.team))
							{
								teams.team_score[inflictor->owner->client->pers.team-1] += 1;
								teams.team_promos[inflictor->owner->client->pers.team-1]++;
							}
							inflictor->owner->client->resp.score++;
						}
					
					}
					else
					{
						if (li_random == 4)
							gi.bprintf (PRINT_MEDIUM,"%s was clotheslined by %s's laser!\n", self->client->pers.netname, inflictor->owner->client->pers.netname);
						else if (li_random == 3)
							gi.bprintf (PRINT_MEDIUM,"%s's laser bisected %s!\n", inflictor->owner->client->pers.netname, self->client->pers.netname);
						else if (li_random == 2)
							gi.bprintf (PRINT_MEDIUM,"%s's laser said 'How do you do?' to %s!\n", inflictor->owner->client->pers.netname, self->client->pers.netname);
						else
							gi.bprintf (PRINT_MEDIUM,"%s's laser sliced through %s!\n", inflictor->owner->client->pers.netname, self->client->pers.netname);
					}

					if (teamplay == 1)
						teams.team_frags[inflictor->owner->client->pers.team - 1]++;

					if (lostflag)
						inflictor->owner->client->resp.score++;
					else
					{
						VectorCopy(self->s.origin, origin);
						while ((nearflag = findradius(nearflag, origin, 500)) != NULL)
						{
							if (inflictor->owner->client->pers.team == 1)
							{
								if (Q_stricmp (nearflag->classname, "key_blueteam") == 0)
								{
									inflictor->owner->client->resp.score++;
									break;
								}
							}
							else if (inflictor->owner->client->pers.team == 2)
							{
								if (Q_stricmp (nearflag->classname, "key_redteam") == 0)
								{
									inflictor->owner->client->resp.score++;
									break;
								}
							}
						}
					}
				}
				else
					gi.bprintf (PRINT_MEDIUM,"%s was sliced up!\n", self->client->pers.netname);
			}
			else
				gi.bprintf (PRINT_MEDIUM,"%s was sliced up!\n", self->client->pers.netname);
			return;
		}
		else if (attacker && attacker->client)
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
					if (li_random == 3)
						message = "recieved a lethal dose of lead from";
					else if (li_random == 2)
						message = "does the chaingun cha-cha with";
					else if (li_random == 3)
						message = "was ventilated by";
					else
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
					if (li_random > 2)
					{
						message = "rides";
						message2 = "'s rocket of Love";
					}
					else if (li_random == 2)
					{
						message = "was on the business end of";
						message2 = "'s rocket";
					}
					else
					{
						message = "ate";
						message2 = "'s rocket";
					}
					break;
				case MOD_R_SPLASH:
					if (li_random > 2)
					{
						message = "spotted";
						message2 = "'s rocket a little too late";
					}
					else
					{
						message = "almost dodged";
						message2 = "'s rocket";
					}
					break;
				case MOD_HYPERBLASTER:
					message = "was melted by";
					message2 = "'s hyperblaster";
					break;
				case MOD_RAILGUN:
					message = "fell head over bowels for";
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
					if (li_random > 2)
						message = "became one with";
					else
					{
						message = "tried to invade";
						message2 = "'s personal space";
					}
					break;
				case MOD_LASERMOUNT:
					message = "was ripped by";
					message2 = "'s exploding laser";
					break;
				case MOD_HOOK:
					message = "was hooked by";
					break;
				case MOD_KAMISUICIDE:
					message = "was destroyed by";
					break;
				case MOD_APOCSUICIDE:
					if (li_random > 2)
						message = "went to Hell with";
					else
						message = "was disintegrated by";
					break;
				case MOD_DECOYPROMO:
//					if ((self->client->pers.classlevel > 1) && (attacker->client->pers.classlevel < 2))
//						message = "lost a stripe to";
//					else
					message = "was fooled by";
					message2 = "'s decoy promo key";
					break;
				case MOD_TAUNT:
				case MOD_REJOIN:
				case MOD_POISON:
				case MOD_CLUSTERGREN:
				case MOD_BGLASER:
					message = "had too much fun";
					break;
			}
			if (message)
			{
				if (self->client->pers.player_class == 1)
					gi.bprintf (PRINT_MEDIUM,"Team Captain %s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				else
					gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);

				if (deathmatch->value)
				{
					if (teamplay)
					{
						if (OnSameTeam(attacker, self))
						{
							if ((mod != MOD_TELEFRAG) && (!startgame))
							{
								attacker->client->resp.score--;
								attacker->client->pers.kills--;
								teams.team_frags[attacker->client->pers.team - 1]--;
							}
						}
						else
						{
//							if (mod == MOD_DECOYPROMO)
//							{
//								if ((self->client->pers.classlevel > 1) && (attacker->client->pers.classlevel < 2))
//								{
//									self->client->pers.classlevel--;
//									attacker->client->pers.classlevel++;
//								}
//							}

							attacker->client->pers.kills++;

							teams.team_frags[attacker->client->pers.team - 1]++;

							if (lostflag)
								attacker->client->resp.score++;
							else
							{
								wasnear = false;

								VectorCopy(self->s.origin, origin);
								while ((nearflag = findradius(nearflag, origin, 500)) != NULL)
								{
									if (!nearflag->inuse)
										continue;

									if (attacker->client->pers.team == 1)
									{
										if (Q_stricmp (nearflag->classname, "key_blueteam") == 0)
										{
											attacker->client->resp.score++;
											wasnear = true;
											break;
										}
									}
									else if (attacker->client->pers.team == 2)
									{
										if (Q_stricmp (nearflag->classname, "key_redteam") == 0)
										{
											attacker->client->resp.score++;
											wasnear = true;
											break;
										}
									}
								}

								if (!wasnear)
								{
									VectorCopy(attacker->s.origin, origin);
									while ((nearflag = findradius(nearflag, origin, 500)) != NULL)
									{
										if (!nearflag->inuse)
											continue;

										if (attacker->client->pers.team == 1)
										{
											if (Q_stricmp (nearflag->classname, "key_blueteam") == 0)
											{
												attacker->client->resp.score++;
												wasnear = true;
												break;
											}
										}
										else if (attacker->client->pers.team == 2)
										{
											if (Q_stricmp (nearflag->classname, "key_redteam") == 0)
											{
												attacker->client->resp.score++;
												wasnear = true;
												break;
											}
										}
									}

									if (!wasnear)
									{
										VectorCopy(attacker->s.origin, origin);
										while ((nearflag = findradius(nearflag, origin, 1000)) != NULL)
										{
											if (!nearflag->inuse)
												continue;

											if (!nearflag->client)
												continue;

											if (nearflag->client->pers.team != attacker->client->pers.team)
												continue;

											if (nearflag->client->keyindex)
											{
												attacker->client->resp.score++;
												wasnear = true;
												break;
											}
										}
									}
								}
							}

							if (self->client->pers.player_class == 1)
							{
								if (attacker->client->pers.team != self->client->pers.team)
								{
									attacker->client->resp.score++;
									if (teamplay_HasLeader(attacker->client->pers.team))
									{
										teams.team_score[attacker->client->pers.team-1] += 1;
										teams.team_promos[attacker->client->pers.team-1]++;
									}
								}
							}
							else
							{
								if (teamplay_HasLeader(attacker->client->pers.team))
								{
//									if ((attacker->client->pers.player_class != 1) && (self->client->pers.classlevel > 1) && (gsmod_CheckWeapon(mod)))
									if ((attacker->client->pers.player_class != 1) && (self->client->pers.classlevel > 1))
									{
										if ((self->client->pers.classlevel > attacker->client->pers.classlevel + 1) && (attacker->client->quad_framenum <= 0) && (attacker->client->invincible_framenum <= 0))
										{
											if ((attacker->client->invisible) || (attacker->client->quadcloak))
											{
												// Do nothing
											}
											else
											{
												self->client->pers.classlevel -= 1;
												gi.bprintf (PRINT_MEDIUM,"%s took a stripe from %s!\n", attacker->client->pers.netname, self->client->pers.netname);
												attacker->client->pers.classlevel = 1;
												attacker->client->resp.score++;
											}
										}
									}
								}
							}
						}
					}
					else
					{
						if (ff)
						{
							if (!startgame)
							{
								attacker->client->pers.kills--;
								attacker->client->resp.score--;
							}
						}
						else
							attacker->client->pers.kills++;
					}
				}

				if (attacker->client->resp.score < 0)
				{
					if (attacker->client->pers.classlevel > 0)
						gi.bprintf (PRINT_MEDIUM,"%s was demoted to Level 0 for a negative frag count!\n", attacker->client->pers.netname);
					gsutil_centerprint(attacker, "%s", "You will need to get back into\npositive frags before you can\nbe promoted!\n");
					attacker->client->pers.classlevel = 0;
				}

				return;
			}
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s = Dead.\n", self->client->pers.netname);
	if (deathmatch->value)
		self->client->pers.kills--;
}


// Add function in g_cmds.c to do the following:
//
//
//void ClientCommand (edict_t *ent)
//{
//	...
//	else if (Q_stricmp (cmd, "fov") == 0)
//	{
//		ent->client->ps.fov = atoi(gi.argv(1));
//		if (ent->client->ps.fov < 1)
//			ent->client->ps.fov = 90;
//		else if (ent->client->ps.fov > 160)
//			ent->client->ps.fov = 160;
//	}
//	else
//		gsmod_ClientCommand(ent, cmd);		<---
//	...
//}
//
void sniper_flare (edict_t *self);

void gsmod_ClientCommand (edict_t *ent, char *cmd, char *parms)
{
	qboolean nonevalid=false;
	
	if (!ent->client)
		return;

	if ((ent->client->pers.startlevel) && (!ent->client->observer) && (ent->health > 0))
	{
		if (Q_stricmp (cmd, "tracker") == 0)
			gsmod_Tracker (ent);
		else if (Q_stricmp (cmd, "helpon") == 0)
		{
			ent->client->pers.showhelp = true;
			gsutil_centerprint (ent, "%s", "Help messages have been turned on.\n");		
		}
		else if (Q_stricmp (cmd, "helpoff") == 0)
		{
			ent->client->pers.showhelp = false;
			gsutil_centerprint (ent, "%s", "Help messages have been turned off.\n");		
		}
		else if (Q_stricmp (cmd, "scouton") == 0)
		{
			ent->client->pers.scoutmsg = true;
			gsutil_centerprint (ent, "%s", "Scout messages have been turned on.\n");		
		}
		else if (Q_stricmp (cmd, "scoutoff") == 0)
		{
			ent->client->pers.scoutmsg = false;
			gsutil_centerprint (ent, "%s", "Scout messages have been turned off.\n");		
		}
		else if (Q_stricmp (cmd, "alarmon") == 0)
		{
			ent->client->pers.alarmmsg = true;
			gsutil_centerprint (ent, "%s", "Alarm messages have been turned on.\n");		
		}
		else if (Q_stricmp (cmd, "alarmoff") == 0)
		{
			ent->client->pers.alarmmsg = false;
			gsutil_centerprint (ent, "%s", "Alarm messages have been turned off.\n");		
		}
		else if ((Q_stricmp (cmd, "help") == 0) || (Q_stricmp (cmd, "modhelp") == 0))
			gsmod_Help(ent);
		else if (Q_stricmp (cmd, "killmenu") == 0)
			gsmod_Killmenu(ent);
		else if (Q_stricmp (cmd, "menustyle") == 0)
		{
			if (ent->client->pers.fancymenu)
			{
				ent->client->pers.fancymenu = false;
				gsutil_centerprint (ent, "%s", "Fancy menus have been turned off.\n");		
			}
			else
			{
				if (ent->client->ping > 250)
				{
					gsutil_centerprint (ent, "%s", "Your ping is too high for fancy menus.\n");		
				}
				else
				{
					ent->client->pers.fancymenu = true;
					gsutil_centerprint (ent, "%s", "Fancy menus have been turned on.\n");		
				}
			}
		}
		else if (Q_stricmp (cmd, "vote") == 0)
			gsmod_Vote(ent, false, parms);
		else if (Q_stricmp (cmd, "scout") == 0)
			gsmod_Scout(ent, false);
		else if (Q_stricmp (cmd, "scoutall") == 0)
			gsmod_Scout(ent, true);
		else if (Q_stricmp (cmd, "rage") == 0)
			gsmod_Rage(ent);
		else if (Q_stricmp (cmd, "suicide") == 0)
			gsmod_Suicide(ent);
		else if (Q_stricmp (cmd, "longrange") == 0)
		{
			if (ent->client->pers.classlevel > 1)
			{
				if (ent->client->longrange)
				{
					ent->client->longrange = false;
					gsutil_centerprint(ent, "%s", "Long range grenade launcher OFF\n");
				}
				else
				{
					ent->client->longrange = true;
					gsutil_centerprint(ent, "%s", "Long range grenade launcher ON\n");
				}
			}
		}

		else if (Q_stricmp (cmd, "decoy") == 0)
			gsmod_Decoy(ent);
		else if (Q_stricmp (cmd, "flare") == 0)
			sniper_flare (ent);

		else if (Q_stricmp (cmd, "flashlight") == 0)
			FL_make (ent);
		
		else if (Q_stricmp (cmd, "welcome") == 0)
			gsmod_DisplayMsg(ent, "welcome.txt");
		else if (Q_stricmp (cmd, "sight") == 0)
			gsmod_LaserSight (ent);
		else if (Q_stricmp (cmd, "rebind") == 0)
			gsmod_Rebind (ent);
		else if (Q_stricmp (cmd, "coordinates") == 0)
			gsmod_Coordinates (ent);
		else if (Q_stricmp (cmd, "autoid") == 0)
		{
			if (ent->client->pers.autoid)
			{
				ent->client->pers.autoid = false;
				gsutil_centerprint(ent, "%s", "Autoid turned off\n");
			}
			else
			{
				ent->client->pers.autoid = true;
				gsutil_centerprint(ent, "%s", "Autoid turned on\n");
			}
		}
		else if (Q_stricmp (cmd, "alarm") == 0)
			gsmod_PlaceAlarm(ent);
		else if (Q_stricmp (cmd, "laser") == 0)
			gsmod_PlaceLaser(ent);
		else if (Q_stricmp (cmd, "laseron") == 0)
			gsmod_LaserToggle(ent, true);
		else if (Q_stricmp (cmd, "laseroff") == 0)
			gsmod_LaserToggle(ent, false);
		else if (Q_stricmp (cmd, "laserkill") == 0)
			gsmod_LaserKill(ent);
		else if (Q_stricmp (cmd, "alarmkill") == 0)
			gsmod_AlarmKill(ent);
		else if (Q_stricmp (cmd, "sendalarm") == 0)
			gsmod_SendAlarm(ent);
		else if (Q_stricmp (cmd, "autorewire") == 0)
			gsmod_AutoRewire(ent);
		else if (Q_stricmp (cmd, "steal") == 0)
			gsmod_Steal(ent);
		else if (Q_stricmp (cmd, "alarm1") == 0)
			gsmod_NameAlarm(ent, 1, false);
		else if (Q_stricmp (cmd, "alarm2") == 0)
			gsmod_NameAlarm(ent, 2, false);
		else if (Q_stricmp (cmd, "alarm3") == 0)
			gsmod_NameAlarm(ent, 3, false);
		else if (Q_stricmp (cmd, "flagalarm1") == 0)
			gsmod_NameAlarm(ent, 1, true);
		else if (Q_stricmp (cmd, "flagalarm2") == 0)
			gsmod_NameAlarm(ent, 2, true);
		else if (Q_stricmp (cmd, "flagalarm3") == 0)
			gsmod_NameAlarm(ent, 3, true);
		else if (Q_stricmp (cmd, "skill1") == 0)
			gsmod_Skill(ent, 1);
		else if (Q_stricmp (cmd, "skill2") == 0)
			gsmod_Skill(ent, 2);
		else if (Q_stricmp (cmd, "skill3") == 0)
			gsmod_Skill(ent, 3);

		else if (Q_stricmp (cmd, "play_captain") == 0)
			gsmod_Play(ent, 1);
		else if (Q_stricmp (cmd, "play_bodyguard") == 0)
			gsmod_Play(ent, 2);
		else if (Q_stricmp (cmd, "play_sniper") == 0)
			gsmod_Play(ent, 3);
		else if (Q_stricmp (cmd, "play_soldier") == 0)
			gsmod_Play(ent, 4);
		else if (Q_stricmp (cmd, "play_berzerker") == 0)
			gsmod_Play(ent, 5);
		else if (Q_stricmp (cmd, "play_infiltrator") == 0)
			gsmod_Play(ent, 6);
		else if (Q_stricmp (cmd, "play_kamikazee") == 0)
			gsmod_Play(ent, 7);
		else if (Q_stricmp (cmd, "play_nurse") == 0)
			gsmod_Play(ent, 8);
		else if (Q_stricmp (cmd, "play_team") == 0)
			gsmod_Play(ent, 9);
		else if (Q_stricmp (cmd, "play_capturer") == 0)
			gsmod_Play(ent, 10);
		else if (Q_stricmp (cmd, "play_killer") == 0)
			gsmod_Play(ent, 11);
		else if (Q_stricmp (cmd, "play_autoid") == 0)
			gsmod_Play(ent, 12);
		else if (Q_stricmp (cmd, "customwav") == 0)
		{
			if (ent->client->pers.customwav)
			{
				ent->client->pers.customwav = false;
				gsutil_centerprint(ent, "%s", "Custom sounds disabled");
			}
			else
			{
				ent->client->pers.customwav = true;
				gsutil_centerprint(ent, "%s", "Custom sounds enabled");
			}
		}

		else if (Q_stricmp (cmd, "modstatus") == 0)
			gsmod_ModStatus(ent);
		else if (Q_stricmp (cmd, "modversion") == 0)
			gsutil_centerprint(ent, "%s", "Battle of the Sexes Quake2 Mod\nv3.0 Beta #27b\n\nspinoza.warzone.com");
		else if (Q_stricmp (cmd, "zzzturretzzz") == 0)
			ent->hasturret = false;
		else if ((Q_stricmp (cmd, "exitturret") == 0) || (Q_stricmp (cmd, "leaveturret") == 0))
			ent->exitturret = true;
		else if (Q_stricmp (cmd, "detpipe") == 0)
		{
			if (teamplay == 1)
			{
				if (ent->client->pers.player_class != 7)
				{
					gsutil_centerprint(ent, "%s", "Only Kamikazee can use detpipes");
					return;
				}
			}

			gsmod_BlowGrenades(ent);
		}
		else if (Q_stricmp (cmd, "gender") == 0)
		{
			if (IsFemale(ent))
				gsmod_DisplayMsg(ent, "female.txt");
			else
				gsmod_DisplayMsg(ent, "male.txt");
		}
		else
			nonevalid = true;
	}
	else
	{
		if (Q_stricmp (cmd, "killmenu") == 0)
			gsmod_Killmenu(ent);
		else if ((Q_stricmp (cmd, "help") == 0) || (Q_stricmp (cmd, "modhelp") == 0))
			gsmod_Help(ent);
		else if (Q_stricmp (cmd, "modversion") == 0)
			gsutil_centerprint(ent, "%s", "Battle of the Sexes Quake2 Mod\nv3.0 Beta #27b\n\nspinoza.warzone.com");
		else if (Q_stricmp (cmd, "play_killer") == 0)
			gsmod_Play(ent, 11);
		else
			nonevalid = true;
	}

	if (nonevalid)
	{
		if (teamplay == 1)
			teamplay_ClientCommand (ent, cmd, parms);
		else
			gi.cprintf (ent, PRINT_HIGH, "Bad command: %s\n", cmd);
	}
}
