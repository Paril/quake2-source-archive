#include "g_local.h"
#include "w_fire.h"

#define MAX_CLASS 32

d_Class             classtbl[MAX_CLASS];
int                             actual_class;

//WYRM: MOD tables...
d_MDam MODTable[] =
{
        {       "mod_unknown",        MOD_UNKNOWN       },
        {       "mod_blaster",        MOD_BLASTER       },
        {       "mod_shotgun",        MOD_SHOTGUN       },
        {       "mod_sshotgun",       MOD_SSHOTGUN      },
        {       "mod_machinegun",     MOD_MACHINEGUN    },
        {       "mod_chaingun",       MOD_CHAINGUN      },
        {       "mod_grenade",        MOD_GRENADE       },
        {       "mod_g_splash",       MOD_G_SPLASH      },
        {       "mod_rocket",         MOD_ROCKET        },
        {       "mod_r_splash",       MOD_R_SPLASH      },
        {       "mod_hyperblaster",   MOD_HYPERBLASTER  },
        {       "mod_railgun",        MOD_RAILGUN       },
        {       "mod_bfg_laser",      MOD_BFG_LASER     },
        {       "mod_bfg_blast",      MOD_BFG_BLAST     },
        {       "mod_bfg_effect",     MOD_BFG_EFFECT    },
        {       "mod_handgrenade",    MOD_HANDGRENADE   },
        {       "mod_hg_splash",      MOD_HG_SPLASH     },
        {       "mod_water",          MOD_WATER         },
        {       "mod_slime",          MOD_SLIME         },
        {       "mod_lava",           MOD_LAVA          },
        {       "mod_crush",          MOD_CRUSH         },
#if 0
        {       "mod_telefrag",       MOD_TELEFRAG      },
#endif
        {       "mod_falling",        MOD_FALLING       },
#if 0
        {       "mod_suicide",        MOD_SUICIDE       },
#endif
        {       "mod_held_grenade",   MOD_HELD_GRENADE  },
        {       "mod_explosive",      MOD_EXPLOSIVE     },
        {       "mod_barrel",         MOD_BARREL        },
        {       "mod_bomb",           MOD_BOMB          },
        {       "mod_exit",           MOD_EXIT          },
        {       "mod_splash",         MOD_SPLASH        },
        {       "mod_target_laser",   MOD_TARGET_LASER  },
        {       "mod_trigger_hurt",   MOD_TRIGGER_HURT  },
        {       "mod_hit",            MOD_HIT           },
        {       "mod_target_blaster", MOD_TARGET_BLASTER},
//Wyrm: new mods
        {       "mod_freeze",         MOD_FREEZE        },
        {       "mod_pistol",         MOD_PISTOL        },
        {       "mod_flaregun",       MOD_FLAREGUN      },
        {       "mod_airfist",        MOD_AIRFIST       },
        {       "mod_doubleimpact",   MOD_DOUBLEIMPACT  },
        {       "mod_explosivemachinegun",      MOD_EXPLOSIVEMACHINEGUN },
        {       "mod_pulserifle",     MOD_PULSERIFLE    },
        {       "mod_firethrower",    MOD_FIRETHROWER   },
        {       "mod_streetsweeper",  MOD_STREETSWEEPER },
        {       "mod_clustergrenades",MOD_CLUSTERGRENADES},
        {       "mod_railgrenades",   MOD_RAILGRENADES  },
        {       "mod_proxgrenades",   MOD_PROXGRENADES  },
        {       "mod_napalmgrenades", MOD_NAPALMGRENADES},
        {       "mod_stickinggrenades", MOD_STICKINGGRENADES},
        {       "mod_bfgrenades",     MOD_BFGRENADES    },
        {       "mod_napalmrockets",  MOD_NAPALMROCKETS },
        {       "mod_guidedrockets",  MOD_GUIDEDROCKETS },
        {       "mod_plasma",         MOD_PLASMA        },
        {       "mod_disruptor",      MOD_DISRUPTOR     },
        {       "mod_antimatter",     MOD_ANTIMATTER    },
        {       "mod_positron",       MOD_POSITRON      },
        {       "mod_tripbomb",       MOD_TRIPBOMB      },
        {       "mod_lasertripbomb",  MOD_LASERTRIPBOMB },
        {       "mod_grapple",        MOD_GRAPPLE       },
        {       "mod_fire",           MOD_FIRE          },
        {       "mod_fire_splash",    MOD_FIRE_SPLASH   },
        {       "mod_on_fire",        MOD_ON_FIRE       },
        {       "mod_fireball",       MOD_FIREBALL      },
        {       "mod_nail",           MOD_NAIL          },
        {       "mod_supernail",      MOD_SUPERNAIL     },
        {       "mod_vacuum",         MOD_VACUUM        },
        {       "mod_vortex",         MOD_VORTEX        },
        {       "mod_drainer",        MOD_DRAINER       },
        {       "mod_bucky",          MOD_BUCKY         },
        {       "mod_lightning",      MOD_LIGHTNING     },
        {       "mod_discharge",      MOD_DISCHARGE     },
        {       "mod_nuke",           MOD_NUKE          },
        {       "mod_perforator_splash", MOD_PERFORATOR_SPLASH },
        {       "mod_perforator",     MOD_PERFORATOR    },
        {       "mod_sentrybullet",   MOD_SENTRYBULLET  },
        {       "mod_sentryexplode",  MOD_SENTRYEXPLODE },
//last mod!
        {       "",                   -1    }
};
//Wyrm: armor basic info
gitem_armor_t basic_jacketarmor_info  = { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t basic_combatarmor_info  = { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t basic_bodyarmor_info    = {100, 200, .80, .60, ARMOR_BODY};

void ReadClass (void)
{
	cvar_t	*basedir = gi.cvar("basedir", ".", CVAR_SERVERINFO);
	cvar_t	*gamedir = gi.cvar("gamedir", "baseq2", CVAR_SERVERINFO);
        char    class_lst[MAX_QPATH], classattribute[MAX_OSPATH], *point;
	FILE	*file = NULL;
        qboolean sortir;
        int c;

        actual_class = 0;


        if (strlen(classfile->string) > 0)
        {
#ifdef _WIN32
                Com_sprintf(class_lst, sizeof(class_lst), "%s\\%s\\%s", basedir->string, gamedir->string, classfile->string);
#else
                Com_sprintf(class_lst, sizeof(class_lst), "%s/%s/%s", basedir->string, gamedir->string, classfile->string);
#endif
        }
        else
        {
#ifdef _WIN32
                Com_sprintf(class_lst, sizeof(class_lst), "%s\\%s\\default.class", basedir->string, gamedir->string);
#else
                Com_sprintf(class_lst, sizeof(class_lst), "%s/%s/default.class", basedir->string, gamedir->string);
#endif
        }

        if (file = fopen(class_lst, "r"))
	{
                while (fgets(classattribute, sizeof(classattribute), file) && actual_class < MAX_CLASS)
		{
loop:                   
                        if (classattribute[0] != '{')
                                continue;

                        classtbl[actual_class].name[0] = '\0';
                        classtbl[actual_class].skin[0] = '\0';
                        classtbl[actual_class].keys[0] = '\0';
                        classtbl[actual_class].speed = 1;
                        classtbl[actual_class].power = 1;
                        classtbl[actual_class].resistance = 1;
                        classtbl[actual_class].jump = 1;
                        classtbl[actual_class].knockback = 1;
                        classtbl[actual_class].health = 100;
                        classtbl[actual_class].max_health = 100;
                        classtbl[actual_class].max_bullets = 200;
                        classtbl[actual_class].max_shells = 100;
                        classtbl[actual_class].max_rockets = 50;
                        classtbl[actual_class].max_grenades = 50;
                        classtbl[actual_class].max_cells = 200;
                        classtbl[actual_class].max_slugs = 50;
/*
                        for (c=0; c<=MAX_ITEMS; c++)
                                classtbl[actual_class].inventory[c] = 0;
*/
                        memset (&classtbl[actual_class].inventory, 0, sizeof(classtbl[actual_class].inventory));
                        c=0;
                        while (c < MOD_LAST)
                        {
                                classtbl[actual_class].inflict[c] = 1;
                                classtbl[actual_class].receive[c] = 1;
                                c++;
                        }

                        //Wyrm: initial weapon
                        classtbl[actual_class].initial_weapon = FindItem("Blaster");

                        //Wyrm: class armors
                        memcpy(&classtbl[actual_class].jacketarmor_info, &basic_jacketarmor_info, sizeof(classtbl[actual_class].jacketarmor_info));
                        memcpy(&classtbl[actual_class].combatarmor_info, &basic_combatarmor_info, sizeof(classtbl[actual_class].combatarmor_info));
                        memcpy(&classtbl[actual_class].bodyarmor_info, &basic_bodyarmor_info, sizeof(classtbl[actual_class].bodyarmor_info));

                        sortir = false;
                        while (fgets(classattribute, sizeof(classattribute), file) && !sortir)
                        {
                                if (classattribute[0] == '}')
                                {
                                        sortir = true;
                                        continue;
                                }
                                if (classattribute[0] == '/')
                                        continue;
                                if (point = strchr(classattribute, '\n'))
                                        *point = 0;
                                if (point = strchr(classattribute, ' '))
                                        *point++ = 0;
                                else
                                        continue;

                                if (!Q_stricmp(classattribute, "name"))
                                        Com_sprintf(classtbl[actual_class].name, sizeof(classtbl[actual_class].name), "%s", point);
                                else if (!Q_stricmp(classattribute, "skin"))
                                        Com_sprintf(classtbl[actual_class].skin, sizeof(classtbl[actual_class].skin), "%s", point);
                                else if (!Q_stricmp(classattribute, "keys"))
                                {
                                        char    keys_lst[MAX_QPATH], fileline[MAX_OSPATH];
                                        FILE    *keysfile = NULL;
                                        char    *actual = classtbl[actual_class].keys;

                                        #ifdef _WIN32
                                                Com_sprintf(keys_lst, sizeof(keys_lst), "%s\\%s\\%s", basedir->string, gamedir->string, point);
                                        #else
                                                Com_sprintf(keys_lst, sizeof(keys_lst), "%s/%s/%s", basedir->string, gamedir->string, point);
                                        #endif

                                        if (keysfile = fopen(keys_lst, "r"))
                                        {
                                                while (fgets(fileline, sizeof(fileline), keysfile))
                                                {
                                                        strcpy(actual, fileline);
                                                        actual += strlen(fileline);
                                                }
                                                fclose(keysfile);
                                        }
                                }
                                else if (!Q_stricmp(classattribute, "speed"))
                                {
                                        classtbl[actual_class].speed = atof(point);
                                        if ((classtbl[actual_class].speed < 0) || (classtbl[actual_class].speed > 10))
                                                classtbl[actual_class].speed = 1;
                                }
                                else if (!Q_stricmp(classattribute, "power"))
                                {
                                        classtbl[actual_class].power = atof(point);
                                        if ((classtbl[actual_class].power < 0) || (classtbl[actual_class].power > 10))
                                                classtbl[actual_class].power = 1;
                                }
                                else if (!Q_stricmp(classattribute, "resistance"))
                                {
                                        classtbl[actual_class].resistance = atof(point);
                                        if ((classtbl[actual_class].resistance < 0) || (classtbl[actual_class].resistance > 10))
                                                classtbl[actual_class].resistance = 1;
                                }
                                else if (!Q_stricmp(classattribute, "jump"))
                                {
                                        classtbl[actual_class].jump = atof(point);
                                        if ((classtbl[actual_class].jump < 0) || (classtbl[actual_class].jump > 10))
                                                classtbl[actual_class].jump = 1;
                                }
                                else if (!Q_stricmp(classattribute, "resistance"))
                                {
                                        classtbl[actual_class].knockback = atof(point);
                                        if ((classtbl[actual_class].knockback < 0) || (classtbl[actual_class].knockback > 10))
                                                classtbl[actual_class].knockback = 1;
                                }
                                else if (!Q_stricmp(classattribute, "health"))
                                {
                                        classtbl[actual_class].health = atoi(point);
                                        if ((classtbl[actual_class].health < 0) || (classtbl[actual_class].health > 999))
                                                classtbl[actual_class].health = 100;
                                }
                                else if (!Q_stricmp(classattribute, "max_health"))
                                {
                                        classtbl[actual_class].max_health = atoi(point);
                                        if ((classtbl[actual_class].max_health < 0) || (classtbl[actual_class].max_health > 999))
                                                classtbl[actual_class].max_health = 100;
                                }
                                else if (!Q_stricmp(classattribute, "max_shells"))
                                {
                                        classtbl[actual_class].max_shells = atoi(point);
                                        if ((classtbl[actual_class].max_shells < 0) || (classtbl[actual_class].max_shells > 999))
                                                classtbl[actual_class].max_shells = 100;
                                }
                                else if (!Q_stricmp(classattribute, "max_bullets"))
                                {
                                        classtbl[actual_class].max_bullets = atoi(point);
                                        if ((classtbl[actual_class].max_bullets < 0) || (classtbl[actual_class].max_bullets > 999))
                                                classtbl[actual_class].max_bullets = 100;
                                }
                                else if (!Q_stricmp(classattribute, "max_rockets"))
                                {
                                        classtbl[actual_class].max_rockets = atoi(point);
                                        if ((classtbl[actual_class].max_rockets < 0) || (classtbl[actual_class].max_rockets > 999))
                                                classtbl[actual_class].max_rockets = 100;
                                }
                                else if (!Q_stricmp(classattribute, "max_grenades"))
                                {
                                        classtbl[actual_class].max_grenades = atoi(point);
                                        if ((classtbl[actual_class].max_grenades < 0) || (classtbl[actual_class].max_grenades > 999))
                                                classtbl[actual_class].max_grenades = 100;
                                }
                                else if (!Q_stricmp(classattribute, "max_slugs"))
                                {
                                        classtbl[actual_class].max_slugs = atoi(point);
                                        if ((classtbl[actual_class].max_slugs < 0) || (classtbl[actual_class].max_slugs > 999))
                                                classtbl[actual_class].max_slugs = 100;
                                }
                                else if (!Q_stricmp(classattribute, "max_cells"))
                                {
                                        classtbl[actual_class].max_cells = atoi(point);
                                        if ((classtbl[actual_class].max_cells < 0) || (classtbl[actual_class].max_cells > 999))
                                                classtbl[actual_class].max_cells = 100;
                                }
                                //Wyrm: class armors
                                else if (!Q_stricmp(classattribute, "jacketarmor"))
                                {
                                        while (*point == ' ') point++;

                                        if (*point == 'b' || *point == 'B')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].jacketarmor_info.base_count = atoi(point);
                                        }
                                        else if (*point == 'm' || *point == 'M')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].jacketarmor_info.max_count = atoi(point);
                                        }
                                        else if (*point == 'n' || *point == 'N')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].jacketarmor_info.normal_protection = atof(point);
                                        }
                                        else if (*point == 'e' || *point == 'E')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].jacketarmor_info.energy_protection = atof(point);
                                        }
                                }
                                else if (!Q_stricmp(classattribute, "combatarmor"))
                                {
                                        while (*point == ' ') point++;

                                        if (*point == 'b' || *point == 'B')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].combatarmor_info.base_count = atoi(point);
                                        }
                                        else if (*point == 'm' || *point == 'M')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].combatarmor_info.max_count = atoi(point);
                                        }
                                        else if (*point == 'n' || *point == 'N')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].combatarmor_info.normal_protection = atof(point);
                                        }
                                        else if (*point == 'e' || *point == 'E')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].combatarmor_info.energy_protection = atof(point);
                                        }
                                }
                                else if (!Q_stricmp(classattribute, "bodyarmor"))
                                {
                                        while (*point == ' ') point++;

                                        if (*point == 'b' || *point == 'B')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].bodyarmor_info.base_count = atoi(point);
                                        }
                                        else if (*point == 'm' || *point == 'M')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].bodyarmor_info.max_count = atoi(point);
                                        }
                                        else if (*point == 'n' || *point == 'N')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].bodyarmor_info.normal_protection = atof(point);
                                        }
                                        else if (*point == 'e' || *point == 'E')
                                        {
                                                //Base Count
                                                 point++;
                                                 classtbl[actual_class].bodyarmor_info.energy_protection = atof(point);
                                        }
                                }
                                //Wyrm: initial weapon
                                else if (!Q_stricmp(classattribute, "initial_weapon"))
                                {
                                        gitem_t *item = FindItemByClassname(point);
                                        if (item)
                                                classtbl[actual_class].initial_weapon = item;
                                }
                                else /* check for initial items */
                                {
                                        gitem_t *item=FindItemByClassname (classattribute);

                                        if (item)
                                                classtbl[actual_class].inventory[ITEM_INDEX(item)] = atoi(point);
                                        else /*not found!!*/
                                        {
                                                //Check for the MODS...
                                                int i=0, sortir = 0;

                                                while (MODTable[i].name[0] != '\0' && !sortir)
                                                {
                                                        if (!Q_stricmp(classattribute, MODTable[i].name))
                                                                sortir = 1;
                                                        else
                                                                i++;
                                                }
                                                if (sortir) //Trobat
                                                {
                                                        //treure espais...
                                                        while (*point == ' ') point++;

                                                        if (*point == 'i' || *point == 'I')
                                                        {
                                                                //inflict!!
                                                                //look for the first number...
//                                                                while (*point < '1' && *point > '9' && *point != '.' && *point != '0' && *point != '\0')
                                                                        point++;
//                                                                if (*point != '\0')
//                                                                {
                                                                        classtbl[actual_class].inflict[MODTable[i].MOD] = atof(point);
                                                                        if ((classtbl[actual_class].inflict[MODTable[i].MOD] < 0) || (classtbl[actual_class].inflict[MODTable[i].MOD] > 10))
                                                                                classtbl[actual_class].inflict[MODTable[i].MOD] = 1;
//                                                                }
                                                        }
                                                        else if (*point == 'r' || *point == 'R')
                                                        {
                                                                //look for the first number...
//                                                                while (*point < '1' && *point > '0' && *point != '.' && *point)
                                                                        point++;
//                                                                if (*point)
                                                                        classtbl[actual_class].receive[MODTable[i].MOD] = atof(point);
                                                                if ((classtbl[actual_class].receive[MODTable[i].MOD] < 0) || (classtbl[actual_class].receive[MODTable[i].MOD] > 100))
                                                                        classtbl[actual_class].receive[MODTable[i].MOD] = 1;
                                                        }
                                                }
                                        }
                                }
                        }

                        actual_class++;
                        if (classattribute[0] == '{')
                                goto loop;

		}
		fclose(file);
	}

        if (!actual_class)
        {
                classtbl[actual_class].name[0] = '\0';
                classtbl[actual_class].skin[0] = '\0';
                classtbl[actual_class].keys[0] = '\0';
                classtbl[actual_class].speed = 1;
                classtbl[actual_class].power = 1;
                classtbl[actual_class].resistance = 1;
                classtbl[actual_class].jump = 1;
                classtbl[actual_class].knockback = 1;
                classtbl[actual_class].health = 100;
                classtbl[actual_class].max_health = 100;
                classtbl[actual_class].max_bullets = 200;
                classtbl[actual_class].max_shells = 100;
                classtbl[actual_class].max_rockets = 50;
                classtbl[actual_class].max_grenades = 50;
                classtbl[actual_class].max_cells = 200;
                classtbl[actual_class].max_slugs = 50;

                memset (&classtbl[actual_class].inventory, 0, sizeof(classtbl[actual_class].inventory));
                
                c=0;
                while (c < MOD_LAST)
                {
                        classtbl[actual_class].inflict[c] = 1;
                        classtbl[actual_class].receive[c] = 1;
                        c++;
                }

                //Wyrm: initial weapon
                classtbl[actual_class].initial_weapon = FindItem("Blaster");
                //Wyrm: class armors
                memcpy(&classtbl[actual_class].jacketarmor_info, &basic_jacketarmor_info, sizeof(classtbl[actual_class].jacketarmor_info));
                memcpy(&classtbl[actual_class].combatarmor_info, &basic_combatarmor_info, sizeof(classtbl[actual_class].combatarmor_info));
                memcpy(&classtbl[actual_class].bodyarmor_info, &basic_bodyarmor_info, sizeof(classtbl[actual_class].bodyarmor_info));

                actual_class = 1;
        }
}

int ParseClass (int class)
{
        if (class < 0) return 0;
        if (class >= actual_class) return (actual_class - 1);
        return class;
}

char *ClassSkin (char *s, int class)
{
        if (!setclassskin->value)
                return s;

        if (classtbl[class].skin[0] == '\0')
                return s;

        return classtbl[class].skin;
}

void ClassInitPersistant(gclient_t *client, int class)
{
        gitem_t *item;

        client->pers.health             = classtbl[class].health;
        client->pers.max_health         = classtbl[class].max_health;

        client->pers.max_bullets        = classtbl[class].max_bullets;
        client->pers.max_shells         = classtbl[class].max_shells;
        client->pers.max_rockets        = classtbl[class].max_rockets;
        client->pers.max_grenades       = classtbl[class].max_grenades;
        client->pers.max_cells          = classtbl[class].max_cells;
        client->pers.max_slugs          = classtbl[class].max_slugs;

/*
        for (c=1; c < MAX_ITEMS; c++)
                client->pers.inventory[c] = classtbl[class].inventory[c];
*/
        memcpy(client->pers.inventory, classtbl[class].inventory, sizeof(client->pers.inventory));
        client->ps.stats[STAT_WEAPON_STRING] = 0;

//Wyrm: give initial weapon :)
        item = classtbl[class].initial_weapon;
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;
//ZOID
	client->pers.lastweapon = item;
//ZOID

}

void SetClassAttributes(gclient_t *client, int class)
{
        client->ClassSpeed              = classtbl[class].speed;
        client->ClassPower              = classtbl[class].power;
        client->ClassResistance         = classtbl[class].resistance;
        client->ClassJump               = classtbl[class].jump;
        client->ClassKnockback          = classtbl[class].knockback;

}


void stuffcmd(edict_t *ent, char *s);

void BindClassKeys(edict_t *ent, pmenu_t *p)
{
        if (ent->client->resp.keysbinded > 5)
        {
                gi.cprintf(ent, PRINT_HIGH, "Keys has been already binded\n");
                return;
        }

        if (enableclass->value)
        {
                if (classtbl[ent->client->resp.class].keys[0])
                        stuffcmd(ent, classtbl[ent->client->resp.class].keys);
                else
                {
                        gi.cprintf(ent, PRINT_HIGH, "Sorry, this class has no bindings defined\n");
                        return;
                }
        }
        else
                if (classtbl[0].keys[0])
                        stuffcmd(ent, classtbl[0].keys);

        ent->client->resp.keysbinded = 6; //keys binded!!
}

void CTFReturnToMain(edict_t *ent, pmenu_t *p);
void ClassReturnToMain(edict_t *ent, pmenu_t *p);
void LastClassPage(edict_t *ent, pmenu_t *p);
void NextClassPage(edict_t *ent, pmenu_t *p);

void ReturnToMain(edict_t *ent, pmenu_t *p)
{
        if (ctf->value)
                CTFReturnToMain(ent, p);
        else
                ClassReturnToMain(ent, p);
}

void EnterInfoClassMenu(edict_t *ent, pmenu_t *p);

pmenu_t classmenu[] = {
        { "*Wyrm II",                           PMENU_ALIGN_CENTER, 0, NULL },
        { "*Class Selection",                   PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 1, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 2, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 3, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 4, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 5, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 6, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 7, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { "Last...",                            PMENU_ALIGN_CENTER, 0, LastClassPage},
        { "Next...",                            PMENU_ALIGN_CENTER, 0, NextClassPage },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { "View Class Info",                    PMENU_ALIGN_LEFT, 0, EnterInfoClassMenu },
        { "Return to Main Menu",                PMENU_ALIGN_LEFT, 0, ReturnToMain }
};

void ClassMenu(edict_t *ent, pmenu_t *p);

void LastClassPage(edict_t *ent, pmenu_t *p)
{
        int offset = ent->client->resp.tempclass / 8;
        if (!offset)
             return;

        offset--;

        ent->client->resp.tempclass = offset * 8;

        ClassMenu(ent, p);
}

void NextClassPage(edict_t *ent, pmenu_t *p)
{
        int offset;

        offset = (ent->client->resp.tempclass / 8)*8 + 8;
        if (offset >= actual_class)
                return;

        ent->client->resp.tempclass = offset;

        ClassMenu(ent, p);
}

void ClassSelect(edict_t *ent, pmenu_t *p)
{
        int class = p->arg + (ent->client->resp.tempclass / 8)*8;

        if (class >= actual_class)
                ent->client->resp.class = 0;
        else
                ent->client->resp.class = class;

        ent->client->resp.keysbinded = 0;
		
        if (enableclass->value)
                ClassInitPersistant(ent->client, ent->client->resp.class);

        ReturnToMain(ent, p);
}

void ClassMenu(edict_t *ent, pmenu_t *p)
{
        int offset, top, actual;

        if (ent->client->resp.tempclass > actual_class)
                ent->client->resp.tempclass = 0;

        offset = (ent->client->resp.tempclass / 8)*8;
        actual = offset;
        top = offset + 8;

        while (actual < top)
        {
                if (actual < actual_class)
                {
                        classmenu[3 + actual - offset].text = (classtbl[actual].name[0] == '\0') ? "Unnamed" : classtbl[actual].name;
                        classmenu[3 + actual - offset].SelectFunc = ClassSelect;
                }
                else
                {
                        classmenu[3 + actual - offset].text = NULL;
                        classmenu[3 + actual - offset].SelectFunc = NULL;
                }

                actual++;
        }

        classmenu[13].text = "Last...";
        classmenu[13].SelectFunc = LastClassPage;
        classmenu[14].text = "Next...";
        classmenu[14].SelectFunc = NextClassPage;

        if (ent->client->resp.tempclass < 8)
        {
                classmenu[13].text = NULL;
                classmenu[13].SelectFunc = NULL;
        }

        if (actual_class < top)
        {
                classmenu[14].text = NULL;
                classmenu[14].SelectFunc = NULL;
        }

	PMenu_Close(ent);
        PMenu_Open(ent, classmenu, 3+(ent->client->resp.tempclass%8), sizeof(classmenu) / sizeof(pmenu_t));
}

void EnterClassMenu(edict_t *ent, pmenu_t *p)
{
        ent->client->resp.tempclass = ent->client->resp.class;
        ClassMenu(ent, p);
}

//////////////////////
//  InfoClass Menu  //
//////////////////////
void LastInfoClassPage(edict_t *ent, pmenu_t *p);
void NextInfoClassPage(edict_t *ent, pmenu_t *p);

pmenu_t infoclassmenu[] = {
        { "*Wyrm II",                           PMENU_ALIGN_CENTER, 0, NULL },
        { "*Class Information",                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 1, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 2, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 3, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 4, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 5, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 6, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 7, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { "Last...",                            PMENU_ALIGN_CENTER, 0, LastInfoClassPage},
        { "Next...",                            PMENU_ALIGN_CENTER, 0, NextInfoClassPage },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { "Return to Class Selection",          PMENU_ALIGN_LEFT, 0, EnterClassMenu },
        { "Return to Main Menu",                PMENU_ALIGN_LEFT, 0, ReturnToMain }
};

void InfoClassMenu(edict_t *ent, pmenu_t *p);

void LastInfoClassPage(edict_t *ent, pmenu_t *p)
{
        int offset = ent->client->resp.tempclass / 8;
        if (!offset)
             return;

        offset--;

        ent->client->resp.tempclass = offset * 8;

        InfoClassMenu(ent, p);
}

void NextInfoClassPage(edict_t *ent, pmenu_t *p)
{
        int offset;

        offset = (ent->client->resp.tempclass / 8)*8 + 8;
        if (offset >= actual_class)
                return;

        ent->client->resp.tempclass = offset;

        InfoClassMenu(ent, p);
}

void ShowClassPage(edict_t *ent, pmenu_t *p);

void InfoClassSelect(edict_t *ent, pmenu_t *p)
{
        int class = p->arg + (ent->client->resp.tempclass / 8)*8;

        if (class >= actual_class)
                class = 0;

        ent->client->resp.tempclass = class;

        ShowClassPage(ent, p);
/*
        ent->client->resp.keysbinded = 0;
		
                if (enableclass->value)
                        ClassInitPersistant(ent->client, ent->client->resp.class);

        ReturnToMain(ent, p);
*/
}

void InfoClassMenu(edict_t *ent, pmenu_t *p)
{
        int offset, top, actual;

        if (ent->client->resp.tempclass > actual_class)
                ent->client->resp.tempclass = 0;

        offset = (ent->client->resp.tempclass / 8)*8;
        actual = offset;
        top = offset + 8;

        while (actual < top)
        {
                if (actual < actual_class)
                {
                        infoclassmenu[3 + actual - offset].text = (classtbl[actual].name[0] == '\0') ? "Unnamed" : classtbl[actual].name;
                        infoclassmenu[3 + actual - offset].SelectFunc = InfoClassSelect;
                }
                else
                {
                        infoclassmenu[3 + actual - offset].text = NULL;
                        infoclassmenu[3 + actual - offset].SelectFunc = NULL;
                }

                actual++;
        }

        infoclassmenu[13].text = "Last...";
        infoclassmenu[13].SelectFunc = LastInfoClassPage;
        infoclassmenu[14].text = "Next...";
        infoclassmenu[14].SelectFunc = NextInfoClassPage;

        if (ent->client->resp.tempclass < 8)
        {
                infoclassmenu[13].text = NULL;
                infoclassmenu[13].SelectFunc = NULL;
        }

        if (actual_class < top)
        {
                infoclassmenu[14].text = NULL;
                infoclassmenu[14].SelectFunc = NULL;
        }

	PMenu_Close(ent);
        PMenu_Open(ent, infoclassmenu, 3+(ent->client->resp.tempclass%8), sizeof(infoclassmenu) / sizeof(pmenu_t));
}

void EnterInfoClassMenu(edict_t *ent, pmenu_t *p)
{
        ent->client->resp.tempclass = ent->client->resp.class;
        InfoClassMenu(ent, p);
}
void ShowClassPage2(edict_t *ent, pmenu_t *p);

pmenu_t infoclasspage[] = {
        { "*Wyrm II",                           PMENU_ALIGN_CENTER, 0, NULL },
        { "*Class Information",                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { "More...",                            PMENU_ALIGN_RIGHT, 0,  ShowClassPage2},
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { "Return to Class Info",               PMENU_ALIGN_LEFT, 0, InfoClassMenu },
        { "Return to Main Menu",                PMENU_ALIGN_LEFT, 0, ReturnToMain }
};

void ShowClassPage(edict_t *ent, pmenu_t *p)
{
        static char classname[32];
        static char skinname[32];
        static char speedstring[32];
        static char powerstring[32];
        static char resistancestring[32];
        static char jumpstring[32];
        static char knockbackstring[32];
        static char healthstring[32];
        static char maxhealthstring[32];

        int class = ent->client->resp.tempclass;

        sprintf(classname, "Name: %s", classtbl[class].name);
        infoclasspage[2].text = classname;
        
        infoclasspage[3].text = NULL;
        if (classtbl[class].skin[0] && setclassskin->value)
        {
                sprintf (skinname, "Skin: %s", classtbl[class].skin);
                infoclasspage[3].text = skinname;
        }
        
        sprintf (speedstring,      "Speed:      %f", classtbl[class].speed);
        sprintf (powerstring,      "Power:      %f", classtbl[class].power);
        sprintf (resistancestring, "Resistance: %f", classtbl[class].resistance);
        sprintf (jumpstring,       "Jump:       %f", classtbl[class].jump);
        sprintf (knockbackstring,  "Knockback:  %f", classtbl[class].knockback);
        sprintf (healthstring,     "Health:     %3d", classtbl[class].health);
        sprintf (maxhealthstring,  "Max Health: %3d",  classtbl[class].max_health);

        infoclasspage[4].text = healthstring;
        infoclasspage[5].text = maxhealthstring;
        infoclasspage[6].text = speedstring;
        infoclasspage[7].text = powerstring;
        infoclasspage[8].text = resistancestring;
        infoclasspage[9].text = jumpstring;
        infoclasspage[10].text = knockbackstring;

        if (classtbl[class].keys[0])
                infoclasspage[11].text = "This class has key bindings";
        else
                infoclasspage[11].text = NULL;

	PMenu_Close(ent);
        PMenu_Open(ent, infoclasspage, 14, sizeof(infoclasspage) / sizeof(pmenu_t));
}

void EnterShowClassPage3(edict_t *ent, pmenu_t *p);

pmenu_t infoclasspage2[] = {
        { "*Wyrm II",                           PMENU_ALIGN_CENTER, 0, NULL },
        { "*Class Information",                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { "Initial Weapon:",                    PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_CENTER, 0, NULL },
        { "More...",                            PMENU_ALIGN_RIGHT, 0,  EnterShowClassPage3},
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { "Return to Class Info",               PMENU_ALIGN_LEFT, 0, InfoClassMenu },
        { "Return to Main Menu",                PMENU_ALIGN_LEFT, 0, ReturnToMain }
};

void ShowClassPage2(edict_t *ent, pmenu_t *p)
{
        static char maxshellsstring[32];
        static char maxbulletsstring[32];
        static char maxgrenadesstring[32];
        static char maxrocketsstring[32];
        static char maxslugsstring[32];
        static char maxcellsstring[32];
        static char initialweaponstring[64];
        int class = ent->client->resp.tempclass;


        sprintf (maxshellsstring,      "Max Shells:   %d", classtbl[class].max_shells);
        sprintf (maxbulletsstring,     "Max Bullets:  %d", classtbl[class].max_bullets);
        sprintf (maxgrenadesstring,    "Max Grenades: %d", classtbl[class].max_grenades);
        sprintf (maxrocketsstring,     "Max Rockets:  %d", classtbl[class].max_rockets);
        sprintf (maxslugsstring,       "Max Slugs:    %d", classtbl[class].max_slugs);
        sprintf (maxcellsstring,       "Max Cells:    %d", classtbl[class].max_cells);
        sprintf (initialweaponstring, ">%s<", classtbl[class].initial_weapon->pickup_name);

        infoclasspage2[3].text = maxshellsstring;
        infoclasspage2[4].text = maxbulletsstring;
        infoclasspage2[5].text = maxgrenadesstring;
        infoclasspage2[6].text = maxrocketsstring;
        infoclasspage2[7].text = maxslugsstring;
        infoclasspage2[8].text = maxcellsstring;


        infoclasspage2[11].text = initialweaponstring;

	PMenu_Close(ent);
        PMenu_Open(ent, infoclasspage2, 14, sizeof(infoclasspage2) / sizeof(pmenu_t));
}

void CheckShowClassPage(edict_t *ent, pmenu_t *p);

pmenu_t infoclasspage3[] = {
        { "*Wyrm II",                           PMENU_ALIGN_CENTER, 0, NULL },
        { "*Class Information",                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { "Initial Inventory:",                 PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { "More...",                            PMENU_ALIGN_RIGHT, 0,  CheckShowClassPage},
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { "Return to Class Info",               PMENU_ALIGN_LEFT, 0, InfoClassMenu },
        { "Return to Main Menu",                PMENU_ALIGN_LEFT, 0, ReturnToMain }
};

void ShowClassPage3(edict_t *ent, pmenu_t *p);

void CheckShowClassPage(edict_t *ent, pmenu_t *p)
{
        if (ent->client->resp.offset >= 0)
                ShowClassPage3(ent, p);
        else
                ShowClassPage(ent, p);

}

void ShowClassPage3(edict_t *ent, pmenu_t *p)
{
        static char invstring[9][32];
        int class = ent->client->resp.tempclass;
        int offset = ent->client->resp.offset;
        int newoffset = 0;
	int		i;
        int endcheck=true;
        gitem_t *it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
                if (!classtbl[class].inventory[i])
			continue;

                if (offset)
                {
                        offset--;
                        continue;
                }

                if (newoffset >= 9)
                {
                        newoffset = 10;
                        break;
                }

                sprintf(invstring[newoffset], "%-20s %3d", it->pickup_name, classtbl[class].inventory[i]);
                newoffset++;
	}


        if (newoffset == 10)
        {
                endcheck = false;
                newoffset--;
        }

        for (i=0; i<9; i++)
        {
                if (i<newoffset)
                        infoclasspage3[4+i].text = invstring[i];
                else
                        infoclasspage3[4+i].text = NULL;
        }

        if (endcheck)
                ent->client->resp.offset = -1;
        else
                ent->client->resp.offset += newoffset;

	PMenu_Close(ent);
        PMenu_Open(ent, infoclasspage3, 14, sizeof(infoclasspage3) / sizeof(pmenu_t));
}

void EnterShowClassPage3(edict_t *ent, pmenu_t *p)
{
        int i;
        int init_inv;
        int class = ent->client->resp.tempclass;

        init_inv = false;

        for (i=0 ; i<game.num_items ; i++)
	{
                if (!classtbl[class].inventory[i])
			continue;
                
                init_inv = true;
                break;
        }

        if (init_inv)
        {
                ent->client->resp.offset = 0;
                ShowClassPage3(ent, p);
        }
        else
                ShowClassPage(ent, p);
}

///////////////////////////////////////////////////////////////////////

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

void ClassJoinGame(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	ent->svflags &= ~SVF_NOCLIENT;
        ent->flags &= ~FL_NOTARGET;

	ent->client->resp.ctf_state = CTF_STATE_START;

        //Wyrm: class selected!
        ent->client->resp.class_selected = true;

        ClientUserinfoChanged (ent, ent->client->pers.userinfo);

	PutClientInServer (ent);
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
        if (classtbl[ent->client->resp.class].name[0])
                gi.bprintf(PRINT_HIGH, "%s joined the game as %s.\n",
                        ent->client->pers.netname, classtbl[ent->client->resp.class].name);
        else
                gi.bprintf(PRINT_HIGH, "%s joined the game.\n",
                        ent->client->pers.netname);
}

void CTFChaseCam(edict_t *ent, pmenu_t *p);

pmenu_t classjoinmenu[] = {
        { "*Wyrm II",                 PMENU_ALIGN_CENTER, 0, NULL },
        { "*World of Destruction",      PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                         PMENU_ALIGN_CENTER, 0, NULL },
        { NULL,                         PMENU_ALIGN_LEFT, 0, NULL },
        { "Select Class",               PMENU_ALIGN_LEFT, 0, EnterClassMenu },
        { NULL,                         PMENU_ALIGN_LEFT, 0, NULL },
        { "Bind Class Keys",            PMENU_ALIGN_LEFT, 0, BindClassKeys },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { "Join Game",                          PMENU_ALIGN_LEFT, 0, ClassJoinGame },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { "Chase Camera",               PMENU_ALIGN_LEFT, 0, CTFChaseCam },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { NULL,                                 PMENU_ALIGN_LEFT, 0, NULL },
        { "Use [ and ] to move cursor", PMENU_ALIGN_LEFT, 0, NULL },
        { "ENTER to select",    PMENU_ALIGN_LEFT, 0, NULL },
        { "ESC to Exit Menu",   PMENU_ALIGN_LEFT, 0, NULL },
        { "(TAB to Return)",    PMENU_ALIGN_LEFT, 0, NULL },
};

void ClassUpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];

	if (ent->client->chase_target)
                classjoinmenu[10].text = "Leave Chase Camera";
	else
                classjoinmenu[10].text = "Chase Camera";

	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;


        classjoinmenu[2].text = levelname;
}

void ClassOpenJoinMenu(edict_t *ent)
{
        int defaultoption;

        defaultoption = 3;

        ClassUpdateJoinMenu(ent);

	if (ent->client->chase_target)
                defaultoption = 10;

        PMenu_Open(ent, classjoinmenu, defaultoption, sizeof(classjoinmenu) / sizeof(pmenu_t));
}

void ClassReturnToMain(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
        ClassOpenJoinMenu(ent);
}

//Used in g_cmds.c
void Cmd_ShowClass (edict_t *ent)
{
        if (!enableclass->value)
        {
                gi.cprintf(ent, PRINT_HIGH, "Sorry, but classes aren't activated.\n");
                return;
        }
        if (classtbl[ent->client->resp.class].name[0])
                gi.cprintf(ent, PRINT_HIGH, "You're using the %s Class.\n", classtbl[ent->client->resp.class].name);
        else
                gi.cprintf(ent, PRINT_HIGH, "You're using the class number %d.\n", ent->client->resp.class);
};



void Cmd_InfoClass(edict_t *ent)
{
	char		*name;
        int c;
        //Check if a class name

        if (!strlen(gi.args()))
        {
                gi.cprintf (ent, PRINT_HIGH, "Info about what class?\n");
                return;
        }

        name = gi.args();
        c = 0;

        while ((Q_stricmp(name, classtbl[c].name) != 0) && (c < actual_class))
                c++;

        if (c == actual_class)
                c = atoi(name);

        if (c < 0 || c >= actual_class)
                c = 0;

        gi.cprintf (ent, PRINT_HIGH, "--------------------------------------\n");
        gi.cprintf (ent, PRINT_HIGH, "Name: %s\n", classtbl[c].name);
        if (classtbl[c].skin[0] && setclassskin->value)
                gi.cprintf (ent, PRINT_HIGH, "Skin: %s\n", classtbl[c].skin);
        if (classtbl[c].keys[0])
                gi.cprintf (ent, PRINT_HIGH, "This class has key bindings included\n");
        gi.cprintf (ent, PRINT_HIGH, "Speed: %f\n", classtbl[c].speed);
        gi.cprintf (ent, PRINT_HIGH, "Power: %f\n", classtbl[c].power);
        gi.cprintf (ent, PRINT_HIGH, "Resistance: %f\n", classtbl[c].resistance);
        gi.cprintf (ent, PRINT_HIGH, "Jump: %f\n", classtbl[c].jump);
        gi.cprintf (ent, PRINT_HIGH, "Knockback: %f\n", classtbl[c].knockback);
        gi.cprintf (ent, PRINT_HIGH, "Health:       %3d    Max Health:  %3d\n", classtbl[c].health, classtbl[c].max_health);
        gi.cprintf (ent, PRINT_HIGH, "Max Bullets:  %3d    Max Shells:  %3d\n", classtbl[c].max_bullets, classtbl[c].max_shells);
        gi.cprintf (ent, PRINT_HIGH, "Max Grenades: %3d    Max Rockets: %3d\n", classtbl[c].max_grenades, classtbl[c].max_rockets);
        gi.cprintf (ent, PRINT_HIGH, "Max Slugs:    %3d    Max Cells:   %3d\n", classtbl[c].max_slugs, classtbl[c].max_cells);
        gi.cprintf (ent, PRINT_HIGH, "--------------------------------------\n");
}

//Initial Weapon function
gitem_t *GetInitialWeapon(edict_t *ent)
{
        if (!ent)
                return NULL;
        if (!ent->client)
                return NULL;

        return classtbl[ent->client->resp.class].initial_weapon;
}
