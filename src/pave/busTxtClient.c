/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busTxtClient.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  - busTxtClient.c
 *
 *      Text client call back set-up routines and event loop routine.
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *
 * ********************************************************************
 * REVISION HISTORY - busTxtClient.c
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
 * Date: 1-April-94
 * Version: 0.5
 * Change Description: A timeout callback function can be added to
 *                     "select" in BusEventLoop. The callback can be
 *             added and removed through the functions
 *             "BusAddTimeoutCallback" and "BusRemoveTimeoutCallback".
 * Change author: R. Balay, NCSU, CSC
 *
 * Date: 7-Sept-95
 * Version: 0.6c
 * Change Description: Process messages in RecvdMessages queue in
 *    BusEventLoop and before processing any messages from the busMaster
 * Change author: R. Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
/* #include <sys/select.h> */
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

extern int errno;
extern int clientNotDone;
extern int masterAlive;

#include "busSocket.h"
#include "busClient.h"
#include "busError.h"
#include "busDebug.h"

struct BusInputCallback
    {
    struct BusInputCallback *next;    /* linked list...      */

    int fd;                          /* file descriptor     */

    BusInputCBfunc   read_callback;   /* ready for reading   */
    BusInputCBfunc  write_callback;   /* ready for writing   */
    BusInputCBfunc except_callback;   /* exception condition */
    };

int BusAddInputCallback ( struct BusData *bd,
                          int fd,
                          BusInputCBfunc   read_callback,
                          BusInputCBfunc  write_callback,
                          BusInputCBfunc except_callback )
    {
    struct BusInputCallback *newCallback;

    if ( bd == NULL )
        return SBUSERROR_BADPARAMETER;

    newCallback = ( struct BusInputCallback * )
                  malloc ( sizeof ( struct BusInputCallback ) );

    if ( newCallback != NULL )
        {
        newCallback->next = bd->InputCallbacks;
        bd->InputCallbacks = newCallback;

        newCallback->fd = fd;

        newCallback->  read_callback =   read_callback;
        newCallback-> write_callback =  write_callback;
        newCallback->except_callback = except_callback;

        return SBUSERROR_NOT;
        }
    else
        {
        perror ( "Could allocate storage for fd callback" );
        return SBUSERROR_NOMEMORY;
        }
    }

int BusRemoveInputCallback ( struct BusData *bd, int fd )
    {
    struct BusInputCallback *prev, *cur;

    if ( bd == NULL )
        return SBUSERROR_BADPARAMETER;

    prev=0;
    cur = bd->InputCallbacks;
    while ( cur && ( cur->fd != fd ) )
        {
        prev = cur;
        cur  = cur->next;
        }
    if ( cur )
        {
        if ( prev )
            prev->next=cur->next;
        else
            bd->InputCallbacks = bd->InputCallbacks->next;

        cur->next = 0;
        free ( cur );
        return SBUSERROR_NOT;
        }
    else
        return SBUSERROR_CBNOTFOUND;
    }

/* This is used to store the information about the function to
   call when a the select in BusEventLoop times out. */
struct BusTimeoutCallbackNode
    {

    void ( *callback ) ( struct BusData * );
    struct timeval timeout;
    };

/* This function adds a callback which will be called when the
   select in the BusEventLoop times out. The timeout interval for
   select is specified in the "timeout" variable thats passed to the
   function.  */
int BusAddTimeoutCallback ( struct BusData *bd, struct timeval *timeout,
                            void ( *callback ) ( struct BusData * ) )
    {
    if ( bd->TimeoutCallback != NULL )
        return SBUSERROR_GENERAL_FAILURE;

    bd->TimeoutCallback = ( struct BusTimeoutCallbackNode * ) malloc ( sizeof ( struct BusTimeoutCallbackNode ) );
    memcpy ( &bd->TimeoutCallback->timeout, timeout, sizeof ( struct timeval ) );
    bd->TimeoutCallback->callback = callback;
    return SBUSERROR_NOT;
    }

/* Removes the TimeoutCallback. The select in BusEventLoop then
   blocks on select. */
int BusRemoveTimeoutCallback ( struct BusData *bd )
    {
    if ( bd->TimeoutCallback == NULL )
        return SBUSERROR_GENERAL_FAILURE;

    free ( bd->TimeoutCallback );
    bd->TimeoutCallback = NULL;
    return SBUSERROR_NOT;
    }


void BusEventLoop ( struct BusData *bd )
    {
    fd_set rfds, wfds, efds;
    struct BusInputCallback *cur_callback;
    int err;
    int num_fds;
    struct timeval *timeout;

    while ( clientNotDone )
        {

        /* Process messages that have been received */
        BusProcessRecvdMessages ( bd );

        FD_ZERO ( &rfds );
        FD_ZERO ( &wfds );
        FD_ZERO ( &efds );

        /* Code added by Rajini for timeout to select */
        if ( bd->TimeoutCallback == NULL )
            {
            timeout = NULL;
            }
        else
            {
            timeout = &bd->TimeoutCallback->timeout;
            debug1 ( DEBUG_CLIENT, "BusEventLoop : setting timeout to %ld secs \n",
                     timeout->tv_sec );
            }
        num_fds = 0;
        cur_callback = bd->InputCallbacks;

        /* printf("FD_SETTING the fds \n");  */
        while ( cur_callback != NULL ) /* add all callback in the list */
            {
            if ( cur_callback->read_callback != NULL )
                {
                /* printf("fd_set'ting for read from #%d\n",cur_callback->fd); */
                FD_SET ( cur_callback->fd, &rfds );
                num_fds++;
                }

            if ( cur_callback->write_callback != NULL )
                {
                FD_SET ( cur_callback->fd, &wfds );
                num_fds++;
                }

            if ( cur_callback->except_callback != NULL )
                {
                FD_SET ( cur_callback->fd, &efds );
                num_fds++;
                }

            cur_callback = cur_callback->next;
            }

        if ( masterAlive )
            {
            /* printf("Bus Master is alive \n");   */
            FD_SET ( bd->fd,&rfds );
            /* printf("FDSet the bus fd : %d \n", bd->fd); */
            FD_SET ( bd->fd,&efds );
            num_fds++;
            }
        else
            {
            printf ( "Bus Master not alive \n" );
            }

        if ( num_fds > 0 )
            {
            err = select ( FD_SETSIZE,
                           ( SELECTFDSETPTR ) &rfds,
                           ( SELECTFDSETPTR ) &wfds,
                           ( SELECTFDSETPTR ) &efds,
                           timeout );

            if ( ( err <0 ) && ( errno == EINTR ) )
                {
                /* System Interrupt received - cleanup and return */
                clientNotDone = 0;
                continue;
                }
            if ( err<0 )
                /* bad error - select failed, and we don't know why... */
                {
                perror ( "Select failed" );
                clientNotDone = 0;
                /* will probably result in program termination... */
                continue;
                }
            if ( err == 0 )
                {
                /* Select timed out - Call the Timeout function */
                if ( bd->TimeoutCallback != NULL )
                    ( bd->TimeoutCallback->callback ) ( bd );
                }

            if ( FD_ISSET ( bd->fd, &efds ) )
                {
                /* something really bad! exit! */
                printf ( "Except on bus communication channel\n" );
                printf ( "Module going down\n" );
                BusClose ( bd );
                exit ( -1 );
                }

            if ( FD_ISSET (       bd->fd, &rfds ) )
                {
                /* printf("Input from bus on fd #%d \n", bd->fd);  */
                BusProcessRecvdMessages ( bd );
                err = BusDispatch ( bd );
                if ( err != SBUSERROR_NOT )
                    {
                    masterAlive = 0;
                    printf ( "CONNECTION TO BUS HAS BROKEN \n" );
                    if ( err == SBUSERROR_NODATA )
                        {
                        shutdown ( bd->fd, 2 );
                        close ( bd->fd );
                        }
                    }
                }

            /* don't bother just checking the first guy -
            make sure everybody gets a turn... */

            cur_callback = bd->InputCallbacks;
            while ( cur_callback )
                {
                if ( FD_ISSET ( cur_callback->fd,&rfds ) )
                    if ( cur_callback->read_callback != NULL )
                        {
                        /* printf("Input from descriptor %d\n",cur_callback->fd); */
                        debug0 ( DEBUG_CLIENT,"BusInputCallback : Invoking function \n" );
                        ( cur_callback->read_callback ) ( cur_callback->fd,bd );
                        debug0 ( DEBUG_CLIENT,"BusInputCallback : Returning \n" );
                        }
                if ( FD_ISSET ( cur_callback->fd,&wfds ) )
                    if ( cur_callback->write_callback!=NULL )
                        ( cur_callback->write_callback ) ( cur_callback->fd,bd );
                if ( FD_ISSET ( cur_callback->fd,&efds ) )
                    if ( cur_callback->except_callback!=NULL )
                        ( cur_callback->except_callback ) ( cur_callback->fd,bd );

                cur_callback = cur_callback->next;
                }
            }
        else
            {
            printf ( "No active sockets - Returning from BusEventLoop \n" );
            clientNotDone = 0;
            }
        }
    BusClose ( bd );
    }

