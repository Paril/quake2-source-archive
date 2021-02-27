// ModUtils.c
//
// (c) 1998 Two Pole Software
// a.k.a. Rohn, SteQve, and Mr. X
//
// programmed by SteQve (steqve@shore.net)

#define UTIL_DEBUG 0

#define UTIL_SHELL_COLOR_NONE 0
#define UTIL_SHELL_COLOR_RED (RF_SHELL_RED)
#define UTIL_SHELL_COLOR_GREEN (RF_SHELL_GREEN)
#define UTIL_SHELL_COLOR_BLUE (RF_SHELL_BLUE)
#define UTIL_SHELL_COLOR_WHITE (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE)
#define UTIL_SHELL_COLOR_YELLOW (RF_SHELL_RED | RF_SHELL_GREEN)
#define UTIL_SHELL_COLOR_PURPLE (RF_SHELL_RED | RF_SHELL_BLUE)
#define UTIL_SHELL_COLOR_CYAN (RF_SHELL_GREEN | RF_SHELL_BLUE)

int StrBeginsWith (char *s1, char *s2);
void SetPlayerShellColor (edict_t *ent, int color);
int PlayerHasShellEffects (edict_t *ent);
edict_t *FindClientByNetname (char *name);
edict_t *FindClientByMatchingNetname (char *name);
edict_t *GetRandomClient ();
void MakeBoldString (char *text);
void FormatBoldString (char *text);
int CountActiveClients ();
int CountConnectedClients ();
int IsActiveClient (edict_t *ent);

// Hey C programming experts: I don't want to hear about va_args, OK? ;-)
// I want to learn about them only a little more than I want to learn about
// terminal I/O.
#if UTIL_DEBUG
#define DPRINT0(m) {printf(m); fflush(stdout);}
#define DPRINT1(m, a) {printf(m, a); fflush(stdout);}
#define DPRINT2(m, a, b) {printf(m, a, b); fflush(stdout);}
#define DPRINT3(m, a, b, c) {printf(m, a, b, c); fflush(stdout);}
#define DPRINT6(m, a, b, c, d, e, f) {printf(m, a, b, c, d, e, f); fflush(stdout);}

#define BPRINT0(m) printf(m)
#define BPRINT1(m, a) printf(m, a)
#define BPRINT2(m, a, b) printf(m, a, b)
#define BPRINT3(m, a, b, c) printf(m, a, b, c)
#define BPRINT4(m, a, b, c, d) printf(m, a, b, c, d)

#define CPRINT0(c, m) printf(c, m)
#define CPRINT1(c, m, a) printf(c, m, a)
#define CPRINT2(c, m, a, b) printf(c, m, a, b)

#define PRINT0(m) printf(m)
#define PRINT1(m, a) printf(m, a)
#define PRINT2(m, a, b) printf(m, a, b)
#define PRINT3(m, a, b, c) printf(m, a, b, c)

#else

#define DPRINT0(m) gi.dprintf(m)
#define DPRINT1(m, a) gi.dprintf(m, a)
#define DPRINT2(m, a, b) gi.dprintf(m, a, b)
#define DPRINT3(m, a, b, c) gi.dprintf(m, a, b, c)
#define DPRINT6(m, a, b, c, d, e, f) gi.dprintf(m, a, b, c, d, e, f)

#define BPRINT0(m) gi.bprintf(PRINT_MEDIUM, m)
#define BPRINT1(m, a) gi.bprintf(PRINT_MEDIUM, m, a)
#define BPRINT2(m, a, b) gi.bprintf(PRINT_MEDIUM, m, a, b)
#define BPRINT3(m, a, b, c) gi.bprintf(PRINT_MEDIUM, m, a, b, c)
#define BPRINT4(m, a, b, c, d) gi.bprintf(PRINT_MEDIUM, m, a, b, c, d)

#define CPRINT0(c, m) gi.cprintf(c, PRINT_MEDIUM, m)
#define CPRINT1(c, m, a) gi.cprintf(c, PRINT_MEDIUM, m, a)
#define CPRINT2(c, m, a, b) gi.cprintf(c, PRINT_MEDIUM, m, a, b)

#define PRINT0(m) 
#define PRINT1(m, a) 
#define PRINT2(m, a, b) 
#define PRINT3(m, a, b, c) 

#endif
