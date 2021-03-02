#include <ctype.h>
#include <string.h>

void strupr(char *c)
{
	int i;
	if (!c) return;
	for (i=0;i<strlen(c);i++)
  {
  	c[i]=toupper(c[i]);
  }
}
