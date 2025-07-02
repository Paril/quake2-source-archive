#include <sys/types.h> 
#include <sys/stat.h>
#include <ctype.h>
#include "g_local.h"
#include "frames.h"
#include "ctclocal.h"

/* Added by Chris Richards (cjr@tivoli.com) */
#ifdef _WIN32
#include <io.h>
#endif

#ifdef STD_LOGGING	
#include "stdlog.h"	//	StdLog - Mark Davies
#include "gslog.h"	//	StdLog - Mark Davies
#endif

ModelDetails playerModels[] =
		{	{"male",		"Male",		"male",		"players/male/tris.md2",	"players/male/w_chicken.md2",	 "players/male/w_egggun.md2"   },
			{"female",		"Female",	"female",	"players/female/tris.md2",	"players/female/w_chicken.md2",  "players/female/w_egggun.md2" },
			{"cyborg",		"Cyborg",	"male",		"players/cyborg/tris.md2",	"players/cyborg/w_chicken.md2",  "players/cyborg/w_egggun.md2" }
//			{"crakhor",		"Crakhor",	"female",	"players/crakhor/tris.md2",	"models/weapons/crakhor/tris.md2", "models/weapons/crakhor/w_egggun.md2"}
			};


//
//	Global variables
//
#ifdef STD_LOGGING	
qboolean	loggingStarted		= false;		// Has Std logging commenced
#endif

char		motd1[31];
char		motd2[31];
char		motd3[31];
char		motd4[31];

int			respawnTime			= 10;			// Game Time when chicken spawned
int			spawnDelay			= 10;			// Chicken cannot be respawned for spawnDelay seconds
int			chickenGame			= false;		// Has the Chicken game started
int			allowBigHealth		= false; 		// Can players pickup large health items
int			allowSmallHealth	= false; 		// Can players pickup small health items
int			allowArmour			= false;		// Can players pickup armour
int			allowInvulnerable	= false;		// Can players be Invulnerable
int			canDrop				= false;		// Is the options menu up
int			allowGlow			= 1;			// Chicken Glow colour
int			scoreOnDeath		= true;			// Do you score for killing person with chicken
int			scorePeriod			= 5;			// How long to hold chicken before scoring
int			dropDelay           = 10;			// How long you have to hold onto chicken
int			feathers            = 5;			// Chance of throwing random feather
int			menuAllowed         = true;			// Is menu allowed up
int			observerAllowed     = true;			// Are players allowed to go into observer mode
int			cameraAllowed       = true;			// Are players allowed to go into camera mode
int			stdLogging			= false;
int			levelCycling		= false;		// Allow cycling maps
int			autoRespawn			= false;		// Allow Auto Respawn of chicken
int			randomSpawn			= false;		// Does chicken respawn at random place
int			autorespawntime		= 60;			// How often to auto respawn
int			cantTouchDelay		= 0;			// How long after throwing chicken can player pick it up
int			maxHoldScore		= 2;			// Maximum score player can get while holding chicken
int			clientCount			= 0;			// How many clients are in the game (Refreshed peiodicly)

int			chickenItemIndex	= 0;			// Index of chicken item in item list
int			eggGunItemIndex		= 0;			// Index of egg gun item in item list
int			kickback			= 5;			// Kickback when players hit by egg
int			MOTDDuration		= 4;			// Duration to keep MOTD up for

struct gitem_s *chickenItem	    = NULL;			// Pointer to chicken item
struct gitem_s *eggGunItem	    = NULL;			// Pointer to egg gun item

static qboolean	wasInWater		= false;		// Was chicken in the water
static char gameStatusString[]  = "start game             "; // Must be big enough to store any menu text
extern char *dm_statusbar;						// CTC Statusbar

//
// Chicken Internal Functions
//
static void Chicken_Fire				(edict_t *ent);
static void Chicken_ClockStart			(edict_t *ent);
static void Chicken_ClockThink			(edict_t *self);
static void Chicken_Float				(edict_t *self);
static void Chicken_AdvanceFrame		(edict_t *self);
static void Chicken_RandomSound			(edict_t *self);
static void Chicken_ThrowFeather		(edict_t *self, int count);
static int  Chicken_GameStatus			(void *data, int itemId);
static void Chicken_MenuCreate			();
static edict_t *Chicken_FindSpawnSpot	(edict_t *ent);
static void	Chicken_Think				(edict_t *self);
static void	Chicken_EndIt				(edict_t *ent);
static void Chicken_Setup				(edict_t *chicken, edict_t *owner);
static void Chicken_Drop_Temp			(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
static void Chicken_FlyThink			(edict_t *self);
static void Chicken_SelectPlayerMenu	(edict_t *ent, int menuId);
static void	Chicken_ReadIni				();
static void	Chicken_EnsureExists		();
static void	Chicken_Observer			(edict_t *ent);
static void	Chicken_RemoveFromInventory	();
static void Chicken_ReadMapList			();
static void Chicken_MOTDLoad			();

//
// CTC Ini file options
//
typedef struct
{
	char	*ident;
	int		*variable;
} INI_OPTION;

INI_OPTION	option[] = 

//					Noraml CTC Options

		{	{"bighealth",		&allowBigHealth		},
			{"smallhealth",		&allowSmallHealth	},
			{"armor",			&allowArmour		},
			{"invuln",			&allowInvulnerable	},
			{"glow",			&allowGlow			},
			{"droppable",		&canDrop			},
			{"fragscore",		&scoreOnDeath		},
			{"autostart",		&chickenGame		},
			{"scoretime",		&scorePeriod		},
			{"droptime",		&dropDelay			},
			{"feathers",		&feathers			},
			{"respawntime",		&spawnDelay			},
			{"stdlog",			&stdLogging			},	// Dont change ident name
			{"randomspawn",		&randomSpawn		},
			{"allowmenu",		&menuAllowed		},	
			{"autorespawn",		&autoRespawn		},	
			{"autorespawntime", &autorespawntime	},	
			{"allowcamera",		&cameraAllowed		},
			{"allowobserver",	&observerAllowed	},
			{"levelcycling",	&levelCycling		},

//					Teamplay Options

			{"notouchdelay",    &cantTouchDelay		},
			{"maxholdscore",    &maxHoldScore		},
			{"kickback",		&kickback			},
			{"modduration",		&MOTDDuration		},
			{"teams",			&teams				}};

#define MAX_OPTIONS (sizeof(option)/sizeof(option[0]))

//
// CTC Death messages
//
char *killedSelf[] =
		{	{"%s dies\n"									}};
char *killerKilled[] =
		{	{"%s kills %s the chicken rustler\n"			},
			{"%s plucks %s's chicken\n"						},
			{"%s turns %s into nuggets\n"					},
			{"%s has %s with 11 secret herbs and spices\n"	},
			{"%s roasts %s\n"								},
			{"%s plays mash the chicken with %s\n"			}};
char *killer[] =
		{	{"%s frees the chicken's soul\n"				},
			{"%s kills the chicken rustler\n"				},
			{"%s forces release of the chicken\n"			},
			{"%s administers a fatal plucking\n"			},
			{"%s makes a sacrifice to the chicken god\n"	},
			{"%s skews some dinner\n"						},
			{"%s is now fit to enter chicken heaven\n"		},
			{"The chicken god smiles upon %s\n"				}};
char *killed[] =
		{	{"Mc %s Nugggets to go!\n"						},
			{"%s dies for the chicken god\n"				},
			{"%s has been judged by the chicken god\n"		}};

//
// CTC Statusbar
//
char ctc_statusbar[2048];

//
// TCTC Level List
//
LevelName *levelList = NULL;
LevelName *levelTail = NULL;

//
// ----------------------------------------------------------------------------------------
//	Generates Runtime status bar so that all teams are packed to top of screen
//
void Chicken_CreateStatusBar()
{
	char teamBar[256];

	memset(ctc_statusbar, 0, sizeof(ctc_statusbar));

	sprintf(teamBar, "if %d yb -24 xv 0 hnum xv 50 pic 0 endif ", STAT_CHICKEN_OBSERVER);
	strcat(ctc_statusbar, teamBar);

	sprintf(teamBar, "if %d if 2 xv 100 anum xv 150 pic 2 endif endif ", STAT_CHICKEN_OBSERVER);
	strcat(ctc_statusbar, teamBar);

	sprintf(teamBar, "if %d if 4 xv 200 rnum xv 250 pic 4 endif endif ", STAT_CHICKEN_OBSERVER);
	strcat(ctc_statusbar, teamBar);

	sprintf(teamBar, "if %d if 4 xv 200 rnum xv 250 pic 4 endif endif ", STAT_CHICKEN_OBSERVER);
	strcat(ctc_statusbar, teamBar);

	sprintf(teamBar, "if %d if 6 xv 296 pic 6 endif endif ", STAT_CHICKEN_OBSERVER);
	strcat(ctc_statusbar, teamBar);

	sprintf(teamBar, "if %d yb -50 if 7 xv 0 pic 7 xv 26 yb -42 stat_string 8 yb -50 endif endif ", STAT_CHICKEN_OBSERVER);
	strcat(ctc_statusbar, teamBar);

	sprintf(teamBar, "if %d if 9 xv 262 num 2 10 xv 296 pic 9 endif endif ", STAT_CHICKEN_OBSERVER);
	strcat(ctc_statusbar, teamBar);

	sprintf(teamBar, "if %d if 11 xv 148 pic 11 endif endif ", STAT_CHICKEN_OBSERVER);
	strcat(ctc_statusbar, teamBar);

	sprintf(teamBar, "if %d if %d yb -75 xr -24 pic %d xr -46 num 1 %d "
		             "xr -62 num 1 %d xr -78 pic %d xr -110 num 2 %d endif endif ",
					 STAT_CHICKEN_OBSERVER, STAT_DISPLAY_CLOCK, STAT_CHICKEN_CLOCK,
					 STAT_CHICKEN_TIME_2SEC, STAT_CHICKEN_TIME_1SEC, STAT_CHICKEN_COLON, STAT_CHICKEN_TIME_MIN);
	strcat(ctc_statusbar, teamBar);

	if (teams == 0)
	{
		sprintf(teamBar, "xr -50 yt 2 num 3 14 ");
		strcat(ctc_statusbar, teamBar);
	}
	else
	{
		int x	= 0, yt	= 2, ateam = 0;

		for (x = 0; x < MAX_TEAMS; x++)
		{
			int currentTeam = (1 << x);

			if (teams & currentTeam)
			{
				sprintf(teamBar, "if %d if %d xr -60 yt %d num 2 %d xr -24 pic %d endif endif ",
						STAT_SCORES, STAT_CHICKEN_OBSERVER, yt, STAT_DISPLAY_SCORE1 + x, STAT_PIC_TEAM1 + x);
				strcat(ctc_statusbar, teamBar);

				sprintf(teamBar, "if %d xv %d yv %d pic %d endif ", 
						STAT_SCOREBOARD, ((ateam+1)%2 ? LEFT_OFFSET : LEFT_OFFSET + RIGHT_WIDTH), 
						( ateam  < 2 ? TOP_TEXT_OFFSET  : TOP_TEXT_OFFSET  + DOWN_WIDTH), STAT_PIC_TEAM1 + x);
				strcat(ctc_statusbar, teamBar);

				yt = yt + 26;
				ateam++;
			}
		}
	}

	// Make sure we dont blow anything out of the water
	if (strlen(ctc_statusbar) > 1024)
	{
		gi.dprintf("HOLY COW: Statusbar too big %d\n", strlen(ctc_statusbar));
		ctc_statusbar[1024] = 0;
	}
}
//
// ----------------------------------------------------------------------------------------
//	Cleanup routine for when CTC Ends
//
void Chicken_EndIt(edict_t *ent)
{
	edict_t *e	  = NULL;
	int i;

	Chicken_RemoveFromInventory();

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;

		if (e->client)	// Pull everyone out of there menu's. Dont have to just want too..
			e->client->displayMenu = false;

		// Remove anything that could be laying around from the game
		if (!strcmp(e->classname, "item_chicken") ||
		    !strcmp(e->classname, "feather")      ||
		    !strcmp(e->classname, "chicken_timer"))
		{
			e->nextthink	= 0;
			e->health		= 0;
			G_FreeEdict (e);
		}
	}

#ifdef STD_LOGGING	
	if (loggingStarted)
	{
		loggingStarted = false;
		sl_GameEnd( &gi, level );	// StdLog - Mark Davies
	}
#endif

	// Reset globals
	chickenGame			= false;
	sprintf(gameStatusString,"start game");

	if (strlen(SOUND_GAME_END))
		gi.sound(ent, CHAN_AUTO + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_GAME_END),  1, ATTN_NORM, 0);	

	// Back to Deathmatch
}
//
// ----------------------------------------------------------------------------------------
//	Called when player wants to drop chicken from inventory
//
void Chicken_Drop(edict_t *ent,struct gitem_s *item)
{
	// Dont want to crash is something passes crap
	if (ent && item)
	{
		// If players can drop and enough time has expired to allow a drop then do so
		if (!ent->client->pers.connected || (canDrop && ent->client->chickenTimer <= level.time))
			Chicken_Toss(ent,item);
		else
		{
			if (ent->client->nextInformTime < level.time)
			{
				if (canDrop)
				{
//					gi.cprintf (ent, PRINT_HIGH, "Can't drop %d second%s left\n", (int) (ent->client->chickenTimer - level.time + 1), ((int)(ent->client->chickenTimer - level.time + 1)) == 1?"":"s");
				}
				else
					gi.cprintf (ent, PRINT_HIGH, "You can't drop the chicken\n");

				ent->client->nextInformTime = level.time + 4.0;
			}
		}
	}
	else
		gi.dprintf("Chicken_Drop NULL");
}
//
// ----------------------------------------------------------------------------------------
//	Called when player dies and cant keep chicken
//
void Chicken_Toss(edict_t *ent,struct gitem_s *item)
{
	// Can never be too safe. Check for crap
	if (ent && item && ent->client && !ent->client->isThrowing)
	{
		edict_t *chicken = Drop_Item (ent, item);

		if (chicken) 
			Chicken_Setup (chicken, ent);

		// Remove chicken from players inventory
		Chicken_RemoveFromInventory();
	}
}
//
// ----------------------------------------------------------------------------------------
//	Makes chicken seem to float on water. 
//
static void Chicken_Float(edict_t *self)
{
	qboolean		swimming	= false;
	static qboolean	resetBob	= true;
	
	if (self)
	{
		if (self->watertype & CONTENTS_WATER) 
		{
			vec3_t	headLoc;
			vec3_t	chickSize = {  0,  0, 11};

			wasInWater = true;
			resetBob   = true;

			self->s.angles[0] = 0;
			self->s.angles[1] = 0;

			VectorAdd (self->s.origin, chickSize, headLoc);

			if (!(gi.pointcontents (headLoc) & (CONTENTS_SOLID | CONTENTS_WINDOW)))
			{
				self->s.origin[2] += 2;
				gi.linkentity (self);
			}
		}
		else
		{
			// If chicken is no longer in water and he was then make him bob
			if (wasInWater)
			{
				// Stop him from falling back into the water
				self->movetype = MOVETYPE_NONE;

				if (resetBob)
				{
					self->s.angles[0] = 0;
					self->s.angles[1] = 0;
					resetBob = false;
				}
				else
				{
					// Start the bobbing effect
					self->s.angles[0] += (10.0 * crandom());
					self->s.angles[1] += (10.0 * crandom());
					resetBob = true;
				}
			}
			else
				resetBob = true;
		}
	}
	else
		gi.dprintf("Chicken_Float NULL");
}
//
// ----------------------------------------------------------------------------------------
//	Each server frame update any action chicken has to do
//
static void Chicken_AdvanceFrame(edict_t *self)
{
	static qboolean wasSwimming = false;

	// If chicken was in water then make him swim
	if (wasInWater)
	{
		if (wasSwimming == false)
		{
			wasSwimming = true;
			self->s.frame = FRAME_swim01;
			if (strlen(SOUND_SWIM))
				gi.sound(self, CHAN_WEAPON, gi.soundindex(SOUND_SWIM),  1, ATTN_NORM, 0);	
		}
		else
			self->s.frame++;

		if (self->s.frame > FRAME_swim07)
			self->s.frame = FRAME_swim01;
	}
	else
	{
		if (wasSwimming == true)
		{
			wasSwimming = false;
			self->s.frame = FRAME_stand01;
		}

		// If we have finished our previous animation do something different
		if (self->s.frame == FRAME_stand01)
		{	
			int nextAction = (random()*20.0);

			// Add random effect
			switch(nextAction)
			{
				case 0:  case 1:  case 2:
				case 3:  case 4:  case 5:
				case 6:
					// Stand around for a while
					break;

				case 7:	 case 8:  case 9:
				case 10: case 11:
					if (strlen(SOUND_CHICKEN_PECK))
						gi.sound(self, CHAN_AUTO, gi.soundindex(SOUND_CHICKEN_PECK),  1, ATTN_NORM, 0);	
					self->s.frame = FRAME_peck01;
					break;

				case 12:  case 13:  case 14:
				case 15:
					if (strlen(SOUND_CHICKEN_SCRATCH))
						gi.sound(self, CHAN_AUTO, gi.soundindex(SOUND_CHICKEN_SCRATCH),  1, ATTN_NORM, 0);	
					self->s.frame = FRAME_scr01;
					break;

				case 16:
					self->s.frame = FRAME_look01;
					break;

				case 17:
					self->s.frame = FRAME_wob01;
					break;

				case 18:
					self->s.frame = FRAME_walkr01;
					self->ideal_yaw -= 30;
					self->yaw_speed = 30.0/10.0;
					break;

				case 19:
					self->s.frame = FRAME_walkl01;
					self->ideal_yaw += 30;
					self->yaw_speed = 30.0/10.0;
					break;
			}
		}
		else
		{
			if (self->s.frame == FRAME_peck10 || self->s.frame == FRAME_wob25  ||
				self->s.frame == FRAME_scr23  || self->s.frame == FRAME_look23)
				self->s.frame = FRAME_stand01;
			else
			{
				// Make Chicken Peck after any change of angle
				if (self->s.frame == FRAME_walkl10  || self->s.frame == FRAME_walkr10)
				{
					if (strlen(SOUND_CHICKEN_PECK))
						gi.sound(self, CHAN_AUTO, gi.soundindex(SOUND_CHICKEN_PECK),  1, ATTN_NORM, 0);	
					self->s.frame = FRAME_peck01;
				}
				else 
				{
					if (self->s.frame >= FRAME_walkl01 || self->s.frame <= FRAME_walkl10 ||
						self->s.frame >= FRAME_walkr01 || self->s.frame <= FRAME_walkr10)
						M_ChangeYaw (self);

					self->s.frame++;
				}
			}
		}
	}
}
//
// ----------------------------------------------------------------------------------------
//	Check if chicken should auto respawn
//
static qboolean Chicken_CheckAutoRespawn(edict_t *self)
{
	if (autoRespawn && level.time > self->nextRespawn && clientCount > 0)
	{
		Chicken_Spawn();
		gi.bprintf(PRINT_HIGH, "Chicken has auto respawned\n"); 
		return true;
	}
	return false;
}
//
// ----------------------------------------------------------------------------------------
//	Make chicken make some noise so it can be located
//
static void Chicken_RandomSound(edict_t *self)
{
	static int lastPlayedFrame = 0;

	if (self)
	{
		if (lastPlayedFrame >= 50)
		{
			float playSound		= random();
			char  *soundToPlay	= "";

			if (playSound > 0.50 && playSound <= 0.66)
				soundToPlay = SOUND_RANDOM1;
			else if (playSound > 0.66 && playSound <= 0.82)
				soundToPlay = SOUND_RANDOM2;
			else if (playSound > 0.82)
				soundToPlay = SOUND_RANDOM3;

			if (strlen(soundToPlay))
				gi.sound(self, CHAN_AUTO, gi.soundindex(soundToPlay),  1, ATTN_NORM, 0);	

			lastPlayedFrame = 0;
		}
		else
			lastPlayedFrame++;
	}
	else
		gi.dprintf("Chicken_RandomSound NULL");
}
//
// ----------------------------------------------------------------------------------------
//	Main Chicken thinking routine calls all action function to make chicken do stuff
//
static void Chicken_Think (edict_t *self)
{
	if (self)
	{
		//
		// Check for auto respawn if allowed
		//
		if (Chicken_CheckAutoRespawn(self))
			return;

		//
		// Sound the chicken squawk
		//
		Chicken_RandomSound(self);

		// 
		// Check and Make Chicken Float if in Water
		// 
		Chicken_Float(self);

		//
		// Work out what chicken is in
		//
		self->watertype = gi.pointcontents (self->s.origin);

		//
		// If chicken is in the lava or slime force respawn
		//
		if (self->watertype & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			Chicken_Spawn();
			gi.bprintf(PRINT_HIGH, "Chicken Respawned out of danger\n");	
		}

		//
		// Do next Animation Frame
		//
		Chicken_AdvanceFrame(self);

		self->nextthink		= level.time + FRAMETIME;
		gi.linkentity (self);
	}
	else
		gi.dprintf("Chicken_Think NULL");
}
//
// ----------------------------------------------------------------------------------------
//	Flying Chicken thinking routine called for throw chicken
//
static void Chicken_FlyThink (edict_t *self)
{
	if (self)
	{
		if (self->groundentity)
		{
			// Reset model back to standing on ground
			gi.setmodel (self, MODEL_CHICKEN_ITEM);

			self->s.frame     = 0;
			self->s.angles[0] = 0;

			// Go into normal think rountine
			self->think = Chicken_Think; 
		}

		self->nextthink		= level.time + FRAMETIME;
	}
	else
		gi.dprintf("Chicken_FlyThink NULL");
}
//
// ----------------------------------------------------------------------------------------
//	This function is to prevent player from picking up a freshly dropped chicken
//
static void Chicken_Drop_Temp(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner && ent->teleport_time > level.time)
	{
		if (ent->owner && ent->owner->client && ent->owner->client->nextInformTime < level.time)
		{
//			gi.cprintf (ent->owner, PRINT_HIGH, "Can't pickup %d second%s left\n", (int) (ent->teleport_time - level.time + 1), ((int)(ent->teleport_time - level.time + 1)) == 1?"":"s");
			gi.cprintf (ent->owner, PRINT_HIGH, "Can't pickup chicken yet\n");
			ent->owner->client->nextInformTime = level.time + 4.0;
		}
		return;
	}

	Touch_Item (ent, other, plane, surf);
}
//
// ----------------------------------------------------------------------------------------
//	Shoot chicken at something
//
static void Chicken_Fire(edict_t *ent)
{
	if (ent && ent->client)
	{
		// If not in teamplay just make chicken do dumb stuff
		if (teams == 0 && !canDrop)
		{
			if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
			{
				if (ent->client->ps.gunframe == 15 && strlen(SOUND_FIRE_WEAPON))
					gi.sound(ent, CHAN_AUTO, gi.soundindex(SOUND_FIRE_WEAPON),  1, ATTN_NORM, 0);	

				ent->client->weapon_sound = 0;
				ent->client->ps.gunframe++;
			}
		}
		else
		{
			vec3_t	offset;
			vec3_t	forward, right;
			vec3_t	start, up;
			float	timer;
			int		speed;
			edict_t	*chicken;

			timer = ent->client->grenade_time - level.time;
			speed = CHICKEN_MINSPEED + (CHICKEN_TIMER - timer) * ((CHICKEN_MAXSPEED - CHICKEN_MINSPEED) / CHICKEN_TIMER);

			if ((chicken = G_Spawn()) != NULL)
			{
				Chicken_Setup(chicken, ent);

				gi.setmodel (chicken, MODEL_CHICKEN_FLY);

				VectorSet		(offset, 8, 8, ent->viewheight-8);
				AngleVectors	(ent->client->v_angle, forward, right, up);
				P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

				VectorCopy  (start, chicken->s.origin);

				VectorScale (forward, speed, chicken->velocity);
				VectorMA    (chicken->velocity, 200 + crandom() * 10.0, up, chicken->velocity);
				VectorMA    (chicken->velocity, crandom() * 10.0, right, chicken->velocity);

				chicken->nextthink		= level.time + FRAMETIME;
				chicken->think			= Chicken_FlyThink; 

				VectorCopy (ent->s.angles, chicken->s.angles);

				gi.linkentity (chicken);

				ent->client->isThrowing = true;
			}
		}
	}
	else
		gi.dprintf("Chicken_Fire NULL");
}

//
// ----------------------------------------------------------------------------------------
//	Chicken gun animation handling
//
void Chicken_Weapon (edict_t *ent)
{
	static int	pause_frames[]	 = {32, 0};
	static int	fire_frames[]	 = {15, 0};
	static int  count			 = 1;
	static int  nextThrowFeather = 0;

	if (ent && ent->client && ent->client->bIsCamera != 1)
	{
		if (count >= nextThrowFeather)
		{
			// Throw random feathers
			Chicken_ThrowFeather (ent, 1);
			nextThrowFeather = feathers;
			
			count = 0;
		}
		count++;

		// If not in teamplay use generic weapon handling functions 
		if (teams == 0 && !canDrop)
		{
			if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
			{
				Weapon_Generic (ent, 14, 31, 60, 61, pause_frames, fire_frames, Chicken_Fire);

				// Play random sound so people can locate player 
				Chicken_RandomSound(ent);
			}
		}
		else
		{
			if (ent->client->weaponstate == WEAPON_ACTIVATING)
			{
				// Show the reading chicken animation up till its ready
				if (ent->client->ps.gunframe == 14)
				{
					ent->client->weaponstate = WEAPON_READY;
					ent->client->ps.gunframe = 32;
				}
				else
					ent->client->ps.gunframe++;

				return;
			}

			// Chicken ready to throw
			if (ent->client->weaponstate == WEAPON_READY)
			{
				// Play random sound so people can locate player 
				Chicken_RandomSound(ent);

				// Is the fire button down
				if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
				{
					if (!ent->client->pers.connected || (canDrop && ent->client->chickenTimer <= level.time))
					{
						ent->client->latched_buttons	&= ~BUTTON_ATTACK;
						ent->client->ps.gunframe		= 62;
						ent->client->weaponstate		= WEAPON_FIRING;
						ent->client->grenade_time		= 0;
						return;
					}
					else
					{
						if (ent->client->nextInformTime < level.time)
						{
							if (canDrop)
								gi.cprintf (ent, PRINT_HIGH, "Can't throw %d second%s left\n", (int) (ent->client->chickenTimer - level.time + 1), ((int)(ent->client->chickenTimer - level.time + 1)) == 1?"":"s");
							else
								gi.cprintf (ent, PRINT_HIGH, "You can't throw the chicken\n");

							ent->client->nextInformTime = level.time + 4.0;
						}
					}
				}

				// Place pause every now and then
				if (ent->client->ps.gunframe == 32 && rand()&15)
					return;

				if (++ent->client->ps.gunframe > 60)
					ent->client->ps.gunframe = 32;
				return;
			}

			if (ent->client->weaponstate == WEAPON_FIRING)
			{
				if (ent->client->ps.gunframe == 63)
				{
					if (!ent->client->grenade_time)
					{
						// Generate a time for when chicken gets angry
						ent->client->grenade_time = level.time + CHICKEN_TIMER + 0.2;
						ent->client->weapon_sound = 0;

						if (strlen(SOUND_READY_TO_THROW))
							gi.sound(ent, CHAN_WEAPON, gi.soundindex(SOUND_READY_TO_THROW),  1, ATTN_NORM, 0);	
					}

					// they waited too long, let chicken peck em to death
					if (level.time >= ent->client->grenade_time)
					{
						if (strlen(SOUND_CHICKEN_ANGRY))
							gi.sound(ent, CHAN_WEAPON, gi.soundindex(SOUND_CHICKEN_ANGRY),  1, ATTN_NORM, 0);	

						T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 2, 0, DAMAGE_NO_KNOCKBACK, MOD_CHICKEN);

						if (ent->s.modelindex2 == 0)
							return;
					}

					// Keep adding momentum if fire is held
					if (ent->client->buttons & BUTTON_ATTACK)
						return;

					// Where here if player has started to throw chicken

					// Change chicken item gun model to show hand throwing chicken
					chickenItem->view_model  = MODEL_WEAPON_THROW;
					ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
					ent->client->ps.gunframe = -1;
				}

				// Last throw frame
				if (ent->client->ps.gunframe == 4)
				{
					// Remove from players inventory once thrown
					Chicken_RemoveFromInventory();
					return;
				}

				if (ent->client->ps.gunframe == 1)
					Chicken_Fire(ent);

				ent->client->ps.gunframe++;
			}
		}
	}
}
//
// ----------------------------------------------------------------------------------------
//	Select chicken spawn spot
//
static edict_t *Chicken_FindSpawnSpot(edict_t *ent)
{
	edict_t		*spot = NULL;

	if (randomSpawn)
		spot = SelectRandomDeathmatchSpawnPoint ();
	else
		spot = SelectFarthestDeathmatchSpawnPoint ();

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

		if (!spot && !game.spawnpoint[0] && (spot = G_Find(spot, FOFS(classname), "info_player_start")) == NULL)
			gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
	}

	return (spot);
}
//
// ----------------------------------------------------------------------------------------
//	Setup new chicken defaults
//
static void Chicken_Setup(edict_t *chicken, edict_t *owner)
{
	teamWithChicken			= -1;
	chicken->classname		= chickenItem->classname;
	chicken->item			= chickenItem;
	chicken->spawnflags		= DROPPED_PLAYER_ITEM;
	chicken->s.effects		= chickenItem->world_model_flags;
	chicken->s.renderfx		= 0;
	VectorSet (chicken->mins, -7, -7, -15);
	VectorSet (chicken->maxs,  7,  7,   0);
	gi.setmodel (chicken, MODEL_CHICKEN_ITEM);
	chicken->solid			= SOLID_TRIGGER;
	chicken->movetype		= MOVETYPE_STEP;
	chicken->enemy			= NULL;
	chicken->nextRespawn	= level.time + autorespawntime;
	chicken->s.frame		= 0;
	chicken->touch			= Chicken_Drop_Temp;
	chicken->owner			= owner;
	chicken->think			= Chicken_Think; 

	chicken->teleport_time  = level.time + cantTouchDelay;

	// Mask Chicken so that it can stand on spawn points when dropped
	chicken->clipmask		= MASK_PLAYERSOLID;

	wasInWater  = false;
}
//
// ----------------------------------------------------------------------------------------
//	Spawn a chicken
//
void Chicken_Spawn()
{
	edict_t		*chicken	= NULL;
	edict_t		*spot		= NULL;
	edict_t		*e			= NULL;
	int i;

	if (!deathmatch->value)
		return;

	// Remove any abnormalies. This shouldn't have been called otherwise
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;

		// Remove chicken item from game
		if (!strcmp(e->classname, "item_chicken"))
		{
			e->nextthink	= 0;
			e->health		= 0;
			G_FreeEdict (e);
		}
	}

	Chicken_RemoveFromInventory();

	if ((chicken = G_Spawn()) != NULL)
	{
		Chicken_Setup(chicken, NULL);

		spot = Chicken_FindSpawnSpot(chicken);

		chicken->nextthink		= level.time + FRAMETIME;
		chicken->s.frame		= FRAME_stand01;
		chicken->owner			= NULL;

		VectorCopy (spot->s.origin, chicken->s.origin);
		chicken->s.origin[2] += 2;
		VectorCopy (spot->s.angles, chicken->s.angles);
		M_droptofloor(chicken);
		gi.linkentity (chicken);

		// Send Spawn Effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (chicken-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (chicken->s.origin, MULTICAST_PVS);

		if (strlen(SOUND_CHICKEN_SPAWN))
			gi.sound(chicken, CHAN_AUTO + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_CHICKEN_SPAWN), 1, ATTN_NONE, 0);

		chickenGame = true;
		wasInWater  = false;

		respawnTime	= level.time + spawnDelay;
		sprintf(gameStatusString, "end game");
	}
	else
		gi.dprintf("Respawn chicken failed.");
}
//
// ----------------------------------------------------------------------------------------
//	Get clients model name
// 
char *Chicken_GetModelName(edict_t *ent)
{
	int				len;
	static	char	model[128];
	char			*skin = Info_ValueForKey(ent->client->pers.userinfo, "skin");

	// Fetch back model of player
	for(len = 0; *(skin+len); len++)
	{
		if(*(skin+len) == '/')
		{
			model[len] = '\0';
			break;
		}
		else 
			model[len] = *(skin+len);
	}

	return (model);
}
//
// ----------------------------------------------------------------------------------------
/*
void ShowGun(edict_t *ent)
{	
	char heldmodel[128];
	int n;

	if(!ent->client->pers.weapon)
	{
		ent->s.modelindex2 = 0;
		return;
	}
	ent->s.modelindex2 = 255;
	strcpy(heldmodel, "#");
	strcat(heldmodel, ent->client->pers.weapon->icon);	
	strcat(heldmodel, ".md2");
	//gi.dprintf ("%s\n", heldmodel);
	n = (gi.modelindex(heldmodel) - vwep_index) << 8;

	ent->s.skinnum &= 0xFF;
	ent->s.skinnum |= n;
}
*/
#ifndef WIN32
#include <unistd.h>
#endif

// 
void ShowGun(edict_t *ent)
{
	char heldmodel[128];
	char skin[128];
	int len, n;
	int gotSpecial = ent->client->pers.inventory[chickenItemIndex];

	if (!ent->client->pers.weapon)
	{
		ent->s.modelindex2 = 0;
		return;
	}

	ent->s.modelindex2 = 255;

	strcpy(heldmodel, "#");
	strcat(heldmodel, ent->client->pers.weapon->icon);	
	strcat(heldmodel, ".md2");
	n = (gi.modelindex(heldmodel) - vwep_index) << 8;

	ent->s.skinnum &= 0xFF;
	ent->s.skinnum |= n;

	if (chickenGame)
	{
		strcpy(skin, Info_ValueForKey (ent->client->pers.userinfo, "skin"));

		for(len = 0; skin[len]; len++)
		{
			if(skin[len] == '/')
			{
				skin[len] = '\0';
				break;
			}
		}

		// While we are here if we havent tested for the chicken model check now
		if (ent->client->gotChickenModel == -1)
		{
			char model[128];

			strcpy(model, "baseq2\\players\\");
			strcat(model, skin);
			strcat(model, "\\w_chicken.md2");

			if (access(model, 2) == 0)
				ent->client->gotChickenModel = 1;
			else
				ent->client->gotChickenModel = 0;
		}

		strcpy(heldmodel, "players/");
		strcat(heldmodel, skin);
		strcat(heldmodel, "/");
		strcat(heldmodel, ent->client->pers.weapon->icon);	
		strcat(heldmodel, ".md2");

		if (ent->client->pers.inventory[chickenItemIndex] > 0)
		{
			if (ent->client->gotChickenModel > 0)
				ent->s.modelindex2 = gi.modelindex(heldmodel);
			else
			{
				int			currentModel = 0;
				char		*model = NULL;

				model = Chicken_GetModelName(ent);

				// Go through list of possible player models and display appropriate model
				// If player does not have the correct model use first one in list regardless
				do 
				{
					if (!strcmp(model, playerModels[currentModel].playerModel))
						break;
					currentModel++;
				}
				while (currentModel < MAX_PLAYER_MODELS);

				if (currentModel >= MAX_PLAYER_MODELS)
					currentModel = 0;

				strcpy(heldmodel, playerModels[currentModel].chickenModel);
				ent->s.modelindex2 = gi.modelindex(heldmodel);
			}

			if (ent->s.modelindex2 == 0)
				ent->s.modelindex2 = 255;

			return;
		}
	}
}
//
// ----------------------------------------------------------------------------------------
//	Pickup chicken and place in players inventory
//
qboolean Chicken_Pickup(edict_t *ent, edict_t *other)
{	
	if (ent && ent->item && other && other->client)
	{
		Chicken_RemoveFromInventory();

		gi.bprintf(PRINT_HIGH, "%s's picked up the Chicken\n", other->client->pers.netname);

		if (other->flags & FL_GODMODE)
		{
			other->flags ^= FL_GODMODE;
			gi.cprintf(other, PRINT_HIGH, "godmode OFF (Chickens don't like cheats)\n");
		}

		// Add it to the inventory
		other->client->pers.inventory[chickenItemIndex] = 1;
		other->client->newweapon	= chickenItem;
		ent->enemy					= other;

		// Make weapon change straight away. Had problems with firing weapons
		if (other->client->ammo_index)
			other->client->ammo_index = 0;

		// Set when player can drop chicken. Is overriden if dropping not allowed
		other->client->chickenTimer = level.time + dropDelay;

		// Set players gun model
		ShowGun(other);

		// Start the timer
		Chicken_ClockStart (other);

		if (strlen(SOUND_CHICKEN_PICKUP))
			gi.sound(ent, CHAN_AUTO + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_CHICKEN_PICKUP),  1, ATTN_NORM, 0);	

		if (teams)
		{
			teamWithChicken = other->client->team;
			Chicken_TeamReadyEggGun(teamWithChicken);
		}
	}
	else
		gi.dprintf("Chicken_Pickup NULL");

	return true;
}
//
// ----------------------------------------------------------------------------------------
//	Chicken Clock Routines
// ----------------------------------------------------------------------------------------
//	Timer think routine used to update timer on screen and add score
//
static void Chicken_ClockThink (edict_t *timer)
{
	qboolean killTimer = false;

	if (timer && timer->enemy && timer->enemy->client)
	{
		struct gclient_s *client = timer->enemy->client;

		if (client->pers.inventory[chickenItemIndex] == 0 || timer->enemy->deadflag == DEAD_DEAD)
			killTimer = true;
		else
		{
			timer->health++;
			timer->nextthink = level.time + 1;

			// Update the clock
			client->ps.stats[STAT_CHICKEN_TIME_MIN]  = (timer->health - 1)/60;
			client->ps.stats[STAT_CHICKEN_TIME_1SEC] = ((timer->health - 1)/10)%6;
			client->ps.stats[STAT_CHICKEN_TIME_2SEC] = (timer->health - 1)%10;

			// Should we play a sound?
			if (teams)
			{	
				if (strlen(SOUND_TIME_UP) && (scorePeriod*maxHoldScore) == (timer->health-1))
					gi.sound(timer->enemy, CHAN_ITEM, gi.soundindex(SOUND_TIME_UP),   1, ATTN_STATIC, 0);	
				else if (strlen(SOUND_TIME_LOW) && ((scorePeriod*maxHoldScore) - TIME_LOW_TIMEOUT + 1) <= timer->health)
					gi.sound(timer->enemy, CHAN_ITEM, gi.soundindex(SOUND_TIME_LOW),  1, ATTN_STATIC, 0);
			}

			// Has player scored yet..
			if (((timer->health-1)%scorePeriod) == 0 && timer->health > 1)
			{
				qboolean score = true;

				if (teams)
				{
					if (client->team < MAX_TEAMS && client->holdScore <= maxHoldScore)
					{
						client->holdScore++;
						teamDetails[client->team].score++;

							// If we cant score anymore kill the timer
						if (client->holdScore == maxHoldScore)
							killTimer = true;
					}
					else
						score = false;
				}
				else
					client->resp.score++;
#ifdef STD_LOGGING
				if (score)
					sl_LogScore( &gi, client->pers.netname, NULL, "Chicken Held", NULL, 1,level.time );
#endif
			}
			gi.linkentity(timer);
		}
	}
	else
		killTimer = true;

	if (killTimer)
	{
		// Fail safe. Somethings wrong so kill the timer
		timer->nextthink		= 0;
		timer->health			= 0;
		G_FreeEdict (timer);
	}
}
//
// ----------------------------------------------------------------------------------------
//	
//
void Chicken_ResetScoreboard()
{
	int team = 0;

	for (team = 0; team < MAX_TEAMS; team++)
		teamDetails[team].score = 0;
}
//
// ----------------------------------------------------------------------------------------
//	Start a timer since someone musta picked up the chicken
//
static void Chicken_ClockStart (edict_t *ent)
{
	if (ent)
	{
		edict_t *timer = G_Spawn();

		if (timer)
		{
			timer->classname	= "chicken_timer";
			timer->activator	= ent;
			timer->enemy		= ent;
			timer->health		= 0;
			timer->wait			= 0;
			timer->inuse		= true;
			timer->think		= Chicken_ClockThink;
			timer->nextthink	= level.time + 1;

			gi.linkentity(timer);
		}
	}
	else
		gi.dprintf("Chicken_ClockStart NULL");
}
//
// ----------------------------------------------------------------------------------------
//	You guest it when a player dies this is called. Puts up lame messages
//
void Chicken_ScoreCheck(edict_t *self, edict_t *inflictor, edict_t *attacker, int meansOfDeath)
{
	if (chickenGame && scoreOnDeath && self->client->pers.inventory[chickenItemIndex] > 0 && !self->client->isThrowing)
	{
		// If player gets score for killing chicken dude then give it
		attacker->client->resp.score++;
#ifdef STD_LOGGING
		{
			char *pWeaponName = NULL;

			// self->enemy = attacker;
			if( attacker && attacker->client )
			{
				switch (meansOfDeath)
				{
					case MOD_BLASTER:
					case MOD_SHOTGUN:
					case MOD_SSHOTGUN:
					case MOD_MACHINEGUN:
					case MOD_CHAINGUN:
					case MOD_GRENADE:
					case MOD_G_SPLASH:
					case MOD_ROCKET:
					case MOD_R_SPLASH:
					case MOD_HYPERBLASTER:
					case MOD_RAILGUN:
					case MOD_BFG_LASER:
					case MOD_BFG_BLAST:
					case MOD_BFG_EFFECT:
					case MOD_HANDGRENADE:
					case MOD_HG_SPLASH:
					case MOD_HELD_GRENADE:
					default:
					{
						/* Get weapon name, being very careful - mdavies */
						/* This weapon may not have been the weapon used if the weapon was changed before the death */
						pWeaponName = (NULL != attacker->client->pers.weapon)?(attacker->client->pers.weapon->pickup_name):(NULL);
						break;
					}

					case MOD_TELEFRAG:
					{
						/* Set weapon name - mdavies */
						pWeaponName = "Telefrag";
						break;
					}                            
				}
			}
			sl_LogScore( &gi, attacker->client->pers.netname, self->client->pers.netname,
						 "Kill", pWeaponName, 1, level.time );
		}
#endif
	}
}
//
// ----------------------------------------------------------------------------------------
//	You guest it when a player dies this is called. Puts up lame messages
//
qboolean Chicken_PlayerDie(edict_t *self, edict_t *inflictor, edict_t *attacker, int meansOfDeath)
{
	if (chickenGame)
	{
		// Cleanup player just in case player was in middle of throw animation

		// Set chicken item model back before we forget
		// Chicken_Fire takes care of grenade_time and weapon changing
		chickenItem->view_model  = MODEL_WEAPON_NORMAL;

		self->client->ps.gunindex = gi.modelindex(self->client->pers.weapon->view_model);
		self->client->ps.gunframe = 0;

		// Set grenade time to zero else exploding chicken
		self->client->grenade_time = 0;

		// Allow player to score again
		self->client->holdScore = 0;

		if (strlen(SOUND_CHICKEN_DIE))
			gi.sound(self, CHAN_WEAPON, gi.soundindex(SOUND_CHICKEN_DIE),  1, ATTN_NORM, 0);	

		if (self != attacker)
		{
			if (self && self->client)
			{
				self->enemy = attacker;
				if (attacker && attacker->client)
				{
					float which = random();

					if (which < 0.50)
					{
						int msgOff = (int)(((float)sizeof(killerKilled) / sizeof(char *)) * random());
						gi.bprintf (PRINT_MEDIUM, killerKilled[(int)msgOff], attacker->client->pers.netname, self->client->pers.netname);
					}
					if (which >= 0.50)
					{
						int msgOff = (int)(((float)sizeof(killer) / sizeof(char *)) * random());
						gi.bprintf (PRINT_MEDIUM,killer[(int)msgOff], attacker->client->pers.netname);
					}

				}
				else
				{
					int msgOff = (int)(((float)sizeof(killed) / sizeof(char *)) * random());
					gi.bprintf (PRINT_MEDIUM,killed[(int)msgOff], self->client->pers.netname);
				}
			}
			else
				gi.dprintf("Chicken_Die NULL 1");
		}
		else
		{
			if (self)
			{
				int msgOff = (int)(((float)sizeof(killedSelf) / sizeof(char *)) * random());

				self->enemy = NULL;
				gi.bprintf (PRINT_MEDIUM,killedSelf[(int)msgOff], self->client->pers.netname);
			}
			else
				gi.dprintf("Chicken_Die NULL 2");
		}

		return true;
	}
	else
		return false;
}
//
// ----------------------------------------------------------------------------------------
//	Checks to see if a player can take damage while in CTC
//	If true is returned quake will do its normal stuff otherwise no effect
//
int Chicken_TakeDamage(edict_t *self, edict_t *attacker, int damage)
{
	// If player was attacked by the world. Let quake handle it
	if (attacker == world || (attacker && !attacker->client))
		return true;

	// Allow pain to dead player corpses
	if (self->client == NULL)
		return true;

	if (chickenGame)
	{
		vec3_t	vieworg;

		// If players got the chicken there fair game
		if (self && self->client && self->client->pers.inventory[chickenItemIndex] > 0)
		{
			char *painSound = NULL;

			if (damage < 25)						painSound = SOUND_CHICKEN_HURT_25;
			else if (damage >= 25 && damage < 50)	painSound = SOUND_CHICKEN_HURT_50;
			else if (damage >= 50 && damage < 75)	painSound = SOUND_CHICKEN_HURT_75;
			else									painSound = SOUND_CHICKEN_HURT_100;

			if (painSound && strlen(painSound))
				gi.sound(self, CHAN_ITEM, gi.soundindex(painSound),  1, ATTN_NORM, 0);	
			return true;
		}

		if (self && self->client)
		{
			VectorAdd (self->s.origin, self->client->ps.viewoffset, vieworg);

			// If in crap. Too bad you die
			if (gi.pointcontents (vieworg) & (CONTENTS_LAVA|CONTENTS_SLIME))
				return true;

			// If player was attacked by enemy team then ouch!!
			if (teams != 0 && teamWithChicken == self->client->team)
				return true;
		}
	}
	else
		return true;

	// Players safe.. I think
	return false;
}
//
// ----------------------------------------------------------------------------------------
//	Chucks a specified number of feathers out from player with chicken
//
static void Chicken_ThrowFeather (edict_t *self, int count)
{
	// Loop around creating count number of feathers
	while (count > 0)
	{
		edict_t *feather;
		vec3_t	origin, size;

		if ((feather = G_Spawn()) != NULL)
		{
			feather->classname	= "feather";
			VectorScale (self->size,	0.5,	size);
			VectorAdd   (self->absmin,	size,	origin);

			feather->s.origin[0] = origin[0] + crandom() * size[0];
			feather->s.origin[1] = origin[1] + crandom() * size[1];
			feather->s.origin[2] = origin[2] + crandom() * size[2];

			gi.setmodel (feather, MODEL_FEATHER);

			feather->solid			= SOLID_NOT;
			feather->s.effects		= 0;
			feather->flags			|= FL_NO_KNOCKBACK;
			feather->takedamage		= DAMAGE_NO;
			feather->die			= NULL;

			feather->movetype		= MOVETYPE_FLY;
			feather->touch			= NULL;
			feather->gravity		= 0.1;

			feather->velocity[0]	= 6.0 * crandom();
			feather->velocity[1]	= 6.0 * crandom();
			feather->velocity[2]	= -6;

			feather->avelocity[0]	= random()*300;
			feather->avelocity[1]	= random()*300;
			feather->avelocity[2]	= random()*300;

			feather->think			= G_FreeEdict;
			feather->nextthink		= level.time + 2 + random()*8;

			gi.linkentity (feather);

			count--;
		}
		else
			count = 0; // Get outta here no more space
	}
}
//
// ----------------------------------------------------------------------------------------
//	Removes chicken from a players inventory
//
static void Chicken_RemoveFromInventory()
{
	//	Removes all timers. Called when no one has the chicken anymore
	edict_t *e	  = NULL;
	int i;

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse) continue;

		// Remove all timers
		if (!strcmp(e->classname, "chicken_timer"))
		{
			e->nextthink = e->health = 0;
			G_FreeEdict (e);
		}

		if (e->client)
		{
			if (e->client->pers.inventory[eggGunItemIndex] > 0) 
			{
				e->client->pers.inventory[eggGunItemIndex]  = 0;

				// Check for dead players, observer or camera players
				if (e->s.modelindex2 != 0)
				{
					NoAmmoWeaponChange(e);
					e->client->pers.selected_item = ITEM_INDEX(e->client->pers.weapon);	
				}
			}

			// Making assumption that if one player throws chicken to another the catching player wont be able
			// to throw it back before the throwers inventory has been make void of chicken Item
			if (e->client->pers.inventory[chickenItemIndex] > 0)
			{
				// Cleanup player just since player was in middle of throw animation

				// Set chicken item model back before we forget
				// Chicken_Fire takes care of grenade_time and weapon changing
				chickenItem->view_model  = MODEL_WEAPON_NORMAL;
				e->client->isThrowing  = false;

				// Set grenade time to zero else exploding chicken
				e->client->grenade_time = 0;

				// Allow player to score again
				e->client->holdScore = 0;

				// Get ride of all chickens. Should only ever be 1 but set it to 0 to make sure
				e->client->pers.inventory[chickenItemIndex] = 0;

				NoAmmoWeaponChange(e);
				if (e->s.modelindex2 != 0) // Check for dead players, observer or camera players
					ChangeWeapon(e);	// Force the change of weapon NOW!

				e->client->ps.gunindex = gi.modelindex(e->client->pers.weapon->view_model);
				e->client->ps.gunframe = 0;
				e->client->pers.selected_item = ITEM_INDEX(e->client->pers.weapon);
			}
		}
	}
}
//
// ----------------------------------------------------------------------------------------
//	Starts and stops CTC
//
static int Chicken_GameStatus(void *data, int itemId)
{
	edict_t *ent = (edict_t *)data;

	if (chickenGame)
	{
#ifdef STD_LOGGING	
		if (loggingStarted)
		{
			loggingStarted = false;
			sl_GameEnd( &gi, level );	// StdLog - Mark Davies
		}
#endif
		// Finish game
		Chicken_EndIt(ent);

		// Inform players game has ended
		if (ent->client)
			gi.bprintf(PRINT_HIGH, "%s ended Catch the Chicken\n", ent->client->pers.netname); 
		else
			gi.bprintf(PRINT_HIGH, "Catch the Chicken ended\n"); 
	}
	else
	{
#ifdef STD_LOGGING	
		if (!loggingStarted)
		{
			loggingStarted = true;
			sl_GameStart( &gi, CHICKEN_STRING_FULLNAME, level );	//	StdLog - Mark Davies
		}
#endif
		// Spawn chicken
		Chicken_Spawn();

		// Inform players game has now started
		if (ent->client)
			gi.bprintf(PRINT_HIGH, "%s started Catch the Chicken\n", ent->client->pers.netname); 
		else
			gi.bprintf(PRINT_HIGH, "Catch the Chicken started\n"); 

		if (strlen(SOUND_GAME_START))
			gi.sound(ent, CHAN_AUTO + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_GAME_START),  1, ATTN_NORM, 0);	
	}

	// Remove menu ready for next command
	ent->client->displayMenu = false;

	return 0;
}
//
// ----------------------------------------------------------------------------------------
//	Force chicken to respawn regardless of where it is
//
int Chicken_Respawn(void *data, int itemId)
{
	edict_t *ent = (edict_t *)data;

	if (chickenGame)
	{
		if (ent == NULL || respawnTime	< level.time)
		{
			Chicken_Spawn ();										
			if (ent && ent->client)
				gi.bprintf(PRINT_HIGH, "%s respawned Chicken\n", ent->client->pers.netname); 
			else
				gi.bprintf(PRINT_HIGH, "Chicken respawned by server\n"); 

			if (ent && strlen(SOUND_CHICKEN_RESPAWN))
				gi.sound(ent, CHAN_AUTO + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_CHICKEN_RESPAWN),  1, ATTN_NORM, 0);	
		}
		else
			gi.cprintf(ent, PRINT_HIGH, "Chicken cannot be respawned yet\n"); 
	}																
	else															
		gi.bprintf(PRINT_HIGH, "Catch the chicken not started\n");	

	return 0;
}
//
// ----------------------------------------------------------------------------------------
//	Create menu for CTC. Should only be called once
//
static void Chicken_MenuCreate()
{
	static qboolean menuSetup = false;

	if (!menuSetup)
	{
		sprintf(gameStatusString,"start game");

		Chicken_MenuInsert(1);
		
		Chicken_MenuItemInsert(1, 2, NULL,							YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(1, 3, gameStatusString,				WHITE,  LEFT,   OPT_SELECT, Chicken_GameStatus);
		Chicken_MenuItemInsert(1, 4,"respawn chicken",				WHITE,  LEFT,   OPT_SELECT, Chicken_Respawn);
		Chicken_MenuItemInsert(1, 5, NULL,							YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(1, 6, "allow small health",			WHITE,  LEFT,   OPT_TOGGLE, &allowSmallHealth,  NULL);
		Chicken_MenuItemInsert(1, 7, "allow big health",			WHITE,  LEFT,   OPT_TOGGLE, &allowBigHealth,    NULL);
		Chicken_MenuItemInsert(1, 8, "allow armor",					WHITE,  LEFT,   OPT_TOGGLE, &allowArmour,	    NULL);
		Chicken_MenuItemInsert(1, 9, "allow invulnerable",			WHITE,  LEFT,   OPT_TOGGLE, &allowInvulnerable, NULL);
		Chicken_MenuItemInsert(1,10, "chicken glow",				WHITE,  LEFT,   OPT_OPTION, &allowGlow,         1, 2, 3, 4, 0, -1);
		Chicken_MenuItemInsert(1,11, "score on kills",    			WHITE,  LEFT,   OPT_TOGGLE, &scoreOnDeath,      NULL);
		Chicken_MenuItemInsert(1,12, "scoring period (sec)",		WHITE,  LEFT,   OPT_OPTION, &scorePeriod, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 5, 1, -1);
		Chicken_MenuItemInsert(1,13, "droppable chicken",			WHITE,  LEFT,   OPT_TOGGLE, &canDrop,           NULL);
		Chicken_MenuItemInsert(1,14, "drop delay (sec)",	        WHITE,  LEFT,   OPT_OPTION, &dropDelay, 5, 10, 15, 20, 25, 30, 0, -1);
		Chicken_MenuItemInsert(1,15, NULL,							YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(1,16, " use [ and ] to move cursor",	YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(1,17, "press enter to select",		YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(1,18, "esc to exit menu",		    YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(1,20, CHICKEN_STRING_MENU_VERSION,	YELLOW, RIGHT,  OPT_FILLER );

		menuSetup = true;
	}
}
//
// ----------------------------------------------------------------------------------------
//	Create menu for TCTC. Should only be called once
//
static void Chicken_MenuTCTCCreate()
{
	static qboolean menuSetup = false;

	if (!menuSetup)
	{
		sprintf(gameStatusString,"start game");

		Chicken_MenuInsert(4);
		
		Chicken_MenuItemInsert(4, 2, NULL,							YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(4, 3,"respawn chicken",				WHITE,  LEFT,   OPT_SELECT, Chicken_Respawn);
		Chicken_MenuItemInsert(4, 4, NULL,							YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(4, 5, "allow small health",			WHITE,  LEFT,   OPT_TOGGLE, &allowSmallHealth,  NULL);
		Chicken_MenuItemInsert(4, 6, "allow big health",			WHITE,  LEFT,   OPT_TOGGLE, &allowBigHealth,    NULL);
		Chicken_MenuItemInsert(4, 7, "allow armor",					WHITE,  LEFT,   OPT_TOGGLE, &allowArmour,	    NULL);
		Chicken_MenuItemInsert(4, 8, "allow invulnerable",			WHITE,  LEFT,   OPT_TOGGLE, &allowInvulnerable, NULL);
		Chicken_MenuItemInsert(4, 9, "chicken glow",				WHITE,  LEFT,   OPT_OPTION, &allowGlow,         1, 2, 3, 4, 0, -1);
		Chicken_MenuItemInsert(4,10, "score on kills",    			WHITE,  LEFT,   OPT_TOGGLE, &scoreOnDeath,      NULL);
		Chicken_MenuItemInsert(4,11, "scoring period (sec)",		WHITE,  LEFT,   OPT_OPTION, &scorePeriod, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 5, 1, -1);
		Chicken_MenuItemInsert(4,12, "max hold score",				WHITE,  LEFT,   OPT_OPTION, &maxHoldScore, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, -1);
		Chicken_MenuItemInsert(4,13, "egg gun kickback",			WHITE,	LEFT,	OPT_OPTION, &kickback, 5, 6, 7, 8, 9, 10, 15, 20, -1);
		Chicken_MenuItemInsert(4,14, "drop delay (sec)",	        WHITE,  LEFT,   OPT_OPTION, &dropDelay, 5, 10, 15, 20, 25, 30, 0, -1);
		Chicken_MenuItemInsert(4,15, NULL,							YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(4,16, " use [ and ] to move cursor",	YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(4,17, "press enter to select",		YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(4,18, "esc to exit menu",		    YELLOW, CENTER, OPT_FILLER );
		Chicken_MenuItemInsert(4,20, CHICKEN_STRING_MENU_VERSION,	YELLOW, RIGHT,  OPT_FILLER );

		menuSetup = true;
	}
}
//
// ----------------------------------------------------------------------------------------
//	Display chicken timer if player has chicken
//
void Chicken_Stats(edict_t *ent)
{
	if (teams != 0)
	{	
		int x;

		if (ent->client->showscores)
		{
			ent->client->ps.stats[STAT_SCOREBOARD] = 1;
			ent->client->ps.stats[STAT_SCORES]	   = 0;

			for (x = 0; x < MAX_TEAMS; x++)
			{
				if (teams & (1 << x)) 
					if (teamWithChicken == x)
						ent->client->ps.stats[STAT_PIC_TEAM1 + x]	= gi.imageindex (teamDetails[x].scoreHoldIcon);
					else
						ent->client->ps.stats[STAT_PIC_TEAM1 + x]	= gi.imageindex (teamDetails[x].scoreIcon);
			}
		}
		else
		{
			ent->client->ps.stats[STAT_SCOREBOARD] = 0;
			ent->client->ps.stats[STAT_SCORES]     = 1;

			for (x = 0; x < MAX_TEAMS; x++)
			{
				if (teams & (1 << x)) 
				{
					ent->client->ps.stats[STAT_DISPLAY_SCORE1 + x]	= teamDetails[x].score;
					if (teamWithChicken == x)
						ent->client->ps.stats[STAT_PIC_TEAM1 + x]	= gi.imageindex (teamDetails[x].holdIcon);
					else
						ent->client->ps.stats[STAT_PIC_TEAM1 + x]	= gi.imageindex (teamDetails[x].icon);
				}
			}
		}
	}

	// Display Clock if player has chicken
	if (ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing)
		ent->client->ps.stats[STAT_DISPLAY_CLOCK]	  = 1;
	else
	{
		ent->client->ps.stats[STAT_DISPLAY_CLOCK]	  = 0;
		ent->client->ps.stats[STAT_CHICKEN_TIME_MIN]  = 0;
		ent->client->ps.stats[STAT_CHICKEN_TIME_1SEC] = 0;
		ent->client->ps.stats[STAT_CHICKEN_TIME_2SEC] = 0;
	}
}
//
// ----------------------------------------------------------------------------------------
//
void Chicken_ObserverEnd(edict_t *ent)
{
	// Turn off observer
	ent->svflags		&= ~SVF_NOCLIENT;

	// Place client back into game
	PutClientInServer(ent);
	
	// Fixup statusbar
	ent->client->ps.stats[STAT_CHICKEN_OBSERVER] = 1;

	// Display MOTD
	ent->client->removeMOTDTime = level.time + MOTDDuration;
	Chicken_MOTD(ent);

}
//
// ----------------------------------------------------------------------------------------
//
void Chicken_CheckOutPlayer(edict_t *ent)
{
	if (teams && ent->client->team != -1)
	{
		teamDetails[ent->client->team].players--;
		sprintf(teamDetails[ent->client->team].menu, "Team %-6.6s %d Players", teamDetails[ent->client->team].colour, teamDetails[ent->client->team].players);
		ent->client->team    = -1;
	}
}
//
// ----------------------------------------------------------------------------------------
//	Check if player should go straight into observer mode when entering game and ensure a supported model
//
void Chicken_CheckInPlayer(edict_t *ent)
{
	// Set up hud stuff for player
	ent->client->ps.stats[STAT_CHICKEN_CLOCK]	= gi.imageindex (ICON_CLOCK);
	ent->client->ps.stats[STAT_CHICKEN_COLON]	= gi.imageindex (ICON_CLOCK_COLON);

	// Setup player so as not to be throwing chicken. Which they aint
	ent->client->isThrowing	= false;

	Chicken_CheckPlayerModel(ent); // Chicken

	if (chickenGame && teams)
	{
		if (ent->client->team != -1 && ent->client->modelOk)
		{
			ent->svflags		&= ~SVF_NOCLIENT;
			ent->client->ps.stats[STAT_CHICKEN_OBSERVER] = 1;
		}
		else
			Chicken_Observer(ent);
	}
	else
	{
		ent->svflags		&= ~SVF_NOCLIENT;
		ent->client->ps.stats[STAT_CHICKEN_OBSERVER] = 1;
	}
}

//
// ----------------------------------------------------------------------------------------
//
void Chicken_ClientBegin(edict_t *ent)
{
	if (chickenGame && deathmatch->value && ((teams != 0 && ent->client->team != -1 && ent->client->modelOk) || teams == 0) && MOTDDuration != 0)
	{
		ent->client->removeMOTDTime = level.time + MOTDDuration;
		Chicken_MOTD(ent);
	}
}
//
// ----------------------------------------------------------------------------------------
//
static void Chicken_SelectPlayerMenu(edict_t *ent, int menuId)
{
	if (ent->client->chickenMenu != menuId)
	{
		Chicken_MenuSelect(ent, menuId);
		ent->client->chickenMenu = menuId;
	}
}
//
// ----------------------------------------------------------------------------------------
//
qboolean Chicken_ShowMenu(edict_t *ent)
{
	qboolean showMenu = true;

	if (teams == 0 && ent->client->displayMenu)
		Chicken_SelectPlayerMenu(ent, 1);
	else
	{
		if (chickenGame && teams)
		{
			if (ent->client->team == -1)
				Chicken_SelectPlayerMenu(ent, 2);
			else if (ent->client->modelOk == false)
			{
				Chicken_SelectPlayerMenu(ent, 3);

				// Make sure player is in observer mode
				// This is necessary since players can change models during games
				if (!(ent->svflags & SVF_NOCLIENT))
					Chicken_Observer(ent);
			}
			else
			{
				if (ent->client->displayMenu)
					Chicken_SelectPlayerMenu(ent, 4);
				else
					showMenu = false;
			}
		}
		else
			showMenu = false;
	}

	return (showMenu || ent->client->displayMenu);
}
//
// ----------------------------------------------------------------------------------------
//	Enter player int observer mode used for both observer function and initial team play
//
static void Chicken_Observer(edict_t *ent)
{
	// If player has chicken get rid of it
	if (ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing)
		Chicken_Toss(ent, chickenItem);

	// Remove menu and any items player has
	ent->client->displayMenu = false;
	memset(ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));
	
	// Player is now in observer mode. 
	// Setting STAT_CHICKEN_OBSERVER to 0 to go into observer mode seems odd but
	// is done this way since I dont know how to ! in statusbar
	ent->client->ps.stats[STAT_CHICKEN_OBSERVER]	  = 0;
	ent->client->pers.selected_item = -1;

	// Reset score and remove gun model
	ent->client->resp.score  = 0;
	ent->client->ps.gunindex = 0;

	if (ent->client->ammo_index)
		ent->client->ammo_index = 0;

	// You can fly
	ent->movetype		= MOVETYPE_NOCLIP;
	ent->solid			= SOLID_NOT;
	ent->svflags		|= SVF_NOCLIENT;

	gi.linkentity (ent);
}
//
// ----------------------------------------------------------------------------------------
//	Place player into observer mode
//
void Chicken_GoObserver(edict_t *ent)
{
	if (!observerAllowed)
		gi.cprintf (ent, PRINT_HIGH, "Observer mode not allowed\n");
	else if (ent->movetype == MOVETYPE_NOCLIP)
		gi.cprintf (ent, PRINT_HIGH, "You are already an observer\n");
	else if (ent->client && ent->client->bIsCamera)
		gi.cprintf (ent, PRINT_HIGH, "You are already a camera\n");
	else if (ent->client)
	{
		Chicken_Observer(ent);

		gi.bprintf(PRINT_HIGH, "%s is now an observer\n",ent->client->pers.netname); 

		if (strlen(SOUND_OBSERVER))
			gi.sound(ent, CHAN_AUTO + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_OBSERVER),  1, ATTN_NORM, 0);	
	}
}
//
// ----------------------------------------------------------------------------------------
//	Remove all stuff from screen when entering camera mode
//
void Chicken_Camera(edict_t *ent)
{
	if (ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing)
		Chicken_Toss(ent,	chickenItem);

	ent->client->displayMenu = false;
	memset(ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));
	
	ent->client->ps.stats[STAT_CHICKEN_OBSERVER]	  = 0;
	ent->client->pers.selected_item = -1;

	gi.bprintf(PRINT_HIGH, "%s is now a camera\n",ent->client->pers.netname); 

	// Reset score and remove gun model
	ent->client->resp.score  = 0;
	ent->client->ps.gunindex = 0;

	if (ent->client->ammo_index)
		ent->client->ammo_index = 0;

	// You can fly
	ent->svflags		|= SVF_NOCLIENT;

	gi.linkentity (ent);

	if (strlen(SOUND_OBSERVER))
		gi.sound(ent, CHAN_AUTO + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_OBSERVER),  1, ATTN_NORM, 0);	
}
//
// ----------------------------------------------------------------------------------------
//	Chicken initalization for first time game is run
//
void Chicken_GameInit()
{
	static qboolean firstTime = true;							

	if (firstTime)												
	{															
		eggGunItem			= FindItem("EggGun");				
		eggGunItemIndex		= ITEM_INDEX(FindItem("EggGun"));	

		chickenItem			= FindItem("Chicken");				
		chickenItemIndex	= ITEM_INDEX(FindItem("Chicken"));	

		Chicken_MenuCreate();					
		Chicken_ReadIni();
		if (levelCycling)
			Chicken_ReadMapList();

		if (MOTDDuration != 0)
			Chicken_MOTDLoad();

		if (teams)
		{
			Chicken_MenuTCTCCreate();
			Chicken_TeamMenuCreate();
			Chicken_PlayerSelectMenuCreate();
		}

		Chicken_CreateStatusBar();

		firstTime = false;										
	}															
}
//
// ----------------------------------------------------------------------------------------
//	Handles chicken console command
//
void Chicken_Command(edict_t *ent)
{
	if (!deathmatch->value)
		gi.cprintf (ent, PRINT_HIGH, "Catch The Chicken only playable in deathmatch\n");
	else if (!menuAllowed)
		gi.cprintf (ent, PRINT_HIGH, "Menu access not allowed\n");
	else
	{
		if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
			ent->client->displayMenu = true;// Bring up the players menu
		else
			gi.cprintf (ent, PRINT_HIGH, "Catch The Chicken must be started with teams\n");
	}
}
//
// ----------------------------------------------------------------------------------------
//	Checks if player can have powerups when holding chicken
//
qboolean Chicken_AllowPowerUp(edict_t *ent, edict_t *other)
{
	if (chickenGame && other->client && other->client->pers.inventory[chickenItemIndex] > 0 && !other->client->isThrowing)
	{
		if (!allowInvulnerable && !strcmp(ent->item->classname, "item_invulnerability"))
			return false;

		if (!strcmp(ent->item->classname, "item_quad"))
			return false;
	}
	return true;
}
//
// ----------------------------------------------------------------------------------------
//	Reads in chicken configuration at startup
//
static void Chicken_ReadIni()
{
	FILE			*f;
	cvar_t			*game_dir;
	char			filename[128],		buffer[256];
	char			*token	  = NULL,	*valueStr = NULL;
	int				iniOption = 0,		processed = 0,		line = 0;
	static qboolean	alreadyRead = false;

	if (alreadyRead) return;

	game_dir = gi.cvar ("game", "", 0);
/* Change from unix to UNIX and _WIN32 ifdef add by 
 * Chris Richards (cjr@tivoli.com)
 */
#ifdef _WIN32
    sprintf(filename, ".\\%s\\%s", game_dir->string, INI_FILE);
#else
    sprintf(filename, "./%s/%s", game_dir->string, INI_FILE); 
#endif

	if ((f = fopen (filename, "r")) == NULL)
	{
		gi.dprintf("Unable to read %s. Using defaults.\n", INI_FILE);
		return;
	}

	gi.dprintf("\nProcessing CTC %s.. \n", INI_FILE);

	while (fgets(buffer, sizeof(buffer), f) != NULL)
	{
		line++;

		// Get rid of comments
		if (buffer[0] != '\t' && buffer[0] != ' ' && buffer[0] != '\n' && buffer[0] != '#' && buffer[0] != '[')
		{
			token		= strtok(buffer, " \t\n");
			iniOption	= 0;

			while (iniOption < MAX_OPTIONS)
			{
				if (!strcmp(token, option[iniOption].ident))
				{
					char	*ptr = valueStr = strtok(NULL, " \t\n#");
					qboolean bad = false;

					while (ptr != NULL && *ptr)
					{
						if (!isdigit(*ptr))
						{
							bad = true;
							break;
						}
						ptr++;
					}

					if (bad)
						gi.error("Invalid option (%s) in %s on line %d\n", valueStr, token, line);
					else
					{
						if (!strcmp(token, "stdlog"))
							gi.cvar_set("stdlogfile", valueStr);
						else
							*option[iniOption].variable = atoi(valueStr);
					}
					processed++;
					break;
				}
				iniOption++;
			}
		}
	}

	if (feathers > MAX_FEATHER_FRAMES)
		feathers = MAX_FEATHER_FRAMES;

	if (feathers < MIN_FEATHER_FRAMES)
		feathers = MIN_FEATHER_FRAMES;


	// Make sure kickback is reasonable
	if (kickback < 0 || kickback > 20)
	{
		gi.dprintf("kickback of %d is invalid using default\n", kickback);
		kickback = 5;
	}

	// Force certain game options if teams selected
	if (teams) 
	{
		chickenGame = 1;
		dropDelay   = 0;
		canDrop		= true;
	}
	
	gi.dprintf("%d CTC Options processed\n", processed);
	fclose (f);
	alreadyRead = true;	
}
//
// ----------------------------------------------------------------------------------------
//	Reads in map cycling list
//
static void Chicken_ReadMapList()
{
	FILE			*f;
	cvar_t			*game_dir;
	char			filename[128],		buffer[256];
	char			*token	  = NULL,	*valueStr = NULL;
	int				iniOption = 0,		processed = 0,		line = 0;
	qboolean		readIt = true;
	struct stat		buf;   
	static unsigned long lastModTime = 0;

	game_dir = gi.cvar ("game", "", 0);
#ifdef UNIX
	sprintf(filename, "./%s/%s", game_dir->string, MAPLIST_FILE); 
#else
	sprintf(filename, ".\\%s\\%s", game_dir->string, MAPLIST_FILE);
#endif

	if (stat( filename, &buf ) != 0)
	{
		gi.dprintf("Unable to read %s. Using defaults.\n", MAPLIST_FILE);
		readIt = false;
	}
	else   
	{
		if (buf.st_mtime != lastModTime)
			lastModTime = buf.st_mtime;
		else
			readIt = false;
	}

	if (!readIt)
		return;

	if ((f = fopen (filename, "r")) == NULL)
	{
		gi.dprintf("Unable to read %s. Using defaults.\n", MAPLIST_FILE);
		return;
	}

	// Clear out old list first
	if (levelTail)
	{
		levelList		= levelTail->next;
		levelTail->next = NULL;
	}
	else
		levelList		= NULL;

	while (levelList != NULL)
	{
		LevelName *tmp = levelList->next;
		gi.TagFree(levelList);
		levelList = tmp;
	}
	levelTail = NULL;


	gi.dprintf("Reading map list %s.. \n", MAPLIST_FILE);

	while (fgets(buffer, sizeof(buffer), f) != NULL)
	{
		line++;

		// Get rid of comments
		if (buffer[0] != '\t' && buffer[0] != ' ' && buffer[0] != '\n' && buffer[0] != '#' && buffer[0] != '[')
		{
			LevelName	*newLevel = NULL;

			token		= strtok(buffer, " \t\n");

			if ((newLevel			= (LevelName *)gi.TagMalloc(sizeof(*newLevel),	TAG_GAME)) == NULL ||
				(newLevel->mapName	= (char      *)gi.TagMalloc(strlen(token)+1,	TAG_GAME)) == NULL)
				exit(-1);
			else
			{
				strcpy(newLevel->mapName, token);

				if (levelList == NULL)
					levelTail = levelList = newLevel->next = newLevel;
				else
				{
					newLevel->next  = levelTail->next;
					levelTail->next = newLevel;
					levelTail		= newLevel;
				}
			}

			processed++;
		}
	}
	
	gi.dprintf("%d maps listed\n", processed);
	fclose (f);
}
//
// ----------------------------------------------------------------------------------------
//	Message of the Day
//
static void Chicken_MOTDLoad()
{
	FILE			*f;
	cvar_t			*game_dir;
	char			filename[128],		buffer[256];
	char			*token	  = NULL,	*valueStr = NULL;
	int				iniOption = 0,		processed = 0,		line = 0;
	qboolean		readIt = true;
	struct stat		buf;   
	static unsigned long lastModTime = 0;

	game_dir = gi.cvar ("game", "", 0);
#ifdef UNIX
	sprintf(filename, "./%s/%s", game_dir->string, MOD_FILE); 
#else
	sprintf(filename, ".\\%s\\%s", game_dir->string, MOD_FILE);
#endif

	if (stat( filename, &buf ) != 0)
		readIt = false;
	else   
	{
		if (buf.st_mtime != lastModTime)
			lastModTime = buf.st_mtime;
		else
			readIt = false;
	}

	if (!readIt)
		return;

	if ((f = fopen (filename, "r")) == NULL)
		return;

	gi.dprintf("\nReading New MOTD %s.. \n", MOD_FILE);
	motd1[0] = '\0';
	motd2[0] = '\0';
	motd3[0] = '\0';
	motd4[0] = '\0';

	if (fgets(buffer, sizeof(buffer), f) != NULL)
	{
		if ((token = strchr( buffer, '\n' )) != NULL)
			*token = '\0';
		sprintf(motd1, "%-30.30s", buffer);
	}
	if (fgets(buffer, sizeof(buffer), f) != NULL)
	{
		if ((token = strchr( buffer, '\n' )) != NULL)
			*token = '\0';
		sprintf(motd2, "%-30.30s", buffer);
	}
	if (fgets(buffer, sizeof(buffer), f) != NULL)
	{
		if ((token = strchr( buffer, '\n' )) != NULL)
			*token = '\0';
		sprintf(motd3, "%-30.30s", buffer);
	}
	if (fgets(buffer, sizeof(buffer), f) != NULL)
	{
		if ((token = strchr( buffer, '\n' )) != NULL)
			*token = '\0';
		sprintf(motd4, "%-30.30s", buffer);
	}
	fclose (f);
}
//
// ----------------------------------------------------------------------------------------
//	Select Chicken Player for camera view
//
edict_t *Chicken_Follow(edict_t *ent)
{
	edict_t *follow = NULL;

	if (chickenGame)
	{
		edict_t *e	  = NULL;
		int i;

		for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
		{
			if (!e->inuse)
				continue;

			if ((e->client && e->client->pers.inventory[chickenItemIndex] > 0 && !e->client->isThrowing) || !strcmp(e->classname, "item_chicken"))
				follow = e;
		}
	}
	return (follow);
}
//
// ----------------------------------------------------------------------------------------
//	Fallback function to guarentee that chicken exists somewhere
//
static void Chicken_EnsureExists()
{
	static long checkNext = 0;
	
	if (deathmatch->value && chickenGame && checkNext < level.time)
	{
		checkNext = level.time + 10;
		if (chickenGame)
		{
			edict_t		*e  = NULL;
			int	i, foundIt	= false;
			int			contents;

			clientCount = 0;
			// Ensure chicken exists
			for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
			{
				if (!e->inuse)
					continue;

				// Keep a client count so for dedicated servers we dont display stuff to logs
				if (e->client) clientCount++;

				if ((e->client && e->client->pers.inventory[chickenItemIndex] > 0 && !e->client->isThrowing))
					foundIt = true;

				// Check for item chicken not in world or water	
				if (!strcmp(e->classname, "item_chicken"))
				{
					contents = gi.pointcontents (e->s.origin);
					if (!(contents & (CONTENTS_SOLID|CONTENTS_WINDOW)))
						foundIt = true;
				}
	
				// Just ensure that people can score
				if (teams && e->client)
				{
					if (e->client->pers.inventory[chickenItemIndex] == 0 && e->client->holdScore)
					{
						gi.cprintf(e, PRINT_HIGH, "Sorry you couldnt score. Try now\n");
						e->client->holdScore = 0;
					}
				}
			}

			if (!foundIt)
			{
				Chicken_Spawn();
				gi.bprintf(PRINT_HIGH, "Chicken had lost its way and was respawned\n"); 
			}
		}
	}
}
//
// ----------------------------------------------------------------------------------------
//	Display Chicken MOD Message
//
void Chicken_MOTD(edict_t *ent)
{
	char bigString[1024], header[1024];
	int  x = 0;

	if (ent->client && MOTDDuration && ent->client->nextInfoUpdate < level.time &&
		 (strlen(motd1)!= 0 || 
		  strlen(motd2)!= 0 || 
		  strlen(motd3)!= 0 || 
		  strlen(motd4)!= 0 ))
	{
		sprintf(header, "\x01");
		for (x = 0; x < 32; x++)
			strcat(header, "\x02");
		strcat(header, "\x03\n");
		sprintf(bigString,   "%s\x06     CATCH the CHICKEN %s     \x04\n"
							 "\x06                                \x04\n"
							 "\x06 %-30.30s \x04\n"
							 "\x06 %-30.30s \x04\n"
							 "\x06 %-30.30s \x04\n"
							 "\x06 %-30.30s \x04\n"
							 "\x06                                \x04\n",
							    header, CHICKEN_STRING_MENU_VERSION,
								motd1, motd2, motd3, motd4 );

		strcat(bigString, "\x07");
		for (x = 0; x < 32; x++)
			strcat(bigString, "\x08");
		strcat(bigString, "\x09");

		gi.centerprintf(ent, bigString);
		ent->client->nextInfoUpdate		= level.time + 2;
	}
}
//
// ----------------------------------------------------------------------------------------
//	Display Chicken Banner
//
void Chicken_Banner(edict_t *ent)
{
	char buf[128], buf2[128], bigString[1024], header[1024];
	int  x = 0;

	if (ent->client && ent->client->nextInfoUpdate < level.time)
	{
		if (canDrop)
			sprintf(buf, "Chicken droppable after %-3d ", dropDelay);
		else
			sprintf(buf, "Chicken is not droppable    ");

		if (autoRespawn)
			sprintf(buf2, "Chicken respawns after %-3d  ", autorespawntime);
		else
			sprintf(buf2, "Chicken does not respawn    ");

		sprintf(header, "\x01");
		for (x = 0; x < 32; x++)
			strcat(header, "\x02");
		strcat(header, "\x03\n");
		sprintf(bigString,   "%s\x06     CATCH the CHICKEN %s     \x04\n"
							 "\x06                                \x04\n"
							 "\x06 Game Settings                  \x04\n"
							 "\x06                                \x04\n"
							 "\x06 - Chicken score period is %3d  \x04\n"
							 "\x06 - %s \x04\n"
							 "\x06 - %s \x04\n"
							 "\x06 - Chicken respawns %s  \x04\n"
							 "\x06 - Chicken player %s glow%s \x04\n"
							 "\x06 - Menu access %s allowed%s   \x04\n"
							 "\x06 - Camera mode %s allowed%s   \x04\n"
							 "\x06 - Observer mode %s allowed%s \x04\n"
							 "\x06 - Players %s score on death%s  \x04\n" 
							 "\x06                                \x04\n",
							    header, CHICKEN_STRING_MENU_VERSION, scorePeriod, buf, buf2, 
								(randomSpawn?"anywhere  ":"fartherest"),
								(allowGlow?"does":"does not"),  (allowGlow      ?"    ":""),
								(menuAllowed    ?"is":"is not"),(menuAllowed    ?"    ":""),
								(cameraAllowed  ?"is":"is not"),(cameraAllowed  ?"    ":""), 
								(observerAllowed?"is":"is not"),(observerAllowed?"    ":""),
								(scoreOnDeath?"do":"dont"),     (scoreOnDeath   ?"  ":""));

		strcat(bigString, "\x07");
		for (x = 0; x < 32; x++)
			strcat(bigString, "\x08");
		strcat(bigString, "\x09");

		gi.centerprintf(ent, bigString);
		ent->client->nextInfoUpdate = level.time + 2;
	}
}
//
// ----------------------------------------------------------------------------------------
//	Chicken Teamplay Scoreboard
//
qboolean Chicken_TCTCScoreboard (edict_t *ent, edict_t *killer)
{
	gclient_t	*cl;
	char		string[1024], buf[128];
	int			x,i;
	int			teamCount	= 0, 
				deep		= 0,
				total		= 0,
				allowedDepth= 14,
				currentTeam = 0;
	
	if (chickenGame && teams != 0)
	{
		// Lets not get sloppy
		memset(string,    0, sizeof(string));

		// Get count of how many teams there are
		for (x = 0; x < MAX_TEAMS; x++)
			if (teams & (1 << x)) 
				total++;

		// Set maximum number of names that can be displayed
		if (total > 2) allowedDepth = 7;

		// Go through each team as display list of players
		currentTeam = 0;
		for (x = 0; x < MAX_TEAMS; x++)
		{
			if (teams & (1 << x))
			{
				deep = 0;

				sprintf(buf, "xv %d yv %d string \"%4d\" ", 
								((currentTeam+1)%2 ? LEFT_OFFSET + 93: LEFT_OFFSET + RIGHT_WIDTH + 93),
								(currentTeam < 2 ? TOP_TEXT_OFFSET + 4: TOP_TEXT_OFFSET + DOWN_WIDTH + 4), 
								teamDetails[x].score);
				strcat(string, buf);

				sprintf(buf, "xv %d ", ((currentTeam+1)%2 ? LEFT_OFFSET : LEFT_OFFSET + RIGHT_WIDTH));
				strcat(string, buf);

				for (i=0 ; i<game.maxclients ; i++)
				{
					cl = &game.clients[i];

					if (!((edict_t *)(g_edicts + 1 + i))->inuse)
						continue;

					if (cl->team == x)
					{
						if (deep == allowedDepth-1)
						{
							sprintf(buf, "yv %d string \"%-8.8s\" ",
								(currentTeam < 2 ? TOP_TEXT_OFFSET: TOP_TEXT_OFFSET + DOWN_WIDTH) + PIC_WIDTH + (deep * 9), "More ...");
							strcat(string, buf);
							deep++;
							
						}
						else
						{
							if (cl->pers.inventory[chickenItemIndex] > 0 && !cl->isThrowing)
								sprintf(buf, "yv %d string2 \"%-12.12s %3d\" ",
									(currentTeam < 2 ? TOP_TEXT_OFFSET: TOP_TEXT_OFFSET + DOWN_WIDTH) + PIC_WIDTH + (deep * 9), 
									cl->pers.netname, cl->ping > 999 ? 999 : cl->ping);
							else
								sprintf(buf, "yv %d string \"%-12.12s %3d\" ",
									(currentTeam < 2 ? TOP_TEXT_OFFSET: TOP_TEXT_OFFSET + DOWN_WIDTH) + PIC_WIDTH + (deep * 9), 
									cl->pers.netname, cl->ping > 999 ? 999 : cl->ping);
							strcat(string, buf);
							deep++;
						}
					}

					if (deep >= allowedDepth) break;
				}
				currentTeam++;
			}
		}

		gi.WriteByte (svc_layout);
		gi.WriteString (string);

		return true;
	}
	
	return (false);
}

//
// ----------------------------------------------------------------------------------------
//
//	The following function have been created to minimize the changes to the original code
//  Otherwise i would have left them there
//
//
qboolean Chicken_SelectNextItem(edict_t *ent)
{
	if (deathmatch->value && ent->client && Chicken_ShowMenu(ent))
	{
		Chicken_MenuItemNext(ent);
		return false;
	}

	if (ent->client && ent->client->pers.inventory[eggGunItemIndex] > 0)
		return false;

	return true;
}

qboolean Chicken_SelectPrevItem(edict_t *ent)
{
	if (deathmatch->value && ent->client && Chicken_ShowMenu(ent))
	{
		Chicken_MenuItemPrev(ent);
		return false;
	}

	if (ent->client && ent->client->pers.inventory[eggGunItemIndex] > 0)
		return false;

	return true;
}

qboolean Chicken_Cheat(edict_t *ent)
{
	if (deathmatch->value && chickenGame && ent->client && ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing)
	{
		gi.cprintf(ent, PRINT_HIGH, "You cannot cheat while holding the chicken\n");
		return false;
	}
	return true;
}

qboolean Chicken_InvUse(edict_t *ent)
{
	if (deathmatch->value && ent->client && Chicken_ShowMenu(ent))
	{
		Chicken_MenuItemSelect(ent, ent->client->currentItem->itemId);
		return false;
	}
	return true;
}

void Chicken_Kill(edict_t *ent)
{
	// Set to DEAD_DYING so that effects will be turned off
	ent->solid		= SOLID_BBOX;
	ent->svflags	&= ~SVF_NOCLIENT;
	ent->deadflag	= DEAD_DYING;
	ent->s.skinnum	= ent - g_edicts - 1;
	ent->s.effects  = 0;
}

qboolean Chicken_ItemTouch(edict_t *ent, edict_t *other)
{
	if (!strcmp(ent->classname, "item_chicken"))
	{
		gi.sound(other, CHAN_ITEM + CHAN_NO_PHS_ADD, gi.soundindex(SOUND_CHICKEN_PICKUP), 1, ATTN_NONE, 0);	
		G_FreeEdict (ent);
		return true;
	}
	return false;
}

qboolean Chicken_DropMakeTouchable(edict_t *ent)
{
	if (!strcmp(ent->classname, "item_chicken"))
	{	
		ent->nextthink	= level.time + FRAMETIME;
		ent->think		= Chicken_Think;
		return true;
	}	
	return false;
}

qboolean Chicken_CanPickup(edict_t *ent, int allow)
{
	if (deathmatch->value && chickenGame && ent->client && ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing && !allow)
		return false;

	return true;
}

void Chicken_RunFrameEnd(edict_t *ent)
{
	Chicken_EnsureExists();
	if (chickenGame && teams == 0 && ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
	{
		Chicken_EndIt(ent);
		gi.bprintf(PRINT_HIGH, "Catch the Chicken must be started with teamplay\n");
	}
}

qboolean Chicken_Ready(edict_t *ent)
{
	if (ent->client->pers.inventory[chickenItemIndex] > 0)
	{
		ent->client->newweapon = chickenItem;
		return true;
	}

	if (ent->client->pers.inventory[eggGunItemIndex] > 0)
	{
		ent->client->newweapon = eggGunItem;
		return true;
	}

	return false;
}

qboolean Chicken_InvDrop(edict_t *ent)
{
	if (deathmatch->value && ent->client && ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing)
	{
		chickenItem->drop (ent, chickenItem);
		return true;
	}

	if (ent->client && ent->client->pers.inventory[eggGunItemIndex] > 0)
		return true;

	return false;
}

qboolean Chicken_TossCheck(edict_t *ent)
{
	// If you have the chicken but have already throw it dont drop another
	if (deathmatch->value && chickenGame && ent->client && ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing)
	{
		if (!ent->client->isThrowing)
			Chicken_Toss(ent,	chickenItem);

		return true;
	}
	return false;
}

void Chicken_CheckGlow(edict_t *ent)
{
	if (deathmatch->value && chickenGame && allowGlow && ent->client->pers.inventory[chickenItemIndex] > 0 && !ent->client->isThrowing)  
	{
		switch(allowGlow)
		{
			case 2:	
				ent->s.effects |= EF_FLAG1;
				break;

			case 3:	
				ent->s.effects |= EF_FLAG2;
				break;

			case 4:	
				ent->s.effects |= (EF_FLAG2 | EF_FLAG1);
				break;

			default:
				ent->s.effects |= EF_HYPERBLASTER;												
		}
	}
}

qboolean Chicken_CanPickupHealth(edict_t *ent, edict_t *other)
{
	if (ent->count == 2)
	{
		if (!Chicken_CanPickup(other, allowSmallHealth)) 
			return false;
	}
	else
		if (!Chicken_CanPickup(other, allowBigHealth)) 
			return false; 

	return true;
}

void Chicken_NewPlayer(edict_t *ent)
{
	ent->client->gotChickenModel	= -1;
}

void Chicken_ChangeMap(edict_t *ent)
{
	if (deathmatch->value && chickenGame && levelCycling)  
	{
		Chicken_ReadMapList();
		Chicken_MOTDLoad();

		if (levelList != NULL)
		{
			levelList = levelList->next;
			ent->map  = levelList->mapName;
		}
	}
}
