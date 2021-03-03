// q_extra.c -- misc utility functions for game module

#include "g_local.h"
//#include "q_extra.h"
//#include <mmsystem.h>
//#include "fmod.h"
//#include "stream.h"
//#include <windows.h>
/*
==============================================================================

MIDI functions

==============================================================================


void Cmd_PlayMidi_f (edict_t *ent)
{
	char			string[256];
	char			fileName[MAX_OSPATH * 2];  //just to be safe
	char			*shortName = fileName;
	int 			addedPath;

	if (midi->string == NULL)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must first set the midi cvar: midi \"filename\"\n");
		return;
	}

	if (mciSendString("close all", NULL, 0, NULL) != 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Cmd_PlayMidi_f: unable to \"close all\"\n");
		return;
	}

	addedPath = FixFileName (midi->string, fileName, gamedir->string, MIDI_STANDARD_PATH, MIDI_EXTENSION);
	shortName = &fileName[addedPath];
	//gi.cprintf (ent, PRINT_HIGH, "filename: %s\n", fileName);
	sprintf(string, "open sequencer!%s alias MIDI", fileName);

	if (mciSendString(string, NULL, 0, NULL) != 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Unable to open MIDI file: %s\n", shortName);
		return;
	}

	if (mciSendString("play MIDI from 0", NULL, 0, NULL) != 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Error playing MIDI: %s\n", shortName);
		return;
	}

	gi.cprintf (ent, PRINT_HIGH, "Playing MIDI: %s\n", shortName);
	Cmd_PauseMidi_f = MidiPause;
	Cmd_ResumeMidi_f = MidiResume;
}

void MidiPause (edict_t *ent)
{
	if (mciSendString("stop MIDI", NULL, 0, NULL) != 0)
		gi.cprintf (ent, PRINT_HIGH, "Error pausing MIDI: no MIDI to pause\n");
}

void MidiResume (edict_t *ent)
{
	if (mciSendString("play MIDI", NULL, 0, NULL) != 0)
		gi.cprintf (ent, PRINT_HIGH, "Error resuming MIDI: no MIDI to resume\n");
}

void Cmd_StopMidi_f ()
{
	mciSendString("close MIDI", NULL, 0, NULL);
	Cmd_PauseMidi_f = MidiNotPlaying;
	Cmd_ResumeMidi_f = MidiNotPlaying;
}

void MidiNotPlaying (edict_t *ent)
{
	gi.cprintf (ent, PRINT_HIGH, "Use command playmidi to start MIDI\n");
}
*/
void StartMP3 ()
{
FSOUND_Init(22050, 1, 0);
//FSOUND_Init(44100, 1, 0);


//FSOUND_SetBufferSize(15000);

//stream = FSOUND_Stream_OpenMpeg("./007q2/number1.mp3", FSOUND_NORMAL | FSOUND_LOOP_NORMAL);

//sample = FSOUND_Sample_LoadMpeg(FSOUND_FREE, "./007q2/number1.mp3", FSOUND_16BITS);

//FSOUND_Stream_Play(FSOUND_FREE, stream);
//FSOUND_PlaySound(FSOUND_FREE, sample);

}

void StopMP3 ()
{
char buf_filename[256];
FSOUND_STREAM	*stream;// = NULL;

	strcpy(buf_filename, ".//007q2//sound//");

	strncat(buf_filename, level.mapname ,24);

	strcat(buf_filename, ".mp3");

stream = FSOUND_Stream_GetSample(buf_filename, FSOUND_NORMAL | FSOUND_LOOP_NORMAL);

// if (
FSOUND_Stream_Stop(stream);//)
//    	gi.cprintf (ent, PRINT_HIGH, "stop failed\n");	  

if (FSOUND_Stream_Close(stream))
//    	gi.cprintf (ent, PRINT_HIGH, "stop stream succeeded\n");
FSOUND_Close();
}
/*
==============================================================================

misc functions

==============================================================================


void QuakeExePath (char *path)
{
	int		current = 0;
	int		last = strlen(__argv[0]) + 1;  //FIXME: Do I really need this =?

	gi.bprintf (PRINT_MEDIUM,"last: %i\n", last);
			
	do
	{
		if (((__argv[0])[current] == '\\') || ((__argv[0])[current] == '//'))
			last = current;
		path[current] = (__argv[0])[current++];
	} while ((__argv[0])[current]);
	
	path[last] = 0;
}                        */
/*
int FixFileName (char *in, char *out, char *modpath, char *spath, char *extension)
{
	char			*ain = in;
	char			*dir = spath;
	char			*ext = extension;
	char			*modpathbak = modpath;
	char			*sending = out;
	char			*path = spath;
	int				addedRoot = 0;
	int				doAdd = 1;
	FILE			*file;
	
	if ((in[0] != '\\') && (in[0] != '/') && (in[1] != ':'))
	{
		do  //add mod path
			out++[0] = modpath[addedRoot++];
		while (modpath[addedRoot]);
		out++[0] = '\\';
		addedRoot++;

		do  //add file type path
			out++[0] = spath++[0];
		while (spath[0]);
		out++[0] = '\\';
	}

	do
	{
		out++[0] = in++[0];
		if (doAdd)
		{
			if (in[0] == '.')
				doAdd = 0;
		}
		else
			if ((in[0] == '//') || (in[0] == '\\'))
				doAdd = 1;
	} while (in[0]);

	if (doAdd)
	{
		do  //add extension
			out++[0] = extension++[0];
		while (extension[0]);
	}
		
	out[0] = 0;  //close string

	if ((file = fopen(sending, "r")) == NULL)
	{
		if (Q_strcasecmp(modpathbak, BASE) == 0)
			return addedRoot;

		return FixFileName (ain, sending, BASE, path, ext);
	}

	if (fclose(file))
		gi.bprintf (PRINT_HIGH, "Uh oh, can't close file");

	return addedRoot;
}
   
*/