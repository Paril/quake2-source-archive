/* Runes */
#include "g_local.h"
#include "aj_runes.h"
#include "aj_statusbars.h"

#define EF_YELLOWSHELL 0x08000000;
void lithium_runecolour(gitem_t *item, edict_t *ent)
{
	ent->s.renderfx = RF_GLOW;

	if (!use_lithiumrunes->value)
		return;

	if (!strcmp(item->classname, "item_tech1")) // resist
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx = RF_SHELL_BLUE; 
	}
	else if (!strcmp(item->classname, "item_tech2")) // strength
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx = RF_SHELL_RED; 
	}
	else if (!strcmp(item->classname, "item_tech3")) // haste
	{
// what a shit dirty hack!!!
//		ent->s.effects |= EF_COLOR_SHELL;
//		ent->s.renderfx = RF_SHELL_RED | RF_SHELL_GREEN; 
		ent->s.effects |= EF_YELLOWSHELL;
	}
	else if (!strcmp(item->classname, "item_tech4")) // regen
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx = RF_SHELL_GREEN; 
	}
	else if (!strcmp(item->classname, "item_tech5")) // vampire
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx = RF_SHELL_RED | RF_SHELL_BLUE; 
	}

}

char *lmctf_tech1_icon="a_resist",  //ScarFace- fixed names
     *lmctf_tech2_icon="a_strength", 
	 *lmctf_tech3_icon="a_haste",
	 *lmctf_tech4_icon="a_regen",
	 *lmctf_tech5_icon="\0";
char *lithium_tech_icon="k_pyramid";

void lithium_setuprunes(void)
{
	gitem_t *item;
	char *n;

	// yes, this code is a bit risky, as what if the string moves/changes....
	// but its better than calling strdup() and leaking memory - if the
	// cvar is latched, I dont think it CAN change without this function
	// being re-called
	item = FindItemByClassname("item_tech1");
	if (item)
	{
		item->world_model = rune_resist_mdl->string; 
		item->pickup_name = rune_resist_name->string; 
		if (lmctf->value)
			item->icon = lmctf_tech1_icon;
		/*else*/ if (use_lithiumrunes->value) //ScarFace- added else for always using LMCTF icons in LMCTF mode
			item->icon = rune_resist_icon->string;

		n=rune_resist_shortname->string;
		while (n && *n)
		{
			*n += 128;
			n++;
		}
		gi.configstring (CS_TECH1, rune_resist_shortname->string);
	}

	item = FindItemByClassname("item_tech2");
	if (item)
	{
		if (lmctf->value)
			item->icon = lmctf_tech2_icon;
		/*else*/ if (use_lithiumrunes->value) //ScarFace- added else for always using LMCTF icons in LMCTF mode
			item->icon = rune_strength_icon->string;
		
		item->world_model = rune_strength_mdl->string;
		gi.configstring (CS_ITEMS+ITEM_INDEX(item), rune_strength_name->string);
		n=rune_strength_shortname->string;
		while (n && *n)
		{
			*n += 128;
			n++;
		}
		gi.configstring (CS_TECH2, rune_strength_shortname->string);
	}

	item = FindItemByClassname("item_tech3");
	if (item)
	{
		if (lmctf->value)
			item->icon = lmctf_tech3_icon;
		/*else*/ if (use_lithiumrunes->value) //ScarFace- added else for always using LMCTF icons in LMCTF mode
			item->icon = rune_haste_icon->string;

		item->world_model = rune_haste_mdl->string; 
//		item->pickup_name = rune_haste_name->string; 
		gi.configstring (CS_ITEMS+ITEM_INDEX(item), rune_haste_name->string);
		n=rune_haste_shortname->string;
		while (n && *n)
		{
			*n += 128;
			n++;
		}
		gi.configstring (CS_TECH3, rune_haste_shortname->string);
	}

	item = FindItemByClassname("item_tech4");
	if (item)
	{
		if (lmctf->value)
			item->icon = lmctf_tech4_icon;
		/*else*/ if (use_lithiumrunes->value) //ScarFace- added else for always using LMCTF icons in LMCTF mode
			item->icon = rune_regen_icon->string;

		item->world_model = rune_regen_mdl->string;
//		item->pickup_name = rune_regen_name->string; 
		gi.configstring (CS_ITEMS+ITEM_INDEX(item), rune_regen_name->string);
		n=rune_regen_shortname->string;
		while (n && *n)
		{
			*n += 128;
			n++;
		}
		gi.configstring (CS_TECH4, rune_regen_shortname->string);
	}

	item = FindItemByClassname("item_tech5");
	if (item)
	{
		if (lmctf->value)
			item->icon = lmctf_tech5_icon;
		if (use_lithiumrunes->value) //LMCTF has no vampire rune
			item->icon = rune_vampire_icon->string;

		item->world_model = rune_vampire_mdl->string;
//		item->pickup_name = rune_vampire_name->string; 
		gi.configstring (CS_ITEMS+ITEM_INDEX(item), rune_vampire_name->string);
		n=rune_vampire_shortname->string;
		while (n && *n)
		{
			*n += 128;
			n++;
		}
		gi.configstring (CS_TECH5, rune_vampire_shortname->string);
	}
}



extern gitem_t	*item_tech5;

void lithium_applyvampire(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = item_tech5;
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) 
	{
		if (ent->health < rune_vampiremax->value)
		{
			if (ent->health + dmg < rune_vampiremax->value)
				ent->health+=(dmg * rune_vampire->value);
			else ent->health=rune_vampiremax->value;
		}

		CTFApplyVampireSound(ent);
	}
}

qboolean CTFApplyVampireSound(edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = item_tech5;
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		if (ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
			gi.sound(ent, CHAN_VOICE, gi.soundindex(rune_vampire_sound->string), volume, ATTN_NORM, 0);
		}
		return true;
	}
	return false;
}

