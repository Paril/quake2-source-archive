// wf_config.c
// Functions to read the WF server configuration file (wfserver.ini)

#include "g_local.h"
#include "wf_classmgr.h"

void readline(FILE *file, char *str, int max);
void WFFillMapNames();
void ClearMapVotes();
void lcase(char *str);
int LoadClassInfo(char *filename);
void LoadWeaponInfo() ;

FILE *fpconfig;
int	config_line_no;
unsigned char config_line[201];

#define COMMENT_CHAR	'/'

void rtrim(char *str)
{
	int i;

	i = strlen(str) - 1;

	//now trim back the white space
	while (i >= 0 && str[i] == ' ')
	{
		str[i] = 0;
		--i;
	}
}

//Return true if the given string contains banned words
int BannedWords(edict_t *ent, char *str)
{
	int i;

	//Loop through the banned words
	i = 0;

	while (i < MAX_BANWORDS && wf_game.banwords[i][0] != 0)
	{
		if( strstr( str, wf_game.banwords[i] ) )
		{
			safe_cprintf(ent, PRINT_HIGH, "You can't say that on this server!\n");
			return 1;
		}

		++i;
	}

	return 0;
}

void RemoveComments(char *str)
{
	int i;
	int len;

	i = 0;
	len = strlen(str);

	while (i < len && str[i] != COMMENT_CHAR)
	{
		++i;
	}

	str[i] = 0;

	rtrim(str);
}

//Copy a string until you hit a specific character or end of string
int CopyTill(char *fromstr, char *deststr, int start, char c)
{
	int i;
	int pos = 0;
	int len;

	i = start;
	len = strlen(fromstr);

	while (i < len && fromstr[i] != c)
	{
		deststr[pos++] = fromstr[i];
		++i;
	}

	deststr[pos] = 0;
	return i;
}

//Skip all white space
int SkipWhiteSpace(char *str, int start)
{
	int i;
	int len;

	i = start;
	len = strlen(str);

	while (i < len && str[i] == ' ')
		++i;
	return i;
}

//Get a name / value pair in the form:
// "name1=value1" or
// "name1   = value1"
int getNameValue(char *sline, char *sname, char *svalue)
{
	int pos = 0;

	//Skip initial white space
	pos = SkipWhiteSpace(sline, pos);

	//Get name first
	pos = CopyTill(sline, sname, pos, '=');
	rtrim(sname);

	//Get the equals sign
	if (sline[pos] != '=')
	{
//gi.dprintf("Equal sign not found at pos %d: [%s]\n", pos, sline);
		sname[0] = 0;
		svalue[0] = 0;
		return 0;
	}
	else
		++pos;

	//Get value
	pos = SkipWhiteSpace(sline, pos);
	pos = CopyTill(sline, svalue, pos, '='); //this should get till end of string
	rtrim(svalue);

//gi.dprintf("N/V: [%s], n=[%s], v=[%s]\n", sline, sname, svalue);
	return 1;
}

//Get 4 chunks from an imput line
void get4Chunks(char *sline, char *s1, char *s2, char *s3, char *s4)
{
	int pos = 0;

	//string 1
	pos = SkipWhiteSpace(sline, pos);
	pos = CopyTill(sline, s1, pos, ' ');
	rtrim(s1);

	//string 2
	pos = SkipWhiteSpace(sline, pos);
	pos = CopyTill(sline, s2, pos, ' ');
	rtrim(s2);

	//string 3
	pos = SkipWhiteSpace(sline, pos);
	pos = CopyTill(sline, s3, pos, ' ');
	rtrim(s3);

	//string 4
	pos = SkipWhiteSpace(sline, pos);
	pos = CopyTill(sline, s4, pos, ' ');
	rtrim(s4);

	if (wfdebug) gi.dprintf("4chunks:  [%s] [%s] [%s] [%s]\n", s1, s2, s3, s4);

}


void cfgReadLine(FILE *fp, char *str, int maxlen)
{
	readline(fp, str, maxlen);
	if (str[0] == -1) str[0] = 0;
	RemoveComments(str);
	//gi.dprintf("Read: %s, 1st=%d\n", str, str[0]);
	++config_line_no;
}

void cfgReadLineKeepAll(FILE *fp, char *str, int maxlen)
{
	readline(fp, str, maxlen);
	if (str[0] == -1) str[0] = 0;
	//gi.dprintf("Read: %s, 1st=%d\n", str, str[0]);
	rtrim(str);
	++config_line_no;
}

//----------------- End of Utilities ------------------


//-------------------------------------------
// config_map_list() - Read the map list lines
// Format =
//   "mapname [V]"
// where V is an optional flag to indicate if the map is votable
int config_map_list()
{
	int  numberOfMapsInFile = 0;
	char sMapName[80];
	char sVoteOnly[80];

	// read map names into array
	while (!feof(fpconfig))
	{
		cfgReadLine(fpconfig, config_line, 160);
		//fixline(config_line);
		sMapName[0] = 0;
		sVoteOnly[0] = 0;
		sscanf(config_line, "%s %s", sMapName, sVoteOnly );
		if (config_line[0] == 0)	//Skip blank lines
			continue;
		if (Q_stricmp(sMapName, "###") == 0)  // terminator is "###"
			break;
		if (sMapName[0] == '[')  // new section name
			break;
		if (sMapName[0] == 0)	//Skip blank lines
			continue;

		//Found map
//gi.dprintf("Map In: %s [%s]\n", sMapName, sVoteOnly);


		if (numberOfMapsInFile >= MAX_MAPS)
		{
			gi.dprintf("Map Skipped: %s.  Limit is %d maps.\n", sMapName, MAX_MAPS);
		}
		else
		{

			strncpy(maplist.mapnames[numberOfMapsInFile], sMapName, MAX_MAPNAME_LEN);
			if (sVoteOnly[0] == 'v' || sVoteOnly[0] == 'V')
				maplist.voteonly[numberOfMapsInFile] = true;
			else
				maplist.voteonly[numberOfMapsInFile] = false;
			//gi.dprintf("...%s\n", maplist.mapnames[i]);

			numberOfMapsInFile++;
		}
	}

	if (numberOfMapsInFile == 0)
	{
		gi.dprintf ("WFConfig: No maps listed in [maplist] section\n");
		return 0;  // abnormal exit -- no maps in file
	}

	gi.dprintf ("WFConfig: %i map(s) loaded.\n", numberOfMapsInFile);
	maplist.nummaps = numberOfMapsInFile;
	if (maplist.nummaps) maplist.active = 1;

	//maplist.rotationflag = 0;
	maplist.currentmap = -1;
	WFFillMapNames(); //prepare menu
	return 1;     // normal exit
}


int config_scoring()
{
	int  i=0;
	int  lval;
	char sname[160];
	char svalue[160];


//	gi.dprintf ("WFConfig: Setting custom scores...\n");

	// read map names into array
	while (!feof(fpconfig))
	{
		cfgReadLine(fpconfig, config_line, 160);
		lcase(config_line);

		//Skip blank lines
		if (config_line[0] == 0)
			continue;
		// Stop if we hit terminator
		if (Q_stricmp(config_line, "###") == 0)
			break;
		// Stop if we hit new section name
		if (config_line[0] == '[')
			break;

		if (!getNameValue(config_line, sname, svalue))
			continue;	//Skip bad name/value lines

		//Get scores
		lval = (int) atol(svalue);

		if (Q_stricmp(sname, "frag_points") == 0)
			CTF_FRAG_POINTS = lval;
		else if (Q_stricmp(sname, "suicide_points") == 0)
			CTF_SUICIDE_POINTS = lval;
		else if (Q_stricmp(sname, "sentry_points") == 0)
			CTF_SENTRY_POINTS = lval;
		else if (Q_stricmp(sname, "capture_bonus") == 0)
			CTF_CAPTURE_BONUS = lval;
		else if (Q_stricmp(sname, "team_bonus") == 0)
			CTF_TEAM_BONUS = lval;
		else if (Q_stricmp(sname, "recovery_bonus") == 0)
			CTF_RECOVERY_BONUS = lval;
		else if (Q_stricmp(sname, "flag_bonus") == 0)
			CTF_FLAG_BONUS = lval;
		else if (Q_stricmp(sname, "frag_carrier_bonus") == 0)
			CTF_FRAG_CARRIER_BONUS = lval;
		else if (Q_stricmp(sname, "carrier_danger_protect_bonus") == 0)
			CTF_CARRIER_DANGER_PROTECT_BONUS = lval;
		else if (Q_stricmp(sname, "carrier_protect_bonus") == 0)
			CTF_CARRIER_PROTECT_BONUS = lval;
		else if (Q_stricmp(sname, "flag_defense_bonus") == 0)
			CTF_FLAG_DEFENSE_BONUS = lval;
		else if (Q_stricmp(sname, "return_flag_assist_bonus") == 0)
			CTF_RETURN_FLAG_ASSIST_BONUS = lval;
		else if (Q_stricmp(sname, "frag_carrier_assist_bonus") == 0)
			CTF_FRAG_CARRIER_ASSIST_BONUS = lval;
		else
			gi.dprintf("WFConfig: Invalid Score Name: %s\n", sname);
	}

	return 1;     // normal exit
}

int config_classlimits()
{
	int  i=0;
	char sname[160];
	char svalue[160];
	int iname;
	int ivalue;


//	gi.dprintf ("WFConfig: Setting class limits...\n");

	// read map names into array
	while (!feof(fpconfig))
	{
		cfgReadLine(fpconfig, config_line, 160);
		lcase(config_line);

		//Skip blank lines
		if (config_line[0] == 0)
			continue;
		// Stop if we hit terminator
		if (Q_stricmp(config_line, "###") == 0)
			break;
		// Stop if we hit new section name
		if (config_line[0] == '[')
			break;

		//Name = class number
		//Value = limit
		getNameValue(config_line, sname, svalue);
		iname = (int) atol(sname);
		ivalue = (int) atol(svalue);

		if (iname >= 1 && iname <= MAX_CLASSES)
		{
			classinfo[iname].limit = ivalue;
//			gi.dprintf("Class limit of %s = %d\n", classinfo[iname].name, ivalue);
		}
		else
		{
			gi.dprintf("WFConfig: Bad class #: %d\n", iname);
		}

	}

	return 1;     // normal exit
}


int config_motd()
{
	int  i=0;
	wf_game.motd[0][0] = 0;
	wf_game.motd[1][0] = 0;
	wf_game.motd[2][0] = 0;

//	gi.dprintf ("WFConfig: Setting message of the day...\n");

	// read map names into array
	while (!feof(fpconfig))
	{
		cfgReadLineKeepAll(fpconfig, config_line, 160);

		//Skip blank lines
		if (config_line[0] == 0)
			continue;
		// Stop if we hit terminator
		if (Q_stricmp(config_line, "###") == 0)
			break;
		// Stop if we hit new section name
		if (config_line[0] == '[')
			break;

		//Use MOTD line if # lines not exceeded
		if (i < MAX_MOTD_LINES)
		{
			//Don't exceed length
			if (strlen( config_line) > MAX_MOTD_LENGTH)
			{
				gi.dprintf("Max MOTD length exceeded. Truncated to %s characters.\n", MAX_MOTD_LENGTH);
				config_line [ MAX_MOTD_LENGTH ] = 0;
			}
			strncpy(wf_game.motd[i], config_line, MAX_MOTD_LENGTH );
			++i;
		}
		else
		{
			gi.dprintf("Max MOTD lines = %d. Others ignored\n", MAX_MOTD_LINES);
		}
	}

	RemoveComments(config_line);
	rtrim(config_line);
	return 1;     // normal exit
}


int config_banwords()
{
	int  i=0;


//	gi.dprintf ("WFConfig: Setting banned words...\n");

	// read map names into array
	while (!feof(fpconfig))
	{
		cfgReadLine(fpconfig, config_line, 160);
		lcase(config_line);

		//Skip blank lines
		if (config_line[0] == 0)
			continue;
		// Stop if we hit terminator
		if (Q_stricmp(config_line, "###") == 0)
			break;
		// Stop if we hit new section name
		if (config_line[0] == '[')
			break;

		//Use MOTD line if # lines not exceeded
		if (i < MAX_BANWORDS)
		{
			//Don't exceed length
			if (strlen( config_line) > MAX_BANWORDS_LENGTH)
			{
				gi.dprintf("Max MOTD length exceeded. Truncated to %s characters.\n", MAX_BANWORDS_LENGTH);
				config_line [ MAX_BANWORDS_LENGTH ] = 0;
			}
			strncpy(wf_game.banwords[i], config_line, MAX_BANWORDS_LENGTH );
			++i;
		}
		else
		{
			gi.dprintf("Max banned word lines = %d. Others ignored\n", MAX_BANWORDS);
		}
	}

	wf_game.banwords[i][0] = 0;

	return 1;     // normal exit
}

void wfflags_set(int mask)
{
	wfflags->value = (int)((int)wfflags->value | mask);
}

void wfflags_clear(int mask)
{
	wfflags->value = (int)((int)wfflags->value & ~mask);
}

int config_general()
{
	int  i=0;
	int lval;
	char sname[160];
	char svalue[160];
	int retval;


//	gi.dprintf ("WFConfig: General settings...\n");

	// read map names into array
	while (!feof(fpconfig))
	{
		cfgReadLine(fpconfig, config_line, 160);
		lcase(config_line);

		//Skip blank lines
		if (config_line[0] == 0)
			continue;
		// Stop if we hit terminator
		if (Q_stricmp(config_line, "###") == 0)
			break;
		// Stop if we hit new section name
		if (config_line[0] == '[')
			break;

		if (!getNameValue(config_line, sname, svalue))
			continue;	//Skip bad name/value lines

		//Get general settings (wfflags replacement)
		lval = (int) atol(svalue);

		if (Q_stricmp(sname, "friendly_fire") == 0)
		{
			if (lval)
				wfflags_set(WF_ALLOW_FRIENDLY_FIRE);
			else
				wfflags_clear(WF_ALLOW_FRIENDLY_FIRE);
		}
		else if (Q_stricmp(sname, "frag_logging") == 0)
		{
			if (lval)
				wfflags_set(WF_FRAG_LOGGING);
			else
				wfflags_clear(WF_FRAG_LOGGING);
		}
		else if (Q_stricmp(sname, "no_fort_respawn") == 0)
		{
			if (lval)
				wfflags_set(WF_NO_FORT_RESPAWN);
			else
				wfflags_clear(WF_NO_FORT_RESPAWN);
		}
		else if (Q_stricmp(sname, "no_homing") == 0)
		{
			if (lval)
				wfflags_set(WF_NO_HOMING);
			else
				wfflags_clear(WF_NO_HOMING);
		}
		else if (Q_stricmp(sname, "no_decoys") == 0)
		{
			if (lval)
				wfflags_set(WF_NO_DECOYS);
			else
				wfflags_clear(WF_NO_DECOYS);
		}
		else if (Q_stricmp(sname, "no_flying") == 0)
		{
			if (lval)
				wfflags_set(WF_NO_FLYING);
			else
				wfflags_clear(WF_NO_FLYING);
		}
		else if (Q_stricmp(sname, "decoy_pursue") == 0)
		{
			if (lval)
				wfflags_set(WF_DECOY_PURSUE);
			else
				wfflags_clear(WF_DECOY_PURSUE);
		}
		else if (Q_stricmp(sname, "no_turret") == 0)
		{
			if (lval)
				wfflags_set( WF_NO_TURRET);
			else
				wfflags_clear( WF_NO_TURRET);
		}
		else if (Q_stricmp(sname, "no_earthquakes") == 0)
		{
			if (lval)
				wfflags_set( WF_NO_EARTHQUAKES);
			else
				wfflags_clear( WF_NO_EARTHQUAKES);
		}
		else if (Q_stricmp(sname, "no_grapple") == 0)
		{
			if (lval)
				wfflags_set( WF_NO_GRAPPLE);
			else
				wfflags_clear( WF_NO_GRAPPLE);
		}
		else if (Q_stricmp(sname, "map_voting") == 0)
		{
			if (lval)
				wfflags_set( WF_MAP_VOTE);
			else
				wfflags_clear( WF_MAP_VOTE);
		}
		else if (Q_stricmp(sname, "anarchy") == 0)
		{
			if (lval)
				wfflags_set( WF_ANARCHY);
			else
				wfflags_clear( WF_ANARCHY);
		}
		else if (Q_stricmp(sname, "zoid_flagcap") == 0)
		{
			if (lval)
				wfflags_set( WF_ZOID_FLAGCAP);
			else
				wfflags_clear( WF_ZOID_FLAGCAP);
		}
		else if (Q_stricmp(sname, "no_player_classes") == 0)
		{
			if (lval)
				wfflags_set( WF_NO_PLAYER_CLASSES);
			else
				wfflags_clear( WF_NO_PLAYER_CLASSES);
		}
		else if (Q_stricmp(sname, "zbot_detect") == 0)
		{
			if (lval)
				wfflags_set( WF_ZBOT_DETECT);
			else
				wfflags_clear( WF_ZBOT_DETECT);
		}
		else if (Q_stricmp(sname, "auto_team_balance") == 0)
		{
			if (lval)
				wfflags_set( WF_AUTO_TEAM_BALANCE);
			else
				wfflags_clear( WF_AUTO_TEAM_BALANCE);
		}

		else if (Q_stricmp(sname, "ref_password") == 0)
		{
			strncpy(wf_game.ref_password, svalue, MAX_GAMEINFO_LENGTH);
		}

		else if (Q_stricmp(sname, "classdef") == 0)
		{
			strncpy(wf_game.classdef_name, svalue, MAX_GAMEINFO_LENGTH);

			//Now load the class definitions
			if (wf_game.classdef_name[0] != '\0')
			{
				retval = LoadClassInfo(wf_game.classdef_name);
			}
			else
			{
	//		gi.dprintf("CLASSDEF - Default Class Def Selected\n");
				retval = LoadClassInfo("team10.class");
			}

			if (retval == 0)
			{
				gi.dprintf("ERROR: Could not load class file in quake2\\wf directory\n");
			}


		}

		else if (Q_stricmp(sname, "logfilename") == 0)
		{
			strncpy(wf_game.stdlog_name, svalue, MAX_GAMEINFO_LENGTH);
		}
		else if (Q_stricmp(sname, "weaponfilename") == 0)
		{
			strncpy(wf_game.weaponfile_name, svalue, MAX_GAMEINFO_LENGTH);
			//Now load the weapon definition file
			if (wf_game.weaponfile_name[0] != '\0')
			{
				LoadWeaponInfo();
				if (retval == 0)
				{
					gi.dprintf("ERROR: Could not weapon config file in quake2\\wf directory\n");
				}
			}

		}
		else if (Q_stricmp(sname, "floodertime") == 0)
		{
			wf_game.floodertime = lval;
		}
		else if (Q_stricmp(sname, "unbalanced_limit") == 0)
		{
			wf_game.unbalanced_limit = lval;
		}

		else if (Q_stricmp(sname, "special_lights") == 0)
		{
			if (lval)
				wfflags_set( WF_SPECIAL_LIGHTS);
			else
				wfflags_clear(WF_SPECIAL_LIGHTS);
		}
		else if (Q_stricmp(sname, "map_rotation") == 0)
		{
			if (lval == 0)
				maplist.rotationflag = ML_ROTATE_SEQ;
			else if (lval == 1)
				maplist.rotationflag = ML_ROTATE_RANDOM;
			else
				gi.dprintf("WFConfig: Bad maplist rotation: %d. Must be 0 or 1.\n", lval);

			//gi.dprintf("WFCONFIG - Rotation = %d, [%s] %s\n", lval, svalue, config_line);
		}
		else
			gi.dprintf("WFConfig: Invalid General Setting: %s\n", config_line);

		//gi.dprintf("WFConfig: sname=%s, wfflags=%d\n", sname, (int)wfflags->value);

	}


	return 1;     // normal exit
}

int OpenConfigFile()
{
	char path[100];

	config_line_no = 0;

	//Make sure we open file from the game directory
    strcpy(path, gamedir->string);

#if defined(_WIN32) || defined(WIN32)
    strcat(path,"\\");
#else
    strcat(path,"/");
#endif

	if (wfconfig->string && wfconfig->string[0] != 0)
	{
	    strcat(path, wfconfig->string);
	}
	else
	{
		strcat(path, "wfserver.ini");
	}

	//Open file
	fpconfig = WFOpenFile(NULL, path);
	if (!fpconfig)  // opened successfully?
	{
		gi.dprintf("Config [%s] not found!\n",path);
		return 0;
	}
	else
	{
		gi.dprintf("Loading config file: %s\n",path);
		return 1;
	}
}

void ClearConfig()
{
	wf_game.ref_password[0] = 0;
	wf_game.ref_ent = NULL;
	wf_game.game_halted = 0;
	ClearMapList();

//Weapon Damage
wf_game.weapon_damage[WEAPON_BLASTER] = DAMAGE_BLASTER;
wf_game.weapon_damage[WEAPON_SHOTGUN] = DAMAGE_SHOTGUN;
wf_game.weapon_damage[WEAPON_SUPERSHOTGUN] = DAMAGE_SUPERSHOTGUN;
wf_game.weapon_damage[WEAPON_MACHINEGUN] = DAMAGE_MACHINEGUN;
wf_game.weapon_damage[WEAPON_CHAINGUN] = DAMAGE_CHAINGUN;
wf_game.weapon_damage[WEAPON_HYPERBLASTER] = DAMAGE_HYPERBLASTER;
wf_game.weapon_damage[WEAPON_ROCKETLAUNCHER] = DAMAGE_ROCKETLAUNCHER;
wf_game.weapon_damage[WEAPON_GRENADELAUNCHER] =DAMAGE_GRENADELAUNCHER;
wf_game.weapon_damage[WEAPON_RAILGUN] = DAMAGE_RAILGUN;
wf_game.weapon_damage[WEAPON_BFG] = DAMAGE_BFG;
wf_game.weapon_damage[WEAPON_NEEDLER] = DAMAGE_NEEDLER;
wf_game.weapon_damage[WEAPON_NAG] = DAMAGE_NAG;
wf_game.weapon_damage[WEAPON_TELSA] = DAMAGE_TELSA;
wf_game.weapon_damage[WEAPON_LIGHTNING] = DAMAGE_LIGHTNING;
wf_game.weapon_damage[WEAPON_NAILGUN] = DAMAGE_NAILGUN;
wf_game.weapon_damage[WEAPON_CLUSTERROCKET] = DAMAGE_CLUSTERROCKET;
wf_game.weapon_damage[WEAPON_MAGBOLTED] = DAMAGE_MAGBOLTED;
wf_game.weapon_damage[WEAPON_PELLET] = DAMAGE_PELLET;
wf_game.weapon_damage[WEAPON_PULSE] = DAMAGE_PULSE;
wf_game.weapon_damage[WEAPON_SHC] = DAMAGE_SHC;
wf_game.weapon_damage[WEAPON_FLAREGUN] = DAMAGE_FLAREGUN;
wf_game.weapon_damage[WEAPON_NAPALMMISSLE] = DAMAGE_NAPALMMISSLE;
wf_game.weapon_damage[WEAPON_FLAMETHROWER] = DAMAGE_FLAMETHROWER;
wf_game.weapon_damage[WEAPON_TRANQUILIZER] = DAMAGE_TRANQUILIZER;
wf_game.weapon_damage[WEAPON_INFECTEDDART] = DAMAGE_INFECTEDDART;
wf_game.weapon_damage[WEAPON_LASERSNIPER] = DAMAGE_LASERSNIPER;
wf_game.weapon_damage[WEAPON_ARMORDART] = DAMAGE_ARMORDART;
wf_game.weapon_damage[WEAPON_SHOTGUNCHOKE] = DAMAGE_SHOTGUNCHOKE;
wf_game.weapon_damage[WEAPON_SNIPERRIFLE] = DAMAGE_SNIPERRIFLE;
wf_game.weapon_damage[WEAPON_LRPROJECTILE] = DAMAGE_LRPROJECTILE;
wf_game.weapon_damage[WEAPON_SENTRYKILLER] = DAMAGE_SENTRYKILLER;
wf_game.weapon_damage[WEAPON_MEGACHAINGUN] = DAMAGE_MEGACHAINGUN;
wf_game.weapon_damage[WEAPON_TRANQUILDART] = DAMAGE_TRANQUILDART;
wf_game.weapon_damage[WEAPON_KNIFE] = DAMAGE_KNIFE;
wf_game.weapon_damage[WEAPON_AK47] = DAMAGE_AK47;
wf_game.weapon_damage[WEAPON_PISTOL] = DAMAGE_PISTOL;
wf_game.weapon_damage[WEAPON_STINGER] = DAMAGE_STINGER;
wf_game.weapon_damage[WEAPON_DISRUPTOR] = DAMAGE_DISRUPTOR;
wf_game.weapon_damage[WEAPON_ETF_RIFLE] = DAMAGE_ETF_RIFLE;
wf_game.weapon_damage[WEAPON_PLASMA_BEAM] = DAMAGE_PLASMA_BEAM;
wf_game.weapon_damage[WEAPON_ION_RIPPER] = DAMAGE_ION_RIPPER;
wf_game.weapon_damage[WEAPON_PHALANX] = DAMAGE_PHALANX;
wf_game.weapon_damage[WEAPON_FREEZER] = DAMAGE_FREEZER;

//Weapon Speed
wf_game.weapon_speed[WEAPON_BLASTER] = SPEED_BLASTER;
wf_game.weapon_speed[WEAPON_SHOTGUN] = SPEED_SHOTGUN;
wf_game.weapon_speed[WEAPON_SUPERSHOTGUN] = SPEED_SUPERSHOTGUN;
wf_game.weapon_speed[WEAPON_MACHINEGUN] = SPEED_MACHINEGUN;
wf_game.weapon_speed[WEAPON_CHAINGUN] = SPEED_CHAINGUN;
wf_game.weapon_speed[WEAPON_HYPERBLASTER] = SPEED_HYPERBLASTER;
wf_game.weapon_speed[WEAPON_ROCKETLAUNCHER] = SPEED_ROCKETLAUNCHER;
wf_game.weapon_speed[WEAPON_GRENADELAUNCHER] =SPEED_GRENADELAUNCHER;
wf_game.weapon_speed[WEAPON_RAILGUN] = SPEED_RAILGUN;
wf_game.weapon_speed[WEAPON_BFG] = SPEED_BFG;
wf_game.weapon_speed[WEAPON_NEEDLER] = SPEED_NEEDLER;
wf_game.weapon_speed[WEAPON_NAG] = SPEED_NAG;
wf_game.weapon_speed[WEAPON_TELSA] = SPEED_TELSA;
wf_game.weapon_speed[WEAPON_LIGHTNING] = SPEED_LIGHTNING;
wf_game.weapon_speed[WEAPON_NAILGUN] = SPEED_NAILGUN;
wf_game.weapon_speed[WEAPON_CLUSTERROCKET] = SPEED_CLUSTERROCKET;
wf_game.weapon_speed[WEAPON_MAGBOLTED] = SPEED_MAGBOLTED;
wf_game.weapon_speed[WEAPON_PELLET] = SPEED_PELLET;
wf_game.weapon_speed[WEAPON_PULSE] = SPEED_PULSE;
wf_game.weapon_speed[WEAPON_SHC] = SPEED_SHC;
wf_game.weapon_speed[WEAPON_FLAREGUN] = SPEED_FLAREGUN;
wf_game.weapon_speed[WEAPON_NAPALMMISSLE] = SPEED_NAPALMMISSLE;
wf_game.weapon_speed[WEAPON_FLAMETHROWER] = SPEED_FLAMETHROWER;
wf_game.weapon_speed[WEAPON_TRANQUILIZER] = SPEED_TRANQUILIZER;
wf_game.weapon_speed[WEAPON_INFECTEDDART] = SPEED_INFECTEDDART;
wf_game.weapon_speed[WEAPON_LASERSNIPER] = SPEED_LASERSNIPER;
wf_game.weapon_speed[WEAPON_ARMORDART] = SPEED_ARMORDART;
wf_game.weapon_speed[WEAPON_SHOTGUNCHOKE] = SPEED_SHOTGUNCHOKE;
wf_game.weapon_speed[WEAPON_SNIPERRIFLE] = SPEED_SNIPERRIFLE;
wf_game.weapon_speed[WEAPON_LRPROJECTILE] = SPEED_LRPROJECTILE;
wf_game.weapon_speed[WEAPON_SENTRYKILLER] = SPEED_SENTRYKILLER;
wf_game.weapon_speed[WEAPON_MEGACHAINGUN] = SPEED_MEGACHAINGUN;
wf_game.weapon_speed[WEAPON_TRANQUILDART] = SPEED_TRANQUILDART;
wf_game.weapon_speed[WEAPON_KNIFE] = SPEED_KNIFE;
wf_game.weapon_speed[WEAPON_AK47] = SPEED_AK47;
wf_game.weapon_speed[WEAPON_PISTOL] = SPEED_PISTOL;
wf_game.weapon_speed[WEAPON_STINGER] = SPEED_STINGER;
wf_game.weapon_speed[WEAPON_DISRUPTOR] = SPEED_DISRUPTOR;
wf_game.weapon_speed[WEAPON_ETF_RIFLE] = SPEED_ETF_RIFLE;
wf_game.weapon_speed[WEAPON_PLASMA_BEAM] = SPEED_PLASMA_BEAM;
wf_game.weapon_speed[WEAPON_ION_RIPPER] = SPEED_ION_RIPPER;
wf_game.weapon_speed[WEAPON_PHALANX] = SPEED_PHALANX;
wf_game.weapon_speed[WEAPON_FREEZER] = SPEED_FREEZER;

//Grenade damage
wf_game.grenade_damage[GRENADE_TYPE_NORMAL] = DAMAGE_GRENADE;
wf_game.grenade_damage[GRENADE_TYPE_LASERBALL] = DAMAGE_LASERBALL;
wf_game.grenade_damage[GRENADE_TYPE_GOODYEAR] = DAMAGE_GOODYEAR;
wf_game.grenade_damage[GRENADE_TYPE_PROXIMITY] = DAMAGE_PROXIMITY;
wf_game.grenade_damage[GRENADE_TYPE_FLASH] = DAMAGE_FLASH;
wf_game.grenade_damage[GRENADE_TYPE_CLUSTER] = DAMAGE_CLUSTER;
wf_game.grenade_damage[GRENADE_TYPE_EARTHQUAKE] = DAMAGE_EARTHQUAKE;
wf_game.grenade_damage[GRENADE_TYPE_TURRET] = DAMAGE_TURRET;
wf_game.grenade_damage[GRENADE_TYPE_NAPALM] = DAMAGE_NAPALM;
wf_game.grenade_damage[GRENADE_TYPE_CONCUSSION] = DAMAGE_CONCUSSION;
wf_game.grenade_damage[GRENADE_TYPE_NARCOTIC] = DAMAGE_NARCOTIC;
wf_game.grenade_damage[GRENADE_TYPE_PLAGUE] = DAMAGE_PLAGUE;
wf_game.grenade_damage[GRENADE_TYPE_MAGNOTRON] = DAMAGE_MAGNOTRON;
wf_game.grenade_damage[GRENADE_TYPE_SHOCK] = DAMAGE_SHOCK;
wf_game.grenade_damage[GRENADE_TYPE_PIPEBOMB] = DAMAGE_PIPEBOMB;
wf_game.grenade_damage[GRENADE_TYPE_SHRAPNEL] = DAMAGE_SHRAPNEL;
wf_game.grenade_damage[GRENADE_TYPE_FLARE] = DAMAGE_FLARE;
wf_game.grenade_damage[GRENADE_TYPE_SLOW] = DAMAGE_SLOW;
wf_game.grenade_damage[GRENADE_TYPE_PLAGUETIME] = DAMAGE_PLAGUETIME;
wf_game.grenade_damage[GRENADE_TYPE_LASERCUTTER] = DAMAGE_LASERCUTTER;
wf_game.grenade_damage[GRENADE_TYPE_TESLA] = DAMAGE_TESLA;
wf_game.grenade_damage[GRENADE_TYPE_GAS] = DAMAGE_GAS;

//Grenade speed
wf_game.grenade_speed[GRENADE_TYPE_NORMAL] = SPEED_GRENADE;
wf_game.grenade_speed[GRENADE_TYPE_LASERBALL] = SPEED_LASERBALL;
wf_game.grenade_speed[GRENADE_TYPE_GOODYEAR] = SPEED_GOODYEAR;
wf_game.grenade_speed[GRENADE_TYPE_PROXIMITY] = SPEED_PROXIMITY;
wf_game.grenade_speed[GRENADE_TYPE_FLASH] = SPEED_FLASH;
wf_game.grenade_speed[GRENADE_TYPE_CLUSTER] = SPEED_CLUSTER;
wf_game.grenade_speed[GRENADE_TYPE_EARTHQUAKE] = SPEED_EARTHQUAKE;
wf_game.grenade_speed[GRENADE_TYPE_TURRET] = SPEED_TURRET;
wf_game.grenade_speed[GRENADE_TYPE_NAPALM] = SPEED_NAPALM;
wf_game.grenade_speed[GRENADE_TYPE_CONCUSSION] = SPEED_CONCUSSION;
wf_game.grenade_speed[GRENADE_TYPE_NARCOTIC] = SPEED_NARCOTIC;
wf_game.grenade_speed[GRENADE_TYPE_PLAGUE] = SPEED_PLAGUE;
wf_game.grenade_speed[GRENADE_TYPE_MAGNOTRON] = SPEED_MAGNOTRON;
wf_game.grenade_speed[GRENADE_TYPE_SHOCK] = SPEED_SHOCK;
wf_game.grenade_speed[GRENADE_TYPE_PIPEBOMB] = SPEED_PIPEBOMB;
wf_game.grenade_speed[GRENADE_TYPE_SHRAPNEL] = SPEED_SHRAPNEL;
wf_game.grenade_speed[GRENADE_TYPE_FLARE] = SPEED_FLARE;
wf_game.grenade_speed[GRENADE_TYPE_SLOW] = SPEED_SLOW;
wf_game.grenade_speed[GRENADE_TYPE_PLAGUETIME] = SPEED_PLAGUETIME;
wf_game.grenade_speed[GRENADE_TYPE_LASERCUTTER] = SPEED_LASERCUTTER;
wf_game.grenade_speed[GRENADE_TYPE_TESLA] = SPEED_TESLA;
wf_game.grenade_speed[GRENADE_TYPE_GAS] = SPEED_GAS;
}

//Load the weapon info from a file
void LoadWeaponInfo()
{
	FILE *fp;
	int  i=0;
	unsigned char str[200];
	char path[100];
	char s1[60];
	char s2[60];
	char s3[60];
	char s4[60];
	long i2, i3, i4;

	//Make sure we open class def from the game directory
    strcpy(path, gamedir->string);
#if defined(_WIN32) || defined(WIN32)
    strcat(path,"\\");
#else
    strcat(path,"/");
#endif
    strcat(path,wf_game.weaponfile_name);

	//Open file
	fp = WFOpenFile(NULL, path);
	if (!fp)  // opened successfully?
	{
		gi.dprintf("WeaponConfig: Could not open file: %s\n",path);
	}

	readline(fp, str, 200);
	if (str[0] == -1) str[0] = 0;
	RemoveComments(str);

	while (!feof(fp))
	{
		/*Parse components:, ie "W 1 100 800" where

			  1 = "W" for weapon or "G" for grenade
			  2 = weapon or grenade id number
			  3 = damage
			  4 = speed
		*/
		get4Chunks( str, s1, s2, s3, s4);

		//What kind of line?
		if (s1[0])
		{
			sscanf(s2, "%d", &i2);	//id number
			sscanf(s3, "%d", &i3);	//damage
			sscanf(s4, "%d", &i4);	//speed

			if (s1[0] == 'W' && i2 >= 0 && i2 <= WEAPON_COUNT)
			{
				wf_game.weapon_damage[i2] = i3;
				wf_game.weapon_speed[i2] = i4;

			}
			if (s1[0] == 'G' && i2 >= 0 && i2 <= GRENADE_TYPE_COUNT)
			{
				wf_game.grenade_damage[i2] = i3;
				wf_game.grenade_speed[i2] = i4;

			}
		}

		//Read next line
		readline(fp, str, 200);
		if (str[0] == -1) str[0] = 0;
		RemoveComments(str);
	}

	WFCloseFile(NULL, fp);
	gi.dprintf("Loaded custom weapons config file: %s\n",path);
}



int ProcessConfigFile()
{

	if (!OpenConfigFile())
		return false;

	ClearConfig();

	cfgReadLine(fpconfig, config_line, 200);

	while (!feof(fpconfig))
	{
		//Did we find a section header?
		if (config_line[0] == '[')
		{
			lcase(config_line);

			//What section?
			if (Q_stricmp(config_line, "[maplist]") == 0)
			{
				config_map_list();
			}
			else if (Q_stricmp(config_line, "[scoring]") == 0)
			{
				config_scoring();
			}
			else if (Q_stricmp(config_line, "[classlimits]") == 0)
			{
				config_classlimits();
			}
			else if (Q_stricmp(config_line, "[general]") == 0)
			{
				config_general();
			}
			else if (Q_stricmp(config_line, "[motd]") == 0)
			{
				config_motd();
			}
			else if (Q_stricmp(config_line, "[banwords]") == 0)
			{
				config_banwords();
			}
			else
			{
				gi.dprintf("Server Config: Bad section at line %d: '%s'\n", config_line_no, config_line);
				cfgReadLine(fpconfig, config_line, 200);
			}
		}

		//Skip blank lines
		else if (config_line[0] == 0)
		{
			cfgReadLine(fpconfig, config_line, 200);
		}

		//Skip everything after file terminator
		else if (Q_stricmp(config_line, "###") == 0)
			break;	//skip rest of file

		else
		{
			gi.dprintf("Server Config: Line %d not in a section: %s\n", config_line_no, config_line);
			cfgReadLine(fpconfig, config_line, 200);
		}
	}
	WFCloseFile(NULL, fpconfig);
}
