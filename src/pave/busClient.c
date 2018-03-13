/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busClient.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  - busClient.c
 *
 *       Client.c is a collection of service functions and data structures
 *       for the software bus client.
 *
 *     NETWORKING:        remote/network connectivity is via bus library (sockets)
 *
 *     COMPILATION:       (see makefile)
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *                Direct connection setup has to be updated
 *                to use BusSendBusByte instead of BusWriteCharacter
 *
 ********************************************************************
 * REVISION HISTORY - busClient.c
 *
 * Date: 14-Dec-94
 * Version: 0.1
 * Change Description: ORIGINAL CODE
 * Change author: Leland Morrison, NCSU, CSC
 *
 * Date: 15-Dec-94
 * Version: 0.2
 * Change Description: Added main headers
 * Change author: M. Vouk, NCSU, CSC
 *
 * Date: 28-Dec-94
 * Version: 0.2.1
 * Change Description: Modifying the protocol for sending and receiving
 *             messages between busMaster and clients.
 *             Changed calls to BusWriteCharacter to BusSendBusByte.
 * Change author: R. Balay, NCSU, CSC
 *
 * Date: 17-Jan-95
 * Version: 0.3
 * Change Description: Direct connections and FindWhosConnected are working.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 27-Jan-95
 * Version: 0.3a
 * Change Description: Modified the parameters of function BusSendMessage
 *                     from 'struct BusMessage' to 'struct BusMessage *'.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 18-March-95
 * Version: 0.4
 * Change Description: Added functions "BusModuleInfoByName" and
 *                     "BusModuleInfoById". A variable "masterAlive" is
 *             used to keep track if the BusMaster is alive. The
 *             client does not exit if the BusMaster dies.
 * Change author: R. Balay
 *
 * Date: 6-April-95
 * Version: 0.5
 * Change Description: The client on initialization creates a file
 *             in /tmp where it writes its process Id. An exit
 *             handler has been added to the client routines to
 *             cleanup /tmp files when the client exits.
 *             Corrected a bug in BusDispatch when messages arrive
 *             while processing another message. The message from
 *             RecvdQueue is first processed and then Dequed.
 * Change author: R. Balay
 *
 * Date: 12-Apr-95
 * Version: 0.5a
 * Change Description: Added the function BusKillClient that allows only
 *                     the Console to kill a client on the Bus
 * Change author: R. Balay
 *
 * Date: 27-Apr-95
 * Version: 0.5b
 * Change Description: BusInitialize returns SBUSERROR_DUP_CLIENT if client
 *                     with the same name already exists
 * Change author: R. Balay
 *
 * Date: 9-July-95
 * Version: 0.5c
 * Change Description: Replaced function "cuserid" with "getMyuserid()"
 * Change author: R. Balay
 *
 * Date: 24-July-95
 * Version: 0.6a
 * Change Description: BusInitialize takes extra parameters to set exitCallback
 *            on timeout
 * Change author: R. Balay
 *
 * Date: 19-Aug-95
 * Version: 0.6b
 * Change Description: BusInitialize does not create the /tmp files and
 *                    changed function defns to use const char *
 * Change author: R. Balay
 *
 * Date: 7-Sept-95
 * Version: 0.6c
 * Change Description: Encapsulated the code for processing RecvdMessages into
 *        a function
 * Change author: R. Balay
 *
 * Date: 15-Aug-96
 * Version: 0.7
 * Change Description: Clearer debug statements to help gather timestamp info.
 *                     Return value of BusGetResponse is checked by functions
 *                      before returning, avoiding accessing memory that has'nt
 *                      been allocated.
 * Change author: R. Balay
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "busClient.h"
#include "busSocket.h"
#include "busRW.h"
#include "busRWMessage.h"
#include "busMsgQue.h"
#include "busRepReq.h"
#include "busError.h"
#include "busDebug.h"
#include "busXtClient.h"
#include "busVersion.h"

int clientNotDone;
extern int masterAlive;


int BusMakeConnection ( int *s )
/* use the information in the environment variable SBUSHOST and SBUSPORT
   to make the underlying connection to the bus master */
/* s - in - a pointer to a integer to store the
   file descriptor for the socket connection to the bus master in */
    {
    char *host_addr;
    char *port;
    int portNumber;

    host_addr = getenv ( SBUSHOST );
    port      = getenv ( SBUSPORT );

    if ( port == NULL )
        return SBUSERROR_SBUSPORT_NOT_SET;

    portNumber = atoi ( port );

    *s = busSocket_makeConnection ( host_addr, portNumber );

    if ( *s > 0 )
        return SBUSERROR_NOT;
    else
        return *s;
    }

/* For the X-Clients */
struct BusXtData xbd  /* { XtInputId xtid; } */;

/* Interrupt handler function for Client modules */
void BusClientExitHandler ( int signo )
    {
    /*    printf("In exit handler \n"); */
    clientNotDone = 0;
    }

int BusInitialize ( struct BusData *bd, int setTimeout, int minsToLive,
                    void ( *exitCallback ) ( struct BusData *bd ) )
/* Initialize the BusData structure,
   create the socket connection to the bus master,
   and give the bus master the necessary information about the client
   */
    {
    int err, bus_pid;
    FILE *procfd;
    char pid_fname[256] ;
    struct sigaction act;
    struct timeval timeout;

    masterAlive = 0;
    clientNotDone = 1;

    bd->nextSeqNum = -1;
    bd->Messages = NULL;
    bd->RecvdMessages = NULL; /* Added by Rajini  */
    bd->xtd = &xbd;
    /* bd->xtd = NULL; */

    bd->  TypeCallbacks=NULL;
    bd->DirectCallbacks=NULL;
    bd-> InputCallbacks=NULL;
    bd->TimeoutCallback = NULL;

    err = BusMakeConnection ( & ( bd->fd ) );
    if ( err != SBUSERROR_NOT )
        return err;

    /* Now tell the Bus Master what it wants to know about
       this client */
    err = BusWriteASCIIZ ( bd->fd, bd->name );
    if ( err == SBUSERROR_NODATA )
        {
        sleep ( 1 );
        err = BusWriteASCIIZ ( bd->fd, bd->name );
        }
    if ( err != SBUSERROR_NOT )
        {
        close ( bd->fd );
        bd->fd = -1;
        return err;
        }

    err = BusReadInteger ( bd->fd, & ( bd->moduleId ) );
    if ( err == SBUSERROR_NODATA )
        {
        sleep ( 1 );
        err = BusReadInteger ( bd->fd, & ( bd->moduleId ) );
        return err;
        }
    if ( err != SBUSERROR_NOT )
        {
        close ( bd->fd );
        bd->fd = -1;
        return err;
        }
    if ( bd->moduleId == FIND_ID_ERR )
        {
        printf ( "Cannot attach to bus - module with name : %s already exists !! \n",
                 bd->name );
        return SBUSERROR_DUP_CLIENT;
        }

    bus_pid = getpid();

#ifdef SBUS_CREATE_TMPFILE
    sprintf ( pid_fname, "/tmp/sbus_%d_%s", bd->moduleId, BusGetMyUserid() );
    if ( ( procfd = fopen ( pid_fname, "w+" ) ) )
        {
        fprintf ( procfd,"%d",bus_pid );
        fclose ( procfd );
        }
    else
        {
        sprintf ( s,"Could not open %s", pid_fname );
        perror ( s );
        }
#endif

#if defined(TIME_ST_BUS) || defined(TIME_IAT)
    BusTSInitFile ( bd->name, bd->moduleId );
#endif

    act.sa_handler = BusClientExitHandler;
    memset ( & ( act.sa_mask ), 0, sizeof ( sigset_t ) );

    /* allow client to be killed by signal action */
    act.sa_flags = 0;
    sigaction ( SIGKILL, &act, NULL );
    sigaction ( SIGINT, &act, NULL );
    sigaction ( SIGQUIT, &act, NULL );

    /* Set the exit-timeout */
    if ( setTimeout > 0 )
        {
        timeout.tv_sec = 60*minsToLive;
        timeout.tv_usec = 10;
        BusAddTimeoutCallback ( bd, &timeout, exitCallback );
        }

    masterAlive = 1;
    return SBUSERROR_NOT;
    }

void BusClose ( struct BusData *bd )
/* Tell the bus master that this client is leaving,
   close the socket connection,
   and it should free up memory being used by bus data */
    {
    char procfname[256];

    debug0 ( DEBUG_CLIENT,"BusClose : Module leaving \n" );
    BusSendBusByte ( bd, MASTERID, BUSBYTE_MODULE_LEAVING, 0, NULL );
    shutdown ( bd->fd, 2 );
    close ( bd->fd );

#ifdef SBUS_CREATE_TMPFILE
    sprintf ( procfname, "/tmp/sbus_%d_%s", bd->moduleId, BusGetMyUserid() );
    if ( remove ( procfname ) )
        printf ( "Could not remove %s\n",procfname );
    else
        printf ( "Removed %s\n",procfname );
#endif

#if defined(TIME_ST_BUS) || defined(TIME_IAT)
    BusTSCloseFile();
#endif
    }

/* This function should be called only by the "Console" to kill
   other client modules attached to the Bus */
int BusKillClient ( struct BusData *bd, const char *name )
    {
    int err;

    debug1 ( DEBUG_CLIENT, "BusKillClient : %s \n", name );
    if ( BusFindModuleByName ( bd, "Console" ) != bd->moduleId )
        return SBUSERROR_GENERAL_FAILURE;

    err = BusSendBusByte ( bd, MASTERID, BUSBYTE_KILL_CLIENT,
                           strlen ( name )+1, ( char * ) name );
    debug0 ( DEBUG_CLIENT, "BusKillClient : Returning \n" );
    return err;
    }

/* This is the data struct used to store the
   type callbacks in a circularly linked list */
struct BusTypeCallbackNode
    {
    struct BusTypeCallbackNode *next;

    int typeId;
    void ( *callback ) ( struct BusData *, struct BusMessage * );
    };


int BusAddTypeCallback ( struct BusData *bd, int type,
                         void ( *callback ) ( struct BusData *,struct BusMessage * ) )
/* Add a callback to handle a given type of message received.
   bd is a pointer to the BusData structure sent to BusInitialize()
   type is the type number gotten from BusFindTypeByName()
   callback is the function to call when a message with the specified type
   is received.
   */
    {
    struct BusTypeCallbackNode *btcn;

    if ( bd == NULL )
        return SBUSERROR_BADPARAMETER;
    if ( type<0 )
        return SBUSERROR_BADPARAMETER;
    if ( callback==NULL )
        return SBUSERROR_BADPARAMETER;

    btcn = ( struct BusTypeCallbackNode * )
           malloc ( sizeof ( struct BusTypeCallbackNode ) );

    if ( btcn == NULL )
        return SBUSERROR_NOMEMORY;

    btcn -> next     = NULL;
    btcn -> typeId   = type;
    btcn -> callback = callback;

    if ( bd->TypeCallbacks != NULL )
        {
        btcn->next = bd->TypeCallbacks;
        bd->TypeCallbacks = btcn;
        }
    else
        bd->TypeCallbacks = btcn;

    return SBUSERROR_NOT;
    }

int BusRemoveTypeCallback ( struct BusData *bd, int type,
                            void ( *callback ) ( struct BusData *,
                                    struct BusMessage * ) )
/* This function removes a callback from the set of callbacks
   associated with a given type */
    {
    struct BusTypeCallbackNode *prev, *btcn;

    if ( bd == NULL )
        return SBUSERROR_BADPARAMETER;
    if ( type<0 )
        return SBUSERROR_BADPARAMETER;

    prev = NULL;
    btcn = bd->TypeCallbacks;
    while ( ( btcn!=NULL ) && ( btcn->typeId!=type ) && ( btcn->callback!=callback ) )
        {
        prev = btcn;
        btcn = btcn -> next;
        }

    if ( btcn == NULL )
        return SBUSERROR_CBNOTFOUND;
    else

        {
        if ( prev )
            prev->next = btcn->next;
        else
            bd->TypeCallbacks = btcn->next;

        btcn->next = NULL;
        free ( btcn );

        return SBUSERROR_NOT;
        }
    }

/* This is used to store the information about the function to
   call when a direct connection associated with a type is
   created. */
struct BusDirectCallbackNode
    {
    struct BusDirectCallbackNode *next;

    int typeId;
    void ( *callback ) ( int id, char *data );
    char  *data;
    };

int BusAddDirectCallback ( struct BusData *bd,
                           int typeId,
                           void ( *callback ) ( int id, char *data ),
                           char *data )
/* add a callback to respond when a request for a point-to-point
   connection is made
   bd - a pointer to the data structure sent to BusInitialize
   typeId - the "type" of data which will be sent between the clients
   callback - the function to call when a point-to-point connection
              associated with the given type is formed
   data - an arbitrary pointer that is passed to the callback
          function when it is called
      */
    {
    struct BusDirectCallbackNode *bdcn;

    if ( callback == NULL )
        return SBUSERROR_BADPARAMETER;

    bdcn = ( struct BusDirectCallbackNode * )
           malloc ( sizeof ( struct BusDirectCallbackNode ) );
    if ( bdcn != NULL )
        {
        bdcn->next = bd->DirectCallbacks;
        bd->DirectCallbacks = bdcn;
        bdcn -> typeId      = typeId;
        bdcn -> callback    = callback;
        bdcn -> data        = data;

        return SBUSERROR_NOT;
        }
    else
        return SBUSERROR_NOMEMORY;
    }

int BusRemoveDirectCallback ( struct BusData *bd,
                              int typeId,
                              void ( *callback ) ( int,char * ) )
/* removes a type from the list of types for which this client
   will respond to a request for a point-to-point connection */
    {
    struct BusDirectCallbackNode *prev,*bdcn;

    bdcn = bd->DirectCallbacks;
    prev = NULL;

    while ( bdcn && ( bdcn->typeId != typeId ) &&
            ( bdcn->callback != callback ) )
        {
        prev = bdcn;
        bdcn = bdcn->next;
        }

    if ( bdcn != NULL )
        {
        if ( prev != NULL )
            prev->next = bdcn->next;
        else
            bd->DirectCallbacks = bdcn->next;

        free ( bdcn );
        return SBUSERROR_NOT;
        }
    else
        return SBUSERROR_CBNOTFOUND;
    }

void BusDResetConnection ( struct BusData *bd )
/* This function should attempt to clear the connection to the
   bus master of error status and extraneous data

   bd - a pointer to the data structure sent to BusInitialize
*/
    {
    char buffer[80];
    int x;

    sleep ( 1 );

    /* read all data from input */
    for ( x=1; x>0; )
        x = read ( bd->fd, buffer, 80 );
    }

void BusProcessMessage ( struct BusData *bd, struct BusMessage *bmsg )
/*
   Process the message read from the bus master by looking for
   (and calling if found)
   a callback function associated with the message's type */
    {
    struct BusTypeCallbackNode *btcn;

    debug2 ( DEBUG_CLIENT, "BusProcessMessage : Processing message with seq %d from %d \n",
             bmsg->serial, bmsg->fromModule );

#ifdef BUS_WITH_ACK
    BusReplyBusByte ( bd->fd, bmsg->serial, bmsg->fromModule, bd->moduleId,
                      BUSBYTE_DEQUE_MESSAGE, 0,  NULL );
#endif

    btcn = bd->TypeCallbacks;
    while ( btcn!=NULL )
        {
        if ( ( btcn->typeId == bmsg->messageType ) || ( btcn->typeId == -1 ) )
            ( btcn->callback ) ( bd, bmsg );
        btcn = btcn->next;
        }
    /*
       debug1(DEBUG_CLIENT, "BusProcessMessage : Sending BUSBYTE_DEQUE_MESSAGE for seq %d \n",
            bmsg->serial);
       BusReplyBusByte( bd->fd, bmsg->serial, bmsg->fromModule, bd->moduleId,
            BUSBYTE_DEQUE_MESSAGE, 0,  NULL );
    */
    debug0 ( DEBUG_CLIENT, "BusProcessMessage : Returning \n" );
    }

void BusDPassMessage ( struct BusData *bd )
/* Send a que'd message (that is from the client) to the bus master */
    {
    struct BusMessage *bmsg;

    bmsg = BusGetFirstMessage ( bd->Messages );

    if ( bmsg != NULL )
        {
        BusWriteMessage ( bd->fd, bmsg );
        }
    }

/*
void DirectInputCallback( void (*callback)(int), int fd)
{
   (callback)(fd);
   shutdown(fd, 2);
   close(fd);
}
*/

void BusDHandleDirect ( struct BusData *bd, struct BusMessage *bmsg )
/* check for a callback function that can handle the
   request for a message over a point-to-point connection,
   if not function to handle it is found, deny the request,
   otherwise, setup the connection, and call the function found */
    {
    struct BusDirectCallbackNode *bdcn;
    int typeId, port;
    char host_addr[512];
    unsigned char busByte;

    sscanf ( bmsg->message, "%d %d %s", &typeId, &port, host_addr );
    debug1 ( DEBUG_CLIENT,"BusHandleDirect : Request for type %d\n", typeId );

    bdcn = bd->DirectCallbacks;
    while ( bdcn && ( bdcn->typeId != typeId ) )
        bdcn = bdcn -> next;

    if ( bdcn )
        {
        int   s;

        /* printf("Attempting to connect to %s:%d\n",host_addr,port); */

        s = busSocket_makeConnection ( host_addr, port );
        /* debug3(DEBUG_CLIENT, "BusHandleDirect : Connection made to %s:%d on socket %d \n", host_addr, port, s); */

        if ( s>0 )
            {
            busByte = BUSBYTE_DIRECT_CONNECT_OK;
            debug1 ( DEBUG_CLIENT,"BusDirectConnReply : Replying to %d \n",
                     bmsg->fromModule );
            BusReplyBusByte ( bd->fd, bmsg->serial, bmsg->fromModule, bd->moduleId,
                              BUSBYTE_REPLY_DIRECT_CONNECT, 2, ( char * ) &busByte );
            /* debug0(DEBUG_CLIENT,"BusHandleDirect : Invoking the Callback fn \n");*/
            ( bdcn->callback ) ( s,bdcn->data );
            /* debug0(DEBUG_CLIENT,"BusHandleDirect : Callback fn returned \n"); */
            shutdown ( s,2 );
            close ( s );
            debug0 ( DEBUG_CLIENT,"BusHandleDirect : Returning CONNECT_OK\n" );
            }
        else
            {
            busByte = BUSBYTE_DIRECT_CONNECT_FAIL;
            debug1 ( DEBUG_CLIENT,"BusDirectConnReply : Replying to %d \n",
                     bmsg->fromModule );
            BusReplyBusByte ( bd->fd, bmsg->serial, bmsg->fromModule, bd->moduleId,
                              BUSBYTE_REPLY_DIRECT_CONNECT, 2, ( char * ) &busByte );
            debug0 ( DEBUG_CLIENT,"BusHandleDirect : Returning CONNECT_FAIL\n" );
            }

        }
    else
        {
        busByte = BUSBYTE_DIRECT_CONNECT_FAIL;
        debug1 ( DEBUG_CLIENT,"BusDirectConnReply : Replying to %d \n",
                 bmsg->fromModule );
        BusReplyBusByte ( bd->fd, bmsg->serial, bmsg->fromModule, bd->moduleId,
                          BUSBYTE_REPLY_DIRECT_CONNECT, 2, ( char * ) &busByte );
        debug0 ( DEBUG_CLIENT,"BusHandleDirect : Returning CONNECT_FAIL\n" );
        }
    }

void BusProcessRecvdMessages ( struct BusData *bd )
    {
    while ( bd->RecvdMessages != NULL )
        {
        struct BusMessage *tmp_bmsg;

        debug0 ( DEBUG_DISP,"BusDispatch : Processing message from RecvdMessages queue \n" );
        tmp_bmsg = BusGetFirstMessage ( bd->RecvdMessages );
        BusProcessOption ( bd, tmp_bmsg );

        bd->RecvdMessages =
            Bus_DequeMessage ( bd->RecvdMessages );
        }
    return;
    }

int BusDispatch ( struct BusData *bd )
/* Read a "command" from the bus master, and calls a function
to take an appropriate action */
    {
    int err;
    struct BusMessage bmsg;
#ifdef TIME_BUS
    hrtime_t start_time, end_time, time1, time2;

    time1 = gethrtime();
#endif
    err = BusReadMessage ( bd->fd, &bmsg );
#ifdef TIME_BUS
    time2 = gethrtime();
#endif

    if ( err != SBUSERROR_NOT )
        {
        /* connection has probably crashed */
        /*
        if (err == SBUSERROR_NODATA)
        {
          perror("Bus Crash???");
          BusClose( bd );
          exit(0);
        }
             else
        {
          perror("Connection to bus has crashed...");
          exit(-1);
        }
            */
        return err;
        }
#ifdef TIME_BUS
    start_time = gethrtime();
#endif
    BusProcessOption ( bd, &bmsg );
#ifdef TIME_BUS
    end_time = gethrtime();
    printf ( "Message Read time = %lld Process time = %lld \n",
             ( time2-time1 ), ( end_time-start_time ) );
#endif
    if ( bmsg.messageLength > 0 )
        {
        free ( bmsg.message );     /* Code added by Rajini */
        }
    BusProcessRecvdMessages ( bd );
    return SBUSERROR_NOT;
    }

/* This function checks the messageOption field of the message and
   process the message accordingly.
 */
void BusProcessOption ( struct BusData *bd, struct BusMessage *bmsg )
    {
    int err;

    switch ( bmsg->messageOption )
        {
        case BUSBYTE_RESET_CONNECTION:
            debug0 ( DEBUG_DISP, "BusProcessOption : Got BUSBYTE_RESET_CONNECTION.\n" );
            BusDResetConnection ( bd );
            break;
        case BUSBYTE_MESSAGE_HERE:
            debug0 ( DEBUG_DISP, "BusProcessOption : Got BUSBYTE_MESSAGE_HERE.\n" );
            /*
            if (DEBUG_DISP)
               debugShowMessage(bmsg);
            */
            BusProcessMessage ( bd, bmsg );
            break;
        case BUSBYTE_PLEASE_ACKNOWLEDGE:
            debug0 ( DEBUG_DISP, "BusProcessOption : Got BUSBYTE_PLEASE_ACKNOWLEDGE\n" );
            BusReplyBusByte ( bd->fd, bmsg->serial, MASTERID, bd->moduleId,
                              BUSBYTE_ACKNOWLEDGEMENT, 0, NULL );
            break;
        case BUSBYTE_GET_FIRST_MESSAGE:
            debug0 ( DEBUG_DISP, "BusProcessOption : Got BUSBYTE_GET_FIRST_MESSAGE.\n" );
            BusDPassMessage ( bd );
            break;
        case BUSBYTE_DEQUE_MESSAGE:
            debug0 ( DEBUG_DISP, "BusProcessOption : Got BUSBYTE_DEQUE_MESSAGE.\n" );
            bd->Messages =
                Bus_DequeMessageBySeq ( bd->Messages, bmsg->serial );
            break;
        case BUSBYTE_REQ_DIRECT_CONNECT:
            debug0 ( DEBUG_DISP, "BusProcessOption : Got BUSBYTE_REQ_DIRECT_CONNECT.\n" );
            BusDHandleDirect ( bd, bmsg );
            break;
        case BUSBYTE_I_DONT_UNDERSTAND:
            debug0 ( DEBUG_DISP, "WARNING: bus byte sent to bus master wasn't understood!!!\n" );
            break;
        case BUSBYTE_KILL_CLIENT:
            debug0 ( DEBUG_DISP, "BusProcessOption : Got BUSBYTE_KILL_CLIENT.\n" );
            printf ( "%s: Recieved a DIE message from BusMaster !!! Shutting Down ! \n", bd->name );
            fflush ( stdout );
            BusClose ( bd );
            exit ( 1 );
            break;
        default:
            debug0 ( DEBUG_DISP, "WARNING: Did not understand byte from bus master!!!\n" );

            err = BusSendBusByte ( bd, MASTERID, BUSBYTE_I_DONT_UNDERSTAND,
                                   0, NULL );
            if ( err != SBUSERROR_MASTER_ABSENT )
                {
                BusDResetConnection ( bd );
                BusSendBusByte ( bd, MASTERID, BUSBYTE_RESET_CONNECTION,
                                 0, NULL );
                }
        }
    }



int BusFindModuleByName ( struct BusData *bd, const char *name )
/* Given a client's name, return its id number
   bd is a pointer to the data struct sent to BusInitialize
   name is the name of the client to attempt to find it id */
    {
    int id;
    char *response;
    int reqNum;
    int err;

    debug1 ( DEBUG_CLIENT, "BusFindModuleByName : %s \n", name );
    reqNum = BusSendBusByte ( bd, MASTERID,
                              BUSBYTE_FIND_MODULE_BY_NAME, strlen ( name )+1, ( char * ) name );

    if ( reqNum != SBUSERROR_MASTER_ABSENT )
        {
        err = BusGetResponse ( bd, reqNum, BUSBYTE_FIND_MODULE_BY_NAME, &response );
        if ( err != SBUSERROR_NOT ) return FIND_ID_ERR;
        sscanf ( response, "%d", &id );
        debug2 ( DEBUG_CLIENT, "BusFindModuleByName : Returning ID of %s = %d \n",
                 name, id );
        free ( response );
        return id;
        }
    else
        {
        debug2 ( DEBUG_CLIENT, "BusFindModuleByName : Returning ID of %s = %d \n",
                 name, FIND_ID_ERR );
        return FIND_ID_ERR;
        }
    }

char *BusFindModuleById ( struct BusData *bd, int clientId )
/* Given a client's id number, return its name
   The pointer returned should be free'd when no longer used
   bd is a pointer to the data structure sent to BusInitialize,
   clientId is the id number of the client */
    {
    char *name;
    int reqNum;
    char tmp_buf[32];
    int err;

    name = NULL;

    debug1 ( DEBUG_CLIENT, "BusFindModuleById : %d \n", clientId );
    sprintf ( tmp_buf, "%d", clientId );
    reqNum = BusSendBusByte ( bd, MASTERID, BUSBYTE_FIND_MODULE_BY_ID,
                              strlen ( tmp_buf )+1, tmp_buf );

    if ( reqNum != SBUSERROR_MASTER_ABSENT )
        {
        err = BusGetResponse ( bd, reqNum, BUSBYTE_FIND_MODULE_BY_ID, &name );

        if ( err != SBUSERROR_NOT )
            {
            name = malloc ( 5 );
            sprintf ( name, "%s", FIND_NAME_ERR );
            debug2 ( DEBUG_CLIENT,"BusFindModuleById : Returning Name of %d = %s \n",
                     clientId, name );
            return name;
            }
        debug2 ( DEBUG_CLIENT, "BusFindModuleById : Returning Name of %d = %s \n",
                 clientId, name );
        return name;
        }
    else
        {
        name = malloc ( 5 );
        sprintf ( name, "%s", FIND_NAME_ERR );
        debug2 ( DEBUG_CLIENT, "BusFindModuleById : Returning Name of %d = %s \n",
                 clientId, name );
        return name;
        }
    }

int BusFindTypeByName ( struct BusData *bd, const char *name )
/* Given a type's name, find its id number
   bd is a pointer to the structure sent to BusInitialize
   name is the name of the type */
    {
    int id;
    int reqNum;
    char *response;
    int err;

    debug1 ( DEBUG_CLIENT, "BusFindTypeByName :  %s \n", name );
    reqNum = BusSendBusByte ( bd, MASTERID,
                              BUSBYTE_FIND_TYPE_BY_NAME, strlen ( name )+1, ( char * ) name );

    if ( reqNum != SBUSERROR_MASTER_ABSENT )
        {
        err = BusGetResponse ( bd, reqNum, BUSBYTE_FIND_TYPE_BY_NAME, &response );

        if ( err != SBUSERROR_NOT ) return FIND_ID_ERR;
        sscanf ( response, "%d", &id );
        debug2 ( DEBUG_CLIENT, "BusFindTypeByName : Returning ID of %s = %d \n",
                 name, id );
        free ( response );
        return id;
        }
    else
        {
        debug2 ( DEBUG_CLIENT, "BusFindTypeByName : Returning ID of %s = %d\n",
                 name, FIND_ID_ERR );
        return FIND_ID_ERR;
        }
    }

char *BusFindTypeById ( struct BusData *bd, int id )
/* Given a type's id number, look up the type's name
   The pointer returned should be freed when no longer used
   bd is a pointer to the structure sent to BusInitialize
   id is the id number of the type */
    {
    char *name;
    int reqNum;
    char tmp_buf[32];
    int err;

    debug1 ( DEBUG_CLIENT, "BusFindTypeById : %d \n", id );
    name = NULL;
    sprintf ( tmp_buf, "%d", id );
    reqNum = BusSendBusByte ( bd, MASTERID, BUSBYTE_FIND_TYPE_BY_ID,
                              strlen ( tmp_buf )+1, tmp_buf );

    if ( reqNum != SBUSERROR_MASTER_ABSENT )
        {
        err = BusGetResponse ( bd, reqNum, BUSBYTE_FIND_TYPE_BY_ID, &name );
        if ( err != SBUSERROR_NOT )
            {
            name = malloc ( 5 );
            sprintf ( name, "%s", FIND_NAME_ERR );
            }
        }
    else
        {
        name = malloc ( 5 );
        sprintf ( name, "%s", FIND_NAME_ERR );
        }
    debug2 ( DEBUG_CLIENT, "BusFindTypeById : Returning Name of %d = %s \n",
             id, name );
    return name;
    }

int BusRegisterType ( struct BusData *bd, int id )
/* Given a message sent by type, this adds a type to the
   set of types for which the message will be sent to the
   client given a match in the type.
   It doesn't make sense to call this unless a callback
   is/has been added for the type specified by id

   id is the type id number */
    {
    int reqNum, new_id;
    char *response;
    char tmp_buf[32];
    int err;


    sprintf ( tmp_buf, "%d", id );
    reqNum = BusSendBusByte ( bd, MASTERID,
                              BUSBYTE_REGISTER_TYPE, strlen ( tmp_buf )+1, tmp_buf );
    if ( reqNum != SBUSERROR_MASTER_ABSENT )
        {
        err = BusGetResponse ( bd, reqNum, BUSBYTE_REGISTER_TYPE, &response );
        if ( err != SBUSERROR_NOT ) return FIND_ID_ERR;
        sscanf ( response, "%d", &new_id );
        printf ( "BusRegisterType %d \n", id );
        return ( new_id );
        }
    else
        return FIND_ID_ERR;
    }

struct BusModuleData *BusFindWhosConnected ( struct BusData *bd, int *numModules )
/* Find the modules that are connected to the Bus */

    {
    char *response;
    int reqNum, i;
    char *token;
    struct BusModuleData *ModuleArray;
    int err;


    debug0 ( DEBUG_CLIENT, "BusFindWhosConnected \n" );
    reqNum = BusSendBusByte ( bd, MASTERID, BUSBYTE_WHOS_CONNECTED, 0, NULL );

    if ( reqNum == SBUSERROR_MASTER_ABSENT )
        {
        *numModules = 0;
        return NULL;
        }

    err = BusGetResponse ( bd, reqNum, BUSBYTE_WHOS_CONNECTED, &response );
    if ( err != SBUSERROR_NOT ) return NULL;
    /* sscanf(response, "%d", &id); */

    /* The first entry in the response contains the number of modules connected.
       Read that and malloc an array of BusModuleData */

    token = strtok ( response, " " );
    *numModules = atoi ( token );
    ModuleArray = ( struct BusModuleData * )
                  malloc ( *numModules * sizeof ( struct BusModuleData ) );

    i = 0;
    token = strtok ( NULL, " " );
    while ( token != NULL )
        {
        ModuleArray[i].moduleId = atoi ( token );

        token = strtok ( NULL, " " );
        ModuleArray[i].name = malloc ( strlen ( token )+1 );
        strcpy ( ModuleArray[i].name, token );

        token = strtok ( NULL, " " );
        ModuleArray[i].ip_addr = malloc ( strlen ( token )+1 );
        strcpy ( ModuleArray[i].ip_addr, token );

        token = strtok ( NULL, " " );
        i++;
        }
    if ( i != *numModules )
        printf ( "The information got about clients does not match with number of clients returned \n" );
    debug1 ( DEBUG_CLIENT, "BusFindWhosConnected : Returning %d modules \n",
             *numModules );
    return ModuleArray;
    }

int BusGetModuleInfoByName ( struct BusData *bd, const char *name, struct BusModuleData *moduleInfo )
/* Find information about a module given its name*/

    {
    char *response;
    int reqNum;
    char *token;
    int err;

    debug0 ( DEBUG_CLIENT,"BusGetModuleInfoByName \n" );
    reqNum = BusSendBusByte ( bd, MASTERID, BUSBYTE_MODULE_INFO_BY_NAME, strlen ( name )+1, ( char * ) name );

    if ( reqNum == SBUSERROR_MASTER_ABSENT )
        {
        return reqNum;
        }

    err = BusGetResponse ( bd, reqNum, BUSBYTE_MODULE_INFO_BY_NAME, &response );
    if ( err != SBUSERROR_NOT ) return err;

    /* Get the various fields of the BusModuleData structure for the response */
    token = strtok ( response, " " );
    if ( token != NULL )
        {
        moduleInfo->moduleId = atoi ( token );

        token = strtok ( NULL, " " );
        moduleInfo->name = malloc ( strlen ( token )+1 );
        strcpy ( moduleInfo->name, token );

        token = strtok ( NULL, " " );
        moduleInfo->ip_addr = malloc ( strlen ( token )+1 );
        strcpy ( moduleInfo->ip_addr, token );
        debug0 ( DEBUG_CLIENT,"BusGetModuleInfoByName : Returning \n" );
        return SBUSERROR_NOT;
        }

    /* Probably module does not exist : return an error */
    printf ( "BusGetModuleInfoByName : Master returned a null response \n" );
    debug0 ( DEBUG_CLIENT,"BusGetModuleInfoByName : Returning \n" );
    return SBUSERROR_GENERAL_FAILURE;
    }

int BusGetModuleInfoById ( struct BusData *bd, int moduleId, struct BusModuleData *moduleInfo )
/* Find information about module with id "moduleId" */

    {
    char *response;
    int reqNum;
    char *token;
    char tmp_buf[32];
    int err;

    debug0 ( DEBUG_CLIENT,"BusGetModuleInfoById \n" );
    sprintf ( tmp_buf, "%d", moduleId );
    reqNum = BusSendBusByte ( bd, MASTERID, BUSBYTE_MODULE_INFO_BY_ID, strlen ( tmp_buf )+1, tmp_buf );

    if ( reqNum == SBUSERROR_MASTER_ABSENT )
        {
        return reqNum;
        }

    err = BusGetResponse ( bd, reqNum, BUSBYTE_MODULE_INFO_BY_ID, &response );
    if ( err != SBUSERROR_NOT ) return err;

    /* Get the various fields of the BusModuleData structure */
    token = strtok ( response, " " );
    if ( token != NULL )
        {
        moduleInfo->moduleId = atoi ( token );

        token = strtok ( NULL, " " );
        moduleInfo->name = malloc ( strlen ( token )+1 );
        strcpy ( moduleInfo->name, token );

        token = strtok ( NULL, " " );
        moduleInfo->ip_addr = malloc ( strlen ( token )+1 );
        strcpy ( moduleInfo->ip_addr, token );
        debug0 ( DEBUG_CLIENT,"BusGetModuleInfoById : Returning \n" );
        return SBUSERROR_NOT;
        }

    /* Module probably does not exist : return an error */
    printf ( "BusGetModuleInfoById : Master returned null response \n" );
    debug0 ( DEBUG_CLIENT,"BusGetModuleInfoById : Returning \n" );
    return SBUSERROR_GENERAL_FAILURE;
    }

int BusSendMessage ( struct BusData *bd, struct BusMessage *bmsg )
/* Put a [generic] message from the client in the que to
   be sent to the bus master */
    {
    struct BusMessageQueue *bmq;
#ifdef TIME_BUS
    hrtime_t start_time, end_time, q_time, time1, time2, time3, time4;
#endif

#ifdef TIME_BUS
    start_time = gethrtime();
#endif

    /*
      debug2(DEBUG_CLIENT,"BusSendMessage : to %d of type %d \n",
                bmsg->toModule, bmsg->messageType);
    */
    /* fromModule should be overwritten by bus master, but... */
    bmsg->fromModule = bd->moduleId;
    bmsg->messageOption = BUSBYTE_MESSAGE_HERE;
    bmsg->serial = getSeqNum ( bd );

    /* put the message in the que */
#ifdef TIME_BUS
    time1 = gethrtime();
#endif

#ifdef BUS_WITH_ACK
    bmq = Bus_EnqueMessage ( bd->Messages, bmsg );
#endif

#ifdef TIME_BUS
    time2 = gethrtime();
#endif

#ifdef BUS_WITH_ACK
    /* and if necessary, tell the bus master a message is waiting */
    if ( bmq != NULL )
        {
#endif
        /*
        if (bd->Messages == NULL) {
        }
        */
        bmsg->messageOption = BUSBYTE_MESSAGE_HERE;
#ifdef TIME_BUS
        time3 = gethrtime();
#endif
        BusWriteMessage ( bd->fd, bmsg );
#ifdef TIME_BUS
        time4 = gethrtime();
#endif

#ifdef BUS_WITH_ACK
        bd->Messages = bmq;
#endif

#ifdef TIME_BUS
        end_time = gethrtime();
        printf ( "Total time = %lld Queue time = %lld Write time = %lld \n",
                 ( end_time-start_time ), ( time2-time1 ), ( time4-time3 ) );
#endif
        debug2 ( DEBUG_CLIENT,"BusSendMessage : to %d of type %d \n",
                 bmsg->toModule, bmsg->messageType );
        return SBUSERROR_NOT;
#ifdef BUS_WITH_ACK
        }
    else
        {
        printf ( " BusSendMessage : Out of memory \n" );
        return SBUSERROR_NOMEMORY;
        }
#endif
    }

int BusBroadcastMessage ( struct BusData *bd, int typeId,
                          char *message, int length )
/* send a message to all clients on the bus
   typeId is the id number of the data type of the message
   message is the data (generally a text string)
   length is the length of the data (in case it isn't null terminated) */
    {
    struct BusMessage bmsg;

    bmsg.toModule = BusBroadcast;

    bmsg.messageType = typeId;

    bmsg.messageLength = length;
    bmsg.message = message;

    return BusSendMessage ( bd, &bmsg );
    }

int BusSendByType ( struct BusData *bd, int typeId,
                    char *message, int length )
/* send a message to all client asking for messages of a given type
   (client that have done BusRegister(bd,typeId))
   typeId is the id number of the type
   message,length is the body and length of the body of the message */
    {
    struct BusMessage bmsg;

    bmsg.toModule = BusByType;

    bmsg.messageType = typeId;

    bmsg.messageLength = length;
    bmsg.message = message;

    return BusSendMessage ( bd, &bmsg );
    }

int BusBounceMessage ( struct BusData *bd, int typeId,
                       char *message, int length )
/* Send a message to yourself
   typeId is the type id number to associated with the message
   message,length is the body of the message, and length of the body */
    {
    struct BusMessage bmsg;

    bmsg.toModule = BusBounce;

    bmsg.messageType = typeId;

    bmsg.messageLength = length;
    bmsg.message = message;

    return BusSendMessage ( bd, &bmsg );
    }

int BusSendASCIIZ ( struct BusData *bd, int toModule, char *message )
/* send a null terminated string with type "ASCIIZ" to a client
   specified by that client's id number [toModule],
   toModule is the other client's id number
   message is the null terminated ASCII string to send */
    {
    struct BusMessage bmsg;

    bmsg.toModule = toModule;

    bmsg.messageType = BusFindTypeByName ( bd, "ASCIIZ" );

    bmsg.messageLength = strlen ( message ) + 1;
    bmsg.message = message;

    return BusSendMessage ( bd, &bmsg );
    }

int BusBroadcastASCIIZ ( struct BusData *bd, char *message )
/* Send a null terminated string to all clients attached to
   the bus
   message is the null terminated string to broadcast */
    {
    return BusSendASCIIZ ( bd, BusBroadcast, message );
    }



int BusSendDirect ( struct BusData *bd, int toModule, int typeId,
                    void ( *send_call ) ( int, char * ), char *data )
/* Request a point-to-point link to another client.
   if the other client is willing, the connection is
   formed, and the send_call function is called.
   otherwise an error is returned.

   toModule is the other client's id number
   typeId is the "type" of data to be transmitted of the
   link
   send_call is the function call made when the link
   has been establish, it is passed the socket descirptor
   (first int), and a generic data pointer. */
    {
    int portSock, s, portNumber;
    struct sockaddr_in sin;
    unsigned char busByte;
    char tmp_msg[512];
    char *res;
    int reqSeq;
    int err;

    portNumber = 0;
    portSock = busSocket_createAcceptorSocket ( &portNumber );

    if ( portSock < 0 )
        return SBUSERROR_GENERAL_FAILURE;

    sprintf ( tmp_msg,"%d %d", portNumber, typeId );
    reqSeq = BusSendBusByte ( bd, toModule,
                              BUSBYTE_REQ_DIRECT_CONNECT, strlen ( tmp_msg )+1, tmp_msg );

    if ( reqSeq == SBUSERROR_MASTER_ABSENT ) return reqSeq;
    debug3 ( DEBUG_CLIENT,"BusDirectConnReq : toModule = %d typeId = %d portNumber = %d\n",  toModule, typeId, portNumber );

    err = BusGetResponse ( bd, reqSeq, BUSBYTE_REPLY_DIRECT_CONNECT, &res );
    debug0 ( DEBUG_CLIENT,"BusDirectConnReq : Returning \n" );
    if ( err != SBUSERROR_NOT ) return err;

    busByte = res[0];
    if ( busByte == BUSBYTE_DIRECT_CONNECT_OK )
        {
        /* debug2(DEBUG_CLIENT, "Calling busSocket_acceptConnection on port %d sock %d \n", portNumber, portSock); */
        s = busSocket_acceptConnection ( portSock, &sin );
        close ( portSock );

        if ( s>=0 )
            {
            /* debug0(DEBUG_CLIENT,"BusSendDirect : Invoking calling function \n"); */
            send_call ( s,data );
            /* debug0(DEBUG_CLIENT,"BusSendDirect : Calling function returned \n"); */
            shutdown ( s,2 );
            close ( s );
            debug0 ( DEBUG_CLIENT,"BusSendDirect : Returning SUCCESS \n" );
            return SBUSERROR_NOT;
            }
        else
            {
            perror ( "Send Direct - accept failed" );
            debug0 ( DEBUG_CLIENT,"BusSendDirect : Returning FAILURE \n" );
            return SBUSERROR_ACCEPT_FAILED;
            }
        }
    else
        {
        if ( busByte == BUSBYTE_DIRECT_CONNECT_FAIL )
            {
            printf ( "BusSendDirect : Received BUSBYTE_DIRECT_CONNECT_FAIL \n" );
            }
        else
            {
            printf ( "BusSendDirect : busByte received is unknown \n" );
            }
        close ( portSock );
        debug0 ( DEBUG_CLIENT,"BusSendDirect : Returning FAILURE \n" );
        return SBUSERROR_MSG_NOT_UNDERSTOOD;
        }
    }




