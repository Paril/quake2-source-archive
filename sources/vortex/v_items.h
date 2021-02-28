#ifndef ITEMS_H
#define ITEMS_H

#define MAX_VRXITEMS			11
#define MAX_VRXITEMMODS			6

#define TYPE_NONE			0
#define TYPE_WEAPON			1
#define TYPE_ABILITY		2
#define TYPE_HIDDEN			4

typedef struct imodifier_s
{
	int type;		//see above
	int index;		//Tells us which weapon or ability it is..
	int value;
	int set;		//how many set items needed to see this modifier
}imodifier_t;

#define ITEM_NONE			0
#define ITEM_WEAPON			1
#define ITEM_ABILITY		2
#define ITEM_CLASSRUNE		4
#define ITEM_UNIQUE			8
#define ITEM_SET			16
#define ITEM_COMBO			32
#define ITEM_POTION			64
#define ITEM_ANTIDOTE		128
#define ITEM_GRAVBOOTS		256
#define ITEM_FIRE_RESIST	512
#define ITEM_AUTO_TBALL		1024

#define RUNE_POINTS_PER_LEVEL	1.5	//A player can equip the item its itemLevel / this value is less than their level. ex: A lvl 10 player can equip a 25 pt rune.

typedef struct item_s
{
	int				itemtype;		//See defined item types
	int				itemLevel;		//lvl required to equip the item
	int				quantity;		//1 for runes, > 1 for other items
	int				untradeable;	//Can this item be traded to someone else?
    char			id[16];			//item's id string
	char			name[24];		//custom name for the item
	int				numMods;		//number of modifiers
	int				setCode;		//Used for set items
	int				classNum;		//Used for class-specific runes
	imodifier_t		modifiers[MAX_VRXITEMMODS];	//Up to 6 seperate mods
}item_t;

#define RUNE_SPAWN_BASE			0.01	// (0.01 = 1% per frag) base chance of a rune spawning, before level modifiers
#define RUNE_WEAPON_MAXVALUE	10		// maximum modifier for weapon runes
#define RUNE_ABILITY_MAXVALUE	10		// maximum modifier for ability runes
#define RUNE_COST_BASE			2500
#define RUNE_COST_ADDON			750

#define CHANCE_NORM				750		//50%	(1 in 1.5)
#define CHANCE_COMBO			250		//27%	(1 in 3.7)
#define CHANCE_CLASS			100		//2.8%	(1 in 40)
#define CHANCE_UNIQUE			35		//0.2%	(1 in 500)

#endif