#include "g_local.h"

void SelectSpawnPoint      (edict_t *ent, vec3_t origin, vec3_t angles);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void PrintOtherClass       (entity *ent);
void Set_ClassEquipment    (gclient_t *client, int j);
void strcpy_ (char *s, char *fmt, ...);
void Rip_SkinHim           (edict_t *ent);
void Cell_VicMake          (edict_t *ent);

char *cla_names[] = 
{
	NULL,
	"warrior",
	"necromancer",
	"thief",
	"infantry",
	"miner",
	"scientist",
	"flamer",
	"trooper",
	"ghost",
	NULL
};

void SetCustomEquipment (gclient_t *client)
{
	char *s;
	int arm;

	s = client->resp.as;
	arm = client->resp.arm;

	Set_ClassEquipment (client, client->resp.it);
   	client->pers.inventory[ITEM_INDEX(FindItem(s))] = arm;
}

void sparks (edict_t *ent)
{
    // send effect
	muzzleflash (ent, MZ_LOGIN);
}

void MyCustom_Sel (edict_t *ent, int choice) 
{
	switch (choice)
	{
	case 0:
       ent->client->resp.it = 1;
	   stuffcmd (ent, "menu_speed\n");
       break;
	case 1:
	   ent->client->resp.it = 2;
	   stuffcmd (ent, "menu_speed\n");
       break;
	case 2:
	   ent->client->resp.it = 3;
	   stuffcmd (ent, "menu_speed\n");
       break;
	case 3:
	   ent->client->resp.it = 4;
	   stuffcmd (ent, "menu_speed\n");
       break;
	case 4:
	   ent->client->resp.it = 5;
	   stuffcmd (ent, "menu_speed\n");
       break;
	case 5:
	   ent->client->resp.it = 6;
	   stuffcmd (ent, "menu_speed\n");
       break;
	case 6:
	   ent->client->resp.it = 7;
	   stuffcmd (ent, "menu_speed\n");
       break;
    case 7:
	   ent->client->resp.it = 8;
	   stuffcmd (ent, "menu_speed\n");
	   break;
	}
} // MyWave_Sel

void Cmd_Custom_f (edict_t *ent)
{
	// Check to see if the menu is already open

   if (ent->client->showscores || ent->client->showinventory || 
        ent->client->showmenu || ent->client->showmsg)
        return;

   // send the layout
   
   Menu_Title(ent,"Select your equipment");
   Menu_Add(ent,"Necromancer");
   Menu_Add(ent,"Thief");
   Menu_Add(ent,"Infantry");
   Menu_Add(ent,"Miner");
   Menu_Add(ent,"Scientist");
   Menu_Add(ent,"Flamer");
   Menu_Add(ent,"Trooper");
   Menu_Add(ent,"Warrior");

   // Setup the User Selection Handler

   ent->client->usr_menu_sel = MyCustom_Sel;
   Menu_Open(ent);

} // Cmd_MyWave_f

void MySpeed_Sel (edict_t *ent, int choice) 
{

	switch (choice)
	{
	case 0:
	   ent->client->resp.maxfbspeed = 4;
       stuffcmd (ent, "menu_armor\n");
       break;
	case 1:
	   ent->client->resp.maxfbspeed = 2;
       stuffcmd (ent, "menu_armor\n");
       break;
	case 2:
	   ent->client->resp.maxfbspeed = 5;
       stuffcmd (ent, "menu_armor\n");
       break;
	case 3:
	   ent->client->resp.maxfbspeed = 3;
       stuffcmd (ent, "menu_armor\n");
       break;
	case 4:
	   ent->client->resp.maxfbspeed = 4;
       stuffcmd (ent, "menu_armor\n");
       break;
	}
} // MyWave_Sel

void Cmd_Speed_f (edict_t *ent)
{
	// Check to see if the menu is already open

   if (ent->client->showscores || ent->client->showinventory || 
        ent->client->showmenu || ent->client->showmsg)
        return;

   // send the layout
   
   Menu_Title(ent, "Select your speed");
   Menu_Add(ent,"25  * x"); //400
   Menu_Add(ent,"50  * x"); //100
   Menu_Add(ent,"100 * x");//250
   Menu_Add(ent,"150 * x");//150
   Menu_Add(ent,"200 * x");//300
    // Setup the User Selection Handler

   ent->client->usr_menu_sel = MySpeed_Sel;
   Menu_Open(ent);

} // Cmd_MyWave_f

void MyArmor_Sel (edict_t *ent, int choice) 
{
	switch (choice)
	{
	case 0:
	ent->client->resp.as = "Body Armor";
	ent->client->resp.arm = 250/ent->client->resp.maxfbspeed;
	stuffcmd (ent, "menu_done\n");
       break;
	case 1:
	ent->client->resp.as = "Jacket Armor";
	ent->client->resp.arm = 650/ent->client->resp.maxfbspeed;
	stuffcmd (ent, "menu_done\n");
		break;
	case 2:
    ent->client->resp.as = "Combat Armor";
	ent->client->resp.arm = 325/ent->client->resp.maxfbspeed;
 	stuffcmd (ent, "menu_done\n");
       break;
	}
} // MyWave_Sel

void Cmd_Armor_f (edict_t *ent)
{
	// Check to see if the menu is already open

   if (ent->client->showscores || ent->client->showinventory || 
        ent->client->showmenu || ent->client->showmsg)
        return;

   // send the layout
   
   Menu_Title(ent, "Select your armor");
   Menu_Add(ent,"Jacket Armor");
   Menu_Add(ent,"Combat Armor");
   Menu_Add(ent,"Body Armor  ");

   // Setup the User Selection Handler

   ent->client->usr_menu_sel = MyArmor_Sel;
   Menu_Open(ent);

} // Cmd_MyWave_f

void MyDone_Sel (edict_t *ent, int choice) 
{
    ClassFunction (ent, 10);
} // MyWave_Sel

void Cmd_Done_f (edict_t *ent)
{
    if (ent->playerclass > 0)
		return;

	// Check to see if the menu is already open

   if (ent->client->showscores || ent->client->showinventory || 
        ent->client->showmenu || ent->client->showmsg)
        return;

   // send the layout
   
   Menu_Title(ent, "Done");
   Menu_Add(ent,"Done");

   gi.dprintf ("%s %i\n", ent->client->resp.as, ent->client->resp.arm);
   // Setup the User Selection Handler

   ent->client->usr_menu_sel = MyDone_Sel;
   Menu_Open(ent);

} // Cmd_MyWave_f

void SetClassName (edict_t *ent)
{
	switch (ent->playerclass)
	{
		case 1:
			ent->playerclasss = "warrior";
			break;
		case 2:
			ent->playerclasss = "necromancer";
			break;
		case 3:
			ent->playerclasss = "thief";
			break;
		case 4:
			ent->playerclasss = "infantry";
			break;
		case 5:
			ent->playerclasss = "miner";
			break;
		case 6:
			ent->playerclasss = "scientist";
			break;
		case 7:
			ent->playerclasss = "flamer";
			break;
        case 8:
			ent->playerclasss = "trooper";
			break;
		case 9:
			ent->playerclasss = "ghost";
			break;
		case 10:
			ent->playerclasss = "custom";
			break;
	}
}

void SetClassMass (edict_t *ent)
{
	switch (ent->playerclass)
	{
	   case 1:
      	   ent->mass = 500;
		   break;
	   case 2:
		   ent->mass = 200;
		   break;
	   case 3:
		   ent->mass = 100;
		   break;
       case 4:
		   ent->mass = 202;
		   break;
       case 5:
		   ent->mass = 300;
		   break;
	   case 6:
		   ent->mass = 100;
		   break;
	   case 7:
		   ent->mass = 300;
		   break;
       case 8:
		   ent->mass = 200;
		   break;
	   case 10:
		   ent->mass = 200;
		   break;
       case 9:
	       ent->mass = 100;
		   break;
	}
}

void SetTeamState (edict_t *ent)
{
	switch (ent->playerclass)
	{
	   case 1:
      	   ent->teamstate = 1; // defence state
		   break;
	   case 2:
		   ent->teamstate = 2; // attack state
		   break;
	   case 3:
		   ent->teamstate = 2; // attack state
		   break;
       case 4:
		   ent->teamstate = 2; //attack state
		   break;
       case 5:
		   ent->teamstate = 3; // middle state
		   break;
	   case 6:
		   ent->teamstate = 1; //defence state
		   break;
	   case 7:
		   ent->teamstate = 2; //attack state
		   break;
       case 8:
		   ent->teamstate = 2;
		   break;
       case 9:
		   ent->teamstate = 3;
		   break;
       case 10:
		   ent->teamstate = 3;
		   break;
	}
}

void Print_ClassProperties (edict_t *ent)
{
	switch (ent->playerclass)
	{
	   case 1:
       gi.centerprintf (ent,"WARRIOR\n\nWeapons:\nRocket Launcher, Machinegun.\nGrenades: Rail.\n\nArmor: Body, %i.\nSpeed: %i.\n", ent->client->resp.arm, ent->ClassSpeed);
	   break;
	   case 2:
       gi.centerprintf (ent,"NECROMANCER\n\nWeapon:\nHyperblaster.\nGrenades: Leapfrog.\n\nArmor: Combat, %i.\nSpeed: %i.\n", ent->client->resp.arm, ent->ClassSpeed);
	   break;
	   case 3:
       gi.centerprintf (ent,"THIEF\n\nWeapons:\nShotgun, Super Shotgun.\nGrenades: Bounce.\n\n\nArmor: Combat, %i.\nSpeed: %i.\n", ent->client->resp.arm, ent->ClassSpeed);
	   break;
	   case 4:
       gi.centerprintf (ent,"INFANTRY\n\nWeapons:\nMachinegun, Chaingun.\nGrenades: Smoke.\n\nArmor: Jacket, %i.\nSpeed: %i.\n", ent->client->resp.arm, ent->ClassSpeed);
	   break;
	   case 5:
       gi.centerprintf (ent,"MINER\n\nWeapons:\nPipe Launcher, Grenade Launcher.\nGrenades: Cluster.\n\nArmor: Jacket, %i.\nSpeed: %i.\n", ent->client->resp.arm, ent->ClassSpeed);
	   break;
	   case 6:
       gi.centerprintf (ent,"SCIENTIST\n\nWeapon:\nShotgun, Plasma Rifle.\nGrenades: Concussion.\n\nArmor: Combat, %i.\nSpeed: %i.\n", ent->client->resp.arm, ent->ClassSpeed);
	   break;
	   case 7:
	   gi.centerprintf (ent,"FLAMER\n\nWeapons:\nFlame Launcher, Flamethrower.\nGrenades: Flame.\n\nArmor: Body, %i.\nSpeed: %i.\n", ent->client->resp.arm, ent->ClassSpeed);
	   break;
	   case 8:
       gi.centerprintf (ent,"TROOPER\n\nWeapons:\nSuper Shotgun, BFG10k.\nGrenades: Plasma.\n\nArmor: Jacket, %i.\nSpeed: %i.\n", ent->client->resp.arm, ent->ClassSpeed);
	   break;
	   case 9:
       gi.centerprintf (ent,"GHOST\n\nWeapons:\nHyperblaster (with more powerful cells).\nGrenades: Tag.\n\nArmor: None.\nSpeed: %i\n", ent->ClassSpeed);
	   break;
	   case 10:
       gi.centerprintf (ent,"CUSTOM\n\nArmor: %s, %i.\nSpeed: %i.\n", ent->client->resp.arm, ent->client->pers.inventory[ArmorIndex (ent)], ent->client->resp.maxfbspeed * 50);
	   break;
	}
}

void Set_ClassArmor (gclient_t *client, int i)
{
	switch (i)
	{
	case 1:
		client->resp.armortype = ARMOR_BODY;
		client->resp.arm = 40;
    break;
	case 2:
		client->resp.armortype = ARMOR_COMBAT;
		client->resp.arm = 50;
    break;
	case 3:
		client->resp.armortype = ARMOR_COMBAT;
		client->resp.arm = 40;
    break;
	case 4:
        client->resp.armortype = ARMOR_JACKET;
		client->resp.arm = 40;
	break;
	case 5:
     	client->resp.armortype = ARMOR_JACKET;
     	client->resp.arm = 80;
 	break;
	case 6:
     	client->resp.armortype = ARMOR_COMBAT;
    	client->resp.arm = 80;
	break;
	case 7:
     	client->resp.armortype = ARMOR_BODY;
    	client->resp.arm = 20;
	break;
	case 8:
    	client->resp.armortype = ARMOR_JACKET;
    	client->resp.arm = 60;
	break;
	case 9:
    	client->resp.armortype = 0;
    	client->resp.arm = 0;
	break;

	default:
        client->resp.armortype = -1;
	}
}

void Set_ClassEquipment (gclient_t *client, int j)
{
	gitem_t		*item;

	memset (&client->pers, 0, sizeof(client->pers));

    // Init Flood Protection Variables 
    client->flood_num_msgs = 0; 
    client->flood_post_time = 0; 
    client->flood_timer = 0; 

	switch (j)
	{
	case 1:
		client->pers.max_health		= 90;
		client->pers.health	= 90;

		client->pers.max_bullets	= 100;
		client->pers.max_shells		= 0;
		client->pers.max_rockets	= 50;
		client->pers.max_grenades	= 5;
		client->pers.max_cells		= 0;
		client->pers.max_slugs		= 0;

       	item = FindItem("Blaster");
      	client->pers.selected_item = ITEM_INDEX(item);
    	client->pers.inventory[client->pers.selected_item] = 1;

	    item = FindItem("Grenades");
	    client->pers.selected_item = ITEM_INDEX(item);
	    client->pers.inventory[client->pers.selected_item] = 5;

	    item = FindItem("Machinegun");
	    client->pers.selected_item = ITEM_INDEX(item);
	    client->pers.inventory[client->pers.selected_item] = 1;

	    client->pers.lastweapon = item;

	    item = FindItem("Rocket Launcher");
	    client->pers.selected_item = ITEM_INDEX(item);
    	client->pers.inventory[client->pers.selected_item] = 1;

	    client->pers.weapon = item;

	    item = FindItem("rockets");
	    client->pers.selected_item = ITEM_INDEX(item);
	    client->pers.inventory[client->pers.selected_item] += client->pers.max_rockets;

	    item = FindItem("bullets");
	    client->pers.selected_item = ITEM_INDEX(item);
      	client->pers.inventory[client->pers.selected_item] += client->pers.max_bullets;

		client->resp.arm = 200;
		client->resp.as = "Body Armor";
	break;
	case 2:
		client->pers.max_health		= 90;
		client->pers.health	= 90;

		client->pers.max_bullets	= 0;
		client->pers.max_shells		= 0;
		client->pers.max_rockets	= 0;
		client->pers.max_grenades	= 10;
		client->pers.max_cells		= 100;
		client->pers.max_slugs		= 0;
    
	    item = FindItem("Blaster");
		client->pers.selected_item = ITEM_INDEX(item);
     	client->pers.inventory[client->pers.selected_item] = 1;

    	item = FindItem("Grenades");
    	client->pers.selected_item = ITEM_INDEX(item);
    	client->pers.inventory[client->pers.selected_item] = 10;

     	item = FindItem("Magic");
      	client->pers.selected_item = ITEM_INDEX(item);
    	client->pers.inventory[client->pers.selected_item] = 1;

       	client->pers.lastweapon = item;

      	item = FindItem("Hyperblaster");
	    client->pers.selected_item = ITEM_INDEX(item);
       	client->pers.inventory[client->pers.selected_item] = 1;

	    client->pers.weapon = item;

	    item = FindItem("cells");
	    client->pers.selected_item = ITEM_INDEX(item);
	    client->pers.inventory[client->pers.selected_item] += client->pers.max_cells;

		client->resp.arm = 50;
		client->resp.as = "Combat Armor";
	break;
	case 3:
		client->pers.max_health		= 110;
		client->pers.health	        = 110;

		client->pers.max_bullets	= 0;
		client->pers.max_shells		= 100;
		client->pers.max_rockets	= 0;
		client->pers.max_grenades	= 5;
      	client->pers.max_cells		= 0;
     	client->pers.max_slugs		= 0;

     	item = FindItem("Blaster");
    	client->pers.selected_item = ITEM_INDEX(item);
     	client->pers.inventory[client->pers.selected_item] = 1;

      	item = FindItem("Grenades");
    	client->pers.selected_item = ITEM_INDEX(item);
      	client->pers.inventory[client->pers.selected_item] = 10;
 
	    item = FindItem("Shotgun");
        client->pers.selected_item = ITEM_INDEX(item);
      	client->pers.inventory[client->pers.selected_item] = 1;

      	client->pers.lastweapon = item;

      	item = FindItem("Super Shotgun");
      	client->pers.selected_item = ITEM_INDEX(item);
    	client->pers.inventory[client->pers.selected_item] = 1;

     	client->pers.weapon = item;

       	item = FindItem("shells");
        client->pers.selected_item = ITEM_INDEX(item);
      	client->pers.inventory[client->pers.selected_item] += client->pers.max_shells;

		client->resp.arm = 40;
		client->resp.as = "Combat Armor";
	break;
	case 4:
		client->pers.max_health		= 60;
		client->pers.health	        = 60;

		client->pers.max_bullets	= 200;
		client->pers.max_shells		= 0;
		client->pers.max_rockets	= 0;
		client->pers.max_grenades	= 5;
		client->pers.max_cells		= 0;
		client->pers.max_slugs		= 30;

		item = FindItem("Grenades");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 5;

		item = FindItem("Blaster");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

		item = FindItem("Chaingun");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

		client->pers.lastweapon = item;

		item = FindItem("Railgun");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

		client->pers.weapon = item;

		item = FindItem("bullets");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] += client->pers.max_bullets;

		item = FindItem("slugs");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] += client->pers.max_slugs;

		client->resp.arm = 40;
		client->resp.as = "Jacket Armor";
	break;
	case 5:
		client->pers.max_health		= 80;
		client->pers.health	= 80;

		client->pers.max_bullets	= 0;
		client->pers.max_shells		= 0;
		client->pers.max_rockets	= 0;
		client->pers.max_grenades	= 70;
		client->pers.max_cells		= 0;
		client->pers.max_slugs		= 0;

		item = FindItem("Blaster");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

		item = FindItem("Grenades");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] += client->pers.max_grenades;

		item = FindItem("Pipebomb Launcher");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

		client->pers.lastweapon = item;

		item = FindItem("Grenade Launcher");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

		client->pers.weapon = item;

		item = FindItem("pipebombs");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] += 5;

		client->resp.arm = 80;
		client->resp.as = "Jacket Armor";
	break;
	case 6:
		client->pers.max_health		= 150;
		client->pers.health	= 150;

		client->pers.max_bullets	= 50;
		client->pers.max_shells		= 50;
		client->pers.max_rockets	= 10;
		client->pers.max_grenades	= 5;
		client->pers.max_cells		= 70;
		client->pers.max_slugs		= 30;

		item = FindItem("Grenades");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 10;

     	item = FindItem("Blaster");
     	client->pers.selected_item = ITEM_INDEX(item);
      	client->pers.inventory[client->pers.selected_item] = 1;

     	client->pers.lastweapon = item;

     	item = FindItem("Plasma Rifle");
     	client->pers.selected_item = ITEM_INDEX(item);
     	client->pers.inventory[client->pers.selected_item] = 1;

      	client->pers.weapon = item;
 
     	item = FindItem("shells");
     	client->pers.selected_item = ITEM_INDEX(item);
    	client->pers.inventory[client->pers.selected_item] = 50;

      	item = FindItem("cells");
      	client->pers.selected_item = ITEM_INDEX(item);
      	client->pers.inventory[client->pers.selected_item] = 70;

     	item = FindItem("slugs");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 30;

		item = FindItem("rockets");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 10;

		item = FindItem("bullets");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 20;

		client->resp.arm = 80;
		client->resp.as = "Combat Armor";
	break;
	case 7:
       	client->pers.max_health		= 90;
    	client->pers.health	= 90;

      	client->pers.max_bullets	= 0;
     	client->pers.max_shells		= 0;
      	client->pers.max_rockets	= 0;
      	client->pers.max_grenades	= 0;
     	client->pers.max_cells		= 50;
     	client->pers.max_slugs		= 0;

    	item = FindItem("Blaster");
    	client->pers.selected_item = ITEM_INDEX(item);
    	client->pers.inventory[client->pers.selected_item] = 1;
 
     	item = FindItem("Grenades");
    	client->pers.selected_item = ITEM_INDEX(item);
    	client->pers.inventory[client->pers.selected_item] = 15;

        item = FindItem("Flame Launcher");
       	client->pers.selected_item = ITEM_INDEX(item); 
     	client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.lastweapon = item;

	    item = FindItem("Flamethrower");
	    client->pers.selected_item = ITEM_INDEX(item);
      	client->pers.inventory[client->pers.selected_item] = 1;

    	client->pers.weapon = item;

	    item = FindItem("cells");
     	client->pers.selected_item = ITEM_INDEX(item);
    	client->pers.inventory[client->pers.selected_item] = 50;

    	client->resp.arm = 20;
      	client->resp.as = "Body Armor";
	break;
	case 8:
     	client->pers.max_health		= 80;
     	client->pers.health	= 80;

       	client->pers.max_bullets	= 0;
     	client->pers.max_shells		= 50;
     	client->pers.max_rockets	= 0;
     	client->pers.max_grenades	= 10;
    	client->pers.max_cells		= 50;
     	client->pers.max_slugs		= 0;

	    item = FindItem("Blaster");
    	client->pers.selected_item = ITEM_INDEX(item);
     	client->pers.inventory[client->pers.selected_item] = 1;

     	item = FindItem("Grenades");
    	client->pers.selected_item = ITEM_INDEX(item);
    	client->pers.inventory[client->pers.selected_item] = 10;

        item = FindItem("Super Shotgun");
      	client->pers.selected_item = ITEM_INDEX(item);
     	client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.lastweapon = item;

    	item = FindItem("BFG10k");
     	client->pers.selected_item = ITEM_INDEX(item);
	    client->pers.inventory[client->pers.selected_item] = 1;

     	client->pers.weapon = item;

	    item = FindItem("cells");
     	client->pers.selected_item = ITEM_INDEX(item);
      	client->pers.inventory[client->pers.selected_item] = 50;

    	item = FindItem("shells");
    	client->pers.selected_item = ITEM_INDEX(item);
     	client->pers.inventory[client->pers.selected_item] = 50;

      	client->resp.arm = 60;
     	client->resp.as = "Jacket Armor";
	break;
	case 9:
     	client->pers.max_health		= 110;
     	client->pers.health	= 110;

       	client->pers.max_bullets	= 0;
     	client->pers.max_shells		= 0;
     	client->pers.max_rockets	= 0;
     	client->pers.max_grenades	= 15;
    	client->pers.max_cells		= 100;
     	client->pers.max_slugs		= 0;

	    item = FindItem("Blaster");
    	client->pers.selected_item = ITEM_INDEX(item);
     	client->pers.inventory[client->pers.selected_item] = 1;

        client->pers.lastweapon = item;

	    item = FindItem("Grenades");
    	client->pers.selected_item = ITEM_INDEX(item);
     	client->pers.inventory[client->pers.selected_item] = 15;

	    item = FindItem("Hyperblaster");
    	client->pers.selected_item = ITEM_INDEX(item);
     	client->pers.inventory[client->pers.selected_item] = 1;

     	client->pers.weapon = item;
	break;
	default:
	break;
	}

	if (client->resp.it == 0 && j != 9)
	{
		item = FindItem(client->resp.as);
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = client->resp.arm;
	}

	client->pers.connected = true;
}
   
void ClassFunction (edict_t *ent, int i)
{
	vec3_t	spawn_origin, spawn_angles;
	client_persistant_t	saved;
	client_respawn_t	resp;
	int d, o;
	char		userinfo[MAX_INFO_STRING];
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};

    SelectSpawnPoint (ent, spawn_origin, spawn_angles);
    
	resp = ent->client->resp;

	if (i < 10)
		ent->client->resp.it = 0;

	memcpy (userinfo, ent->client->pers.userinfo, sizeof(userinfo));
	if (i == 10) 
		SetCustomEquipment (ent->client);
	else
		Set_ClassEquipment (ent->client, i);
    ClientUserinfoChanged (ent, userinfo);

	// clear everything but the persistant data
	saved = ent->client->pers;
	memset (ent->client, 0, sizeof(*ent->client));
	ent->client->pers = saved;
    ent->client->resp = resp;

	if (ent->client->pers.health <= 0)
	{
		if (i == 10)
			SetCustomEquipment (ent->client);
		else
			Set_ClassEquipment (ent->client, i);
	}

	FetchClientEntData (ent);

	ent->model = "players/male/tris.md2";
	ent->client = &game.clients[ent-g_edicts-1];
	ent->s.frame = 0;
	ent->burnout = 0;
    ent->style = 1;
	ent->groundentity = NULL;
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 7;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->DrunkTime = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;
	ent->svflags &= ~SVF_NOCLIENT;
	o = ent->playerclass;
	ent->lastclass = i;
	ent->client->gr_type = 1;
	ent->playerclass = ent->lastclass;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	if (ent->lasersight)
		G_FreeEdict (ent->lasersight);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(ent->client->ps));

	ent->client->blindBase = 0;
    ent->client->blindTime = 0;

	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
	ent->client->ps.fov = 90;
	ent->client->ps.pmove.origin[0] = spawn_origin[0]*8;
	ent->client->ps.pmove.origin[1] = spawn_origin[1]*8;
	ent->client->ps.pmove.origin[2] = spawn_origin[2]*8;

	// clear entity state values
	ent->s.effects = 0;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;
	ent->s.origin[2] += 2;	// make sure off ground
	VectorCopy (spawn_origin, ent->s.origin);

	// set the delta angle
	for (d=0 ; d<3 ; d++)
		ent->client->ps.pmove.delta_angles[d] = ANGLE2SHORT(spawn_angles[d] - ent->client->resp.cmd_angles[d]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;

	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
	VectorCopy (ent->s.angles, ent->client->v_angle);

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

    Rip_SetSpeed (ent);
	Rip_SetSkin (ent);
	Set_ClassArmor (ent->client, ent->playerclass);

	if (o != i)
	{
		SetClassMass(ent);
		SetClassName (ent);
    	tprintf (ent, ent, MOD_NORMAL, PRINT_HIGH, "%s's class is %s\n", ent->client->pers.netname, ent->playerclasss);
		PrintOtherClass (ent);
   		Print_ClassProperties(ent);
        SetTeamState (ent);

		if (atoi(Info_ValueForKey(ent->client->pers.userinfo, "exec_class")) > 0)
		{
			stuffcmd (ent, "exec cl_cfg/");
			stuffcmd (ent, ent->playerclasss);
			stuffcmd (ent, ".cfg\n");
		}
	}

	// force the current weapon up
	ent->client->newweapon = ent->client->pers.weapon;
	ChangeWeapon (ent);

	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;

	if (ent->playerclass == 9)
	    Cell_VicMake (ent); 
}

void Rip_SetSkin (edict_t *ent)
{
     if (ent->playerclass == 1 || ent->playerclass == 5 || ent->playerclass == 7)
		 strcpy (ent->skincheck, "male/");
	 else if (ent->playerclass == 2)
		 strcpy (ent->skincheck, "necro/");
	 else if (ent->playerclass == 3)
		 strcpy (ent->skincheck, "female/");
	 else if (ent->playerclass == 6)
		 strcpy (ent->skincheck, "waste/");
	 else
		 strcpy (ent->skincheck, "cyborg/");

	 if (ent->client->resp.s_team == 2)
		 strcat (ent->skincheck, "4");
	 else if (ent->client->resp.s_team == 1)
		 strcat (ent->skincheck, "13");

	 stuffcmd (ent, "skin ");
	 stuffcmd (ent, ent->skincheck);
	 stuffcmd (ent, "\n");

     Info_SetValueForKey(ent->client->pers.userinfo, "skin", ent->skincheck);

	 Rip_SkinHim (ent);

	 gi.linkentity (ent);

	 if (!(ent->ripstate & STATE_SKIN))
		 ent->ripstate |= STATE_SKIN;
}

void Rip_SkinHim (edict_t *ent)
{
	int playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, ent->skincheck) );
}

void stuffcmd_speed (edict_t *ent, char s[10])
{
	stuffcmd (ent, "cl_forwardspeed ");
	stuffcmd (ent, s);
	stuffcmd (ent, "\n");
    stuffcmd (ent, "cl_sidespeed ");
	stuffcmd (ent, s);
	stuffcmd (ent, "\n");
}
    
void Rip_SetSpeed (edict_t *ent)
{
	char s[10];

    switch (ent->playerclass)
	{
    	case 0:
			ent->ClassSpeed = 225;
		break;
        case 1:
			ent->ClassSpeed = 175;
	    break;
        case 2:
			ent->ClassSpeed = 225;
		break;
		case 3:
			ent->ClassSpeed = 320;
	    break;
        case 4:
			ent->ClassSpeed = 150;
        break;
		case 5:
			ent->ClassSpeed = 180;
		break;
        case 6:
			ent->ClassSpeed = 315;
        break;
		case 7:
			ent->ClassSpeed = 225;
	    break;
		case 8:
			ent->ClassSpeed = 240;
		break;
		case 9:
			ent->ClassSpeed = 150;
		break;
        case 10:
		    ent->ClassSpeed = ent->client->resp.maxfbspeed * 45;
		break;
	} 

	if (ent->ripstate & STATE_LEGSHOT)
		ent->ClassSpeed /= 2;

	strcpy_ (s, "%i", ent->ClassSpeed);
	stuffcmd_speed (ent, s);

    if (ent->ripstate & STATE_SKIN)
	{
		stuffcmd (ent, "skin ");
		stuffcmd (ent, ent->skincheck);
		stuffcmd (ent, "\n");
        Info_SetValueForKey (ent->client->pers.userinfo, "skin", ent->skincheck);
	}
}