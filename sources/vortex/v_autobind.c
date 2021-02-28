#include "g_local.h"

#define S(x) G_StuffPlayerCmds(ent,x)

// basic list
const char* StuffList[] = 
{
	"alias +thrust thrust on\nalias -thrust thrust off\n",
	"alias +manacharge meditate\nalias -manacharge meditate\n",
	"alias +hook hook\nalias -hook unhook\n",
	"alias +superspeed sspeed\nalias -superspeed nosspeed on\n",
	"alias +sprint sprinton\nalias -sprint sprintoff\n",
	"alias +shield shieldon\nalias -shield shieldoff\n",
	"alias +lockon lockon_on\nalias -lockon lockon_off\n",
	"alias +beam beam_on\nalias -beam beam_off\n",
	"alias +jetpack thrust on\nalias -jetpack thrust off\n",
};

const char* CommonAutobind[] = 
{
	"bind uparrow invprev\nbind downarrow invnext\n",
	"bind enter invuse\n",
	"bind z drop tech\nbind f flashlight\n",
	"bind ctrl use tball self\n"
};

void V_AutoStuff(edict_t* ent)
{
	int i;

	for (i = 0; i < sizeof(StuffList) / sizeof(char*); i++)
	{
		S(StuffList[i]);
	}
}