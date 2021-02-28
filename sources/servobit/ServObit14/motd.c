// motd.c, for ServObit 1.3
// by SteQve (steqve@shore.net), 1/10/98.

#include "g_local.h"

// The only function that is "published" is DisplayTimedMessage.
// The other functions just manage the edict that gets created to
// support the timed message.
void DeleteTimedMessage (edict_t *mess)
{
	free(mess->message);
	G_FreeEdict(mess);
}

void SendTimedMessage (edict_t *mess)
{
	//gi.dprintf("send timed message\n");
	if (mess->last_move_time <= level.time)
	{
		// Destroy message in next frame
		mess->think = DeleteTimedMessage;
		mess->nextthink = level.time + FRAMETIME;
	}
	else {
		// Send message every 2 seconds, which seems to be enough to
		// keep it displaying constantly
		gi.centerprintf(mess->owner, mess->message);
		mess->nextthink = level.time + 2;
	}
}

edict_t *GetTimedMessage (edict_t *ent)
{
	int i;

	for(i=0; i<game.maxentities; i++)
	{
		if (g_edicts[i].classname != NULL)
			if (strcmp(g_edicts[i].classname, "TimedMessage") == 0)
			{
				return(g_edicts+i);
			}
	}
	return(NULL);
}

void StopTimedMessage (edict_t *ent)
{
	edict_t *mess = GetTimedMessage(ent);
	if (mess != NULL)
		DeleteTimedMessage(mess);
}

void DisplayTimedMessage(edict_t *ent, char *message, int seconds)
{

	edict_t *mess = G_Spawn();

	// message is freed in DeleteTimedMessage
	mess->classname = "TimedMessage";
	mess->message = strdup(message);
	mess->owner = ent;
	mess->think = SendTimedMessage;
	mess->nextthink = level.time + 1;
	// This keeps track of when the motd was started.
	mess->last_move_time = level.time + seconds;
	gi.linkentity(mess);
}
