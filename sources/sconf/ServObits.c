// ServObits.c
// by SteQve (steqve@shore.net)
// See ServObits.h for version information.

// The code has been divided into the following sections:
// - Global Variables
// - String Utilities
// - Weapon Characteristics and Conversions
// - Player Characteristics
// - Parser Functions
// - Obituary Matching and Construction
// - Main Functions

// This code is written to be readable and understandable.  As a result,
// it's not particularly elegant.  You have been warned.

// Thanks to idsoftware for Quake - natch.

// And thanks to NewtonD (newton@moongates.com) for his Death Message mod which
// gave me ideas and encouragement for extending ServObit.  Too bad he's a good
// C programmer; I couldn't understand his code before I ran out of patience ;-)

#include <stdio.h>
#include <stdlib.h>

#include "g_local.h"

// GenderMod
#include "gender.h"
// End GenderMod

#include "ServObits.h"
#include "motd.h"
#include "ModUtils.h"

#include "SrvObAPI.h"
#include "s_readconfig.h"

/* *******************************
   Global Variables
   ******************************* */

struct ServObitGameState ServObit;

FILE *LOG;

struct Obituary **ServObits;
struct ServObitSubstitution *ServObitSubstitutionList;

/* **************************************************************
   String Utilities 
   ************************************************************** */

// This is the wrong name and function; such is life.  Should really just be
// picking off word characters.
int IsTokenSeparator (char x)
{
	if ((x == ' ') || (x == '\t') || (x == '\n')) return(1);
	else return(0);
}

// **** NOTE ****  StrBeginsWith() was moved to ModUtils.c

// Replaces every occurrence of field with value in the format_str.
// Places result into mess.  Returns -1 if it ran out of room,
// 0 if no match found, and >0 for number of matches inserted.
//
// This isn't used by most ServObit code because it is inefficient if you have to insert
// more than one value (you'd basically be doing multiple passes through the string
// with lots of substring matching).
//
// Hey, at least I did *one* optimization - optimization by omission ;-)
//
// Speaking of optimization, can anybody say strstr()?  Sure they can...  but
// I want a case-insensitive strstr().
int ServObitInsertValue(char *mess, char *format_str, char *field, char *value, int max_size)
{
	int x=0, y=0, matches = 0;
	mess[0] = 0;
	while (format_str[x] && (x < max_size))
	{
		if (StrBeginsWith(field, format_str + x))
		{
			if (x + strlen(value) < max_size)
			{
				//PRINT2("Found match on '%s' for field '%s\n", format_str, field);
				matches++;
				strcpy(mess+y, value);
				y = y + strlen(value);
				x = x + strlen(field);
			}
			else {
				// Buffer overflow would occur.
				return(-1);
			}
		}
		else {
			mess[y++] = format_str[x++];
		}
	}
	if (y < max_size)
		mess[y] = 0;
	else mess[max_size - 1] = 0;
	return(matches);
}

void WriteServObitPathname(char *path, char *filename)
{
	//sprintf(path, "%s%s", ServObit.Path, filename);
	sprintf(path, "%s%s", obitsDir, filename);
}

/* **************************************************************
   Weapon and Player Characteristics
   ************************************************************** */

int IsEnemyWeapon (int wtype)
{
	return((wtype >= SO_WEAPON_START) &&
		   (wtype <= SO_WEAPON_END));
}

int IsEnvironmentWeapon (int wtype)
{
	return((wtype >= SO_ENV_START) &&
		   (wtype <= SO_ENV_END));
}


// Is b1 same or worse than b2?	
// b1 == victim bstate, b2 == obit bstate
int BodyStateSameOrWorseThan(b1, b2)
{
	if (b2 == SO_BODY_STATE_ANY)
		return(1);
	else if (b2 <= b1)
		return(1);
	else return(0);
}


/* **************************************************************
   Substitutions
   ************************************************************** */


struct ServObitSubstitution *FindServObitSubstitution (char *key)
{
	struct ServObitSubstitution *cur = ServObitSubstitutionList;

	while (cur != NULL)
	{
		if (Q_stricmp(key, cur->key) == 0)
			return(cur);
		cur = cur->next;
	}
	return(NULL);
}

char * GetRandomSubstitution (char *key)
{
	struct ServObitSubstitution *sub = FindServObitSubstitution(key);

	if (sub == NULL)
		return(NULL);
	else if (sub->NumValues == 0)
		return(NULL);
	else
		return(sub->values[rand() % sub->NumValues]);
}

// DESTRUCTIVE - writes into string
void PerformSubstitutions (char *orig)
{
	int x = 0, n = 0, tmp, len = strlen(orig);
	char key[MAX_STRING_CHARS], *sub;
	char newstr[MAX_STRING_CHARS];

	while (x < len)
	{
		if (orig[x] == '%')
		{
			// *** Pick off substitution id ***
			for (tmp=x+1; (orig[tmp] != '\0') && (orig[tmp] != '%'); tmp++);

			if (orig[tmp] == '\0')
			{
				// No matching %; keep it.
				newstr[n++] = orig[x++];
				continue;
			}
			// Extract the key
			strncpy(key, orig+x+1, tmp - (x + 1));
			key[tmp - (x + 1)] = '\0';
			sub = GetRandomSubstitution(key);
			if (sub == NULL)
			{
				newstr[n++] = orig[x++];
				continue;
			}
			*(newstr+n) = '\0';
			strcat(newstr+n, sub);
			n += strlen(sub);
			x = tmp+1;
		}
		else
			newstr[n++] = orig[x++];
	}
	newstr[n] = '\0';
	strcpy(orig, newstr);
}

struct ServObitSubstitution *MakeServObitSubstitution (char *key,
													   struct ServObitSubstitution *last)
{
	struct ServObitSubstitution *sub;
	int i;

	sub = (struct ServObitSubstitution *)malloc(sizeof(struct ServObitSubstitution));
	sub->key = key;
	sub->NumValues = 0;
	sub->next = NULL;
	// *** MONSTROUS waste of space!!! ***//
	sub->values = (char **)malloc(sizeof(char *) * ServObit.MaxSubstitutions);

	for (i=0; i<ServObit.MaxSubstitutions; i++)
	{
		sub->values[i] = NULL;
	}
	if (last != NULL)
		last->next = sub;

	return(sub);
}

/* **************************************************************
   Parser Functions
   ************************************************************** */

int ConvertObitBodyStateSpec (char *str)
{
	if (str[0] == '*')
		return(SO_BODY_STATE_ANY);
	else if ((str[0] == 'M') || (str[0] == 'm'))
		return(SO_BODY_STATE_MEGAGIB);
	else if ((str[0] == 'G') || (str[0] == 'g'))
		return(SO_BODY_STATE_GIB);
	else if ((str[0] == 'N') || (str[0] == 'n'))
		return(SO_BODY_STATE_NORMAL);
	else return(0);
}

int ConvertObitPowerSpec (char * power)
{
	if (power[0] == '*') 
		return(SO_POWER_ANY);
	else if (power[0] == 'Q' || power[0] == 'q')
		return(SO_POWER_QUAD);
	else if (power[0] == 'N' || power[0] == 'n')
		return(SO_POWER_NORMAL);
	else
		return(0);
}

int ConvertObitKillerSpec (char *killer)
{
	if (killer[0] == 'S' || killer[0] == 's')
		return(SO_KILLER_SELF);
	else if (killer[0] == 'E' || killer[0] == 'e')
		return(SO_KILLER_ENEMY);
	else if (killer[0] == 'W' || killer[0] == 'w')
		return(SO_KILLER_ENVIRONMENT);
	else if (killer[0] == '*')
		return(SO_KILLER_ANY);
	else return(0);
}

int ConvertObitWeaponSpec (char *weapon)
{
	//printf("Weapon Spec %s\n", weapon);
	if (StrBeginsWith("sup", weapon))
		return(SO_WEAPON_SUPER_SHOTGUN);
	else if (StrBeginsWith("sh", weapon))
		return(SO_WEAPON_SHOTGUN);
	else if (StrBeginsWith("bfg_b", weapon))
		return(SO_WEAPON_BFG_BLAST);
	else if (StrBeginsWith("bfg_l", weapon))
		return(SO_WEAPON_BFG_LASER);
	else if (StrBeginsWith("bfg_e", weapon))
		return(SO_WEAPON_BFG_EFFECT);
	else if (StrBeginsWith("bl", weapon))
		return(SO_WEAPON_BLASTER);
	else if (StrBeginsWith("hy", weapon))
		return(SO_WEAPON_HYPER_BLASTER);
	else if (StrBeginsWith("ha", weapon))
		return(SO_WEAPON_HAND_GRENADE);
	else if (StrBeginsWith("hg_s", weapon))
		return(SO_WEAPON_HAND_GRENADE_SPLASH);
	else if (StrBeginsWith("hg_h", weapon))
		return(SO_WEAPON_HAND_GRENADE_HELD);
	else if (StrBeginsWith("gr", weapon))
		return(SO_WEAPON_GRENADE_LAUNCHER);
	else if (StrBeginsWith("g_", weapon))
		return(SO_WEAPON_GRENADE_SPLASH);
	else if (StrBeginsWith("ro", weapon))
		return(SO_WEAPON_ROCKET_LAUNCHER);
	else if (StrBeginsWith("r_", weapon))
		return(SO_WEAPON_ROCKET_LAUNCHER_SPLASH);
	else if (StrBeginsWith("ra", weapon))
		return(SO_WEAPON_RAILGUN);
	else if (StrBeginsWith("c", weapon))
		return(SO_WEAPON_CHAINGUN);
	else if (StrBeginsWith("m", weapon))
		return(SO_WEAPON_MACHINEGUN);
	else if (StrBeginsWith("te", weapon))
		return(SO_WEAPON_TELEFRAG);
	else if (StrBeginsWith("lav", weapon))
		return(SO_ENV_LAVA);
	else if (StrBeginsWith("las", weapon))
		return(SO_ENV_LASER);
	else if (StrBeginsWith("ex", weapon))
		return(SO_ENV_EXIT);
	else if (StrBeginsWith("we", weapon))
		return(SO_WEAPON_ENEMY);
	else if (StrBeginsWith("wa", weapon))
		return(SO_ENV_WATER);
	else if (StrBeginsWith("sl", weapon))
		return(SO_ENV_SLIME);
	else if (StrBeginsWith("sui", weapon))
		return(SO_WEAPON_SUICIDE);
	else if (StrBeginsWith("f", weapon))
		return(SO_ENV_FALL);
	else if (StrBeginsWith("to", weapon))
		return(SO_ENV_TOUCH);
	else if (StrBeginsWith("sq", weapon))
		return(SO_ENV_SQUISH);
	else if (StrBeginsWith("e", weapon))
		return(SO_ENV_ANY);
	else if (weapon[0] == '*')
		return(SO_DEATH_ANY);
	else return(0);
}

// I hate it when compilers complain.  Hopefully server operators won't hate
// it when ServObit complains.
int ComplainInvalidServObitInput (int value, char *line, char *field, 
								  char *val, int line_num)
{
	if (! value)
	{
		PRINTLOG3("Parse Error in %s of line %d; value = '%s'\n", 
				field, line_num, val);
		PRINTLOG1("-->Text: %s\n", line);
		ServObit.ParseErrors++;
		return(1);
	}
}

// Look at HimHer, HisHer, etc.
void ComplainInvalidConnectNameSpecifiers (char *message, int line_no)
{
	char *str;

	str = message;
	while (*str != '\0')
	{
		if (StrBeginsWith("$", str))
		{
			if (StrBeginsWith("$N", str))
				str += 2;
			else if (StrBeginsWith("$HIS", str))
				str += 4;
			else if (StrBeginsWith("$HIM", str))
				str += 4;
			else if (StrBeginsWith("$HERSELF", str))
				str += 8;
			else if (StrBeginsWith("$SHE", str))
				str += 4;
			else
			{
				PRINTLOG2("ERROR: line %d: Can't understand $ specifier starting with '%s'\n",
						  line_no, str);
				PRINTLOG1("--> %s\n", message);
				ServObit.ParseErrors++;
				str += 1;
			}
		}
		else str++;
	}
}

// Look at HimHer, HisHer, etc.
void ComplainInvalidNameSpecifiers (char *message, int line_no)
{
	char *str;

	str = message;
	while (*str != '\0')
	{
		if (StrBeginsWith("$", str))
		{
			if (StrBeginsWith("$V", str))
				str += 2;
			else if (StrBeginsWith("$K", str))
				str += 2;
			else if (StrBeginsWith("$HISK", str))
				str += 5;
			else if (StrBeginsWith("$HISV", str))
				str += 5;
			else if (StrBeginsWith("$HIMK", str))
				str += 5;
			else if (StrBeginsWith("$HIMV", str))
				str += 5;
			else if (StrBeginsWith("$HERSELFV", str))
				str += 9;
			else if (StrBeginsWith("$HERSELFK", str))
				str += 9;
			else if (StrBeginsWith("$SHEK", str))
				str += 5;
			else if (StrBeginsWith("$SHEV", str))
				str += 5;
			else if (StrBeginsWith("$Q", str))
				str += 2;
			else if (StrBeginsWith("$G", str))
				str += 2;
			else 
			{
				PRINTLOG2("ERROR: line %d: Can't understand $ specifier starting with '%s'\n",
						  line_no, str);
				PRINTLOG1("--> %s\n", message);
				ServObit.ParseErrors++;
				str += 1;
			}
		}
		else str++;
	}
}

void ComplainInvalidSubstitutions (char *message, int line_no)
{
	int i, len;
	char str[512], *start, *tmp;
	struct ServObitSubstitution *sub;

	i = 0;
	len = strlen(message);
	start = strchr(message, '%');
	while (start != NULL)
	{
		i = 0;
		tmp = start;
		tmp++;
		while ((*tmp != '\0') && (*tmp != '%'))
			tmp++;
		if (*tmp == '\0')
		{
			PRINTLOG2("ERROR: line %d: Started substitution '%s' with no closing %%\n",
					  line_no, start);
			PRINTLOG1("--> %s\n", message);
			ServObit.ParseErrors++;
			break;
		}
		else 
		{
			// Check to make sure substitution exists
			*tmp = '\0'; // temporarily alters the message string
			strcpy(str, start+1);
			*tmp = '%'; // need to restore the % character into the message
			sub = FindServObitSubstitution(str);
			if (sub == NULL)
			{
				PRINTLOG2("ERROR: line %d: Substitution '%s' not defined!\n",
						  line_no, str);
				PRINTLOG1("--> %s\n", message);
				ServObit.ParseErrors++;
			}
			start = strchr(tmp+1, '%');
		}
	}
}

void ComplainInvalidServObitMessageSpecifiers (char *message, int line_no)
{
	ComplainInvalidSubstitutions(message, line_no);
	ComplainInvalidNameSpecifiers(message, line_no);
}

void ComplainInvalidServObitConnectMessageSpecifiers (char *message, int line_no)
{
	ComplainInvalidSubstitutions(message, line_no);
	ComplainInvalidConnectNameSpecifiers(message, line_no);
}


void AddMessageToServObit(struct Obituary *Obit, char *message, int line_no)
{
	struct ServObitSubstitution *sub;
	char *end;
	int i;

	if (message[0] == '[')
	{
		// User is defining a wholesale obit substitution
		message++;
		end = strchr(message, ']');
		if (end != NULL)
			end[0] = '\0';
		sub = FindServObitSubstitution(message);
		if (sub == NULL)
		{
			// Bad substitution?  Complain.
			PRINTLOG2("ERROR: line %d: unknown obit substitution: %s\n", 
					  line_no, message);
			ServObit.ParseErrors++;
		}
		else {
			for(i=0; i<sub->NumValues; i++)
			{
				if (Obit->MessageSize == ServObit.MaxObitMessages)
				{
					PRINTLOG1("ERROR: line %d: Exceeded MaxObitMessages.  Increase its value in ServObit.ini\n",
							line_no);
					PRINTLOG1("--> %s\n", message);
					ServObit.ParseErrors++;
					break;
				}
				Obit->Messages[Obit->MessageSize] = sub->values[i];
				Obit->MessageSize++;
				// Should complain once, when the obit substitutions are
				// being defined.
				ComplainInvalidServObitMessageSpecifiers(sub->values[i], line_no);
			}
		}
	}
	else {
		if (Obit->MessageSize == ServObit.MaxObitMessages)
		{
			PRINTLOG1("ERROR: Line %d: Exceeded MaxObitMessages.  Increase its value in ServObit.ini\n",
					   line_no);
			PRINTLOG1("--> %s\n", message);
			ServObit.ParseErrors++;
		}
		else
		{
			Obit->Messages[Obit->MessageSize] = message;
			ComplainInvalidServObitMessageSpecifiers(message, line_no);
			Obit->MessageSize = Obit->MessageSize + 1;
		}
	}
}

// I'm patently embarrassed by this whole function.  Don't bug me
// about it.  It's a horrific display of bad, unelegant coding.
struct Obituary * ParseObitLine (char * line, int line_no)
{
	int x, y, max;
	char killer[SO_MAX_OBIT_LENGTH], weapon[SO_MAX_OBIT_LENGTH], 
		power[SO_MAX_OBIT_LENGTH], kgen[SO_MAX_OBIT_LENGTH], vgen[SO_MAX_OBIT_LENGTH],
		bstate[SO_MAX_OBIT_LENGTH];
	struct Obituary *obit;

	//printf("STEQVE: parsing obit line %s\n", line);
	obit = (struct Obituary *)malloc(sizeof(struct Obituary));
	// Optimize!  Lots o' wasted space here.
	obit->Messages = (char **)malloc(sizeof(char *) * ServObit.MaxObitMessages);
	killer[0] = 0;
	weapon[0] = 0;
	power[0] = 0;
	kgen[0] = 0;
	vgen[0] = 0;
	bstate[0] = 0;
	obit->MessageSize = 0;

	if (strlen(line) < SO_MAX_OBIT_LENGTH)
		max = strlen(line);
	else max = SO_MAX_OBIT_LENGTH;

	// HEY!  Do ya think I should have written a procedure to do these string
	// copies, or used some strtok() function?  Well, do ya, punk?
	// Cut-n-paste is easier than research.

	y = 0;
	for(x=0; (x < max) && (! IsTokenSeparator(line[x])); x++) {
		//printf("x = %d\n", x);
		//fflush(stdout);
		killer[y] = line[x];
		y++;
	}
	killer[y] = 0;
	y = 0;
	for(x=x+1; (x < max) && (! IsTokenSeparator(line[x])); x++) {
		weapon[y] = line[x];
		y++;
	}
	weapon[y] = 0;
	y = 0;
	for(x=x+1; (x < max) && (! IsTokenSeparator(line[x])); x++) {
		power[y] = line[x];
		y++;
	}
	power[y] = 0;
	y = 0;
	for(x=x+1; (x < max) && (! IsTokenSeparator(line[x])); x++) {
		kgen[y] = line[x];
		y++;
	}
	kgen[y] = 0;
	y = 0;
	for(x=x+1; (x < max) && (! IsTokenSeparator(line[x])); x++) {
		vgen[y] = line[x];
		y++;
	}
	vgen[y] = 0;
	y = 0;
	for(x=x+1; (x < max) && (! IsTokenSeparator(line[x])); x++) {
		bstate[y] = line[x];
		y++;
	}
	bstate[y] = 0;
	obit->killer = ConvertObitKillerSpec(killer);
	obit->weapon = ConvertObitWeaponSpec(weapon);
	obit->power = ConvertObitPowerSpec(power);
	obit->kgender = ConvertGenderSpec(kgen);
	obit->vgender = ConvertGenderSpec(vgen);
	obit->bodystate = ConvertObitBodyStateSpec(bstate);

	ComplainInvalidServObitInput(obit->killer, line, "KillerType", killer, line_no);
	ComplainInvalidServObitInput(obit->weapon, line, "WeaponType", weapon, line_no);
	ComplainInvalidServObitInput(obit->power, line, "PowerType", power, line_no);
	ComplainInvalidServObitInput(obit->kgender, line, "KillerGender", kgen, line_no);
	ComplainInvalidServObitInput(obit->vgender, line, "VictimGender", vgen, line_no);
	ComplainInvalidServObitInput(obit->bodystate,line, "BodyState", bstate, line_no);
	return (obit);
}

void ParseWelcomeDisplayTime (char *str)
{
	ServObit.WelcomeDisplayTime = 5;
	if (! (sscanf(str, "%d", &ServObit.WelcomeDisplayTime) == 1))
	{
		ServObit.WelcomeDisplayTime = 5;
		return;
	}
	if (ServObit.WelcomeDisplayTime <= 0)
		ServObit.WelcomeDisplayTime = 2;
	else if (ServObit.WelcomeDisplayTime >= 999)
		ServObit.WelcomeDisplayTime = 999;
}

int ServObitCommandParse (char *str)
{
	//Assume : has been stripped off.
	if (str[0] == ' ')
		return(SO_PARSE_OBIT_ENTRY);
	else if (StrBeginsWith("End", str))
		return(SO_PARSE_END);
	else if (StrBeginsWith("Welcome", str))
		return(SO_PARSE_WELCOME);
	else if (StrBeginsWith("Connect", str))
		return(SO_PARSE_CONNECT);
	else if (StrBeginsWith("Disconnect", str))
		return(SO_PARSE_DISCONNECT);
	else if (StrBeginsWith("Substitutions", str))
		return(SO_PARSE_SUBSTITUTION);
	else if (StrBeginsWith("Include", str))
		return(SO_PARSE_INCLUDE);
	else return(SO_PARSE_ERROR);
}

/* **************************************************************
   Obituary Matching and Construction
   ************************************************************** */


int MatchObituary(struct Obituary *obit, int ktype, int kgen, int vgen,
					int weapon, int power, int bstate)
{

	// There are all these checks for nonzero values so that if any parse
	// errors occurred (resulting in 0), they are treated like wildcards.

	// Optimize the rest of these by:
	// (a) cardinality of the set of possible values
	// (b) expected percentage of NON wildcards

	// The idea is that very few obituary selectors will match the situation.
	// Therefore, we want to fail this test ASAP.

	// Optimization: unless a server op does REALLY weird things like selecting
	// primarily on something other than weapons, the weapon check is most
	// likely to return failure first.

	if (obit->weapon && (obit->weapon != SO_DEATH_ANY)
		&& (weapon != obit->weapon))
		return(0);

	// Killer type is likely not going to be a wildcard, so will fail
	// sooner than the other select fields.
	if (obit->killer && (! (obit->killer == SO_KILLER_ANY))) {
		if (obit->killer != ktype) return(0);
	}

	// The rest are all basically the same here, but if you're a modder looking
	// to change this code, you may want to muck around.
	if (obit->kgender && (! (obit->kgender == GENDER_ANY))) {
		if (obit->kgender != kgen) return(0);
	}
	if (obit->vgender && (! (obit->vgender == GENDER_ANY))) {
		if (obit->vgender != vgen) return(0);
	}
	if (obit->bodystate && (! (obit->bodystate == SO_BODY_STATE_ANY))) {
		// e.g.: if the obit says "GIB" and you're megagib, you match;
		// but if obit says "MEGA" and you're normal, you don't match
		if (! BodyStateSameOrWorseThan(bstate, obit->bodystate))
			return(0);
	}
	if (obit->power && (! (obit->power == SO_POWER_ANY))) {
		if (obit->power != power) return(0);
	}
	return(1);
}

char *FormatObituaryMessage(char *victim, char *killer, char *format, int kgender,
							 int vgender, int power, int bodystate)
{
	static char line[512], obit[512];
	char *str;
	int x = 0, y = 0, len = 0, tmp=0, len2;

	strcpy(obit, format);
	PerformSubstitutions(obit);
	while (obit[y]) 
	{
		if (! (obit[y] == '$' || obit[y] == '%'))
		{
			line[x++] = obit[y++];
			continue;
		}
		else if (StrBeginsWith("$V", obit+y))
		{
			len = strlen(victim);
			strncpy(line+x, victim, len);
			x = x + len;
			y = y + 2;
		}
		else if (StrBeginsWith("$K", obit+y))
		{
			len = strlen(killer);
			strncpy(line+x, killer, len);
			x = x + len;
			y = y + 2;
		}
		else if (StrBeginsWith("$H", obit+y))
		{
			str = "ERROR";
			if (StrBeginsWith("$HISK", obit+y))
			{
				str = GenderToHisHer(kgender);
				len2 = 5;
			}
			else if (StrBeginsWith("$HISV", obit+y))
			{
				str = GenderToHisHer(vgender);
				len2 = 5;
			}
			else if (StrBeginsWith("$HIMK", obit+y))
			{
				str = GenderToHimHer(kgender);
				len2 = 5;
			}
			else if (StrBeginsWith("$HIMV", obit+y))
			{
				str = GenderToHimHer(vgender);
				len2 = 5;
			}
			else if (StrBeginsWith("$HERSELFV", obit+y))
			{
				str = GenderToHimselfHerself(vgender);
				len2 = 9;
			}
			else if (StrBeginsWith("$HERSELFK", obit+y))
			{
				str = GenderToHimselfHerself(kgender);
				len2 = 9;
			}
			else {
				line[x++] = obit[y++];
			}
			len = strlen(str);
			strncpy(line+x, str, len);
			x = x + len;
			y = y + len2;
		}
		else if (StrBeginsWith("$SHE", obit+y))
		{
			str = "ERROR";
			if (StrBeginsWith("$SHEK", obit+y))
				str = GenderToSheHe(kgender);
			else
				str = GenderToSheHe(vgender);
			strncpy(line+x, str, strlen(str));
			x = x + strlen(str);
			y = y + 5;
		}
		else if ((obit[y+1] == 'G') || (obit[y+1] == 'g'))
		{
			// gibbed or mega-gibbed
			if (bodystate == SO_BODY_STATE_MEGAGIB)
				str = "mega-gibbed";
			else if (bodystate == SO_BODY_STATE_GIB)
				str = "gibbed";
			else str = "destroyed";
			strncpy(line+x, str, strlen(str));
			x = x + strlen(str);
			y = y + 2;
		}
		else if ((obit[y + 1] == 'Q') || (obit[y+1] == 'q'))
		{
			if (power == SO_POWER_QUAD)
			{
				// NOTE TO STEQVE (and others): LEAVE THE SPACE IN THIS STRING,
				// FOR THE LAST TIME, DAMMIT.  Otherwise, if the obit went:
				// "$V killed $K with $Q weapon" and there was no Quad, there 
				// would be 2 spaces: "$V killed $K with  weapon".  A minor
				// point, but there it is.
				strncpy(line+x, "Quad ", 5);
				x = x + 5;
			}
			y = y + 2;
		}
	}
	line[x] = 0;
	FormatBoldString(line);
	return(line);
}


/* ******************************************************************
   Main Functions
   ******************************************************************
*/

void ReadServerObits ()
{
	FILE *in;
	char line[SO_MAX_OBIT_LENGTH], path[256];
	int ind = -1, line_no, new_mode, process_mode, subind;
	int parse_error = 0, num_signons=0, num_signoffs=0;
	char welcome[SO_MAX_WELCOME_SIZE], *str;
	struct ServObitSubstitution *cursub;

	//DPRINT1("*** %s ***\n  (steqve@shore.net)\n", SERVOBIT_VERSION);
	//DPRINT0("  thanks to newtonD, EAVY,\n  Hawkeye, and Sati\n");
	//DPRINT0("  a.k.a. \"Queen of Death Messages\"\n");

//		PRINT0( "Starting to read server obits.\n");

	// bug of sorts: if operator is editing the file and it is locked,
	// and this function is called, the fopen will fail, and previously
	// loaded ServObits will be lost.
	ServObits[0] = NULL;
	ServObitSubstitutionList = NULL;

	WriteServObitPathname(path, "obits.txt");
	in = fopen(path, "r");
	if (in == NULL) 
	{
		DPRINT1("ServObit Error: COULD NOT OPEN SERVOBIT FILE %s!\n", path);
		PRINTLOG1("ERROR: Could not open %s!\n", path);
		//PRINT(PRINT_MEDIUM, "Couldn't open ServObits file!\n");
		return;
	}
	ind = -1;
	line_no = 0;
	subind = 0;
	cursub = NULL;
	while ((fgets(line, SO_MAX_OBIT_LENGTH, in) != NULL) &&
		   ind < ServObit.MaxSelectors)
	{
		line_no++;
		// Trim end-of-line
		line[strlen(line) - 1] = 0;
		// Strip out comments
		if (line[0] == '#')
			continue;
		// Now check out the line.
		if (strlen(line)) {
			if ((line[0] == ':')) {
				// parse ServObit Command
				new_mode = ServObitCommandParse(line + 1);
				if (new_mode == SO_PARSE_ERROR)
				{
					PRINTLOG1("ERROR: Couldn't understand command at line %d.\n", line_no);
					PRINTLOG1("-->%s\n", line);
					ServObit.ParseErrors++;
					new_mode = SO_PARSE_NEUTRAL;
				}
				else if (new_mode == SO_PARSE_OBIT_ENTRY)
				{
					ind++;
					ServObits[ind] = ParseObitLine(line + 2, line_no);
				}
				else if (new_mode == SO_PARSE_SUBSTITUTION)
				{
					// nothing doing
				}
				else if (new_mode == SO_PARSE_WELCOME) {
					ServObit.WelcomeDisplayTime = 5;
					if (strlen(line) > 8) {
						ParseWelcomeDisplayTime(line + 8);
					}
					else {
						PRINTLOG0("Warning: Welcome will last default 5 seconds\n");
					}
					ServObit.Welcome = "";
					welcome[0] = 0;
				}
				else if (new_mode == SO_PARSE_END)
					new_mode = SO_PARSE_NEUTRAL;

				process_mode = new_mode;
			}
			else if (process_mode == SO_PARSE_SUBSTITUTION)
			{
				if (line[0] == '[')
				{
					str = strchr(line+1, ']');
					if (str != NULL)
						// truncate line at ]
						*str = '\0';
					cursub = MakeServObitSubstitution(strdup(line+1), cursub);
					if (ServObitSubstitutionList == NULL)
					{
						ServObitSubstitutionList = cursub;
					}
				}
				else {
					if (cursub == NULL)
					{
						PRINTLOG1("ERROR: line %d: Substitution entry without definition\n",
								   line_no);
						PRINTLOG1("---> %s\n", line);
						ServObit.ParseErrors++;
					}
					else if (cursub->NumValues >= ServObit.MaxObitMessages)
					{
						PRINTLOG2("ERROR: line %d: Substitution exceeded max %d per definition\n",
								  line_no, ServObit.MaxObitMessages);
						PRINTLOG0("       Set MaxObitMessages in ServObit.ini to a higher number\n");
						PRINTLOG1("---> %s\n", line);
						ServObit.ParseErrors++;
					}
					else
					{
						cursub->values[cursub->NumValues++] = strdup(line);
					}
				}
			}
			else if (process_mode == SO_PARSE_CONNECT) {
				if (num_signons == ServObit.MaxConnectMessages) {
					PRINTLOG1("ERROR: line %d; Exceeded maximum number of Connect messages\n",
						line_no);
					PRINTLOG1("---> %s\n", line);
					ServObit.ParseErrors++;
				}
				else {
					ServObit.ConnectMessages[num_signons++] = strdup(line);
					ComplainInvalidServObitConnectMessageSpecifiers (line, line_no);
				}
			}
			else if (process_mode == SO_PARSE_DISCONNECT) {
				if (num_signoffs == ServObit.MaxConnectMessages) {
					PRINTLOG1("ERROR: line %d; Exceeded maximum number of Disconnect messages\n",
							line_no);
					PRINTLOG1("---> %s\n", line);
					ServObit.ParseErrors++;
				}
				else {
					ServObit.DisconnectMessages[num_signoffs++] = strdup(line);
					ComplainInvalidServObitConnectMessageSpecifiers (line, line_no);
				}
			}
			else if (process_mode == SO_PARSE_WELCOME) {
				// Stick the newline back into the message.
				line[strlen(line)] = '\n';
				line[strlen(line)] = 0;
				if ((strlen(welcome) + strlen(line)) >= SO_MAX_WELCOME_SIZE) {
					PRINTLOG1("ERROR: line %d; Exceeded maximum number of characters in welcome\n",
						line_no);
					PRINTLOG1("---> %s\n", line);
					ServObit.ParseErrors++;
				}
				else {
					strcpy(welcome + strlen(welcome), line);
				}
			}
			else if (process_mode == SO_PARSE_OBIT_ENTRY) {
					AddMessageToServObit(ServObits[ind], strdup(line), line_no);
				}
			else {
				ServObit.ParseErrors++;
				PRINTLOG1("ERROR: ignoring non-comment line %d; expecting command\n",
					line_no);
				PRINTLOG1("--> %s\n", line);
			}
		}
	}
	if (ind == ServObit.MaxSelectors) {
		//gi.dprintf("ServObit Error: Exceeded Max Obits %d in line %d\n", SO_MAX_OBITS, line_no);
		PRINTLOG2("ServObit Error: Exceeded Max Obits %d in line %d\n", SO_MAX_OBIT_SELECTORS, line_no);
		//DPRINT2("ServObit Error: Exceeded Max Obits %d in line %d\n", SO_MAX_OBIT_SELECTORS, line_no);
		ServObits[ServObit.MaxSelectors-1] = NULL;
		ServObit.ParseErrors++;
	}
	else {
		ServObits[ind] = NULL;
	}
	fclose(in);  // Error checking?  What's that?

	ServObit.ConnectMessageCount = num_signons;
	ServObit.DisconnectMessageCount = num_signoffs;
	ServObit.Welcome = strdup(welcome);

	if (ServObit.ParseErrors)
		DPRINT1("\n\n\n\nServObit ERROR: Found %d parse errors.\nCheck SrbObLog.txt\n\n\n\n",
				ServObit.ParseErrors);

	DPRINT0("ServObit loaded.\n");
}

// DESTRUCTIVE.  Destroys format and writes into mess.

// This should be optimized.
void FormatConnectMessage (char *format, char *mess, char *name, int gender)
{

	// Perform substitutions on the format string
	PerformSubstitutions(format);

	// Successive calls to ServObitInsertValue basically turn the previously
	// generated message into the format string for the current one.
	//
	// This is WAYYYY inefficient.
	ServObitInsertValue(mess, format, "$N", name, MAX_STRING_CHARS);
	strcpy(format, mess);
	ServObitInsertValue(mess, format, "$HIM", GenderToHimHer(gender),
						MAX_STRING_CHARS);
	strcpy(format, mess);
	ServObitInsertValue(mess, format, "$HIS", GenderToHisHer(gender),
						MAX_STRING_CHARS);
	strcpy(format, mess);
	ServObitInsertValue(mess, format, "$SHE", GenderToSheHe(gender),
						MAX_STRING_CHARS);
	strcpy(format, mess);
	ServObitInsertValue(mess, format, "$HERSELF", GenderToHimselfHerself(gender),
						MAX_STRING_CHARS);
	FormatBoldString(mess);
	// make absolutely sure mess has a carriage return as last char
	mess[MAX_STRING_CHARS - 2] = '\n';
	mess[MAX_STRING_CHARS - 1] = 0;	
}


// This implements a really really fancy search algorithm called "Linear 
// Search."  (For you beginners, that's the slowest yet easiest search to
// code.)
//
// This could be a pain to optimize because of the wildcards and the
// "guarantee" that the first matching Selector will be used.  NewtonD
// mentioned binary search trees, which would be cool, but how do you
// decide which characteristic to branch on first?  The optimization
// will need to be designed based on the individual server operator's
// preferences.  One operator may care about Quad more than gender,
// and vice versa.
int DisplayObituary(char *victim, char *killer, int killertype, int kgen, int vgen,
					int weapon, int power, int bstate)
{
	int i = 0, x;
	char *text, *randmessage;

	for (i=0; ServObits[i] != NULL; i++)
	{
		if (MatchObituary(ServObits[i], killertype, kgen, vgen, weapon, power, bstate))
		{
			if (ServObits[i]->MessageSize) {
				x = rand() % ServObits[i]->MessageSize;
				randmessage = (ServObits[i])->Messages[x];
				text = FormatObituaryMessage(victim, killer, randmessage, kgen, vgen, 
											power, bstate);
				BPRINT1("%s\n", text);
				return(2);
			}
			// Otherwise, we *should* have matched on this but there were no
			// messages available, so go to the next best obit.
		}
	}

	// Generic Obituaries - just in case nothing matches.
	if (killertype == SO_KILLER_ENEMY) {
		BPRINT2("%s killed %s\n", killer, victim);
	}
	else if (killertype == SO_KILLER_SELF) {
		BPRINT1("%s decided to end it all\n", victim);
	}
	else if (killertype == SO_KILLER_ENVIRONMENT) {
		BPRINT1("%s became one with the map\n", victim);
	}
	else {
		// Hell if I know, he's just dead!
		// [that was my favorite comment I saw in the old Quake C code... SteQve]
		BPRINT1("%s died for unknown reasons\n", victim);
	}
	return(1);
}



void ReadServObitInitFile ()
{
	FILE *in;
	char line[SO_MAX_OBIT_LENGTH], path[256], field[SO_MAX_OBIT_LENGTH];
	int pos, val;

	WriteServObitPathname(path, "ServObit.ini");
	in = fopen(path, "r");
	if (in == NULL)
	{
		DPRINT1("ServObit Error: Couldn't open %s.\n                Using default values.\n", path);
		return;
	}
	while (fgets(line, SO_MAX_OBIT_LENGTH, in) != NULL)
	{
		line[strlen(line) -1] = '\0';
		if (line[0] == '#')
			continue;
		if (line[0] == '\0')
			continue;
		pos = strcspn(line,"=");
		if (line[pos] == '\0') continue;
		line[pos] = 0;
		strcpy(field, line);
		val = atoi(line + pos + 1);
		if (Q_stricmp(field, "MaxConnectMessages") == 0)
		{
			if (val < 5)
			{
				PRINTLOG0("ServObit Warning: Set MaxConnectMessages to 5\n");
				val = 5;
			}
			ServObit.MaxConnectMessages = val;
		}
		else if (Q_stricmp(field, "MaxSelectors") == 0)
		{
			if (val < 500)
			{
				PRINTLOG0("ServObit Warning: Set MaxSelectors to 500\n");
				val = 500;
			}
			ServObit.MaxSelectors = val;
		}
		else if (Q_stricmp(field, "MaxObitMessages") == 0)
		{
			if (val < 10)
			{
				PRINTLOG0("ServObit Warning: Set MaxObitMessages to 10\n");
				val = 10;
			}
			ServObit.MaxObitMessages = val;
		}
		else if (Q_stricmp(field, "MaxSubstitutions") == 0)
		{
			if (val < 5)
			{
				PRINTLOG0("ServObit Warning: Set MaxSubstitutions to 5\n");
				val = 5;
			}
			ServObit.MaxSubstitutions = val;
		}
	}
}

void InitServObitData ()
{

	// Get info that could (eventually) be set by server op.
	ServObit.MaxConnectMessages = SO_MAX_CONNECTS;
	ServObit.MaxObitMessages = SO_MAX_OBIT_MESSAGES;
	ServObit.WelcomeDisplayTime = 5;
	ServObit.Welcome = "ServObit 1.4";
	ServObit.MegaGib = SO_MEGAGIB_MINIMUM;
	ServObit.MaxSelectors = SO_MAX_OBIT_SELECTORS;
	ServObit.MaxSubstitutions = SO_MAX_SUBSTITUTIONS;

	ReadServObitInitFile();

	// Allocate memory for data structures
	ServObit.ConnectMessages = (char **)malloc(ServObit.MaxConnectMessages * sizeof(char *));
	ServObit.DisconnectMessages = (char **)malloc(ServObit.MaxConnectMessages * sizeof(char *));

	// WOW!  Said the sharp eyed C programmers.  This guy's not being the most
	// efficient with memory, is he?  See his Lisp (we don't care about memory,
	// we can always buy more) bias in action!
	ServObits = (struct Obituary **)malloc(sizeof(struct Obituary *) *
											ServObit.MaxSelectors);

	ServObit.DisconnectMessageCount = -1;
	ServObit.ConnectMessageCount = -1;
	ServObit.ParseErrors = 0;

}

void InitializeServObit ()
{
	char path[256];

	//ServObit.Path = "ServObit/";
		
	WriteServObitPathname(path, "SrvObLog.txt");
	
	LOG = fopen(path, "w");
	if (LOG == NULL) {
		DPRINT1("\n\n\n\nServObit ERROR: Could not open log file %s!\n\n\n\n", path);
		return;
	}

	InitServObitData();
	ReadServerObits();

	fclose(LOG); // Error checking?  What's that?

}
