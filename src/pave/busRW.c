/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busRW.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  - busRW.c
 *
 *        Library of bus read/write functions for different data types
 *        including data format translations.
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *
 ********************************************************************
 * REVISION HISTORY - busRW.c
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
 * Date: 9-July-95
 * Version: 0.5c
 * Change Description: Added function "BusGetMyUserid()" as a replacement for
 *                     "cuserid()"
 * Change author: R. Balay
 *
 * Date: 19-Aug-95
 * Version: 0.6b
 * Change Description: Modified "BusGetMyUserid()" to use "cuserid()"
 *                    if getenv("USER") returns NULL
 * Change author: R. Balay
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#include <stdio.h>
#include <sys/types.h>    /* sys/types.h needed for netinet/in.h */
#include <rpc/types.h>
#include <rpc/xdr.h>
#ifndef __OPENNT /* this was around rpc incs */
#endif

/* #include <netinet/in.h> */  /* definition of htonl */
#include <string.h>       /* strcpy, strncpy     */
#include <malloc.h>
#include <unistd.h>       /* read, write, sleep  */
#include <stdlib.h>       /* getenv */

#include "busRW.h"
#include "busError.h"

#define READERR(err)  \
(err<0?SBUSERROR_READ:((err==0)?SBUSERROR_NODATA:SBUSERROR_NOT))

#define WRITEERR(err) \
(err<0?SBUSERROR_WRITE:((err==0)?SBUSERROR_NODATA:SBUSERROR_NOT))

/**********************************************************
  Function to read from/write to a communications channel
  socket/pipe based implementation
  ********************************************************/

int BusReadSleepLoop ( int fd, char *inbuf, int length )
    {
    int sum, err;

    sum = 0;
    err = 0;
    do
        {
        if ( sum>0 )
            sleep ( 0 );
        err  = read ( fd, inbuf+sum, length-sum );
        if ( err < 0 ) printf ( "Error reading fd \n" );
        sum += err;
        }
    while ( sum<length && ( err>=0 && sum>0 ) );

    return err;
    }

int BusWriteSleepLoop ( int fd, char *outbuf, int length )
    {
    int sum, err;

    sum = 0;
    err = 0;
    do
        {
        if ( sum>0 )
            sleep ( 0 );
        err = write ( fd, outbuf+sum, length-sum );
        if ( err < 0 ) printf ( "ERROR WRITING TO FD \n" );
        sum += err;
        }
    while ( sum<length && ( err>=0 && sum>0 ) );

    return err;
    }

int BusReadCharacter ( int id, unsigned char *c )
    {
    int err;

    err = read ( id, c, 1 );

    return READERR ( err );
    }

int BusWriteCharacter ( int id, unsigned char c )
    {
    int err;

    err = write ( id, &c, 1 );

    return WRITEERR ( err );
    }

int BusReadInteger ( int id, int *i )
    {
    int  err;
    XDR  xdrs;
    char buf[BUFSIZE];

    xdrmem_create ( &xdrs, buf, BUFSIZE, XDR_DECODE );

    err = BusReadSleepLoop ( id, buf, 4 );
    if ( err > 0 ) xdr_int ( &xdrs, i );

    return READERR ( err );
    }

int BusWriteInteger ( int id, int i )
    {
    int  err;
    XDR  xdrs;
    char buf[BUFSIZE];

    xdrmem_create ( &xdrs, buf, BUFSIZE, XDR_ENCODE );
    xdr_int ( &xdrs, &i );

    err = BusWriteSleepLoop ( id, buf, 4 );

    return WRITEERR ( err );
    }

int BusReadnString ( int id, char **buffer, int *length )
    {
    int err;

    err = BusReadInteger ( id, length );
    if ( err!=SBUSERROR_NOT )
        return err;

    /* printf("BusReadnString : Length of string = %d \n", *length); */
    if ( *length > 0 )
        {
        *buffer = ( char * ) malloc ( *length );
        err = BusReadSleepLoop ( id, *buffer, *length );

        /* printf("BusReadnString : String read = %s \n", *buffer);  */
        return READERR ( err );
        }

    return SBUSERROR_NOT;

    }

int BusWritenString ( int id, char *buffer, int length )
    {
    int err;

    /* printf("BusWritenString : length = %d fd = %d \n", length, id); */
    err = BusWriteInteger ( id, length );
    if ( err!=SBUSERROR_NOT )
        {
        printf ( " BusWritenString : Error writing length %d \n", length );
        return err;
        }

    if ( length > 0 )
        {
        /*printf("BusWritenString: Length = %d String = %s \n", length, buffer); */
        err = BusWriteSleepLoop ( id, buffer, length );
        return WRITEERR ( err );
        }
    return SBUSERROR_NOT;
    }

int BusReadASCIIZ ( int id, char **buffer )
    {
    int err;
    int length;
    char *inbuffer;

    *buffer = NULL;

    err = BusReadnString ( id, &inbuffer, &length );
    if ( err!=SBUSERROR_NOT )
        return err;

    *buffer = ( char * ) malloc ( length + 1 );
    if ( *buffer == NULL )
        return SBUSERROR_NOMEMORY;
    strncpy ( *buffer, inbuffer, length );

    free ( inbuffer );

    ( *buffer ) [length] = '\0';

    return SBUSERROR_NOT;
    }

int BusWriteASCIIZ ( int id, char *string )
    {
    int err, length;

    length = strlen ( string );

    err = BusWritenString ( id, string, length );
    return err;
    }

int BusReadFloat ( int id, float *f )
    {
    /**********************************************************
      fill in using library for ASCII trans of float from vouk
      *********************************************************/
    return SBUSERROR_READ;
    }

int BusWriteFloat ( int id, float f )
    {
    return SBUSERROR_WRITE;
    }

char *BusGetMyUserid()
    {
    char *id;

    id = getenv ( "USER" );
    if ( id == NULL )
        return ( cuserid ( NULL ) );
    else
        return id;
    }
