#include "g_local.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include "g_genmidi.h"
#include "g_gensock.h"

#define STATE_OFF		0
#define STATE_PLAYING	1
#define STATE_PAUSED	2
#define STATE_STOPPED	3

float midiendtime=0.0;

typedef struct g_midistate
{
	int state;
	char filename[128];
	unsigned int unpausePos;
	HWND	hwnd;
	MCIDEVICEID mciDevice;
	float	time;
}midistate_t;
midistate_t midstate;


float GetCurMidiTime()
{
return level.miditime;
}


char *GetExtension(char *filename)
{
	static char exten[8];
	int		i;
	memset(exten,0,8);

	while (*filename && *filename != '.')
		filename++;
	if (!*filename)
		return "";
	filename++;
	for (i=0 ; i<7 && *filename ; i++,filename++)
		exten[i] = *filename;
	exten[i] = 0;
	return exten;
}



void ChkLocalMidi(void)
{
	if(midstate.state == STATE_PLAYING)
	{
		if(level.time >= midiendtime)
		{
//gi.dprintf("MIDI ENDED\n");
			if(!GenRestartMidi())
				GenStopMidi();
		}
	}
}


int SendMCI(MCIDEVICEID device, UINT msg, DWORD command, DWORD param)
{
	int res;
	res = mciSendCommand(device, msg, command, param);
	if (res) 
	{
		char errorStr[256];
		mciGetErrorString(res, errorStr, 255);
		gi.dprintf("MCI error: %s\n",errorStr);
	}
	return res;
}

int GenPlayMidi(char *midi)
{
	MCI_OPEN_PARMS openParms;
	MCI_PLAY_PARMS playParms;
	

	midstate.time =GetCurMidiTime();
	if(!midstate.time)
	{
		gi.dprintf("Couldnt find run time of track\n");
		return 0;
	}
	
	if(midstate.state == STATE_PLAYING)
		if(!GenShutdownMidi())
			return 0;

	//save misc info
	strcpy(midstate.filename,"gen\\midi\\");
	strcat(midstate.filename,midi);

	if(strcmp(GetExtension(midi),"mid"))
		strcat(midstate.filename,".mid");

//gi.dprintf("Trying to play :%s\n",midstate.filename);
	midstate.hwnd = GetForegroundWindow();
	openParms.lpstrDeviceType = "sequencer";
	openParms.lpstrElementName=midstate.filename;
	
	if(!midstate.state)
	if (SendMCI	(0, 
				 MCI_OPEN, 
				 MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, 
				(DWORD)&openParms))
	{
		gi.dprintf("MCI - Error initializing\n");
		return 0;
	}

	midstate.state = STATE_STOPPED;
	//save the Device ID to struct
	midstate.mciDevice = openParms.wDeviceID;
	
	playParms.dwCallback = (DWORD)midstate.hwnd;
	if(SendMCI (openParms.wDeviceID, 
				MCI_PLAY, 
				MCI_NOTIFY, 
				(DWORD)&playParms)) 
	{
		SendMCI (openParms.wDeviceID, MCI_CLOSE, 0, (DWORD)NULL);
		return 0;
	}
	
	midstate.state = STATE_PLAYING;
	midiendtime = level.time + 	midstate.time - 1.0;	
	return 1;
}


int GenPauseMidi (void)
{
	MCI_SET_PARMS setParms;
	MCI_STATUS_PARMS statusParms;

	if(midstate.state != STATE_PLAYING)
		return 0;

	setParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	if (SendMCI (midstate.mciDevice, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)&setParms))
			return 0;

	statusParms.dwCallback = (DWORD)midstate.hwnd;
	statusParms.dwItem = MCI_STATUS_POSITION;
	if (SendMCI (midstate.mciDevice, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&statusParms))
			return 0;

	midstate.unpausePos = statusParms.dwReturn;
	SendMCI (midstate.mciDevice, MCI_STOP, 0, (DWORD)NULL);
	midstate.state = STATE_PAUSED;
	return 1;
}

int GenResumeMidi (void)
{
	MCI_SET_PARMS setParms;
	MCI_PLAY_PARMS playParms;

	if(midstate.state != STATE_PAUSED)
		return 0;

	setParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	if (SendMCI (midstate.mciDevice, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)&setParms))
		return 0;
			
	playParms.dwCallback = (DWORD)midstate.hwnd;
	playParms.dwTo = 0;
	playParms.dwFrom = midstate.unpausePos;
	if (SendMCI (midstate.mciDevice, MCI_PLAY, MCI_NOTIFY | MCI_FROM, (DWORD)&playParms))
		return 0;

	midstate.state = STATE_PLAYING;
	return 1;
}


int GenRestartMidi (void)
{
	MCI_PLAY_PARMS playParms;

	if(midstate.state != STATE_PLAYING)
		return 0;

	playParms.dwFrom = 0;
	playParms.dwCallback = (DWORD)midstate.hwnd;
	if (SendMCI (midstate.mciDevice, MCI_PLAY, MCI_NOTIFY | MCI_FROM, (DWORD)(LPVOID)&playParms)) 
	{
		SendMCI (midstate.mciDevice, MCI_CLOSE, 0, (DWORD)NULL);
		midstate.mciDevice = 0;
		return 0;
	}

	midiendtime = level.time + midstate.time + 1.0;	
	return 1;
}


int GenStopMidi(void)
{
	if (midstate.state == STATE_STOPPED || 
		midstate.state == STATE_OFF)
		return 0;

	midstate.state = STATE_STOPPED;
	midiendtime = 0.0;

	SendMCI (midstate.mciDevice, MCI_STOP, 0, (DWORD)NULL);
	return 1;
}

int GenShutdownMidi(void)
{
	if (midstate.state == STATE_OFF)
		return 0;

	GenStopMidi();
	midstate.state = STATE_OFF;
	midstate.time = 0.0;
	
	SendMCI (midstate.mciDevice, MCI_CLOSE, 0, (DWORD)NULL);
	midstate.mciDevice = 0;
	return 1;
}

//Interface to the midi system
//Cleanup, add more checks
//minimum traffic, dont depend on client side listener
#define MSG_PLAY		'a'		
#define	MSG_PAUSE		'b'
#define MSG_RESUME		'c'
#define MSG_STOP		'd'
#define MSG_DPLAYINTERMISSION 'i'


void GemMidiCommand(edict_t *ent,int cmd)
{
#ifdef GEN_SOCK
	if(dedicated->value)
	if(cmd == MCMD_PLAY)
	{
		char midi[8];
		char *name;
		
		int a = gi.argc();
		if(a==2)
		{
			name = gi.argv(2);
			if(name)
			{
				if(strlen(name) <= 8)
					strcpy(midi,name);
			}
		}
		else if(strlen(level.midiname))
				strcpy(midi, level.midiname);
//cur default				
		else
		{
			strcpy(midi,"D_e1m7");
			level.miditime = 150;
		}

		gi.dprintf("Send Play Midi :%s to %s\n", midi,ent->client->sock.ipaddr);
		//gi.dprintf("Send Play Midi :%s to %s\n", midi,ent->client->ipaddr);
		if(!GenSendMsg(ent->client,MSG_PLAY,midi))
			gi.dprintf("Error Sending CMD play\n");
		return;
	}
#endif
	GenMidiCmd(ent,cmd);


}

void GenMidiCmd(edict_t *ent,int cmd)
{
#ifdef GEN_SOCK
	if(dedicated->value)
	{
		switch(cmd)
		{
			case  MCMD_PLAY:
			{
				char midi[8];
/*				char *name;

				int a = gi.argc();
				if(a==1)
				{
					name = gi.argv(1);
					if(name)
					{
						if(strlen(name) <= 8)
							strcpy(midi,name);
					}
				}
				else*/
				if(strlen(level.midiname))
						strcpy(midi, level.midiname);
//cur default				
				else
				{
					strcpy(midi,"D_e1m7");
//					level.miditime = 150;
				}

				gi.dprintf("Send Play Midi :%s to %s\n", midi,ent->client->sock.ipaddr);
				//gi.dprintf("Send Play Midi :%s to %s\n", midi,ent->client->ipaddr);
				if(!GenSendMsg(ent->client,MSG_PLAY,midi))
					gi.dprintf("Error Sending CMD play\n");
				break;
			}
			case MCMD_PAUSE:
			{
				if(!GenSendMsg(ent->client,MSG_PAUSE,NULL))
					gi.dprintf("Error Sending CMD pause\n");
				break;
			}
			case MCMD_STOP:
			{
				if(!GenSendMsg(ent->client,MSG_STOP,NULL))
					gi.dprintf("Error Sending CMD stop \n");
				break;
			}
			case MCMD_RESUME:
			{
				if(!GenSendMsg(ent->client,MSG_RESUME,NULL))
					gi.dprintf("Error Sending CMD resume \n");
				break;
			}
			case MCMD_DINTER:
			{
				if(!GenSendMsg(ent->client,MSG_DPLAYINTERMISSION,NULL))
					gi.dprintf("Error Sending CMD intermission \n");
				break;
			}
		}
		return;	
	}
#endif

//	if(dedicated->value)
//		return;

	switch(cmd)
	{
		case MCMD_PLAY:
			{
				char midi[128];
	
				if(strlen(level.midiname))
					strcpy(midi, level.midiname);
				else
				{
					strcpy(midi,"D_e1m7");
					level.miditime = 150;
				}
				
//				gi.dprintf("Midipath :%s\n", midi); 
	
				if(GenPlayMidi(midi))
					return;
				gi.dprintf("Error Playing Midi\n");
				break;
			}
		case MCMD_PAUSE:
			{
				if(GenPauseMidi())
					return;
				gi.dprintf("Error Pausing Midi\n");
				break;
			}
		case MCMD_STOP:
			{
				if(GenStopMidi())
					return;
				gi.dprintf("Error Stopping Midi\n");
				break;
			}
		case MCMD_RESUME:
			{
				if(GenResumeMidi())
					return;
				gi.dprintf("Error Resuming Midi\n");
				break;
			}
	}
}


#endif
  
