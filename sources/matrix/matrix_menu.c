#include "g_local.h"

static void speed (edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	Cmd_BuySpeed_f(ent);
}


/*
pmenu_t upgrademenu[] = {
	{ "*Upgrades",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Upgrade----Console Command",							PMENU_ALIGN_LEFT, NULL, NULL },
};


static void openupgrade (edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	PMenu_Open(ent, upgrademenu, -1, sizeof(upgrademenu) / sizeof(pmenu_t));

}
  */
static void posses (edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
//	MatrixPlayerChange(ent);
	MatrixStartSwap(ent);
}
static void stopb (edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	Cmd_StopBullets_f(ent);
}
static void jump (edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	Cmd_Jump_f(ent);
}
static void irvis (edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	Cmd_Infrared_f(ent);
}
static void cloak (edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	Cmd_Cloak_f(ent);
}
static void light (edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	Cmd_Lights_f(ent);
}
static void kungfu (edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	Cmd_KungFu_f(ent);
}
pmenu_t spellmenu[] = {
	{ "*Spells",		PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Spell----Console Command",							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "            |           ",							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Posses      |     Change",					PMENU_ALIGN_LEFT, NULL, posses },
	{ "Stop Bullets|StopBullets",					PMENU_ALIGN_LEFT, NULL, stopb },
	{ "Matrix Jump | MatrixJump",					PMENU_ALIGN_LEFT, NULL, jump },
	{ "Ir Vision   |   IRVision",					PMENU_ALIGN_LEFT, NULL, irvis },
	{ "Cloak       |      Cloak",					PMENU_ALIGN_LEFT, NULL, cloak },
	{ "Speed       |   Up_Speed",					PMENU_ALIGN_LEFT, NULL, speed },
	{ "EMP Blast   |     Lights",					PMENU_ALIGN_LEFT, NULL, light },
//	{ "KungFuSkillz|     KungFu",					PMENU_ALIGN_LEFT, NULL, kungfu }
};

static void openspell (edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	PMenu_Open(ent, spellmenu, -1, sizeof(spellmenu) / sizeof(pmenu_t));

}

static void  Autobuyspell (edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	Cmd_AutoBuy_f(ent);
}
pmenu_t matrixmenu[] = {
	{ "The Matrix For Quake II",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Auto-Buy Spell",					PMENU_ALIGN_LEFT, NULL, Autobuyspell },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use Spell",					PMENU_ALIGN_LEFT, NULL, openspell },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to view inventory)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ GAMEVERSION,					PMENU_ALIGN_RIGHT, NULL, NULL },
};

void MatrixOpenMenu (edict_t *ent)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	PMenu_Open(ent, matrixmenu, -1, sizeof(matrixmenu) / sizeof(pmenu_t));

}
static void dropstamina (edict_t *ent, pmenu_t *p)
{
	if (ent->stamina < 100)
	{
		gi.cprintf (ent, PRINT_HIGH, "You dont have enough energy stored\n");
		return;
	}
	ent->stamina -= 100;
	if (ent->client->menu)
		PMenu_Close(ent);
	MatrixTankDropItem(ent,IT_STAMINA);
}
static void droparmour (edict_t *ent, pmenu_t *p)
{
	if (ent->stamina < 100)
	{
		gi.cprintf (ent, PRINT_HIGH, "You dont have enough energy stored\n");
		return;
	}
	ent->stamina -= 100;
	if (ent->client->menu)
		PMenu_Close(ent);
	MatrixTankDropItem(ent,IT_ARMOR);
}
static void droppower (edict_t *ent, pmenu_t *p)
{
	if (ent->stamina < 100)
	{
		gi.cprintf (ent, PRINT_HIGH, "You dont have enough energy stored\n");
		return;
	}
	ent->stamina -= 100;
	if (ent->client->menu)
		PMenu_Close(ent);
	MatrixTankDropItem(ent,IT_POWERUP);
}
static void dropweap (edict_t *ent, pmenu_t *p)
{
	if (ent->stamina < 100)
	{
		gi.cprintf (ent, PRINT_HIGH, "You dont have enough energy stored\n");
		return;
	}
	ent->stamina -= 100;
	
	if (ent->client->menu)
		PMenu_Close(ent);
	MatrixTankDropItem(ent,IT_WEAPON);
}
pmenu_t tankmenu[] = {
	{ "The Matrix For Quake II",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Tank Mode",					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Drop Weapon",				PMENU_ALIGN_LEFT, NULL, dropweap },
	{ "Drop Stamina",					PMENU_ALIGN_LEFT, NULL, dropstamina },
	{ "Drop Armor",				PMENU_ALIGN_LEFT, NULL, droparmour },
	{ "Drop PowerUp",				PMENU_ALIGN_LEFT, NULL, droppower },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",			PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to view inventory)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, NULL, NULL },
	{ GAMEVERSION,					PMENU_ALIGN_RIGHT, NULL, NULL },
};

void MatrixOpenTankMenu (edict_t *ent)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	PMenu_Open(ent, tankmenu, -1, sizeof(tankmenu) / sizeof(pmenu_t));

}