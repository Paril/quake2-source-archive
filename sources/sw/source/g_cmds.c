#include "g_local.h"
#include "m_player.h"

void Cmd_Set_MouseSensitivity(edict_t *ent);
void Cmd_Taunt(edict_t *ent);
void Cmd_Saber_Color(edict_t *ent);

//BOTSAFE ROUTINES

void debug_printf(char *fmt, ...)
{
/*	int     i;
	char	bigbuffer[0x10000];
	int		len;
	va_list	argptr;
	edict_t	*cl_ent;
	
	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	if (dedicated->value)
		safe_cprintf(NULL, PRINT_MEDIUM, bigbuffer);

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || cl_ent->is_bot)
			continue;

		safe_cprintf(cl_ent,  PRINT_MEDIUM, bigbuffer);
	}
*/
}

///////////////////////////////////////////////////////////////////////
// botsafe cprintf
///////////////////////////////////////////////////////////////////////

void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...)
{
	char	bigbuffer[0x10000];
	va_list		argptr;
	int len;

	if (ent && (!ent->inuse || ent->is_bot))
		return;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	gi.cprintf(ent, printlevel, bigbuffer);
}

///////////////////////////////////////////////////////////////////////
// botsafe centerprintf
///////////////////////////////////////////////////////////////////////
void safe_centerprintf (edict_t *ent, char *fmt, ...)
{
/*	char	bigbuffer[0x10000];
	va_list		argptr;
	int len;

	if (!ent->inuse || ent->is_bot)
		return;
	
	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);
	
	gi.centerprintf(ent, bigbuffer);
*/	
}

///////////////////////////////////////////////////////////////////////
// botsafe bprintf
///////////////////////////////////////////////////////////////////////
void safe_bprintf (int printlevel, char *fmt, ...)
{
	int i;
	char	bigbuffer[0x10000];
	int		len;
	va_list		argptr;
	edict_t	*cl_ent;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	if (dedicated->value)
		safe_cprintf(NULL, printlevel, bigbuffer);

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || cl_ent->is_bot)
			continue;

		safe_cprintf(cl_ent, printlevel, bigbuffer);
	}

}



//*******************************
//SWTC START
//*******************************

//
//	Includes Priv's new taunt scenes.
//


void Cmd_Saber_Color(edict_t *ent)
{
	char *color;

	if(gi.argc())
	{
		color = gi.args();
	}
	else
	{
		safe_cprintf(ent, PRINT_MEDIUM, "Colors:\n  Green\n  Red\n  Yellow\n  Blue\n");
		return;
	}

	if(Q_strcasecmp(color, "red") == 0)
	{
		ent->client->saber_c = 1;
		safe_cprintf(ent, PRINT_MEDIUM, "Using Red Light Saber\n");
	}
	else if(Q_strcasecmp(color, "green") == 0)
	{
		ent->client->saber_c = 2;
		safe_cprintf(ent, PRINT_MEDIUM, "Using Green Light Saber\n");
	}
	else if(Q_strcasecmp(color, "yellow") == 0)
	{
		ent->client->saber_c = 3;
		safe_cprintf(ent, PRINT_MEDIUM, "Using Yellow Light Saber\n");
	}
	else if(Q_strcasecmp(color, "blue") == 0)
	{
		ent->client->saber_c = 4;
		safe_cprintf(ent, PRINT_MEDIUM, "Using Blue Light Saber\n");
	}
	else
	{
		ent->client->saber_c = 4;
		safe_cprintf(ent, PRINT_MEDIUM, "No color defined, please type:\ncolor いい with いい being either red, green, yellow or blue.\n\nUsing Blue as default Saber Color.\n");
	}
	ChangeWeapon(ent);
}

void Cmd_Taunt(edict_t *ent)
{
	if(!ent->groundentity)
		return;
	if(ent->client->pers.weapon == FindItem("Lightsaber"))
	{
		ent->s.frame = FRAME_flashsbrA;
		ent->client->anim_end = FRAME_flashsbrB;
	}
	else
	{
		ent->s.frame = FRAME_flashwepA;
		ent->client->anim_end = FRAME_flashwepB;
	}
}

//
//	Allows for a variable sensitivity of the mouse
//	in Sniper Zoom. Can be turned off with the command
//	"mouse_s off" or to a basic sensitivity value with
//	"mouse_s #" where # is a number between 1 and 12.
//

void Cmd_Set_MouseSensitivity(edict_t *ent)
{
	char *sensitivity;

	if(gi.argc())
	{
		sensitivity = gi.args();
	}
	else
	{
		safe_cprintf(ent, PRINT_MEDIUM, "Acceptable commands are \"mouse_s off\" and \"mouse_s #\" with # being a number\n");
		return;
	}

	if(Q_stricmp(sensitivity, "off") == 0)
	{
		ent->client->mouse_s = 0;
		safe_cprintf(ent, PRINT_MEDIUM, "Custom mouse sensitivity is off\n");
	}
	else
	{
		ent->client->mouse_s = atoi(sensitivity);
		_stuffcmd(ent, "sensitivity \"%f\"\n", ent->client->mouse_s);
		safe_cprintf(ent, PRINT_MEDIUM, "Custom mouse sensitivity is %f\n", ent->client->mouse_s);
	}
}

void Cmd_Spawn_New_Bot ()
{
	char *name, *skin;

	name = gi.argv(1);
	skin = gi.argv(2);

	Spawn_New_Bot (name, skin);
}

/*==================
RipVTide's Awesome Computer code
START
==================*/
void Cmd_Change_Dir (edict_t *ent)
{
	char		*cds;

	if(gi.argc())
	{
		cds = gi.args();
		Computer_Change_Dir(cds, ent);
	}
}

void Cmd_Cat (edict_t *ent)
{
	char		*txt;


	if(gi.argc())
		{
		txt = gi.args();
		Computer_Cat(txt, ent);
		}
}

void Cmd_Change_Pass (edict_t *ent)
{
	char		*pass;


	if(gi.argc())
		{
		pass = gi.args();
		Computer_Pass(pass, ent);
		}
}

void Cmd_Dir (edict_t *ent)
{
	Computer_Dir(ent);
}

void Cmd_Exec (edict_t *ent)
{
	char		*file;


	if(gi.argc())
		{
		file = gi.args();
		Computer_Exec(file, ent);
		}
}
/*==================
RipVTide's Awesome Computer code
END
==================*/

void Cmd_Set_AutoReload(edict_t *ent)
{
	if(ent->client->a_reload)
	{
		safe_cprintf (ent, PRINT_MEDIUM, "Autoreload ON\n");
		ent->client->a_reload = 0;
	}
	else
	{
		safe_cprintf (ent, PRINT_MEDIUM, "Autoreload OFF\n");
		ent->client->a_reload = 1;
	}
}

void Cmd_vehicle_f (edict_t *ent)
{
	int value1, count;

	count = gi.argc() - 1;
	value1 = -1;

	if(count == 0 || count > 1)
	{
		safe_cprintf (ent, PRINT_HIGH, "Usage: vehicle num\n");
		safe_cprintf (ent, PRINT_HIGH, "Where num is the number of the vehicle\n");
		return;
	}
	if(count == 1)
	{
		value1 = atoi(gi.argv(1));
	}

	switch (value1)
	{
	case 0: //switch back to player
		ent->client->vflags = 0;
		break;
	case VEHICLE_SPEEDER:
		Cmd_sbike(ent);
		break;
	case VEHICLE_XWING:
		break;
	case 3:
		break;
	case 4:
		break;
	default:
		break;
	}
}

void Cmd_sbike (edict_t *ent)
{
	if(ent->client->vflags & IN_VEHICLE)
	{
		// turn ON client side prediction for this player
		gi.WriteByte(11);		// 11 = svc_stufftext
		gi.WriteString("cl_predict 1\n");
		gi.unicast(ent, 1);

		gi.WriteByte(11);		// 11 = svc_stufftext
		gi.WriteString("cl_footsteps 1\n");
		gi.unicast(ent, 1);

		ent->s.modelindex = 255;

		gi.linkentity (ent);

		ent->client->vflags = 0;
		ent->client->throttle = 0;
		ent->client->max_height = 0;
		ent->client->current_height = 0;
		return;
	}

	gi.sound(ent, CHAN_AUTO, gi.soundindex("vehicles/s_bike/start.wav"), 1, ATTN_NORM, 0);
//	ent->client->vehicle_sound_time = level.time + 2;

	// turn OFF client side prediction for this player
	gi.WriteByte(11);		// 11 = svc_stufftext
	gi.WriteString("cl_predict 0\n");
	gi.unicast(ent, 1);

	gi.WriteByte(11);		// 11 = svc_stufftext
	gi.WriteString("cl_footsteps 0\n");
	gi.unicast(ent, 1);

	ent->s.modelindex = gi.modelindex("models/vehicles/spdrbike/tris.md2");
//	ent->s.modelindex = gi.modelindex("players/male/tris.md2");

	gi.linkentity (ent);

	ent->client->vflags |= IN_VEHICLE;
	ent->client->vflags |= VEHICLE_SPEEDER;
	ent->client->throttle = 0;
	ent->client->max_height = 40; //around 2 metres
	ent->client->current_height = 0;
}


//
// Added Chasecam functions 7-Aug-2000
//
void cmd_holster (edict_t *ent)
{
	if(saberonly->value)
		return;
	if(ent->holstered == 0) //if not holstered(0)
	{
		ent->holstered = 2; //begin holstering(2)
		ent->holstered_weap = ent->client->pers.weapon;
		ent->client->newweapon = FindItem("Hands");
		ChasecamStart (ent);
	}
	else if (ent->holstered == 1) //if holstered(1)
	{
		ent->client->newweapon = ent->holstered_weap;
		ent->holstered = 3; //begin unholstering(3)
		ChasecamRemove (ent, "off");
	}
	else
		return;
}

void Cmd_OpenDoor(edict_t *ent)
{
	vec3_t start, end, forward;
	trace_t tr;

	VectorCopy(ent->s.origin, start);
	start[2] += 24;
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);
	VectorMA (start, 64, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SOLID);

	if(strcmp(tr.ent->classname, "worldspawn") == 0)
	{
		return;
	}

	if(strcmp(tr.ent->classname, "func_door") == 0)
	{
		if(!tr.ent->targetname)
		{
			door_use (tr.ent, ent, ent);
		}
	}
	else if(strcmp(tr.ent->classname, "func_button") == 0)
	{
		button_use (tr.ent, ent, ent);
	}
	else if(strcmp(tr.ent->classname, "func_door_rotating") == 0)
	{
		if(!tr.ent->targetname)
		{
			door_use (tr.ent, ent, ent);
		}
	}
}

void Cmd_Secondary_Fire (edict_t *ent)
{
	ent->client->buttons |= BUTTON_ATTACK;
	Think_Weapon (ent, 1);
}

void Cmd_Reload (edict_t *ent)
{
	if(saberonly->value)
		return;
	ent->client->reload = 12;
	Think_Weapon (ent, 0);
}

/*********************************
Force Power Stuff and Weapon stuff
*********************************/

void Cmd_Set_Force_Useable(edict_t *ent)
{
	ent->client->force_user = 1;

	ent->client->pers.force.powers[NFORCE_PUSH] = 1;
	ent->client->pers.force.power_values[NFORCE_PUSH] = 200;

	ent->client->pers.force.powers[NFORCE_PULL] = 1;
	ent->client->pers.force.power_values[NFORCE_PULL] = 200;

	ent->client->pers.force.powers[LFORCE_HEAL] = 1;
	ent->client->pers.force.power_values[LFORCE_HEAL] = 200;

	ent->client->pers.force.powers[DFORCE_CHOKE] = 1;
	ent->client->pers.force.power_values[DFORCE_CHOKE] = 200;

	calc_subgroup_values(ent);
	calc_darklight_value(ent);
	calc_top_level_value(ent);
	ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
//	ent->client->pers.force.pool = ent->client->pers.force.pool_max;
	sort_useable_powers(ent);
}

void Cmd_Set_Force_Level(edict_t *ent)
{
	int		level, i, skill, max, min;
	
	if (!deathmatch->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "This command only works in deathmatch\n");
		return;
	}

	if (!sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	level = atoi(gi.argv(1));

	if(level > 50)
	{
		level = 50;
	}
	else if (level < -50)
	{
		level = -50;
	}
	gi.dprintf("setting force level to: %i\n", level);
	max = level + 50;
	min = level - 50;

	ent->client->pers.force.affiliation = level;
	ent->client->force_user = 1;

	for(i=1; i<NUM_POWERS+1; i++)
	{
		if(powerlist[i].lightdark == 0)
		{
			ent->client->pers.force.powers[i] = 1;
			ent->client->pers.force.power_values[i] = 1000;
		}
		else
		{
			skill = powerlist[i].skillreq;
			if(skill >= min && skill <= max)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 1000;
			}
			else
			{
				ent->client->pers.force.powers[i] = 0;
				ent->client->pers.force.power_values[i] = 0;
			}
		}
	}
	calc_subgroup_values(ent);
	calc_darklight_value(ent);
	calc_top_level_value(ent);
	ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
	ent->client->pers.force.pool = ent->client->pers.force.pool_max;
}

/*================================
void cmd_force_info (edict_t *ent);
Various Debugging info on all of ent's force powers.
================================*/
void cmd_force_info (edict_t *ent)
{
	int i;
	gforce_t *power;

	if(!ent->client->force_user)
	{
		if(!ent->is_bot)
			gi.dprintf("YOU CANNOT USE THE FORCE\n");
		return;
	}

	for(i=1; i<NUM_POWERS+1; i++)
	{
		if(ent->client->pers.force.powers[i] == 0)
			continue;
		power = &powerlist[i];
		safe_cprintf (ent, PRINT_HIGH, "%i", power->skillreq);
		safe_cprintf (ent, PRINT_HIGH, " %s:", power->name);
		if(power->lightdark == -1)
			safe_cprintf (ent, PRINT_HIGH, " DARK");
		else if (power->lightdark == 1)
			safe_cprintf (ent, PRINT_HIGH, " LIGHT");
		else
			safe_cprintf (ent, PRINT_HIGH, " NEUTRAL");
		safe_cprintf (ent, PRINT_HIGH, " %f", ent->client->pers.force.power_values[i]);
		safe_cprintf (ent, PRINT_HIGH, "\n");
	}

	if(ent->is_bot)
	{
	gi.dprintf("POOL = %f\n", ent->client->pers.force.pool);
	gi.dprintf("POOL_MAX = %i\n", ent->client->pers.force.pool_max);
	gi.dprintf("TOP LEVEL = %f\n", ent->client->pers.force.top_level);
	gi.dprintf("AFFILIATION = %f\n", ent->client->pers.force.affiliation);
	gi.dprintf("SUBGROUP1 = %f\n", ent->client->pers.force.subgroup_value[1]);
	gi.dprintf("SUBGROUP2 = %f\n", ent->client->pers.force.subgroup_value[2]);
	gi.dprintf("SUBGROUP3 = %f\n", ent->client->pers.force.subgroup_value[3]);
	gi.dprintf("SUBGROUP4 = %f\n", ent->client->pers.force.subgroup_value[4]);
	gi.dprintf("SUBGROUP5 = %f\n", ent->client->pers.force.subgroup_value[5]);
	gi.dprintf("SUBGROUP6 = %f\n", ent->client->pers.force.subgroup_value[6]);
	}
}


/*================================
void sniper_zoom (edict_t *ent);
Sniper zooming
================================*/
void sniper_zoom (edict_t *ent)
{
	gitem_t		*weapon;

	//check if weapon is a zoomable one.

	weapon = FindItem("Night_Stinger");
	if(ent->client->pers.weapon == weapon || zoomall->value)
	{
		if(!ent->groundentity && (ent->client->ps.pmove.pm_type == PM_NORMAL))
		{
			gi.dprintf("You can't use the Night Stinger zoom while moving\n");
			return;
		}
		if(ent->client->in_snipe)
		{
			ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model); 
			ent->client->in_snipe = 0;
//			ent->client->zoom_factor = 90;
			ent->client->ps.fov = 90;
//			set_fov (ent);
			if(ent->client->mouse_s)
			{
				_stuffcmd(ent, "sensitivity \"%f\"\n", ent->client->mouse_s);
			}
			ent->movetype = MOVETYPE_WALK;
			ent->client->ps.pmove.pm_type = PM_NORMAL;
		}
		else
		{
//			if(!ent->client->zoom_factor || ent->client->zoom_factor == 90)
//				ent->client->zoom_factor = 45;
			set_fov(ent);
			ent->client->in_snipe = 1;
			ent->movetype = MOVETYPE_NONE;
			ent->client->ps.gunindex = 0;
			if (ent->client->chasetoggle > 0)
				ChasecamRemove (ent, "off");
			ent->client->ps.pmove.pm_type = PM_FREEZE;
		}
	}
	else
	{
		ent->client->in_snipe = 0;
//		ent->client->zoom_factor = 90;
		ent->client->ps.fov = 90;
//		set_fov (ent);
	}
}	

/*================================
void cmd_Weapon_Menu (edict_t *ent);
Brings up, or manipulates the weapon selection menu.
================================*/
void cmd_Weapon_Menu (edict_t *ent)
{
	int value1;

	value1 = atoi(gi.argv(1));


	if(saberonly->value)
		return;

	if(ent->client->in_snipe)
		return;

	if(value1 > 4)
		value1 = ent->client->weapon_menu_sub_sel;

	setup_weap_menu (ent);

	if(ent->client->menu_time > level.time)
	{
		if(ent->client->weapon_menu_sub_sel == value1)
		{
			ent->client->weapon_menu_weap_sel += 1;
			if(ent->client->weapon_menu_weap_sel > ent->client->weapon_sub_num[ent->client->weapon_menu_sub_sel-1])
				ent->client->weapon_menu_weap_sel = 1;
		}
		else
		{
			ent->client->weapon_menu_sub_sel = value1;
			ent->client->weapon_menu_weap_sel = 1;
		}
	}
	else
	{
		ent->client->weapon_menu_sub_sel = value1;
		ent->client->weapon_menu_weap_sel = 1;
	}

	ent->client->icon_menu_active = 2;
	ent->client->menu_time = level.time + 3;
}

/*void cmd_Weapon_Menu_Next (edict_t *ent)
{
	setup_weap_menu (ent);

	if(ent->client->menu_time > level.time)
	{
		if(ent->client->weapon_menu_sub_sel == value1)
		{
			ent->client->weapon_menu_weap_sel += 1;
			if(ent->client->weapon_menu_weap_sel > ent->client->weapon_sub_num[ent->client->weapon_menu_sub_sel-1])
				ent->client->weapon_menu_weap_sel = 1;
		}
		else
		{
			ent->client->weapon_menu_weap_sel = 1;
		}
	}
	else
	{
		ent->client->weapon_menu_weap_sel = 1;
	}

	ent->client->icon_menu_active = 2;
	ent->client->menu_time = level.time + 3;
}

void cmd_Weapon_Menu_Prev (edict_t *ent)
{
	int	has_weaps[4];

	setup_weap_menu (ent);
	has_weaps[0] = ent->client->weapon_sub_num[0];
	has_weaps[1] = ent->client->weapon_sub_num[1];
	has_weaps[2] = ent->client->weapon_sub_num[2];
	has_weaps[3] = ent->client->weapon_sub_num[3];

	if(ent->client->menu_time > level.time)
	{
		ent->client->weapon_menu_weap_sel--;
		if(ent->client->weapon_menu_weap_sel == 0)
		{
			ent->client->weapon_menu_weap_sel = 1;
			ent->client->weapon_menu_sub_sel--;
			if(has_weaps[ent->client->weapon_menu_sub_sel] == 0)
			{
			}
		}
	}
	else
	{
		ent->client->weapon_menu_weap_sel = 1;
	}

	ent->client->icon_menu_active = 2;
	ent->client->menu_time = level.time + 3;
}

*/
void cmd_Force_Menu (edict_t *ent)
{
	int value1;
	int power, temp;

	if(!ent->client->force_user)
		return;


	power = FindPowerByName(gi.argv(1));
	if(power)
	{
		if(ent->client->pers.force.powers[power] == 1)
		{
			if(powerlist[power].constant == 1)
			{
				temp = ent->client->pers.current_power;
				ent->client->pers.force_power = &powerlist[power];
				ent->client->pers.current_power = power;
//				gi.dprintf("activating force %s\n", ent->client->pers.force_power->name);
				ent->client->pers.force_power->think(ent, 1);

				//switch back to previous power
				ent->client->pers.force_power = &powerlist[temp];
				ent->client->pers.current_power = temp;
			}
			else
			{
				ent->client->pers.force_power = &powerlist[power];
				ent->client->pers.current_power = power;
//				gi.dprintf("using force %s\n", ent->client->pers.force_power->name);
			}
			return;
		}
	}

	value1 = atoi(gi.argv(1));

	if(value1 > NUM_SUBGROUPS || value1 < 1)
		return;

	setup_force_menu (ent);

	if(ent->client->menu_time > level.time)
	{
		if(ent->client->force_menu_sub_sel == value1)
		{
			ent->client->force_menu_pow_sel += 1;
			if(ent->client->force_menu_pow_sel == ent->client->force_sub_num[value1])
				ent->client->force_menu_pow_sel = 0;
		}
		else
		{
			ent->client->force_menu_sub_sel = value1;
			ent->client->force_menu_pow_sel = 0;
		}
	}
	else
	{
		ent->client->force_menu_sub_sel = value1;
		ent->client->force_menu_pow_sel = 0;
	}

	ent->client->icon_menu_active = 1;
	ent->client->menu_time = level.time + 3;
}

/*void cmd_Force_Menu_Next (edict_t *ent)
{
	int value1;

	if(!ent->client->force_user)
		return;

	if(ent->client->menu_time > level.time)
	{
		if(ent->client->force_menu_sub_sel == value1)
		{
			ent->client->force_menu_pow_sel += 1;
			if(ent->client->force_menu_pow_sel == 6)
			ent->client->force_menu_pow_sel = 1;
		}
		else
		{
			ent->client->force_menu_sub_sel = value1;
			ent->client->force_menu_pow_sel = 1;
		}
	}
	else
	{
		ent->client->force_menu_sub_sel = value1;
		ent->client->force_menu_pow_sel = 1;
	}

	ent->client->icon_menu_active = 1;
	ent->client->menu_time = level.time + 3;
}

void cmd_Force_Menu_Prev (edict_t *ent)
{
	int value1;

	if(!ent->client->force_user)
		return;

	value1 = atoi(gi.argv(1));

	if(value1 > 6)
		value1 = ent->client->force_menu_sub_sel;

	if(ent->client->menu_time > level.time)
	{
		if(ent->client->force_menu_sub_sel == value1)
		{
			ent->client->force_menu_pow_sel += 1;
			if(ent->client->force_menu_pow_sel == 6)
			ent->client->force_menu_pow_sel = 1;
		}
		else
		{
			ent->client->force_menu_sub_sel = value1;
			ent->client->force_menu_pow_sel = 1;
		}
	}
	else
	{
		ent->client->force_menu_sub_sel = value1;
		ent->client->force_menu_pow_sel = 1;
	}

	ent->client->icon_menu_active = 1;
	ent->client->menu_time = level.time + 3;
}
*/

//*******************************
//SWTC END
//*******************************

char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//ZOID
	if (cl->menu)
	{
		PMenu_Next(ent);
		return;
	}
	else if (cl->chase_target)
	{
		ChaseNext(ent);
		return;
	}
//ZOID

//RipVTide

	if (holdthephone == 1);
	{
		if (NoTouch == 0)
		{
			if (highlighted == 9)
			{
				highlighted = 2;
				conversation_content[9].flags[3] = 0;
				conversation_content[2].flags[3] = 1;
				return;
			}
			
			if (conversation_content[highlighted + 1].flags[2] == 0)
			{
				conversation_content[highlighted].flags[3] = 0;
				highlighted = 2;
				conversation_content[2].flags[3] = 1;
				return;
			}
			else
			{
				highlighted = highlighted + 1;
				conversation_content[highlighted].flags[3] = 1;
				conversation_content[highlighted - 1].flags[3] = 0;
				return;
			}
		}
		return;
	}

//RipVTide
	
	
	
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//ZOID
	if (cl->menu)
	{
		PMenu_Prev(ent);
		return;
	}
	else if (cl->chase_target)
	{
		ChasePrev(ent);
		return;
	}
//ZOID

//RipVTide

	if (holdthephone == 1);
	{
		if (NoTouch == 0)
		{
			if (highlighted == 2)
			{
				highlighted = 9;
				while(conversation_content[highlighted].flags[2] == 0)
				{
					highlighted--;
				}
				conversation_content[2].flags[3] = 0;
				conversation_content[highlighted].flags[3] = 1;
				return;
			}
			
			highlighted = highlighted - 1;
			conversation_content[highlighted].flags[3] = 1;
			conversation_content[highlighted + 1].flags[3] = 0;
			return;
		}
		return;
	}

//RipVTide

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}



//RipVTide Choose force affiliation

void Cmd_Set_Affiliation(edict_t *ent)
{
	char		*side;

	if(theforce->value)
	{
		if(gi.argc())
		{
			side = gi.args();
			if(!Q_strncasecmp(side, "neut", 4))
			{
				watchmesing(ent);
				ent->client->resp.side = 11;
			}
			else if(!Q_strncasecmp(side, "light", 5))
			{
				imabigchicken(ent);
				ent->client->resp.side = 12;
			}
			else if(!Q_strncasecmp(side, "dark", 4))
			{
				googl3(ent);
				ent->client->resp.side = 13;
			}
			else
				safe_cprintf(ent, PRINT_HIGH, "Try typing a bit clearer\n");
		}
	}
}
void watchmesing(edict_t *ent)
{
	int			i;
	gforce_t	*power;


	ent->client->force_user = 1;
		for(i=1; i<NUM_POWERS+1; i++)
		{
			power = &powerlist[i];
			if(power->lightdark == -1)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 0;
			}
			else if (power->lightdark == 0)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 1000;
			}
			else
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 1000;
			}
		}

		calc_subgroup_values(ent);
		calc_darklight_value(ent);
		calc_top_level_value(ent);
		ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
		sort_useable_powers(ent);

		return;
}

void imabigchicken(edict_t *ent)
{
	int			i;
	gforce_t	*power;

	ent->client->force_user = 1;
		for(i=1; i<NUM_POWERS+1; i++)
		{
			power = &powerlist[i];
			if(power->lightdark == -1)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 0;
			}
			else if (power->lightdark == 0)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 1000;
			}
			else
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 0;
			}
		}

		calc_subgroup_values(ent);
		calc_darklight_value(ent);
		calc_top_level_value(ent);
		ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
		sort_useable_powers(ent);

		return;
}

void googl3(edict_t *ent)
{
	int			i;
	gforce_t	*power;

	ent->client->force_user = 1;
		for(i=1; i<NUM_POWERS+1; i++)
		{
			power = &powerlist[i];
			if(power->lightdark == -1)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 1000;
			}
			else if (power->lightdark == 0)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 1000;
			}
			else
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 0;
			}
		}

		calc_subgroup_values(ent);
		calc_darklight_value(ent);
		calc_top_level_value(ent);
		ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
		sort_useable_powers(ent);

		return;
}

void Cmd_RK(edict_t *ent)
{
	int			x;
	gforce_t	*power;

				ent->client->force_user = 1;
				for(x=1; x<NUM_POWERS+1; x++)
				{
					power = &powerlist[x];
					ent->client->pers.force.powers[x] = 1;
					ent->client->pers.force.power_values[x] = 1000;
				}

				calc_subgroup_values(ent);
				calc_darklight_value(ent);
				calc_top_level_value(ent);
				ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
				sort_useable_powers(ent);
				ent->client->force_time = 10;
				ent->client->pers.force.pool_max = 1000;
}
//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	gforce_t	*power;

	if (deathmatch->value && !sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "force") == 0)
	{
		ent->client->force_user = 1;
		for(i=1; i<NUM_POWERS+1; i++)
		{
			power = &powerlist[i];
			ent->client->pers.force.powers[i] = 1;
			ent->client->pers.force.power_values[i] = 1000;
		}

		calc_subgroup_values(ent);
		calc_darklight_value(ent);
		calc_top_level_value(ent);
		ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
		sort_useable_powers(ent);
		return;
	}

	if (Q_stricmp(name, "light") == 0)
	{
		ent->client->force_user = 1;
		for(i=1; i<NUM_POWERS+1; i++)
		{
			power = &powerlist[i];
			if(power->lightdark == -1)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 0;
			}
			else if (power->lightdark == 0)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 1000;
			}
			else
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 1000;
			}
		}

		calc_subgroup_values(ent);
		calc_darklight_value(ent);
		calc_top_level_value(ent);
		ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
		sort_useable_powers(ent);

		return;
	}

	if (Q_stricmp(name, "neutral") == 0)
	{
		ent->client->force_user = 1;
		for(i=1; i<NUM_POWERS+1; i++)
		{
			power = &powerlist[i];
			if(power->lightdark == -1)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 0;
			}
			else if (power->lightdark == 0)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 1000;
			}
			else
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 0;
			}
		}

		calc_subgroup_values(ent);
		calc_darklight_value(ent);
		calc_top_level_value(ent);
		ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
		sort_useable_powers(ent);

		return;
	}

	if (Q_stricmp(name, "dark") == 0)
	{
		ent->client->force_user = 1;
		for(i=1; i<NUM_POWERS+1; i++)
		{
			power = &powerlist[i];
			if(power->lightdark == -1)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 1000;
			}
			else if (power->lightdark == 0)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 1000;
			}
			else
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = 0;
			}
		}

		calc_subgroup_values(ent);
		calc_darklight_value(ent);
		calc_top_level_value(ent);
		ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
		sort_useable_powers(ent);

		return;
	}

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{

		if(saberonly->value)
			return;

		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

/*	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}*/

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			safe_cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		safe_cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	safe_cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	safe_cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	safe_cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		safe_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		safe_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

//ZOID--special case for tech powerups
	if (Q_stricmp(gi.args(), "tech") == 0 && (it = CTFWhat_Tech(ent)) != NULL)
	{
		it->drop (ent, it);
		return;
	}
//ZOID

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		safe_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		safe_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

//ZOID
	if (ent->client->menu)
	{
		PMenu_Close(ent);
		ent->client->update_chase = true;
		return;
	}
//ZOID

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

//ZOID
	if (ctf->value && cl->resp.ctf_team == CTF_NOTEAM)
	{
		CTFOpenJoinMenu(ent);
		return;
	}
//ZOID

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

//ZOID
	if (ent->client->menu)
	{
		PMenu_Select(ent);
		return;
	}
//ZOID

//RipVTide

	if (holdthephone == 1)
	{
		if (NoTouch == 0)
		{
			IChooseYou();
			return;
		}
	}

//RipVTide

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		safe_cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

//ZOID
/*
=================
Cmd_LastWeap_f
=================
*/
void Cmd_LastWeap_f (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if(saberonly->value)
		return;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	cl->pers.lastweapon->use (ent, cl->pers.lastweapon);
}
//ZOID

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if(saberonly->value)
		return;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if(saberonly->value)
		return;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if(saberonly->value)
		return;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		safe_cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
//ZOID
	if (ent->solid == SOLID_NOT)
		return;
//ZOID

	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;

//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
	ent->client->update_chase = true;
//ZOID
}

int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	safe_cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			safe_cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			safe_cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		safe_cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		safe_cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			safe_cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	safe_cprintf(ent, PRINT_HIGH, "%s", text);
}


/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_stricmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_stricmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_stricmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);

	//CaRRaC - RipVTide

	else if (!Q_stricmp (cmd, "opendoor"))
		Cmd_OpenDoor(ent);
	else if (!Q_stricmp (cmd, "secondary"))
		Cmd_Secondary_Fire(ent);
	else if (!Q_stricmp (cmd, "reload"))
		Cmd_Reload (ent);
	else if (!Q_stricmp (cmd, "zoom"))
		sniper_zoom (ent);
	else if (!Q_stricmp (cmd, "forceinfo"))
		cmd_force_info (ent);
	else if (!Q_stricmp (cmd, "force"))
		cmd_Force_Menu (ent);
//	else if (!Q_stricmp (cmd, "force_next"))
//		cmd_Force_Menu_Next (ent);
//	else if (!Q_stricmp (cmd, "force_prev"))
//		cmd_Force_Menu_Prev (ent);
	else if (!Q_stricmp (cmd, "weapon"))
		cmd_Weapon_Menu (ent);
	else if (!Q_stricmp (cmd, "holster"))
		cmd_holster (ent);
	else if (Q_stricmp (cmd, "chasecam") == 0)
		Cmd_Chasecam_Toggle (ent);
	else if (Q_stricmp (cmd, "chaselock") == 0)
		Cmd_Chasecam_Viewlock(ent, "");
	else if (Q_stricmp (cmd, "camzoomout") == 0)
		Cmd_Chasecam_Zoom(ent, "out");
	else if (Q_stricmp (cmd, "camzoomin") == 0)
		Cmd_Chasecam_Zoom(ent, "in");
	else if (!Q_stricmp (cmd, "speak"))
		JustTalk (ent, ent);
/*==================
RipVTide's Awesome Computer code
START
==================*/
	else if (!Q_stricmp (cmd, "cd"))
		Cmd_Change_Dir (ent);
	else if (!Q_stricmp (cmd, "cd.."))
		Computer_Change_Dir("..", ent);
	else if (!Q_stricmp (cmd, "cat"))
		Cmd_Cat (ent);
	else if (!Q_stricmp (cmd, "pass"))
		Cmd_Change_Pass (ent);
	else if (!Q_stricmp (cmd, "update") || !Q_stricmp (cmd, "rel"))
		Cmd_Dir (ent);
	else if (!Q_stricmp (cmd, "run"))
		Cmd_Exec (ent);
	else if (!Q_stricmp (cmd, "add"))
	{
		ent->frame_end++;
//		gi.dprintf("%i\n", ent->frame_end);
	}
/*==================
RipVTide's Awesome Computer code
END
==================*/
//ZOID
	else if (Q_stricmp (cmd, "team") == 0)
	{
		CTFTeam_f (ent);
	}
	else if (Q_stricmp(cmd, "id") == 0)
	{
		CTFID_f (ent);
	}
//ZOID
	else if (!Q_stricmp (cmd, "save_nodes"))
		cmd_Save_f ();
	else if (!Q_stricmp (cmd, "load_nodes"))
		cmd_Load_f ();
	else if (!Q_stricmp (cmd, "show_nodes"))
		show_visible_nodes (ent);
	else if (!Q_stricmp (cmd, "show_all_nodes"))
		show_nodes ();
	else if (!Q_stricmp (cmd, "mapping"))
		mapping_toggle ();
	else if (!Q_stricmp (cmd, "nodetest"))
		create_nodetest(ent);
	else if (!Q_stricmp (cmd, "vehicle"))
		Cmd_vehicle_f (ent);
	else if (!Q_stricmp (cmd, "bot"))
		Cmd_Spawn_New_Bot();
	else if (!Q_stricmp (cmd, "forcelevel"))
		Cmd_Set_Force_Level(ent);
	else if (!Q_stricmp (cmd, "giveforce"))
		Cmd_Set_Force_Useable(ent);
	else if (!Q_stricmp (cmd, "duel"))
	{
		if(!advanced->value)
			return;
		if(ent->client->duel == 1)
		{
			ent->client->duel = 0;
			safe_bprintf(PRINT_HIGH, "Duel Mode Off\n");
		}
		else
		{
			ent->client->duel = 1;
			safe_bprintf(PRINT_HIGH, "Duel Mode On\n");
		}
		gi.dprintf("Duel Function removed on Red Knight's Request\n");
	}
	else if (!Q_stricmp (cmd, "noreload"))
	{
		Cmd_Set_AutoReload(ent);
	}
	else if (!Q_stricmp (cmd, "credits"))
	{
		if (showingit == 1)
			showingit = 0;
		else
			showingit = 1;
	}
	else if (!Q_stricmp (cmd, "choose"))
		Cmd_Set_Affiliation(ent);
	else if (!Q_stricmp (cmd, "mouse"))
		Cmd_Set_MouseSensitivity(ent);
	else if (!Q_stricmp (cmd, "rk"))
		Cmd_RK(ent);
	else if (!Q_stricmp (cmd, "taunt"))
		Cmd_Taunt(ent);
	else if (!Q_stricmp (cmd, "color"))
		Cmd_Saber_Color(ent);
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
