#include "g_local.h"
#include "m_player.h"

void GiveDefaultEquipment(edict_t *ent)
{
	char	*s, *start;
	char	data[MAX_QPATH];
	int len;
	gitem_t *item;

	s = (char *)sv_equipment->string;
	
	if (!s || !s[0])
		return;

	while (*s)
	{
		start = s;
		while (*s && *s != ' ')
			s++;

		len = s-start;

		if (len >= MAX_QPATH || len < 5)
			break;

		memcpy (data, start, len);
		data[len] = 0;

		item = FindItemByClassname(data);
		if (item)
	        	GiveItem(ent, item->tag);

		if (*s)
			s++;
	}
}


/*
==============
z_PutClientInServer
==============
*/
void z_PutClientInServer(edict_t *ent)
{
        int i;

        ent->client->itemSelect = -1;
        ent->client->cycleItems = -10;
        ent->client->ps.stats[STAT_HIGHLIGHT] = -1;
	ent->client->newRightWeapon = -1;
	ent->client->newLeftWeapon = -1;

        for (i = STAT_INV1; i <= STAT_HIGHLIGHT; i++)
                ent->client->ps.stats[i] = gi.imageindex ("i_null");

        SetupItemModels(ent);
}
/*
==============
z_InitClientPers
==============
*/
void z_InitClientPers(edict_t *ent)
{
        memset(ent->client->pers.cstats, 0, sizeof(ent->client->pers.cstats));
        memset(ent->client->pers.item_bodyareas, 0, sizeof(ent->client->pers.item_bodyareas));
        memset(ent->client->pers.item_quantities, 0, sizeof(ent->client->pers.item_quantities));
        memset(ent->client->pers.item_flags, 0, sizeof(ent->client->pers.item_flags));

        ent->client->pers.cstats[CSTAT_RIGHTHAND] = II_HANDS;
        ent->client->pers.cstats[CSTAT_LEFTHAND]= II_HANDS;
	ent->client->newRightWeapon = -1;
	ent->client->newLeftWeapon = -1;

        ent->client->pers.weapon = GetItemByTag(II_HANDS);

	if (sv_edit->value)
        	GiveItem(ent, II_WEAPON_EDIT);

	GiveDefaultEquipment(ent);
}
