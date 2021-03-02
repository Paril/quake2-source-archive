/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#if defined(__FreeBSD__)
#include <sys/soundcard.h>
#endif
#if defined(__linux__)
#include <linux/soundcard.h>
#endif
#include <stdio.h>

#include "../client/client.h"
#include "../client/snd_loc.h"

int audio_fd = -1;
int snd_inited;

cvar_t *s_khz;

static int tryrates[] = { 48000, 44100, 22050, 11025, 8000 };

struct sndinfo * si;

qboolean SNDDMA_Init(struct sndinfo * s)
{

	int rc;
	int fmt;
	int tmp;
	int i;
	struct audio_buf_info info;
	int caps;

	//if (snd_inited)
	//	return 1;

	snd_inited = 0;
	si = s;
	
	if(!strcmp(si->snddevice->string, "default"))  //silly alsa default
		si->snddevice->string = "/dev/dsp";

// open /dev/dsp, confirm capability to mmap, and get size of dma buffer

	if (audio_fd == -1)
	{
		audio_fd = open(si->snddevice->string, O_RDWR);

		if (audio_fd == -1)
		{
			perror(si->snddevice->string);
			si->Com_Printf("SNDDMA_Init: Could not open %s.\n", si->snddevice->string);
			return 0;
		}
	}

	rc = ioctl(audio_fd, SNDCTL_DSP_RESET, 0);
	if (rc == -1)
	{
		perror(si->snddevice->string);
		si->Com_Printf("SNDDMA_Init: Could not reset %s.\n", si->snddevice->string);
		close(audio_fd);
		audio_fd = -1;
		return 0;
	}

	if (ioctl(audio_fd, SNDCTL_DSP_GETCAPS, &caps)==-1)
	{
		perror(si->snddevice->string);
		si->Com_Printf("SNDDMA_Init: Sound driver too old.\n");
		close(audio_fd);
		audio_fd = -1;
		return 0;
	}

	if (!(caps & DSP_CAP_TRIGGER) || !(caps & DSP_CAP_MMAP))
	{
		si->Com_Printf("SNDDMA_Init: Sorry, but your soundcard doesn't support trigger or mmap. (%08x)\n", caps);
		close(audio_fd);
		audio_fd = -1;
		return 0;
	}
    
// set sample bits & speed

	si->dma->samplebits = (int)si->sndbits->value;
	if (si->dma->samplebits != 16 && si->dma->samplebits != 8)
	{
		ioctl(audio_fd, SNDCTL_DSP_GETFMTS, &fmt);
		if (fmt & AFMT_S16_LE) si->dma->samplebits = 16;
		else if (fmt & AFMT_U8) si->dma->samplebits = 8;
	}

	if (si->dma->samplebits == 16)
	{
        	rc = AFMT_S16_LE;
		rc = ioctl(audio_fd, SNDCTL_DSP_SETFMT, &rc);
		if (rc < 0)
		{
			perror(si->snddevice->string);
			si->Com_Printf("SNDDMA_Init: Could not support 16-bit data.  Try 8-bit.\n");
			close(audio_fd);
			audio_fd = -1;
			return 0;
		}
	}
	else if (si->dma->samplebits == 8)
    	{
		rc = AFMT_U8;
		rc = ioctl(audio_fd, SNDCTL_DSP_SETFMT, &rc);
		if (rc < 0)
		{
			perror(si->snddevice->string);
			si->Com_Printf("SNDDMA_Init: Could not support 8-bit data.\n");
			close(audio_fd);
			audio_fd = -1;
			return 0;
		}
	}
	else
	{
		perror(si->snddevice->string);
		si->Com_Printf("SNDDMA_Init: %d-bit sound not supported.", si->dma->samplebits);
		close(audio_fd);
		audio_fd = -1;
		return 0;
	}

	si->dma->speed = (int)si->sndspeed->value;

	if (!si->dma->speed) {	
		
		if (si->s_khz->value == 48) {
			si->dma->speed = 48000;
		}
		else if (si->s_khz->value == 44) {
			si->dma->speed = 44100;
		}
		else if (si->s_khz->value == 22) {
			si->dma->speed = 22050;
		}
		else  {
			si->dma->speed = 11025;
		}
			si->dma->speed = 48000; /** Uhh **/
	}
	
	if (!si->dma->speed)
	{
		for (i=0 ; i<sizeof(tryrates)/4 ; i++)
			if (!ioctl(audio_fd, SNDCTL_DSP_SPEED, &tryrates[i]))
				break;
		si->dma->speed = tryrates[i];
	}

	si->dma->channels = (int)si->sndchannels->value;
	if (si->dma->channels < 1 || si->dma->channels > 2)
		si->dma->channels = 2;
        
	tmp = 0;
	if (si->dma->channels == 2)
		tmp = 1;
	rc = ioctl(audio_fd, SNDCTL_DSP_STEREO, &tmp); //FP: bugs here.
	if (rc < 0)
	{
		perror(si->snddevice->string);
		si->Com_Printf("SNDDMA_Init: Could not set %s to stereo=%d.", si->snddevice->string, si->dma->channels);
		close(audio_fd);
		audio_fd = -1;
		return 0;
	}

	if (tmp)
		si->dma->channels = 2;
	else
		si->dma->channels = 1;


	rc = ioctl(audio_fd, SNDCTL_DSP_SPEED, &si->dma->speed);
	if (rc < 0)
	{
		perror(si->snddevice->string);
		si->Com_Printf("SNDDMA_Init: Could not set %s speed to %d.", si->snddevice->string, si->dma->speed);
		close(audio_fd);
		audio_fd = -1;
		return 0;
	}

	if (ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info)==-1) 
	{   
		perror("GETOSPACE");
		si->Com_Printf("SNDDMA_Init: GETOSPACE ioctl failed.\n");
		close(audio_fd);
		audio_fd = -1;
		return 0;
	}
	si->dma->samples = info.fragstotal * info.fragsize / (si->dma->samplebits/8);
	si->dma->submission_chunk = 1;

// memory map the dma buffer

	if (!si->dma->buffer)
		si->dma->buffer = (unsigned char *)mmap(NULL, info.fragstotal
		         * info.fragsize, PROT_WRITE|PROT_READ, MAP_FILE|MAP_SHARED, audio_fd, 0);
	if (!si->dma->buffer || si->dma->buffer == MAP_FAILED)
	{
		perror(si->snddevice->string);
		si->Com_Printf("SNDDMA_Init: Could not mmap %s.\n", si->snddevice->string);
		close(audio_fd);
		audio_fd = -1;
		return 0;
	}

// toggle the trigger & start her up

	tmp = 0;
	rc  = ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
	if (rc < 0)
	{
		perror(si->snddevice->string);
		si->Com_Printf("SNDDMA_Init: Could not toggle. (1)\n");
		close(audio_fd);
		audio_fd = -1;
		return 0;
	}
	tmp = PCM_ENABLE_OUTPUT;
	rc = ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
	if (rc < 0)
	{
		perror(si->snddevice->string);
		si->Com_Printf("SNDDMA_Init: Could not toggle. (2)\n");
		close(audio_fd);
		audio_fd = -1;
		return 0;
	}

	si->Com_Printf("\nInitializing OSS Sound System\n");
	si->Com_Printf("Buffer size: %d\n", info.fragstotal * info.fragsize);
	si->Com_Printf("Stereo: %d\n", si->dma->channels - 1);
	si->Com_Printf("Samples: %d\n", si->dma->samples);
	si->Com_Printf("Samplepos: %d\n", si->dma->samplepos);
	si->Com_Printf("Samplebits: %d\n", si->dma->samplebits);
	si->Com_Printf("Submission_chunk: %d\n", si->dma->submission_chunk);
	si->Com_Printf("Speed: %d\n", si->dma->speed);
	
	si->dma->samplepos = 0;

	snd_inited = 1;
	return 1;
}

int SNDDMA_GetDMAPos(void)
{
	struct count_info count;

	if (!snd_inited) return 0;

	if (ioctl(audio_fd, SNDCTL_DSP_GETOPTR, &count)==-1)
	{
		perror(si->snddevice->string);
		si->Com_Printf("SNDDMA_GetDMAPos: GETOPTR failed.\n");
		close(audio_fd);
		audio_fd = -1;
		snd_inited = 0;
		return 0;
	}
//	si->dma->samplepos = (count.bytes / (si->dma->samplebits / 8)) & (si->dma->samples-1);
//	fprintf(stderr, "%d    \r", count.ptr);
	si->dma->samplepos = count.ptr / (si->dma->samplebits / 8);

	return si->dma->samplepos;
}

void SNDDMA_Shutdown(void)
{
#if 0
	if (snd_inited)
	{
		close(audio_fd);
		audio_fd = -1;
		snd_inited = 0;
	}
#endif
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit(void)
{
}

void SNDDMA_BeginPainting (void)
{
}

