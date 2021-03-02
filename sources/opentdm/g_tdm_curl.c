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

//Curl interface functions. OpenTDM can use libcurl to fetch and POST to the
//website, used for downloading configs and uploading stats (todo).

#include "g_local.h"
#include "g_tdm.h"

#ifdef HAVE_CURL
#define CURL_STATICLIB
#ifdef _WIN32
#define CURL_CALLING_CONVENTION __cdecl
#endif
#include <curl/curl.h>

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

typedef struct dlhandle_s
{
	CURL			*curl;
	size_t			fileSize;
	size_t			position;
	double			speed;
	char			filePath[1024];
	char			URL[2048];
	char			*tempBuffer;
	qboolean		inuse;
	tdm_download_t	*tdm_handle;
} dlhandle_t;

//we need this high in case a sudden server switch causes a bunch of people
//to connect, we want to be able to download their configs
#define MAX_DOWNLOADS	16

dlhandle_t	downloads[MAX_DOWNLOADS];

static CURLM				*multi = NULL;
static unsigned				handleCount = 0;

static char					otdm_api_ip[16];
static char					hostHeader[64];
static struct curl_slist	*http_header_slist;

static time_t				last_dns_lookup;

/*
===============
HTTP_EscapePath

Properly escapes a path with HTTP %encoding. libcurl's function
seems to treat '/' and such as illegal chars and encodes almost
the entire URL...
===============
*/
static void HTTP_EscapePath (const char *filePath, char *escaped)
{
	int		i;
	size_t	len;
	char	*p;

	p = escaped;

	len = strlen (filePath);
	for (i = 0; i < len; i++)
	{
		if (!isalnum (filePath[i]) && filePath[i] != ';' && filePath[i] != '/' &&
			filePath[i] != '?' && filePath[i] != ':' && filePath[i] != '@' && filePath[i] != '&' &&
			filePath[i] != '=' && filePath[i] != '+' && filePath[i] != '$' && filePath[i] != ',' &&
			filePath[i] != '[' && filePath[i] != ']' && filePath[i] != '-' && filePath[i] != '_' &&
			filePath[i] != '.' && filePath[i] != '!' && filePath[i] != '~' && filePath[i] != '*' &&
			filePath[i] != '\'' && filePath[i] != '(' && filePath[i] != ')')
		{
			sprintf (p, "%%%02x", filePath[i]);
			p += 3;
		}
		else
		{
			*p = filePath[i];
			p++;
		}
	}
	p[0] = 0;

	//using ./ in a url is legal, but all browsers condense the path and some IDS / request
	//filtering systems act a bit funky if http requests come in with uncondensed paths.
	len = strlen(escaped);
	p = escaped;
	while ((p = strstr (p, "./")))
	{
		memmove (p, p+2, len - (p - escaped) - 1);
		len -= 2;
	}
}

/*
===============
HTTP_Recv

libcurl callback.
===============
*/
static size_t EXPORT HTTP_Recv (void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t		bytes;
	dlhandle_t	*dl;

	dl = (dlhandle_t *)stream;

	bytes = size * nmemb;

	if (!dl->fileSize)
	{
		dl->fileSize = bytes > 131072 ? bytes : 131072;
		dl->tempBuffer = gi.TagMalloc ((int)dl->fileSize, TAG_GAME);
	}
	else if (dl->position + bytes > dl->fileSize)
	{
		char		*tmp;

		tmp = dl->tempBuffer;

		dl->tempBuffer = gi.TagMalloc ((int)(dl->fileSize*2), TAG_GAME);
		memcpy (dl->tempBuffer, tmp, dl->fileSize);
		gi.TagFree (tmp);
		dl->fileSize *= 2;
	}

	memcpy (dl->tempBuffer + dl->position, ptr, bytes);
	dl->position += bytes;
	dl->tempBuffer[dl->position] = 0;

	return bytes;
}

/*
===============
HTTP_Header

libcurl callback to update header info.
===============
*/
static size_t EXPORT HTTP_Header (void *ptr, size_t size, size_t nmemb, void *stream)
{
	char	headerBuff[1024];
	size_t	bytes;
	size_t	len;

	bytes = size * nmemb;

	if (bytes <= 16)
		return bytes;

	if (bytes < sizeof(headerBuff)-1)
		len = bytes;
	else
		len = sizeof(headerBuff)-1;

	Q_strncpy (headerBuff, ptr, len);

	if (!Q_strncasecmp (headerBuff, "Content-Length: ", 16))
	{
		dlhandle_t	*dl;

		dl = (dlhandle_t *)stream;

		//allocate buffer based on what the server claims content-length is. +1 for nul
		dl->fileSize = strtoul (headerBuff + 16, NULL, 10);
		if (!dl->tempBuffer)
		{
			if (dl->fileSize < 1048576)
				dl->tempBuffer = gi.TagMalloc (dl->fileSize + 1, TAG_GAME);
			else
				gi.dprintf ("Suspiciously large file while trying to download %s!\n", dl->URL);
		}
	}

	return bytes;
}

int EXPORT CURL_Debug (CURL *c, curl_infotype type, char *data, size_t size, void * ptr)
{
	if (type == CURLINFO_TEXT)
	{
		char	buff[4096];
		if (size > sizeof(buff)-1)
			size = sizeof(buff)-1;
		Q_strncpy (buff, data, size);
		gi.dprintf ("  OpenTDM HTTP DEBUG: %s", buff);
		if (!strchr (buff, '\n'))
			gi.dprintf ("\n");
	}

	return 0;
}

/*
===============
HTTP_ResolveOTDMServer

Resolve the g_http_domain and cache it, so we don't do DNS
lookups at critical times (eg mid match).
===============
*/
void HTTP_ResolveOTDMServer (void)
{
	if (!g_http_enabled->value)
		return;

	//re-resolve if its been more than one day since we last did it
	if (time(NULL) - last_dns_lookup > 86400)
	{
		struct hostent	*h;
		h = gethostbyname (g_http_domain->string);

		if (!h)
		{
			otdm_api_ip[0] = '\0';
			gi.dprintf ("WARNING: Could not resolve OpenTDM web API server '%s'. HTTP functions unavailable.\n", g_http_domain->string);
			return;
		}

		time (&last_dns_lookup);

		Q_strncpy (otdm_api_ip, inet_ntoa (*(struct in_addr *)h->h_addr_list[0]), sizeof(otdm_api_ip)-1);
	}
}

/*
===============
HTTP_StartDownload

Actually starts a download by adding it to the curl multi
handle.
===============
*/
void HTTP_StartDownload (dlhandle_t *dl)
{
	cvar_t				*hostname;
	char				escapedFilePath[2048];

	hostname = gi.cvar ("hostname", NULL, 0);
	if (!hostname)
		TDM_Error ("HTTP_StartDownload: Couldn't get hostname cvar");

	dl->tempBuffer = NULL;
	dl->speed = 0;
	dl->fileSize = 0;
	dl->position = 0;

	if (!dl->curl)
		dl->curl = curl_easy_init ();

	HTTP_EscapePath (dl->filePath, escapedFilePath);

	Com_sprintf (dl->URL, sizeof(dl->URL), "http://%s%s%s", otdm_api_ip, g_http_path->string, escapedFilePath);

	curl_easy_setopt (dl->curl, CURLOPT_HTTPHEADER, http_header_slist);	
	curl_easy_setopt (dl->curl, CURLOPT_ENCODING, "");
	curl_easy_setopt (dl->curl, CURLOPT_DEBUGFUNCTION, CURL_Debug);
	curl_easy_setopt (dl->curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt (dl->curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt (dl->curl, CURLOPT_WRITEDATA, dl);
	if (g_http_bind->string[0])
		curl_easy_setopt (dl->curl, CURLOPT_INTERFACE, g_http_bind->string);
	else
		curl_easy_setopt (dl->curl, CURLOPT_INTERFACE, NULL);

	curl_easy_setopt (dl->curl, CURLOPT_WRITEFUNCTION, HTTP_Recv);

	if (g_http_proxy->string[0])
		curl_easy_setopt (dl->curl, CURLOPT_PROXY, g_http_proxy->string);
	else
		curl_easy_setopt (dl->curl, CURLOPT_PROXY, NULL);
	curl_easy_setopt (dl->curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt (dl->curl, CURLOPT_MAXREDIRS, 5);
	curl_easy_setopt (dl->curl, CURLOPT_WRITEHEADER, dl);
	curl_easy_setopt (dl->curl, CURLOPT_HEADERFUNCTION, HTTP_Header);
	curl_easy_setopt (dl->curl, CURLOPT_PROGRESSDATA, dl);
	curl_easy_setopt (dl->curl, CURLOPT_USERAGENT, "OpenTDM (" OPENTDM_VERSION ")");
	curl_easy_setopt (dl->curl, CURLOPT_REFERER, hostname->string);
	curl_easy_setopt (dl->curl, CURLOPT_URL, dl->URL);

	if (curl_multi_add_handle (multi, dl->curl) != CURLM_OK)
	{
		gi.dprintf ("HTTP_StartDownload: curl_multi_add_handle: error\n");
		return;
	}

	handleCount++;
}

/*
===============
HTTP_Init

Init libcurl.
===============
*/
void HTTP_Init (void)
{
	curl_global_init (CURL_GLOBAL_NOTHING);
	multi = curl_multi_init ();

	Com_sprintf (hostHeader, sizeof(hostHeader), "Host: %s", g_http_domain->string);
	http_header_slist = curl_slist_append (http_header_slist, hostHeader);

	gi.dprintf ("%s initialized.\n", curl_version());
}

void HTTP_Shutdown (void)
{
	if (multi)
	{
		curl_multi_cleanup (multi);
		multi = NULL;
	}

	curl_slist_free_all (http_header_slist);

	curl_global_cleanup ();
}

/*
===============
CL_FinishHTTPDownload

A download finished, find out what it was, whether there were any errors and
if so, how severe. If none, rename file and other such stuff.
===============
*/
static void HTTP_FinishDownload (void)
{
	int			msgs_in_queue;
	CURLMsg		*msg;
	CURLcode	result;
	dlhandle_t	*dl;
	CURL		*curl;
	long		responseCode;
	double		timeTaken;
	double		fileSize;
	unsigned	i;

	do
	{
		msg = curl_multi_info_read (multi, &msgs_in_queue);

		if (!msg)
		{
			gi.dprintf ("HTTP_FinishDownload: Odd, no message for us...\n");
			return;
		}

		if (msg->msg != CURLMSG_DONE)
		{
			gi.dprintf ("HTTP_FinishDownload: Got some weird message...\n");
			continue;
		}

		curl = msg->easy_handle;

		for (i = 0; i < MAX_DOWNLOADS; i++)
		{
			if (downloads[i].curl == curl)
				break;
		}

		if (i == MAX_DOWNLOADS)
			TDM_Error ("HTTP_FinishDownload: Handle not found!");

		dl = &downloads[i];

		result = msg->data.result;

		switch (result)
		{
			//for some reason curl returns CURLE_OK for a 404...
			case CURLE_HTTP_RETURNED_ERROR:
			case CURLE_OK:
			
				curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &responseCode);
				if (responseCode == 404)
				{
					TDM_HandleDownload (dl->tdm_handle, NULL, 0, responseCode);
					gi.dprintf ("HTTP: %s: 404 File Not Found\n", dl->URL);
					curl_multi_remove_handle (multi, dl->curl);
					dl->inuse = false;
					continue;
				}
				else if (responseCode == 200)
				{
					TDM_HandleDownload (dl->tdm_handle, dl->tempBuffer, dl->position, responseCode);
					gi.TagFree (dl->tempBuffer);
				}
				else
				{
					TDM_HandleDownload (dl->tdm_handle, NULL, 0, responseCode);
					if (dl->tempBuffer)
						gi.TagFree (dl->tempBuffer);
				}
				break;

			//fatal error
			default:
				TDM_HandleDownload (dl->tdm_handle, NULL, 0, 0);
				gi.dprintf ("HTTP Error: %s: %s\n", dl->URL, curl_easy_strerror (result));
				curl_multi_remove_handle (multi, dl->curl);
				dl->inuse = false;
				continue;
		}

		//show some stats
		curl_easy_getinfo (curl, CURLINFO_TOTAL_TIME, &timeTaken);
		curl_easy_getinfo (curl, CURLINFO_SIZE_DOWNLOAD, &fileSize);

		//FIXME:
		//technically i shouldn't need to do this as curl will auto reuse the
		//existing handle when you change the URL. however, the handleCount goes
		//all weird when reusing a download slot in this way. if you can figure
		//out why, please let me know.
		curl_multi_remove_handle (multi, dl->curl);

		dl->inuse = false;

		gi.dprintf ("HTTP: Finished %s: %.f bytes, %.2fkB/sec\n", dl->URL, fileSize, (fileSize / 1024.0) / timeTaken);
	} while (msgs_in_queue > 0);
}

qboolean HTTP_QueueDownload (tdm_download_t *d)
{
	unsigned	i;

	if (handleCount == MAX_DOWNLOADS)
	{
		if (d->type == DL_CONFIG)
			gi.cprintf (d->initiator, PRINT_HIGH, "Another download is already pending, please try again later.\n");
		return false;
	}

	if (!g_http_enabled->value)
	{
		if (d->type == DL_CONFIG)
			gi.cprintf (d->initiator, PRINT_HIGH, "HTTP functions are disabled on this server.\n");
		return false;
	}

	if (!otdm_api_ip[0])
	{
		if (d->type == DL_CONFIG)
			gi.cprintf (d->initiator, PRINT_HIGH, "This server failed to resolve the OpenTDM web API server.\n");
		return false;
	}

	for (i = 0; i < MAX_DOWNLOADS; i++)
	{
		if (!downloads[i].inuse)
			break;
	}

	if (i == MAX_DOWNLOADS)
	{
		if (d->type == DL_CONFIG)
			gi.cprintf (d->initiator, PRINT_HIGH, "The server is too busy to download configs right now.\n");
		return false;
	}

	downloads[i].tdm_handle = d;
	downloads[i].inuse = true;
	strncpy (downloads[i].filePath, d->path, sizeof(downloads[i].filePath)-1);
	HTTP_StartDownload (&downloads[i]);

	return true;
}

/*
===============
HTTP_RunDownloads

This calls curl_multi_perform to actually do stuff. Called every frame to process
downloads.
===============
*/
void HTTP_RunDownloads (void)
{
	int			newHandleCount;
	CURLMcode	ret;

	//nothing to do!
	if (!handleCount)
		return;

	do
	{
		ret = curl_multi_perform (multi, &newHandleCount);
		if (newHandleCount < handleCount)
		{
			HTTP_FinishDownload ();
			handleCount = newHandleCount;
		}
	} while (ret == CURLM_CALL_MULTI_PERFORM);

	if (ret != CURLM_OK)
	{
		gi.dprintf ("HTTP_RunDownloads: curl_multi_perform error.\n");
	}
}
#else
void HTTP_RunDownloads (void)
{
}

void HTTP_Init (void)
{
	gi.dprintf ("WARNING: OpenTDM was built without libcurl. Some features will be unavailable.\n");
}

qboolean HTTP_QueueDownload (tdm_download_t *d)
{
	if (d->type == DL_CONFIG)
		gi.cprintf (d->initiator, PRINT_HIGH, "HTTP functions are not compiled on this server.\n");
	return false;
}
void HTTP_ResolveOTDMServer (void)
{
}
#endif
