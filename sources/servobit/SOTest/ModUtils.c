/* ***** STRIPPED MODUTILS.C *****
*/

//#include "g_local.h"
#include "ModUtils.h"

// Slightly adapted from Q_strncasecmp.
int StrBeginsWith (char *s1, char *s2)
	{
	int		c1, c2, max = 999999;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;
			if (!c1) return(1); // Reached end of search string
		if (!max--)
			return 1;		// strings are equal until end point
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return 0;		// strings not equal
		}
	} while (c1);
	
	return 1;		// strings are equal
}


// DESTRUCTIVELY converts an entire string to bold text.
void MakeBoldString (char *text)
{
	while (*text)
		if (*text == '\n') 
			text++;
		else
			*(text++) = *text + 0x80;
	return;
}

// DESTRUCTIVE!!!
void FormatBoldString (char *text)
{
	int t, i, do_bold;

	t=0;
	do_bold = 0;
	for (i=0; text[i] != '\0'; i++)
	{
		// WOW!!!!  Check out that inefficiency!!!!  I am DANGEROUS!!!
		if (StrBeginsWith("<b>", text+i))
		{
			do_bold = 1;
			i += 2;
		}
		else if (StrBeginsWith("</b>", text+i))
		{
			do_bold = 0;
			i += 3;
		}
		else if (do_bold)
		{
			if (*(text+i) == '\n')
				text[t++] = *(text+i);
			else
				text[t++] = *(text+i) + 0x80;
		}
		else {
			text[t++] = *(text+i);
		}
	}
	text[t] = '\0';
}
