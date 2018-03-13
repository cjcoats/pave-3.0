/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busRWMessage.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  - busRWMessage.c
 *
 *       busRWMessage.c contains the functions for Reading and Writing messages
 *       to a module/busMaster. The function BusReadMessage reads the different
 *       fields of the message structure and BusWriteMessages writes them to
 *       the file descriptor. The functions BusSendBusByte sends a request
 *       to the Master - the message may have a NULL message body. BusReplyBusByte
 *       is a reply to the corresponding BusSendBusByte from the busMaster. The
 *       serial # of the reply message matches that of the request.
 *       BusGetResponse waits for a reply to a request sent to the busMaster.
 *       Messages received that do not match the request are queued and
 *       processed in BusDispatchLoop later.
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *
 ********************************************************************
 * REVISION HISTORY - busRWMessage.c
 *
 * Date: 7-Jan-95
 * Version: 0.2.1
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 17-Jan-95
 * Version: 0.3
 * Change Description: Modified BusSendBusByte for generating
 *             SeqNum from nextSeqNum field in BusData structure
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 27-Jan-95
 * Version: 0.3a
 * Change Description: Modified parameters to Bus_EnqueMessage from
 *                     'struct BusMessage' to 'struct BusMessage *'
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 18-March-95
 * Version: 0.4
 * Change Description: The BusSendBusByte and BusGetResponse functions
 *                     write/read from the bus only if the busMaster is
 *             alive.
 * Change author: Balay, R.
 *
 * Date: 12-Apr-95
 * Version: 0.5a
 * Change Description: BusWriteMessage returns an int instead of void and
 *                     BusSend/ReplyBusByte pack DUMMY_TYPE in the messageType
 *             field of the message.
 * Change author: Balay, R.
 *
 * Date: 19-Aug-95
 * Version: 0.6b
 * Change Description: Changed function definitions of BusSendBusByte etc to
 *                     use  char *
 * Change author: Balay, R.
 *
 * Date: 7-Sept-95
 * Version: 0.6c
 * Change Description: Fixed bug in queuing messages into RecvdMessages
 * Change author: Balay, R.
 *
 * Date: 15-Aug-96
 * Version: 0.7
 * Change Description: Modified BusWriteMessage to not to write to fd when an
 *                     write error is detected
 * Change author: Balay, R.
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <stdio.h>
#include <malloc.h>

#include "busMsgQue.h"
#include "busClient.h"
#include "busSocket.h"
#include "busRW.h"
#include "busRepReq.h"
#include "busError.h"
#include "busDebug.h"

#define   DUMMY_TYPE  100

int masterAlive = 0; /* Variable to keep track of status of busMaster */


int getSeqNum ( struct BusData *bd )
    {
    if ( ++bd->nextSeqNum > 2048 )
        bd->nextSeqNum = 0;
    return bd->nextSeqNum;
    }

int BusReadMessage ( int fd, struct BusMessage *bmsg )
    {
    int err;

    /* printf("BusReadMessage : \n"); */

    if ( ( err=BusReadInteger ( fd, & ( bmsg->toModule ) ) ) !=SBUSERROR_NOT )
        return err;
    if ( ( err=BusReadInteger ( fd, & ( bmsg->fromModule ) ) ) !=SBUSERROR_NOT )
        return err;
    if ( ( err=BusReadInteger ( fd, & ( bmsg->serial ) ) ) !=SBUSERROR_NOT )
        return err;
    if ( ( err=BusReadCharacter ( fd, & ( bmsg->messageOption ) ) ) !=SBUSERROR_NOT )
        return err;
    if ( ( err=BusReadInteger ( fd, & ( bmsg->messageType ) ) ) !=SBUSERROR_NOT )
        return err;
    err=BusReadnString ( fd, & ( bmsg->message ), & ( bmsg->messageLength ) );
    /* printf("BusReadMessage :  done\n"); */
    return err;

    }

int BusWriteMessage ( int fd, struct BusMessage *bmsg )
    {
    int err;

    /* printf("BusWriteMessage : \n");  */

    err = BusWriteInteger ( fd, bmsg->toModule );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing toModule \n" );
        return err;
        }
    err = BusWriteInteger ( fd, bmsg->fromModule );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing fromModule \n" );
        return err;
        }
    err = BusWriteInteger ( fd, bmsg->serial );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing serial \n" );
        return err;
        }
    err = BusWriteCharacter ( fd, bmsg->messageOption );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing messageOption \n" );
        return err;
        }
    err = BusWriteInteger ( fd, bmsg->messageType  );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing messageType \n" );
        return err;
        }
    err = BusWritenString ( fd, bmsg->message, bmsg->messageLength );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing message : err = %d \n",err );
        return err;
        }
    return SBUSERROR_NOT;
    /* printf("BusWriteMessage : done \n"); */
    }

#ifdef WRITE_MESSAGE
int BusWriteMessage ( int fd, struct BusMessage *bmsg )
    {
    int err;

    /* printf("BusWriteMessage : \n");  */

    err = BusWriteInteger ( fd, bmsg->toModule );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing toModule \n" );
        BusWriteInteger ( fd,-1 );
        return err;
        }
    err = BusWriteInteger ( fd, bmsg->fromModule );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing fromModule \n" );
        BusWriteInteger ( fd,-1 );
        return err;
        }
    err = BusWriteInteger ( fd, bmsg->serial );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing serial \n" );
        BusWriteInteger ( fd,-1 );
        return err;
        }
    err = BusWriteCharacter ( fd, bmsg->messageOption );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing messageOption \n" );
        BusWriteInteger ( fd,-1 );
        return err;
        }
    err = BusWriteInteger ( fd, bmsg->messageType  );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing messageType \n" );
        BusWriteInteger ( fd,-1 );
        return err;
        }
    err = BusWritenString ( fd, bmsg->message, bmsg->messageLength );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( "  Error Writing message : err = %d \n",err );
        BusWritenString ( fd,"",0 );
        return err;
        }
    return SBUSERROR_NOT;
    /* printf("BusWriteMessage : done \n"); */
    }
#endif

int BusSendBusByte ( struct BusData *bd, int toId, unsigned char option,
                     int len,  char *buf )
    {

    struct BusMessage bmsg;

    debug1 ( DEBUG_RWMSG, "BusSendBusByte:  Sending busbyte %x \n",option );

    if ( !masterAlive ) return SBUSERROR_MASTER_ABSENT;
    bmsg.toModule = toId;
    bmsg.fromModule = bd->moduleId;
    bmsg.serial = getSeqNum ( bd );
    bmsg.messageOption = option;
    bmsg.messageType = DUMMY_TYPE;
    bmsg.messageLength = len;
    bmsg.message = buf;

    BusWriteMessage ( bd->fd, &bmsg );
    return bmsg.serial;
    }

int BusReplyBusByte ( int fd, int seq, int toId, int myId, unsigned char option, int len, char *buf )
    {

    struct BusMessage bmsg;

    debug1 ( DEBUG_RWMSG, "BusReplyBusByte:  Replying busbyte %x \n",option );
    bmsg.toModule = toId;
    bmsg.fromModule = myId;
    bmsg.serial = seq;
    bmsg.messageOption = option;
    bmsg.messageType = DUMMY_TYPE;
    bmsg.messageLength = len;
    bmsg.message = buf;

    return BusWriteMessage ( fd, &bmsg );
    }

int BusForwdBusByte ( int fd, int seq, int toId, int myId, unsigned char option, int len,  char *buf )
    {
    return BusReplyBusByte ( fd, seq, toId, myId, option, len, buf );
    }

int  BusGetResponse ( struct BusData *bd, int seq, unsigned char busByte, char **result )
    {
    struct BusMessage bmsg;
    struct BusMessageQueue *bmq;
    int err;

    err = BusReadMessage ( bd->fd, &bmsg );
    debug4 ( DEBUG_RWMSG, "BusGetResponse : Waiting for seq = %d busByte = %x \n \t\t Got seq = %d busByte = %x \n",
             seq, busByte, bmsg.serial, bmsg.messageOption );
    while ( !err && ( ( bmsg.serial != seq ) || ( bmsg.messageOption != busByte ) ) )
        {
        bmq = Bus_EnqueMessage ( bd->RecvdMessages, &bmsg );

        if ( bmq != NULL )
            {
            bd->RecvdMessages = bmq;
            }

        if ( bmsg.messageLength > 0 ) free ( bmsg.message );
        err = BusReadMessage ( bd->fd, &bmsg );
        debug4 ( DEBUG_RWMSG, "BusGetResponse : Waiting for seq = %d busByte = %x \n \t\t Got seq = %d busByte = %x \n",
                 seq, busByte, bmsg.serial, bmsg.messageOption );
        }
    if ( ! err )
        *result = bmsg.message;
    return err;
    }

