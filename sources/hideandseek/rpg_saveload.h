// Save/Load class for players.
class CSaveLoad
{
protected:

	char			fileName[MAX_QPATH];
	struct edict_s	*player;
	FILE			*fp;
	void			*lastReadData;

public:

	void Create (struct edict_s *ent);
	void Write (void *data, int size)
	{
		if (!fp)
			return;

		fwrite (data, size, 1, fp);
	}
	void WriteAmt (void *data, int size, int count)
	{
		if (!fp)
			return;

		fwrite (data, size, count, fp);
	}
	void *Read (int size)
	{
		if (!fp)
			return NULL;

		fread (lastReadData, size, 1, fp);
		return lastReadData;
	}
	void *ReadAmt (int size, int count)
	{
		if (!fp)
			return NULL;

		fread (lastReadData, size, count, fp);
		return lastReadData;
	}

	bool Save ();
	bool Load ();
} SaveLoad;