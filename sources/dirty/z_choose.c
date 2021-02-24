#include "g_local.h"
#include "m_player.h"

void StageThree(edict_t *ent);
void GiveStartingItems (edict_t *ent);

void GoalDammit(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
        //Cmd_MeGoal_f (ent);
}


pmenu_t change_menu[] = {
        { "*SELECTION MENU",            PMENU_ALIGN_CENTER, NULL, NULL },
        { "Change team",                PMENU_ALIGN_LEFT, NULL, OpenChoosingMenu },
        { "Change class/items",         PMENU_ALIGN_LEFT, NULL, ChangeThings },
        { "Teams goal",                 PMENU_ALIGN_LEFT, NULL, GoalDammit },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL }
};

void ChangeMenu (edict_t *ent)
{
        if (ent->client->pers.spectator)
        {
                gi.cprintf (ent, PRINT_HIGH, "Spectators can't join in\n");
                return;
        }

        if (max_teams < 1)
                ChangeThings (ent, NULL);
        else
        {
                PMenu_Close(ent);
                PMenu_Open(ent, change_menu, -1, sizeof(change_menu) / sizeof(pmenu_t));
        }
}

void ChosenEquipMent (edict_t *ent)
{
        gitem_t         *item;
        gclient_t       *client;
        int             i;

        client = ent->client;

        item = NULL;

        if (client->resp.weapon && (client->resp.weapon == FindItem("Hands")))
                client->pers.weapon = client->resp.weapon;
        else if (client->resp.weapon)
        {
                client->pers.inventory[ITEM_INDEX(client->resp.weapon)] += 1;

                if (client->resp.weapon->ammo)
                        item = FindItem(client->resp.weapon->ammo);
        
                if (!Q_stricmp(client->resp.weapon->pickup_name, "Combat Knife"))
                {
                        item = client->resp.weapon;
                        i = 10;
                }
                else if (item && (item->flags & IT_CLIP))
                {
                        if (item->weight < 4)
                                i = 4;
                        else
                                i = 2;
                }
                else if (item)
                {
                        i = item->quantity * 2;
                        if (item->flags & IT_WEAPON)
                                i *= 2;
                }

                if (client->resp.item)
                {
                        if (!Q_stricmp(client->resp.item->pickup_name, "Bandolier"))
                                i *= 2;
                        else if (!Q_stricmp(client->resp.item->pickup_name, "Back Pack"))
                                i *= 4;
                }

                if (item)
                {
                        client->pers.inventory[ITEM_INDEX(item)] += i;
                        if (item->flags & IT_CLIP)
                                client->pers.clips[CLIP_INDEX(FindClip(item->pickup_name))] = item->quantity * i;
                }

                if (!Q_stricmp(client->resp.weapon->pickup_name, "MK23 Pistol"))
                        client->pers.weapon2 = client->resp.weapon;
                else
                        client->pers.weapon = client->resp.weapon;
        }

        if (client->resp.armor)
        {
                client->pers.inventory[ITEM_INDEX(client->resp.armor)] = 1;
                client->pers.armor = client->resp.armor;
                client->pers.armor_index = client->resp.armor->quantity;
        }

        if (client->resp.item)
                client->pers.inventory[ITEM_INDEX(client->resp.item)] += 1;

        if (ent->client->resp.team)
                GiveStartingItems (ent);
}

void GiveStartingItems (edict_t *ent)
{
        char            *s, *start;
        char            data[MAX_QPATH];
        weapons_t       *slot;
        gitem_t         *item;
        gitem_t         *item2;
        gitem_t         *ammo;
        int             len;
        gclient_t       *client;
        int             i;

        client = ent->client;

	// parse the space seperated precache string for other items
        s = client->resp.team->starting_equipment;

	if (!s || !s[0])
		return;

	while (*s)
	{
		start = s;
		while (*s && *s != ' ')
			s++;

		len = s-start;
		if (len >= MAX_QPATH || len < 5)
                        gi.error ("GiveStartingItems: Team %s has bad starting_equipment string", client->resp.team->name);
		memcpy (data, start, len);
		data[len] = 0;
		if (*s)
			s++;

                item = FindItemByClassname (data);
                if (!item)
                        continue;
                else if (item == FindItem("Hands"))
                        client->pers.weapon = item;
                else if ((item->pickup == Pickup_Weapon) && ((client->pers.weapon == NULL) || (client->pers.weapon == FindItem("Hands"))))
                {
                        client->pers.weapon = item;
                        client->pers.inventory[ITEM_INDEX(item)] += 1;

                        ammo = NULL;
                        if (item->ammo)
                                ammo = FindItem(item->ammo);

                        if (ammo)
                        {
                                if (ammo->flags & IT_CLIP)
                                {
                                        if (ammo->weight < 4)
                                                i = 4;
                                        else
                                                i = 2;
                                }
                                else
                                        i = ammo->quantity * 2;

                                if (client->pers.inventory[ITEM_INDEX(FindItem("Bandolier"))])
                                        i *= 2;
                                else if (client->pers.inventory[ITEM_INDEX(FindItem("Back Pack"))])
                                        i *= 4;

                                client->pers.inventory[ITEM_INDEX(ammo)] += i;
                                if (ammo->flags & IT_CLIP)
                                        client->pers.clips[CLIP_INDEX(FindClip(ammo->pickup_name))] = ammo->quantity * i;
                        }
                }
                else if (item->pickup == Pickup_Weapon)
                {
                        slot = NULL;
                        if (item->flags & IT_BIG)
                        {
                                slot = FindSlot(ent, "Large Weapon Slot");
                                if (slot->pickup_name)
                                        slot = NULL;
                        }
                        else
                                slot = FindBlank(ent, 1);

                        item2 = NULL;
                        ammo = NULL;
                        if (slot)
                        {
                                client->pers.inventory[ITEM_INDEX(item)] += 1;
                                if (item->scomp)
                                {
                                        item2 = FindItem(item->scomp);
                                        SetSlot (ent, slot->slot_name, item->pickup_name, 0, 0, item->ammo, item->quantity, item2->ammo, item2->quantity);
                                }
                                else
                                        SetSlot (ent, slot->slot_name, item->pickup_name, 0, 0, item->ammo, item->quantity, NULL, 0);

                                if (item->ammo)
                                        ammo = FindItem(item->ammo);

                                if (ammo)
                                {
                                        if (ammo->flags & IT_CLIP)
                                        {
                                                if (ammo->weight < 4)
                                                        i = 4;
                                                else
                                                        i = 2;
                                        }
                                        else
                                                i = ammo->quantity * 2;

                                        if (client->pers.inventory[ITEM_INDEX(FindItem("Bandolier"))])
                                                i *= 2;
                                        else if (client->pers.inventory[ITEM_INDEX(FindItem("Back Pack"))])
                                                i *= 4;

                                        client->pers.inventory[ITEM_INDEX(ammo)] += i;
                                        if (ammo->flags & IT_CLIP)
                                                client->pers.clips[CLIP_INDEX(FindClip(ammo->pickup_name))] = ammo->quantity * i;
                                }
                        }
                }
                else
                {
                        client->pers.inventory[ITEM_INDEX(item)] += 1;
                        if (item->flags & IT_CLIP)
                                client->pers.clips[CLIP_INDEX(FindClip(item->pickup_name))] += item->quantity;
                }
	}
}


void StageThree(edict_t *ent);

void OpenStageThree(edict_t *ent, pmenu_t *p)
{
        int     i;

        i = ent->client->menu->cur;

        ent->client->resp.armor = NULL;
        ent->client->resp.item = NULL;

        if (i == 1)
                ent->client->resp.armor = FindItem("Kevlar Vest");
        else if (i == 2)
                ent->client->resp.item = FindItem("Laser Sight");
        else if (i == 3)
                ent->client->resp.item = FindItem("Stealth Slippers");
        else if (i == 4)
                ent->client->resp.item = FindItem("Silencer");
        else if (i == 5)
                ent->client->resp.item = FindItem("Bandolier");
        else if (i == 6)
                ent->client->resp.item = FindItem("Flash Light");
        else
                ent->client->resp.item = NULL;

        StageThree(ent);
}

pmenu_t choose_item_menu[] = {
        { "*ITEM CHOICES",              PMENU_ALIGN_CENTER, NULL, NULL },
        { "Kevlar Vest",                PMENU_ALIGN_LEFT, NULL, OpenStageThree },
        { "Laser Sight",                PMENU_ALIGN_LEFT, NULL, OpenStageThree },
        { "Stealth Slippers",           PMENU_ALIGN_LEFT, NULL, OpenStageThree },
        { "Silencer",                   PMENU_ALIGN_LEFT, NULL, OpenStageThree },
        { "Bandolier",                  PMENU_ALIGN_LEFT, NULL, OpenStageThree },
        { "Flash Light",                PMENU_ALIGN_LEFT, NULL, OpenStageThree },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL }
};

void UpdateAndOpenItemMenu (edict_t *ent)
{
        char    *s;
        //int     i;
        gclient_t *client;

        client = ent->client;

        memcpy(ent->client->current_menu, choose_item_menu, sizeof(choose_item_menu));

        s = NULL;
        if (client->resp.team && (client->resp.team->item_choices))
                s = client->resp.team->item_choices;

        if ((!s) && sv_itemchoices->string)
                s = sv_itemchoices->string;

        if (!s || !s[0] || (strstr (s, "all")))
        {
                PMenu_Close(ent);
                PMenu_Open(ent, client->current_menu, -1, sizeof(choose_item_menu) / sizeof(pmenu_t));
		return;
        }

        if (strstr (s, "none"))
        {
                StageThree (ent);
                return;
        }

        if (!strstr (s, "item_armor_jacket"))
        {
                client->current_menu[1].text = "";
                client->current_menu[1].SelectFunc = NULL;
        }                
        
        if (!strstr (s, "item_laser_sight"))
        {
                client->current_menu[2].text = "";
                client->current_menu[2].SelectFunc = NULL;
        }

        if (!strstr (s, "item_slippers"))
        {
                client->current_menu[3].text = "";
                client->current_menu[3].SelectFunc = NULL;
        }                

        if (!strstr (s, "item_silencer"))
        {
                client->current_menu[4].text = "";
                client->current_menu[4].SelectFunc = NULL;
        }                

        if (!strstr (s, "item_bandolier"))
        {
                client->current_menu[5].text = "";
                client->current_menu[5].SelectFunc = NULL;
        }

        if (!strstr (s, "item_flashlight"))
        {
                client->current_menu[6].text = "";
                client->current_menu[6].SelectFunc = NULL;
        }                

        PMenu_Close(ent);
        PMenu_Open(ent, client->current_menu, -1, sizeof(client->current_menu) / sizeof(pmenu_t));
}

void OpenChooseItemMenu(edict_t *ent, pmenu_t *p)
{
        int     i;

        i = ent->client->menu->cur;

        if (i == 1)
                ent->client->resp.weapon = FindItem("MP5/10 Submachinegun");
        else if (i == 2)
                ent->client->resp.weapon = FindItem("M3 Super 90 Assault Shotgun");
        else if (i == 3)
                ent->client->resp.weapon = FindItem("Handcannon");
        else if (i == 4)
                ent->client->resp.weapon = FindItem("Sniper Rifle");
        else if (i == 5)
                ent->client->resp.weapon = FindItem("M4 Assault Rifle");
        else if (i == 6)
                ent->client->resp.weapon = FindItem("Combat Knife");
        else if (i == 7)
                ent->client->resp.weapon = FindItem("MK23 Pistol");

        PMenu_Close(ent);

        //if (max_teams)
                UpdateAndOpenItemMenu (ent);
        //else
        //        PMenu_Open(ent, choose_item_menu, -1, sizeof(choose_item_menu) / sizeof(pmenu_t));
}

pmenu_t choose_weapon_menu[] = {
        { "*WEAPON CHOICES",            PMENU_ALIGN_CENTER, NULL, NULL },
        { "MP5/10 Submachinegun",       PMENU_ALIGN_LEFT, NULL, OpenChooseItemMenu },
        { "M3 Super 90 Assault Shotgun",PMENU_ALIGN_LEFT, NULL, OpenChooseItemMenu },
        { "Handcannon",                 PMENU_ALIGN_LEFT, NULL, OpenChooseItemMenu },
        { "SSG1 Sniper Rifle",          PMENU_ALIGN_LEFT, NULL, OpenChooseItemMenu },
        { "M4 Assault Rifle",           PMENU_ALIGN_LEFT, NULL, OpenChooseItemMenu },
        { "Combat Knives",              PMENU_ALIGN_LEFT, NULL, OpenChooseItemMenu },
        { "Dual Pistols",               PMENU_ALIGN_LEFT, NULL, OpenChooseItemMenu },
        { NULL,                         PMENU_ALIGN_LEFT, NULL, NULL }
};

void UpdateAndOpenWeaponMenu (edict_t *ent)
{
        char    *s;
        //int     i;
        gclient_t *client;

        client = ent->client;

        memcpy(ent->client->current_menu, choose_weapon_menu, sizeof(choose_weapon_menu));

        s = NULL;
        if (client->resp.team && (client->resp.team->weapon_choices))
                s = client->resp.team->weapon_choices;

        if ((!s) && sv_weaponchoices->string)
                s = sv_weaponchoices->string;

        if (!s || !s[0] || (strstr (s, "all")))
        {
                PMenu_Close(ent);
                PMenu_Open(ent, client->current_menu, -1, sizeof(client->current_menu) / sizeof(pmenu_t));
		return;
        }

        if (strstr (s, "none"))
        {
                ent->client->resp.weapon = FindItem("Hands");
                UpdateAndOpenItemMenu (ent);
                return;
        }
        
        if (!strstr (s, "weapon_mp5"))
        {
                client->current_menu[1].text = "";
                client->current_menu[1].SelectFunc = NULL;
        }                
        
        if (!strstr (s, "weapon_shotgun"))
        {
                client->current_menu[2].text = "";
                client->current_menu[2].SelectFunc = NULL;
        }

        if (!strstr (s, "weapon_cannon"))
        {
                client->current_menu[3].text = "";
                client->current_menu[3].SelectFunc = NULL;
        }                

        if (!strstr (s, "weapon_sniper"))
        {
                client->current_menu[4].text = "";
                client->current_menu[4].SelectFunc = NULL;
        }                

        if (!strstr (s, "weapon_rifle"))
        {
                client->current_menu[5].text = "";
                client->current_menu[5].SelectFunc = NULL;
        }

        if (!strstr (s, "weapon_knife"))
        {
                client->current_menu[6].text = "";
                client->current_menu[6].SelectFunc = NULL;
        }                

        if (!strstr (s, "weapon_pistol"))
        {
                client->current_menu[7].text = "";
                client->current_menu[7].SelectFunc = NULL;
        }

        PMenu_Close (ent);
        PMenu_Open (ent, client->current_menu, -1, sizeof(client->current_menu) / sizeof(pmenu_t));
}

void OpenChooseWeaponMenu(edict_t *ent)
{
        ent->client->resp.weapon = NULL;
        ent->client->resp.armor = NULL;
        ent->client->resp.item = NULL;
        ent->client->resp.item2 = NULL;

        if (ent->client->menu)
                PMenu_Close(ent);

        if ((int)bflags->value & BF_JOHN_WOO)
        {
                StageThree (ent);
                return;
        }

        //if (max_teams)
                UpdateAndOpenWeaponMenu (ent);
        //else
        //        PMenu_Open(ent, choose_weapon_menu, -1, sizeof(choose_weapon_menu) / sizeof(pmenu_t));
}

void ChangeThings (edict_t *ent, pmenu_t *p)
{
        if (!FloodCheck (ent))
                return;

        if (ent->client->pers.spectator)
        {
                gi.cprintf (ent, PRINT_HIGH, "Spectators can't join in\n");
                return;
        }

        if ((level.time - ent->client->respawn_time) < 0.5)
        {
                gi.cprintf(ent, PRINT_HIGH, "forced wait\n");
		return;
        }

        if ((ent->health > 0) && (ent->movetype != MOVETYPE_NOCLIP))
        {
                ent->health = 0;

                // Force lives to 1 if alive...
                // This way, player_die removes them from the living check
                ent->client->resp.lives = 1;

                meansOfDeath = MOD_SUICIDE;
                TypeOfDamage = 0;
                player_die (ent, ent, ent, 100000, vec3_origin);
                ent->deadflag = DEAD_DEAD;
                // don't even bother waiting for death frames
                // and stop holograms being made.
                ent->s.frame = FRAME_crdeath5;
                ent->client->anim_end  = ent->s.frame;
        }

        if (ent->deadflag && (ent->movetype != MOVETYPE_NOCLIP))
        {
                if (!ent->master)
                        CreateBodyInventory (ent);
                CopyToBodyQue (ent);
        }

        // GRIM - FIX ME
        // If NOT dead, you would be missed by the above level check.
        // That would mean match games would remain going, even when everyone
        // was changing items. That is not good. This should fix the problem
        // for now. Improve the actual code later.
        if (turns_on && (ent->client->resp.lives > 0) && (ent->movetype != MOVETYPE_NOCLIP))
        {
                ent->client->resp.lives = 0;
                level.living--;
                MatchOverCheck ();
        }
        // GRIM                

	ent->svflags = 0;
        ent->deadflag = DEAD_NO;
        ent->client->resp.state = CS_NOT_READY;

        //if ((ent->movetype != MOVETYPE_NOCLIP) && (ent->client->resp.score > 0))
        //        AdjustScore(ent, -1);

        ent->client->resp.armor = NULL;
        ent->client->resp.item = NULL;
        ent->client->resp.item2 = NULL;

        MakeObserver (ent);

        OpenChooseWeaponMenu(ent);
}
