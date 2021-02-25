#include <ctype.h>
#include "g_local.h"
#include "ctc.h"

#ifdef STD_LOGGING	
#include "stdlog.h"	//	StdLog - Mark Davies
#include "gslog.h"	//	StdLog - Mark Davies
#endif

#define TEAMMENU_START	 3	// Offset to calculate selected team in menu
#define PLAYERMENU_START 10	// Offset to calculate selected player model

int		teams = 0;			// How many teams can be in tctc

TeamDetails teamDetails[] =
		{	{"Red",		"kw_red",		"t_red",		"t_red_c",		"tag_red",		"tag_red_c",		0,		0,	""},
			{"Blue",	"kw_blue",		"t_blue",		"t_blue_c",		"tag_blue",		"tag_blue_c",		0,		0,	""},
			{"Yellow",	"kw_yellow",	"t_yellow",		"t_yellow_c",	"tag_yellow",	"tag_yellow_c",		0,		0,	""},
			{"Green",	"kw_green",		"t_green",		"t_green_c",	"tag_green",	"tag_green_c",		0,		0,	""}
		};

//
// Chicken Teamplay Options
//
int		teamWithChicken	= -1;
char	teamPlayerMenuLine[MAX_TEAMS][MENU_ITEM_LINE_LEN];	// Menu line string for each player model

//
// Chicken Internal Functions
//
static int	Chicken_SelectTeam		(void *data, int itemId);
static int	Chicken_SetPlayerSkin	(edict_t *ent);
static int	Chicken_SetPlayerModel	(void *data, int itemId);
static void Chicken_ThrowEggShell	(edict_t *self, char *modelname, float speed, vec3_t origin, int count);
static void Chicken_EggSplatTouch	(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
static void Chicken_EggSplat		(edict_t *ent, vec3_t origin, vec3_t dir);
static void Chicken_EggTouch		(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
static void Chicken_FireEgg			(edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer);
static void Chicken_EggGunFire		(edict_t *ent);


//
// ----------------------------------------------------------------------------------------
// Gives player on the selected team the egggun and readies the correct weapon
//
qboolean Chicken_PlayerReadyEggGun(edict_t *ent, int team)
{
	qboolean changed = false;

	if (deathmatch->value && chickenGame && teams != 0 && ent->client->team == team && ent->s.modelindex2 != 0)
	{
		// Give em the egggun
		ent->client->pers.inventory[eggGunItemIndex] = 1;

		// Ready correct weapon chicken or egggun
		Chicken_Ready(ent);

		if (strlen(SOUND_EGGGUN_READY))
			gi.sound(ent, CHAN_AUTO, gi.soundindex(SOUND_EGGGUN_READY),  1, ATTN_NORM, 0);	

		ent->client->pers.selected_item = ITEM_INDEX(ent->client->pers.weapon);
		ent->client->ammo_index	= 0;
		changed = true;
	}

	return (changed);
}
//
// ----------------------------------------------------------------------------------------
// Gives all players on the selected team the egggun and readies the correct weapon
//
void Chicken_TeamReadyEggGun(int team)
{
	edict_t		*e  = NULL;
	int i;

	// If it is not a team game No EGGUN
	if (teams != 0)
	{
		for (i=0 ; i<game.maxclients ; i++)
		{
			e = g_edicts + 1 + i;

			if (!e->inuse) continue;

			// For every Client egggun egggun if required
			Chicken_PlayerReadyEggGun(e, team);
		}
	}
}
//
// ----------------------------------------------------------------------------------------
// Check if players model is allowed in game
//
qboolean Chicken_CheckPlayerModel(edict_t *ent)
{
	int			x = 0;
	qboolean	goodModel = false;
	char		*model = NULL;
	
	model = Chicken_GetModelName(ent);

	do 
	{
		if (!strcmp(model, playerModels[x].playerModel))
			goodModel = true;
		x++;
	}
	while (!goodModel && x < MAX_PLAYER_MODELS);

	ent->client->modelOk = goodModel;
	return (goodModel);
}
//
// ----------------------------------------------------------------------------------------
// Select team player wish's to join
//
static int Chicken_SelectTeam(void *data, int itemId)
{
	edict_t *ent   = (edict_t *)data;
	int		offset = itemId-TEAMMENU_START;
	int     team;

	// Find out which team they selected from the menu
	for (team = 0; team < MAX_TEAMS; team++)
	{
		if (teams & (1 << team))
		{
			if (offset == 0) break;
			offset--;
		}
	}

	// Increment number of players on team
	teamDetails[team].players++;

	// Update menu so that people can see correct count of players
	sprintf(teamDetails[team].menu, "Team %-6.6s %d Players", teamDetails[team].colour, teamDetails[team].players);

	// You are now on this team
	ent->client->team = team;
	gi.bprintf(PRINT_HIGH, "%s has joined %s team\n", ent->client->pers.netname, teamDetails[team].colour);	

	sl_LogPlayerTeamChange( &gi, ent->client->pers.netname, teamDetails[team].colour, level.time);
	
	// If the models OK then they can join now
	if (ent->client->modelOk)
	{
		Chicken_SetPlayerSkin(ent);
		NoAmmoWeaponChange(ent);
		ChangeWeapon (ent);

		Chicken_ObserverEnd(ent);
	}

	return 0;
}
//
// ----------------------------------------------------------------------------------------
// Set players skin to the team selected
//
static int Chicken_SetPlayerSkin(edict_t *ent)
{
	char	newUserInfo [MAX_INFO_STRING];
	char	userinfo    [MAX_INFO_STRING];
	char	newString   [MAX_INFO_STRING];
	char	*tok;

	// Make sure we are clean about this
	memset(newUserInfo, 0, sizeof(newUserInfo));
	memset(userinfo,    0, sizeof(userinfo));

	memcpy(userinfo, ent->client->pers.userinfo, strlen(ent->client->pers.userinfo));

	// Go though the userinfo and change skin to new skin
	tok = strtok(userinfo, "/\\\n\0");
	while(tok != NULL)
	{
		// If this is not the skin just copy it as is
		if (strcmp(tok, "skin"))
			sprintf(newString, "\\%s", tok);
		else
		{
			// Set the new skin to the team colour
			sprintf(newString, "\\skin\\%s/%s", strtok(NULL, "/"), teamDetails[ent->client->team].skin);
			strtok(NULL, "\\");
		}

		strcat(newUserInfo, newString);
		tok = strtok(NULL, "/\\\n\0");
	}

	// Lets go to it and update the players details
	ClientUserinfoChanged (ent, newUserInfo);

	return 0;
}
//
// ----------------------------------------------------------------------------------------
// Select new player model that is supported
//
static int Chicken_SetPlayerModel(void *data, int itemId)
{
	edict_t *ent		= (edict_t *)data;
	int		playerModel = itemId-PLAYERMENU_START;
	char	newUserInfo [MAX_INFO_STRING];
	char	userinfo    [MAX_INFO_STRING];
	char	newString   [MAX_INFO_STRING];
	char	*tok;

	// Make sure we are clean about this
	memset(newUserInfo, 0, sizeof(newUserInfo));
	memset(userinfo,    0, sizeof(userinfo));

	memcpy(userinfo, ent->client->pers.userinfo, strlen(ent->client->pers.userinfo));

	tok = strtok(userinfo, "/\\\n\0");
	while(tok != NULL)
	{
		if (strcmp(tok, "skin"))
		{
			if (strcmp(tok, "gender"))
				sprintf(newString, "\\%s", tok);
			else
			{
				sprintf(newString, "\\gender\\%s", playerModels[playerModel].playerGender);
				strtok(NULL, "\\");
			}
		}
		else
		{
			sprintf(newString, "\\skin\\%s/%s", playerModels[playerModel].playerModel, teamDetails[ent->client->team].skin);
			strtok(NULL, "/");
			strtok(NULL, "\\");
		}

		strcat(newUserInfo, newString);
		tok = strtok(NULL, "/\\\n\0");
	}

	ent->s.modelindex2 = gi.modelindex (playerModels[playerModel].playerModelPath);
	ClientUserinfoChanged (ent, newUserInfo);

	Chicken_ObserverEnd(ent);

	return 0;
}
//
// ----------------------------------------------------------------------------------------
//	Create menu for TCTC. Should only be called once
//
void Chicken_PlayerSelectMenuCreate()
{
	static qboolean menuSetup = false;

	if (!menuSetup)
	{
		int x;

		Chicken_MenuInsert(3);
		
		Chicken_MenuItemInsert(3,3, "The model you are using",		YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(3,4, "is not supported",				YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(3,5, NULL,							YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(3,6, "Please select new model",		YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(3,7, NULL,							YELLOW, CENTER, OPT_FILLER );

		for (x = 0; x < MAX_PLAYER_MODELS; x++)
		{
			sprintf(teamPlayerMenuLine[x], "%s", playerModels[x].menuText);
			Chicken_MenuItemInsert(3, x+PLAYERMENU_START, teamPlayerMenuLine[x], 	WHITE, LEFT, OPT_SELECT, Chicken_SetPlayerModel);
		}

		Chicken_MenuItemInsert(3,16, " use [ and ] to move cursor",	YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(3,17, "press enter to select",		YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(3,18, "                ",		    YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(3,20, CHICKEN_STRING_MENU_VERSION,	YELLOW, RIGHT,  OPT_FILLER );

		menuSetup = true;
	}
}
//
// ----------------------------------------------------------------------------------------
//	Create menu for CTC. Should only be called once
//
void Chicken_TeamMenuCreate()
{
	int x, y = 0;
	static qboolean menuSetup = false;

	if (!menuSetup)
	{

		Chicken_MenuInsert(2);
		
		for (x = 0; x < MAX_TEAMS; x++)
		{
			int currentTeam = (1 << x);

			if (teams & currentTeam)
			{
				sprintf(teamDetails[x].menu, "Team %-6.6s %d Players", teamDetails[x].colour, teamDetails[x].players);
				Chicken_MenuItemInsert(2, y+TEAMMENU_START, teamDetails[x].menu, 	WHITE, CENTER, OPT_SELECT, Chicken_SelectTeam);
				y++;
			}
		}
		Chicken_MenuItemInsert(2,16, " use [ and ] to move cursor",	YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(2,17, "press enter to select",		YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(2,18, "                ",		    YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(2,20, CHICKEN_STRING_MENU_VERSION,	YELLOW, RIGHT,  OPT_FILLER );

		menuSetup = true;
	}
}
//
// ----------------------------------------------------------------------------------------
// You guest it throw an egg shell
//
static void Chicken_ThrowEggShell (edict_t *self, char *modelname, float speed, vec3_t origin, int count)
{
	edict_t	*chunk;

	while (count)
	{
		if ((chunk = G_Spawn()) != NULL)
		{
			VectorCopy  (origin, chunk->s.origin);
			gi.setmodel (chunk, modelname);

			VectorScale (self->velocity, speed, chunk->velocity);

			chunk->velocity[0] += crandom()*200;
			chunk->velocity[1] += crandom()*200;

			chunk->movetype		= MOVETYPE_BOUNCE;
			chunk->solid		= SOLID_NOT;
			chunk->avelocity[0] = random()*600;
			chunk->avelocity[1] = random()*600;
			chunk->avelocity[2] = random()*600;
			chunk->think		= G_FreeEdict;
			chunk->nextthink	= level.time + 3 + random()*4;
			chunk->s.frame		= 0;
			chunk->flags		= 0;
			chunk->classname	= "debris";
			chunk->takedamage	= DAMAGE_YES;
			chunk->die			= debris_die;

			gi.linkentity (chunk);
		}
		else
			break;

		count--;
	}
}
//
// ----------------------------------------------------------------------------------------
//
static void Chicken_EggSplatTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if ((surf && (surf->flags & SURF_SKY)) || !plane)
	{
		G_FreeEdict (self);
		return;
	}

	vectoangles (plane->normal, self->s.angles);
	self->s.angles[0] += 90;
}
//
// ----------------------------------------------------------------------------------------
//
static void Chicken_EggSplat (edict_t *ent, vec3_t origin, vec3_t dir)
{
	edict_t	*eggsplat;

	eggsplat = G_Spawn();
	VectorCopy (origin, eggsplat->s.origin);

	vectoangles (dir, eggsplat->s.angles);

	eggsplat->s.angles[0] += 90;

	VectorClear (eggsplat->mins);
	VectorClear (eggsplat->maxs);
	
	eggsplat->s.modelindex = gi.modelindex (MODEL_EGG_SPLAT);

	eggsplat->movetype	= MOVETYPE_TOSS;
	eggsplat->solid		= SOLID_BBOX;
	eggsplat->gravity	= 0.2;
	eggsplat->s.effects = 0;
	eggsplat->owner		= world;
	eggsplat->touch		= Chicken_EggSplatTouch;
	eggsplat->nextthink = level.time + 3 + random()*4;
	eggsplat->think		= G_FreeEdict;
	eggsplat->classname = "eggsplat";

	gi.linkentity (eggsplat);

	if (strlen(SOUND_EGG_SPLAT))
		gi.sound(ent, CHAN_AUTO, gi.soundindex(SOUND_EGG_SPLAT),  1, ATTN_NORM, 0);	
}
//
// ----------------------------------------------------------------------------------------
//
static void Chicken_EggTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	// If its not on this earth get ride of it
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	// Throw egg shell debris
	Chicken_ThrowEggShell (self, MODEL_EGG_SHELL, 0.4, self->s.origin, 3);

	if (other->takedamage)
	{
		// An entity was hit!! DO SOMETHING ABOUT IT
		if (!other->client)
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 10, 1, DAMAGE_ENERGY, 0); //Do 10 damage
		else
		{
			float multi = kickback/10.0;

			// If shooter has quad do the multiply stuff
			if (self->owner && self->enemy->client->quad_framenum > level.framenum)
				multi *= 4;

			// If shooter was from the same team dont kick so much
			if (self->owner && self->owner->client->team == other->client->team)
				multi *= 0.25;

			if (other->groundentity)		// Throw player more if on ground
			{
				other->velocity[0] += (self->velocity[0] * multi);
				other->velocity[1] += (self->velocity[1] * multi);
				other->velocity[2] += 300;
			}
			else if (other->waterlevel)
				VectorMA (other->velocity, multi, self->velocity, other->velocity);
			else
			{
				other->velocity[0] += (self->velocity[0] * multi);
				other->velocity[1] += (self->velocity[1] * multi);
			}
		}
	}
	else
	{
		// If its touching a plane set it to the planes normal
		if (!plane)
			Chicken_EggSplat (self, self->s.origin, vec3_origin);
		else
			Chicken_EggSplat (self, self->s.origin, plane->normal);
	}

	G_FreeEdict (self);
}
//
// ----------------------------------------------------------------------------------------
//
static void Chicken_EggGunFire (edict_t *ent)
{
	edict_t	*egg;
	vec3_t	dir;
	vec3_t	forward, right, up;
	vec3_t	offset;
	vec3_t	start;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if ((egg = G_Spawn()) != NULL)
	{
		vectoangles (forward, dir);
		AngleVectors (dir, forward, right, up);

		VectorCopy (start, egg->s.origin);
		VectorScale (forward, EGG_SPEED, egg->velocity);

		VectorMA  (egg->velocity, 150 + crandom() * 10.0, up, egg->velocity);
		VectorMA  (egg->velocity, crandom() * 10.0, right, egg->velocity);
		VectorSet (egg->avelocity, 300, 300, 300);

		egg->s.modelindex = gi.modelindex (MODEL_EGG);

		VectorSet (egg->mins, -4, -4, -4);
		VectorSet (egg->maxs,  4,  4,  4);

		egg->movetype	= MOVETYPE_TOSS;
		egg->clipmask	= MASK_SHOT;
		egg->solid		= SOLID_BBOX;
		egg->s.effects	= 0;
		egg->owner		= ent;
		egg->touch		= Chicken_EggTouch;
		egg->nextthink	= level.time + 2.5;
		egg->think		= G_FreeEdict;
		egg->dmg		= 0;
		egg->dmg_radius = 0;
		egg->classname	= "egg";
		egg->gravity    = 0.75;
		egg->enemy      = ent;

		gi.linkentity (egg);

		ent->client->ps.gunframe++;

		if (strlen(SOUND_EGGGUN_FIRE))
			gi.sound(ent, CHAN_AUTO, gi.soundindex(SOUND_EGGGUN_FIRE),  1, ATTN_NORM, 0);
	}
}
//
// ----------------------------------------------------------------------------------------
//
void Weapon_Egggun (edict_t *ent)
{
	static int	pause_frames[]	= {5, 25, 34, 44, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 5, 13, 50, 55, pause_frames, fire_frames, Chicken_EggGunFire);
}
