#include "g_local.h"

void edict_t::CSaveLoad::Create (struct edict_s *ent)
{
	Q_snprintfz (fileName, MAX_QPATH, "qrpg/characters/%s.chr", ent->client->pers.netname);
	player = ent;
}

bool edict_t::CSaveLoad::Save ()
{
	if (!player || !fileName[0])
		return false;

	fp = fopen (fileName, "w+");

	// Should this happen?
	if (!fp)
		return false;

	// Write poop here

	fclose (fp);

	return true;
}

bool edict_t::CSaveLoad::Load ()
{
	if (!player || !fileName[0])
		return false;

	fp = fopen (fileName, "w+");

	if (!fp)
		return false;

	// Read poop here

	fclose (fp);

	return true;
}