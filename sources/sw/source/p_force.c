#include "g_local.h"
#include "m_player.h"

#define FORCE_GAIN		1000

//neutral
void force_push (edict_t *ent, int last);			//DONE!
void force_pull (edict_t *ent, int last);			//DONE!
void force_levitate (edict_t *ent, int last);		//DONE!
void force_negate (edict_t *ent, int last);			//DONE!
void force_jump (edict_t *ent, int last);			//DONE!
void force_speed (edict_t *ent, int last);			//DONE!

//light
void force_lightheal (edict_t *ent, int last);		//DONE!
void force_wall_of_light (edict_t *ent, int last);	//DONE!
void force_shield (edict_t *ent, int last);			//DONE!
void force_invisibility (edict_t *ent, int last);	//DONE!
void force_wind (edict_t *ent, int last);			//DONE!
void force_reflect (edict_t *ent, int last);		//DONE!
void force_scout (edict_t *ent, int last);			//REMOVED!
void force_bind (edict_t *ent, int last);			//DONE!%

//dark
void force_darkheal (edict_t *ent, int last);		//DONE!
void force_lightning (edict_t *ent, int last);		//DONE!
void force_choke (edict_t *ent, int last);			//80%
void force_absorb (edict_t *ent, int last);			//REMOVED!
void force_wall_of_darkness (edict_t *ent, int last);	//DONE!
void force_taint (edict_t *ent, int last);			//30%
void force_inferno (edict_t *ent, int last);		//DONE!
void force_rage (edict_t *ent, int last);			//DONE!


//RipVTide
void ForceAnimCD (edict_t *ent);
void ForceAnim(edict_t *ent);
void makelightning(edict_t *ent);
void l_count(edict_t *ent);

/*==============================
int FindPowerByName (char *name)
	returns the index number of a force power
	use it like:

		int index
		gforce_t *power;
		index = FindPowerByName (char *name);
		power = &powerlist[index];
==============================*/

int FindPowerByName (char *name)
{
	int		i;
	gforce_t	*it;

	it = powerlist;
	for (i=0 ; i<(NUM_POWERS+1) ; i++, it++)
	{
		if (!it->name)
			continue;
		if (!Q_stricmp(it->name, name))
			return i;
	}

	return 0;
}



/*==============================
gforce_t *GetPowerByIndex (int index)
returns the poiner to the appropriate force power
it's the equilivelant of typing &powerlist[index];
use it like:

gforce_t *power;
power = GetPowerByIndex(int index);
==============================*/

gforce_t *GetPowerByIndex (int index)
{
	if (index == 0)
		return NULL;

	return &powerlist[index];
}



/*==============================
This is the array of force powers.
I decided to use a system similar to id's original item system. Because it fits better with this than it does
with items =)
FUNCTION	think()  The function which gets called for the power effect.
CHAR		force_sound  Sound created when cast (not implemented yet)
CHAR		icon  Icon for normal use
CHAR		menu icon  Icon in force menu
CHAR		menu icon selected  Icon in force menu when selected.
CHAR		name  Name of the power
FLOAT		start_level  The level the player starts at when the power gets learned.
FLOAT		cost  The number of pool units lost when used. This depends on the style of casting
SHORT		lightdark  The powers light/dark value
UNS SHORT	constant  1 = constant power; 0 = press once; 2 = hold
UNS SHORT	subgroup  The subgroup number.
INT		skillreq  The skill required before the power can be used
==============================*/

gforce_t	powerlist[] = 
{
	{
		NULL
	},	// leave index 0 alone


//NEUTRAL POWERS
//NFORCE_PUSH		1
	{
		force_push,			//FUNCTION	think()
		NULL,				//CHAR		force_sound
		"force/neutral/push1",	//CHAR		icon
		"force/neutral/push2",	//CHAR		menu icon
		"force/neutral/push3",	//CHAR		menu icon selected
		"push",			//CHAR		name
		1,				//FLOAT	start_level
		5,				//FLOAT	cost
		0,				//INT		lightdark
		2,				//UNS SHORT	constant
		SUBGROUP_NEUTRAL,		//UNS SHORT	subgroup
		0,				//INT	skillreq
	},

//NFORCE_PULL		2
	{
		force_pull,
		NULL,
		"force/neutral/pull1",
		"force/neutral/pull2",
		"force/neutral/pull3",
		"pull",
		1,
		5,
		0,
		2,
		SUBGROUP_NEUTRAL,
		1000,
	},

//NFORCE_LEVITATE	3
	{
		force_levitate,
		NULL,
		"force/neutral/levitate1",
		"force/neutral/levitate2",
		"force/neutral/levitate3",
		"levitate",
		1,
		10,
		0,
		1,
		SUBGROUP_NEUTRAL2,
		60,
	},

//NFORCE_NEGATE	4
	{
		force_negate,
		NULL,
		"force/neutral/negate1",
		"force/neutral/negate2",
		"force/neutral/negate3",
		"negate",
		1,
		20,
		0,
		0,
		SUBGROUP_NEUTRAL,
		50,
	},

//NFORCE_JUMP	5
	{
		force_jump,
		NULL,
		"force/neutral/jump1",
		"force/neutral/jump2",
		"force/neutral/jump3",
		"jump",
		1,
		10,
		0,
		1,
		SUBGROUP_NEUTRAL2,
		20,
	},

//NFORCE_SPEED	6
	{
		force_speed,
		NULL,
		"force/neutral/speed1",
		"force/neutral/speed2",
		"force/neutral/speed3",
		"speed",
		1,
		30,
		0,
		1,
		SUBGROUP_NEUTRAL2,
		20,
	},

//LIGHT SIDE POWERS
//LFORCE_HEAL	7
	{
		force_lightheal,
		NULL,
		"force/light/heal1",
		"force/light/heal2",
		"force/light/heal3",
		"light_heal",
		1,
		10,
		1,
		2,
		SUBGROUP_SPIRIT,
		10,
	},

//LFORCE_WALL	8
	{
		force_wall_of_light,
		NULL,
		"force/light/wall1",
		"force/light/wall2",
		"force/light/wall3",
		"wall_of_light",
		1,
		40,
		1,
		0,
		SUBGROUP_SPIRIT,
		90,
	},

//LFORCE_SHIELD	9
	{
		force_shield,
		NULL,
		"force/light/shield1",
		"force/light/shield2",
		"force/light/shield3",
		"shield",
		1,
		10,
		1,
		1,
		SUBGROUP_ENERGY,
		30,
	},

//LFORCE_INVISIBILITY	10
	{
		force_invisibility,
		NULL,
		"force/light/invisi1",
		"force/light/invisi2",
		"force/light/invisi3",
		"invisibility",
		1,
		55,
		1,
		1,
		SUBGROUP_ENERGY,
		70,
	},

//LFORCE_WIND	11
	{
		force_wind,
		NULL,
		"force/light/wind1",
		"force/light/wind2",
		"force/light/wind3",
		"wind",
		1,
		25,
		1,
		2,
		SUBGROUP_TELEKINESIS,
		50,
	},

//LFORCE_REFLECT	12
	{
		force_reflect,
		NULL,
		"force/light/reflect1",
		"force/light/reflect2",
		"force/light/reflect3",
		"reflect",
		1,
		70,
		1,
		1,
		SUBGROUP_ENERGY,
		80,
	},

//LFORCE_SCOUT	13
	{
		force_scout,
		NULL,
		"force/light/scout1",
		"force/light/scout2",
		"force/light/scout3",
		"scout",
		1,
		10,
		1,
		1,
		SUBGROUP_SPIRIT,
		60,
	},

//LFORCE_BIND	14
/*	{
		force_bind,
		NULL,
		"force/light/bind1",
		"force/light/bind2",
		"force/light/bind3",
		"bind",
		1,
		20,
		1,
		2,
		SUBGROUP_TELEKINESIS,
		40,
	},
*/
//DARK SIDE POWERS
//DFORCE_HEAL	15
	{
		force_darkheal,
		NULL,
		"force/dark/heal1",
		"force/dark/heal2",
		"force/dark/heal3",
		"dark_heal",
		1,
		4,
		-1,
		2,
		SUBGROUP_SPIRIT,
		-30,
	},

//DFORCE_LIGHTNING	16
	{
		force_lightning,
		NULL,
		"force/dark/lightning1",
		"force/dark/lightning2",
		"force/dark/lightning3",
		"lightning",
		1,
		4,
		-1,
		2,
		SUBGROUP_ENERGY,
		-50,
	},

//DFORCE_CHOKE	17
	{
		force_choke,
		NULL,
		"force/dark/choke1",
		"force/dark/choke2",
		"force/dark/choke3",
		"choke",
		1,
		20,
		-1,
		2,
		SUBGROUP_TELEKINESIS,
		-10,
	},

//DFORCE_ABSORB	18
	{
		force_absorb,
		NULL,
		"force/dark/absorb1",
		"force/dark/absorb2",
		"force/dark/absorb3",
		"absorb",
		1,
		10,
		-1,
		1,
		255,
		-80,
	},

//DFORCE_DARKNESS	19
	{
		force_wall_of_darkness,
		NULL,
		"force/dark/darkness1",
		"force/dark/darkness2",
		"force/dark/darkness3",
		"darkness",
		1,
		50,
		-1,
		0,
		SUBGROUP_SPIRIT,
		-90,
	},

//DFORCE_TAINT	20
	{
		force_taint,
		NULL,
		"force/dark/taint1",
		"force/dark/taint2",
		"force/dark/taint3",
		"taint",
		1,
		40,
		-1,
		1,
		SUBGROUP_ENERGY,
		-60,
	},

//DFORCE_INFERNO	21
	{
		force_inferno,
		NULL,
		"force/dark/inferno1",
		"force/dark/inferno2",
		"force/dark/inferno3",
		"inferno",
		1,
		75,
		-1,
		0,
		SUBGROUP_TELEKINESIS,
		-70,
	},

//DFORCE_RAGE	22
	{
		force_rage,
		NULL,
		"force/dark/rage1",
		"force/dark/rage2",
		"force/dark/rage3",
		"rage",
		1,
		35,
		-1,
		0,
		SUBGROUP_SPIRIT,
		-40,
	},

	{NULL}
};

/*==============================
==============================*/
void force_frame (edict_t *ent)
{
	if(deathmatch->value || ctf->value)
	{
		Check_Active_Powers (ent);
		return;
	}
	else
	{
		calc_subgroup_values(ent);
		calc_darklight_value(ent);
		calc_top_level_value(ent);
		Check_Active_Powers(ent);
		sort_useable_powers(ent);
	}
}

/*==============================
==============================*/

void sort_useable_powers(edict_t *ent)
{
	int i;

	for(i=1; i<NUM_POWERS+1; i++)
	{
		if(Force_Power_Available (ent, i) == 1)
		{
			ent->client->pers.force.powers[i] = 1;
			if(ent->client->pers.force.power_values[i] == 0)
			{
				ent->client->pers.force.power_values[i] = 10;
			}
		}
		else
		{
			ent->client->pers.force.powers[i] = 0;
		}
	}
}

/*==============================
void calc_top_level_value (edict_t *ent)
calculates ent's top level value. Look in RK's documentation for an explanation of this.
==============================*/

void calc_top_level_value (edict_t *ent)
{
	unsigned short i;		//'for' counter
	float temp;			//holds the value of the top level before division
	qboolean	light;		//Is the player light/neutral, or dark
	int counter;			//counts the number of powers added to temp
	gforce_t	*power;	//pointer to current power(i)

	counter = 0;
	temp = 0.0f;

	if(ent->client->pers.force.affiliation > 0)	//is ent lightside?
	{
		light = 1;
	}
	else if(ent->client->pers.force.affiliation < 0)	//is ent darkside?
	{
		light = 0;
	}
	else	//else ent is neutral
	{
		light = 1;
	}

	for(i=1; i<NUM_POWERS+1; i++)
	{
		if(!ent->client->pers.force.powers[i])	//does the player know this power?
		{
			continue;
		}
		power = &powerlist[i];		//create pointer to the power
		if(power->lightdark == 0)		//is ent neutral?
		{
			temp += ent->client->pers.force.power_values[i];	//add the power's value into temp
			counter++;							//increment counter
		}
		else if (power->lightdark > 0 && light)	//is ent lightside?
		{
			temp += ent->client->pers.force.power_values[i];
			counter++;
		}
		else if (power->lightdark < 0 && !light) //is ent darkside?
		{
			temp += ent->client->pers.force.power_values[i];
			counter++;
		}
	}
	ent->client->pers.force.top_level = (temp/counter)/10;	//divide temp by the number of powers.
}



/*==============================
void calc_darklight_value (edict_t *ent)
calculates ent's darklight value. Look in RK's documentation for an explanation of this.
==============================*/

void calc_darklight_value (edict_t *ent)
{
	unsigned short i;		//'for' counter
	float temp;			//holds the value of the darklight before division
	float value;
	int counter;			//number of powers added to temp
	gforce_t	*power;	//pointer to power(i)

	counter = 0;
	temp = 0.0f;

	for(i=1; i<NUM_POWERS+1; i++)
	{
		if(!ent->client->pers.force.powers[i])	//does the player know this power?
		{
			continue;
		}
		power = GetPowerByIndex (i);

		value = ent->client->pers.force.power_values[i];
		if(power->lightdark == 1)	//is this power lightside?
		{
			temp += value;
			counter++;
		}
		else if (power->lightdark == -1)	//is this power darkside?
		{
			temp -= value;
			counter++;
		}
		else	//it's neutral
		{
			continue;
		}
	}
	ent->client->pers.force.affiliation = (temp/counter)/10;	//divide temp by number of powers added
}



/*==============================
void calc_subgroup_values (edict_t *ent)
calculates ent's subgroup values. Look in RK's documentation for an explanation of this.
==============================*/

void calc_subgroup_values (edict_t *ent)
{
	unsigned short i, j;			//'for' counters
	qboolean	cool[NUM_POWERS+1];	//urm, I can't remember =)
	float temp;				//temp value to store the sub group vaue before divide.
	int counter;				//counter of the number of powers in subgroup(i)
	gforce_t	*power;		//pointer to power(j)

	for(i=1; i<NUM_SUBGROUPS+1; i++)
	{
		counter = 0;
		temp = 0;
		for(j=1; j<NUM_POWERS+1; j++)
		{
			if(cool[j] == 1)
			{
				continue;
			}
			if(!ent->client->pers.force.powers[j])
			{
				continue;
			}
			power = &powerlist[j];
			if(power->subgroup != i)
			{
				continue;
			}
			temp += ent->client->pers.force.power_values[j];	//add value into temp
			counter++;	//increment counter
			cool[j] = 1;	//set cool =)
		}
		if(counter)	//if any powers in this subgroup added
		{
			ent->client->pers.force.subgroup_value[i] = (temp/counter)/10;
		}
	}
}



/*==============================
qboolean Force_Power_Available (edict_t *ent, int index)
checks to see whether power(index) is available for use
returns (1) if power is available
returns (0) if power in not available
==============================*/

qboolean Force_Power_Available (edict_t *ent, int index)
{
	gforce_t	*power;		//pointer to the power
	float		max_pow, min_pow;	//limits of players abilities. (50 each way of players affiliation)
	int check;

//	if(ent->client->pers.force.powers[index] == 0)
//		return 0;
	check = 0;

	power = &powerlist[index];
	//set limits
	max_pow = ent->client->pers.force.affiliation + 50;
	min_pow = ent->client->pers.force.affiliation - 50;

//CHECK #1
//Does the power fall within the affiliation limits (max_pow, min_pow)?
	if(!power->lightdark)	//if power is neutral
	{
		if(power->skillreq > max_pow || power->skillreq < min_pow)
		{
			if(check)
				gi.dprintf("CHECK1 NEUTRAL: skill req too high %i > %f\n", power->skillreq, max_pow);
			return 0;
		}
	}
	else
	{
		if(power->skillreq > max_pow || power->skillreq < min_pow)
		{
			if(check)
				gi.dprintf("CHECK1 skill req too high %i > %f < %f\n", power->skillreq, max_pow, min_pow);
			return 0;
		}
	}

//CHECK #2
//Does the power's skill comply with the subgroup value for the power?
	if(power->lightdark >= 0)
	{
		if(power->skillreq > ent->client->pers.force.subgroup_value[power->subgroup])
		{
			if(check)
				gi.dprintf("CHECK2 LIGHT skillreq greater than SG value %i > %f\n", power->skillreq, ent->client->pers.force.subgroup_value[power->subgroup]);
			return 0;
		}
	}
	else
	{
		if(-power->skillreq > ent->client->pers.force.subgroup_value[power->subgroup])
		{
			if(check)
				gi.dprintf("CHECK2 DARK skillreq greater than SG value %i > %f\n", power->skillreq, ent->client->pers.force.subgroup_value[power->subgroup]);
			return 0;
		}
	}

//CHECK #3
//checkwhether the player actually has enough pool to cast it.
//	if(power->cost > ent->client->pers.force.pool)
//	{
//		gi.dprintf("CHECK3 cost too high %i > %f\n", power->cost, ent->client->pers.force.pool);
//		return 0;
//	}
	return 1;
}


/*==============================
int Force_constant_active (edict_t *ent, int index)
checks to see whether power(index) is active as a constant power
returns (255) if it is not active.
returns the force_table slot number if it is active.
==============================*/

int Force_constant_active (edict_t *ent, int index)
{
	int i;

	for(i=0; i<6; i++)
	{
		if(ent->client->pers.force_table[i] == index)
		{
			return i;
		}
	}
	return 255;
}


/*==============================
qboolean Force_Power_Useable (edict_t *ent, int index)
checks to see whether power(index) is useable
returns (1) if it is useable.
returns (0) if it is not.
==============================*/

qboolean Force_Power_Useable (edict_t *ent, int index)
{
	gforce_t	*power;

	power = GetPowerByIndex (index);
//Check one
	if(Force_Power_Available (ent, index) == 0)
	{
		return 0;
	}
//Check two
	if(ent->client->pers.force.pool < power->cost)
	{
		return 0;
	}

	return 1;
}



//LEARNING CODE
//CaRRaC 11 April 1999

/*==============================
void force_unlearn (edict_t *ent, int index, float lose)
'unlearns' powers, to balance the system.
ent = the ent who is unlearning
index = power which is causing the unlearn
lose = ammount of skill to lose.
==============================*/

void force_unlearn (edict_t *ent, int index, float lose)
{
	gforce_t	*power;
	int		i, side, highest, highvalue;

	if (deathmatch->value)
	{
		return;
	}

	side = powerlist[index].lightdark;

	if(side == 0)
		return;

	highest = index;
	highvalue = 0;

	for(i=1; i<NUM_POWERS+1; i++)
	{
		power = &powerlist[i];
		if(power->lightdark == side || power->lightdark == 0)
			continue;
		if(ent->client->pers.force.power_values[i] > highvalue)
		{
			highvalue = ent->client->pers.force.power_values[i];
			highest = i;
		}
	}
	if(highest == index)
	{
//		gi.dprintf("NO POWERS ARE LOSING\n");
		return;
	}
	else
	{
//		gi.dprintf("%i is losing %f\n", highest, lose);
		ent->client->pers.force.power_values[highest] -= lose;
		if(ent->client->pers.force.power_values[highest] < 0)
		{
			ent->client->pers.force.power_values[highest] = 0;
		}
	}
}



/*==============================
void force_group_learn (edict_t *ent, int curr, float gain)
'Learns' powers, so that you can gain extra powers in the subgroup.
ent = the ent who is learning
curr = current power. It's subgroup will be used.
gain = ammount of skill to gain per power.
==============================*/

void force_group_learn (edict_t *ent, int curr, float gain)
{
	int i;
	gforce_t *power, *curpow;

	curpow = &powerlist[curr];

	for(i=1; i<NUM_POWERS+1; i++)
	{
		power = &powerlist[i];
//		if(power->lightdark == curpow->lightdark || power->lightdark == 0)
//		{
			if(power->subgroup == curpow->subgroup)
			{
				if(i == curr)
					continue;
				ent->client->pers.force.power_values[i] += gain;
				if(ent->client->pers.force.power_values[i] > 1000)
					ent->client->pers.force.power_values[i] = 1000;
			}
//		}
	}
}



/*==============================
void force_learn (edict_t *ent, int power)
==============================*/

void force_learn (edict_t *ent, int power)
{
	float powgain, grpgain;

	if(power == 0)
		return;

	if (deathmatch->value)
	{
		return;
	}

	powgain = FORCE_GAIN/ent->client->pers.force.power_values[power];
	grpgain = powgain/5;

	ent->client->pers.force.power_values[power] += powgain;
	if(ent->client->pers.force.power_values[power] > 1000)
		ent->client->pers.force.power_values[power] = 1000;

//	gi.dprintf("learn %f\n", powgain);

	force_unlearn (ent, power, powgain);
	force_group_learn (ent, power, grpgain);

//	sort_useable_powers(ent);
//	calc_subgroup_values(ent);
//	calc_darklight_value(ent);
//	calc_top_level_value(ent);
}



/*==============================
void Check_Active_Powers (edict_t *ent)
Checks all powers in the active power list (force_table[])
It performs everything required, like pool drain, icon showing, etc
==============================*/

void Drain_Force_Pool (edict_t *ent, int power)
{
	ent->client->pers.force.pool -= powerlist[power].cost;
	if(ent->client->pers.force.pool <= 0)
	{
		ent->client->pers.force.pool = 0;
		ent->client->pers.force_table[0] = 0;
		ent->client->pers.force_table[1] = 0;
		ent->client->pers.force_table[2] = 0;
		ent->client->pers.force_table[3] = 0;
		ent->client->pers.active_constants = 0;
		ent->s.modelindex = 255;
	}
}

void Drain_Force_Pool_Constant (edict_t *ent, int power)
{
	if(power == NFORCE_JUMP)
	{
		return;
	}

	if(ent->client->force_time > level.time)
	{
		return;
	}
	ent->client->force_time = level.time + FRAMETIME;

	ent->client->pers.force.pool -= powerlist[power].cost/10;
	if(ent->client->pers.force.pool <= 0)
	{
		ent->client->pers.force.pool = 0;
		ent->client->pers.force_table[0] = 0;
		ent->client->pers.force_table[1] = 0;
		ent->client->pers.force_table[2] = 0;
		ent->client->pers.force_table[3] = 0;
		ent->client->pers.active_constants = 0;
		ent->s.modelindex = 255;
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("killsound.wav"), 1, ATTN_NORM, 0);
	}
}

void Check_Active_Powers (edict_t *ent)
{
	int i;

	ent->client->ps.stats[STAT_FACTIVE2] = 0;
	ent->client->ps.stats[STAT_FACTIVE3] = 0;
	ent->client->ps.stats[STAT_FACTIVE4] = 0;

	for(i=0; i<ent->client->pers.active_constants; i++)
	{
		switch(i)
		{
			case 0:
				force_learn(ent, ent->client->pers.force_table[i]);
				ent->client->ps.stats[STAT_FACTIVE2] = gi.imageindex(powerlist[ent->client->pers.force_table[i]].icon);
				Drain_Force_Pool_Constant (ent, ent->client->pers.force_table[i]);
/*				ent->client->pers.force.pool -= powerlist[ent->client->pers.force_table[i]].cost/10;
				if(ent->client->pers.force.pool <= 0)
				{
					ent->client->pers.force.pool = 0;
					ent->client->pers.force_table[0] = 0;
					ent->client->pers.force_table[1] = 0;
					ent->client->pers.force_table[2] = 0;
					ent->client->pers.force_table[3] = 0;
					ent->client->pers.active_constants = 0;
					ent->s.modelindex = 255;
				}*/
				break;
			case 1:
				force_learn(ent, ent->client->pers.force_table[i]);
				ent->client->ps.stats[STAT_FACTIVE3] = gi.imageindex(powerlist[ent->client->pers.force_table[i]].icon);
				Drain_Force_Pool_Constant (ent, ent->client->pers.force_table[i]);
/*				ent->client->pers.force.pool -= powerlist[ent->client->pers.force_table[i]].cost/10;
				if(ent->client->pers.force.pool <= 0)
				{
					ent->client->pers.force.pool = 0;
					ent->client->pers.force_table[0] = 0;
					ent->client->pers.force_table[1] = 0;
					ent->client->pers.force_table[2] = 0;
					ent->client->pers.force_table[3] = 0;
					ent->client->pers.active_constants = 0;
					ent->s.modelindex = 255;
				}*/
				break;
			case 2:
				force_learn(ent, ent->client->pers.force_table[i]);
				ent->client->ps.stats[STAT_FACTIVE4] = gi.imageindex(powerlist[ent->client->pers.force_table[i]].icon);
				Drain_Force_Pool_Constant (ent, ent->client->pers.force_table[i]);
/*				ent->client->pers.force.pool -= powerlist[ent->client->pers.force_table[i]].cost/10;
				if(ent->client->pers.force.pool <= 0)
				{
					ent->client->pers.force.pool = 0;
					ent->client->pers.force_table[0] = 0;
					ent->client->pers.force_table[1] = 0;
					ent->client->pers.force_table[2] = 0;
					ent->client->pers.force_table[3] = 0;
					ent->client->pers.active_constants = 0;
					ent->s.modelindex = 255;
				}*/
				break;
			case 3:
				force_learn(ent, ent->client->pers.force_table[i]);
				Drain_Force_Pool_Constant (ent, ent->client->pers.force_table[i]);
/*				ent->client->pers.force.pool -= powerlist[ent->client->pers.force_table[i]].cost/10;
				if(ent->client->pers.force.pool <= 0)
				{
					ent->client->pers.force.pool = 0;
					ent->client->pers.force_table[0] = 0;
					ent->client->pers.force_table[1] = 0;
					ent->client->pers.force_table[2] = 0;
					ent->client->pers.force_table[3] = 0;
					ent->client->pers.active_constants = 0;
					ent->s.modelindex = 255;
				}*/
				break;
			default:
				break;
		}
	}
}



/*==============================
void check_new_powers_available (edict_t *ent)
Checks for any powers which, according to the rules, should now be available for use by ent
==============================*/

void check_new_powers_available (edict_t *ent)
{
	gforce_t	*power;
	float		max_pow, min_pow;
	int		i;

	max_pow = ent->client->pers.force.affiliation + 50;
	min_pow = ent->client->pers.force.affiliation - 50;

	for(i=1; i<NUM_POWERS+1; i++)
	{
		if(ent->client->pers.force.powers[i] == 1)
			continue;

//		power = GetPowerByIndex (i);
		power = &powerlist[i];

		if(!power->lightdark)
		{
//			gi.dprintf("NN%s %i %f %f\n", power->name, power->skillreq, min_pow, max_pow);
			if(power->skillreq > max_pow && power->skillreq < min_pow)
			{
				continue;
			}
		}
		else
		{
//			gi.dprintf("LD%s %i %f %f\n", power->name, power->skillreq, min_pow, max_pow);
			if(power->skillreq < max_pow && power->skillreq > min_pow)
			{
				ent->client->pers.force.powers[i] = 1;
				ent->client->pers.force.power_values[i] = power->start_level;
//				safe_centerprintf(ent, "Force power %s Has become available to you\n", power->name);
			}
		}
	}
}



/*==============================
void Think_Force (edict_t *ent)
void Think_Force_hold (edict_t *ent)
Checks if the force power use button is depressed, and executes the required functions for the current power.
==============================*/

void Think_Force (edict_t *ent)
{
	int power;

//	sort_useable_powers(ent);
//	calc_subgroup_values(ent);
//	calc_darklight_value(ent);
//	calc_top_level_value(ent);
//	Check_Active_Powers (ent);
//	check_new_powers_available(ent);

	if((ent->forceflags[AFF_LIGHT] & FFL_WALL) || (ent->forceflags[AFF_DARK] & FFD_DARKNESS))
	{
		ent->forceflags[AFF_NEUTRAL] = 0;
		ent->forceflags[AFF_LIGHT] = 0;
		ent->forceflags[AFF_DARK] = 0;
		return;
	}

	ent->forceflags[AFF_NEUTRAL] = 0;
	ent->forceflags[AFF_LIGHT] = 0;
	ent->forceflags[AFF_DARK] = 0;

	if(ent->deadflag)
		return;

	if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE)
	{
		if(ent->client->icon_menu_active == 1)
		{
			select_menu_power(ent);
			ent->client->menu_time = level.time - 1;
			return;
		}

		if(ent->client->force_last_frame && !(ent->client->pers.force_power->constant == 2))
		{
			ent->client->ps.stats[STAT_FACTIVE1] = 0;
			return;
		}

		power = FindPowerByName (ent->client->pers.force_power->name);
		
//		if(Force_Power_Available (ent, power) == 0)
//			return;
		if(Force_Power_Useable (ent, power) == 0)
			return;
		ent->client->pers.force_power->think(ent, 1);
//		gi.positioned_sound (ent->s.origin, ent, CHAN_WEAPON, gi.soundindex ("The Force.wav"), 1, ATTN_NORM, 0);
		ent->client->force_last_frame = 1;
		if(ent->client->pers.force_power->constant != 1)
		{
			force_learn(ent, ent->client->pers.current_power);
			ent->client->ps.stats[STAT_FACTIVE1] = gi.imageindex(ent->client->pers.force_power->icon);
			ent->client->pers.force.pool -= ent->client->pers.force_power->cost;
			if(ent->client->pers.force.pool <= 0)
			{
				ent->client->pers.force.pool = 0;
				ent->client->pers.force_table[0] = 0;
				ent->client->pers.force_table[1] = 0;
				ent->client->pers.force_table[2] = 0;
				ent->client->pers.force_table[3] = 0;
				ent->client->pers.active_constants = 0;
				ent->s.modelindex = 255;
			}
		}
	}
	else
	{
		ent->client->ps.stats[STAT_FACTIVE1] = 0;
		ent->client->force_last_frame = 0;
	}
}

void Think_Force_hold (edict_t *ent)
{
	int power;


//	sort_useable_powers(ent);
//	calc_subgroup_values(ent);
//	calc_darklight_value(ent);
//	calc_top_level_value(ent);
//	Check_Active_Powers (ent);
//	check_new_powers_available(ent);

	if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_USE) )
	{
		ent->client->latched_buttons &= ~BUTTON_USE;
		//*****************************************
		//FIXME   check here for force usage tests.
		//*****************************************
		ent->client->force_last_frame = 1;
		ent->client->pers.force_power->think(ent, 0);
		ent->client->ps.stats[STAT_FACTIVE1] = gi.imageindex(ent->client->pers.force_power->icon);
	}
	else
	{
		if(ent->client->force_last_frame)
		{
			power = FindPowerByName (ent->client->pers.force_power->name);

			if(Force_Power_Available (ent, power) == 0)
				return;
//			if(Force_Power_Useable (ent, power) == 0)
//				return;
			ent->client->pers.force_power->think(ent, 1);
			ent->client->force_last_frame = 0;
			ent->client->force_time = 0;
			ent->client->force_target = NULL;
			ent->client->ps.stats[STAT_FACTIVE1] = 0;
		}
	}
}


//******************************
//******************************
//******************************
//ACTUAL FORCE POWER EFFECT CODE
//******************************
//******************************
//******************************

//EF_TRACKER = darkness effect
//EF_FLAG1 = light side effect
//EF_FLAG2 = light side effect

void heal_effect_update (edict_t *self)
{
	vec3_t dir;
	vec3_t vec;
	float dist;

	self->nextthink = level.time + FRAMETIME;
	self->count++;

	if(self->count > 15 || (self->velocity[1] == 0))
	{
		G_FreeEdict(self);
		return;
	}

	VectorSubtract (self->s.origin, self->owner->s.origin, vec);
	dist = VectorLengthSquared(vec);

	if(dist < 65536)
	{
		G_FreeEdict(self);
		return;
	}

	VectorSubtract (self->owner->s.origin, self->s.origin, dir);
	VectorNormalize (dir);

	VectorScale (dir, 2000, self->velocity);
}

void heal_effect_spawn (vec3_t from, edict_t *to, qboolean side)
{
	edict_t *bolt;
	vec3_t dir;

	VectorSubtract (to->s.origin, from, dir);
	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (from, bolt->s.origin);
	VectorScale (dir, 2000, bolt->velocity);

	bolt->count = 0;
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->solid = SOLID_NOT;
	bolt->s.modelindex = gi.modelindex("sprites/s_flash.sp2");
	bolt->owner = to;

	if(side)
	{
		bolt->s.effects |= EF_TRACKER;
	}
	else
	{
		if(random() > 0.5)
			bolt->s.effects |= EF_FLAG1;
		else
			bolt->s.effects |= EF_FLAG2;
	}

	bolt->s.effects |= EF_ANIM_ALLFAST;

	VectorSet (bolt->mins, -3, -3, -3);
	VectorSet (bolt->maxs, 3, 3, 3);

	bolt->nextthink = level.time + FRAMETIME;
	bolt->think = heal_effect_update;
                
	gi.linkentity (bolt);
}

void heal_effect (edict_t *ent, int level, qboolean side)
{
	int	i;
	vec3_t rvel;
	trace_t tr;

	for(i=0;i<level;i++)
	{
		rvel[0] = (random() * 8192) - 4096;
		rvel[1] = (random() * 8192) - 4096;
		rvel[2] = (random() * 1024) - 512;

		tr = gi.trace(ent->s.origin, NULL, NULL, rvel, ent, MASK_ALL);

		heal_effect_spawn (tr.endpos, ent, side);
	}
}

void force_push_hold (edict_t *ent, int last)
{
	vec3_t start, end, kvel, forward;
	float strength;
	trace_t tr;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);

//	ent->client->pers.force.pool -= ent->client->pers.force_power->cost;

	if(!(ent->client->force_target))
	{
		VectorCopy(ent->s.origin, start);
		start[2] += ent->viewheight;
		VectorMA(start, 1024, forward, end);
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_ALL);
		if(!tr.ent || !(strcmp(tr.ent->classname, "worldspawn")) || tr.ent->solid != SOLID_BBOX)
		{
			return;
		}
		ent->client->force_target = tr.ent;
	}

	if(!ent->client->force_time)
		ent->client->force_time = level.time;

	strength = level.time - ent->client->force_time;

	if(last)
	{
		VectorScale (forward, (200*strength), kvel);
		VectorAdd (ent->client->force_target->velocity, kvel, ent->client->force_target->velocity);
	}
}

void airburst(edict_t *self, vec3_t origin, float power, float radius)
{
	edict_t *ent=NULL;
	vec3_t dir;
	float dist, strength;
	qboolean test=false;

	while ((ent = findradius(ent, origin, radius)) != NULL)
	{
		if (!ent->takedamage)
			continue;
		VectorSubtract(ent->s.origin, origin, dir);
		dist = VectorLength(dir);
		strength = (power*(1/dist))*10;
		VectorNormalize(dir);
		VectorScale(dir,strength,dir);
		VectorAdd(dir, ent->velocity, ent->velocity);
	}
}

void fire_airburst (edict_t *self, vec3_t start, vec3_t aimdir)
{
	vec3_t          end,forward;
	trace_t         tr;
	int             mask;
	float strength;

	strength = self->client->pers.force.power_values[NFORCE_PUSH];

	VectorMA (start, (strength+strength), aimdir, end);
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	tr = gi.trace (start, NULL, NULL, end, self, mask);
	if (tr.fraction==1)
		return;

	if(!strcmp(tr.ent->classname, "func_button"))
	{
		button_use(tr.ent, self, self);
		return;
	}

	if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
	{       
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorScale(forward, strength/2, forward);
		VectorAdd(forward, tr.ent->velocity, tr.ent->velocity);
	}
	else if (tr.ent->solid==(SOLID_BSP|SOLID_BBOX))
	{
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
		airburst(self, tr.endpos, strength, 300);
		return;
	}               
}

void force_push_fire (edict_t *ent)
{
	vec3_t          start;
	vec3_t          forward, right;
	vec3_t          offset;
	float			kick;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	kick = -(3*(ent->client->pers.force.power_values[NFORCE_PUSH]/1000));

	VectorScale (forward, kick, ent->client->kick_origin);
	ent->client->kick_angles[0] = kick;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_airburst (ent, start, forward);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void force_push (edict_t *ent, int last)
{
	force_push_fire (ent);

	if(ent->client->force_countdown < 1)
	{
		ent->client->current_force = NFORCE_PUSH;
		if(ent->client->anim_duck)
		{
			ent->s.frame = FRAME_crouchfinA;
			ent->client->anim_end = FRAME_crouchfinB;
		}
		else
		{
			ent->s.frame = FRAME_forceinA;
			ent->client->anim_end = FRAME_forceinB;
		}
		ForceAnimCD(ent);
	}
	else if(ent->client->force_countdown == 1)
	{
		if(ent->client->current_force == NFORCE_PUSH)
		{
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchflpA;
				ent->client->anim_end = FRAME_crouchflpB;
			}
			else
			{
				ent->s.frame = FRAME_forcelpA;
				ent->client->anim_end = FRAME_forcelpB;
			}
		}
		else
		{
			ent->client->current_force = NFORCE_PUSH;
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchfinA;
				ent->client->anim_end = FRAME_crouchfinB;
			}
			else
			{
				ent->s.frame = FRAME_forceinA;
				ent->client->anim_end = FRAME_forceinB;
			}
			ForceAnimCD(ent);
		}
	}
	if(ent->client->forcesound == 0)
	{
		sound_delay(ent, 0.5, 0);
//		sound_delay(ent, 1.985, 0);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/pull.wav"), 1, ATTN_NORM, 0);
	}
	else if(ent->client->forcesound == 2)
	{
		sound_delay(ent, 0.5, 0);
//		sound_delay(ent, 1.985, 0);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/pull.wav"), 1, ATTN_NORM, 0);
	}
}

void force_pull (edict_t *ent, int last)
{
	vec3_t 		start, end, forward, kvel, temp;
	trace_t		tr;
	float			level;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	VectorMA(start, 1024, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_FORCE);

	if(ent->client->force_countdown < 1)
	{
		ent->client->current_force = NFORCE_PULL;
		if(ent->client->anim_duck)
		{
			ent->s.frame = FRAME_crouchfinA;
			ent->client->anim_end = FRAME_crouchfinB;
		}
		else
		{
			ent->s.frame = FRAME_forceinA;
			ent->client->anim_end = FRAME_forceinB;
		}
		ForceAnimCD(ent);
	}
	else if(ent->client->force_countdown == 1)
	{
		if(ent->client->current_force == NFORCE_PULL)
		{
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchflpA;
				ent->client->anim_end = FRAME_crouchflpB;
			}
			else
			{
				ent->s.frame = FRAME_forcelpA;
				ent->client->anim_end = FRAME_forcelpB;
			}
		}
		else
		{
			ent->client->current_force = NFORCE_PULL;
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchfinA;
				ent->client->anim_end = FRAME_crouchfinB;
			}
			else
			{
				ent->s.frame = FRAME_forceinA;
				ent->client->anim_end = FRAME_forceinB;
			}
			ForceAnimCD(ent);
		}
	}

	if(ent->client->forcesound == 0)
	{
		sound_delay(ent, 0.5, 0);
//		sound_delay(ent, 1.985, 0);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/pull.wav"), 1, ATTN_NORM, 0);
	}
	else if(ent->client->forcesound == 2)
	{
		sound_delay(ent, 0.5, 0);
//		sound_delay(ent, 1.985, 0);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/pull.wav"), 1, ATTN_NORM, 0);
	}

	if(!tr.ent)
	{
		if(!ent->client->force_target)
		{
			return;
		}
	}
	if((strcmp(tr.ent->classname, "worldspawn") == 0) || tr.ent->solid != SOLID_BBOX)
	{
		if(!ent->client->force_target)
			return;
	}
	if(tr.ent)
	{
		ent->client->force_target = tr.ent;
	}

	level = ent->client->pers.force.power_values[2];

	ent->client->force_target->forceflags[0] |= FFN_PULL;

	if(deathmatch->value)
	{
		level = level;
//		gi.dprintf("%s %f\n", ent->client->force_target->classname, level);
		VectorSubtract(ent->s.origin, ent->client->force_target->s.origin, temp);
		VectorNormalize2(temp, forward);
		VectorScale (forward, level, kvel);
		VectorAdd (ent->client->force_target->velocity, kvel, ent->client->force_target->velocity);/*
		VectorCopy(forward, temp);
		VectorInverse(temp);
		VectorScale (temp, level*1.5, temp);
		VectorAdd (ent->client->force_target->velocity, temp, ent->client->force_target->velocity);*/
	}
	else
	{
		level = level/5;
		VectorSubtract(ent->s.origin, ent->client->force_target->s.origin, temp);
		VectorNormalize2(temp, forward);
		VectorScale (forward, level, kvel);
		VectorAdd (ent->client->force_target->velocity, kvel, ent->client->force_target->velocity);
	}
}

void force_levitate (edict_t *ent, int last)
{
	int index;

	index = Force_constant_active(ent, NFORCE_LEVITATE);

	if(ent->client->anim_duck)
	{
		ent->client->current_force = NFORCE_LEVITATE;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = NFORCE_LEVITATE;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	if(index == 255)
	{
		if(ent->client->pers.active_constants == 6)
			return;
		ent->client->pers.force_table[ent->client->pers.active_constants] = NFORCE_LEVITATE;
		ent->client->pers.active_constants++;
		if(ent->client->forcesound == 0)
		{
			sound_delay(ent, 2.743, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/levstart.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->client->forcesound == 2)
		{
			sound_delay(ent, 2.743, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/levstart.wav"), 1, ATTN_NORM, 0);
		}
	}
	else
	{
		ent->client->pers.force_table[index] = 255;
		if(ent->client->pers.active_constants != 0)
			ent->client->pers.active_constants--;
		if (ent->client->forcesound == 2)
		{
			sound_delay(ent, 2.743, 0);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/levloop.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->client->forcesound == 0)
		{
			sound_delay(ent, 1.776, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/levloop.wav"), 1, ATTN_NORM, 0);
		}
	}
}

void force_negate (edict_t *ent, int last)
{
	int i;

	if(ent->client->anim_duck)
	{
		ent->client->current_force = NFORCE_NEGATE;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = NFORCE_NEGATE;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	if (ent->client->forcesound == 0)
	{
		sound_delay(ent, 0.5, 0);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/negate.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->client->forcesound == 2)
	{
		sound_delay(ent, 0.5, 2);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/negate.wav"), 1, ATTN_NORM, 0);
	}
	if(deathmatch->value)
	{
		for(i=0; i<num_players; i++)
		{
			if(players[i]->client->force_target == ent)
			{
				players[i]->client->force_target = NULL;
			}
		}
	}
}

void force_jump (edict_t *ent, int last)
{
	int index;

	index = Force_constant_active(ent, NFORCE_JUMP);

	if(ent->client->anim_duck)
	{
		ent->client->current_force = NFORCE_JUMP;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = NFORCE_JUMP;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	if(index == 255)
	{
		if(ent->client->pers.active_constants == 6)
			return;
		ent->client->pers.force_table[ent->client->pers.active_constants] = NFORCE_JUMP;
		ent->client->pers.active_constants++;
	}
	else
	{
		ent->client->pers.force_table[index] = 255;
		if(ent->client->pers.active_constants != 0)
			ent->client->pers.active_constants--;
	}
}

void force_speed (edict_t *ent, int last)
{
	int index;

	index = Force_constant_active(ent, NFORCE_SPEED);

	if(ent->client->anim_duck)
	{
		ent->client->current_force = NFORCE_SPEED;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = NFORCE_SPEED;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	if(index == 255)
	{
		if(ent->client->pers.active_constants == 6)
			return;
		ent->client->pers.force_table[ent->client->pers.active_constants] = NFORCE_SPEED;
		ent->client->pers.active_constants++;
	}
	else
	{
		ent->client->pers.force_table[index] = 255;
		if(ent->client->pers.active_constants != 0)
			ent->client->pers.active_constants--;
	}
}

void force_saber_throw (edict_t *ent, int last)
{
}

//light
void force_lightheal (edict_t *ent, int last)
{
	float level;

	if(ent->client->force_countdown < 1)
	{
		ent->client->current_force = LFORCE_HEAL;
		if(ent->client->anim_duck)
		{
			ent->s.frame = FRAME_crouchfinA;
			ent->client->anim_end = FRAME_crouchfinB;
		}
		else
		{
			ent->s.frame = FRAME_forceinA;
			ent->client->anim_end = FRAME_forceinB;
		}
		ForceAnimCD(ent);
	}
	else if(ent->client->force_countdown == 1)
	{
		if(ent->client->current_force == LFORCE_HEAL)
		{
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchflpA;
				ent->client->anim_end = FRAME_crouchflpB;
			}
			else
			{
				ent->s.frame = FRAME_forcelpA;
				ent->client->anim_end = FRAME_forcelpB;
			}
		}
		else
		{
			ent->client->current_force = LFORCE_HEAL;
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchfinA;
				ent->client->anim_end = FRAME_crouchfinB;
			}
			else
			{
				ent->s.frame = FRAME_forceinA;
				ent->client->anim_end = FRAME_forceinB;
			}
			ForceAnimCD(ent);
		}
	}

	if (ent->client->forcesound == 0)
	{
		sound_delay(ent, 0.2, 0);
//		sound_delay(ent, 0.653, 0);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/lightheal.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->client->forcesound == 2)
	{
		sound_delay(ent, 0.2, 0);
//		sound_delay(ent, 0.653, 2);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/lightheal.wav"), 1, ATTN_NORM, 0);
	}

	if(ent->health >= 100)
		return;

	level = ent->client->pers.force.power_values[LFORCE_HEAL];
	level = level/50;

	if(level < 1)
		level = 1;

	ent->health += level;
	if(ent->health > ent->max_health)
		ent->health = ent->max_health;

	level = (int)level/2;

	heal_effect (ent, level, 0);
}

void force_wall_of_light (edict_t *ent, int last)
{
	int i;
	float mylevel, yourlevel;

	if(ent->client->anim_duck)
	{
		ent->client->current_force = LFORCE_WALL;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = LFORCE_WALL;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	mylevel = -(ent->client->pers.force.power_values[LFORCE_WALL]/10);

	if(deathmatch->value)
	{
		for(i=0; i<num_players; i++)
		{
			if(infront(ent, players[i]))
			{
				yourlevel = players[i]->client->pers.force.top_level;
				if(yourlevel >= mylevel)
				{
					players[i]->forceflags[AFF_LIGHT] |= FFL_WALL;
				}
			}
		}
	}
}

void force_shield (edict_t *ent, int last)
{
	int index;

	index = Force_constant_active(ent, LFORCE_SHIELD);

	if(ent->client->anim_duck)
	{
		ent->client->current_force = LFORCE_SHIELD;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = LFORCE_SHIELD;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	if(index == 255)
	{
		if(ent->client->pers.active_constants == 6)
			return;
		ent->client->pers.force_table[ent->client->pers.active_constants] = LFORCE_SHIELD;
		ent->client->pers.active_constants++;
		if (ent->client->forcesound == 0)
		{
			sound_delay(ent, 0.2, 0);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/shield.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->client->forcesound == 2)
		{
			sound_delay(ent, 0.2, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/shield.wav"), 1, ATTN_NORM, 0);
		}
	}
	else
	{
		ent->client->pers.force_table[index] = 255;
		if(ent->client->pers.active_constants != 0)
			ent->client->pers.active_constants--;
	}
}

void force_invisibility (edict_t *ent, int last)
{
	int index;

	index = Force_constant_active(ent, LFORCE_INVISIBILITY);

	if(ent->client->anim_duck)
	{
		ent->client->current_force = LFORCE_INVISIBILITY;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = LFORCE_INVISIBILITY;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	if(index == 255)
	{
		if(ent->client->pers.active_constants == 6)
			return;
		ent->client->pers.force_table[ent->client->pers.active_constants] = LFORCE_INVISIBILITY;
		ent->client->pers.active_constants++;
		ent->client->invisi_time = level.time - 1;
		if (ent->client->forcesound == 0)
		{
			sound_delay(ent, 0.6, 0);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/invis.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->client->forcesound == 2)
		{
			sound_delay(ent, 0.6, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/invis.wav"), 1, ATTN_NORM, 0);
		}
	}
	else
	{
		ent->client->pers.force_table[index] = 255;
		if(ent->client->pers.active_constants != 0)
			ent->client->pers.active_constants--;
		ent->s.modelindex = 255;
	}
}

void airburst_wind(edict_t *self, vec3_t origin, float power, float radius)
{
	edict_t *ent=NULL;
	vec3_t dir;
	float dist;
	qboolean test=false;

	if(self->client->anim_duck)
	{
		self->client->current_force = LFORCE_WINDS;
		self->s.frame = FRAME_crouchfinA;
		self->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		self->client->current_force = LFORCE_WINDS;
		self->s.frame = FRAME_forceinA;
		self->client->anim_end = FRAME_forceinB;
	}

	while ((ent = findradius(ent, origin, radius)) != NULL)
	{
		if (!ent->takedamage)
			continue;
		if(ent->solid != SOLID_BBOX)
			continue;
		if(ent == self)
			continue;
		if(!(gi.inPVS(self->s.origin, ent->s.origin)))
			continue;
		VectorSubtract(ent->s.origin, origin, dir);
		dist = VectorLength(dir);
		dir[2] += 30;
		VectorNormalize(dir);
		VectorScale(dir,power,dir);
		VectorAdd(dir, ent->velocity, ent->velocity);
	}
}

void force_wind (edict_t *ent, int last)
{
	gforce_t		*power;

	power = &powerlist[LFORCE_WINDS];
	
	if(ent->client->pers.force.pool <= power->cost)
		return;

	airburst_wind(ent, ent->s.origin, 10000, ent->client->pers.force.power_values[LFORCE_WINDS]);
	if(ent->client->wind_time <= level.time)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_NUKEBLAST);
		gi.WritePosition (ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PHS);
		ent->client->wind_time = level.time + 2;
	}

	if (ent->client->forcesound == 0)
	{
		sound_delay(ent, 0.4, 0);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/wind.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->client->forcesound == 2)
	{
		sound_delay(ent, 0.4, 2);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/wind.wav"), 1, ATTN_NORM, 0);
	}
}

void force_reflect (edict_t *ent, int last)
{
	int index;

	index = Force_constant_active(ent, LFORCE_REFLECT);

	if(ent->client->anim_duck)
	{
		ent->client->current_force = LFORCE_REFLECT;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = LFORCE_REFLECT;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	if(index == 255)
	{
		if(ent->client->pers.active_constants == 6)
			return;
		ent->client->pers.force_table[ent->client->pers.active_constants] = LFORCE_REFLECT;
		ent->client->pers.active_constants++;
		if (ent->client->forcesound == 0)
		{
			sound_delay(ent, 0.2, 0);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/reflect.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->client->forcesound == 2)
		{
			sound_delay(ent, 0.2, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/reflect.wav"), 1, ATTN_NORM, 0);
		}
	}
	else
	{
		ent->client->pers.force_table[index] = 255;
		if(ent->client->pers.active_constants != 0)
			ent->client->pers.active_constants--;
	}
}

void force_scout (edict_t *ent, int last)
{
	int index;

	index = Force_constant_active(ent, LFORCE_SCOUT);

	if(ent->client->anim_duck)
	{
		ent->client->current_force = LFORCE_SCOUT;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = LFORCE_SCOUT;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	if(index == 255)
	{
		if(ent->client->pers.active_constants == 6)
			return;
		ent->client->pers.force_table[ent->client->pers.active_constants] = LFORCE_SCOUT;
		ent->client->pers.active_constants++;
		if (ent->client->forcesound == 0)
		{
			sound_delay(ent, 1, 0);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/scout.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->client->forcesound == 2)
		{
			sound_delay(ent, 1, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/scout.wav"), 1, ATTN_NORM, 0);
		}
	}
	else
	{
		ent->client->pers.force_table[index] = 255;
		if(ent->client->pers.active_constants != 0)
			ent->client->pers.active_constants--;
	}
}

void force_bind (edict_t *ent, int last)
{
	vec3_t 		start, end, forward;
	trace_t		tr;

	if(ent->client->force_countdown < 1)
	{
		ent->client->current_force = LFORCE_BIND;
		if(ent->client->anim_duck)
		{
			ent->s.frame = FRAME_crouchfinA;
			ent->client->anim_end = FRAME_crouchfinB;
		}
		else
		{
			ent->s.frame = FRAME_forceinA;
			ent->client->anim_end = FRAME_forceinB;
		}
		ForceAnimCD(ent);
	}
	else if(ent->client->force_countdown == 1)
	{
		if(ent->client->current_force == LFORCE_BIND)
		{
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchflpA;
				ent->client->anim_end = FRAME_crouchflpB;
			}
			else
			{
				ent->s.frame = FRAME_forcelpA;
				ent->client->anim_end = FRAME_forcelpB;
			}
		}
		else
		{
			ent->client->current_force = LFORCE_BIND;
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchfinA;
				ent->client->anim_end = FRAME_crouchfinB;
			}
			else
			{
				ent->s.frame = FRAME_forceinA;
				ent->client->anim_end = FRAME_forceinB;
			}
			ForceAnimCD(ent);
		}
	}

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	VectorMA(start, 1024, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_FORCE);
		if (ent->client->forcesound == 0)
		{
			sound_delay(ent, 0.2, 0);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/bind.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->client->forcesound == 2)
		{
			sound_delay(ent, 0.2, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/bind.wav"), 1, ATTN_NORM, 0);
		}
	if(!tr.ent)
	{
		if(!ent->client->force_target)
		{
			return;
		}
	}
	if((strcmp(tr.ent->classname, "worldspawn") == 0) || tr.ent->solid != SOLID_BBOX)
	{
		if(!ent->client->force_target)
			return;
	}
	else if(tr.ent)
	{
		ent->client->force_target = tr.ent;
	}

	ent->client->force_target->forceflags[AFF_LIGHT] |= FFL_BIND;
}


//dark
void force_darkheal (edict_t *ent, int last)
{
	float level;

	if(ent->client->force_countdown < 1)
	{
		ent->client->current_force = DFORCE_HEAL;
		if(ent->client->anim_duck)
		{
			ent->s.frame = FRAME_crouchfinA;
			ent->client->anim_end = FRAME_crouchfinB;
		}
		else
		{
			ent->s.frame = FRAME_forceinA;
			ent->client->anim_end = FRAME_forceinB;
		}
		ForceAnimCD(ent);
	}
	else if(ent->client->force_countdown == 1)
	{
		if(ent->client->current_force == DFORCE_HEAL)
		{
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchflpA;
				ent->client->anim_end = FRAME_crouchflpB;
			}
			else
			{
				ent->s.frame = FRAME_forcelpA;
				ent->client->anim_end = FRAME_forcelpB;
			}
		}
		else
		{
			ent->client->current_force = DFORCE_HEAL;
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchfinA;
				ent->client->anim_end = FRAME_crouchfinB;
			}
			else
			{
				ent->s.frame = FRAME_forceinA;
				ent->client->anim_end = FRAME_forceinB;
			}
			ForceAnimCD(ent);
		}
	}

	if (ent->client->forcesound == 0)
	{
		sound_delay(ent, 0.3, 0);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/darkheal.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->client->forcesound == 2)
	{
		sound_delay(ent, 0.3, 2);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/darkheal.wav"), 1, ATTN_NORM, 0);
	}

	if(ent->health >= 100)
		return;

	level = ent->client->pers.force.power_values[LFORCE_HEAL];
	level = level/100;

	if(level < 1)
		level = 1;

	ent->health += level;
	ent->client->force_added_health += level;
	if(ent->health > ent->max_health)
		ent->health = ent->max_health;

	level = (int)level/2;

	heal_effect (ent, level, 1);

}

void fire_lightning(edict_t *ent, vec3_t g_offset)
{
	vec3_t	forward, right;
	vec3_t	start, end;
	trace_t	tr;
	int		damage;



	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, g_offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorMA(ent->s.origin, 8096, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_LIGHTNING);
	gi.WriteShort (tr.ent - g_edicts);
	gi.WriteShort (ent - g_edicts);
	gi.WritePosition (tr.endpos);
	gi.WritePosition (start);
	gi.multicast (start, MULTICAST_PVS);

	damage = ent->client->pers.force.power_values[DFORCE_LIGHTNING]/80;

	if (tr.ent->takedamage)
	{
		T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, damage, 0, DAMAGE_ENERGY, MOD_LIGHTNING);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void force_lightning (edict_t *ent, int last)
{
	vec3_t	offset;

	VectorSet(offset, 14, 3, ent->viewheight-4);
	fire_lightning(ent, offset);
	if(ent->client->force_countdown < 1)
	{
		ent->client->current_force = DFORCE_LIGHTNING;
		if(ent->client->anim_duck)
		{
			ent->s.frame = FRAME_crouchfinA;
			ent->client->anim_end = FRAME_crouchfinB;
		}
		else
		{
			ent->s.frame = FRAME_forceinA;
			ent->client->anim_end = FRAME_forceinB;
		}
		ForceAnimCD(ent);
	}
	else if(ent->client->force_countdown == 1)
	{
		if(ent->client->current_force == DFORCE_LIGHTNING)
		{
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchflpA;
				ent->client->anim_end = FRAME_crouchflpB;
			}
			else
			{
				ent->s.frame = FRAME_forcelpA;
				ent->client->anim_end = FRAME_forcelpB;
			}
		}
		else
		{
			ent->client->current_force = DFORCE_LIGHTNING;
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchfinA;
				ent->client->anim_end = FRAME_crouchfinB;
			}
			else
			{
				ent->s.frame = FRAME_forceinA;
				ent->client->anim_end = FRAME_forceinB;
			}
			ForceAnimCD(ent);
		}
	}

	if (ent->client->forcesound == 0)
	{
		makelightning(ent);
		ent->client->light_sound = 12;
		if(random() > 0.75)
		{
//			sound_delay(ent, 0.8, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/lght1.wav"), 1, ATTN_NORM, 0);
		}
		else if(random() > 0.66)
		{
//			sound_delay(ent, 0.8, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/lght2.wav"), 1, ATTN_NORM, 0);
		}
		else if(random() > 0.50)
		{
//			sound_delay(ent, 0.8, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/lght3.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
//			sound_delay(ent, 0.8, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/lght4.wav"), 1, ATTN_NORM, 0);
		}
	}
	else if (ent->client->forcesound == 2)
	{
		if (ent->client->light_sound < 1)
		{
			ent->client->light_sound = 11;
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/lghtloop.wav"), 1, ATTN_NORM, 0);
		}
		ent->client->light_countdown = 3;
//			if(random() > 0.75)
//			{
//				sound_delay(ent, 3.109, 2);
//				gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/lght1.wav"), 1, ATTN_NORM, 0);
//			}
//			else if(random() > 0.66)
//			{
//				sound_delay(ent, 3.109, 2);
//				gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/lght2.wav"), 1, ATTN_NORM, 0);
//			}
//			else if(random() > 0.50)
//			{
//				sound_delay(ent, 3.109, 2);
//				gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/lght3.wav"), 1, ATTN_NORM, 0);
//			}
//			else
//			{
//				sound_delay(ent, 3.109, 2);
//				gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/lght4.wav"), 1, ATTN_NORM, 0);
//			}
//		sound_delay(ent, 0.5, 2);
	}	
}

void force_choke (edict_t *ent, int last)
{
	vec3_t 		start, end, forward;
	trace_t		tr;
	unsigned	short	level;
	int damage;

//	ent->client->pers.force.pool -= powerlist[1].cost;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	VectorMA(start, 1024, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_FORCE);

	if(ent->client->force_countdown < 1)
	{
		ent->client->current_force = DFORCE_CHOKE;
		if(ent->client->anim_duck)
		{
			ent->s.frame = FRAME_crouchfchinA;
			ent->client->anim_end = FRAME_crouchfchinB;
		}
		else
		{
			ent->s.frame = FRAME_forcechinA;
			ent->client->anim_end = FRAME_forcechinB;
		}
		ForceAnimCD(ent);
	}
	else if(ent->client->force_countdown == 1)
	{
		if(ent->client->current_force == DFORCE_CHOKE)
		{
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchfchlpA;
				ent->client->anim_end = FRAME_crouchfchlpB;
			}
			else
			{
				ent->s.frame = FRAME_forcechA;
				ent->client->anim_end = FRAME_forcechB;
			}
		}
		else
		{
			ent->client->current_force = DFORCE_CHOKE;
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchfchinA;
				ent->client->anim_end = FRAME_crouchfchinB;
			}
			else
			{
				ent->s.frame = FRAME_forcechinA;
				ent->client->anim_end = FRAME_forcechinB;
			}
			ForceAnimCD(ent);
		}
	}

	if(!tr.ent)
	{
		if(!ent->client->force_target)
		{
			return;
		}
	}
	if(strcmp(tr.ent->classname, "worldspawn") == 0)
	{
		if(!ent->client->force_target)
			return;
	}
	else if(tr.ent)
	{
		if(!tr.ent->takedamage)
			return;
		ent->client->force_target = tr.ent;
//
//	RipVTide 15-Aug-2000
//	Choking animations for target routine
//
//		if(tr.contents == MASK_PLAYERSOLID)
//		{
			tr.ent->client->choking++;
			tr.ent->client->choking++;
//		}
	}
	if(ent->client->force_target->deadflag)
		return;

	level = ent->client->pers.force.power_values[1];

	damage = level/100;
	damage += 1;
	T_Damage (ent->client->force_target, ent, ent, vec3_origin, ent->client->force_target->s.origin, vec3_origin, damage, 1, 1, MOD_CHOKE);
}

void force_absorb (edict_t *ent, int last)
{
	int index;

	index = Force_constant_active(ent, DFORCE_ABSORB);

	if(ent->client->anim_duck)
	{
		ent->client->current_force = DFORCE_ABSORB;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = DFORCE_ABSORB;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	if(index == 255)
	{
		if(ent->client->pers.active_constants == 6)
			return;
		ent->client->pers.force_table[ent->client->pers.active_constants] = DFORCE_ABSORB;
		ent->client->pers.active_constants++;
	}
	else
	{
		ent->client->pers.force_table[index] = 255;
		if(ent->client->pers.active_constants != 0)
			ent->client->pers.active_constants--;
	}
}

void force_wall_of_darkness (edict_t *ent, int last)
{
	int i;
	float mylevel, yourlevel;

	mylevel = (ent->client->pers.force.power_values[DFORCE_DARKNESS]/10);

	if(ent->client->force_countdown < 1)
	{
		ent->client->current_force = DFORCE_DARKNESS;
		if(ent->client->anim_duck)
		{
			ent->s.frame = FRAME_crouchfinA;
			ent->client->anim_end = FRAME_crouchfinB;
		}
		else
		{
			ent->s.frame = FRAME_forceinA;
			ent->client->anim_end = FRAME_forceinB;
		}
		ForceAnimCD(ent);
	}
	else if(ent->client->force_countdown == 1)
	{
		if(ent->client->current_force == DFORCE_DARKNESS)
		{
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchflpA;
				ent->client->anim_end = FRAME_crouchflpB;
			}
			else
			{
				ent->s.frame = FRAME_forcelpA;
				ent->client->anim_end = FRAME_forcelpB;
			}
		}
		else
		{
			ent->client->current_force = DFORCE_DARKNESS;
			if(ent->client->anim_duck)
			{
				ent->s.frame = FRAME_crouchfinA;
				ent->client->anim_end = FRAME_crouchfinB;
			}
			else
			{
				ent->s.frame = FRAME_forceinA;
				ent->client->anim_end = FRAME_forceinB;
			}
			ForceAnimCD(ent);
		}
	}

	if(deathmatch->value)
	{
		if (ent->client->forcesound == 0)
		{
			sound_delay(ent, 2, 0);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/walldark.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->client->forcesound == 2)
		{
			sound_delay(ent, 2, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/walldark.wav"), 1, ATTN_NORM, 0);
		}
		for(i=0; i<num_players; i++)
		{
			if(infront(ent, players[i]))
			{
				yourlevel = players[i]->client->pers.force.top_level;
				if(yourlevel <= mylevel)
				{
					players[i]->forceflags[AFF_DARK] |= FFD_DARKNESS;
				}
			}
		}
	}
}

void force_taint (edict_t *ent, int last)
{
	int index;

	index = Force_constant_active(ent, DFORCE_TAINT);

	if(ent->client->anim_duck)
	{
		ent->client->current_force = DFORCE_TAINT;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = DFORCE_TAINT;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	if(index == 255)
	{
		if(ent->client->pers.active_constants == 6)
			return;
		ent->client->pers.force_table[ent->client->pers.active_constants] = DFORCE_TAINT;
		ent->client->pers.active_constants++;
		if (ent->client->forcesound == 0)
		{
			sound_delay(ent, 1, 0);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/taint.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->client->forcesound == 2)
		{
			sound_delay(ent, 1, 2);
			gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/taint.wav"), 1, ATTN_NORM, 0);
		}
	}
	else
	{
		ent->client->pers.force_table[index] = 255;
		if(ent->client->pers.active_constants != 0)
			ent->client->pers.active_constants--;
	}
}

vec3_t inferno[] =
{
	0,	0,	0,
	0,	64,	0,
	32,	96,	0,
	64,	64,	0,
	48,	48,	0,
	32,	32,	0,
	10,	10,	 0,
};

void Inferno_Effect_Touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(other == self->owner)
		return;

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, 0);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void Inferno_Effect_Think (edict_t *self)
{
	vec3_t	forward, right;
	trace_t	tr;
	
	self->s.frame++;
	self->nextthink += 0.2;

	VectorCopy(self->s.origin, self->s.old_origin);

	AngleVectors (self->move_angles, forward, right, NULL);
	G_ProjectSource (self->owner->s.origin, inferno[self->s.frame], forward, right, self->s.origin);

	tr = gi.trace(self->s.origin, NULL, NULL, self->owner->s.origin, self, MASK_SHOT);

	if(tr.ent->takedamage && (tr.ent != self->owner))
		Inferno_Effect_Touch (self, tr.ent, NULL, NULL);

	if(gi.pointcontents(self->s.origin) == CONTENTS_SOLID)
		Inferno_Effect_Touch (self, tr.ent, &tr.plane, tr.surface);

	if(self->s.frame == 6)
		G_FreeEdict(self);
}

void Inferno_Effect_Spawn (edict_t *ent, vec3_t dir)
{
	edict_t *fire;
	vec3_t	forward, right;

	fire = G_Spawn();

	fire->owner = ent;	

	AngleVectors (dir, forward, right, NULL);
	G_ProjectSource (ent->s.origin, inferno[0], forward, right, fire->s.origin);
	VectorCopy(ent->s.origin, fire->s.old_origin);
	VectorCopy(dir, fire->move_angles);

	fire->dmg = ent->client->pers.force.power_values[DFORCE_INFERNO];
	fire->s.modelindex = gi.modelindex("monsters/trooper/tris.md2");
	VectorSet(fire->mins, -16, -16, -16);
	VectorSet(fire->maxs, 16, 16, 16);
	fire->s.frame = 0;
	fire->solid = SOLID_TRIGGER;
	fire->movetype = MOVETYPE_NONE;

	fire->touch = Inferno_Effect_Touch;
	fire->think = Inferno_Effect_Think;
	fire->nextthink = level.time + 0.1;

	gi.linkentity(fire);
}

void Inferno_Effect (edict_t *ent)
{
	vec3_t	angle;

	angle[1] = 0;
	angle[0] = 0;
	angle[2] = 0;
	Inferno_Effect_Spawn(ent, angle);

	angle[1] = 90;
	angle[0] = 0;
	Inferno_Effect_Spawn(ent, angle);

	angle[1] = 180;
	angle[0] = 0;
	Inferno_Effect_Spawn(ent, angle);

	angle[1] = 270;
	angle[0] = 0;
	Inferno_Effect_Spawn(ent, angle);
}

void force_inferno (edict_t *ent, int last)
{
	Inferno_Effect(ent);

	if(ent->client->anim_duck)
	{
		ent->client->current_force = DFORCE_INFERNO;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = DFORCE_INFERNO;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	if (ent->client->forcesound == 0)
	{
		sound_delay(ent, 1, 0);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/inferno.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->client->forcesound == 2)
	{
		sound_delay(ent, 1, 2);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/inferno.wav"), 1, ATTN_NORM, 0);
	}
}

void force_rage (edict_t *ent, int last)
{
	if(ent->health <= 50)
	{
		return;
	}

	if(ent->client->anim_duck)
	{
		ent->client->current_force = DFORCE_RAGE;
		ent->s.frame = FRAME_crouchfinA;
		ent->client->anim_end = FRAME_crouchfinB;
	}
	else
	{
		ent->client->current_force = DFORCE_RAGE;
		ent->s.frame = FRAME_forceinA;
		ent->client->anim_end = FRAME_forceinB;
	}

	ent->client->pers.force.pool = 100+(ent->client->pers.force.power_values[DFORCE_RAGE]/10);
	ent->health -= 50;
//	T_Damage (ent, ent, ent, ent->s.angles, ent->s.origin, vec3_origin, 50, 0, DAMAGE_NO_ARMOR, MOD_HIT);
	if (ent->client->forcesound == 0)
	{
		sound_delay(ent, 2.377, 0);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/rage.wav"), 1, ATTN_NORM, 0);
	}
	else if (ent->client->forcesound == 2)
	{
		sound_delay(ent, 2.377, 2);
		gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/rage.wav"), 1, ATTN_NORM, 0);
	}
}


/*=========================================================

	WEAPON MENU STUFF

=========================================================*/

gitem_t *weapon_menu_use (edict_t *ent)
{
	int num;
	gitem_t *it;

	num = ent->client->weapon_menu_choice[ent->client->weapon_menu_weap_sel-1];

	switch(num)
	{
		case 0:
			it = FindItem ("Blaster");
			break;
		case 1:
			it = FindItem ("Trooper_Rifle");
			break;
		case 2:
			it = FindItem ("Repeater");
			break;
		case 3:
			it = FindItem ("Bowcaster");
			break;
		case 4:
			it = FindItem ("Blaster");
			break;
		case 5:
			it = FindItem ("Wrist_Rocket");
			break;
		case 6:
			it = FindItem ("Rocket_Launcher");
			break;
		case 7:
			it = FindItem ("Disruptor");
			break;
		case 8:
			it = FindItem ("Night_Stinger");
			break;
		case 9:
			it = FindItem ("Beam_Tube");
			break;
		case 10:
			it = FindItem ("Thermals");
			break;
		case 11:
			it = FindItem ("Lightsaber");
			break;
		case 12:
			it = FindItem ("Blaster");
			break;
		default:
			it = FindItem ("Blaster");
			break;
	}
	return it;
}

// Looks for which weapons the player is currently holding, so we know what to display, and what not.

//RipVTide check out
int Check_Weapon (edict_t *ent, char *s)
{
	gitem_t *it;
	int ammoindex, index;

	it = FindItem (s);
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		return 0;
	}
	if(!strcmp("Lightsaber", s))
		return 1;

	it = FindItem (it->ammo);
	ammoindex = ITEM_INDEX(it);
	if(!ent->client->pers.inventory[ammoindex] && !ent->client->pers.clipammo[index])
	{
		return 0;
	}
	return 1;
}

void setup_weap_menu (edict_t *ent)
{
	ent->client->weapon_sub_num[0] = 0;
	ent->client->weapon_sub_num[1] = 0;
	ent->client->weapon_sub_num[2] = 0;
	ent->client->weapon_sub_num[3] = 0;

//BLASTER PISTOL
	if(Check_Weapon(ent, "Blaster"))
	{
		ent->client->weapon_list[0] = 1;
		ent->client->weapon_sub_num[0]++;
	}
	else
		ent->client->weapon_list[0] = 0;

//BLASTER RIFLE
	if(Check_Weapon(ent, "Trooper_Rifle"))
	{
		ent->client->weapon_list[1] = 1;
		ent->client->weapon_sub_num[0]++;
	}
	else
		ent->client->weapon_list[1] = 0;

//REPEATER
	if(Check_Weapon(ent, "Repeater"))
	{
		ent->client->weapon_list[2] = 1;
		ent->client->weapon_sub_num[0]++;
	}
	else
		ent->client->weapon_list[2] = 0;

//BOWCASTER
	if(Check_Weapon(ent, "Bowcaster"))
	{
		ent->client->weapon_list[3] = 1;
		ent->client->weapon_sub_num[0]++;
	}
	else
		ent->client->weapon_list[3] = 0;

//THERMAL DET LAUNCHER
//	if(Check_Weapon(ent, "Blaster"))
//	{
//		ent->client->weapon_list[4] = 1;
//		ent->client->weapon_sub_num[1]++;
//	}
//	else
		ent->client->weapon_list[4] = 0;

//WRIST ROCKET
	if(Check_Weapon(ent, "Wrist_Rocket"))
	{
		ent->client->weapon_list[5] = 1;
		ent->client->weapon_sub_num[1]++;
	}
	else
		ent->client->weapon_list[5] = 0;

//MISSILE TUBE
	if(Check_Weapon(ent, "Rocket_Launcher"))
	{
		ent->client->weapon_list[6] = 1;
		ent->client->weapon_sub_num[1]++;
	}
	else
		ent->client->weapon_list[6] = 0;

//DISRUPTOR
	if(Check_Weapon(ent, "Disruptor"))
	{
		ent->client->weapon_list[7] = 1;
		ent->client->weapon_sub_num[2]++;
	}
	else
		ent->client->weapon_list[7] = 0;

//NIGHTSTINGER
	if(Check_Weapon(ent, "Night_Stinger"))
	{

		ent->client->weapon_list[8] = 1;
		ent->client->weapon_sub_num[2]++;
	}
	else
		ent->client->weapon_list[8] = 0;

//BEAM TUBE
	if(Check_Weapon(ent, "Beam_Tube"))
	{
		ent->client->weapon_list[9] = 1;
		ent->client->weapon_sub_num[2]++;
	}
	else
		ent->client->weapon_list[9] = 0;

//THERMAL DETS
	if(Check_Weapon(ent, "Thermals"))
	{
		ent->client->weapon_list[10] = 1;
		ent->client->weapon_sub_num[3]++;
	}
	else
		ent->client->weapon_list[10] = 0;

//CHARGE TYPE 1
	if(Check_Weapon(ent, "Lightsaber"))
	{
		ent->client->weapon_list[11] = 1;
		ent->client->weapon_sub_num[3]++;
	}
	else
		ent->client->weapon_list[11] = 0;

//CHARGE TYPE 2
//	if(Check_Weapon(ent, "Blaster"))
//	{
//		ent->client->weapon_list[12] = 1;
//		ent->client->weapon_sub_num[3]++;
//	}
//	else
		ent->client->weapon_list[12] = 0;
}

void set_sub_menu1_weap (edict_t *ent)
{
	int i;
	int done[4];

	done[0] = 255;
	done[1] = 255;
	done[2] = 255;
	done[3] = 255;

	if(ent->client->weapon_menu_sub_sel == 1)
	{
		ent->client->ps.stats[STAT_FMENU] = gi.imageindex ("weapons/menu1");

		for(i=0; i<5; i++)
		{
			if(ent->client->weapon_list[0] && done[0] == 255)
			{
				done[0] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_pist2");
				ent->client->weapon_menu_choice[i] = 0;
				continue;
			}
			if(ent->client->weapon_list[1] && done[1] == 255)
			{
				done[1] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_trpr2");
				ent->client->weapon_menu_choice[i] = 1;
				continue;
			}
			if(ent->client->weapon_list[2] && done[2] == 255)
			{
				done[2] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_rapid2");
				ent->client->weapon_menu_choice[i] = 2;
				continue;
			}
			if(ent->client->weapon_list[3] && done[3] == 255)
			{
				done[3] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_arrw2");
				ent->client->weapon_menu_choice[i] = 3;
				continue;
			}
			ent->client->ps.stats[STAT_FPOW1+i] = 0;
		}
		if(ent->client->ps.stats[STAT_FPOW1] == 0)
			return;
		ent->client->ps.stats[19 + ent->client->weapon_menu_weap_sel]--;
		return;
	}
}

void set_sub_menu2_weap (edict_t *ent)
{
	int i;
	int done[3];

	done[0] = 255;
	done[1] = 255;
	done[2] = 255;

	if(ent->client->weapon_menu_sub_sel == 2)
	{
		ent->client->ps.stats[STAT_FMENU] = gi.imageindex ("weapons/menu2");

		for(i=0; i<5; i++)
		{
			if(ent->client->weapon_list[4] && done[0] == 255)
			{
				done[0] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_tdlauncher2");
				ent->client->weapon_menu_choice[i] = 4;
				continue;
			}
			if(ent->client->weapon_list[5] && done[1] == 255)
			{
				done[1] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_wrstrkt2");
				ent->client->weapon_menu_choice[i] = 5;
				continue;
			}
			if(ent->client->weapon_list[6] && done[2] == 255)
			{
				done[2] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_mtube2");
				ent->client->weapon_menu_choice[i] = 6;
				continue;
			}
			ent->client->ps.stats[STAT_FPOW1+i] = 0;
		}
		if(ent->client->ps.stats[STAT_FPOW1] == 0)
			return;
		ent->client->ps.stats[19 + ent->client->weapon_menu_weap_sel]--;
		return;
	}
}

void set_sub_menu3_weap (edict_t *ent)
{
	int i;
	int done[3];

	done[0] = 255;
	done[1] = 255;
	done[2] = 255;

	if(ent->client->weapon_menu_sub_sel == 3)
	{
		ent->client->ps.stats[STAT_FMENU] = gi.imageindex ("weapons/menu3");

		for(i=0; i<5; i++)
		{
			if(ent->client->weapon_list[7] && done[0] == 255)
			{
				done[0] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_dis2");
				ent->client->weapon_menu_choice[i] = 7;
				continue;
			}
			if(ent->client->weapon_list[8] && done[1] == 255)

			{
				done[1] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_nstg2");
				ent->client->weapon_menu_choice[i] = 8;
				continue;
			}
			if(ent->client->weapon_list[9] && done[2] == 255)
			{
				done[2] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_beam2");
				ent->client->weapon_menu_choice[i] = 9;
				continue;
			}
			ent->client->ps.stats[STAT_FPOW1+i] = 0;
		}
		if(ent->client->ps.stats[STAT_FPOW1] == 0)
			return;
		ent->client->ps.stats[19 + ent->client->weapon_menu_weap_sel]--;
		return;
	}
}

void set_sub_menu4_weap (edict_t *ent)
{
	int i;
	int done[3];

	done[0] = 255;
	done[1] = 255;
	done[2] = 255;

	if(ent->client->weapon_menu_sub_sel == 4)
	{
		ent->client->ps.stats[STAT_FMENU] = gi.imageindex ("weapons/menu4");

		for(i=0; i<5; i++)
		{
			if(ent->client->weapon_list[10] && done[0] == 255)
			{
				done[0] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_thrm2");
				ent->client->weapon_menu_choice[i] = 10;
				continue;
			}
			if(ent->client->weapon_list[11] && done[1] == 255)
			{
				done[1] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_sabr2");
				ent->client->weapon_menu_choice[i] = 11;
				continue;
			}
			if(ent->client->weapon_list[12] && done[2] == 255)
			{
				done[2] = i;
				ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex ("weapons/wm_thrm2");
				ent->client->weapon_menu_choice[i] = 12;
				continue;
			}
			ent->client->ps.stats[STAT_FPOW1+i] = 0;
		}
		if(ent->client->ps.stats[STAT_FPOW1] == 0)
			return;
		ent->client->ps.stats[19 + ent->client->weapon_menu_weap_sel]--;
		return;
	}
}


//************************************************
//
//  FORCE POWERS
//
//************************************************

void setup_force_menu (edict_t *ent)
{
	int i;

	for(i=0; i<NUM_SUBGROUPS+1; i++)
	{
		ent->client->force_sub_num[i] = 0;
	}

	for(i=0; i<NUM_POWERS+1; i++)
	{
		if(ent->client->pers.force.powers[i] == 1)
		{
			if(Force_Power_Available (ent, i) == 0)
			{
//				gi.dprintf("%i not available\n", i);
				continue;
			}
			ent->client->force_sub_list[powerlist[i].subgroup][ent->client->force_sub_num[powerlist[i].subgroup]] = i;
			ent->client->force_sub_num[powerlist[i].subgroup]++;
		}
	}
}

void select_menu_power (edict_t *ent)
{
	int i, p;

	i=ent->client->force_menu_pow_sel;
	p = ent->client->force_sub_list[ent->client->force_menu_sub_sel][i];
	ent->client->pers.force_power = &powerlist[p];
	ent->client->pers.current_power = p;
}

void set_sub_menu1 (edict_t *ent)
{
	int i, p;
	unsigned short num;

	if(ent->client->force_menu_sub_sel != SUBGROUP_NEUTRAL)
	{
		return;
	}

	ent->client->ps.stats[STAT_FMENU] = gi.imageindex ("force/menu1");

	num = ent->client->force_sub_num[SUBGROUP_NEUTRAL];

	if(num == 0)
		return;

	for(i=0; i<num; i++)
	{
		p = ent->client->force_sub_list[SUBGROUP_NEUTRAL][i];
		ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon2);
	}
	i=ent->client->force_menu_pow_sel;
	p = ent->client->force_sub_list[SUBGROUP_NEUTRAL][i];
	ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon3);
}

void set_sub_menu2 (edict_t *ent)
{
	int i, p;
	unsigned short num;

	if(ent->client->force_menu_sub_sel != SUBGROUP_NEUTRAL2)
	{
		return;
	}

	ent->client->ps.stats[STAT_FMENU] = gi.imageindex ("force/menu2");

	num = ent->client->force_sub_num[SUBGROUP_NEUTRAL2];
	
	if(num == 0)
		return;
	
	for(i=0; i<num; i++)
	{
		p = ent->client->force_sub_list[SUBGROUP_NEUTRAL2][i];
		ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon2);
	}
	i=ent->client->force_menu_pow_sel;
	p = ent->client->force_sub_list[SUBGROUP_NEUTRAL2][i];

	ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon3);
}

void set_sub_menu3 (edict_t *ent)
{
	int i, p;
	unsigned short num;

	if(ent->client->force_menu_sub_sel != SUBGROUP_SPIRIT)
	{
		return;
	}

	ent->client->ps.stats[STAT_FMENU] = gi.imageindex ("force/menu3");

	num = ent->client->force_sub_num[SUBGROUP_SPIRIT];
	
	if(num == 0)
		return;

	for(i=0; i<num; i++)
	{
		p = ent->client->force_sub_list[SUBGROUP_SPIRIT][i];
		ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon2);
	}
	i=ent->client->force_menu_pow_sel;
	p = ent->client->force_sub_list[SUBGROUP_SPIRIT][i];
	ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon3);
}

void set_sub_menu4 (edict_t *ent)
{
	int i, p;
	unsigned short num;

	if(ent->client->force_menu_sub_sel != SUBGROUP_ENERGY)
	{
		return;
	}

	ent->client->ps.stats[STAT_FMENU] = gi.imageindex ("force/menu4");

	num = ent->client->force_sub_num[SUBGROUP_ENERGY];

	if(num == 0)
		return;

	for(i=0; i<num; i++)
	{
		p = ent->client->force_sub_list[SUBGROUP_ENERGY][i];
		ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon2);
	}
	i=ent->client->force_menu_pow_sel;
	p = ent->client->force_sub_list[SUBGROUP_ENERGY][i];
	ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon3);
}

void set_sub_menu5 (edict_t *ent)
{
	int i, p;
	unsigned short num;

	if(ent->client->force_menu_sub_sel != SUBGROUP_TELEKINESIS)
	{
		return;
	}

	ent->client->ps.stats[STAT_FMENU] = gi.imageindex ("force/menu5");

	num = ent->client->force_sub_num[SUBGROUP_TELEKINESIS];
	
	if(num == 0)
		return;

	for(i=0; i<num; i++)
	{
		p = ent->client->force_sub_list[SUBGROUP_TELEKINESIS][i];
		ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon2);
	}
	i=ent->client->force_menu_pow_sel;
	p = ent->client->force_sub_list[SUBGROUP_TELEKINESIS][i];
	ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon3);
}

void set_sub_menu6 (edict_t *ent)
{
	int i, p;
	unsigned short num;

	if(ent->client->force_menu_sub_sel != SUBGROUP_FREE)
	{
		return;
	}

	ent->client->ps.stats[STAT_FMENU] = gi.imageindex ("force/menu6");

	num = ent->client->force_sub_num[SUBGROUP_FREE];
	
	if(num == 0)
		return;

	for(i=0; i<num; i++)
	{
		p = ent->client->force_sub_list[SUBGROUP_FREE][i];
		ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon2);
	}
	i=ent->client->force_menu_pow_sel;
	p = ent->client->force_sub_list[SUBGROUP_FREE][i];
	ent->client->ps.stats[STAT_FPOW1+i] = gi.imageindex (powerlist[p].icon3);
}

//RipVTide

void sound_delay(edict_t *ent, float delay, int setto)
{
	edict_t		*t;

	t = G_Spawn();
	t->nextthink = level.time + delay;
	t->think = sound_reinit;
	t->health = setto;
	t->enemy = ent;

	ent->client->forcesound = 1;
}

void sound_reinit(edict_t *ent)
{
	ent->enemy->client->forcesound = ent->health;
	G_FreeEdict (ent);
}

void ForceAnimCD(edict_t *ent)
{
	edict_t		*t;

	ent->client->force_countdown = 5;

	t = G_Spawn();
	t->nextthink = level.time + 0.01;
	t->think = ForceAnim;
	t->enemy = ent;
}

void ForceAnim(edict_t *ent)
{
	if(ent->enemy->client->force_countdown < 1)
	{
		ent->enemy->client->current_force = 0;
		if(ent->enemy->client->current_force == DFORCE_LIGHTNING)
		G_FreeEdict (ent);
	}
	else
	{
		ent->nextthink = level.time + 0.01;
		ent->enemy->client->force_countdown--;
	}
}

void makelightning(edict_t *ent)
{
	edict_t		*t;

	ent->client->light_countdown = 3;

	ent->client->forcesound = 2;

	t = G_Spawn();
	t->nextthink = level.time + 0.01;
	t->think = l_count;
	t->enemy = ent;
}

void l_count(edict_t *ent)
{
	if(ent->enemy->client->light_countdown < 1)
	{
		ent->enemy->client->forcesound = 0;
		G_FreeEdict (ent);
	}
	else
	{
		ent->nextthink = level.time + 0.01;
		ent->enemy->client->light_countdown--;
		ent->enemy->client->forcesound = 2;
		ent->enemy->client->light_sound--;
	}
}
