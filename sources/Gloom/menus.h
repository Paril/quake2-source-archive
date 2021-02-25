/*
    Quake II Glooom, a total conversion mod for Quake II
    Copyright (C) 1999-2007  Gloom Developers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

void OpenClassMenuWrapper (edict_t *ent);
void CTFChaseCam(edict_t *ent);
void Back(edict_t *ent);
void SpawnClass(edict_t *ent);
void Activate_Upgrade (edict_t *ent);

void ExitQueue(edict_t*ent);
void ExitQueueShowMenu(edict_t*ent);

void SkipJoinQueueMenu(edict_t*ent);

void JoinHumanQueueMenu(edict_t*ent);
void JoinAlienQueueMenu(edict_t*ent);

//  build menus

pmenu_t breeder_menu[] = {
        { "",   0, NULL },
        { "",   0, NULL },
        { "", 0, lay_egg },
        { "", 0, lay_obstacle },
        { "", 0, lay_spiker },
        { "", 0, lay_healer },
        { "", 0, lay_gasser },
        { NULL, 0, NULL }
};

pmenu_t engineer_menu[] = {
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, create_teleporter},
        { "", 0, create_turret},
        { "", 0, create_mgturret},
        { "", 0, create_detector},
        { "", 0, create_tripwire},
        { "", 0, create_depot},
        { NULL, 0, NULL }
};

//  team menus

pmenu_t alien_menu[] = {        
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, Back },
        { NULL, 0, NULL }
};
                        
pmenu_t alien_menu_exit[] = {   
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, NULL },
        { "   Leave queue", 0, ExitQueue },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, Back },
        { NULL, 0, NULL }
};
                        
pmenu_t human_menu[] = {        
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, Back },
        { NULL, 0, NULL }
};

pmenu_t human_menu_exit[] = {   
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, SpawnClass },
        { "", 0, NULL },
        { "   Leave queue", 0, ExitQueue },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, Back },
        { NULL, 0, NULL }
};

pmenu_t upgrade_menu_exterm[] = 
{
                { "*Exterminator Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
                //{ "1 - Lightweight Armour", 0, Activate_Upgrade },
		{ "", 0, NULL },
                { "1 - +30 Cell Pack", 0, Activate_Upgrade },
                { "1 - Advanced Coolant", 0, Activate_Upgrade },
		{ "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_mech[] = 
{
                { "*Mech Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
                { "1 - Modular Armour", 0, Activate_Upgrade },
                { "2 - Dual C4 Pack", 0, Activate_Upgrade },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_guardian[] = 
{
                { "*Guardian Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
                { "2 - Improved Mimetism", 0, Activate_Upgrade },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_commando[] = 
{
                { "*Commando Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
		{ "", 0, NULL },//{ "1 - Lightweight Grenades", 0, Activate_Upgrade },
                { "1 - Shrapnel", 0, Activate_Upgrade },
                { "1 - C4 Kit", 0, Activate_Upgrade },
		{ "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_ht[] = 
{
                { "*HT Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
		{ "", 0, NULL },//	{ "1 - Mute Warning", 0, Activate_Upgrade },
                { "", 0, NULL },//	{ "1 - Shrapnel", 0, Activate_Upgrade },
	        { "1 - Mortar Add-On", 0, Activate_Upgrade },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_st[] =
{
                { "*ST Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
		{ "1 - Equipment Kit", 0, Activate_Upgrade },
                { "", 0, NULL },
	        { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_hatch[] = 
{
                { "*Hatchling Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
                { "1 - Mimetic Skin", 0, Activate_Upgrade },
		{ "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_drone[] = 
{
                { "*Drone Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
                { "1 - Mucus Glands", 0, Activate_Upgrade },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_kami[] = 
{
                { "*Kamikaze Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
                { "1 - Spike Pouch", 0, Activate_Upgrade },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_grunt[] = 
{
                { "*Grunt Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
                { "1 - Full Auto Mode", 0, Activate_Upgrade },
                { "1 - Aimbot", 0, Activate_Upgrade },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_engi[] = 
{
                { "*Engineer Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
                { "1 - Dual Wielder", 0, Activate_Upgrade },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_wraith[] = 
{
                { "*Wraith Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
                { "1 - Poison Spit", 0, Activate_Upgrade },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_bio[] = 
{
                { "*Biotech Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
                { "1 - Medtech Kit", 0, Activate_Upgrade },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t upgrade_menu_stalker[] = 
{
                { "*Stalker Upgrades", PMENU_ALIGN_CENTER, NULL  },
                { "", 0, NULL },
                { "", 0, NULL },
                { "1 - Spike Sac", 0, Activate_Upgrade },
                { "1 - Aerodynamic Scales", 0, Activate_Upgrade },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "", 0, NULL },
                { "Close", 0, PMenu_Close },
                { NULL, 0, NULL }
};

pmenu_t team_menu[] = {
        { "", 0, NULL },
        { "", 0, NULL },
//      { "", 0, OpenClassMenuWrapper },
        { "", 0, JoinAlienQueueMenu },
        { "", 0, NULL },   // dynamic data
//      { "", 0, OpenClassMenuWrapper },
        { "", 0, JoinHumanQueueMenu },
        { "", 0, NULL },   // dynamic data
        { "", 0, CTFChaseCam },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },        //motd
        { "", 0, NULL },        //motd
        { "", 0, NULL },        //motd
        { "", 0, NULL },
        { "", 0, NULL },
        { "*"GLOOMVERSION,                      PMENU_ALIGN_RIGHT, NULL },
        { NULL, 0, NULL }
};

pmenu_t menu_exitqueue[] = {
        { "", 0, NULL },
        { "", 0, NULL },
        { "Leave queue", 0, ExitQueueShowMenu},
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "Toggle chasecam", 0, CTFChaseCam},
        { "", 0, NULL },
        { "", 0, NULL },        //motd
        { "", 0, NULL },        //motd
        { "", 0, NULL },        //motd
        { "", 0, NULL },
        { "", 0, NULL },
        { "*"GLOOMVERSION,                      PMENU_ALIGN_RIGHT, NULL },
        { NULL, 0, NULL }
};

pmenu_t menu_join_admin[] = {
        { "", 0, NULL },
        { "", 0, NULL },
        { "Leave queue", 0, ExitQueueShowMenu},
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "", 0, NULL },
        { "Toggle chasecam", 0, CTFChaseCam},
        { "", 0, NULL },
        { "Skip queue", 0, SkipJoinQueueMenu},
        { "", 0, NULL },
        { "", 0, NULL },        //motd
        { "", 0, NULL },        //motd
        { "", 0, NULL },        //motd
        { "", 0, NULL },
        { "*"GLOOMVERSION,                      PMENU_ALIGN_RIGHT, NULL },
        { NULL, 0, NULL }
};
