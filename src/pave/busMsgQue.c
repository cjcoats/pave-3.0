/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busMsgQue.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  - busMsgQue.c
 *
 *       Functions that handle bus message queues.
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *
 ********************************************************************
 * REVISION HISTORY - busMsgQue.c
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
 * Date: 17-Jan-95
 * Version: 0.3
 * Change Description: Added functions to Enque and Deque messages by
 *             Message & Option and Seq Number
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 27-Jan-95
 * Version: 0.3a
 * Change Description: Modified the function BusCopyBusMessage to check it
 *                     the messageLength > 0 before mallocing space and
 *             copying the message. Similarly, the Deque functions
 *             have been modified to check the messageLength before
 *             'free'ing the message.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "busMsgQue.h"
#include "busDebug.h"

void debugShowMessage ( struct BusMessage *bmsg )
    {
    printf ( "Seq: %d To: %d From: %d Option: %x Type: %d Len: %d",
             bmsg->serial, bmsg->toModule, bmsg->fromModule,
             bmsg->messageOption, bmsg->messageType, bmsg->messageLength );
    if ( bmsg->message != NULL )
        {
        if ( bmsg->messageType == 4096 )
            printf ( " Body: %s",bmsg->message );
        }
    printf ( "\n" );
    fflush ( stdout );
    }

void BusCopyBusMessage ( struct BusMessage *dst,
                         struct BusMessage *src )
    {
    dst->  toModule    = src->  toModule   ;
    dst->fromModule    = src->fromModule   ;
    dst->serial        = src->serial       ;
    dst->messageOption  = src->messageOption ;
    dst->messageType   = src->messageType  ;
    dst->messageLength = src->messageLength;

    /* Check if the length of message is 0 - otherwise allocate
       space and copy message - Modified by Rajini after version 6.3 */
    if ( src->messageLength > 0 )
        {
        dst->message = ( char * ) malloc ( dst->messageLength );

        strncpy ( dst->message, src->message, dst->messageLength );
        }
    }

struct BusMessage *
BusGetFirstMessage ( struct BusMessageQueue *last )
    {
    if ( last )
        {
        if ( last->next )
            {
            debug0 ( DEBUG_QUEUE, "Get First: " );
            fflush ( stdout );
            if ( DEBUG_QUEUE ) debugShowMessage ( & ( last->next->data ) );

            return & ( last->next->data );
            }
        else
            {
            debug0 ( DEBUG_QUEUE, "Get First: " );
            fflush ( stdout );
            if ( DEBUG_QUEUE ) debugShowMessage ( & ( last->data ) );

            return & ( last->data );
            }
        }
    else
        return NULL;
    }

int Bus_MessagesLeft ( struct BusMessageQueue *last )
    {
    return last!=NULL;
    }

struct BusMessageQueue *
Bus_DequeMessage ( struct BusMessageQueue *last )
    {
    if ( last )
        {
        if ( ( last->next != NULL ) && ( last->next != last ) )
            {
            struct BusMessageQueue *tmp;

            debug0 ( DEBUG_QUEUE, "Dequing message: " );
            fflush ( stdout );
            if ( DEBUG_QUEUE ) debugShowMessage ( & ( last->data ) );

            tmp = last->next;
            last->next = tmp->next;

            /* Check if the message length is > 0 and then free the message
               Modified by Rajini after version 6.3 */
            if ( tmp->data.messageLength > 0 )
                free ( tmp->data.message );
            free ( tmp );

            return last;
            }
        else
            {
            debug0 ( DEBUG_QUEUE, "Dequing only message: " );
            fflush ( stdout );
            if ( DEBUG_QUEUE ) debugShowMessage ( & ( last->data ) );

            /* Check if the message length is > 0 and then free the message
               Modified by Rajini after version 6.3 */
            if ( last->data.messageLength > 0 )
                free ( last->data.message );
            free ( last );

            return NULL;
            }
        }
    else
        return NULL;
    }

struct BusMessageQueue *
Bus_EnqueMessage ( struct BusMessageQueue *last,
                   struct BusMessage *bmsg )
    {
    struct BusMessageQueue *tmp;
    if ( last )
        {
        tmp = ( struct BusMessageQueue * )
              malloc ( sizeof ( struct BusMessageQueue ) );
        if ( tmp == NULL )
            return NULL;
        BusCopyBusMessage ( & ( tmp->data ), bmsg );
        tmp->next = last->next;
        last->next = tmp;
        }
    else
        {
        tmp = ( struct BusMessageQueue * )
              malloc ( sizeof ( struct BusMessageQueue ) );
        if ( tmp == NULL )
            return NULL;
        BusCopyBusMessage ( & ( tmp->data ), bmsg );
        tmp->next = tmp;
        }

    debug0 ( DEBUG_QUEUE, "Enqued Message : " );
    if ( DEBUG_QUEUE )
        debugShowMessage ( & ( tmp->data ) );

    return tmp;
    }

struct BusMessage *
BusGetMessageByModuleOption ( struct BusMessageQueue *last, int moduleId,
                              unsigned char option )
    {

    struct BusMessageQueue *tmpNode;
    int found = 0;

    debug2 ( DEBUG_QUEUE, "BusGetMessageByModuleOption : module = %d option = %x \n", moduleId, option );
    if ( last )
        {
        tmpNode = last;
        do
            {
            if ( DEBUG_QUEUE )
                debugShowMessage ( & ( tmpNode->data ) );
            if ( ( tmpNode->data.fromModule == moduleId ) &&
                    ( tmpNode->data.messageOption == option ) )
                {
                found = 1;
                }
            else
                {
                tmpNode = tmpNode->next;
                }
            }
        while ( ( tmpNode != last ) && ( !found ) );
        if ( found )
            {
            return ( & ( tmpNode->data ) );
            }
        else
            {
            return NULL;
            }
        }
    else
        {
        return NULL;
        }
    }

struct BusMessage *
BusGetMessageBySeq ( struct BusMessageQueue *last, int seq )
    {

    struct BusMessageQueue *tmpNode;
    int found = 0;

    debug1 ( DEBUG_QUEUE, "BusGetMessageBySeq : seq = %d \n", seq );
    if ( last )
        {
        tmpNode = last;
        do
            {
            if ( DEBUG_QUEUE )
                debugShowMessage ( & ( tmpNode->data ) );
            if ( tmpNode->data.serial == seq )
                {
                found = 1;
                debug0 ( DEBUG_QUEUE, "Found matching entry in queue \n" );
                }
            else
                {
                tmpNode = tmpNode->next;
                }
            }
        while ( ( tmpNode != last ) && ( !found ) );
        if ( found )
            {
            return ( & ( tmpNode->data ) );
            }
        else
            {
            return NULL;
            }
        }
    else
        {
        return NULL;
        }
    }

struct BusMessageQueue *
Bus_DequeMessageByModuleOption ( struct BusMessageQueue *last, int moduleId,
                                 unsigned char option )
    {
    struct BusMessageQueue *prevNode, *tmpNode;
    int found = 0;

    debug2 ( DEBUG_QUEUE, "Bus_DequeMessageByModuleOption : module = %d option = %x \n", moduleId, option );
    if ( last )
        {
        prevNode = tmpNode = last;
        do
            {
            if ( DEBUG_QUEUE )
                debugShowMessage ( & ( tmpNode->data ) );
            if ( ( tmpNode->data.fromModule == moduleId ) &&
                    ( tmpNode->data.messageOption == option ) )
                {
                found = 1;
                debug0 ( DEBUG_QUEUE, "Found corresponding node in Queue \n" );
                }
            else
                {
                prevNode = tmpNode;
                tmpNode = tmpNode->next;
                }
            }
        while ( ( tmpNode != last ) && ( !found ) );
        if ( found )
            {
            if ( prevNode->next == prevNode )
                {
                if ( tmpNode->data.messageLength > 0 )
                    free ( tmpNode->data.message );
                free ( tmpNode );
                return NULL;
                }
            else
                {
                if ( prevNode == tmpNode )
                    {
                    struct BusMessageQueue *lastNode;

                    lastNode = tmpNode->next;
                    while ( lastNode->next != tmpNode )
                        lastNode = lastNode->next;
                    lastNode->next = tmpNode->next;
                    return lastNode;
                    }
                else
                    {
                    prevNode->next = tmpNode->next;
                    if ( tmpNode->data.messageLength > 0 )
                        free ( tmpNode->data.message );
                    free ( tmpNode );
                    return last;
                    }
                }
            }
        else
            {
            return last;
            }
        }
    else
        {
        return NULL;
        }
    }

struct BusMessageQueue *
Bus_DequeMessageBySeq ( struct BusMessageQueue *last, int seq )
    {
    struct BusMessageQueue *prevNode, *tmpNode;
    int found = 0;

    debug1 ( DEBUG_QUEUE, "Bus_DequeMessageBySeq : seq = %d \n", seq );
    if ( last )
        {
        prevNode = tmpNode = last;
        do
            {
            if ( DEBUG_QUEUE )
                debugShowMessage ( & ( tmpNode->data ) );
            if ( tmpNode->data.serial == seq )
                {
                found = 1;
                debug0 ( DEBUG_QUEUE, "Found matching entry in queue \n" );
                }
            else
                {
                prevNode = tmpNode;
                tmpNode = tmpNode->next;
                }
            }
        while ( ( tmpNode != last ) && ( !found ) );
        if ( found )
            {
            if ( prevNode->next == prevNode )
                {
                if ( tmpNode->data.messageLength > 0 )
                    free ( tmpNode->data.message );
                free ( tmpNode );
                return NULL;
                }
            else
                {
                if ( prevNode == tmpNode )
                    {
                    struct BusMessageQueue *lastNode;

                    lastNode = tmpNode->next;
                    while ( lastNode->next != tmpNode )
                        lastNode = lastNode->next;
                    lastNode->next = tmpNode->next;
                    return lastNode;
                    }
                else
                    {
                    prevNode->next = tmpNode->next;
                    if ( tmpNode->data.messageLength > 0 )
                        free ( tmpNode->data.message );
                    free ( tmpNode );
                    return last;
                    }
                }
            }
        else
            {
            return last;
            }
        }
    else
        {
        return NULL;
        }
    }

struct BusMessageQueue *
Bus_DequeMessageByModuleSeq ( struct BusMessageQueue *last, int moduleId, int seq )
    {
    struct BusMessageQueue *prevNode, *tmpNode;
    int found = 0;

    debug2 ( DEBUG_QUEUE, "Bus_DequeMessageByModuleSeq : module = %d seq = %d \n", moduleId, seq );
    if ( last )
        {
        prevNode = tmpNode = last;
        do
            {
            if ( DEBUG_QUEUE )
                debugShowMessage ( & ( tmpNode->data ) );
            if ( ( tmpNode->data.serial == seq ) &&
                    ( tmpNode->data.fromModule == moduleId ) )
                {
                found = 1;
                debug0 ( DEBUG_QUEUE, "Found matching entry in queue \n" );
                }
            else
                {
                prevNode = tmpNode;
                tmpNode = tmpNode->next;
                }
            }
        while ( ( tmpNode != last ) && ( !found ) );
        if ( found )
            {
            if ( prevNode->next == prevNode )
                {
                if ( tmpNode->data.messageLength > 0 )
                    free ( tmpNode->data.message );
                free ( tmpNode );
                return NULL;
                }
            else
                {
                if ( prevNode == tmpNode )
                    {
                    struct BusMessageQueue *lastNode;

                    lastNode = tmpNode->next;
                    while ( lastNode->next != tmpNode )
                        lastNode = lastNode->next;
                    lastNode->next = tmpNode->next;
                    return lastNode;
                    }
                else
                    {
                    prevNode->next = tmpNode->next;
                    if ( tmpNode->data.messageLength > 0 )
                        free ( tmpNode->data.message );
                    free ( tmpNode );
                    return last;
                    }
                }
            }
        else
            {
            return last;
            }
        }
    else
        {
        return NULL;
        }
    }
