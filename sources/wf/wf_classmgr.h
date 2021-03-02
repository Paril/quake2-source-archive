/*==============================================================================
The Weapons Factory - 
Class Manager Include File
Original code by Gregg Reno
==============================================================================*/

#define MAX_CLASSES 16

typedef struct 
{
	char name[32];
	char model_name[32];
	int  model;
	char skin1[32];
	char skin2[32];
	int  decoyskin;
	int  weapon[10];
	int  grenade[3];
	int  max_ammo[7];
	int  max_armor;
	int  max_speed;
	int  max_health;
	int  special;
	int  items;
	int  limit;     //Limit the number allowed for this class
} ClassInfo;

ClassInfo classinfo[MAX_CLASSES + 1];
char classdefname[32];
int numclasses;


