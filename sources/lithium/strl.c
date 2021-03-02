#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "strl.h"

#ifdef STRL_DEBUG_BACKTRACE
#include <execinfo.h>
#endif

#define MIN(x,y) (((x) < (y)) ? (x) : (y))

#ifdef NEED_STRLCAT

size_t strlcat_s(char *dest, char *src, size_t n, int count) {
	size_t fr;
	size_t ret;
	size_t copy;
	size_t srclen;
	char overflow;

	srclen = strlen(src);
	ret = strlen(dest) + srclen;

	fr = n - (ret - srclen) - 1;

	if (count < 0) {
		copy = fr;
		overflow = (ret >= n);
	} else {
		copy = MIN(count,(int)fr);
		overflow = (count > (int)fr);
	}
		
	if (fr > 0)
		strncat(dest, src, copy);

#ifdef STRL_DEBUG
	if (overflow) {
		if (count < 0)
			fprintf(stdout, "strlcat: buffer overflow avoided (%zd >= %zd)\n", ret, n);
		else
			fprintf(stdout, "strlcat_s: buffer overflow avoided (%zd >= %zd)\n", (ret - srclen)+copy, n);
	
#ifdef STRL_DEBUG_BACKTRACE
		{
			void* array[10];
			size_t size;
			char **strings;
			size_t i;
			char s[64];
			size_t sl;

			sl = MIN(srclen,63);
			strncpy(s, src, sl);
			s[63] = '\0';

			size = backtrace(array, 10);
			strings = backtrace_symbols(array, size);

			if (strings != NULL) {
				for (i=0; i<size; i++)
					fprintf(stdout, "  %s\n", strings[i]);
				free(strings);
			}
			fprintf(stdout, "Source buffer: %s%s\n", s, (ret > 63)?"...":"");
		}
#endif
		fflush(stdout);
	}
#endif

	return ret;
}

#endif

#ifdef NEED_STRLCPY

size_t strlcpy_s(char *dest, char *src, size_t n, int count) {
	size_t ret;
	size_t copy;
	char overflow;

	ret = strlen(src);
	if (n == 0)
		return ret;

	if (count < 0) {
		copy = n-1;
		overflow = (ret >= n);
	} else {
		copy = MIN(count, (int)n-1);
		overflow = (count >= (int)n);
	}

	strncpy(dest, src, copy);
	*(dest+copy) = '\0';

#ifdef STRL_DEBUG
	if (overflow) {
		if (count < 0)
			fprintf(stdout, "strlcpy: buffer overflow avoided (%zd >= %zd)\n", ret, n);
		else
			fprintf(stdout, "strlcpy_s: buffer overflow avoided (%d >= %zd)\n", count, n);
#ifdef STRL_DEBUG_BACKTRACE
		{
			void* array[10];
			size_t size;
			char **strings;
			size_t i;
			char s[64];
			size_t sl;

			sl = MIN(ret,63);
			strncpy(s, src, sl);
			s[63] = '\0';

			size = backtrace(array, 10);
			strings = backtrace_symbols(array, size);

			if (strings != NULL) {
				for (i=0; i<size; i++)
					fprintf(stdout, "  %s\n", strings[i]);
				free(strings);
			}
			fprintf(stdout, "Source buffer: %s%s\n", s, (ret > 63)?"...":"");
		}
#endif
		fflush(stdout);
	}
#endif

	return ret;
}

#endif
