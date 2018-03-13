/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busUtil.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  busUtil.c
 *
 *     busUtil.c consists of higher level utility functions : Functions to start
 *     new clients on different machines
 *
 *     INPUT FILES:       stdin stream
 *     OUTPUT FILES:      stdout stream
 *     ERROR HANDLING:    output to stderr (screen)
 *     INPUT PARAMETERS:  client name (ascii string), optional
 *     INPUT OPTIONS:     if client name is not input, default is
 *
 *     NETWORKING:        remote/network connectivity is via bus library (sockets)
 *
 *     COMPILATION:       (see makefile)
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *
 ****************************************************************************
 *  REVISION HISTORY - busUtil.c
 *
 * Date: 17-July-95
 * Version: 0.6
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 24-July-95
 * Version: 0.6a
 * Change Description: Added function BusExitCallback
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 19-Aug-95
 * Version: 0.6b
 * Change Description: Modified BusStartClient to wait in steps of 3secs before
 *    checking if the client has started
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 8-Sept-95
 * Version: 0.6c
 * Change Description: The process that is started by "rsh" is run in the
 *    background.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 18-Sept-95
 * Version: 0.6d
 * Change Description: Do not redirect o/p of process started by 'rsh'
 *             to /dev/null.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 27-Oct-95
 * Version: 0.6d
 * Change Description: Fixed bug in getRemoteUserId where h_info was not
 *    being checked for NULL before dereferencing it.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 15-Aug-96
 * Version: 0.7
 * Change Description: Added debug statements
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "bus.h"

int BusStartModule ( struct BusData *bd, char *ipAddress, char *moduleName,
                     char *name, int secsToWait, char *args, char *errorstr );

/****************************************************************************/
/* This function returns the executable name for a given module name.. It  **/
/* looks in the file set in the SBUS_EXEC_RC env var for the module name.  **/
/* If the module name is found, the function returns the executable name,  **/
/* otherwise NULL.                                                         **/
/****************************************************************************/
char *getExecName ( char *client_name, char *errStr )
    {
    struct stat statbuf;
    FILE *execfd;
    char tmp_name[256];
    char s[160], ss[160]; /* working string */
    char *execcf = getenv ( "SBUS_EXEC_RC" );

    if ( execcf == NULL )
        {
        sprintf ( errStr, "SBUS_EXEC_RC not set" );
        return NULL;
        }
    if ( stat ( execcf, &statbuf ) )
        {
        sprintf ( errStr,"Cannot locate configuration file for executables : %s",
                  execcf );
        return NULL;
        }
    strcpy ( s,execcf );

    /* it appears to exist, try to open it */
    if ( ( execfd = fopen ( execcf, "r" ) ) )
        {
        while ( fgets ( ss, 160, execfd ) != NULL )
            {
            if ( ss[0] == '#' ) continue; /* comment, jump over */
            strcpy ( tmp_name,strtok ( ss," " ) );
            if ( ! strcmp ( client_name, tmp_name ) )
                {
                return ( strtok ( NULL," " ) );
                }
            }
        sprintf ( errStr,"Could not find entry for module name %s", client_name );
        return NULL;
        }
    else
        {
        sprintf ( errStr,"Cannot open configuration file %s", execcf );
        return NULL;
        }
    }

/****************************************************************************/
/* This function returns the userid of the current user on a remote host   **/
/* "ipAddress". The file specified in SBUS_RLOGIN_RC is scanned for the    **/
/* a remote login entry. The function returns NULL in case of error        **/
/****************************************************************************/
char *getRemoteUserID ( char *ipAddress, char *errStr )
    {
    struct hostent *h_info;
    struct in_addr *hptr;
    struct stat statbuf;
    FILE *rloginfd;
    char *rlogincf = NULL;
    char host_name[256], host_ipaddr[256];
    char s[160], ss[160]; /* working string */
    char *busowner = getenv ( "HOME" );
    char *busRloginFile = getenv ( "SBUS_RLOGIN_RC" );

    if ( busRloginFile == NULL )
        {
        sprintf ( errStr, "SBUS_RLOGIN_RC not set" );
        return NULL;
        }
    sprintf ( s,"%s/%s",busowner, busRloginFile );
    rlogincf = s;
    if ( stat ( rlogincf, &statbuf ) )
        {
        sprintf ( errStr,"Cannot locate configuration file for remote UserId: %s",
                  rlogincf );
        return NULL;
        }
    strcpy ( s,rlogincf );

    /* it appears to exist, try to open it */
    if ( ( rloginfd = fopen ( rlogincf, "r" ) ) )
        {
        while ( fgets ( ss, 160, rloginfd ) != NULL )
            {
            if ( ss[0] == '#' ) continue; /* comment, jump over */
            strcpy ( host_name,strtok ( ss," " ) );

            /* Check if the ipAddress matches the ipAdress of host_name */
            h_info = gethostbyname ( host_name );

            if ( h_info == NULL )
                {
                printf ( "Invalid hostname \"%s\" in %s \n", host_name, s );
                }
            else
                {
                while ( ( hptr = ( struct in_addr * ) *h_info->h_addr_list++ ) != NULL )
                    {
                    sprintf ( host_ipaddr, "%s", inet_ntoa ( *hptr ) );
                    if ( ! strcmp ( ipAddress, host_ipaddr ) )
                        {
                        return ( strtok ( NULL," " ) );
                        }
                    }
                }
            }
        sprintf ( errStr,"Could not find entry for host %s", ipAddress );
        return NULL;
        }
    else
        {
        sprintf ( errStr,"Cannot open configuration file %s", rlogincf );
        return NULL;
        }
    }

/****************************************************************************/
/* This function checks if the host in "ipAddress" is the Local host       **/
/****************************************************************************/
int isLocal ( char *ipAddress )
    {
    char local_hname[256];
    struct hostent *h_info;
    struct in_addr *hptr;

    /* Get IP information of local host */
    gethostname ( local_hname, 256 );
    h_info = gethostbyname ( local_hname );

    /* Check if the IP address of the file host matches that of the local
       machine */
    while ( ( hptr = ( struct in_addr * ) *h_info->h_addr_list++ ) != NULL )
        {
        sprintf ( local_hname, "%s", inet_ntoa ( *hptr ) );
        if ( ! strcmp ( ipAddress, local_hname ) )
            {
            return 1;    /* Match found */
            }
        }
    return 0;
    }

/****************************************************************************/
/* This function returns the IP address of the local host in the variable  **/
/*  'ipaddr'.                                                              **/
/****************************************************************************/
void BusGetMyIPaddress ( char *ipaddr )
    {
    char hname[256];
    struct hostent *h_info;
    struct in_addr *hptr;

    gethostname ( hname, 256 );
    h_info = gethostbyname ( hname );

    hptr = ( struct in_addr * ) *h_info->h_addr_list;

    /* Fill in the variable "ipaddr" with the IPaddress of the host */
    sprintf ( ipaddr,"%s", inet_ntoa ( *hptr ) );
    }

/****************************************************************************/
/** BusVerifyClient checks if a client 'moduleName'/daemon exists on the   **/
/** machine 'ipAddress' and starts one up, if it does'nt exist.            **/
/****************************************************************************/
int BusVerifyClient ( struct BusData *bd, char *ipAddress, char *name,
                      int isUnique, int secsToWait, char *args, char *errorstr )
    {

    char myIPaddr[256];
    char moduleName[256];

    debug0 ( DEBUG_CLIENT,"BusVerifyClient \n" );
    /* If 'ipAddress == NULL' set ipAddress to that of the local host */
    if ( ipAddress == NULL )
        {
        BusGetMyIPaddress ( myIPaddr );
        ipAddress = myIPaddr;
        }
    /* Note that isUnique is used to calculate whether the clients's name on
     * the bus has the IP address at the end of it.  An alternative would
     * be to put the IP address at the end of each client's name
     */
    if ( isUnique )
        {
        strcpy ( moduleName, name );
        }
    else
        {
        sprintf ( moduleName,"%s_%s",name,ipAddress );
        }

    if ( BusFindModuleByName ( bd, moduleName ) != FIND_ID_ERR )
        {
        /* module is on bus */
        sprintf ( errorstr, "Module already on Bus \n" );
        debug0 ( DEBUG_CLIENT,"BusVerifyClient : Returning \n" );
        return 0;
        }
    else
        {
        /* module is not on the bus, so start it */
        return BusStartModule ( bd, ipAddress, moduleName, name,
                                secsToWait, args, errorstr  );
        }
    }

int BusStartModule ( struct BusData *bd, char *ipAddress, char *moduleName,
                     char *name, int secsToWait, char *args, char *errorstr )
    {
    char *tmpName, execName[256];
    char remUserId[256], *tmpUserId;
    char *command;
    char *sbushost, myIPaddr[256];
    int pid;

    /* debug1(DEBUG_CLIENT,"BusStartModule : %s \n", moduleName); */
    /* Get the executable name for the module */
    tmpName = getExecName ( name, errorstr );
    if ( tmpName == NULL )
        strcpy ( execName, name );
    else
        strcpy ( execName, tmpName );
    if ( execName[strlen ( execName )-1] == '\n' )
        execName[strlen ( execName )-1] = '\0';

    /* printf("Executable name = %s \n", execName ); */

    if ( args != NULL )
        command = ( char * ) malloc ( 512+strlen ( args ) );
    else
        command = ( char * ) malloc ( 512 );

    if ( ( sbushost = getenv ( "SBUSHOST" ) ) == NULL )
        {
        BusGetMyIPaddress ( myIPaddr );
        sbushost = myIPaddr;
        }
    if ( isLocal ( ipAddress ) )
        {
        if ( args != NULL )
            sprintf ( command,"env SBUSPORT=%s SBUSHOST=%s %s %s &",
                      getenv ( "SBUSPORT" ), sbushost, execName, args );
        else
            sprintf ( command,"env SBUSPORT=%s SBUSHOST=%s %s &",
                      getenv ( "SBUSPORT" ), sbushost, execName );
        }
    else
        {
        /* Start module on remote host */
        tmpUserId = getRemoteUserID ( ipAddress, errorstr );
        /* This reads the file ~/.pave_rlogin_rc */
        if ( tmpUserId == NULL )
            strcpy ( remUserId, BusGetMyUserid() );
        else
            strcpy ( remUserId, tmpUserId );
        if ( remUserId[strlen ( remUserId )-1] == '\n' )
            remUserId[strlen ( remUserId )-1] = '\0';

        if ( args != NULL )
            sprintf ( command,"rsh -n -l %s %s 'env SBUSPORT=%s SBUSHOST=%s %s %s &'&",
                      remUserId, ipAddress, getenv ( "SBUSPORT" ), sbushost, execName, args );
        else
            sprintf ( command,"rsh -n -l %s %s 'env SBUSPORT=%s SBUSHOST=%s %s  &'&",
                      remUserId, ipAddress, getenv ( "SBUSPORT" ), sbushost, execName );
        }

    /* printf("Command = %s \n", command);  */
    /* this does not compile on the CRAY
    #ifdef 0
         system(command);
    #endif
    */

    if ( ( pid = fork() ) == 0 )
        {
        execlp ( "/bin/sh", "sh", "-c", command, ( char * ) 0 );
        perror ( "Could not start the application" );
        exit ( 10 );
        }
    else if ( pid == -1 )
        {
        perror ( "Could not create new process " );
        debug0 ( DEBUG_CLIENT,"BusVerifyClient : Returning \n" );
        return -1;
        }
    else
        {

        free ( command );
        while ( secsToWait > 0 )
            {
            sleep ( 2 );
            if ( BusFindModuleByName ( bd, moduleName ) != FIND_ID_ERR )
                {
                sprintf ( errorstr, "Started client %s successfully \n", moduleName );

                /* Give the client time to register its message types */
                /* sleep(4); */
                debug0 ( DEBUG_CLIENT,"BusVerifyClient : Returning \n" );
                return 0;
                }
            secsToWait -= 2;
            }
        /* Starting the module failed, so return an error code */
        sprintf ( errorstr,"Could not start %s on host at address %s",
                  moduleName, ipAddress );
        debug0 ( DEBUG_CLIENT,"BusVerifyClient : Returning \n" );
        return -1;
        }
    }

void BusExitCallback ( struct BusData *bd )
    {
    BusClose ( bd );
    exit ( 0 );
    }

