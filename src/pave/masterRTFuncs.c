
/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: masterRTFuncs.c 83 2018-03-12 19:24:33Z coats $
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
 ***************************************************************************
 *  ABOUT:  masterRTFuncs.c
 *
 *  Library of bus-master run-time functions to handle (Module and Client are
 *  used interchangeably):
 *
 *    Reset Connection  (when a socket channel is deemed in error)
 *    Module Leaving the bus
 *    Reading of Messages From a Module
 *    Enqueing of a Message to and at clients
 *    Broadcasting (general, and by Type)
 *    Getting of Messages from clients
 *    Passing Messages to clients on their request
 *    Finding a Module By Name
 *    Finding a Module By Id  or its handle
 *    Finding a Type handle By Name
 *    Finding a Type By Id handle
 *    Telling Who is Connected
 *    Handshaking during a Direct connection
 *
 * KNOWN BUGS:  :-(
 *
 * OTHER NOTES: :-)
 *            Direct Connection setup and WhosConnected
 *            functions should be modified to use BusSendBusByte.
 *
 ****************************************************************************
 *  REVISION HISTORY - masterRTFuncs.c
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
 * Date: 7-Jan-95
 * Version: 0.2.1
 * Change Description: Changed calls to BusWriteCharacter to BusReplyBusByte/
 *                     BusSendBusByte/BusWriteMessage
 * Change author: R. Balay, NCSU, CSC
 *
 * Date: 17-Jan-95
 * Version: 0.3
 * Change Description: Direct Connection establishment and FindWhosConnected
 *             work with the new protocol.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 12-Jan-95
 * Version: 0.3a
 * Change Description: Modified parameters of functions Bus_EnqueMessage,
 *                     BusMasterRTPutMessageInQue_s etc to take
 *             'struct BusMessage *' instead of 'struct BusMessage'.
 *             Also fixed the bug in BusMasterRTTellWhosConnected
 *             that was due to 'strcat'.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 18-March-95
 * Version: 0.4
 * Change Description: Added functions for GetModuleInfoByName and
 *                     GetModuleInfoById
 * Change author: Balay, R.
 *
 * Date: 12-Apr-95
 * Version: 0.5a
 * Change Description: Added function BusMasterRTKillClient that sends a
 *                     DIE message to the Client module
 * Change author: Balay, R.
 *
 * Date: 7-Sept-95
 * Version: 0.6c
 * Change Description: Corrected enqueing to the Waiting queue.
 * Change author: Balay, R.
 *
 * Date: 15-Aug-96
 * Version: 0.7
 * Change Description: Acknowledgment is not sent for every message  Sent only
 *                    if compiled with BUS_WITH_ACK option
 *            KILL_CLIENT removes the client entry from busMaster DB after
 *            sending signal to client - a hack for getting around client
 *            that dont die with the KILL_CLIENT option because they are
 *            waiting for the child processes created by them through
 *            'rsh' to exit.
 * Change author: Balay, R.
 * 
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

char *vcmasterRTFuncs= "$Id: masterRTFuncs.c 83 2018-03-12 19:24:33Z coats $" ;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "busSocket.h" /* for socket prototypes (shutdown()) */
#include "busRW.h"
#include "busRWMessage.h"
#include "busRepReq.h"
#include "masterRTFuncs.h"
#include "busMsgQue.h"
#include "masterDB.h"
#include "busError.h"
#include "busDebug.h"

int BusMasterRTResetConnection  ( struct BusMasterData *bmd,
                                  struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {
    int err;
    struct BusMessageQueue *bmq;

    sleep ( 1 );
    err = BusForwdBusByte ( bmn->fd, bmsg->serial, MASTERID, bmn->moduleId,
                            BUSBYTE_PLEASE_ACKNOWLEDGE, 0, NULL );
    bmq = Bus_EnqueMessage ( bmd->WaitingReq, bmsg );
    if ( bmq != NULL )
        {
        bmd->WaitingReq = bmq;
        }
    return err;
    }

int BusMasterRTModuleLeaving ( struct BusMasterData *bmd,
                               struct BusModuleNode *bmn )
    {
    struct BusModuleNode *prev, *cur;

    if ( bmd == NULL )                /* bus master database is dead !!!    */
        return SBUSERROR_BADPARAMETER;
    else if ( bmd->Modules == NULL )  /* no modules connection !!!          */
        return SBUSERROR_NODATA;
    else if ( bmd->Modules == bmn )   /* module leaving was last to connect */
        {
        prev = cur = bmn;

        while ( ( prev != NULL ) && ( prev->next != bmn ) )
            prev = prev->next;
        }
    else
        {
        prev = NULL;
        cur  = bmd->Modules;
        }

    while ( ( cur!=NULL ) && ( cur->next != bmd->Modules ) && ( cur != bmn ) )
        {
        prev = cur;
        cur  = cur->next;
        }

    if ( cur != bmn )
        {
        printf ( "Did not find module in list (for deletion)\n" );
        return SBUSERROR_GENERAL_FAILURE;
        }
    else
        {
        if ( prev != NULL ) /* can only be null if module list is not circular */
            prev -> next = bmn -> next;

        if ( bmn == bmd->Modules )
            {
            printf ( "last module to connect leaving\n" );

            if ( bmd->Modules == bmd->Modules->next )
                bmd->Modules = NULL;     /* only entry in circular list */
            else
                bmd->Modules = bmd->Modules->next;
            }

        shutdown ( bmn->fd,2 );
        close ( bmn->fd );

        printf ( "Connection #%d to client %s shutdown and closed.\n",
                 bmn->fd, bmn->moduleName );

        free ( bmn->moduleName );
        free ( bmn );

        return SBUSERROR_NOT;
        }
    }

void BusMasterEnqueMessage ( struct BusModuleNode *bmn,
                             struct BusMessage    *bmsg )
    {
    struct BusMessageQueue *bmq;

    bmsg->messageOption = BUSBYTE_MESSAGE_HERE;
    if ( ( bmn->Messages == NULL ) && ( bmn->fd >= 0 ) )
        BusWriteMessage ( bmn->fd, bmsg );

#ifdef BUS_WITH_ACK
    bmq = Bus_EnqueMessage ( bmn->Messages, bmsg );
    if ( bmq != NULL )
        bmn->Messages = bmq;
    else
        printf ( "Error enqueing a message.\n" );
#endif
    }

void BusMasterBroadcast ( struct BusMasterData *bmd,
                          struct BusMessage    *bmsg )
    {
    struct BusModuleNode *bmn;

    if ( bmd->Modules != NULL )
        {
        bmn = bmd->Modules;

        do
            {
            if ( bmsg->fromModule != bmn->moduleId )
                BusMasterEnqueMessage ( bmn, bmsg );
            bmn = bmn->next;
            }
        while ( bmn && bmn != bmd->Modules );
        }
    }

void BusMasterBroadcastByType ( struct BusMasterData *bmd,
                                struct BusMessage    *bmsg )
    {
    struct BusModuleNode *bmn;

    bmn = bmd->Modules;

    printf ( "Distributing by type: " );
    fflush ( stdout );

    do
        {
        if ( BusMasterCheckRegisteredTypes ( bmn, bmsg->messageType ) )
            {
            printf ( "sending to %s ",bmn->moduleName );
            fflush ( stdout );
            BusMasterEnqueMessage ( bmn, bmsg );
            }
        else
            {
            printf ( "skipping %s ",bmn->moduleName );
            fflush ( stdout );
            }
        bmn = bmn->next;
        }
    while ( bmn && bmn != bmd->Modules );
    printf ( "\n" );
    fflush ( stdout );
    }

void BusMasterRTPutMessageInQue_s ( struct BusMasterData *bmd,
                                    struct BusMessage    *bmsg )
    {
    if ( bmsg->toModule==BusBroadcast ) /* send to everyone */
        BusMasterBroadcast ( bmd, bmsg );
    else if ( bmsg->toModule==BusBounce ) /* return to sender */
        {
        struct BusModuleNode *bmn;

        bmn = BusMasterFindModuleById ( bmd->Modules, bmsg->fromModule );
        if ( bmn != NULL )
            BusMasterEnqueMessage ( bmn, bmsg );
        else
            printf ( "Message bounce from unidentified module %d.\n",
                     bmsg->fromModule ); /* otherwise error ???? */
        }
    else if ( bmsg->toModule==BusByType ) /* send to those who understand */
        BusMasterBroadcastByType ( bmd, bmsg );
    else                               /* send to specific client */
        {
        struct BusModuleNode *bmn;
        bmn = BusMasterFindModuleById ( bmd->Modules, bmsg->toModule );
        if ( bmn != NULL )
            BusMasterEnqueMessage ( bmn, bmsg );
        else
            printf ( "Message sent to unidentified module %d.\n",
                     bmsg->toModule );
        }
    }

int BusMasterRTProcessMessage       ( struct BusMasterData *bmd,
                                      struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {
    int err;

    /* store in busMessage, and put message in destination que */
    /* bmsg->serial = bmd->nextMessageSerialNumber; */
    bmd->nextMessageSerialNumber++;
    bmsg->fromModule = bmn->moduleId;

#ifdef BUS_WITH_ACK
    /* tell the client to deque the message */
    err = BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                            BUSBYTE_DEQUE_MESSAGE, 0, NULL );
#else
    err = SBUSERROR_NOT;
#endif

    BusMasterRTPutMessageInQue_s ( bmd, bmsg );

    return err;
    }

int BusMasterRTFindModuleByName ( struct BusMasterData *bmd,
                                  struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {
    struct BusModuleNode *bmn2;
    int err;

    bmn2 = BusMasterFindModuleByName ( bmd->Modules, bmsg->message );
    if ( bmn2 != NULL )
        {
        char tmp_buf[32];

        sprintf ( tmp_buf, "%d", bmn2->moduleId );
        debug2 ( DEBUG_MASTER, "Module %s has id #%s\n",bmsg->message, tmp_buf );
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                                BUSBYTE_FIND_MODULE_BY_NAME,
                                strlen ( tmp_buf )+1, tmp_buf );
        }
    else
        {
        int findErr;
        char tmp_buf[32];

        printf ( "No module \"%s\" found.\n",bmsg->message );
        findErr = -1;
        sprintf ( tmp_buf, "-1" );
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                                BUSBYTE_FIND_MODULE_BY_NAME,
                                strlen ( tmp_buf )+1, tmp_buf );
        }
    return err;
    }

int BusMasterRTFindModuleById   ( struct BusMasterData *bmd,
                                  struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {
    struct BusModuleNode *bmn2;
    int id, err;

    sscanf ( bmsg->message, "%d", &id );
    bmn2 = BusMasterFindModuleById ( bmd->Modules, id );
    if ( bmn2 != NULL )
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                                BUSBYTE_FIND_MODULE_BY_ID, strlen ( bmn2->moduleName )+1,
                                bmn2->moduleName );
    else
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                                BUSBYTE_FIND_MODULE_BY_ID, strlen ( FIND_NAME_ERR )+1,
                                FIND_NAME_ERR );
    return err;
    }

int BusMasterRTFindTypeByName   ( struct BusMasterData *bmd,
                                  struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {
    struct BusTypeNode *btn;
    int err;
    char *name;

    name = bmsg->message;
    /* printf("type name = %s \n", bmd->Types->next->typeName ); */
    btn = BusMasterFindTypeByName ( bmd->Types, bmsg->message );
    if ( btn != NULL )
        {
        char tmp_buf[32];

        sprintf ( tmp_buf, "%d", btn->typeId );
        debug2 ( DEBUG_MASTER, "Type %s has id #%d.\n",name,btn->typeId );
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                                BUSBYTE_FIND_TYPE_BY_NAME,
                                strlen ( tmp_buf )+1, tmp_buf );
        }
    else
        {
        char tmp_buf[32];

        btn = BusMasterAddType ( bmd->Types,
                                 name,
                                 bmd->nextTypeId++ );
        if ( btn != NULL )
            bmd->Types = btn;
        else
            perror ( "Warning - Type addition failed" );

        debug2 ( DEBUG_MASTER, "Added type %s - id #%d.\n",name, bmd->nextTypeId-1 );
        /* type not found, add it and return the newly allocated typeid */
        sprintf ( tmp_buf, "%d", bmd->nextTypeId-1 );
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                                BUSBYTE_FIND_TYPE_BY_NAME, strlen ( tmp_buf )+1, tmp_buf );
        }

    return err;
    }

int BusMasterRTFindTypeById     ( struct BusMasterData *bmd,
                                  struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {
    struct BusTypeNode *btn;
    int typeId;
    int err;

    sscanf ( bmsg->message, "%d", &typeId );
    btn = BusMasterFindTypeById ( bmd->Types, typeId );
    if ( btn != NULL )
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                                BUSBYTE_FIND_TYPE_BY_ID, strlen ( btn->typeName )+1,
                                btn->typeName );
    else
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                                BUSBYTE_FIND_TYPE_BY_ID, strlen ( FIND_NAME_ERR )+1,
                                FIND_NAME_ERR );
    return err;
    }

int BusMasterRTGetModuleInfoById   ( struct BusMasterData *bmd,
                                     struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {
    struct BusModuleNode *bmn2;
    int id, err;

    sscanf ( bmsg->message, "%d", &id );
    bmn2 = BusMasterFindModuleById ( bmd->Modules, id );
    if ( bmn2 != NULL )
        {
        char *tmpList;

        tmpList = malloc ( MAX_INTSTR_SIZE+MAX_MODULENAME+32 );
        sprintf ( tmpList, " %d %s %s", bmn2->moduleId,
                  bmn2->moduleName, inet_ntoa ( bmn2->sin.sin_addr ) );
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, MASTERID, bmn->moduleId,
                                BUSBYTE_MODULE_INFO_BY_ID, strlen ( tmpList )+1, tmpList );
        free ( tmpList );
        }
    else
        {
        printf ( "Error obtaining information for moduleId %d \n", id );
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                                BUSBYTE_MODULE_INFO_BY_ID, strlen ( FIND_NAME_ERR )+1,
                                FIND_NAME_ERR );
        }
    return err;
    }

int BusMasterRTGetModuleInfoByName ( struct BusMasterData *bmd,
                                     struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {
    struct BusModuleNode *bmn2;
    int err;

    bmn2 = BusMasterFindModuleByName ( bmd->Modules, bmsg->message );
    if ( bmn2 != NULL )
        {
        char *tmpList;

        tmpList = malloc ( MAX_INTSTR_SIZE+MAX_MODULENAME+32 );
        sprintf ( tmpList, " %d %s %s", bmn2->moduleId,
                  bmn2->moduleName, inet_ntoa ( bmn2->sin.sin_addr ) );
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, MASTERID, bmn->moduleId,
                                BUSBYTE_MODULE_INFO_BY_NAME, strlen ( tmpList )+1, tmpList );
        free ( tmpList );
        }
    else
        {
        printf ( "Error obtaining information about module %s \n", bmsg->message );
        err = BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                                BUSBYTE_MODULE_INFO_BY_NAME, strlen ( FIND_NAME_ERR )+1,
                                FIND_NAME_ERR );
        }
    return err;
    }


int BusMasterRTTellWhosConnected ( struct BusMasterData *bmd,
                                   struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {
    struct BusModuleNode *bmn2;
    int err, bufferSize;
    int done = 0;
    int numModules = 0;
    char *moduleList, *tmpList, *mList;

    bmn2 = bmd->Modules;
    tmpList = NULL;

    /* Allocate enough space for bmd->nextModuleId-FD_SETSIZE entries and the
       number of modules (an integer) */
    bufferSize = ( bmd->nextModuleId-FD_SETSIZE ) * ( MAX_INTSTR_SIZE+MAX_MODULENAME+32 )+MAX_INTSTR_SIZE;
    moduleList = malloc ( bufferSize );
    mList = malloc ( bufferSize );
    memset ( moduleList, 0, bufferSize );
    memset ( mList, 0, bufferSize );

    debug0 ( DEBUG_MASTER, "BusFindWhosConnected : \n" );

    while ( ( bmn2 != NULL ) && !done )
        {
        if ( bmn2->fd != -1 )
            {
            numModules++;
            tmpList = malloc ( MAX_INTSTR_SIZE+MAX_MODULENAME+32 );
            sprintf ( tmpList, " %d %s %s", bmn2->moduleId,
                      bmn2->moduleName, inet_ntoa ( bmn2->sin.sin_addr ) );
            debug1 ( DEBUG_MASTER,"	%s \n", tmpList );
            moduleList = strcat ( moduleList, tmpList );
            free ( tmpList );
            }
        bmn2 = bmn2->next;
        if ( bmn2 == bmd->Modules )
            done = 1;
        }

    /* First entry of the string is the number of modules connected */
    sprintf ( mList, "%d", numModules );
    mList = strcat ( mList, moduleList );
    err = BusReplyBusByte ( bmn->fd, bmsg->serial, MASTERID, bmn->moduleId,
                            BUSBYTE_WHOS_CONNECTED, strlen ( mList )+1, mList );
    if ( moduleList != NULL )
        {
        free ( moduleList );
        }
    if ( mList != NULL )
        {
        free ( mList );
        }
    return err;
    }

int BusMasterRTReqDirect ( struct BusMasterData *bmd,
                           struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {
    struct BusModuleNode *otherbmn;
    int otherClient, messageType, portNumber;

    debug0 ( DEBUG_MASTER, "BusMasterRTReqDirect : " );
    otherClient = bmsg->toModule;
    sscanf ( bmsg->message,"%d %d", &portNumber, &messageType );
    debug3 ( DEBUG_MASTER, "Port = %d OtherClient = %d messageType = %d \n",
             portNumber, otherClient, messageType );

    otherbmn = BusMasterFindModuleById ( bmd->Modules, otherClient );

    if ( otherbmn != NULL )
        {
        if ( otherbmn -> Messages == NULL )
            {
            char tmp_buf[512];
            struct BusMessageQueue *bmq;

            sprintf ( tmp_buf, "%d %d %s", messageType, portNumber,
                      inet_ntoa ( ( bmn->sin ).sin_addr ) );
            debug1 ( DEBUG_MASTER, "Sending REQ to otherclient %d \n",
                     otherbmn->moduleId );
            BusForwdBusByte ( otherbmn->fd, bmsg->serial, otherbmn->moduleId,
                              bmn->moduleId, BUSBYTE_REQ_DIRECT_CONNECT, strlen ( tmp_buf )+1, tmp_buf );

            bmq = Bus_EnqueMessage ( bmd->WaitingReq, bmsg );
            if ( bmq != NULL )
                {
                bmd->WaitingReq = bmq;
                }
            }
        return SBUSERROR_NOT;
        }
    else
        {
        unsigned char busByte;

        debug0 ( DEBUG_MASTER, "Client requested == NULL sending a CONNECT FAIL \n" );
        busByte = BUSBYTE_DIRECT_CONNECT_FAIL;
        BusReplyBusByte ( bmn->fd, bmsg->serial, bmn->moduleId, MASTERID,
                          BUSBYTE_REPLY_DIRECT_CONNECT, 2, ( char * ) &busByte );
        return SBUSERROR_NOT;
        }
    }

int BusMasterRTReplyDirect ( struct BusMasterData *bmd,
                             struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {

    struct BusMessage *tmp_bmsg;
    struct BusModuleNode *other_bmn;

    if ( bmd->WaitingReq != NULL )
        {
        tmp_bmsg = BusGetMessageByModuleOption ( bmd->WaitingReq, bmsg->toModule,
                   BUSBYTE_REQ_DIRECT_CONNECT );

        if ( tmp_bmsg == NULL )
            {
            printf ( "Message does not exist in WaitingReq Queue \n" );
            return SBUSERROR_GENERAL_FAILURE;
            }
        else
            {
            bmsg->serial = tmp_bmsg->serial;

            other_bmn = BusMasterFindModuleById ( bmd->Modules, tmp_bmsg->fromModule );
            bmd->WaitingReq =
                Bus_DequeMessageByModuleOption ( bmd->WaitingReq, bmsg->toModule,
                                                 BUSBYTE_REQ_DIRECT_CONNECT );
            if ( other_bmn == NULL )
                {
                printf ( "Client %d requesting Direct connection does'nt exist!! \n",
                         bmsg->toModule );
                return SBUSERROR_GENERAL_FAILURE;
                }
            BusWriteMessage ( other_bmn->fd, bmsg );
            return SBUSERROR_NOT;
            }
        }
    return SBUSERROR_GENERAL_FAILURE;
    }

int BusMasterRTKillClient ( struct BusMasterData *bmd,
                            struct BusModuleNode *bmn, struct BusMessage *bmsg )
    {
    struct BusModuleNode *bmn1, *bmn2;

    bmn1 = BusMasterFindModuleByName ( bmd->Modules, "Console" );
    if ( bmn1 != NULL )
        {
        if ( bmn1->moduleId != bmsg->fromModule )
            return SBUSERROR_GENERAL_FAILURE;
        bmn2 = BusMasterFindModuleByName ( bmd->Modules, bmsg->message );
        if ( bmn2 != NULL )
            {
            BusForwdBusByte ( bmn2->fd, bmsg->serial, bmn2->moduleId,
                              bmn->moduleId, BUSBYTE_KILL_CLIENT, 0, NULL );
            BusMasterRTModuleLeaving ( bmd, bmn2 );
            return SBUSERROR_NOT;
            }
        }
    return SBUSERROR_GENERAL_FAILURE;
    }
