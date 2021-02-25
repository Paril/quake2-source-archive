/******************************************************************************
**
**	Standard Logging functions - Packet Functions
**
**
**	$Id: sl_packet.c 1.3 1999/04/25 17:51:27 mdavies Exp $
**
**  Copyright (c) 1998-1999 Mark Davies.
**  Distributed under the "Artistic License".
**  Please read the file artistic.txt for complete licensing and
**  redistribution information.
**
******************************************************************************/

#include "sl_packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************
**
**	LOCAL VARIABLES
**
******************************************************************************/

static const char *_unused_id_sl_packet_c = "$Id: sl_packet.c 1.3 1999/04/25 17:51:27 mdavies Exp $";
static const char *_unused_id_sl_packet_h = __SL_PACKET_ID__;

static const char aPacketBytesHeader[] = "SLP\0";
#define PACKET_HEADER_SIZE (4)

/******************************************************************************
**
**	Log Method Functions
**
******************************************************************************/

static void _sl_numcpy( void *pDestination, const void *pSource, size_t len );


/******************************************************************************
**
**	Compression Functions
**
******************************************************************************/

static unsigned char *_sl_Compress( BYTE *pData, DWORD dwSize );



/******************************************************************************
**
**  PACKET_BYTES
**
**  aPacketBytesHeader  (DWORD)
**  nPacket             (DWORD)  - Packet Number
**  nClients            (DWORD)  - Number of client infos
**  dwFlags             (DWORD)  - Compression ?
**  dwStrLen            (DWORD)  - Original Str sizes 
**
**  pServerInfo
**  pLogString
**  
**  pClientInfo
**  pClientInfo
**  ...
**
*******************************************************************************
**
******************************************************************************/

#define SL_PACKET_COMPRESSED    (1<<0)

/******************************************************************************
**
**	ConstructPacketBytes
**
**
*******************************************************************************
**
******************************************************************************/
int sl_ConstructPacketBytes( PACKET_BYTES  **ppPacketBytes,
                             unsigned long  *pPacketBytesLength,
                             PACKET_STRUCT  *pPacketStruct,
                             unsigned long   ulFlags )
{
    int             iRetCode      = SL_ERROR_NONE;
    DWORD           dwLenStrs     = 0;
    DWORD           dwStrLen     = 0;
    int             fCont         = 1;
    BYTE           *pStrBytes     = NULL;
    DWORD           dwFlags       = 0;

    /* Concatenate strings ready for compression */
    if( fCont )
    {
        DWORD nClients = pPacketStruct->nClients;
        
        /* Work out size */
        char **ppStrs = pPacketStruct->ppClientInfo;

        if( pPacketStruct->pServerInfo )
            dwStrLen += strlen(pPacketStruct->pServerInfo)+1;

        if( pPacketStruct->pLogString )
            dwStrLen += strlen(pPacketStruct->pLogString)+1;

        while( nClients-- )
        {
            dwStrLen += strlen(*ppStrs)+1;
            ppStrs++;
        }
    }

    if( fCont )
    {
        /* Alloc mem & copy */
        pStrBytes = calloc( dwStrLen, sizeof(char) );
        if( NULL != pStrBytes )
        {
            char **ppStrs = pPacketStruct->ppClientInfo;
            BYTE  *pPoint = pStrBytes;
            DWORD  nClients = pPacketStruct->nClients;
            DWORD  len;

            /* Server Info */
            if( pPacketStruct->pServerInfo )
            {
                len = strlen(pPacketStruct->pServerInfo)+1;
                memcpy( pPoint,
                        pPacketStruct->pServerInfo,
                        len );
                pPoint += len;
            }
            else
            {
                *pPoint++ = '\0';
            }
            
            /* Log Info */
            if( pPacketStruct->pLogString )
            {
                len = strlen(pPacketStruct->pLogString)+1;
                memcpy( pPoint,
                        pPacketStruct->pLogString,
                        len );
                pPoint += len;
            }
            else
            {
                *pPoint++ = '\0';
            }
                
            /* Client info */
            while( nClients-- )
            {
                len = strlen(*ppStrs)+1;
                memcpy( pPoint,
                        *ppStrs,
                        len );
                pPoint += len;
                ppStrs++;
            }
        }
        else
        {
            /* Out of mem */
            fCont = 0;
            iRetCode = SL_ERROR_NOMEM;
        }
    }

    /* Compress strings updating
    ** dwLenStrs & pStrBytes */
    if( fCont )
    {
        if( ulFlags & SL_FLAGS_COMPRESS )
        {
            unsigned char *pBytes = _sl_Compress( pStrBytes, dwStrLen );

            if( pBytes )
            {
                DWORD   dwNewSize   = 0;

                _sl_numcpy( &dwNewSize,
                            pBytes,
                            sizeof(dwNewSize) );

                memcpy( pStrBytes,
                        pBytes,
                        dwNewSize );

                free(pBytes);
                
                dwLenStrs = dwNewSize;

                dwFlags |= SL_PACKET_COMPRESSED;
            }
            else
            {
                /* Couldn't compress */
                dwLenStrs = dwStrLen;
            }
        }
        else
        {
            dwLenStrs = dwStrLen;
        }
    }
    
    /* Construct Packet */
    if( fCont )
    {
        DWORD   dwPacketLength = 0;
        BYTE   *pPacket = NULL;

        dwPacketLength += PACKET_HEADER_SIZE;
        dwPacketLength += sizeof(pPacketStruct->nPacket);
        dwPacketLength += sizeof(pPacketStruct->nClients);
        dwPacketLength += sizeof(dwFlags);
        dwPacketLength += sizeof(dwStrLen);
        dwPacketLength += dwLenStrs;

        pPacket = calloc( dwPacketLength, sizeof(*pPacket) );
        if( pPacket )
        {
            /* Copy data into packet */
            BYTE *pPoint = pPacket;

            /* Header */
            memcpy( pPoint,
                    aPacketBytesHeader,
                    PACKET_HEADER_SIZE );
            pPoint += PACKET_HEADER_SIZE;

            /* Packet number */
            _sl_numcpy( pPoint,
                        &pPacketStruct->nPacket,
                        sizeof(pPacketStruct->nPacket) );
            pPoint += sizeof(pPacketStruct->nPacket);

            /* Num client infos */
            _sl_numcpy( pPoint,
                        &pPacketStruct->nClients,
                        sizeof(pPacketStruct->nClients) );
            pPoint += sizeof(pPacketStruct->nPacket);

            /* Flags, eg. compressed strings */
            _sl_numcpy( pPoint,
                        &dwFlags,
                        sizeof(dwFlags) );
            pPoint += sizeof(dwFlags);
            
            /* Flags, eg. compressed strings */
            _sl_numcpy( pPoint,
                        &dwStrLen,
                        sizeof(dwStrLen) );
            pPoint += sizeof(dwStrLen);

            /* String data */
            memcpy( pPoint,
                    pStrBytes,
                    dwLenStrs );
            pPoint += dwLenStrs;


            /* Set return data */
            if( fCont )
            {
                *ppPacketBytes = pPacket;
                *pPacketBytesLength = dwPacketLength;
            }
            
        }
        else
        {
            /* Out of mem */
            fCont = 0;
            iRetCode = SL_ERROR_NOMEM;
        }
    }

    /* Cleanup */
    free(pStrBytes);
    pStrBytes = NULL;
    
    return iRetCode;
}

/******************************************************************************
**
**	DestroyPacketBytes
**
**
*******************************************************************************
**
******************************************************************************/
void sl_DestroyPacketBytes( PACKET_BYTES  *pPacketBytes,
                            unsigned long  ulPacketBytesLength )
{
    free( pPacketBytes );
}


/******************************************************************************
**
**	DeconstructPacketBytes
**
**
*******************************************************************************
**
******************************************************************************/
int sl_DeconstructPacketBytes( PACKET_STRUCT **ppPacketStruct,
                               PACKET_BYTES   *pPacketBytes,
                               unsigned long   ulPacketBytesLength,
                               unsigned long   ulFlags )
{
    PACKET_STRUCT  *pPacketStruct = NULL;
    int             iRetCode      = SL_ERROR_NONE;
    int             fCont         = 1;
    BYTE           *pPoint        = pPacketBytes;
    DWORD           nPacket       = 0;
    DWORD           nClients      = 0;
    DWORD           dwFlags       = 0;
    DWORD           dwStrLen      = 0;
    DWORD           dwSize        = 0;


    /* Check size */
    if( fCont &&
        (ulPacketBytesLength < (sizeof(DWORD) * 5)) )
    {
        fCont = 0;
        iRetCode = SL_ERROR_BADPACKET;
    }
    
    /* Check Header */
    if( fCont &&
        !memcmp( pPoint,
                 aPacketBytesHeader,
                 PACKET_HEADER_SIZE) )
    {
        pPoint += PACKET_HEADER_SIZE;

        /* Packet number */
        _sl_numcpy( &nPacket,
                    pPoint,
                    sizeof(nPacket) );
        pPoint += sizeof(nPacket);

        /* Num client infos */
        _sl_numcpy( &nClients,
                    pPoint,
                    sizeof(nClients) );
        pPoint += sizeof(nPacket);

        /* Flags, eg. compressed strings */
        _sl_numcpy( &dwFlags,
                    pPoint,
                    sizeof(dwFlags) );
        pPoint += sizeof(dwFlags);

        /* Flags, eg. compressed strings */
        _sl_numcpy( &dwStrLen,
                    pPoint,
                    sizeof(dwStrLen) );
        pPoint += sizeof(dwStrLen);

        /* Point == start of data */

        /* Work out mem to malloc */
        dwSize  = sizeof( *pPacketStruct );
        dwSize += sizeof(char*) * nClients;
        dwSize += dwStrLen;
    }
    else
    {
        fCont = 0;
        iRetCode = SL_ERROR_BADPACKET;
    }


    /* Alloc mem */
    if( fCont )
    {
        pPacketStruct = (PACKET_STRUCT*)calloc( dwSize, sizeof(BYTE) );
        if( pPacketStruct )
        {
            pPacketStruct->nPacket = nPacket;
            pPacketStruct->nClients = nClients;

            pPacketStruct->ppClientInfo = (char**)(((BYTE*)pPacketStruct) + sizeof(*pPacketStruct));
        }
        else
        {
            /* Out of mem */
            fCont = 0;
            iRetCode = SL_ERROR_NOMEM;
        }
    }


    /* Copy strings to memory */
    if( fCont )
    {
        if( dwFlags & SL_PACKET_COMPRESSED )
        {
            /* Uncompress strings */
        }
        else
        {
            /* Copy strings */
            memcpy( ((BYTE*)pPacketStruct->ppClientInfo) + (sizeof(char*) * nClients),
                    pPoint,
                    dwStrLen );
        }
    }

    /* Set up string pointers */
    if( fCont )
    {
        pPoint = ((BYTE*)pPacketStruct->ppClientInfo) + (sizeof(char*) * nClients);

        pPacketStruct->pServerInfo = (char*)pPoint;
        pPoint += strlen((char*)pPoint)+1;
        
        pPacketStruct->pLogString  = (char*)pPoint;
        pPoint += strlen((char*)pPoint)+1;
        
        /* Set up client info table  */
        if( nClients )
        {
            char **ppStrs = pPacketStruct->ppClientInfo;

            while( nClients-- )
            {
                *ppStrs++ = (char*)pPoint;
                pPoint += strlen((char*)pPoint)+1;
            }
        }
        else
        {
            /* There is no client info
            ** so we had better clear
            ** the pointer */
            pPacketStruct->ppClientInfo = NULL;
        }
    }

    /* Set return pointer */
    if( fCont )
    {
        *ppPacketStruct = pPacketStruct;
    }
    

    /* Cleanup */

    
    return iRetCode;
}


/******************************************************************************
**
**	_sl_numcpy
**
**
*******************************************************************************
**
******************************************************************************/
static void _sl_numcpy( void *pDestination, const void *pSource, size_t len )
{
    BYTE *pDest = (BYTE*)pDestination;
    BYTE *pSrc  = (BYTE*)pSource;

#ifdef __BIG_ENDIAN__
    memcpy( pDest, pSrc, len );
#else
    unsigned int i;
    for( i=0; i<len; i++ )
        *(pDest+i) = *(pSrc+len-i-1);
#endif
}

static unsigned char *_sl_Compress( BYTE *pData, DWORD dwSize )
{
    BYTE   *pResult     = NULL;

    /* TODO */

    return pResult;
}


/* end of file */
