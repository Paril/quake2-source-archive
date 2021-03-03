
#define	NFORCE_PUSH		1	//NEUTRAL
#define	NFORCE_PULL		2	//NEUTRAL
#define	NFORCE_LEVITATE	3	//NEUTRAL2
#define	NFORCE_NEGATE		4	//NEUTRAL
#define	NFORCE_JUMP		5	//NEUTRAL2
#define	NFORCE_SPEED		6	//NEUTRAL2

#define	LFORCE_HEAL		7	//SPIRIT
#define	LFORCE_WALL		8	//SPIRIT
#define	LFORCE_SHIELD		9	//ENERGY
#define	LFORCE_INVISIBILITY	10	//ENERGY
#define	LFORCE_WINDS		11	//TELEKINESIS
#define	LFORCE_REFLECT	12	//ENERGY
#define	LFORCE_SCOUT		13	//SPIRIT
#define	LFORCE_BIND		14	//TELEKINESIS

#define	DFORCE_HEAL		15	//SPIRIT
#define	DFORCE_LIGHTNING	16	//ENERGY
#define	DFORCE_CHOKE		17	//TELEKINESIS
#define	DFORCE_ABSORB		18	//TELEKINESIS
#define	DFORCE_DARKNESS	19	//SPIRIT
#define	DFORCE_TAINT		20	//ENERGY
#define	DFORCE_INFERNO	21	//TELEKINESIS
#define	DFORCE_RAGE		22	//SPIRIT

//force flags
#define	FFN_PUSH		1	//NEUTRAL
#define	FFN_PULL		2	//NEUTRAL
#define	FFN_LEVITATE		4	//NEUTRAL2
#define	FFN_NEGATE		8	//NEUTRAL2
#define	FFN_JUMP		16	//NEUTRAL
#define	FFN_SABER_THROW	32	//NEUTRAL2
#define	FFN_SPEED		64	//NEUTRAL

#define	FFL_HEAL		1	//SPIRIT
#define	FFL_WALL		2	//SPIRIT
#define	FFL_SHIELD		4	//ENERGY
#define	FFL_INVISIBILITY	8	//ENERGY
#define	FFL_WINDS		16	//TELEKINESIS
#define	FFL_REFLECT		32	//ENERGY
#define	FFL_SCOUT		64	//SPIRIT
#define	FFL_BIND		128	//TELEKINESIS

#define	FFD_HEAL		1	//SPIRIT
#define	FFD_LIGHTNING		2	//ENERGY
#define	FFD_CHOKE		4	//TELEKINESIS
#define	FFD_ABSORB		8	//TELEKINESIS
#define	FFD_DARKNESS		16	//SPIRIT
#define	FFD_TAINT		32	//ENERGY
#define	FFD_INFERNO		64	//TELEKINESIS
#define	FFD_RAGE		128	//SPIRIT

#define	AFF_NEUTRAL		0
#define	AFF_LIGHT		1
#define	AFF_DARK		2

//#define	SUBGROUP_TELEKINETIC		1
//#define	SUBGROUP_NEUTRAL		2
//#define	SUBGROUP_LIGHT		3
//#define	SUBGROUP_LIGHT2		4
//#define	SUBGROUP_DARK			5
//#define	SUBGROUP_DARK2		6

#define	SUBGROUP_NEUTRAL		1
#define	SUBGROUP_NEUTRAL2		2
#define	SUBGROUP_SPIRIT		3
#define	SUBGROUP_ENERGY		4
#define	SUBGROUP_TELEKINESIS		5
#define	SUBGROUP_FREE			6

typedef struct gforce_s
{
	void			(*think)(struct edict_s *ent, int end);
	char			*force_sound;
	char			*icon;
	char			*icon2;
	char			*icon3;
	char			*name;
	float			start_level;
	float			cost;
	short			lightdark;
	unsigned short	constant;
	unsigned short	subgroup;
	int			skillreq;
} gforce_t;

#define NUM_POWERS		22
#define NUM_SUBGROUPS	5

extern gforce_t	powerlist[];
