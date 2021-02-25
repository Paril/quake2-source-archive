

//Midi stuff
qboolean ParseIP (char *ip, char *newip);
qboolean GenInitSocks(void);
void GenShutdownSocks(void);
void GenChkSockState(void);
qboolean GenSendMsg(gclient_t *client, char msgid, char *midi);


qboolean GenCloseClientSocket(gclient_t *client);
qboolean GenInitClientSock(gclient_t *client);