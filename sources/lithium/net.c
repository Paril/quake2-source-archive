/*============================================================================

    This file is part of Lithium II Mod for Quake II
    Copyright (C) 1997, 1998, 1999, 2010 Matthew A. Ayres

    Lithium II Mod is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Lithium II Mod is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lithium II Mod.  If not, see <http://www.gnu.org/licenses/>.

    Quake II is a trademark of Id Software, Inc.

    Code by Matt "WhiteFang" Ayres, matt@lithium.com

============================================================================*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "net.h"
#include "strl.h"

#ifdef WIN32
#define socklen_t int
#endif

#define MAX_LISTEN 512
#define MAX_BACKLOG 16

int ssock = 0;
int rsock[MAX_LISTEN];
struct sockaddr_in raddr[MAX_LISTEN];
char *recvbuf[MAX_LISTEN];
char *recvpos[MAX_LISTEN];

int send_total = 0;
int recv_total = 0;

#ifdef WIN32
#define _ioctl(s, cmd, argp) ioctlsocket(s, cmd, argp)
#else
#define _ioctl(s, cmd, argp) ioctl(s, cmd, argp)
#endif

void sigpipe(int num) {
}

int Net_Init(void) {
#ifdef WIN32
	WSADATA wsadata;
	if(WSAStartup(MAKEWORD(1, 1), &wsadata))
		return 0;
#else
	signal(SIGPIPE, sigpipe);
#endif

	recvbuf[0] = (char *)malloc(BUF_LEN + BUF_OVERFLOW);
	recvpos[0] = NULL;

	return 1;
}

struct sockaddr_in baddr;

void Net_Exit(void) {
	if(ssock)
		Net_Close(ssock);

#ifdef WIN32
	WSACleanup();
#endif
}

// this does a little more then set non-blocking now...
int Net_SetNonBlocking(int sock) {
	static int argp = 1;
	struct linger l;
	l.l_onoff = 0;
	l.l_linger = 0;
	setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&l, sizeof(l));
#ifdef FreeBSD
	setsockopt(ssock, SOL_SOCKET, SO_REUSEPORT, (char *)&argp, sizeof(argp));
#endif
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&argp, sizeof(argp));

	// set non-blocking
	return _ioctl(sock, FIONBIO, &argp);
}

int Net_Listen(int port) {
	int i;

#ifdef FreeBSD
	struct rlimit rlimit;
	rlimit.rlim_cur = MAX_LISTEN;
	rlimit.rlim_max = MAX_LISTEN;
	setrlimit(RLIMIT_NOFILE, &rlimit);
#endif

	for(i = 0; i < MAX_LISTEN; i++) {
		if(i)
			recvbuf[i] = (char *)malloc(BUF_LEN + BUF_OVERFLOW);
		recvpos[i] = NULL;
		rsock[i] = 0;
	}

	if((ssock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
		return 0;

	if(Net_SetNonBlocking(ssock) == -1)
		return 0;

	memset(&baddr, 0, sizeof(baddr));
	baddr.sin_family = AF_INET;
	baddr.sin_addr.s_addr = INADDR_ANY;
	baddr.sin_port = htons((unsigned short)port);

	if(bind(ssock, (void *)&baddr, sizeof(baddr)) != 0)
		return 0;

	if(listen(ssock, MAX_BACKLOG) != 0)
		return 0;

	return 1;
}

void Net_StopListen(void) {
	if(!ssock)
		return;

	shutdown(ssock, 2);
}

int Net_Check(void) {
	int i;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	int sock;
	static int nextfree = -1;

	if(nextfree == -1) {
		for(i = 0; i < MAX_LISTEN; i++)
			if(!rsock[i])
				break;
		if(i == MAX_LISTEN)
			return 0;
		nextfree = i;
	}

	sock = accept(ssock, (struct sockaddr *)&raddr[nextfree], &addrlen);
	if(sock == -1)
		return 0;

	rsock[nextfree] = sock;
	nextfree = -1;

	if(Net_SetNonBlocking(sock) == -1) {
		Net_Close(sock);
		return 0;
	}

	return sock;
}

int Net_Lookup(char *host) {
	struct hostent *hostentry;

	hostentry = gethostbyname(host);
	if(!hostentry)
		return -1;

	return *(int *)hostentry->h_addr_list[0];
}

int Net_Connect(int haddr, int port) {
	int sock;
	struct sockaddr_in addr;

	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
		return -1;

	if(Net_SetNonBlocking(sock) == -1)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = haddr;
	addr.sin_port = htons((unsigned short)port);

	connect(sock, (struct sockaddr *)&addr, sizeof(addr));

	return sock;
}

int Net_Send(int sock, char *buf) {
	int code;
	int len = strlen(buf) + 1;
	code = send(sock, buf, len, 0);
	send_total += len;
	return code;
}

int Net_Sendf(int sock, char *format, ...) {
	va_list argptr;
	char buf[512];

	va_start(argptr, format);
	vsnprintf(buf, sizeof(buf), format, argptr);
	va_end(argptr);

	return Net_Send(sock, buf);
}

int Net_IsConnect(int sock) {
	char rbuf[BUF_LEN];
	int rlen;
	
	rlen = recv(sock, rbuf, BUF_LEN, 0);

	if(rlen > 0) {
		recvpos[0] = recvbuf[0];
		memcpy(recvpos[0], rbuf, rlen);
		recvpos[0] += rlen;
		return 1;
	}

	return 0;
}

// return -1 if connection lost
// return 0 if no data waiting
// return len of data recv'd otherwise
int Net_Recv(int sock, char *buf, int len) {
	char rbuf[BUF_LEN];
	char *r;
	int rlen;
	int i = 0;
	int err;

	if(ssock)
		for(i = 0; i < MAX_LISTEN; i++)
			if(sock == rsock[i])
				break;

	if(!recvpos[i])
		recvpos[i] = recvbuf[i];

	r = recvbuf[i];
	while(r < recvpos[i]) {
		if(!*r) {
			strlcpy(buf, recvbuf[i], sizeof(buf));
			rlen = strlen(buf) + 1;
			memcpy(recvbuf[i], r + 1, recvpos[i] - r);
			recvpos[i] -= rlen;
			return rlen;
		}
		r++;
	}

	rlen = recv(sock, rbuf, BUF_LEN, 0);

	if(rlen > 0) {
		memcpy(recvpos[i], rbuf, rlen);
		recvpos[i] += rlen;
		recv_total += rlen;
		return 0;
	}

	if(rlen == 0)
		return -1;

	// must be an error (rlen == -1)
#ifdef WIN32
	err = WSAGetLastError();
	if(err == WSAEWOULDBLOCK)
		return 0;
	if(err == WSAECONNRESET)
		return -1;
#else
	err = errno;
	if(err == EAGAIN)
		return 0;
	if(err == ECONNRESET)
		return -1;
#endif

	return -1;
}

int Net_Close(int sock) {
	int i;
	for(i = 0; i < MAX_LISTEN; i++)
		if(sock == rsock[i])
			rsock[i] = 0;

#ifdef WIN32
	return closesocket(sock);
#else
	return close(sock);
#endif
}

int Net_SendTotal(void) {
	return send_total;
}

int Net_RecvTotal(void) {
	return recv_total;
}


//---------------------------------
// time code 

#include <sys/timeb.h>

int start_sec;
int start_msec;

void ReadTime(int *sec, int *msec) {
#ifdef WIN32
    struct _timeb t;
    _ftime(&t);
	*sec = (int)t.time;
	*msec = (int)t.millitm;
#else
    struct timeval t;
    gettimeofday(&t, NULL);
    *sec = (int)t.tv_sec;
    *msec = (int)t.tv_usec / 1000;
#endif
}

void StartTime(void) {
	ReadTime(&start_sec, &start_msec);
}

// returns milliseconds since StartTime()
int GetTime(void) {
	int sec, msec;
	ReadTime(&sec, &msec);
	return (sec - start_sec) * 1000 + (msec - start_msec);
}

//---------------------------------
// misc code

char *Net_GetAddrStr(int sock) {
	int i;
	int haddr;
	static char ip[64];

	for(i = 0; i < MAX_LISTEN; i++) {
		if(sock == rsock[i]) {
			haddr = ntohl(raddr[i].sin_addr.s_addr);
			snprintf(ip, sizeof(ip), "%d.%d.%d.%d", (haddr >> 24) & 0xff, (haddr >> 16) & 0xff, (haddr >> 8) & 0xff, haddr & 0xff);
			return ip;
		}
	}

	return "";
}

void Sys_Sleep(int msec) {
#ifdef WIN32
	Sleep(msec);
#else
	// uhh broken in BSD btw...
	usleep(msec);
#endif
}

void Split(char *msg, char *s1, unsigned int s1len, char *s2, unsigned int s2len) {
	char *c;
	char buf[BUF_LEN];

	strlcpy(buf, msg, sizeof(buf));
	c = (char *)strstr(buf, DELIM);
	if(c) {
		*c = 0;
		strlcpy(s1, buf, s1len);
		strlcpy(s2, c + 1, s2len);
	}
	else {
		strlcpy(s1, buf, s1len);
		strlcpy(s2, "", s2len);
	}
}

void wf_strlwr(char *str) {
	while(*str) {
		*str = tolower(*str);
		str++;
	}
}

//---------------------------------
// test code

#if 0

int s_port = 24000;

void server(void) {
	char buf[256];
	char snd[256];
	int i, len;
	int sock;

	printf("Server\n");

	Net_Init();

	if(!Net_Listen(s_port)) {
		printf("couldn't open socket to listen\n");
		return;
	}

	printf("listening for packets...\n");

	while(1) {
		Sys_Sleep(50);

		while(1) {
			sock = Net_Check();
			if(!sock)
				break;
			printf("accept connect from '%s' (%d)\n", Net_GetAddrStr(sock), sock);
		}

		for(i = 0; i < MAX_LISTEN; i++) {
			if(!rsock[i])
				continue;
			while(1) {
				len = Net_Recv(rsock[i], buf, sizeof(buf));
				if(!len)
					break;

				if(len == -1) {
					printf("dropped connect (%d)\n", rsock[i]);
					Net_Close(rsock[i]);
					rsock[i] = 0;
					break;
				}

				printf("packet data = '%s' (%d)\n", buf, len);

				snprintf(snd, sizeof(snd), "you said: %s", buf);
				Net_Send(rsock[i], snd);
			}
		}
	}
}

void client(char *host, char *msg) {
	char buf[256];
	int sock;
	int i = 0;
	int addrlen = sizeof(struct sockaddr_in);
	int len;

	printf("Client\n");

	Net_Init();

	sock = Net_Connect(Net_Lookup(host), s_port);

	while(!Net_IsConnect(sock))
		;

	for(i = 0; i < 5; i++) {
		strlcpy(buf, msg, sizeof(buf));
		Net_Send(sock, buf);
		printf("sent packet, data = '%s'.\n", buf);
	}

	strlcpy(buf, "bye!", sizeof(buf));
	Net_Send(sock, buf);
	printf("sent packet, data = '%s'.\n", buf);

	for(i = 0; i < 10; i++) {
		Sys_Sleep(50);
		while(1) {
			len = Net_Recv(sock, buf, sizeof(buf));
			if(!len)
				break;
			if(len == -1) {
				Net_Close(sock);
				printf("dropped connect!\n");
				return;
			}
			printf("packet data = '%s' (%d)\n", buf, len);
		}
	}

	Sys_Sleep(100);
	Net_Close(sock);
}

int main(int argc, char *argv[]) {
	if(Net_Init() == -1)
		return 0;
	if(argc < 2)
		return 0;
	if(argv[1][0] == '1')
		server();
	else
		client(argv[2], argv[3]);
	Net_Exit();
	return 1;
}

#endif
