/******************************************************************************
**
**	Standard Logging functions - Packet Functions
**
**
**	$Id: sl_packet.h 1.2 1999/04/07 21:18:04 mdavies Exp $
**
**  Copyright (c) 1998-1999 Mark Davies.
**  Distributed under the "Artistic License".
**  Please read the file artistic.txt for complete licensing and
**  redistribution information.
**
******************************************************************************/

#ifndef __SL_PACKET_H__
#define __SL_PACKET_H__


#define __SL_PACKET_ID__ "$Id: sl_packet.h 1.2 1999/04/07 21:18:04 mdavies Exp $"

/******************************************************************************
**
**	DEFINES
**
******************************************************************************/

#define SL_ERROR_NONE        0
#define SL_ERROR_NOMEM      -1
#define SL_ERROR_BADPACKET  -2

#define SL_FLAGS_NONE        0
#define SL_FLAGS_COMPRESS    (1<<0)


/******************************************************************************
**
**	Typedefs
**
******************************************************************************/

typedef unsigned long DWORD;
typedef unsigned char BYTE;

typedef struct
{
    DWORD   nPacket;
    DWORD   nClients;
    char   *pServerInfo;
    char   *pLogString;
    char  **ppClientInfo;               /* NULL terminated Array of strings pointers */
} PACKET_STRUCT;

typedef unsigned char   PACKET_BYTES;



extern int sl_ConstructPacketBytes( PACKET_BYTES  **ppPacketBytes,
                                    unsigned long  *pPacketBytesLength,
                                    PACKET_STRUCT  *pPacketStruct,
                                    unsigned long   ulFlags );

extern void sl_DestroyPacketBytes( PACKET_BYTES  *pPacketBytes,
                                   unsigned long  ulPacketBytesLength );


extern int sl_DeconstructPacketBytes( PACKET_STRUCT **ppPacketStruct,
                                      PACKET_BYTES   *pPacketBytes,
                                      unsigned long   ulPacketBytesLength,
                                      unsigned long   ulFlags );

#endif /* __SL_PACKET_H__ */

/* end of file */
