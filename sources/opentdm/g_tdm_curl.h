/*==============
HTTP Stuff
==============*/
void HTTP_Init (void);
void HTTP_Shutdown (void);
qboolean HTTP_QueueDownload (tdm_download_t *d);
void HTTP_ResolveOTDMServer (void);
