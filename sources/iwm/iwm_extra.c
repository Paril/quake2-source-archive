#include "g_local.h"

// This file has helper functions and misc stuff.
// Thanks Mr. Grim
/*
================
Audio Code
================
*/

#define svc_sound 9

#define MASK_VOLUME 			1
#define MASK_ATTENUATION 		2
#define MASK_POSITION 			4
#define MASK_ENTITY_CHANNEL             8
#define MASK_TIMEOFS 			16	

/*
================
unicastSound - Send a sound to one player (uni, duh).
================
*/
void unicastSound(edict_t *player, int sound_index, float volume)
{
	int mask = MASK_ENTITY_CHANNEL;
	
	if (player->is_bot)
		return;

	if (volume != 1.0)
		mask |= MASK_VOLUME;
	
	gi.WriteByte(svc_sound);
	gi.WriteByte((byte)mask);
	gi.WriteByte((byte)sound_index);
	if (mask & MASK_VOLUME)
		gi.WriteByte((byte)(volume * 255));
	
	gi.WriteShort(((player - g_edicts - 1) << 3) + CHAN_NO_PHS_ADD);
	gi.unicast (player, true);
}

void SendSoundToAll (int soundindex, float volume)
{
	int i;
	edict_t *e;

	for (i = 0; i < game.maxclients; i++)
	{
		e = g_edicts + 1 + i;
		if (!e->inuse)
			continue;
		if (e->is_bot)
			continue;

		unicastSound (e, soundindex, volume);
	}
}


int FindOpenIndexSpace (char *array[255])
{
	int i;

	for (i = 0; i < 254; i++)
	{
		// There's something there, skip.
		if (array[i] != NULL)
			continue;

		// Okay, we found an empty space - use it.
		return i;
	}
	// Oh jeez, we're out of indexes.. the sky is falling!

	return -1;
}

int IndexBeingUsed (char *index)
{
	int i;

	for (i = 0; i < 255; i++)
	{
		if (index == NULL || currentmodelindexes[i] == NULL)
			continue;
		// Nope, not it.
		if (index != currentmodelindexes[i])
			continue;

		// Wow, we got one!
		// This index is already used. Don't index it twice, moron.
		return 1;
	}

	return 0;
}

// Returns true if the function is a model.
qboolean IsNotAModel (char *index)
{
	char *pch;

	if (index == NULL || index[0] == 0 || (unsigned char) index[0] == 0xf8)
		return true;

	// Not an md2 file
	pch = strstr(index, ".md2");
	if (pch != NULL)
		return false;

	// Not a sprite
	pch = strstr(index, ".sp2");
	if (pch != NULL)
		return false;

	// Woot, it's a model.
	return true;
}

int ModelIndex (char *index)
{
	int in = FindOpenIndexSpace (currentmodelindexes);


	if (in == -1) // Out of indexes
	{
		gi.dprintf ("===========================\nWARNING: Model indexes overflowed! IWM recommends you restart the map!\n===========================");
		return 0;
	}

	if (IndexBeingUsed(index))
	{
		//gi.dprintf ("Index being used: %s\n", index);
		return gi.modelindex(index); // Already used, so just return without going forward.
	}

	// This model isn't actually a model, it's
	// a brush model. Skip this in printing, but still index it.
	if (IsNotAModel(index))
		modelindex_notamodel[in] = index;

	currentmodelindexes[in] = index;
	nummodelindexes++;

	//gi.dprintf ("%i %s\n", in, currentmodelindexes[in]);

	return gi.modelindex(index);
}

void CheckModelIndexes (edict_t *ent)
{
	int l;
	int index_t, bindex_t;
	gi.dprintf ("Current ModelIndexes (might overflow!)\n=========================\n");

	index_t = bindex_t = 0;

	for (l = 0; l < nummodelindexes; l++)
	{
		if (currentmodelindexes[l] == NULL)
			continue;
		if (currentmodelindexes[l][0] == 0)
			continue;
		if (IsNotAModel(currentmodelindexes[l]))
		{
			gi.dprintf ("Index %i: Brush Model (rotating object, door, etc)\n", l);
			bindex_t ++;
			continue;
		}
		if (currentmodelindexes[l] != NULL)
			gi.dprintf ("Index %i: %s\n", l, currentmodelindexes[l]);

		index_t++;
	}

	gi.dprintf ("\nTotal Indexes: %i\nBrush Models: %i\nNormal Indexes: %i\n=========================\n", bindex_t + index_t, bindex_t, index_t);
}




qboolean IsNotASound (char *index)
{
	char *pch;

	if (index == NULL || index[0] == 0 || (unsigned char) index[0] == 0xf8)
		return true;

	pch = strstr(index, ".wav");
	if (pch != NULL)
		return false;

	return true;
}


int SoundIndexBeingUsed (char *index)
{
	int i;

	for (i = 0; i < 254; i++)
	{
		if (index == NULL || currentsoundindexes[i] == NULL)
			continue;
		// Nope, not it.
		if (index != currentsoundindexes[i])
			continue;

		// Wow, we got one!
		// This index is already used. Don't index it twice, moron.
		return 1;
	}

	return 0;
}


int SoundIndex (char *index)
{
	int in = FindOpenIndexSpace (currentsoundindexes);

	if (in == -1) // Out of indexes
	{
		gi.dprintf ("===========================\nWARNING: Sound indexes overflowed! IWM recommends you restart the map!\n===========================");
		return 0;
	}

	if (SoundIndexBeingUsed(index))
		return gi.soundindex(index); // Already used, so just return without going forward.

	currentsoundindexes[in] = index;
	numsoundindexes++;

	//gi.dprintf ("%i %s\n", in, currentsoundindexes[in]);

	return gi.soundindex(index);
}

void CheckSoundIndexes (edict_t *ent)
{
	int l;
	gi.dprintf ("Current SoundIndexes (might overflow!)\n=========================\n");

	for (l = 0; l < numsoundindexes; l++)
	{
		if (currentsoundindexes[l] == NULL)
			continue;
		if (currentsoundindexes[l][0] == 0)
			continue;
		if (!currentsoundindexes[l] || currentsoundindexes[l] == 0x00000000)
			continue;
		/*if (currentmodelindexes[l][0] == -18)
		{
			gi.dprintf ("Index %i: Unknown Sound (Probably a variable)\n", l);
			continue;
		}*/
		if (IsNotASound(currentsoundindexes[l]))
		{
			gi.dprintf ("Index %i: Unknown Sound (Probably a variable)\n", l);
			continue;
		}
		if (currentsoundindexes[l] != NULL)
			gi.dprintf ("Index %i: %s\n", l, currentsoundindexes[l]);
	}

	gi.dprintf ("=========================\n");
}


int ImageIndexBeingUsed (char *index)
{
	int i;

	for (i = 0; i < 255; i++)
	{
		if (index == NULL || currentimageindexes[i] == NULL)
			continue;
		// Nope, not it.
		if (index != currentimageindexes[i])
			continue;

		// Wow, we got one!
		// This index is already used. Don't index it twice, moron.
		return 1;
	}

	return 0;
}

int ImageIndex (char *index)
{
	int in = FindOpenIndexSpace (currentimageindexes);

	if (in == -1) // Out of indexes
	{
		gi.dprintf ("===========================\nWARNING: Image indexes overflowed! IWM recommends you restart the map!\n===========================");
		return 0;
	}

	if (ImageIndexBeingUsed(index))
		return gi.imageindex(index); // Already used, so just return without going forward.

	currentimageindexes[in] = index;
	numimageindexes++;

	//gi.dprintf ("%i %s\n", in, currentsoundindexes[in]);

	return gi.imageindex(index);
}

void CheckImageIndexes (edict_t *ent)
{
	int l;
	gi.dprintf ("Current ImageIndexes (might overflow!)\n=========================\n");

	for (l = 0; l < numimageindexes; l++)
	{
		if (currentimageindexes[l] == NULL)
			continue;
		if (currentimageindexes[l][0] == 0)
			continue;
		if (currentimageindexes[l] != NULL)
			gi.dprintf ("Index %i: %s\n", l, currentimageindexes[l]);
	}

	gi.dprintf ("=========================\n");
}
