/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busFtpIntrnl.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  - busFtpIntrnl.c
 *
 *     busFtpIntrnl.c contains internal functions used by the client interface
 *     functions in busFtp.c.
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
 * REVISION HISTORY - busFtpIntrnl.c
 *
 * Date: 1-Feb-95
 * Version: 0.3
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 20-Mar-95
 * Version: 0.5
 * Change Description: Added functions for the getDir functionality.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 7-Sept-95
 * Version: 0.5.3
 * Change Description: readfromfileBINARY and writetofileBINARY synchronize
 *    fopen errors and then tranfer files.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <string.h>
#include "busFtp.h"

int readfromfileBINARY ( int fd, char *filename )
    {
    FILE *fp;
    char buf[MAXDATA];
    int err, err1, err2;

    debug1 ( DEBUG_FTP, " readfromfileBINARY : %s \n", filename );
    if ( ( fp = fopen ( filename, "r" ) ) == NULL )
        {
        printf ( "Error opening file : %s for reading \n", filename );
        /*
        buf[0] = 4;
        write(fd, buf, 1);
        close(fd);
        return FTP_ERR_FOPEN;
         */
        err1 = 1;
        }
    else
        err1 = 0;

    BusWriteInteger ( fd, err1 );

    /* Read error status from other end */
    BusReadInteger ( fd, &err2 );

    if ( ( err1 == 1 ) || ( err2 == 1 ) )
        return FTP_ERR_FOPEN;

    while ( ( err = read ( fileno ( fp ), buf, MAXDATA ) ) > 0 )
        {
        write ( fd, buf, err );
        }
    return FTP_ERR_NONE;
    }

int writetofileBINARY ( int fd, char *filename )
    {
    FILE *fp;
    char buf[MAXDATA];
    int err, err1, err2;

    debug1 ( DEBUG_FTP, " writetofileBINARY : %s \n", filename );
    if ( ( fp = fopen ( filename, "w" ) ) == NULL )
        {
        printf ( "Error opening file : %s for writing \n", filename );
        err1 = 1;
        /*
        close(fd);
        return FTP_ERR_FOPEN;
               */
        }
    else
        err1 = 0;

    BusWriteInteger ( fd, err1 );

    /* Read error status from other end */
    BusReadInteger ( fd, &err2 );

    if ( ( err1 == 1 ) || ( err2 == 1 ) )
        return FTP_ERR_FOPEN;

    while ( ( err = read ( fd, buf, MAXDATA ) ) > 0 )
        {
        write ( fileno ( fp ), buf, err );
        }
    return FTP_ERR_NONE;
    }

void DirectBINARY ( int fd, char *data )
    {
    int mode;
    char *token;
    int err;

    debug0 ( DEBUG_FTP, "Direct Connection has been set up: Transferrring the file now \n" );
    token = strtok ( data, " " );
    mode = atoi ( token );

    token = strtok ( NULL, " " ); /* Read the remote file name */

    /* Write the filename on the socket */
    BusWriteASCIIZ ( fd, token );
    token = strtok ( NULL, " " ); /* Read the local file name */

    if ( mode == FTP_GET )
        err = writetofileBINARY ( fd, token );
    else if ( mode == FTP_PUT )
        err = readfromfileBINARY ( fd, token );
    if ( err != FTP_ERR_NONE )
        sprintf ( data, "0" );
    else
        sprintf ( data, "1" );
    }

void getBINARY_callback ( int fd, char *data )
    {
    char *filename;

    debug0 ( DEBUG_FTP,"In getBINARY_callback \n" );
    BusReadASCIIZ ( fd, &filename );
    readfromfileBINARY ( fd, filename );

    }

void putBINARY_callback ( int fd, char *data )
    {
    char *filename;

    debug0 ( DEBUG_FTP, "In putBINARY_callback \n" );
    BusReadASCIIZ ( fd, &filename );
    writetofileBINARY ( fd, filename );
    }

void dir_callback ( struct BusData *bd, struct BusMessage *bmsg )
    {
    char *buf, *fileList[MAXNUMFILES];
    int nfiles, i, returnType;
    struct BusMessage new_bmsg;


    if ( bmsg->messageLength <= 0 )
        {
        buf = malloc ( 256 );
        sprintf ( buf, "DIR_OPEN_ERR" );
        }
    else
        {
        int selectCode;
        char *dir;

        dir = malloc ( bmsg->messageLength );
        sscanf ( bmsg->message, "%d %d %s", &selectCode, &returnType, dir );

        debug2 ( DEBUG_FTP, "dir_callback : Code = %d returnType =%d \n",
                 selectCode, returnType );
        nfiles = FTP_dirLocal ( dir, selectCode, fileList );
        /* printf("Number of entries = %d Code = %d \n", nfiles, selectCode); */
        if ( nfiles > 0 )
            {
            char tmp_buf[260];

            buf = malloc ( 256*nfiles+8 );
            sprintf ( buf, "%d %d", selectCode, nfiles );
            for ( i=0; i<nfiles; i++ )
                {
                sprintf ( tmp_buf, " %s", fileList[i] );
                /* printf("buf = %s \n", buf);  */
                strcat ( buf, tmp_buf );
                }
            }
        else
            {
            buf = malloc ( 32 );
            sprintf ( buf, "%d %d", selectCode, nfiles );
            }
        free ( dir );
        }
    /* Send a reply back */
    new_bmsg.toModule = bmsg->fromModule;
    new_bmsg.fromModule = bd->moduleId;
    new_bmsg.messageLength = strlen ( buf ) + 1;
    new_bmsg.message = buf;

    /*
    printf("Sending a message of type FTP_DIR_HERE \n");
    new_bmsg.messageType = BusFindTypeByName( bd, "FTP_DIR_HERE" );
    */
    new_bmsg.messageType = returnType;
    BusSendMessage ( bd, &new_bmsg );

    free ( buf );
    }

