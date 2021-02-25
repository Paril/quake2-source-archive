#include "g_local.h"
#include "stuff.h"
void stuffcmd(edict_t *e, char *s) 
{
gi.WriteByte (11);
gi.WriteString (s);
gi.unicast (e, true);
}