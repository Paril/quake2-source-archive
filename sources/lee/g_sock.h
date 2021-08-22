

//Midi stuff
qboolean ParseIP (char *ip, char *newip);
qboolean InitSocks(void);
void ShutdownSocks(void);
void ChkSockState(void);
qboolean SendMsg(gclient_t *client, char msgid, char *midi);


qboolean CloseClientSocket(gclient_t *client);
qboolean InitClientSock(gclient_t *client);