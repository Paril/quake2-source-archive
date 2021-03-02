#include "g_local.h"

qboolean CheckBan (cvar_t *cvar, int value)
{
	if (value == SHOTGUNBAN_CO2CANNON && Q_stricmp(cvar->string, "all") == 0)
		return true;
	if (cvar == blaster_ban && blaster_ban->string && Q_stricmp(blaster_ban->string, "all") == 0)
		return true;

	if (cvar == feature_ban && feature_ban->string && Q_stricmp(feature_ban->string, "all") == 0)
		return true;

	if ((int)cvar->value & value)
		return true;
	else
		return false;
}
