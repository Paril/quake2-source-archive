#define SERVOBIT_VERSION "ServObit 1.4 4/8/98"

#ifndef SERVOBIT_DEBUG
#define SERVOBIT_DEBUG 0
#endif

// Weapons
// ** NOTE ** SO_WEAPON_START and SO_WEAPON_END are ADMINISTRATIVE
// constants, used to QUICKLY determine that weapon created the death.  You
// MUST put ALL weapons in between these two values
#define SO_WEAPON_START 1
#define SO_WEAPON_BLASTER 2
#define SO_WEAPON_SHOTGUN 3
#define SO_WEAPON_SUPER_SHOTGUN 4
#define SO_WEAPON_MACHINEGUN 5
#define SO_WEAPON_CHAINGUN 6
#define SO_WEAPON_GRENADE_LAUNCHER 7
#define SO_WEAPON_GRENADE_SPLASH 8
#define SO_WEAPON_HAND_GRENADE 9
#define SO_WEAPON_HAND_GRENADE_SPLASH 10
#define SO_WEAPON_HAND_GRENADE_HELD 11
#define SO_WEAPON_ROCKET_LAUNCHER 12
#define SO_WEAPON_ROCKET_LAUNCHER_SPLASH 13
#define SO_WEAPON_HYPER_BLASTER 14
#define SO_WEAPON_RAILGUN 15
#define SO_WEAPON_BFG_LASER 16
#define SO_WEAPON_BFG_EFFECT 17
#define SO_WEAPON_BFG_BLAST 18
#define SO_WEAPON_TELEFRAG 19
#define SO_WEAPON_ENEMY 20
#define SO_WEAPON_SUICIDE 21
#define SO_WEAPON_ANY 22
#define SO_WEAPON_END 23

// Environmental "weapons"

// Leave "room" for more weapon declarations, so start at 50
#define SO_ENV_START 50
#define SO_ENV_LAVA 51
#define SO_ENV_SLIME 52
#define SO_ENV_WATER 53
#define SO_ENV_FALL 54
#define SO_ENV_SQUISH 55
#define SO_ENV_TOUCH 56
#define SO_ENV_EXIT 57
#define SO_ENV_LASER 58
#define SO_ENV_ANY 59
#define SO_ENV_END 60

// "Interpret" SO_DEATH_ANY as (SO_WEAPON_ANY or SO_ENV_ANY)
#define SO_DEATH_ANY 80

// Power Types

#define SO_POWER_ANY 1
#define SO_POWER_QUAD 2
#define SO_POWER_NORMAL 3

// Killer Types

#define SO_KILLER_ANY 1
#define SO_KILLER_SELF 2
#define SO_KILLER_ENEMY 3
#define SO_KILLER_ENVIRONMENT 4

// Body State Types

// Body States must come in this order to help 
// "rank" body states; e.g. a megagib is worse than a gib
#define SO_BODY_STATE_NORMAL 1
#define SO_BODY_STATE_GIB 2
#define SO_BODY_STATE_MEGAGIB 3
#define SO_BODY_STATE_ANY 4

// Parser States Types

// There's also a parse_obit_selector state, but it's only for a single
// line
#define SO_PARSE_CONNECT 1
#define SO_PARSE_DISCONNECT 2
#define SO_PARSE_OBIT_ENTRY 3
#define SO_PARSE_WELCOME 4
#define SO_PARSE_NEUTRAL 5
#define SO_PARSE_END 6
#define SO_PARSE_ERROR 7
#define SO_PARSE_SUBSTITUTION 8
#define SO_PARSE_INCLUDE 9

// These limit size and number of obits and their parts.

// Do not set this to less than MAX_STRING_CHARS.  I imagine you're also
// taking a risk with something greater than MAX_STRING_CHARS.  This seems
// to be the max that Quake can handle.

#define SO_MAX_WELCOME_SIZE MAX_STRING_CHARS
#define SO_MAX_OBIT_LENGTH 256
#define SO_MAX_OBIT_SELECTORS 1000
#define SO_MAX_OBIT_MESSAGES 20
#define SO_MAX_CONNECTS SO_MAX_OBIT_MESSAGES
#define SO_MAX_SUBSTITUTIONS 5

// When the victim has health less than -80, they're mega-gibbed.
// Don't make this greater than -40, since -40 is the health at
// which gibbing occurs.
#define SO_MEGAGIB_MINIMUM -80

// This should be called Selector, but I came up with that name long after
// most of the code was written, and I knew *you'd* understand why it's
// not particularly important to change it.
struct Obituary {
	int killer, weapon, power, kgender, vgender, bodystate;
	char **Messages;
	int MessageSize;
};

// *** MONSTROUS waste of space!!! ***//
struct ServObitSubstitution {
	char *key;
	char **values;
	int NumValues;
	struct ServObitSubstitution *next;
};

struct ServObitGameState {
	int MaxMeansOfDeath;
	int *MeansOfDeathMap;
	int MaxObitMessages;
	int MaxConnectMessages;
	int MaxSelectors;
	int MaxSubstitutions;
	int WelcomeDisplayTime;
	char **ConnectMessages, **DisconnectMessages;
	int ConnectMessageCount, DisconnectMessageCount;
	char *Welcome;
	int MegaGib;
	char *Path;
	int ParseErrors;
};


extern struct ServObitGameState ServObit;



void ServObitInitGame ();
void PerformSubstitutions (char *orig);
int ServObitInsertValue(char *mess, char *format_str, char *field, char *value, int max_size)
;
void FormatConnectMessage (char *format, char *mess, char *name, int gender);

int DisplayObituary(char *killed, char *killer, int killertype, int killergender,
					int victimgender, int weapon, int power, int bodystate);
int IsEnvironmentWeapon (int wtype);
int IsEnemyWeapon (int wtype);
void InitializeServObit();

// I use these macros to make it easy to switch between gi.bprintf and regular
// printf - gi.bprintf seems to crash when I run "outside" the engine.

#define PRINTLOG0(m) {fprintf(LOG, m); fflush(LOG);}
#define PRINTLOG1(m, a) {fprintf(LOG, m, a); fflush(LOG);}
#define PRINTLOG2(m, a, b) {fprintf(LOG, m, a, b); fflush(LOG);}
#define PRINTLOG3(m, a, b, c) {fprintf(LOG, m, a, b, c); fflush(LOG);}
#define PRINTLOG4(m, a, b, c, d) {fprintf(LOG, m, a, b, c, d); fflush(LOG);}
#define PRINTLOG6(m, a, b, c, d, e, f) {fprintf(LOG, m, a, b, c, d, e, f); fflush(LOG);}

