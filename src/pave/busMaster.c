/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busMaster.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  - busMaster.c
 *
 *     This module contains the library of functions and data structures
 *     specific to the bus controller or BusMaster.
 *
 *     NETWORKING:        remote/network connectivity is via bus library (sockets)
 *
 *     COMPILATION:       (see makefile)
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *                Direct Connection setup and WhosConnected functions
 *                should be modified to use BusSendBusByte instead
 *                of BusWriteCharacter.
 *
 ********************************************************************
 * REVISION HISTORY - busMaster.c
 *
 * Date: 14-Dec-94
 * Version: 0.1
 * Change Description: ORIGINAL CODE
 * Change author: Leland Morrison, NCSU, CSC
 *
 * Date: 15-Dec-94
 * Version: 0.2
 * Change Description: Added main comment headers
 * Change author: M. Vouk, NCSU, CSC
 *
 * Date: 7-Jan-95
 * Version: 0.2.1
 * Change Description: Modifying the protocol for sending and receiving
 *                     messages between busMaster and clients.
 *                     Changed calls to BusWriteCharacter to BusReplyBusByte/
 *                     BusSendBusByte/BusWriteMessage
 *                     The BusMaster looks in the messageOption field of
 *                     the message instead of busByte to determine how to
 *                     process the message.
 * Change author: R. Balay, NCSU, CSC
 *
 * Date: 17-Jan-95
 * Version: 0.3
 * Change Description: Direct Connection and FindWhosConnected are working.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 27-Jan-95
 * Version: 0.3a
 * Change Description: Modified the parameter passing to function
 *                     BusMasterBroadcast from 'struct BusMessage' to
 *             'struct BusMessage *'.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 18-March-95
 * Version: 0.4
 * Change Description: BusMaster responds to requests for MODULE_INFO_BY_NAME
 *                     and MODULE_INFO_BY_ID.
 * Change author: Rajini Balay
 *
 * Date: 6-Apr-95
 * Version: 0.5
 * Change Description: BusMaster creates the file /tmp/sbus_port_$USER even
 *            if the SBUSFILE env variable is not set
 * Change author: Rajini Balay
 *
 * Date: 12-Apr-95
 * Version: 0.5a
 * Change Description: BusMaster responds to KILL_CLIENT messages
 * Change author: Rajini Balay
 *
 * Date: 9-July-95
 * Version: 0.5c
 * Change Description: Replaced function "cuserid" with "BusGetMyUserid()"
 * Change author: R. Balay
 *
 * Date: 15-Aug-96
 * Version: 0.7
 * Change Description: Debugging statements and stmts for measuring busmaster
 *                     service time (TIME_BM_RU, TIME_BM_HR, TIME_BM_TIMES)
 * Change author: R. Balay
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <sys/types.h>

#include <sys/socket.h>  /* socket structures */
#include <netinet/in.h>  /* defines sockaddr_in */
#ifndef __OPENNT
#include <sys/un.h>      /* defines sockaddr_un */
#endif
#include <arpa/inet.h>
#include <netdb.h>       /* gethostbyname */

#include <stdlib.h>      /* exit   */
#include <stdio.h>       /* printf */
#include <time.h>        /* struct timeval (in select) */
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/file.h>
#include <sys/stat.h>    /* open / S_IRUSR, ... */

#if defined(TIME_BM_RU) || defined(TIME_BM_HR) || defined(TIME_BM_TIMES)
#include "busMasterTime.h"
#endif

#include <fcntl.h>
#include "busSocket.h"
#include "busMaster.h"

#include "busRW.h"
#include "busRWMessage.h"
#include "busRepReq.h"
#include "masterRTFuncs.h"
#include "busError.h"
#include "masterDB.h"
#include "busDebug.h"

#define BUS_MASTER_CONNECT_BACKLOG 2 /* shouldn't be set lower */

int BusMasterPortInitialization ( int *port )
/* create the first socket, bind it to a port specificied by
   the environment variable SBUSPORT to listen for
   connection attempts */
/* it returns a positive socket id (file descriptor) on success,
   or a negative error number on failure */
/* port - the address of a variable to hold the port # the socket
   has been bound to */
    {
    char *pbuf;
    int portSocket,portNumber;

    pbuf = getenv ( SBUSPORT );

    if ( pbuf == NULL )
        portNumber = 0; /* lowest available port is used */
    else
        {
        /* get port number to use from environment variable */
        portNumber = atoi ( pbuf );
        }

    portSocket = busSocket_createAcceptorSocket ( &portNumber );

    if ( port!=NULL )
        *port = portNumber;

    if ( pbuf==NULL )
        fprintf ( stderr, "%s=%d\n", SBUSPORT, portNumber );

#ifdef SBUSLOCSAVE
        {
        char *filename, sbusfile[128];
        int x;

        filename = getenv ( SBUSLOCSAVE );
        if ( filename == NULL )
            {
            sprintf ( sbusfile, "/tmp/sbus_port_%s", BusGetMyUserid() );
            filename = sbusfile;
            }
        if ( filename != NULL )
            {
            x=open ( filename,
                     ( O_WRONLY | O_CREAT | O_TRUNC ),
                     0600 /* yes, octal */ );
            if ( x>0 )
                {
                char buffer[40];
                sprintf ( buffer, "%d", portNumber );
                write ( x,buffer,strlen ( buffer ) );
                close ( x );
                }
            else
                perror ( "saving of port number failed" );
            }
        }
#endif /* SBUSLOCSAVE */

    return portSocket;
    }

int BusMasterInitialize ( struct BusMasterData *bmd )
    {
    struct BusTypeNode *btn;

    bmd->portSocket   = -1;
    bmd->portNumber   = -1;

    bmd->Types        = NULL;
    bmd->nextTypeId   = 2*FD_SETSIZE;

    bmd->Modules      = NULL;
    bmd->nextModuleId = FD_SETSIZE;

    bmd->nextMessageSerialNumber = 1;

    bmd->open_action  = NULL;
    bmd->open_data    = NULL;

    bmd->close_action = NULL;
    bmd->close_data   = NULL;

    /* New field added in v6.2 for keeping track of
       requests for Direct Connections    */
    bmd->WaitingReq = NULL;

    bmd->portSocket = BusMasterPortInitialization ( & ( bmd->portNumber ) );
    if ( bmd->portSocket < 0 )
        return bmd->portSocket;

    /* bmd->unixSocket = ... (bmd->unixPath) ... */

    btn = BusMasterAddType ( bmd->Types, "ASCIIZ",  bmd->nextTypeId++ );
    if ( btn != NULL )
        bmd->Types = btn;
    else
        return SBUSERROR_NOMEMORY;

#if defined(TIME_ST_BUS) || defined(TIME_IAT)
    BusTSInitFile ( "busMaster",100 );
#endif


    if ( bmd->portSocket>0 )
        return SBUSERROR_NOT;
    else
        return bmd->portSocket;
    }


void BusMasterBroadcastBusByte ( int fd, int seq, unsigned char *dummy )
    {
    BusForwdBusByte ( fd, seq, MASTERID, BusBroadcast, *dummy, 0, NULL );
    }

void BusMasterShutdownAction ( int fd, char *dummy )
    {
    /* the following disables writes to the connection from
       the client module */
    shutdown ( fd,2 );
    }

void BusMasterCloseAction ( int fd, char *dummy )
    {
    debug1 ( DEBUG_MASTER,"closing #%d, ",fd );
    close ( fd );
    }

void BusMasterShutdown ( struct BusMasterData *bmd )
/* send out the shutdown message, and close all the
open file descriptors */
    {
    struct BusMessage     shutdown;
    struct BusTypeNode   *btn;

    if ( bmd->close_action != NULL )
        BusMasterActOnfds ( bmd,bmd->close_action,bmd->close_data );
    printf ( "Performed close_action for all descriptors...\n" );

    /* initialize a BusMessage structure with the
       message to shutdown */
    shutdown.toModule = BusBroadcast;
    shutdown.fromModule = -1;

    printf ( "Looking up type \"Application Msg Shutdown\"\n" );
    btn = BusMasterFindTypeByName ( bmd->Types, "Application Msg Shutdown" );

    if ( btn != NULL )
        {
        printf ( "Found the type - id = %d\n",btn->typeId );
        shutdown.messageType = btn->typeId;
        }
    else
        shutdown.messageType = -1;

    shutdown.messageLength = strlen ( "Shutdown" )+1;
    shutdown.message = "Shutdown";

    printf ( "Sending out message\n" );
    /* send out the shutdown message to everyone */
    BusMasterBroadcast ( bmd,&shutdown );
    printf ( "Shutdown message broadcast...\n" );

    /* Disable further receives... */
    /*BusMasterActOnfds(bmd,BusMasterShutdownAction,NULL);
    printf("No further receives...\n");*/

    /* close all of our connections */
    BusMasterActOnfds ( bmd,BusMasterCloseAction,NULL );
    printf ( "All connections closed\n" );

    /* remove the unix domain socket from the filesystem
    unlink( (bmd->unixPath) );
    */

    /* The software bus has been shut down */
    }


void BusMasterSetOpenAction ( struct BusMasterData *bmd,
                              void ( *action ) ( int fd, char *data ),
                              char *data )
    {
    bmd -> open_action = action;
    bmd -> open_data   = data;
    }

void BusMasterSetCloseAction ( struct BusMasterData *bmd,
                               void ( *action ) ( int fd, char *data ),
                               char *data )
    {
    bmd -> close_action = action;
    bmd -> close_data   = data;
    }


void BusMasterActOnfds ( struct BusMasterData *bmd,
                         void ( *action ) ( int fd,char *data ), char *data )
/* perfom an action on all of the file descriptors in use
by the bus */
    {
    struct BusModuleNode *curModule;

    if ( bmd->Modules != NULL )
        {
        curModule = bmd->Modules;

        do
            {
            if ( curModule->fd>=0 )
                ( action ) ( curModule->fd, data );
            curModule=curModule->next;
            }
        while ( curModule && curModule != bmd->Modules );
        }

    ( action ) ( bmd->portSocket, data );
    /*(action)( bmd->unixSocket, data );*/
    }


int BusMasterCheckForNewConnection ( struct BusMasterData *bmd,
                                     int fd )
    {
    struct sockaddr_in sin;
    struct BusModuleNode *bmn;
    char *name;
    int s;

    if ( bmd==NULL )
        return SBUSERROR_BADPARAMETER;
    if ( ( fd != bmd->portSocket ) /* && (fd != bmd->unixSocket) */ )
        return SBUSERROR_BADPARAMETER;


    s = busSocket_acceptConnection ( fd, &sin );
    debug1 ( DEBUG_MASTER,"Getting a new connection ... current value of s = %d",s );

    if ( s<0 )
        {
        perror ( "Attempt to connect failed." );
        return s;
        }

    debug3 ( DEBUG_MASTER,"Connection established from %s:%d on socket %d, reading name...\n",
             inet_ntoa ( sin.sin_addr ),ntohs ( sin.sin_port ), s );

    BusReadASCIIZ ( s, &name );

    debug1 ( DEBUG_MASTER,"Module %s is being added.\n", name );

    /* Checking for duplicate name */
    bmn = BusMasterFindModuleByName ( bmd->Modules, name );

    if ( bmn != NULL )
        {
        BusWriteInteger ( s, FIND_ID_ERR );
        shutdown ( s,2 );
        close ( s );

        return SBUSERROR_GENERAL_FAILURE;
        }

    bmn = BusMasterAddModule ( bmd->Modules, name, bmd->nextModuleId++, s );
    if ( bmn != NULL )
        {
        bmd->Modules = bmn;

        bmn->sin.sin_family      = sin.sin_family;

        if ( ( sin.sin_addr.s_addr == htonl ( INADDR_LOOPBACK ) ) ||
                ( sin.sin_addr.s_addr == htonl ( INADDR_ANY ) ) )
            {
            char host_name[32];
            struct hostent *host;

            gethostname ( host_name, 32 );
            debug1 ( DEBUG_MASTER,"Getting inet address for host (%s)\n",host_name );

            host = gethostbyname ( host_name );

            bmn->sin.sin_addr.s_addr =
                * ( ( unsigned long * ) ( host->h_addr_list[0] ) );
            debug1 ( DEBUG_MASTER,"Host address is %s.\n",inet_ntoa ( bmn->sin.sin_addr ) );
            }
        else
            bmn->sin.sin_addr.s_addr = sin.sin_addr.s_addr;

        bmn->sin.sin_port        = sin.sin_port;

        debug1 ( DEBUG_MASTER, "Informing module its id # is %d\n",bmn->moduleId );
        BusWriteInteger ( s, bmn->moduleId );
        debug2 ( DEBUG_NEW_MOD,"BusMaster : Module %s added with ID %d\n", name,
                 bmn->moduleId );
        return SBUSERROR_NOT;
        }
    else
        {
        shutdown ( s,2 );
        close ( s );

        return SBUSERROR_NOMEMORY;
        }
    }

int BusMasterResponder ( struct BusMasterData *bmd,
                         struct BusModuleNode *bmn )
    {
    int err;
    struct BusMessage bmsg;

#if defined(TIME_BM_RU) || defined(TIME_BM_HR) || defined(TIME_BM_TIMES)
    double readTime, servTime;

    t_start();
#endif

    err = BusReadMessage ( bmn->fd, &bmsg );

#if defined(TIME_BM_RU) || defined(TIME_BM_HR) || defined(TIME_BM_TIMES)
    t_stop();
    readTime = t_getutime();
#endif
    /*
      if (DEBUG_RESPONDER)
         debugShowMessage(&bmsg);
    */
#if defined(TIME_BM_RU) || defined(TIME_BM_HR) || defined(TIME_BM_TIMES)
    t_start();
#endif

    if ( err != SBUSERROR_NOT )
        {
        perror ( "Connection with client has crashed..." );
        BusMasterRTModuleLeaving ( bmd, bmn );
        sleep ( 2 );
        return SBUSERROR_NOT;
        }
    switch ( bmsg.messageOption )
        {
        case BUSBYTE_RESET_CONNECTION:
            debug2 ( DEBUG_RESPONDER, "BUSBYTE_RESET_CONNECTION: Reseting Connection for module \"%s\" (fd=%d)\n",
                     bmn->moduleName, bmn->fd );
            err = BusMasterRTResetConnection ( bmd, bmn, &bmsg );
            break;
        case BUSBYTE_MESSAGE_HERE:
            debug3 ( DEBUG_RESPONDER, "BUSBYTE_MESSAGE_HERE: Message from module \"%s\"  of length=%d (fd=%d)\n", bmn->moduleName, bmsg.messageLength, bmn->fd );
            err = BusMasterRTProcessMessage       ( bmd, bmn, &bmsg );
            break;
        case BUSBYTE_PLEASE_ACKNOWLEDGE:
            debug0 ( DEBUG_RESPONDER, "BUSBYTE_PLEASE_ACKNOWLEDGE: Module request Acknowledgement\n" );
            err = BusReplyBusByte ( bmn->fd, bmsg.serial, MASTERID, bmn->moduleId,
                                    BUSBYTE_ACKNOWLEDGEMENT, 0, NULL );
            break;
        case BUSBYTE_ACKNOWLEDGEMENT:
            debug0 ( DEBUG_RESPONDER, "BUSBYTE_ACKNOWLEDGEMENT: An acknowledgement was received (for what???)\n" );
            if ( BusGetMessageByModuleOption ( bmd->WaitingReq, bmn->moduleId,
                                               BUSBYTE_RESET_CONNECTION ) != NULL )
                {
                if ( bmn->Messages != NULL )
                    {
                    struct BusMessage *tmp_bmsg;

                    tmp_bmsg = BusGetFirstMessage ( bmn->Messages );
                    BusWriteMessage ( bmn->fd, tmp_bmsg );
                    }
                }
            /* this probably means something isn't right... */
            break;
        case BUSBYTE_GET_FIRST_MESSAGE:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_GET_FIRST_MESSAGE: Client \"%s\" wants its first message...\n",bmn->moduleName );
            if ( bmn->Messages != NULL )
                {
                struct BusMessage *tmp_bmsg;

                tmp_bmsg = BusGetFirstMessage ( bmn->Messages );
                BusWriteMessage ( bmn->fd, tmp_bmsg );
                }
            break;
        case BUSBYTE_DEQUE_MESSAGE:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_DEQUE_MESSAGE: Dequing message sent to client \"%s\".\n",bmn->moduleName );
            if ( bmn->Messages != NULL )
                {
                bmn->Messages = Bus_DequeMessageByModuleSeq ( bmn->Messages,
                                bmsg.toModule, bmsg.serial );
                /* If there are messages waiting to be sent on the queue,
                send the first one in line                   */
                if ( bmn->Messages != NULL )
                    {
                    struct BusMessage *tmp_bmsg;

                    tmp_bmsg = BusGetFirstMessage ( bmn->Messages );
                    debug2 ( DEBUG_RESPONDER, " Sending a waiting message seq = %d from = %d \n", tmp_bmsg->serial, tmp_bmsg->fromModule );
                    BusWriteMessage ( bmn->fd, tmp_bmsg );
                    }
                }
            break;
        case BUSBYTE_FIND_MODULE_BY_NAME:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_FIND_MODULE_BY_NAME: Client \"%s\" is asking for a module name -> id\n",bmn->moduleName );
            err = BusMasterRTFindModuleByName ( bmd, bmn, &bmsg );
            break;
        case BUSBYTE_FIND_MODULE_BY_ID:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_FIND_MODULE_BY_ID: Client \"%s\" is asking for a module id -> name\n",bmn->moduleName );
            err = BusMasterRTFindModuleById   ( bmd, bmn, &bmsg );
            break;
        case BUSBYTE_FIND_TYPE_BY_NAME:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_FIND_TYPE_BY_NAME: Client \"%s\" is asking for a type name -> id\n", bmn->moduleName );
            err = BusMasterRTFindTypeByName   ( bmd, bmn, &bmsg );
            break;
        case BUSBYTE_FIND_TYPE_BY_ID:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_FIND_TYPE_BY_ID: Client \"%s\" is asking for a type id -> name\n", bmn->moduleName );
            err = BusMasterRTFindTypeById     ( bmd, bmn, &bmsg );
            break;
        case BUSBYTE_WHOS_CONNECTED:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_WHOS_CONNECTED: Client \"%s\" is asking for a list of everyone connected.\n", bmn->moduleName );
            err = BusMasterRTTellWhosConnected ( bmd, bmn, &bmsg );
            break;
        case BUSBYTE_MODULE_INFO_BY_ID:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_MODULE_INFO_BY_ID: Client \"%s\" is asking for module info by ID.\n", bmn->moduleName );
            err = BusMasterRTGetModuleInfoById ( bmd, bmn, &bmsg );
            break;
        case BUSBYTE_MODULE_INFO_BY_NAME:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_MODULE_INFO_BY_NAME: Client \"%s\" is asking for module info by name.\n", bmn->moduleName );
            err = BusMasterRTGetModuleInfoByName ( bmd, bmn, &bmsg );
            break;

        case BUSBYTE_REGISTER_TYPE:
            {
            int typeId;
            char tmp_buf[32];

            debug1 ( DEBUG_RESPONDER, "BUSBYTE_REGISTER_TYPE: Client \"%s\" is registering type ",bmn->moduleName );

            sscanf ( bmsg.message, "%d", &typeId );
            debug1 ( DEBUG_RESPONDER, "%d with the bus\n",typeId );
            err = BusMasterAddRegisteredType ( bmn, typeId );

            if ( err != SBUSERROR_NOT )
                {
                sprintf ( tmp_buf,"-1" );
                }
            else
                {
                sprintf ( tmp_buf,"%d", typeId );
                }
            BusReplyBusByte ( bmn->fd, bmsg.serial, MASTERID, bmn->moduleId,
                              BUSBYTE_REGISTER_TYPE, strlen ( tmp_buf )+1, tmp_buf );
            }
        break;
        case BUSBYTE_MODULE_LEAVING:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_MODULE_LEAVING: Client \"%s\" is disconnecting from the bus.\n",
                     bmn->moduleName );
            BusMasterRTModuleLeaving ( bmd, bmn );
            break;
        case BUSBYTE_I_DONT_UNDERSTAND:
            debug0 ( DEBUG_RESPONDER, "WARNING: bus byte sent to client wasn't understood!!!\n" );
            break;
        case BUSBYTE_REQ_DIRECT_CONNECT:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_REQ_DIRECT_CONNECT: Client \"%s\" is requesting direct connection. \n", bmn->moduleName );
            err = BusMasterRTReqDirect ( bmd, bmn, &bmsg );
            break;
        case BUSBYTE_REPLY_DIRECT_CONNECT:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_REPLY_DIRECT_CONNECT: Client \"%s\" is responding to direct connection request.\n", bmn->moduleName );
            err = BusMasterRTReplyDirect ( bmd, bmn, &bmsg );
            break;
        case BUSBYTE_KILL_CLIENT:
            debug1 ( DEBUG_RESPONDER, "BUSBYTE_KILL_CLIENT: Client \"%s\" asking to terminate a module. \n", bmn->moduleName );
            err = BusMasterRTKillClient ( bmd, bmn, &bmsg );
            break;
        default:
            debug1 ( DEBUG_RESPONDER, "Did not understand bus byte from client \"%s\"!!!\n",
                     bmn->moduleName );
            err = BusReplyBusByte ( bmn->fd, bmsg.serial, bmn->moduleId, MASTERID,
                                    BUSBYTE_I_DONT_UNDERSTAND, 0, NULL );
            err = SBUSERROR_MSG_NOT_UNDERSTOOD;
        }
    if ( bmsg.messageLength > 0 )
        {
        free ( bmsg.message );
        }

#if defined(TIME_BM_RU) || defined(TIME_BM_HR) || defined(TIME_BM_TIMES)
    t_stop();
    servTime = t_getutime();
    printf ( "***** Service time = %e  Read time = %e \n", servTime, readTime );
#endif

    return err;
    }

void debugBusMasterData ( struct BusMasterData *bmd )
    {
    struct BusTypeNode *btn;
    struct BusModuleNode *bmn;

    if ( bmd == NULL )
        {
        printf ( "NULL BusMasterData pointer ERROR\n" );
        fflush ( stdout );
        return;
        }
    printf ( "Acceptor socket is descriptor %d\n",bmd->portSocket );

    if ( bmd->Types != NULL )
        {
        btn = bmd->Types;
        printf ( "Types:  " );
        fflush ( stdout );
        do
            {
            printf ( "%3d = %s  ",btn->typeId,btn->typeName );
            fflush ( stdout );
            btn = btn->next;
            }
        while ( ( btn != NULL ) && ( btn != bmd->Types ) );

        if ( btn == NULL )
            printf ( " -grounded list\n" );
        else
            printf ( " -circular list\n" );
        fflush ( stdout );
        }
    else
        {
        printf ( "No types known by bus\n" );
        fflush ( stdout );
        }

    if ( bmd->Modules != NULL )
        {
        struct BusRegisteredTypeNode *brtn;

        bmn = bmd->Modules;
        printf ( "Modules Connected:\n" );
        fflush ( stdout );
        do
            {
            printf ( "\t%3d=%s, ",bmn->moduleId, bmn->moduleName );
            fflush ( stdout );

            printf ( "fd=%d,",bmn->fd );
            printf ( "from %s:%d",
                     inet_ntoa ( bmn->sin.sin_addr ),
                     ntohs ( bmn->sin.sin_port ) );
            printf ( " Understands: " );
            fflush ( stdout );

            if ( bmn->Types != NULL )
                {
                brtn = bmn->Types;

                while ( brtn != NULL )
                    {
                    printf ( "%d",brtn->typeId );
                    fflush ( stdout );

                    btn = BusMasterFindTypeById ( bmd->Types,brtn->typeId );
                    if ( btn != NULL )
                        printf ( "=%s, ",btn->typeName );
                    else
                        printf ( "=???, " );
                    fflush ( stdout );

                    brtn = brtn->next;
                    }
                }
            else
                {
                printf ( "Nothing." );
                fflush ( stdout );
                }

            printf ( " " );
            fflush ( stdout );

            if ( bmn->Messages == NULL )
                {
                printf ( "No messages.\n" );
                fflush ( stdout );
                }
            else
                {
                struct BusMessageQueue *bmq;
                int count=0;

                bmq = bmn->Messages;
                do
                    {
                    count ++;
                    bmq = bmq->next;
                    }
                while ( bmq && bmq != bmn->Messages );

                printf ( "%d message(s)", count );
                fflush ( stdout );

                if ( bmq == NULL )
                    {
                    printf ( " in a grounded queue.\n" );
                    fflush ( stdout );
                    }
                else if ( bmq == bmn->Messages )
                    {
                    printf ( " in a circular queue.\n" );
                    fflush ( stdout );
                    }
                else
                    {
                    printf ( " in a DEAD MESS!!!\n" );
                    fflush ( stdout );
                    }
                }

            bmn = bmn->next;
            }
        while ( ( bmn != NULL ) && ( bmn != bmd->Modules ) );

        if ( bmn == NULL )
            {
            printf ( "Grounded Module List\n" );
            fflush ( stdout );
            }
        else
            {
            printf ( "Circular Module List\n" );
            fflush ( stdout );
            }
        }
    else
        {
        printf ( "No connected Modules\n" );
        fflush ( stdout );
        }

    if ( bmd->open_action != NULL )
        {
        printf ( "open_action set\n" );
        fflush ( stdout );
        }
    if ( bmd->close_action != NULL )
        {
        printf ( "close_action set\n" );
        fflush ( stdout );
        }
    }

void BusMasterDispatcher ( struct BusMasterData *bmd,
                           int fd )
    {
    struct BusModuleNode *bmn;
    int err;

    debug1 ( DEBUG_RESPONDER,"Incoming data from descriptor %d\n",fd );

    bmn = BusMasterFindModuleByFd ( bmd->Modules, fd );

    if ( bmn == NULL )
        {
        debug0 ( DEBUG_RESPONDER,"Checking for incoming connection?\n" );
        err = BusMasterCheckForNewConnection ( bmd, fd );
        }
    else
        {
        debug1 ( DEBUG_RESPONDER,"Responding to message from client %s\n",
                 bmn->moduleName );
        err = BusMasterResponder ( bmd, bmn );
        }
    /* debugBusMasterData( bmd ); */
    }



