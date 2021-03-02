//*************************************************************************************
//*************************************************************************************
// File: menu.h
//*************************************************************************************
//*************************************************************************************

enum 
{
	PMENU_ALIGN_LEFT,
	PMENU_ALIGN_CENTER,
	PMENU_ALIGN_RIGHT
};

struct SMenu
{
	char text[ 50 ];
	int align;
	int arg;
	void (*SelectFunc)( edict_t *ent, SMenu *entry );
};

struct SMenuHnd
{
	SMenu *entries;
	int cur;
	int num;

	void (*SelectFunc)( edict_t *ent, SMenu *entry );
};

void PMenu_Open(edict_t *ent, SMenu *entries, int cur, int num);
