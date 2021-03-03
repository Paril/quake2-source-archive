//===========================================================================
//
// Name:         g_log.h
// Function:     log file stuff
// Programmer:   Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:  1997-12-31
// Tab Size:     3
//===========================================================================

qboolean LogCmd(char *cmd);
void Log_ShutDown(void);
void Log_Write(char *fmt, ...);
void Log_WriteTimeStamped(char *fmt, ...);

