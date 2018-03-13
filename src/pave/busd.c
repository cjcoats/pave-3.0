/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busd.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  busd.c
 *
 *     busd.c is a Client program that allows transfer of files between
 *     machines through Direct Communication.
 *
 *     INPUT FILES:       stdin stream
 *     OUTPUT FILES:      stdout stream
 *     ERROR HANDLING:    output to stderr (screen)
 *     INPUT PARAMETERS:  client name (ascii string), optional
 *     INPUT OPTIONS:     if client name is not input, default is
 *
 *     NETWORKING:        remote/network connectivity is via bus library (sockets)
 *
 * KNOWN BUGS:  :-(
 *
 * OTHER NOTES: :-)
 *
 ********************************************************************
 *
 * HISTORY:  busd.c
 *
 * Date: 1-Feb-95
 * Version: 0.3a
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 24-July-95
 * Version: 0.3b
 * Change Description: Changed name of module to "busd_1243.." and
 *    added exit timeout and callback
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/types.h>

#include <sys/socket.h>  /* socket structures */
#include <netinet/in.h>  /* internet domain sockets */
#include <sys/un.h>      /* defines sockaddr_un (Unix domain sockets) */

#include <malloc.h>      /* prototypes for malloc/free */
#include <time.h>        /* struct timeval (in select) */
#include <unistd.h>

#include <errno.h>

/*----------------------------------------------------------------------*/

#include "busFtp.h"

static const char SVN_ID[] = "$Id: busd.c 83 2018-03-12 19:24:33Z coats $";

void getBINARY_callback ( int fd, char *data );
void putBINARY_callback ( int fd, char *data );
void dir_callback ( struct BusData *bd, struct BusMessage *bmsg );

void stdin_callback ( int fd, struct BusData *bd )
/* Function called when input comes from the keyboard */
/* This is a sample routine that is meant to illustrate the
   communication options that exist for a client */
    {
    char buffer[80];
    int err;

    /* Since this routine will be called when there is input from keyboard
       need to get that msg first */
    printf ( "Input from #%d: ",fd ); /* client itself */
    err = read ( fd, buffer, 80 );
    printf ( "%d bytes read\n",err );

    /* properly terminate the buffered string */
    buffer[err]='\0';
    if ( buffer[err-1]=='\n' )
        buffer[err-1]='\0';

    fflush ( stdout );

    /* check for close conditions: -1 an error, 0 end-of-file, 4 end of
                                                                     transmission */
    if ( ( err<0 ) || ( err==0 ) || ( buffer[0]==4 ) )
        {
        printf ( "Closing standard input\n" );
        if ( err<0 )
            perror ( "Error reading stdin" );
        close ( fd );
        BusRemoveInputCallback ( bd, fd );
        }
    /* Identify the type of transmission that you want to send */

    /* I want to send a message directly to client X */
    else if ( !strcmp ( buffer, "Direct" ) ) /* See messaging protocol */
        {
        int moduleId, typeId, mode;
        char hostname[128], localFname[1024], remoteFname[1024];

        printf ( "Hostname       : " );
        fflush ( stdout );
        err = read ( fd, hostname, 80 );
        if ( err>0 )
            {
            hostname[err-1 ] = '\0';
            }
        if ( strlen ( hostname ) <1 )
            return;


        /* Establish if the message type is a registered type, and send
           whatever sendDirect routine collects directly to the destination
           client. This is meant to illustrate longer direct messages. */
        mode = 0;
        do
            {
            printf ( "Transfer Type       : Get/Put? " );
            fflush ( stdout );
            err = read ( fd, buffer, 80 );
            if ( err>0 )
                buffer[ err-1 ] = '\0';
            if ( strlen ( buffer ) <1 )
                return;

            if ( !strcmp ( buffer,"Get" ) )
                mode = FTP_GET;
            else if ( !strcmp ( buffer, "Put" ) )
                mode = FTP_PUT;
            }
        while ( ( mode != FTP_GET ) && ( mode != FTP_PUT ) );

        printf ( "Local Filename       : " );
        fflush ( stdout );
        err = read ( fd, localFname, 256 );
        if ( err>0 )
            localFname[ err-1 ] = '\0';
        if ( strlen ( localFname ) <1 )
            return;

        printf ( "Remote Filename       : " );
        fflush ( stdout );
        err = read ( fd, remoteFname, 256 );
        if ( err>0 )
            remoteFname[ err-1 ] = '\0';
        if ( strlen ( remoteFname ) <1 )
            return;

        printf ( "Calling FTP_xferBINARY with parameters : \n \t mode = %d hostname = %s \n \t localFname = %s remoteFname = %s \n",mode,hostname, localFname, remoteFname );
        FTP_xferBINARY ( bd, mode, hostname, localFname, remoteFname );
        }

    /* I really want to end this session */
    else if ( !strcmp ( buffer, "Quit" ) )
        {
        printf ( "bye-bye\n" );
        BusClose ( bd );
        exit ( 0 );
        }
    /* I want to send a message via the bus master */
    else if ( !strcmp ( buffer, "Send" ) )
        {
        struct BusMessage bmsg;
        int moduleId, typeId;

        /* identify message distribution mode */
        do
            {
            printf ( "Destination Module : " );
            fflush ( stdout );
            err = read ( fd, buffer, 80 );
            if ( err )
                buffer[ err ] = '\0';
            if ( err>0 && buffer[err-1]=='\n' )
                buffer[ err-1 ] = '\0';
            if ( strlen ( buffer ) <1 )
                return;

            if ( !strcmp ( buffer,"Everyone" ) )
                moduleId = BusBroadcast;
            else if ( !strcmp ( buffer, "By Type" ) )
                moduleId = BusByType;
            else if ( !strcmp ( buffer, "Bounce" ) )
                moduleId = BusBounce;
            else
                moduleId = BusFindModuleByName ( bd, buffer );
            }
        while ( moduleId == -1 );

        /* Specify message type */
        do
            {
            printf ( "Message Type       : " );
            fflush ( stdout );
            err = read ( fd, buffer, 80 );
            if ( err>0 )
                buffer[ err-1 ] = '\0';
            if ( strlen ( buffer ) <1 )
                return;
            typeId = BusFindTypeByName ( bd, buffer );
            }
        while ( typeId < 0 );

        /* Type in an 80 character message */
        printf ( "Message : " );
        fflush ( stdout );

        err = read ( fd, buffer, 80 );
        if ( err>=1 )
            buffer[err-1] = '\0';

        /* fill the messaging structure */
        bmsg.toModule = moduleId;
        bmsg.messageType = typeId;
        bmsg.messageLength = strlen ( buffer ) + 1;
        bmsg.message     = buffer;

        /* send the message via bus-master */
        BusSendMessage ( bd, &bmsg );
        }
    /* register a new message type */
    else if ( !strcmp ( buffer, "Register" ) )
        {
        int typeId;

        err = read ( fd, buffer, 80 );
        if ( err >= 1 )
            buffer[ err-1 ] = '\0';

        typeId = BusFindTypeByName ( bd, buffer );
        BusRegisterType ( bd, typeId );
        }
    else if ( !strcmp ( buffer, "WhosConnected" ) )
        {
        struct BusModuleData *bmd;
        int    numNodes, i;

        bmd = BusFindWhosConnected ( bd, &numNodes );
        printf ( "BusFindWhosConnected Returns : \n" );
        for ( i=0; i<numNodes; i++ )
            {
            printf ( "MODULE %d : Name= %s \t Id= %d \t IP-addr= %s \n", i,
                     bmd[i].name, bmd[i].moduleId, bmd[i].ip_addr );
            }

        }
    else if ( !strcmp ( buffer, "ModuleInfoByName" ) )
        {
        struct BusModuleData bmd;
        int    numNodes, i;

        printf ( "Module Name: " );
        fflush ( stdout );

        err = read ( fd, buffer, 80 );
        if ( err >= 1 )
            buffer[ err-1 ] = '\0';

        if ( BusGetModuleInfoByName ( bd, buffer, &bmd ) == SBUSERROR_NOT )
            {
            printf ( "BusGetModuleInfoByName : \n" );
            printf ( "Name= %s \t Id= %d \t IP-addr= %s \n",
                     bmd.name, bmd.moduleId, bmd.ip_addr );
            }
        else
            {
            printf ( "Error Obtaining data for %s \n", buffer );
            }
        }
    else if ( !strcmp ( buffer, "ModuleInfoById" ) )
        {
        struct BusModuleData bmd;
        int    moduleId, i;

        printf ( "Module Id: " );
        fflush ( stdout );

        err = read ( fd, buffer, 80 );
        sscanf ( buffer, "%d", &moduleId );

        if ( BusGetModuleInfoById ( bd, moduleId, &bmd ) == SBUSERROR_NOT )
            {
            printf ( "BusGetModuleInfoById : \n" );
            printf ( "Name= %s \t Id= %d \t IP-addr= %s \n",
                     bmd.name, bmd.moduleId, bmd.ip_addr );
            }
        else
            {
            printf ( "Error Obtaining data for %d \n", moduleId );
            }
        }
    else
        {
        printf ( "Broadcasting message \"%s\"\n",buffer );
        BusBroadcastASCIIZ ( bd, buffer );
        }
    }

int main ( int argc, char *argv[] )
    {
    int    err;        /* used to collect return status codes */
    char  name[300];       /* local name pointer */
    struct BusData bd; /* use BusData structure */
    struct hostent *h_info;
    struct in_addr *hptr;
    int    typeId;
    char hostName[256];

    /* Initialize the name of the module as "busd_123.45.56.78" where
       123.... is the ip address of the current host                 */
    gethostname ( hostName, 256 );
    h_info = gethostbyname ( hostName );
    hptr = ( struct in_addr * ) *h_info->h_addr_list;
    sprintf ( name, "busd_%s", inet_ntoa ( *hptr ) );

    /* allocate memory and copy name to client descriptior structure */
    bd.name = malloc ( strlen ( name )+1 );
    strcpy ( bd.name, name );

    /* Try to attach to the bus via SBUSPORT and SBUSHOST */
    err = BusInitialize ( &bd , 1, 60, BusExitCallback );
    if ( err != SBUSERROR_NOT )
        {
        perror ( "Could not attached to bus" );
        exit ( -1 );
        }
    else
        printf ( "Bus Connection established.\n" );

    /* Set call-backs for input, standard data types, etc. */
    /* printf("Setting callback for input from stdin\n"); */
    /*
    BusAddInputCallback(&bd,
           STDIN_FILENO,
           stdin_callback,
           NULL,
           NULL);
     */

    printf ( "Find id for type FTP_DIR_GET\n" );
    typeId = BusFindTypeByName ( &bd, "FTP_DIR_GET" );

    printf ( "Setting callback for type FTP_DIR_GET\n" );
    BusAddTypeCallback ( &bd, typeId, dir_callback );



    /* Get id for direct client-to-client BINARY communication */
    typeId = BusFindTypeByName ( &bd, "getBINARY" );
    BusAddDirectCallback ( &bd, typeId, getBINARY_callback, NULL );

    typeId = BusFindTypeByName ( &bd, "putBINARY" );
    BusAddDirectCallback ( &bd, typeId, putBINARY_callback, NULL );

    /* Keep looping, trapping bus events, and responding to them */
    printf ( "Entering BusEventLoop\n" );

    BusEventLoop ( &bd );

    /* End of story */
    return 0;
    }

