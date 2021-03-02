#include "../client/client.h"
//#include "../client/sound.h"
#include "../client/snd_loc.h"
#ifndef WIN32
#include <dlfcn.h>
#endif
cvar_t	*s_a3d;
s_a3d_t a3d;
int a3dsound_started=0;
void S_Q2A3DPCMStream (int samples, int rate, int width, int channels, byte *data)
{
	a3d.A3D_PCMStream (samples, rate, width, channels, data);
}
void S_Q2A3DCloseLibrary(void)
{
	if (a3d.reflib) {
		Com_Printf("Freeing q2a3d Module.\n");
#ifdef __unix__
		dlclose(a3d.reflib);
#else
		FreeLibrary(a3d.reflib);
#endif
		a3d.reflib=NULL;
	}
	a3d.A3D_OggStreamStart = NULL;
	a3d.A3D_OggStreamStop = NULL;
	a3d.A3D_PCMStream = NULL;
	a3d.A3D_Update = NULL;
	a3d.Init_A3D = NULL;
	a3d.A3D_Shutdown = NULL;
	a3d.A3D_cachefile = NULL;
	a3d.A3D_StartSound = NULL;
	a3d.A3D_UpdateSound = NULL;
	a3d.A3D_StopSound = NULL;
	a3d.A3D_SetEnv = NULL;
}
void S_Q2A3DInit (void)
{
#ifdef WIN32
	const char *enginename = "q2a3d.dll";
	char name[MAX_OSPATH];
	char *path;
	int loadLibError;
#endif

	if (a3dsound_started)
		return;//already enabled?
	if (!a3d.reflib) {
	#ifndef WIN32
		a3d.reflib = dlopen("q2a3d.so", RTLD_LAZY);
		
		if (a3d.reflib)
			Com_Printf("Loaded q2a3d.so...\n");
		else
			Com_Printf("dlopen(\"q2a3d.so\") failed: %s\n", dlerror());
	#else
		path = NULL;
		Com_sprintf(name, sizeof(name), "%s", enginename);
		a3d.reflib = LoadLibrary(name);
		if (a3d.reflib)
			Com_Printf("LoadLibrary (%s).\n", name);
		else
			Com_Printf("LoadLibrary (%s) Failed.\n", name);

		
	#endif
	}
	if (a3d.reflib) {
#ifdef __unix__
		a3d.A3D_OggStreamStart = (void *) dlsym(a3d.reflib, "A3D_OggStreamStart");
		a3d.A3D_OggStreamStop = (void *) dlsym(a3d.reflib, "A3D_OggStreamStop");
		a3d.A3D_PCMStream = (void *) dlsym(a3d.reflib, "A3D_PCMStream");
		a3d.A3D_Update = (void *) dlsym(a3d.reflib, "A3D_Update");
		a3d.Init_A3D = (void *) dlsym(a3d.reflib, "Init_A3D");
		a3d.A3D_Shutdown = (void *) dlsym(a3d.reflib, "A3D_Shutdown");
		a3d.A3D_cachefile = (void *) dlsym(a3d.reflib, "A3D_cachefile");
		a3d.A3D_StartSound = (void *) dlsym(a3d.reflib, "A3D_StartSound");
		a3d.A3D_UpdateSound = (void *) dlsym(a3d.reflib, "A3D_UpdateSound");
		a3d.A3D_StopSound = (void *) dlsym(a3d.reflib, "A3D_StopSound");
		a3d.A3D_SetEnv = (void *) dlsym(a3d.reflib, "A3D_SetEnv");
		
#else
		a3d.A3D_OggStreamStart = (void *) GetProcAddress(a3d.reflib, "A3D_OggStreamStart");
		a3d.A3D_OggStreamStop = (void *) GetProcAddress(a3d.reflib, "A3D_OggStreamStop");
		a3d.A3D_PCMStream = (void *) GetProcAddress(a3d.reflib, "A3D_PCMStream");
		a3d.A3D_Update = (void *) GetProcAddress(a3d.reflib, "A3D_Update");
		a3d.Init_A3D = (void *) GetProcAddress(a3d.reflib, "Init_A3D");
		a3d.A3D_Shutdown = (void *) GetProcAddress(a3d.reflib, "A3D_Shutdown");
		a3d.A3D_cachefile = (void *) GetProcAddress(a3d.reflib, "A3D_cachefile");
		a3d.A3D_StartSound = (void *) GetProcAddress(a3d.reflib, "A3D_StartSound");
		a3d.A3D_UpdateSound = (void *) GetProcAddress(a3d.reflib, "A3D_UpdateSound");
		a3d.A3D_StopSound = (void *) GetProcAddress(a3d.reflib, "A3D_StopSound");
		a3d.A3D_SetEnv = (void *) GetProcAddress(a3d.reflib, "A3D_SetEnv");
		if (!a3d.A3D_OggStreamStart || !a3d.A3D_OggStreamStop || !a3d.A3D_PCMStream
			|| !a3d.A3D_Update || !a3d.Init_A3D || !a3d.A3D_Shutdown
			|| !a3d.A3D_cachefile || !a3d.A3D_StartSound || !a3d.A3D_UpdateSound
			|| !a3d.A3D_StopSound || !a3d.A3D_SetEnv) {
			Com_Printf("Error while loading q2a3d module.\n");
			S_Q2A3DCloseLibrary();
			return;
		}

#endif
	}
	else
	{
#ifdef _WIN32
		LPVOID lpMsgBuf;

		Com_Printf("Error loading up q2a3d module.\n");
		loadLibError = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, loadLibError, 
		               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
		Com_Printf("%s\n", lpMsgBuf);

#else
		Com_Printf("Error loading up q2a3d module.\n");
#endif
		S_Q2A3DCloseLibrary();
		return;
	}

	if (a3d.Init_A3D() > 1) //initialise A3D
		Com_Printf("\nEAX Extenstions Available\nA3D initialised.\n");//engine returned EAX available
	else
		Com_Printf("\nA3D initialised.\n");

	a3dsound_started = 1;//set the enabled flag
}//initialize the A3D engine

sfx_t *S_Q2A3DRegisterSound (const char *name)
{
	sfx_t	*sfx;
	sfx = S_FindName (name, true);
	//Com_Printf("%s and %s\n",name,sfx->name);
	return sfx;
}//sound effect name registration

channel_t *S_Q2A3DPickChannel(int entnum, int entchannel)
{
	int			ch_idx;
	channel_t	*ch;
	for (ch_idx=0 ; ch_idx < MAX_CHANNELS ; ch_idx++)
	{
		if (channels[ch_idx].autosound)
			continue;
		ch = &channels[ch_idx];
		memset (ch, 0, sizeof(*ch));
		return ch;
	}
	return NULL;
}//pick a channel for looping sound (renewed every frame)
void S_Q2A3DStartSound(vec3_t origin, int entnum, int entchannel, sfx_t *sfx, float fvol, float attenuation, float timeofs)
{
	int debugbuff,water=0,cont;
	centity_t	*old;
	char *name;
	char	namebuffer[MAX_QPATH];
	trace_t			tr;
	vec3_t sound_origin={0,0,0},head_origin;
	byte	*wavfile;
	float atten;
	if (sfx->truename)
		name = sfx->truename;
	else
		name = sfx->name;
	
	if (name[0] == '#')
		Com_sprintf (namebuffer, sizeof(namebuffer), &name[1]);
	else
		Com_sprintf (namebuffer, sizeof(namebuffer), "sound/%s", name);//affix directory structure
	if (origin)
	{
		VectorCopy (origin, sound_origin);//set its origin
	}
	else if (entnum>1 && entnum != cl.playernum+1)
	{
		old = &cl_entities[entnum];
		VectorCopy (old->lerp_origin, sound_origin);
	}//entity making the sound? if so get its origin
	
	cont = CM_PointContents(listener_origin,0);
	
	if (cont & MASK_WATER)
		water=2;//is this an underwater sound (EAX extentions)
	//set up obstructions
	VectorCopy (listener_origin, head_origin);
	head_origin[2]+=42;
	tr = CM_BoxTrace (head_origin,sound_origin, vec3_origin, vec3_origin,0,  MASK_SOLID | MASK_WATER);
#if 0 // jitsound
	if (sound_origin[2])
		if (entchannel==CHAN_WEAPON)
			atten=0.2+(tr.fraction/*attenuation*0.8*/);//attenuation > = quieter
		else if (attenuation==ATTN_NONE)
		{
			VectorCopy(head_origin, sound_origin); // jitsound
			atten=1;
		}
		else
			atten=(tr.fraction/*attenuation*0.8*/)-0.15;
	else	
		atten=0.9/*attenuation*/;//attenuation > = quieter
	/*if(sound_origin[2])
		atten=0.5+(tr.fraction/*attenuation*0.3);//attenuation > = quieter
	else
		atten=0.5/*attenuation*/;//attenuation > = quieter*/
#else // jitsound:
	if (attenuation == ATTN_NONE) // sound heard no matter what
	{
		atten = 1;
		VectorCopy(head_origin, sound_origin);
		water = 0;
	}
	else
	{
		atten = 3.0f / attenuation;

		if (tr.fraction != 1.0f) // blocked
			atten *= (1.0f - tr.fraction);
	}
#endif
	//end set up obstructions.
	//if(sfx->Id)
	//	Com_Printf("%s %f %f %d  %d %f\n",namebuffer,sound_origin[0],sound_origin[1],water,sfx->Id,atten);
	
	if (debugbuff = a3d.A3D_StartSound(sound_origin, listener_forward,listener_origin,namebuffer,atten,s_volume->value,sfx->Id,water))
		return;

	//if startsound failed, the A3D engine doesn't have it cahched
	//so get a handle to the file and cache it.
	if (!a3d.A3D_cachefile(namebuffer,NULL))//try and reassign it to a buffer
	{
		FS_LoadFile(namebuffer, (void **)&wavfile);

		if (!wavfile)
		{
			a3d.A3D_StartSound(sound_origin, listener_forward,listener_origin,namebuffer,atten,s_volume->value,sfx->Id,0);
			return;//couldn't find, see if startsound can handle it (e.g. ogg files)
		}

		if (!a3d.A3D_cachefile(namebuffer,wavfile))
		{
			FS_FreeFile(wavfile);//if we fail to cache, free it for use later
			return; // jit
		}

		//then try to start it again
		if (debugbuff = a3d.A3D_StartSound(sound_origin, listener_forward,listener_origin,namebuffer,atten,s_volume->value,sfx->Id,0))
		{
			FS_FreeFile(wavfile);//its now in the cache and nolonger needed
			return;
		}

		FS_FreeFile(wavfile);//just in case it failed, free the file
	}

	a3d.A3D_StartSound(sound_origin, listener_forward, listener_origin, namebuffer, atten, s_volume->value, sfx->Id, 0);

	//else we failed, play it as a normal sound (should never get here)
	return;
}

void S_Q2A3DAddLoopSounds (void)
{
	int			i;
	int			sounds[MAX_EDICTS];
	channel_t	*ch;
	sfx_t		*sfx;
	int			num;
	entity_state_t	*ent;
	//extern struct model_s	*cl_mod_paintball; // jitsound
	//extern struct sfx_s	*cl_sfx_paintfly[3]; // jitsound
	//int flysound;

	
	for (i=0 ; i<cl.frame.num_entities ; i++)
	{
		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];
		sounds[i] = ent->sound;
	}//check for any entity sounds

	for (i=0 ; i<cl.frame.num_entities ; i++)
	{
		/*if(cl.model_draw[ent->modelindex] == cl_mod_paintball) // jitsound -- add sounds to paintballs
			flysound = rand() % 3 + 1;
		else*/ if (!sounds[i])
			continue;

		/*if(flysound) // jitsound
			sfx = cl_sfx_paintfly[1];
		else*/
			sfx = cl.sound_precache[sounds[i]];
		if (!sfx)
			continue;		// bad sound effect
		//Com_Printf("%s\n",sfx->name);
		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];

		ch = S_Q2A3DPickChannel(0, 0);//found one, assign it to a channel
		if (!ch)
			return;


		ch->autosound = true;	// removed next frame, enable this channel
		ch->sfx = sfx;//set the name

		if (ent)
			VectorCopy(ent->origin,ch->origin);//copy in this sounds position
	}
}//adds continious looping sounds

void S_Q2A3DUpdate(vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
{
	trace_t			tr;
	float atten;
	char *name;
	vec3_t head_origin;
	char	namebuffer[MAX_QPATH];
	int			i,speed;
	int			total;
	channel_t	*ch;
	VectorCopy(origin, listener_origin);
	VectorCopy(forward, listener_forward);
	VectorCopy(right, listener_right);
	VectorCopy(up, listener_up);
	
	a3d.A3D_Update(origin,forward);//update audio streams (ogg music and cinematic audio)

	ch = channels;
	for (i=0 ; i<MAX_CHANNELS; i++, ch++)
	{
		if (!ch->sfx)
			continue;
		if (ch->autosound)
		{	// autosounds are regenerated fresh each frame
			memset (ch, 0, sizeof(*ch));
			continue;
		}
	}//clear all the channels

	S_Q2A3DAddLoopSounds ();//reassign for this frame

	total = 0;
	ch = channels;
	for (i=0 ; i<MAX_CHANNELS; i++, ch++)
	{
		if (ch->sfx )
		{
			atten=0;
			if (ch->autosound)
			{
				
				//set up its traceable ID.
				
				//set up obstruction
				VectorCopy (listener_origin, head_origin);
				head_origin[2]+=42;
				tr = CM_BoxTrace (head_origin,ch->origin, vec3_origin, vec3_origin,0,  MASK_SOLID | MASK_WATER);//CL_PMSurfaceTrace (listener_origin, NULL, NULL, ch->origin, MASK_SOLID | MASK_WATER);
				atten=0.5+(tr.fraction*0.3);//allow propagation for map sounds
				//end set up obstructions.
				ch->sfx->Id = i+1;
				
				if (atten<0.1)
				{
					a3d.A3D_StopSound(i+1);
					continue;
				}
				
				if (ch->sfx->truename)
					name = ch->sfx->truename;
				else
					name = ch->sfx->name;
				
				if (name[0] == '#')
					Com_sprintf (namebuffer, sizeof(namebuffer), &name[1]);
				else
					Com_sprintf (namebuffer, sizeof(namebuffer), "sound/%s", name);
				if (!(speed = a3d.A3D_UpdateSound(ch->sfx->Id,namebuffer,ch->origin, listener_forward,listener_origin,atten)) && ch->sfx->Id)
				{
					//Com_Printf("failed %d %s\n",i+1,ch->sfx->name);
					S_Q2A3DStartSound (ch->origin, 0, 0, ch->sfx, 1, ATTN_NORM, 0);//if we cant update it, its because it hasn't been started
				}
				//if(speed!=1 && speed!=0)
				//	Com_Printf ("%i %i\n",i,speed);
			}
			
			//Com_Printf ("%3i %3i %s %d %f\n", ch->leftvol, ch->rightvol, ch->sfx->name,ch->sfx->Id,atten);
			//total++;
		}
		else
			a3d.A3D_StopSound(i+1);//otherwise make sure this channel is reset
	}

	

	if (s_show->value)
	{
		total = 0;
		ch = channels;
		for (i=0 ; i<MAX_CHANNELS; i++, ch++)
			if (ch->sfx)
			{
				Com_Printf ("%s = %d\n", ch->sfx->name,i);
				total++;
			}
	}
}//update the audio environment with continious loop sounds
void S_Q2A3DStopAllSounds(void)
{
	int i;

	a3d.A3D_OggStreamStop();//stop any ogg stream
	// clear all the channels
	for (i=0 ; i<MAX_CHANNELS; i++)
		a3d.A3D_StopSound(i+1);
	memset(channels, 0, sizeof(channels));
}//stop and reset all A3D channels.
