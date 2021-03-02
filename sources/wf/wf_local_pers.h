//WF - Additional persistant data
    int			player_class;
    int			next_player_class;
	int			scanner_active;
	int			grenade_type;
	int			grenade_num;
	qboolean	HasVoted;	//True if the person voted for a map
	int			active_grenades[GRENADE_TYPE_COUNT + 1];
	int			active_special[SPECIAL_COUNT + 1];

	//Homing state
    qboolean	homing_state;   // are homing missiles activated

	//For sniper
	int			laseron;
	int			autozoom;
	int			fastaim;

	int			feign;

#define SPAM_NOTEAMSOUND	 1	//Turns off wfplay_team sounds
#define SPAM_NOTEAMSOUNDTEXT 2	//Turns off wfplay_team sound text
#define SPAM_NOTEAMMSG		 4	//Turns off player messages
	int			nospam_level;

	//Auto execut class specific configs?
	qboolean	autoconfig;

	//Friend settings
#define MAX_FRIENDS	4
	edict_t		*friend_ent[MAX_FRIENDS];
	int			hasfriends;
	int	i_am_a_bot;		//set if this is a bot client

