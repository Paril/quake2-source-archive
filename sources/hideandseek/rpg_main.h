enum EPlayerClass
{
	CLASS_NONE = 0,
	CLASS_WARRIOR,

	CLASS_ALL
};

class PlayerClass
{
public:
	char			*className;
	EPlayerClass	classID;
};

extern PlayerClass classList[CLASS_ALL]; 