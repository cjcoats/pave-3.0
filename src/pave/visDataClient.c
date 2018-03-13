/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: visDataClient.c 83 2018-03-12 19:24:33Z coats $
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
 *  REVISION HISTORY
 *      Author:      Rajini Balay, NCSU,  February 25, 1995
 *****************************************************************************/

/* bald messes this up in some header file */
#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */

#include "visDataClient.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "busClient.h"
#include "busMsgQue.h"
#include "busError.h"
#include "busDebug.h"
#include "busXtClient.h"
#include "busRW.h"
#include "busVersion.h"
#include "busRpc.h"
#include "busUtil.h"

/* get_info : checks if the file is on the local machine or on a remote
 * machine and calls the appropriate function
 */
int get_info ( struct BusData *bd, VIS_DATA *info, char *message )
    {
    int err;

#ifdef DIAGNOSTICS
    if ( info ) if ( info->filename ) fprintf ( stderr, "Enter get_info() with '%s'\n",
                    info->filename );
#endif /* #ifdef DIAGNOSTICS */

    err = check_local_file ( info );

    if ( err == -1 )     /* Invalid hostname */
        {
        sprintf ( message, "Invalid hostname : %s ", info->filehost.name );
        return FAILURE;
        }

    if ( ( err == 1 ) || ( bd == NULL ) )
        {
        return get_info_local ( info, message );
        }

    if ( info->grid != NULL )
        {
        free ( info->grid );
        info->grid = NULL;
        }
    if ( info->sdate != NULL )
        {
        free ( info->sdate );
        info->sdate = NULL;
        }
    if ( info->stime != NULL )
        {
        free ( info->stime );
        info->stime = NULL;
        }
    return get_remote ( bd, GET_INFO, info, message );
    }


/* get_data : checks if the file is on the local machine or on a remote
 * machine and calls the appropriate function
 * Returns nonzero if success; 0 if failure
 */
int get_data ( struct BusData *bd, VIS_DATA *info, char *message )
    {
    int err;
    int i;

    err = check_local_file ( info );

    if ( err == -1 )     /* Invalid hostname */
        {
        err = FAILURE;
#ifdef DIAGNOSTICS
        fprintf ( stderr, "get_data 1 about to return %d\n",err ); /* added 951025 SRT */
#endif /* DIAGNOSTICS */
        return err;
        }

    if ( ( err == 1 ) || ( bd == NULL ) )
        {
        err = get_data_local ( info, message );
#ifdef DIAGNOSTICS
        fprintf ( stderr, "get_data 2 about to return %d\n",err ); /* added 951025 SRT */
#endif /* DIAGNOSTICS */
        return err;
        }

    err = get_remote ( bd, GET_DATA, info, message );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "get_data 3 about to return %d\n",err ); /* added 951025 SRT */
#endif /* DIAGNOSTICS */
    return err;
    }


/* Function to check if the file specified in the VIS_DATA sturcture
   resides on local host or a remote host. The function returns a 1, if
   the file is on the local host, and 0 if the file is on a remote machine
   and a -1 if the hostname is invalid.
*/
int check_local_file ( VIS_DATA *info )
    {
    char **hsav; /*added 950724 SRT*/
    struct hostent *h_info;
    struct in_addr *hptr;
    char local_hname[256];
    char file_ipaddr[256];

    if ( getenv ( "USE_LOCAL_VISD" ) )
        {
        /* Find IP address of local machine */
        gethostname ( local_hname, 256 );
        info->filehost.name = strdup ( local_hname );
        /* lie about file is remote to force visd */
        }

    /* Check if the hostname field  of VISDATA structure has been filled */
    if ( info->filehost.name == NULL )
        return 1;         /* Consider it as local host */

    /* Find IP address of the machine where the file is located */
    if ( info->filehost.ip == NULL )
        {

        /*SRT NOTE: if info->filehost.name == NULL this should return
              from this routine with an error flag, rather than
              calling gethostbyname which will do a core dump */

        h_info = gethostbyname ( info->filehost.name );
        /*SRT*/ if ( h_info == ( struct hostent * ) NULL )
            {
#ifdef DIAGNOSTICS
            fprintf ( stderr,
                      "gethostbyname() returned NULL in visDataClient.c!\n" );
#endif /* DIAGNOSTICS */
            return -1;      /* Illegal hostname */
            }
        hptr = ( struct in_addr * ) *h_info->h_addr_list;
        sprintf ( file_ipaddr,"%s", inet_ntoa ( *hptr ) );
        }
    else
        {
        strcpy ( file_ipaddr, info->filehost.ip );
        }

    /* Find IP address of local machine */
    gethostname ( local_hname, 256 );
    h_info = gethostbyname ( local_hname );

    /* Check if the IP address of the file host matches that of the local
       machine */
    /* hptr = (struct in_addr *) *h_info->h_addr_list; /*SRT 950724*/
    /* while (hptr != NULL) /*SRT 950724*/
    hsav = h_info->h_addr_list; /*added SRT 950724*/
    while ( ( hptr = ( struct in_addr * ) *h_info->h_addr_list++ ) != NULL )
        {
        sprintf ( local_hname, "%s", inet_ntoa ( *hptr ) );
        if ( !strcmp ( file_ipaddr, local_hname ) )
            {
            h_info->h_addr_list = hsav; /*added SRT 950724*/
            return 1;   /* Match found */
            }
        /* hptr++; /*SRT 950724*/
        }
    h_info->h_addr_list = hsav; /*added SRT 950724*/
    return 0;    /* File is not on local machine */
    }

/* get_remote : Establishes a Direct Connection with the remote host
 * through the Software Bus and sends the parameters to the function
 * get_info_local and receives the results on the socket.
 */
int get_remote ( struct BusData *bd, int code, VIS_DATA *info, char *message )
    {
    struct BusMessage bmsg;
    char moduleName[256];
    int moduleId = -1, typeId;
    int err;
    char **hsav;
    struct hostent *h_info;
    struct in_addr *hptr;
    char tmp_msg[512], ipaddress[80];
    int res, i;

    /* Check if its a valid host and the ftp deamon exists on that machine */
    res = is_localhost ( info->filehost.name, ipaddress );
    if ( res == -1 )  /* Illegal hostname */
        {
        sprintf ( message, "%s: Illegal hostname", info->filehost.name );
#ifdef DIAGNOSTICS
        fprintf ( stderr, "1 get_remote() ERROR : %s\n", message );
#endif /* DIAGNOSTICS */
        return FAILURE;
        }
    else if ( res == 0 ) /* then it is a remote host - check for a visd */
        {
        sprintf ( moduleName, "visd_%s", ipaddress );
        moduleId = BusFindModuleByName ( bd, moduleName );
        if ( moduleId < 0 ) /* we need to start the visd daemon */
            {
            BusVerifyClient ( bd, ipaddress, "visd", 1, 18, NULL, message );
            moduleId = BusFindModuleByName ( bd, moduleName );
            }
        if ( moduleId < 0 ) /* we couldn't start the visd daemon */
            return FAILURE;
        }

    if ( code == GET_INFO )
        typeId = BusFindTypeByName ( bd, "EVAP_GetInfo" );
    else
        typeId = BusFindTypeByName ( bd, "EVAP_GetData" );

    /* Send a Direct Connection Request and call the function "EVAPLocalStub
       when the connection is setup
     */
    err = BusCallRemote ( bd, moduleId, typeId, EVAPLocalStub, ( char * ) info, tmp_msg );
    if ( err == SBUSERROR_NOT )
        {
        sscanf ( tmp_msg,"%d %s",&err, message );
#ifdef DIAGNOSTICS
        fprintf ( stderr, "2 get_remote() returning %d\n", err );
#endif /* DIAGNOSTICS */
        return err;
        }
    else
        {
#ifdef DIAGNOSTICS
        fprintf ( stderr, "3 get_remote() returning FAILURE\n", message );
#endif /* DIAGNOSTICS */
        return FAILURE;
        }
    }


/* Stub function for packing the parameters and receiving the results
 */
void EVAPLocalStub ( int fd, char *data, char *res )
    {
    VIS_DATA *info;
    char *msg, *tfname;
    int val, err;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "EVAPLocalStub : Direct Connection has been set up \n" );
#endif /* DIAGNOSTICS */
    info = ( VIS_DATA * ) data;
    if ( ( err = sendVisData ( fd, info ) ) == XFER_ERR )
        {
        sprintf ( res, "%d  ", err );
        return;
        }
    if ( ( err = getInteger ( fd, &val ) ) == XFER_ERR )
        {
        sprintf ( res, "%d  ", err );
        return;
        }
    if ( val == FAILURE )
        {
        getString ( fd, &msg );
        printf ( "%s \n", msg );
        sprintf ( res, "%d %s", val, msg );
        return;
        }
    else
        sprintf ( res, "%d", val );
    tfname = info->filename; /* SRT 961024 memory management */
    if ( ( err = getVisData ( fd, info ) ) == XFER_ERR )
        {
        sprintf ( res, "%d  ", err );
        return;
        }
    if ( tfname != info->filename ) /* SRT 961024 memory management */
        if ( tfname )      /* SRT 961024 memory management */
            {
            /* SRT 961024 memory management */
            free ( tfname );    /* SRT 961024 memory management */
            tfname = NULL;      /* SRT 961024 memory management */
            }           /* SRT 961024 memory management */
    }

/* This Function attaches the client to the BusMaster -
      The name of the module is "infod_123.45.67.89" where
    123.45.67.89 is the ip-address of the host where the client runs
 */
int initVisDataClient ( struct BusData *bd, char *modName )
    {
    struct hostent *h_info;
    struct in_addr *hptr;
    char local_hname[256];
    int err, typeId;

    gethostname ( local_hname, 256 );
    h_info = gethostbyname ( local_hname );
    hptr = ( struct in_addr * ) *h_info->h_addr_list;
    sprintf ( local_hname, "%s_%s", modName, inet_ntoa ( *hptr ) );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Adding module by name %s \n" , local_hname );
#endif

    bd->name = strdup ( local_hname ); /*(char *)malloc(strlen(local_hname));SRT*/
    if ( !bd->name )                 /* added SRT */
        {
        /* added SRT */
        perror ( "strdup failed in visDataClient.c\n" ); /* added SRT */
        return SBUSERROR_NOT+1;          /* added SRT */
        }                        /* added SRT */
    strcpy ( bd->name, local_hname );
    err = BusInitialize ( bd , 1, 60, BusExitCallback ); /* 950731 added args SRT*/
    if ( err != SBUSERROR_NOT )
        {
        perror ( "Could not attached to bus" );
        return err;
        }

#ifdef DIAGNOSTICS
    else
        printf ( "Bus Connection established.\n" );
#endif

    /* Get id for remote GetInfo function */
    typeId = BusFindTypeByName ( bd, "EVAP_GetInfo" );
    BusAddDirectCallback ( bd, typeId, EVAP_GetInfo, NULL );

    /* Get id for remote GetData function */
    typeId = BusFindTypeByName ( bd, "EVAP_GetData" );
    BusAddDirectCallback ( bd, typeId, EVAP_GetData, NULL );

    return SBUSERROR_NOT;
    }

/* Callback for GetInfo on the remote machine.. The function calls
   get_local_info on the current machine and sends the results back on
   the socket
 */
void EVAP_GetInfo ( int fd, char *data )
    {
    VIS_DATA info;
    char message[512];
    int val;
    int err;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "EVAP_GetInfo : getting VisData information \n" );
#endif /* DIAGNOSTICS */

    init_vis (info ) ;
    if ( ( err = getVisData ( fd, &info ) ) == XFER_ERR )
        {
        fprintf ( stderr,
                  "EVAP_GetInfo() ERROR in receiving the VisData Structure \n" );
        sendInteger ( fd, 1, &val );
        sendString ( fd, message, strlen ( message ) );
        return;
        }
#ifdef DIAGNOSTICS
    print_vis_data ( &info );
#endif /* DIAGNOSTICS */
    val = get_info_local ( &info, message );
    if ( val == FAILURE )
        {
        fprintf ( stderr, "ERROR in executing GETINFO \n" );
        fprintf ( stderr, "%s \n", message );
        sendInteger ( fd, 1, &val );
        sendString ( fd, message, strlen ( message ) );
        return;
        }
    else
        {
        sendInteger ( fd, 1, &val );
        }
#ifdef DIAGNOSTICS
    dump_VIS_DATA ( &info, NULL, NULL );
#endif /* DIAGNOSTICS */
    sendVisData ( fd, &info );
    if ( info.data_label ) free ( info.data_label );
    info.data_label=NULL; /* SRT 971223 */
    }

/* Callback for GetData on the remote machine.. The function calls
   get_local_data on the current machine and sends the results back on
   the socket
 */
void EVAP_GetData ( int fd, char *data )
    {
    VIS_DATA info;
    char message[512];
    int val, err;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "EVAP_GetData : getting VisData information \n" );
#endif /* DIAGNOSTICS */

    init_vis ( info ) ;
    if ( ( err = getVisData ( fd, &info ) ) == XFER_ERR )
        {
        fprintf ( stderr,
                  "EVAP_GetData() ERROR in receiving the VisData Structure \n" );
        sendInteger ( fd, 1, &val );
        sendString ( fd, message, strlen ( message ) );
        return;
        }
#ifdef DIAGNOSTICS
    print_vis_data ( &info );
#endif /* DIAGNOSTICS */
    val = get_data_local ( &info, message );
    if ( val == FAILURE )
        {
        fprintf ( stderr, "ERROR in executing GETDATA \n" );
        sendInteger ( fd, 1, &val );
        sendString ( fd, message, strlen ( message ) );
        return;
        }
    else
        sendInteger ( fd, 1, &val );

#ifdef DIAGNOSTICS
    print_vis_data ( &info );
#endif /* DIAGNOSTICS */
    sendVisData ( fd, &info );
    }
