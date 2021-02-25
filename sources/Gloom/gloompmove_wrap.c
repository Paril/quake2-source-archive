#include "g_local.h"
#include <math.h>
#define PMPrint(msg) gi.dprintf("SVPM: %s", msg);
#define PMPrintf(fmt, ...) gi.dprintf("SVPM: " fmt, __VA_ARGS__ );
#define GPMIfClient(...) 0
#include "gloompmove.c"