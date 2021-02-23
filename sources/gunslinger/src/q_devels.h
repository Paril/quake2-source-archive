//
// QDEVELS.H - cool support functions for Quake II development
// Version 1.5 (last updated Jan 29, 1998)
// 
// Published at http://www.planetquake.com/qdevels
// Code by various authors, released by SumFuka@planetquake.com
//
// Please browse qdevels.c for function descriptions.


//#define for_each_player(JOE_BLOGGS,INDEX) for(INDEX=1;INDEX<=maxclients->value;INDEX++) if ((JOE_BLOGGS=&g_edicts[i]) && JOE_BLOGGS->inuse)
#define for_each_player(JOE_BLOGGS,INDEX) for(INDEX=1;INDEX<=maxclients->value +1;INDEX++) if ((JOE_BLOGGS=&g_edicts[INDEX]) && JOE_BLOGGS->inuse)

#define rndnum(y,z) ((random()*((z)-((y)+1)))+(y))

void stuffcmd(edict_t *e, char *s);
edict_t *ent_by_name (char *target);
void centerprint_all (char *msg);



// 7. Item lists by smith57@airmail.net
// Ugh. Inaccurate! -- Stone

#define ITEMLIST_NULLINDEX			0
#define ITEMLIST_BODYARMOR			1
#define ITEMLIST_COMBATARMOR		2
#define ITEMLIST_JACKETARMOR		3
#define ITEMLIST_ARMORSHARD			4
#define ITEMLIST_POWERSCREEN		5
#define ITEMLIST_POWERSHIELD		6
#define ITEMLIST_BLASTER			7
#define ITEMLIST_REVOLVER			8
#define ITEMLIST_DOUBLEREVOLVER		9
#define ITEMLIST_COACHGUN			10
#define ITEMLIST_PUMPACTIONSHOTGUN	11
#define ITEMLIST_GRENADELAUNCHER	12
#define ITEMLIST_ROCKETLAUNCHER		13
#define ITEMLIST_WINCHESTER_RIFLE	14
#define ITEMLIST_SHARPS_RIFLE		15
#define ITEMLIST_CANNON				16
#define ITEMLIST_SHELLS				17
#define ITEMLIST_BULLETS45LC		18
#define ITEMLIST_BULLETS3030		19
#define ITEMLIST_COCKTAILS			20
#define ITEMLIST_HATCHETS			21
#define ITEMLIST_BULLETS4570		22
#define ITEMLIST_QUADDAMAGE			23
#define ITEMLIST_INVULNERABILITY	24
#define ITEMLIST_SILENCER			25
#define ITEMLIST_REBREATHER			26
#define ITEMLIST_ENVIRONMENTSUIT	27
#define ITEMLIST_ANCIENTHEAD		28
#define ITEMLIST_ADRENALINE			29
#define ITEMLIST_BANDOLIER			30
#define ITEMLIST_AMMOPACK			31
#define ITEMLIST_DATACD				32
#define ITEMLIST_POWERCUBE			33
#define ITEMLIST_PYRAMIDKEY			34
#define ITEMLIST_DATASPINNER		35
#define ITEMLIST_SECURITYPASS		36
#define ITEMLIST_BLUEKEY			37
#define ITEMLIST_REDKEY				38
#define ITEMLIST_COMMANDERSHEAD		39
#define ITEMLIST_AIRSTRIKEMARKER	40
#define ITEMLIST_HEALTH				41
