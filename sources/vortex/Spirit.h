#ifndef SPIRIT_H
#define SPIRIT_H

//************************************************************
//			Yin spirit
//************************************************************
//Healing amount
#define YIN_HEAL_BASE				0
#define YIN_HEAL_MULT				5
#define YIN_AMMO_BASE				0.0
#define YIN_AMMO_MULT				0.075	//v4.0 (was 0.1)
//Refire		(level 1 = 2.5s, level 10 = 1.0s, level 15 = 0.75s, level 20 = 0.6s, etc..)
#define YIN_ATTACK_DELAY_BASE		3.0
#define YIN_ATTACK_DELAY_MULT		0.2

//************************************************************
//			Yang spirit
//************************************************************
//Damage		(level 1= 25, level 10 = 50, level 15 = 60, level 20 = 70, etc..)
#define YANG_DAMAGE_BASE			25
#define YANG_DAMAGE_MULT			10
//Refire		(level 1 = 2.5s, level 10 = 1.0s, level 15 = 0.75s, level 20 = 0.6s, etc..)
#define YANG_ATTACK_DELAY_BASE		3.0
#define YANG_ATTACK_DELAY_MULT		0.2
#define YANG_ATTACK_DELAY_MIN		1.0

//************************************************************
//			General spirit stuff
//************************************************************
//Physics
#define SPIRIT_DISTANCE				35.0
#define SPIRIT_SIGHT_RADIUS			1024
//Spirit spell cost
#define SPIRIT_COST					50

void cmd_Spirit(edict_t *ent, int type);

#endif