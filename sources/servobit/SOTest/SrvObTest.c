/* SrvObTest 0.1 - Test program for ServObit 1.4 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "external.h"
#include "gender.h"

#include "ModUtils.h"
#include "ServObits.h"

extern FILE *LOG;
extern struct Obituary **ServObits;
int MatchObituary(struct Obituary *obit, int ktype, int kgen, int vgen,
					int weapon, int power, int bstate);
char *FormatObituaryMessage(char *victim, char *killer, char *format, int kgender,
							 int vgender, int power, int bodystate);


int ConvertObitPowerSpec (char * power);
int ConvertObitBodyStateSpec (char * bstate);
int ConvertObitKillerSpec (char * ktype);
int ConvertObitWeaponSpec (char *weapon);
int IsEnvironmentWeapon (int wtype);


int TestRepeats;
int TestKillerType;
int TestKillerGender;
int TestVictimGender;
int TestPower;
int TestBodyState;
int TestWeaponType;
//int TestAll;
//int TestOthers;

char **PrintedObits;

char * WeaponTypeToName (int w)
{
	//printf("Weapon Spec %s\n", weapon);
	if (w == SO_WEAPON_SUPER_SHOTGUN)
		return("Super Shotgun");
	else if (w == SO_WEAPON_SHOTGUN)
		return("Shotgun");
	else if (w == SO_WEAPON_BFG_BLAST)
		return("BFG Blast");
	else if (w == SO_WEAPON_BFG_LASER)
		return("BFG laser");
	else if (w == SO_WEAPON_BFG_EFFECT)
		return("BFG effect");
	else if (w == SO_WEAPON_BLASTER)
		return("Blaster");
	else if (w == SO_WEAPON_HYPER_BLASTER)
		return("Hyper Blaster");
	else if (w == SO_WEAPON_HAND_GRENADE)
		return("Hand Grenade");
	else if (w == SO_WEAPON_HAND_GRENADE_SPLASH)
		return("Hand Grenade Splash");
	else if (w == SO_WEAPON_HAND_GRENADE_HELD)
		return("Hand Grenade Held");
	else if (w == SO_WEAPON_GRENADE_LAUNCHER)
		return("Grenade Launcher");
	else if (w == SO_WEAPON_GRENADE_SPLASH)
		return("Grenade Splash");
	else if (w == SO_WEAPON_ROCKET_LAUNCHER)
		return("Rocket Launcher");
	else if (w == SO_WEAPON_ROCKET_LAUNCHER_SPLASH)
		return("Rocket Launcher Splash");
	else if (w == SO_WEAPON_RAILGUN)
		return("Railgun");
	else if (w == SO_WEAPON_CHAINGUN)
		return("Chaingun");
	else if (w == SO_WEAPON_MACHINEGUN)
		return("Machinegun");
	else if (w == SO_WEAPON_TELEFRAG)
		return("Telefrag");
	else if (w == SO_ENV_LAVA)
		return("Lava");
	else if (w == SO_ENV_LASER)
		return("Laser");
	else if (w == SO_ENV_EXIT)
		return("Exit");
	else if (w == SO_WEAPON_ENEMY)
		return("Any Enemy Weapon");
	else if (w == SO_ENV_WATER)
		return("Water");
	else if (w == SO_ENV_SLIME)
		return("Slime");
	else if (w == SO_WEAPON_SUICIDE)
		return("Suicide");
	else if (w == SO_ENV_FALL)
		return("Fall");
	else if (w == SO_ENV_TOUCH)
		return("Touch");
	else if (w == SO_ENV_SQUISH)
		return("Squish");
	else if (w == SO_ENV_ANY)
		return("Any Environment");
	else return("UNKNOWN WEAPON");
}

char * PowerToName (int p)
{
	if (p == SO_POWER_QUAD)
		return("Quad");
	else if (p == SO_POWER_NORMAL)
		return("Normal");
	else if (p == SO_POWER_ANY)
		return("Any Power");
	else return("UNKNOWN POWER");
}

char * BodyStateToName (int b)
{
	if (b == SO_BODY_STATE_NORMAL)
		return("Normal");
	else if (b == SO_BODY_STATE_GIB)
		return("Gib");
	else if (b == SO_BODY_STATE_MEGAGIB)
		return("Megagib");
	else if (b == SO_BODY_STATE_ANY)
		return("Any Body State");
	else
		return("UNKNOWN BODY STATE");
}

char * KillerTypeToName (int k)
{
	if (k == SO_KILLER_ENEMY)
		return("Enemy");
	else if (k == SO_KILLER_SELF)
		return("Self");
	else if (k == SO_KILLER_ENVIRONMENT)
		return("Environment");
	else if (k == SO_KILLER_ANY)
		return("Any Killer Type");
	else return("UNKNOWN KILLER TYPE");
}


char *GetObituaryMessage(char *victim, char *killer, int killertype, int kgen, int vgen,
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
				return(text);
			}
			// Otherwise, we *should* have matched on this but there were no
			// messages available, so go to the next best obit.
		}
	}

	// Generic Obituaries - just in case nothing matches.
	if (killertype == SO_KILLER_ENEMY) {
		return("*** ERROR: Diagnosed enemy kill, but no obit match found.\n");
	}
	else if (killertype == SO_KILLER_SELF) {
		return("*** ERROR: Diagnosed suicide, but no obit match found.\n");
	}
	else if (killertype == SO_KILLER_ENVIRONMENT) {
		return("*** ERROR: Diagnosed environmental death, but no obit match found.\n");
	}
	else {
		// Hell if I know, he's just dead!
		// [that was my favorite comment I saw in the old Quake C code... SteQve]
		return("*** ERROR: unrecognized death.\n");
	}
}


void do_body_state_test (int wep, int ktype, int kgen, int vgen, int power, int bstate)
{
	int i, j, found_repeat, PrintedEllipse;

	char *str, *mess;

	if (TestBodyState != SO_BODY_STATE_ANY) str = "*";
	else str = "";
	PRINTLOG3("    %s-->BodyState: %s (%d)\n", str, BodyStateToName(bstate), bstate);

	PrintedObits[0] = NULL;
	PrintedEllipse = 0;
	for(i=1; i<=TestRepeats; i++)
	{
		mess = GetObituaryMessage("Victim", "Killer", ktype, kgen, vgen, wep, 
								  power, bstate);
		found_repeat = 0;
		for(j=0; PrintedObits[j] != NULL; j++)
		{
			if (strcmp(PrintedObits[j], mess) == 0)
			{
				found_repeat = 1;
				if (! PrintedEllipse)
					PRINTLOG0("       ...\n");
				PrintedEllipse = 1;
				break;
			}
		}
		if (! found_repeat)
		{
			// GetObituaryMessage (and FormatObituaryMessage that it calls)
			// returns a pointer to a string.  Makes it easy to ALWAYS compare
			// properly in the strcmp() above.  So duplicate it so the strcmp()
			// isn't looking at the same address.
			PrintedObits[j] = strdup(mess);
			PrintedObits[j+1] = NULL;
			PRINTLOG1("       %s\n", mess);
		}
	}
	for (j=0; PrintedObits[j] != NULL; j++)
		free(PrintedObits[j]);
}

void do_power_test (int wep, int ktype, int kgen, int vgen, int power)
{

	char *str;

	if (TestPower != SO_POWER_ANY) str = "*";
	else str = "";
	PRINTLOG3("   %s-->Power: %s (%d)\n", str, PowerToName(power), power);

	if (TestBodyState == SO_BODY_STATE_ANY)
	{
		do_body_state_test(wep, ktype, kgen, vgen, power, SO_BODY_STATE_NORMAL);
		do_body_state_test(wep, ktype, kgen, vgen, power, SO_BODY_STATE_GIB);
		do_body_state_test(wep, ktype, kgen, vgen, power, SO_BODY_STATE_MEGAGIB);
	}
	else
	{
		do_body_state_test(wep, ktype, kgen, vgen, power, TestBodyState);
	}
}

void do_victim_gender_test (int wep, int ktype, int kgen, int vgen)
{

	char *str;

	if (TestVictimGender == GENDER_ANY) str = "*";
	else str = "";
	PRINTLOG3("  %s-->VictimGender: %s (%d)\n", str, GenderToName(vgen), vgen);

	if (TestPower == SO_POWER_ANY)
	{
		do_power_test(wep, ktype, kgen, vgen, SO_POWER_QUAD);
		do_power_test(wep, ktype, kgen, vgen, SO_POWER_NORMAL);
	}
	else
	{
		do_power_test(wep, ktype, kgen, vgen, TestPower);
	}
}

void do_killer_gender_test (int wep, int ktype, int kgen)
{

	char *str;

	if (TestKillerGender == GENDER_ANY) str = "*";
	else str = "";
	PRINTLOG3(" %s-->KillerGender: %s (%d)\n", str, GenderToName(kgen), kgen);


	if (TestVictimGender == GENDER_ANY)
	{
		do_victim_gender_test(wep, ktype, kgen, GENDER_MALE);
		do_victim_gender_test(wep, ktype, kgen, GENDER_FEMALE);
		do_victim_gender_test(wep, ktype, kgen, GENDER_NEUTER);
	}
	else
		do_victim_gender_test(wep, ktype, kgen, TestVictimGender);
}

void do_weapon_test (int wep, int ktype)
{

	PRINTLOG2("\n\n\n=== Weapon: %s (%d)===\n", WeaponTypeToName(wep), wep);

	if (TestKillerGender == GENDER_ANY)
	{
		do_killer_gender_test(wep, ktype, GENDER_MALE);
		do_killer_gender_test(wep, ktype, GENDER_FEMALE);
		do_killer_gender_test(wep, ktype, GENDER_NEUTER);
	}
	else
	{
		do_killer_gender_test(wep, ktype, TestKillerGender);
	}
}

int IsSuicideWeapon (int wep)
{
	return((wep == SO_WEAPON_BFG_BLAST) ||
		   (wep == SO_WEAPON_ROCKET_LAUNCHER_SPLASH) ||
		   (wep == SO_WEAPON_GRENADE_LAUNCHER) ||
		   (wep == SO_WEAPON_GRENADE_SPLASH) ||
		   (wep == SO_WEAPON_HAND_GRENADE) ||
		   (wep == SO_WEAPON_HAND_GRENADE_SPLASH) ||
		   (wep == SO_WEAPON_SUICIDE));
}

void do_suicide_test (int weapon)
{
	printf("Doing %s\n", WeaponTypeToName(weapon));
	do_weapon_test(weapon, SO_KILLER_SELF);
}

void do_obit_test ()
{
	int x;
	char *wname;

	wname = WeaponTypeToName(TestWeaponType);

	if (TestKillerType == SO_KILLER_ENEMY ||
		TestKillerType == SO_KILLER_ANY)
	{
		PRINTLOG0("\n\n+++ Deaths by another player +++\n\n");
		if (TestWeaponType == SO_WEAPON_ENEMY || TestWeaponType == SO_WEAPON_ANY)
		{
			for(x=SO_WEAPON_START+1; x<=SO_WEAPON_ENEMY; x++)
			{
				if (x == SO_WEAPON_ANY)
					continue;
				printf("Doing %s\n", WeaponTypeToName(x));
				do_weapon_test(x, SO_KILLER_ENEMY);
			}
		}
		else if (IsEnemyWeapon(TestWeaponType))
		{
			printf("Doing %s\n", wname);
			do_weapon_test(TestWeaponType, SO_KILLER_ENEMY);
		}
		else
		{
			printf("Not doing %s for enemy death\n", wname);
		}
	}
	if (TestKillerType == SO_KILLER_SELF ||
		TestKillerType == SO_KILLER_ANY)
	{
		PRINTLOG0("\n\n+++ Suicide Deaths +++\n\n");
		if (TestWeaponType == SO_WEAPON_ENEMY || TestWeaponType == SO_WEAPON_ANY)
		{
			do_suicide_test(SO_WEAPON_BFG_BLAST);
			do_suicide_test(SO_WEAPON_ROCKET_LAUNCHER_SPLASH);
			do_suicide_test(SO_WEAPON_GRENADE_LAUNCHER);
			do_suicide_test(SO_WEAPON_GRENADE_SPLASH);
			do_suicide_test(SO_WEAPON_HAND_GRENADE);
			do_suicide_test(SO_WEAPON_HAND_GRENADE_SPLASH);
			do_suicide_test(SO_WEAPON_SUICIDE);
		}
		else if (! IsSuicideWeapon(TestWeaponType))
		{
			printf("Not doing %s for suicide death\n", wname);
		}
		else {
			do_suicide_test(TestWeaponType);
		}
	}

	if (TestKillerType == SO_KILLER_ENVIRONMENT ||
		TestKillerType == SO_KILLER_ANY)
	{
		PRINTLOG0("\n\n+++ Environment Deaths +++\n\n");
		if (TestWeaponType == SO_ENV_ANY || TestWeaponType == SO_WEAPON_ANY)
			for(x=SO_ENV_START+1; x<SO_ENV_ANY; x++)
			{
		//		if (x == SO_ENV_ANY) continue;
				printf("Doing %s\n", WeaponTypeToName(x));
				do_weapon_test(x, SO_KILLER_ENVIRONMENT);
			}
		else if (! IsEnvironmentWeapon(TestWeaponType))
		{
			printf("Not doing %s for environmental death\n", wname);
		}
		else {
			do_weapon_test(x, SO_KILLER_ENVIRONMENT);
		}
	}
}

void do_connect_test ()
{
	char format_string[512], mess[512];
	int i;

	PRINTLOG0("\n\n+++ Connect Messages +++\n\n");
	for(i=1; i<=TestRepeats; i++)
	{
		if (ServObit.ConnectMessageCount > 0)
			strcpy(format_string, ServObit.ConnectMessages[rand() % ServObit.ConnectMessageCount]);
		else strcpy(format_string, "$N entered the game\n");
		FormatConnectMessage(format_string, mess, "Player", GENDER_MALE);
		PRINTLOG1("%s\n", mess);
	}
}

void do_disconnect_test ()
{
	char format_string[512], mess[512];
	int i;

	PRINTLOG0("\n\n+++ Disconnect Messages +++\n\n");
	for(i=1; i<=TestRepeats; i++)
	{
		if (ServObit.DisconnectMessageCount > 0)
			strcpy(format_string, ServObit.DisconnectMessages[rand() % ServObit.DisconnectMessageCount]);
		else strcpy(format_string, "$N entered the game\n");
		FormatConnectMessage(format_string, mess, "Player", GENDER_MALE);
		PRINTLOG1("%s\n", mess);
	}
}

void Usage ()
{
	printf("Usage: sotest [weapon|killertype|kgen|vgen|power|body|none|all] [repeats=NUM]\n");
	printf("none = just do error check, connect/disconnect messages\n");
	printf("weapon = type of weapon used (see ServObit.txt for values, or use *)\n");
	printf("killertype = type of killer (World, Enemy, Self, *)\n");
	printf("kgen = Killer Gender (Male, Female, Neuter, *)\n");
	printf("vgen = Victim Gender (Male, Female, Neuter, *)\n");
	printf("power = Power (Quad, Normal, *)\n");
	printf("body = Body State (Normal, Gib, Mega-gib, *)\n");
	printf("repeats = NUMber of messages to print for each situation\n");
}

char * GetArgValue (char *str)
{
	char *tmp;
	
	for (tmp = str; (*tmp != '\0') && (*tmp != '='); tmp++)
		;
	if (*tmp == '\0') return(NULL);
	else return(tmp + 1);
}

int main (int argc, char **argv)
{

	int i, res, j, TestNone, val, arg_error;
	char *str;

	srand(time(NULL));

	arg_error = 0;
	TestRepeats = 3;
	TestNone = 0;

	TestKillerType = SO_KILLER_ANY;
	TestKillerGender = GENDER_ANY;
	TestVictimGender = GENDER_ANY;
	TestPower = SO_POWER_ANY;
	TestBodyState = SO_BODY_STATE_ANY;
	TestWeaponType = SO_WEAPON_ANY;

	if (argc == 0)
	{
		Usage();
		return(1);
	}

	for(i=1; i<argc;i++)
	{
		str = GetArgValue(argv[i]);
		if (StrBeginsWith("kgen", argv[i]))
		{
			if (str == NULL)
			{
				printf("ERROR: kgen argument was not of form 'kgen=ARG'\n");
				arg_error = 1;
			}
			else
			{
				val = ConvertGenderSpec(str);
				if (val == GENDER_ERROR)
				{
					printf("ERROR: kgen= argument was not m, n, or f\n");
					arg_error = 1;
				}
				else TestKillerGender = val;
			}
		}
		else if (StrBeginsWith("vgen", argv[i]))
		{
			if (str == NULL)
			{
				printf("ERROR: vgen argument was not of form 'vgen=ARG'\n");
				arg_error = 1;
			}
			else
			{
				val = ConvertGenderSpec(str);
				if (val == GENDER_ERROR)
				{
					printf("ERROR: vgen= argument was not m, n, f, or *\n");
					arg_error = 1;
				}
				else TestVictimGender = val;
			}
		}
		else if (StrBeginsWith("power", argv[i]))
		{
			if (str == NULL)
				{
				printf("ERROR: power argument was not of form 'power=ARG'\n");
				arg_error = 1;
			}
			else
			{
				val = ConvertObitPowerSpec(str);
				if (val == 0)
				{
					printf("ERROR: power= argument was not q, n, or *\n");
					arg_error = 1;
				}
				else TestPower = val;
			}
		}
		else if (StrBeginsWith("bstate", argv[i]))
		{
			if (str == NULL)
			{
				printf("ERROR: bstate argument was not of form 'bstate=ARG'\n");
				arg_error = 1;
			}
			else
			{
				val = ConvertObitBodyStateSpec(str);
				if (val == 0)
				{
					printf("ERROR: bstate= argument was not n, g, m, or *\n");
					arg_error = 1;
				}
				else TestBodyState = val;
			}
		}
		else if (StrBeginsWith("killertype", argv[i]))
		{
			if (str == NULL)
			{
				printf("ERROR: killertype argument was not of form 'killertype=ARG'\n");
				arg_error = 1;
			}
			else
			{
				val = ConvertObitKillerSpec(str);
				if (val == 0)
				{
					printf("ERROR: killertype= argument was not e, s, w, or *\n");
					arg_error = 1;
				}
				else TestKillerType = val;
			}
		}
		else if (StrBeginsWith("weapon", argv[i]))
		{
			if (str == NULL)
			{
				printf("ERROR: weapon argument was not of form 'weapon=ARG'\n");
				arg_error = 1;
			}
			else
			{
				val = ConvertObitWeaponSpec(str);
				if (val == 0)
				{
					printf("ERROR: weapon= argument was not valid - see ServObit.txt for values\n");
					arg_error = 1;
				}
				else TestWeaponType = val;
			}
		}
		else if (Q_stricmp(argv[i], "all") == 0)
		{
			TestKillerType = SO_KILLER_ANY;
			TestKillerGender = GENDER_ANY;
			TestVictimGender = GENDER_ANY;
			TestPower = SO_POWER_ANY;
			TestBodyState = SO_BODY_STATE_ANY;
			TestWeaponType = SO_WEAPON_ANY;
		}
		else if (Q_stricmp(argv[i], "none") == 0)
		{
			TestNone = 1;
		}
		else if (StrBeginsWith("repeats", argv[i]))
		{
			res = sscanf(argv[i], "repeats=%d", &j);
			if (! (res == 1))
			{
				printf("Error: repeats must be of form 'repeats=NUMBER'\n");
				arg_error = 1;
			}
			else TestRepeats = j;
		}
		else
		{
			printf("Error: don't understand argument '%s'\n", argv[i]);
			arg_error = 1;
		}
	}
	if (arg_error)
	{
		Usage();
		return(1);
	}

	// Changed InitializeServObit() to use no path in ServObit.Path
	InitializeServObit();

	PrintedObits = (char **)malloc(sizeof(char *) * (TestRepeats + 1));

	PRINTLOG0("\n\n\nOBITUARY TEST RESULTS\n\n\n");

	do_connect_test();
	do_disconnect_test();
	
	if (TestNone) 
	{
		fclose(LOG);
		return(0);
	}

	do_obit_test();
	printf("*** Look in SrvObTst.txt for test results\n");
	fclose(LOG);
	return(0);
}
