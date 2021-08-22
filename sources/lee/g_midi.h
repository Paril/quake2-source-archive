

#define MCMD_PLAY	1
#define MCMD_PAUSE	2
#define MCMD_RESUME	3
#define MCMD_STOP	4
#define MCMD_DINTER	5

extern struct g_midistate midstate;

int StopMidi(void);
int ResumeMidi (void);
int PauseMidi (void);
int RestartMidi (void);
int ShutdownMidi(void);

void MidiCmd(edict_t *ent, int cmd);
void MidiCommand(edict_t *ent,int cmd); //uses gi.argcs
float GetCurMidiTime();
void ChkLocalMidi(ent);