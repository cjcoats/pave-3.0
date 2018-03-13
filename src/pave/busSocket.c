/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busSocket.c 83 2018-03-12 19:24:33Z coats $
 *  Copyright (C) 1996-2004 MCNC
 *            (C) 2004-2010 UNC Institute for the Environment
 *            (C) 2018-     Carlie J. Coats, Jr., Ph.D.
 *
 *  Licensed under the GNU General Public License Version 2.
 *  See enclosed gpl.txt for more details
 *
 *  For further information on PAVE:
 *      Usage: type -usage in PAVE's standard input
 *      User Guide: https://cjcoats.github.io/pave/PaveManual.html
 *      FAQ:        https://cjcoats.github.io/pave/Pave.FAQ.html
 *
 ****************************************************************************
 * ABOUT:  - busSocket.c
 *
 *      Bus library routines and structures for creation and manipulation of
 *      sockets.
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *
 ********************************************************************
 * REVISION HISTORY - busSocket.c
 *
 * Date: 14-Dec-94
 * Version: 0.1
 * Change Description: ORIGINAL CODE
 * Change author: Leland Morrison, NCSU, CSC
 *
 * Date: 15-Dec-94
 * Version: 0.2
 * Change Description: Added some comments and main headers
 * Change author: M. Vouk, NCSU, CSC
 *
 * Date: 14-Mar-95
 * Version: 0.4
 * Change Description: Deleted the connectionless socket functionality
 *            in busSocket_makeConnection
 * Change author: M. Vouk, NCSU, CSC
 *
 * Date: 14-Aug-96
 * Version: 0.7
 * Change Description: Added Debug statements and setsockoptions for
 *                     TCP_NODELAY
 * Change author: R. Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "busMsgQue.h"
#include "busSocket.h"
#include "busError.h"
#include "busDebug.h"

int busSocket_createAcceptorSocket ( int *portNumber )
    {
    /* create a socket, and binds it to receive connections from
       the port specified. if the port specified is 0, the
       port number is filled in */
    struct sockaddr_in sin;
    int portSocket;
    int sin_size;
    int on=1;
    int bufsize = 32768;

    portSocket = socket ( AF_INET, SOCK_STREAM, IPPROTO_IP );

    if ( setsockopt ( portSocket, IPPROTO_TCP, TCP_NODELAY, ( char * ) &on, sizeof ( on ) ) < 0 )
        printf ( "setsockopt IPPROTO_TCP error \n" );
    /*
    */
    if ( setsockopt ( portSocket, SOL_SOCKET, SO_SNDBUF, ( char * ) &bufsize,
                      sizeof ( bufsize ) ) < 0 )
        printf ( "setsockopt SO_SNDBUF error \n" );
    if ( setsockopt ( portSocket, SOL_SOCKET, SO_RCVBUF, ( char * ) &bufsize,
                      sizeof ( bufsize ) ) < 0 )
        printf ( "setsockopt SO_RCVBUF error \n" );

    /* portSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP); */
    if ( portSocket < 0 )
        return SBUSERROR_SOCKET_COULDNT_CREATE;

    sin.sin_family = AF_INET;
    sin.sin_port    = htons ( *portNumber );
    sin.sin_addr.s_addr = htonl ( INADDR_ANY );

    if ( bind ( portSocket, ( struct sockaddr * ) &sin, sizeof ( sin ) ) <0 )
        {
        perror ( "bind failed" );
        close ( portSocket );
        return -2;
        }

    sin_size = sizeof ( struct sockaddr );
    if ( getsockname ( portSocket, ( struct sockaddr * ) &sin, &sin_size ) <0 )
        return SBUSERROR_COULDNT_getsockname;

    *portNumber = ntohs ( sin.sin_port );

    listen ( portSocket, 2 );

    return portSocket;
    }

int busSocket_makeBoundConnectionless ( int *portNumber )
    {
    struct sockaddr_in sin;
    int portSocket;
    int sin_size;

    portSocket = socket ( AF_INET, SOCK_DGRAM, 0 );
    if ( portSocket < 0 )
        {
        perror ( "call to socket failed" );
        return SBUSERROR_SOCKET_COULDNT_CREATE;
        }

    sin.sin_family = AF_INET;
    sin.sin_port    = htons ( *portNumber );
    sin.sin_addr.s_addr = htonl ( INADDR_ANY );

    if ( bind ( portSocket, ( struct sockaddr * ) &sin, sizeof ( sin ) ) <0 )
        {
        perror ( "bind failed" );
        close ( portSocket );
        return SBUSERROR_SOCKET_COULDNT_CREATE;
        }

    sin_size = sizeof ( struct sockaddr );
    if ( getsockname ( portSocket, ( struct sockaddr * ) &sin, &sin_size ) <0 )
        return SBUSERROR_COULDNT_getsockname;

    *portNumber = ntohs ( sin.sin_port );

    return portSocket;
    }

int busSocket_acceptConnection ( int portSocket,
                                 struct sockaddr_in *sad )
    {
    int conSock, sad_size;

    sad_size = sizeof ( struct sockaddr_in );

    /* printf("calling accept on port %d \n", portSocket);  */
    fflush ( stdout );

    conSock = accept ( portSocket, ( struct sockaddr * ) sad, &sad_size );

    if ( conSock < 0 )
        return SBUSERROR_ACCEPT_FAILED;

    /* consider setsockopt( conSock, SOL_SOCKET, SO_REUSEADDR, NULL, 0 ); */

    return conSock;
    }

int busSocket_acceptFromConnectionless ( int sunSocket,
        struct sockaddr_in *sin )
    {
    int s, pbuf, port, sin_size, err;

    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = htonl ( INADDR_ANY );
    sin->sin_port = htons ( 0 );

    sin_size = sizeof ( struct sockaddr_in );

    debug0 ( DEBUG_SOCKET,"calling recvfrom " );
    err = recvfrom ( sunSocket, ( char * ) &pbuf, 4, 0,
                     ( struct sockaddr * ) sin, &sin_size );

    if ( err<0 )
        {
        perror ( "recvfrom failed" );
        return SBUSERROR_ACCEPT_FAILED;
        }

    if ( err!=4 )
        return SBUSERROR_GENERAL_FAILURE;

    port = ntohs ( pbuf );

    sin->sin_family = AF_INET;
    sin->sin_port = pbuf;

    s = socket ( sin->sin_family, SOCK_STREAM, 0 );

    if ( s<0 )
        {
        perror ( "call to socket failed" );
        return SBUSERROR_SOCKET_COULDNT_CREATE;
        }

    debug2 ( DEBUG_SOCKET,"will attempt to connect to %s:%d ",
             inet_ntoa ( sin->sin_addr ), port );

    debug0 ( DEBUG_SOCKET,"calling connect " );
    err = connect ( s, ( struct sockaddr * ) sin, sin_size );

    if ( err<0 )
        {
        perror ( "sunport - failed to connect" );
        close ( s );
        return SBUSERROR_ACCEPT_FAILED;
        }

    return s;
    }

extern int errno;

int busSocket_makeConnection ( char *host_addr, int port )
    {
    struct sockaddr_in sin;
    int s,err;
    int on=1;
    int bufsize = 32768;

    s = socket ( AF_INET, SOCK_STREAM, 0 );

    if ( setsockopt ( s, IPPROTO_TCP, TCP_NODELAY, ( char * ) &on, sizeof ( on ) ) < 0 )
        printf ( "setsockopt IPPROTO_TCP error \n" );
    /*
    */
    if ( setsockopt ( s, SOL_SOCKET, SO_SNDBUF, ( char * ) &bufsize,
                      sizeof ( bufsize ) ) < 0 )
        printf ( "setsockopt SO_SNDBUF error \n" );
    if ( setsockopt ( s, SOL_SOCKET, SO_RCVBUF, ( char * ) &bufsize,
                      sizeof ( bufsize ) ) < 0 )
        printf ( "setsockopt SO_RCVBUF error \n" );

    /* s = socket( AF_INET, SOCK_STREAM, IPPROTO_IP); */
    if ( s<0 )
        return SBUSERROR_SOCKET_COULDNT_CREATE;

    sin.sin_family = AF_INET;

    if ( host_addr==NULL )
        sin.sin_addr.s_addr=htonl ( INADDR_LOOPBACK );
    else
        {
        int b0,b1,b2,b3;
        if ( sscanf ( host_addr, "%d.%d.%d.%d",&b0,&b1,&b2,&b3  ) !=4 )
            return SBUSERROR_INVALIDHOST;

        sin.sin_addr.s_addr =
            htonl ( b0 << 24 | b1 << 16 | b2 << 8 | b3 );
        }

    sin.sin_port = htons ( port );

    err = connect ( s, ( struct sockaddr * ) &sin, sizeof ( sin ) );

    if ( err<0 )
        {
        close ( s );

        if ( errno == ECONNREFUSED )
            {
            return SBUSERROR_CONNECT_FAILED;
            }
        else
            return SBUSERROR_CONNECT_FAILED;
        }

    return s;
    }

