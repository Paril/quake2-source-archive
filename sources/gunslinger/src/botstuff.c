#include "g_local.h"
/*
Hi, since you are looking at this piece of code I assume you would
like to remove bots from Q2 as much as I.

I'll give a basic idea on how this code is supposed to work: the
"anti bot" (i call Fluffy) is spawned when the level starts ..(or
after countdown) then it disconnects and respawns at random times
in random locations.  What a auto aimer will do is fire on the anti
bot "fluffy" and when hit We add 1 to the (clients.pers hitantibot)
with 5 hits he is detected and dealt with.  Im my testing I also
found that win32 proxy bots (RAT) cannot handle a "reconnect".  (a
reconnect is required due to the model that a player "MUST download.
(your thinking great,forcing ppl to download a bunch of models.
Well, the sysop can also set allowed player models and a server will
only send models that it has onhand..(not an issue)  The "antibot"
Fluffy is a client in every way the same as any other player .(except
his model and skin are almost invisible) so your real players dont
see him and attack.(and with the check_model function no one can
use Fluffy as a model and be invisible)

I have noted to which player flags the auto aimers look for in a
target in my code.(not all auto aimers use same player flags:example?
ok Rat needs a player with (ent->svflags &= ~SVF_DEADMONSTER) but
zbot didnt seem to care zbot fired at it Rat didn't!.

The hardest thing for me to do was to ensure that Fluffy never got any
messages(would overflow the server).  so we redirected (all the print
functions).

As you know all mods are differant and i seriously doubt this will
"drop in cut-n paste" to your mod.  but I assure you that it works
perfectly in my mod.MatchMod 2.0.

credits:
Maj Bitch, Rivera, Warzone, Mr Exclusive(glad bot), GiZZed (Fluffy model), Rhea(OSP)
and thanks to all the ppl that scrounged up all the auto aimers for us to test it on .

regards
RaVeN

***********************************************
rav_open opens a file with correct path ..
(not really bot detection but usefull)
**********************************************/

FILE *rav_open (const char *filename, const char *t)
{
	FILE *fd;
	char dir[260];

	strcpy (dir, gamename->string);
	if (Q_stricmp (dir, "\0")==0) strcpy (dir, "baseq2");
#ifdef _WIN32
	strcat (dir, "\\");
#else
	strcat (dir, "/");
#endif
	strcat (dir, filename);
	fd=fopen (dir, t);
	return (fd);
}
/***************************************************

  ADD ENTRY to a txt file (my quickie write to a file
  (appends to existing file)
***************************************************/

void addEntry (char *filename, char ip[20])
{
	FILE *ipfile;

	// First, check to see that client isn't already in the file
//	if (entryInFile (filename, ip)) return;

	// add user to file
	strcat (ip, "\n");

	if (ipfile=rav_open(filename, "a"))
	{
		fputs(ip, ipfile);
		fclose (ipfile);
	}
	return;
}

/**********************************************
new bot detection
***********************************************/

// Basic Bot detection functions (cvars and start)

/* set sv_botdetection <value> 31 is all features!
1 Log bot detection to a file
2 Kick detected bot
4 Notify the other players of who is using a bot
8 Include impulses as a detection method
16Bans user name and /or ip.
*/

void BotDetection(edict_t *ent, usercmd_t *ucmd)
{
	static gclient_t *cl;

//	debugmsg("Func: BotDetection\n");

	cl = ent->client;

	if (cl->resp.is_bot)
		return;

	if (ucmd->impulse) {
		if ((int)sv_botdetection->value & BOT_IMPULSE) {
			OnBotDetection(ent, va("impulse %d", ucmd->impulse));
			return;
		}
	}
}

void ClientDisconnect (edict_t *ent);

void OnBotDetection(edict_t *ent, char *msg)
{
	char user[32];
	char userip[80];
	int i, j;
	int pkt_type[] = { 0x01, 0x02, 0x03, 0x09, 0x0c, 0x0e, 0x11, 0x12, 0x14 };

//	debugmsg("Func: OnBotDetection\n");

	sprintf(user, "%s@%s", ent->client->pers.netname, Info_ValueForKey (ent->client->pers.userinfo, "ip"));
	sprintf(userip, "*@%s", Info_ValueForKey (ent->client->pers.userinfo, "ip"));

	ent->client->resp.is_bot = 1;

	if ((int)sv_botdetection->value & BOT_LOG){
		addEntry ("cfg/bot_detected.txt", user);
	}

	if ((int)sv_botdetection->value & BOT_NOTIFY) {
		gi_bprintf(PRINT_HIGH, "%s was Busted By -=BOtCRuSher=-\n",
			user);
	}

	if((int)sv_botdetection->value & BOT_BAN){
		addEntry ("cfg/ip_banned.txt", userip);
	}


	if ((int)sv_botdetection->value & BOT_KICK) {
		ent->movetype = MOVETYPE_NOCLIP;
		// Send a legitimate disconnect
		gi.WriteByte(svc_disconnect);
		gi.unicast(ent, true);

		// Send an illegible message type as well for "smart" proxies.(rhea)
		i = rand() % 9;
		gi.WriteByte(pkt_type[i]);

		j = rand() % 3;
		for(i=0; i<j; i++)
			gi.WriteByte((rand() % 256));
		gi.unicast(ent, true);
		ClientDisconnect(ent);
	}


}

/**********************************
CheckModel
**********************************/
//we will be using an invisible model
//as a target but wouldnt want clients trying it at home
//this will prevent any goofy models messing with our bot protection.
//this also lets the sysop set what models "HE wants on his server"


char *CheckModel (char intext[64])
{
	int i = 0;
	char *pos, *pos2;
	char Model[64];
	static char Default[32];
	char checkModel[128], first[32];
	qboolean end=false;

//	debugmsg("Func: CheckModel\n");
//	debugmsg("Skin: %s\n", intext);

	if (!Q_strcasecmp(intext, "fluffy/skin")) {
//		debugmsg("Returning male/grunt\n");
		return "male/grunt";
	}
	if (strlen (allowed_models->string) == 0) {
//		debugmsg("Returning %s\n", intext);
		return (intext);
	}

	strcpy (Default, "male/grunt");
	strcpy (Model, "\0");

	while (!strchr("/", intext[i]))
	{
		Model[i]=intext[i];
		i++;
	}
	Model[i]='\0';

	strcpy (checkModel, allowed_models->string);

	pos = pos2 = checkModel;

	while (!end)
	{
		pos2 = strchr (pos+1,',');
		if (!pos2) {
			pos2=checkModel+strlen(checkModel);
			end=true;
		}
		strncpy (first, pos, (int)(pos2-pos));
		first[pos2-pos]='\0';
//		debugmsg("Comparing %s to %s\n", Model, first);

		if (!Q_stricmp(Model, first))
		{
//			debugmsg("Returning %s\n", intext);
			return (intext);
		}

		pos=pos2+1;
	}
//	debugmsg("Returning %s\n", Default);
	return (Default);
}

//======================================================
//========== Spawnpoint for bot ==========
//======================================================

//1st choice is a random spawning any where on the map
//2nd choice for our"bot" is the intermission spots
//most maps have them ..if not then we look for client spots
// and spawn close to it !!
// is cant span then go to ammo_shells spot
//if not avaliable then crash the game..(crappy map)
void ED_CallSpawn (edict_t *ent);

edict_t *SelectBotSpawnPoint (edict_t *ent)
{

	edict_t *spot = NULL;
	char *target;

//	debugmsg("Func: SelectBotSpawnPoint\n");

	spot = G_Find (NULL, FOFS(classname), "info_player_intermission");

	if (!spot)
	{
		spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
		spot = G_Spawn ();
		spot->classname = "info_player_intermission";
		spot->spawnflags = 0;
		VectorCopy (spot->s.origin, spot->s.origin);
		ED_CallSpawn (spot);
	}

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_intermission");
		if (!spot)
			return NULL; // we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{ // this is a coop spawn point for one of the clients here
			return spot; // this is it
		}
	}


	return spot;
}

void BotSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t *spot = NULL;

//	debugmsg("Func: BotSpawnPoint\n");

	spot = SelectBotSpawnPoint (ent);

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
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
			{ // there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "ammo_shells");
			}
			if (!spot)
				gi.error ("Couldn't spawn -=BotCRusher=- due to crappy map");
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}
/**************************************
some Bitch Bot functions )
**************************************/

//======================================================
// Returns a Random 'Valid' IP Address (Thanks Riviera!)
//======================================================
char *Random_IP(void) {
	static char ipstr[16];
	int ip1, ip2, ip3, ip4;

//	debugmsg("Func: Random_IP\n");
	do {
		ip1 = rand()%128 + 128;
	} while (ip1 == 192 || ip1 == 172);

	ip2 = rand()%256;
	ip3 = rand()%256;
	ip4 = rand()%256;

	sprintf(ipstr,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);

	return ipstr;
}

//=======================================================
// Generate the bots userinfo array (same as logging on!)
//=======================================================
void Random_Userinfo(edict_t *ent, char *userinfo) {
	char handstr[2];
	static char namestr[30];
	static char skin[256];

//	debugmsg("Func: Random_Userinfo\n");
	memset(userinfo, 0, sizeof(userinfo)); // Clear userinfo

	sprintf(handstr,"%d", rand()&2);
	sprintf(namestr,"Bot CRuSHer"); //playername
	strcpy(skin, "fluffy/skin"); //model skin combo

	// Set the pertinent user information..
	Info_SetValueForKey(userinfo, "name", namestr);
	Info_SetValueForKey(userinfo, "ip", Random_IP());
	Info_SetValueForKey(userinfo, "skin", skin);
	Info_SetValueForKey(userinfo, "hand", handstr);
	Info_SetValueForKey(userinfo, "fov", "90");
}

//=========================================================
// Returns the next available edict_t record.
// outside of maxcients
//this puts our bot above max clients
//(there are ramifications that need to be dealt with.
//we will in a bit
//=========================================================
int G_GetFreeEdict(void)
{
	edict_t *ent;
	int i;
	// Get available edict record outside of maxcients
//	debugmsg("Func: G_GetFreeEdict\n");
	i = maxclients->value;
	ent=g_edicts+i+1;
	return i;
}

/***************************
BOT SAFE PRINTING FUNCTIONS (philip) majBitch
redirection code to avoid overflowing server
****************************/
//===================================================*/
void gi_cprintf(edict_t *ent, int printlevel, char *fmt, ...) {
	char bigbuffer[0x10000];
	va_list argptr;
	int len;

	if (ent && (ent->flags & FL_ANTIBOT))
		return;

	va_start(argptr,fmt);
	len = vsprintf(bigbuffer,fmt,argptr);
	va_end(argptr);

	gi.cprintf(ent, printlevel, bigbuffer);
}

//===================================================
void gi_centerprintf(edict_t *ent, char *fmt, ...)
{

	char bigbuffer[0x10000];

	va_list argptr;

	int len;


	if (ent->flags & FL_ANTIBOT)
		return;

	va_start(argptr,fmt);
	len = vsprintf(bigbuffer,fmt,argptr);
	va_end(argptr);

	gi.centerprintf(ent, bigbuffer);
}

//===================================================
void gi_bprintf(int printlevel, char *fmt, ...)
{
	int		i;
	char	bigbuffer[0x10000];
	int		len;
	va_list	argptr;
	edict_t	*cl_ent;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	if (dedicated->value)
		gi.cprintf(NULL, printlevel, bigbuffer);

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || cl_ent->flags & FL_ANTIBOT)
			continue;

		gi.cprintf(cl_ent, printlevel, bigbuffer);
	}
}


/*****************************************
Actual bot handling code
*****************************************/

// Move anti BOt
void MoveBot()
{
	edict_t *ent;
	int i;

	if (move_bot_time > level.time)
		return;

//	debugmsg("Func: MoveBot\n");

	for_each_player(ent, i)
	{
		if (ent->flags & FL_ANTIBOT)
			BotDisconnect(ent);
	}
	move_bot_time = level.time + rndnum(45,75);

}
// removes bot from game at intermission and at
// move bot times!!

void BotDisconnect (edict_t *ent)
{
	int playernum;

//	debugmsg("Func: BotDisconnect\n");

	if (!ent->client)
		return;

	if (level.intermissiontime)
		return;

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");

	antibot = false;

//	if (match_state == STATE_PLAYING)
		ravenspawnantibot();

}

// this lets us respawn our little toy
// at totally random locations(warzone)

qboolean findspawnpoint (edict_t *ent)
{
	vec3_t loc = {0,0,0};
	vec3_t floor;
	int i;
	int j = 0;
	int k = 0;
	int jmax = 1000;
	int kmax = (int)(jmax/2);
	qboolean found = false;

	trace_t tr;
//	debugmsg("Func: findspawnpoint\n");

	do {
		if (found || j >= jmax || k >= kmax)
			break;
		j++;
//		debugmsg("J: %i\n", j);
		for (i = 0; i < 3; i++) {
			loc[i] = rand() % (4096 + 1) - 2048;
//			debugmsg("loc[%i]: %.0f\n", i, loc[i]);
		}

		if (gi.pointcontents(loc) == 0)
		{
//			debugmsg("Pointcontents 0\n");
			VectorCopy(loc, floor);
			floor[2] = -4096;
			tr = gi.trace (loc, ent->maxs, ent->mins, floor, NULL, MASK_SOLID|MASK_WATER);
			k++;
//			debugmsg("K: %i\n", k);
			if (tr.contents & MASK_WATER)
				found = false;
			else
			{
				VectorCopy (tr.endpos, loc);
				loc[2] += ent->maxs[2] - ent->mins[2]; // make sure the entity can fit!
				found = (gi.pointcontents(loc) == 0 ? true : false);
			}
		}
		else {
//			debugmsg("Not found\n");
			found = false;
		}
	} while (!found && j < jmax && k < kmax);


	if (!found && (j >= jmax || k >= kmax))
		return false;

	VectorCopy(loc,ent->s.origin);
	VectorCopy(loc,ent->s.old_origin);
	return true;
}


//move if we get touched by a player
static void Bot_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	debugmsg("Func: Bot_Touch\n");

	if (other->takedamage)
		BotDisconnect (ent);
}


/******************************************
BOT PIMPS THE MOD FOR US
******************************************/
// this is a neat toy its totally optionial )


void BotGreeting(edict_t *chat)
{
	int i;
	char *message[16];

//	debugmsg("Func: BotGreeting\n");

	for (i = 0; i < 16; i++)
	{
		message[i] = NULL;
	} //end for


	message[0] = "Homepage of MatchMod:http://planetquake.com/matchmod";
	message[1] = "questions or bug reports ?, email: theraven@planetquake.com";
	message[2] = "I am the most accurate bot detection to date!!";
	message[3] = "Welcome to this MatchMod Server";
	message[4] = "Hi, Im Bot Crusher ";
	message[5] = "Hurry over to http://planetquke.com/matchmod and grab the latest map pack's by 'RivrStyX' ";
	message[6] = "Check out the MatchMod launcher by 'GiZZed' ";
	message[7] = "I talk, I watch for Bots, I keep you posted on MatchMod development, Damn I'm Good !! &; >)";

	if (message[0])
	{
		for (i = 0; i < 16; i++)
		{
			if (!message[i]) break;
		} //end for
		i = random() * (float) i;

		gi_bprintf(PRINT_CHAT, "-=BOtCRuSheR=-: %s\n", message[i]);

	}
	bot_next_msg = level.time + rndnum(60,125);
	G_FreeEdict(chat);
}


// spawn into game function
// this was really fun to put together

edict_t *GQ_FindRandomSpawn(void);

void PutBotDetectInServer (edict_t *ent)
{
	int index;
	vec3_t spawn_origin, spawn_angles;
//	vec3_t dir;
	edict_t *chat;
//	trace_t tr;

	// 8 is the smallest BBOx i could use, it is very unlikley that
	// a random shot will touch it .

	vec3_t mins = { -8, -8, -8};
	vec3_t maxs = {8, 8, 8};

	gclient_t *client;
	int i;

//	debugmsg("Func: PutBotDetectInServer\n");

	if (level.intermissiontime)
		return;

	if(antibot)
		return;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);

//	debugmsg("Searching for spawn point\n");
/*	chat=GQ_FindRandomSpawn();
	VectorCopy(chat->s.origin, ent->s.origin);
	ent->s.origin[2]+=64;
	VectorCopy(ent->s.origin, spawn_origin);
	VectorCopy (ent->s.origin, ent->s.old_origin);
	VectorSet(dir, 500-(rand()%1000), 500-(rand()%1000), (rand()%100));
	VectorScale(VectorNormalize(dir), 8192, dir);
	
	tr = gi.trace (ent->s.origin, NULL, NULL, dir, ent, MASK_SHOT);
*/
	if (findspawnpoint(ent)) {
		VectorCopy(ent->s.origin, spawn_origin);
		VectorClear (spawn_angles);
		spawn_angles[YAW] = rand() % 360 - 180; // face a random direction
	} else // couldn't find a good spot, so...
		BotSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;
	ent->groundentity = NULL;
	ent->client = &game.clients[index];

	//we need to set the "player flags"

	ent->takedamage = DAMAGE_NO;// so we dont make noise when hit
	ent->movetype = MOVETYPE_TOSS;// doesnt seem to matter
	ent->health = ent->max_health = 100;// n/a
	ent->gravity = 0;//n/a
	ent->viewheight = 22;//n/a
	ent->inuse = true;//must be set true
	ent->classname = "botdetection";//for check should we "lose our bot"
	ent->mass = 1;//n/a
	ent->solid = SOLID_BBOX;//must be set SOLID_BBOX or bots wont attack us
	ent->deadflag = DEAD_NO;//DEAD_NO lets us be a target for bots
	ent->air_finished = level.time + 12;//n/a
	ent->clipmask = MASK_PLAYERSOLID;//n/a
	ent->model = "players/fluffy/tris.md2";//n/a
	ent->s.renderfx |= RF_TRANSLUCENT;// helps hide the skin
//	ent->s.effects = EF_BLUEHYPERBLASTER;
	ent->yaw_speed = 50;// turn at yaw_speed degrees per FRAME
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;
	ent->svflags &= ~SVF_NOCLIENT;//so auto aimers will see us
	ent->touch = Bot_Touch;
	ent->client->pers.connected = true;
	ent->client->team = 5;
//	ent->client->pers.team = 3;// so team skins wont effect us
//	ent->client->resp.ctf_team = 3;
	ent->client->idon=false;

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

	ent->s.frame = 0;

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

	//set the flag so we only spawn one bot
	antibot = true;

	ent->client = &game.clients[ent-g_edicts-1];

	VectorCopy(spawn_origin, ent->s.origin);
	ent->client->killer_yaw = 0;// chaingun wind-up

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(ent->client->ps));
	ent->client->ps.pmove.gravity = 200;//float it away ?

	ent->client->ps.pmove.origin[0] = ent->s.origin[0]*8;
	ent->client->ps.pmove.origin[1] = ent->s.origin[1]*8;
	ent->client->ps.pmove.origin[2] = ent->s.origin[2]*8;
	ent->client->ps.fov = 90;

	//this sets the model and skin index (has to have this for RAT BOT to shoot!!)
	ent->s.modelindex = 255;
	ent->s.skinnum = ent-g_edicts - 1;

	//optional
	if ((bot_next_msg > level.time)&&(bot_talk->value))
		//in case a sysop would want to turn the talk off
	{
		// spawn the MOD PIMPER!
		chat = G_Spawn();
		chat->owner = ent;
		chat->think = BotGreeting;
		chat->nextthink = level.time + 1.5 + random();
	}

	//not optionial
	gi.linkentity (ent);
}


//=======================================================
// Create the botdetector as a real player-client.
//=======================================================
void ClientUserinfoChanged (edict_t *ent, char *userinfo);

void ravenspawnantibot (void)
{
	edict_t *bot;
	char userinfo[MAX_INFO_STRING];
	int clientnum;

//	debugmsg("Func: ClientUserinfoChanged\n");

	if (antibot) //only need one
		return;

	//lets get some userinfo
	Random_Userinfo(NULL, userinfo);

	clientnum = G_GetFreeEdict();

	bot=g_edicts+clientnum+1;

	G_InitEdict(bot);

	bot->client=&game.clients[clientnum];

	//for Matchmod (client is an avaliable target) ctf/tp and dm.
//	bot->client->pers.in_game = true;
	bot->client->pers.spectator = false;
	// end MM

	bot->client->ping = 200; // some bots look for "ping value"
	bot->flags |= FL_ANTIBOT;

	InitClientResp(bot->client);
	InitClientPersistant(bot->client);
	ClientUserinfoChanged(bot, userinfo);
	PutBotDetectInServer(bot);
	ClientEndServerFrame(bot);
}
//END
