// q_extra.h -- no comment

#ifndef _EXTRAMOD_
#define _EXTRAMOD_

#include <windows.h>
#include <mmsystem.h>

//#define	BASE					"baseq2"
#define	BASE					"007q2"
#define	MIDI_STANDARD_PATH		"midi"
#define	MIDI_EXTENSION			".mid"

//new globals
char	basepath[MAX_OSPATH * 2];  //just to be safe
//misc functions
void	QuakeExePath (char *path);
int		FixFileName (char *in, char *out, char *modpath, char *spath, char *extension);
//midi functions
void	Cmd_PlayMidi_f (edict_t *ent);
void	MidiPause (edict_t *ent);
void	MidiResume (edict_t *ent);
void	Cmd_StopMidi_f ();
void	(*Cmd_PauseMidi_f) (edict_t *ent);
void	(*Cmd_ResumeMidi_f) (edict_t *ent);
void	MidiNotPlaying (edict_t *ent);
void	StartMP3();
void	StopMP3();

#endif
