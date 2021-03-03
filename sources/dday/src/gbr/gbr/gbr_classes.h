/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/gbr/gbr_classes.h,v $
 *   $Revision: 1.13 $
 *   $Date: 2002/06/04 19:49:50 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

/*
typedef struct 
{
	char *name;				//mos name
	char *skinname;			//mos skin (general model defined within user dll)
	char *weapon1;			//primary weapon
	int		ammo1;			//number of mags for w1
	char *weapon2;			//secondary weapon
	int		ammo2;			//number of mags for w2
	char *grenades;			//grenade type
	int grenadenum;			//number of grenades
	char *special;			//special weapon, if any
	int	specnum;			//number of special items given
	mos_t	mos;			//MOS type that struct represents...
	int		available;		//number allowed on team
	float	normal_weight;	//normal ammount ent may carry without speed effect
	float	max_weight;		
	float	speed_mod;		//class speed
	float	marksmanship;	// %chance of greater elevation (less == better)
	char *MOS_Spaw_Point;	//name of spawn location
} SMos_t;
*/

SMos_t GBR_MOS_List[]=
{
	{NULL}, // For NO MOS selection
	{
		"Infantry",
		"class_infantry",
		"Lee Enfield",
		4,
		NULL,
		0,
		"Mills Bomb",
		STARTING_GRENADES,
		NULL,
		0,
		INFANTRY,
		MAX_INFANTRY,
		STANDARD_NORMAL,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD),
		STANDARD_MARKSMANSHIP,
		"info_infantry_start"

	},
	
	{
		"Officer",
		"class_officer",
		"Sten",
		5,
		"Webley",
		4,
		"Mills Bomb",
		STARTING_GRENADES,
		"Binoculars",
		1,
		OFFICER,
		MAX_OFFICERS,
		STANDARD_NORMAL,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD * 0.90),
		STANDARD_MARKSMANSHIP,
		"info_officer_start"
	},	

	{
		"Light Gunner",
		"class_lmg",
		"Bren",
		5,
		NULL,
		0,
		"Mills Bomb",
		STARTING_GRENADES,
		NULL,
		0,
		L_GUNNER,
		MAX_L_GUNNER,
		STANDARD_NORMAL,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD * 0.85),
		STANDARD_MARKSMANSHIP,
		"info_lgunner_start"
	},

	{
		"Heavy Gunner",
		"class_hmg",
		"Vickers",
		5,
		NULL,
		0,
		"Mills Bomb",
		STARTING_GRENADES,
		NULL,
		0,
		H_GUNNER,
		MAX_H_GUNNER,
		STANDARD_NORMAL+15,
		STANDARD_MAX+5,
		(STANDARD_SPEED_MOD * 0.80),
		0.60,
		"info_hgunner_start"
	},

	{
		"Sniper",
		"class_sniper",
		"Lee Enfield Sniper",
		5,
		"Webley",
		4,
		"Mills Bomb",
		STARTING_GRENADES,
		NULL,
		0,
		SNIPER,
		MAX_SNIPER,
		STANDARD_NORMAL-10,
		STANDARD_MAX-10,
		(STANDARD_SPEED_MOD * 1.10),
		0.1,
		"info_sniper_start",
	},

	{
		"Airborne",
		"class_special",
		"Sten",
		5,
		NULL,
		0,
		"Mills Bomb",
		STARTING_GRENADES,
		NULL,//parachute will go here
		0,
		SPECIAL,
		MAX_SPECIAL,
		STANDARD_NORMAL-5,
		STANDARD_MAX-5,
		(STANDARD_SPEED_MOD * 0.95),
		0.33,
		"info_special_start"
	},
	
	{
		"Engineer",
		"class_engineer",
		"PIAT",
		5,
		"Webley",
		4,
		"Mills Bomb",
		STARTING_GRENADES,
		"TNT",
		1,
		ENGINEER,
		MAX_ENGINEER,
		STANDARD_NORMAL,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD * 0.85),
		0.66,
		"info_engineer_start"
	},

	{
		"Medic",
		"class_medic",
		"Morphine",
		1,
		NULL,
		0,
		NULL,
		0,
		NULL,
		0,
		MEDIC,
		MAX_MEDIC,
		STANDARD_NORMAL-15,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD * 1.10),
		.75,
		"info_medic_start"
	},
	
	{
		"Flamethrower",
		"class_flamethrower",
		"Flamethrower",
		0,
		NULL,
		0,
		"Mills Bomb",
		STARTING_GRENADES,
		NULL,
		0,
		FLAMER,
		MAX_FLAMER,
		STANDARD_NORMAL+5,
		STANDARD_MAX,
		(STANDARD_SPEED_MOD * 0.80),
		.66,
		"info_flamethrower_start"
	}
};
