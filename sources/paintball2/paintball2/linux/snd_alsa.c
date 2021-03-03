/*
	snd_alsa.c

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

	$Id: snd_alsa.c,v 1.4 2006/06/30 19:45:03 qudos Exp $
*/

#include <alsa/asoundlib.h>

#include "../client/client.h"
#include "../client/snd_loc.h"

static int  snd_inited;

static snd_pcm_t *playback_handle;
static snd_pcm_hw_params_t *hw_params;

/*
* These are reasonable default values for good latency.  If ALSA
* playback stutters or plain does not work, try adjusting these.
* Period must always be a multiple of 2.  Buffer must always be
* a multiple of period.  See http://alsa-project.org.
*/
static snd_pcm_uframes_t period_size = 2048;
static snd_pcm_uframes_t buffer_size = 8192;

static int sample_bytes;
static int buffer_bytes;

static int tryrates[] = { 48000, 44100, 22050, 11025, 8000 };

struct sndinfo *si;

qboolean SNDDMA_Init (struct sndinfo *s)
{
  int i;
  int err;
  int format;

  si = s;

  if (snd_inited){ // jitlinux
	return 1;
  }
	
  if(!strcmp(si->snddevice->string, "/dev/dsp"))
	si->snddevice->string = "default";
  
  err = snd_pcm_open(&playback_handle, si->snddevice->string, 
		     SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
  if (err < 0) {
    si->Com_Printf("ALSA snd error, cannot open device %s (%s)\n", 
	       si->snddevice->string,
	       snd_strerror(err));
    return 0;
  }
  
  err = snd_pcm_hw_params_malloc(&hw_params);

  if (err < 0) {
    si->Com_Printf("ALSA snd error, cannot allocate hw params (%s)\n",
	       snd_strerror(err));
    return 0;
  }
  
  err = snd_pcm_hw_params_any (playback_handle, hw_params);
  if (err < 0) {
    si->Com_Printf("ALSA snd error, cannot init hw params (%s)\n",
	       snd_strerror(err));
    snd_pcm_hw_params_free(hw_params);
    return 0;
  }
  
  err = snd_pcm_hw_params_set_access(playback_handle, hw_params, 
				     SND_PCM_ACCESS_RW_INTERLEAVED);
  if (err < 0) {
    si->Com_Printf("ALSA snd error, cannot set access (%s)\n",
	       snd_strerror(err));
    snd_pcm_hw_params_free(hw_params);
    return 0;
  }
  
  si->dma->samplebits = si->sndbits->value;
  if (si->dma->samplebits == 16 || si->dma->samplebits != 8) {
    err = snd_pcm_hw_params_set_format(playback_handle, hw_params,
				       SND_PCM_FORMAT_S16_LE);
    if (err < 0) {
      si->Com_Printf("ALSA snd error, 16 bit sound not supported, trying 8\n");
      si->dma->samplebits = 8;
    }
    else {
      format = SND_PCM_FORMAT_S16_LE;
    }
  }
  if (si->dma->samplebits == 8) {
    err = snd_pcm_hw_params_set_format(playback_handle, hw_params,
				       SND_PCM_FORMAT_U8);
  }
  if (err < 0) {
    si->Com_Printf("ALSA snd error, cannot set sample format (%s)\n",
	       snd_strerror(err));
    snd_pcm_hw_params_free(hw_params);
    return 0;
  }
  else {
    format = SND_PCM_FORMAT_U8;
  }
  
  si->dma->speed = si->sndspeed->value;
  if (!si->dma->speed) {
    for (i=0 ; i<sizeof(tryrates); i++) {
      unsigned int test = tryrates[i];
      err = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params,
					    &test, 0);
      if (err < 0) {
	si->Com_Printf("ALSA snd error, cannot set sample rate %d (%s)\n",
		   tryrates[i], snd_strerror(err));
      }
      else {
	si->dma->speed = test;
	break;
      }
    }
  }
  if (!si->dma->speed) {
    si->Com_Printf("ALSA snd error couldn't set rate.\n");
    snd_pcm_hw_params_free(hw_params);
    return 0;
  }

  si->dma->channels = si->sndchannels->value;
  if (si->dma->channels < 1 || si->dma->channels > 2)
    si->dma->channels = 2;
  err = snd_pcm_hw_params_set_channels(playback_handle,hw_params,si->dma->channels);
  if (err < 0) {
    si->Com_Printf("ALSA snd error couldn't set channels %d (%s).\n",
	       si->dma->channels, snd_strerror(err));
    snd_pcm_hw_params_free(hw_params);
    return 0;
  }
  
  if((err = snd_pcm_hw_params_set_period_size_near(playback_handle, 
			hw_params, &period_size, 0)) < 0){
		si->Com_Printf("ALSA: cannot set period size near(%s)\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return 0;
	}
	
	if((err = snd_pcm_hw_params_set_buffer_size_near(playback_handle, 
			hw_params, &buffer_size)) < 0){
		si->Com_Printf("ALSA: cannot set buffer size near(%s)\n", snd_strerror(err));
		snd_pcm_hw_params_free(hw_params);
		return 0;
	}
  
  err = snd_pcm_hw_params(playback_handle, hw_params);
  if (err < 0) {
    si->Com_Printf("ALSA snd error couldn't set params (%s).\n",snd_strerror(err));
    snd_pcm_hw_params_free(hw_params);
    return 0;
  }

  	sample_bytes = si->dma->samplebits / 8;
	buffer_bytes = buffer_size * si->dma->channels * sample_bytes;
	
	si->dma->buffer = malloc(buffer_bytes);  //allocate pcm frame buffer
	memset(si->dma->buffer, 0, buffer_bytes);
	
	si->dma->samples = buffer_size * si->dma->channels;
	si->dma->submission_chunk = period_size * si->dma->channels;
	
	si->dma->samplepos = 0;
	

	si->Com_Printf("Initializing ALSA Sound System\n");
	si->Com_Printf("Period size: %d\n", period_size);
	si->Com_Printf("Buffer size: %d\n", buffer_size);
	si->Com_Printf("Stereo: %d\n", si->dma->channels - 1);
	si->Com_Printf("Samples: %d\n", si->dma->samples);
	si->Com_Printf("Samplepos: %d\n", si->dma->samplepos);
	si->Com_Printf("Samplebits: %d\n", si->dma->samplebits);
	si->Com_Printf("Submission_chunk: %d\n", si->dma->submission_chunk);
	si->Com_Printf("Speed: %d\n", si->dma->speed);
	
	snd_pcm_prepare(playback_handle);
	snd_inited = 1;
	return 1;
}

int
SNDDMA_GetDMAPos (void)
{
  if(snd_inited)
    return si->dma->samplepos;
  else
    si->Com_Printf ("Sound not inizialized\n");
  return 0;
}

void
SNDDMA_Shutdown (void)
{
  if (snd_inited) {
    snd_pcm_drop(playback_handle);
    snd_pcm_close(playback_handle);
    snd_inited = 0;
  }
  free(si->dma->buffer);
  si->dma->buffer = NULL;
}

/*
  SNDDMA_Submit
Send sound to device if buffer isn't really the dma buffer
*/
void
SNDDMA_Submit (void)
{
  int s, w, frames;
	void *start;
	
	if(!snd_inited)
		return;
		
	s = si->dma->samplepos * sample_bytes;
	start = (void *) &si->dma->buffer[s];
	
	frames = si->dma->submission_chunk / si->dma->channels;
	
	if((w = snd_pcm_writei(playback_handle, start, frames)) < 0){  //write to card
		snd_pcm_prepare(playback_handle);  //xrun occured
		return;
	}
	
	si->dma->samplepos += w * si->dma->channels;  //mark progress
	
	if(si->dma->samplepos >= si->dma->samples)
		si->dma->samplepos = 0;  //wrap buffer
}


void SNDDMA_BeginPainting(void)
{    
}
