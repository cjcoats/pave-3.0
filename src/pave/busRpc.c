/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busRpc.c 83 2018-03-12 19:24:33Z coats $
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
 * * ABOUT:  - busRpc.c
 *
 *        Client interface function to call a procedure on a remote
 *        machine.
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *
 ********************************************************************
 * REVISION HISTORY - busRpc.c
 *
 * Date: 25-Feb-95
 * Version: 0.3b
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 15-Aug-96
 * Version: 0.7
 * Change Description: Modified some debug statements
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>

#include "bus.h"
#include "busRpc.h"
#include "busSocket.h"
#include "busRW.h"
#include "busRWMessage.h"
#include "busRepReq.h"


int BusCallRemote ( struct BusData *bd, int toModule, int typeId,
                    void ( *stub ) ( int, char *, char * ), char *params, char *results )
/* Request a remote procedure call via a point-to-point link to another client.
   if the other client is willing, the connection is
   formed, and the stub function is called which packs the parameters
   for the remote function.
   otherwise an error is returned.

   toModule is the other client's id number
   typeId is the "type" of data to be transmitted of the link
   stub is the function call made when the link
   has been establish, it is passed the socket descirptor
   (first int), and a generic data pointer. */
    {
    int portSock, s, portNumber;
    struct sockaddr_in sin;
    unsigned char busByte;
    char tmp_msg[512];
    char *res;
    int reqSeq;

    portNumber = 0;
    portSock = busSocket_createAcceptorSocket ( &portNumber );

    if ( portSock < 0 )
        return SBUSERROR_GENERAL_FAILURE;

    debug3 ( DEBUG_CLIENT,"BusDirectConnReq : toModule = %d typeId = %d portNumber = %d\n",  toModule, typeId, portNumber );
    sprintf ( tmp_msg,"%d %d", portNumber, typeId );
    reqSeq = BusSendBusByte ( bd, toModule,
                              BUSBYTE_REQ_DIRECT_CONNECT, strlen ( tmp_msg )+1, tmp_msg );
    BusGetResponse ( bd, reqSeq, BUSBYTE_REPLY_DIRECT_CONNECT, &res );
    debug0 ( DEBUG_CLIENT,"BusDirectConnReq : Returning \n" );

    busByte = res[0];
    if ( busByte == BUSBYTE_DIRECT_CONNECT_OK )
        {
        /* debug2(DEBUG_CLIENT, "Calling busSocket_acceptConnection on port %d sock %d \n", portNumber, portSock); */
        s = busSocket_acceptConnection ( portSock, &sin );
        close ( portSock );

        if ( s>=0 )
            {
            stub ( s, params, results );
            shutdown ( s,2 );
            close ( s );
            debug0 ( DEBUG_CLIENT,"BusSendDirect : Returning SUCCESS \n" );
            return SBUSERROR_NOT;
            }
        else
            {
            perror ( "Call Remote - accept failed" );
            debug0 ( DEBUG_CLIENT,"BusSendDirect : Returning FAILURE \n" );
            return SBUSERROR_ACCEPT_FAILED;
            }
        }
    else
        {
        if ( busByte == BUSBYTE_DIRECT_CONNECT_FAIL )
            {
            printf ( "BusCallRemote : Received BUSBYTE_DIRECT_CONNECT_FAIL \n" );
            }
        else
            {
            printf ( "BusCallRemote : busByte received is unknown \n" );
            }
        close ( portSock );
        debug0 ( DEBUG_CLIENT,"BusSendDirect : Returning FAILURE \n" );
        return SBUSERROR_MSG_NOT_UNDERSTOOD;
        }
    }

