


/* FUNCTIONS ADAPTED FROM idsoftware SOURCE CODE */

int Q_strncasecmp (char *s1, char *s2, int n)
{
	int		c1, c2;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;		// strings are equal until end point
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;		// strings not equal
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int Q_strcasecmp (char *s1, char *s2)
{
	return Q_strncasecmp (s1, s2, 99999);
}

// FIXME: replace all Q_stricmp with Q_strcasecmp
int Q_stricmp (char *s1, char *s2)
{
//#if defined(WIN32)
//	return _stricmp (s1, s2);
//#else
//	return strcasecmp (s1, s2);
//#endif
	return Q_strcasecmp(s1, s2);
}
