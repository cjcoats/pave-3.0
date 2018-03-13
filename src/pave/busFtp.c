/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busFtp.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  - busFtp.c
 *
 *     busFtp.c comprises of utility functions for a Bus Client to enable
 *     transfer of files between machines through Direct Communication.
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
 ********************************************************************
 * REVISION HISTORY - busFtp.c
 *
 * Date: 1-Feb-95
 * Version: 0.3
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 20-Mar-95
 * Version: 0.5
 * Change Description: Added functions to get contents of a directory from
 *            remote machines.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 16-Aug-95
 * Version: 0.5.1
 * Change Description: FTP_dirLocal uses the 'qsort' function to sort the
 *                     directory and filenames.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 1-Sept-95
 * Version: 0.5.2
 * Change Description: If the directory is "/~' FTP_dirLocal returns the
 *            contents of the home directory.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 7-Sept-95
 * Version: 0.5.3
 * Change Description: DirectBinary returns FOPEN errors
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <string.h>
#include "busFtp.h"

/* This file contains the FTP functions that a Client module can call:

   FTP_xferBINARY(struct BusData *bd, int mode, char *hostName,
          char *localFile, char *remoteFile)
   FTP_dirLocal()
 */
int readfromfileBINARY ( int fd, char *filename );
int writetofileBINARY ( int fd, char *filename );
void DirectBINARY ( int fd, char *data );
void getBINARY_callback ( int fd, char *data );
void putBINARY_callback ( int fd, char *data );
void dir_callback ( struct BusData *bd, struct BusMessage *bmsg );

int fileCompare ( const void *file1, const void *file2 )
    {
    return strcmp ( * ( char ** ) file1, * ( char ** ) file2 );
    }

/* This function opens a direct connection between the
 * local host and "hostName" and transfers a file in the
 * mode (FTP_GET/FTP_PUT) between "localFile" and
 * "remoteFile".
 *
 *  Return values :
 *  FTP_ERR_HOST : Host not found / FTP deamon on that host is not found
 *  FTP_ERR_LOCALHOST : Requested transfer between the local host
 *  FTP_ERR_NONE : Successful tranfer of file
 */
int FTP_xferBINARY ( struct BusData *bd, int mode, char *hostName,
                     char *localFile, char *remoteFile )
    {
    char tmp_buf[256], *data, ipaddress[80];
    int moduleId, typeId, err, err1;

    debug3 ( DEBUG_FTP,"FTP_xferBINARY : hostname = %s \n localFile = %s remoteFile = %s \n", hostName, localFile, remoteFile );

    err = is_localhost ( hostName, ipaddress );
    if ( err == -1 )
        return FTP_ERR_HOST;   /* Illegal hostname */
    else if ( err == 1 )
        {
        printf ( "Hostname specified is the local host - cannot do FTP \n" );
        return FTP_ERR_LOCALHOST;  /* Its a local host - why FTP? */
        }
    else if ( err != 0 )
        return FTP_ERR_HOST;    /* Something wrong with is_localhost */

    sprintf ( tmp_buf,"busd_%s", ipaddress );

    moduleId = BusFindModuleByName ( bd, tmp_buf );
    if ( moduleId < 0 )
        return FTP_ERR_HOST;
    if ( mode == FTP_GET )
        {
        typeId = BusFindTypeByName ( bd, "getBINARY" );
        debug0 ( DEBUG_FTP,"    mode = FTP_GET \n" );
        }
    else if ( mode == FTP_PUT )
        {
        typeId = BusFindTypeByName ( bd, "putBINARY" );
        debug0 ( DEBUG_FTP, "   mode = FTP_PUT \n" );
        }

    data = malloc ( strlen ( localFile ) + strlen ( remoteFile ) + sizeof ( int )+5 );
    sprintf ( data, "%d %s %s", mode, remoteFile, localFile );
    err = BusSendDirect ( bd, moduleId, typeId, DirectBINARY, data );

    sscanf ( data, "%d", &err1 );
    /* printf("err1 = %d \n", err1); */
    if ( strlen ( data ) > 0 )
        free ( data );
    if ( err != SBUSERROR_NOT )
        return FTP_ERR_CONNECT;
    if ( err1 != 1 )
        return FTP_ERR_FOPEN;
    return FTP_ERR_NONE;
    }

int FTP_getdir ( struct BusData *bd, char *hname, char *directory, int code,
                 int returnType, char **fileList )
    {
    char ipaddr[256];
    int err;

    err = is_localhost ( hname, ipaddr );
    /* printf("getdir : return value of is_localhost = %d \n", err); */

    if ( err == 1 )
        {
        return FTP_dirLocal ( directory, code, fileList );
        }
    else if ( err == 0 )
        {
        /* printf("Calling get_dir_remote \n"); */
        return FTP_dirRemote ( bd, ipaddr, directory, code, returnType );
        }
    else
        return DIR_HOST_UNKNOWN;
    }


int FTP_dirRemote ( struct BusData *bd, char *ipaddr, char *directory,
                    int code, int returnType )
    {
    char moduleName[512];
    int moduleId;
    struct BusMessage bmsg;
    char *buf;

    /* printf("Get_dir_remote \n"); */
    sprintf ( moduleName, "busd_%s", ipaddr );
    moduleId = BusFindModuleByName ( bd, moduleName );

    if ( moduleId == FIND_ID_ERR )
        return DIR_HOST_UNKNOWN;

    /* Send a message to the ftp deamon on "ipaddr" to do a get_dir_local
       on that machine */
    bmsg.toModule = moduleId;
    bmsg.fromModule = bd->moduleId;
    bmsg.messageType = BusFindTypeByName ( bd, "FTP_DIR_GET" );

    buf = malloc ( strlen ( directory )+10 );
    sprintf ( buf, "%d %d %s", code, returnType, directory );

    bmsg.messageLength = strlen ( buf ) + 1;
    bmsg.message = buf;
    BusSendMessage ( bd, &bmsg );
    /* printf("Sent a message of type FTP_DIR_GET \n"); */

    free ( buf );

    return DIR_REMOTE_FETCH;
    }

/* This function selects directories or files in "directory"  */
int FTP_dirLocal ( char *directory, int code, char **fList )
    {
    int i = 0;
    char item[BUFSIZE];
    DIR *dirp;
    /*
    struct direct *dp;
    */
    struct dirent *dp;
    char *path;

    debug1 ( DEBUG_FTP, "FTP_dirLocal : %s \n", directory );
    if ( ( strlen ( directory ) > 2 ) && ( directory[0] == '/' ) &&
            ( directory[1] == '~' ) )
        {
        path = getenv ( "HOME" );
        if ( path == NULL )
            {
            path = "/";
            }
        }
    else
        {
        path = directory;
        }
    if ( ( dirp = opendir ( path ) ) == NULL )
        {
        debug0 ( DEBUG_FTP, "  Returning DIR_OPEN_ERR \n" );
        return DIR_OPEN_ERR;
        }

    for ( i=0, dp = readdir ( dirp ); dp != NULL; dp=readdir ( dirp ) )
        {
        if ( directory[strlen ( path )-1] != '/' )
            sprintf ( item, "%s/%s", path, dp->d_name );
        else
            sprintf ( item, "%s%s", path, dp->d_name );
        if ( is_accessible ( item ) == code ) /* Choose between dir and file */
            {
            /* printf("    item is accessible \n"); */
            if ( code == IS_DIR )
                {
                fList[i] = malloc ( strlen ( item )+1 );
                strcpy ( fList[i], item );
                fList[i][strlen ( item )] = '\0';
                }
            else if ( code == IS_FILE )
                {
                /*
                fList[i] = malloc(strlen(item)+1);
                  strcpy(fList[i], item);
                  fList[i][strlen(item)] = '\0';
                              */
                fList[i] = malloc ( strlen ( dp->d_name )+1 );
                strcpy ( fList[i], dp->d_name );
                fList[i][strlen ( dp->d_name )] = '\0';
                }
            debug2 ( DEBUG_FTP, "     fList[%d] = %s \n", i, fList[i] );
            i++;
            }
        }
    closedir ( dirp );

    /* Sort the list */
    qsort ( ( void * ) fList, i, sizeof ( char * ), fileCompare );

    debug1 ( DEBUG_FTP, "FTP_dirLocal returning %d \n", i );
    return i;
    }

/**************************************************************************
   Routine to check if the hostname given in the variable "hname" is the
   local hostname. It also checks for the validity of the hostname. If the
   name is valid, it fills in the variable "ipaddr" with the IPaddress
   of the hostname. Return values :
   -1 : Invalid hostname (host does not exist)
    0 : Not the local host.
    1 : "hname" is the local host.
****************************************************************************/
int is_localhost ( char *hname, char *ipaddr )
    {
    struct hostent *h_info;
    struct in_addr *hptr;
    char local_hname[256];

    /* Obtain IP address of "hname" */
    h_info = gethostbyname ( hname );

    if ( h_info == NULL )
        {
        debug1 ( DEBUG_FTP, "is_localhost : %s does not exist \n", hname );
        return -1;
        }


    hptr = ( struct in_addr * ) *h_info->h_addr_list;

    /* Illegal hostname */
    if ( hptr == NULL ) return -1;

    /* Fill in the variable "ipaddr" with the IPaddress of the host */
    sprintf ( ipaddr,"%s", inet_ntoa ( *hptr ) );

    /* Get IP information of local host */
    gethostname ( local_hname, 256 );
    h_info = gethostbyname ( local_hname );

    /* Check if the IP address of the file host matches that of the local
       machine */
    while ( ( hptr = ( struct in_addr * ) *h_info->h_addr_list++ ) != NULL )
        {
        sprintf ( local_hname, "%s", inet_ntoa ( *hptr ) );
        if ( ! strcmp ( ipaddr, local_hname ) )
            {
            return 1;    /* Match found */
            }
        }
    return 0;    /* File is not on local machine */

    }

/* routine to determine if a file is accessible, or a directory,
 * Return -1 on all errors or if the file is not
 * accessible.  Return 0 if it's a directory or 1 if it's a plain
 * accessible file.
 */
int is_accessible ( char *file )
    {
    struct stat s_buf;

    /* if file can't be accessed (via stat()) return. */
    if ( stat ( file, &s_buf ) == -1 )
        return -1;
    else if ( ( s_buf.st_mode & S_IFMT ) == S_IFDIR )
        return 0; /* a directory */
    else if ( ! ( s_buf.st_mode & S_IFREG ) || access ( file, F_OK ) == -1 )
        /* not a normal file or it is not accessible */
        return -1;

    /* legitimate file */
    return 1;
    }

