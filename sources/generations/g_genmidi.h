

#define MCMD_PLAY	1
#define MCMD_PAUSE	2
#define MCMD_RESUME	3
#define MCMD_STOP	4
#define MCMD_DINTER	5

extern struct g_midistate midstate;

int GenStopMidi(void);
int GenResumeMidi (void);
int GenPauseMidi (void);
int GenRestartMidi (void);
int GenShutdownMidi(void);

void GenMidiCmd(edict_t *ent,int cmd);
void GemMidiCommand(edict_t *ent,int cmd); //uses gi.argcs
float GetCurMidiTime();
void ChkLocalMidi(void);