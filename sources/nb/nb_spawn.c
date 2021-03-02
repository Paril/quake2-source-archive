/*========================================================================
	Neuralbot v0.6						23/November/99
	--------------
		
Neuralbot is made by Nicholas Chapman (c)1999

Feel free to slice and dice this code as you see fit.
	If you want to use any of the code for any of your projects, feel free.
If you release this project, drop me a line as I would like to see what 
this code has been used for.
	If you wish to use any of this code in a commercial release, you must gain 
my written consent first.

The area of bot AI is large, and way more experimentation needs to be done
with neural-networks and g.a.s in bots than I could ever do.  So I encourage 
you all to use this code as a base or inspiration or whatever and see if you 
can squeeze some learning out of a bot.

  Nick Chapman/Ono-Sendai		nickamy@paradise.net.nz		
								onosendai@botepidemic.com
								icq no# 19054613

==========================================================================*/
#include "g_local.h"

///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End

void Bot_Spawn(edict_t *ent);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void CopyToBodyQue (edict_t *ent);
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);


/*=====================================================================================
Bot_Respawn
-----------
called when a bot spawns back into the game
=====================================================================================*/
void Bot_Respawn(edict_t *ent)
{

	CopyToBodyQue(ent);
		//make a copy of the bots body to lie on the ground


	Bot_Spawn(ent);
		//put bot back in the game with health etc..	

		
	ent->s.event = EV_PLAYER_TELEPORT; //add a teleport effect

	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;


	ent->client->respawn_time = level.time;//BUGFIX: changed this back 


	//ent->client->ps.pmove.teleport_time = 50;

	//initializeExploreinfo(ent, &ent->client->botinfo.exploreinfo); BUGFIX
		//reset explore waypoints 

	
}




/*==========================================================================
Addbot
----------------------------------------------------------------------------
Spawn one bot into the game.  Only called when a bot connects, not when 
they respawn
==========================================================================*/
edict_t* Addbot(int botnum) //Add a bot
{



	int      i;

	char     userinfo[MAX_INFO_STRING];

	edict_t *bot;


	for (i = maxclients->value; i > 0; i--)	//look throught the client type entities
	{
	
		bot = g_edicts + i + 1;

		if (!bot->inuse)	//if found a client type entity that is not in use
			break;
	}



	if (bot->inuse)
		bot = NULL;



	if (bot)
	{
		memset(userinfo, 0, MAX_INFO_STRING);

		switch (botnum)
		{
		case 0: Info_SetValueForKey(userinfo, "name", "Fragasaurus-Rex"); 
				Info_SetValueForKey(userinfo, "skin", "female/voodoo");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 1: Info_SetValueForKey(userinfo, "name", "Sir Fragalot"); 
				Info_SetValueForKey(userinfo, "skin", "male/major");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 2: Info_SetValueForKey(userinfo, "name", "Fragenstein"); 
				Info_SetValueForKey(userinfo, "skin", "male/psycho");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 3: Info_SetValueForKey(userinfo, "name", "Fragzilla"); 
				Info_SetValueForKey(userinfo, "skin", "female/athena");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 4: Info_SetValueForKey(userinfo, "name", "Fragmonster"); 
				Info_SetValueForKey(userinfo, "skin", "male/rampage");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 5: Info_SetValueForKey(userinfo, "name", "snore"); 
				Info_SetValueForKey(userinfo, "skin", "male/razor");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 6: Info_SetValueForKey(userinfo, "name", "sleepy boring"); 
				Info_SetValueForKey(userinfo, "skin", "male/scout");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 7: Info_SetValueForKey(userinfo, "name", "Happy man"); 
				Info_SetValueForKey(userinfo, "skin", "male/sniper");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 8: Info_SetValueForKey(userinfo, "name", "Fragmeister"); 
				Info_SetValueForKey(userinfo, "skin", "male/grunt");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 9: Info_SetValueForKey(userinfo, "name", "Zen of frag"); 
				Info_SetValueForKey(userinfo, "skin", "male/pointman");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 10: Info_SetValueForKey(userinfo, "name", "bot10"); 
				Info_SetValueForKey(userinfo, "skin", "female/ensign");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 11: Info_SetValueForKey(userinfo, "name", "bot11"); 
				Info_SetValueForKey(userinfo, "skin", "female/jungle");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 12: Info_SetValueForKey(userinfo, "name", "bot12"); 
				Info_SetValueForKey(userinfo, "skin", "female/venus");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 13: Info_SetValueForKey(userinfo, "name", "bot13"); 
				Info_SetValueForKey(userinfo, "skin", "female/lotus");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 14: Info_SetValueForKey(userinfo, "name", "bot14"); 
				Info_SetValueForKey(userinfo, "skin", "male/razor");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 15: Info_SetValueForKey(userinfo, "name", "bot15"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 16: Info_SetValueForKey(userinfo, "name", "bot16"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 17: Info_SetValueForKey(userinfo, "name", "bot17"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 18: Info_SetValueForKey(userinfo, "name", "bot18"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 19: Info_SetValueForKey(userinfo, "name", "bot19"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 20: Info_SetValueForKey(userinfo, "name", "bot20"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 21: Info_SetValueForKey(userinfo, "name", "bot21"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 22: Info_SetValueForKey(userinfo, "name", "bot22"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 23: Info_SetValueForKey(userinfo, "name", "bot23"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 24: Info_SetValueForKey(userinfo, "name", "bot24"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 25: Info_SetValueForKey(userinfo, "name", "bot25"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 26: Info_SetValueForKey(userinfo, "name", "bot26"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 27: Info_SetValueForKey(userinfo, "name", "bot27"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 28: Info_SetValueForKey(userinfo, "name", "bot28"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 29: Info_SetValueForKey(userinfo, "name", "bot29"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 30: Info_SetValueForKey(userinfo, "name", "bot30"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 31: Info_SetValueForKey(userinfo, "name", "bot31"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		case 32: Info_SetValueForKey(userinfo, "name", "bot32"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;
		default:  Info_SetValueForKey(userinfo, "name", "bot"); 
				Info_SetValueForKey(userinfo, "skin", "male/recon");
				Info_SetValueForKey(userinfo, "hand", "2");
				break;

		}


		ClientConnect(bot, userinfo);//register the bot as connected

		G_InitEdict(bot);//set gravity, inuse for bot

		InitClientResp(bot->client);



		Bot_Spawn(bot);//place the bot into the level

		if(!bot->client)
		{
			safe_bprintf(PRINT_HIGH, "client is null\n");
			return NULL;
		}


		

		bot->client->botinfo.isbot = true;
		bot->client->botinfo.invisible = false;

		/// GA stuff: ///
		bot->client->botinfo.fragslastperiod=0;
		bot->client->botinfo.itemsgot=0;
		bot->client->botinfo.damageinflicted=0;
		bot->client->botinfo.explorenesssofar=0;
		bot->client->botinfo.structure_fragslastperiod=0;
		bot->client->botinfo.structure_itemsgot=0;
		bot->client->botinfo.structure_damageinflicted=0;
		bot->client->botinfo.structure_explorenesssofar=0;


		bot->client->botinfo.wasblocked = false;


		bot->client->botinfo.isrefbot = false;

		///Q2 Camera Begin
		EntityListAdd(bot);//NOTE: UNCOMMENT
		///Q2 Camera End

	
		



		gi.WriteByte(svc_muzzleflash);

		gi.WriteShort(bot - g_edicts);
		
		gi.WriteByte(MZ_LOGIN);

		gi.multicast(bot->s.origin, MULTICAST_PVS);//some lights, flash for spawning in



		//safe_bprintf(PRINT_HIGH, "%s entered the game\n", bot->client->pers.netname);

		ClientEndServerFrame(bot);


	}
	else
		gi.dprintf("%s cannot connect - server is full!\n", "Neuralbot");

	return bot;
}


/*====================================================================================
Bot_Spawn
---------
called every time a bot is respawned in deathmatch and when it joins a game.
Puts the bot in the game, sets up the model, solidness etc..
====================================================================================*/
void Bot_Spawn(edict_t *ent)

{
	//NOTE: save botinfo?

	vec3_t               origin, angles;

	vec3_t               mins = {-16, -16, -24};

	vec3_t               maxs = {16, 16, 32};

	int                  i, index;

	client_persistant_t  pers;     

	client_respawn_t     resp;

	nb_info				 saved_nb_info;// NEWCODE:Neuralbot info to be saved


	if (!deathmatch->value)
	{

		gi.dprintf("Must be in Deathmatch to spawn a bot!\n");

		return;

	}

	saved_nb_info = ent->client->botinfo;//back up nbinfo, as client will be wiped


	SelectSpawnPoint(ent, origin, angles);
		//get a spawn point
	

	index = ent - g_edicts - 1;



	if (deathmatch->value)
	{

		char userinfo[MAX_INFO_STRING];



		resp = ent->client->resp;

		memcpy(userinfo, ent->client->pers.userinfo, MAX_INFO_STRING);

		InitClientPersistant(ent->client);//wipes client->resp, resets inventory max items, connected to true

		ClientUserinfoChanged(ent, userinfo);//set userinfo back to original

	}
	else
		memset(&resp, 0, sizeof(client_respawn_t));

    

	pers = ent->client->pers;

	memset(ent->client, 0, sizeof(gclient_t));//wipe the entire client structure

	ent->client->pers = pers;	//restore persistant data

	ent->client->resp = resp;	//and respawn data



	FetchClientEntData(ent);	//reset health and max_health 

	

	ent->groundentity     = NULL;

	ent->client           = &game.clients[index];

	ent->takedamage       = DAMAGE_AIM;

	ent->movetype         = MOVETYPE_WALK;

	ent->viewheight       = 22;

	ent->inuse            = true;

	ent->classname        = "player";

	ent->mass             = 200;

	ent->solid            = SOLID_BBOX;

	ent->deadflag         = DEAD_NO;

	ent->air_finished     = level.time + 12;

	ent->clipmask         = MASK_PLAYERSOLID;

	ent->think            = bot_think;

	ent->touch            = NULL;

	ent->pain             = player_pain;

	ent->die              = player_die;

	ent->waterlevel       = 0;

	ent->watertype        = 0;

	ent->flags           &= ~FL_NO_KNOCKBACK;

	ent->enemy            = NULL;

	ent->movetarget       = NULL;



	VectorCopy(mins, ent->mins);

	VectorCopy(maxs, ent->maxs);

	VectorClear(ent->velocity);

	

	memset(&ent->client->ps, 0, sizeof(player_state_t));



	for (i = 0; i < 3; i++)

	{

		ent->client->ps.pmove.origin[i] = origin[i] * 8;

		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(angles[i] - ent->client->resp.cmd_angles[i]);

	}



	ent->client->ps.fov = 90;

	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);



	ent->s.effects        = 0;

	ent->s.skinnum        = index;

	ent->s.modelindex     = 255;

	ent->s.modelindex2    = 255;

	ent->s.frame          = 0;



	VectorCopy(origin, ent->s.origin);

	ent->s.origin[2]++;



	ent->s.angles[PITCH]  = 0;

	ent->s.angles[YAW]    = angles[YAW];

	ent->s.angles[ROLL]   = 0;



	VectorCopy(ent->s.angles, ent->client->ps.viewangles);

	VectorCopy(ent->s.angles, ent->client->v_angle);

	
	
	gi.unlinkentity(ent);

	KillBox(ent);         // Telefrag!

	gi.linkentity(ent);

	

	ent->client->newweapon = ent->client->pers.weapon;

	ChangeWeapon(ent);


	ent->client->botinfo = saved_nb_info; //restore saved nb_info

	ent->nextthink = level.time + FRAMETIME;

	giveFreeWeapon(ent);//give a free weapon + ammo if the free weapon option has been set to anything

	initializeExploreinfo(ent, &ent->client->botinfo.exploreinfo);
	
	
}


