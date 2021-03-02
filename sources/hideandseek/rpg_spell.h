enum ESpellType
{
	SPELL_NORMAL,	// Normal, command-based spell that casts to fires through a single function
	SPELL_PASSIVE	// Passive, stays active throughout a time period. Can be activated or always on.
};

#define MAX_SPELL_LEVELS 10

class CSpell
{
public:
	//ESpellList
	unsigned int	spellID;
	PlayerClass		pClass; // Class this spell belongs to
	ESpellType		sType;
	int				cost; // in numbers

	bool			command;
	char			*commandName;

	float			nextThink[MAX_SPELL_LEVELS]; // The next time the spell will think.
	float			args[MAX_SPELL_LEVELS][20]; // 20 private args for spells
	void			(*function) (edict_t *ent);
};