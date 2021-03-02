// Class for experience and levelling up
class CExperience
{
	// "ready" if you can add exp to it.
	// Player experience members
	bool			ready;
	unsigned int	toNext, toPrev;

	// Player and monster experience members
	unsigned int	level;
	unsigned int	experience;

public:
	// FIXME: Check if this is actually called
	CExperience ()
	{
		ready = false;
		level = 0;
		experience = 0;
		toNext = toPrev = 0;
	}

	void Init ()
	{
		ready = true;
		level = 0;
		experience = 0;
		toNext = 100;
	}

	int SetToNextLevel ()
	{
		toPrev = toNext;
		toNext = (toPrev + (toPrev/2));

		experience -= toPrev;
	}

	// Returns true if you levelled up.
	// Does not actually level you up, though.
	bool CheckLevelup ()
	{
		if (experience >= toNext)
			return true;
		return false;
	}

	void AddExp (int exp)
	{
		if (!ready)
			return;

		experience += exp;

		if (CheckLevelup())
			SetToNextLevel();
	}

	void ForceLevel (int up, bool exp)
	{
		if (exp)
			AddExp (toNext);
		else
			level += up;
	}
} Experience;