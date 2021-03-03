#include "g_local.h"
#include "weaponf.h"

char *weapons[] = {
	"weapon_hook",				//    1 (can't be inhibited)
	"weapon_blaster",			//    2 (can't be inhibited)
	"weapon_shotgun",			//    4
	"weapon_supershotgun",		//    8
	"weapon_machinegun",		//   16
	"weapon_chaingun",			//   32
	"weapon_grenadelauncher",	//   64
	"weapon_rocketlauncher",	//  128
	"weapon_hyperblaster",		//  256
	"weapon_railgun",			//  512
	"weapon_bfg"				// 1024
};

qboolean weapon_is_inhibited(char *classname)
{
	int i;
	for (i = 0; i < (sizeof(weapons)/sizeof(weapons[0])); i++)
	{
		if (Q_stricmp(classname, weapons[i]) == 0)
		{
			return (((int)weaponban->value & (1 << i)) > 0);
		}
	}
	return (false);
}