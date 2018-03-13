/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: File.c 83 2018-03-12 19:24:33Z coats $
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
 ******************************************************************************
 * PURPOSE: File.c - Define functions for reading and writing to
 *          ASCII and portable binary (XDR) files, pipes and sockets.
 * NOTES:
 * HISTORY: 12/1995, Todd Plessel, EPA/MMTSI, Created.
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include <stdio.h>      /* For FILE, fopen(), fdopen(), vfprintf(), *getc(). */
#include <stddef.h>     /* For typedef size_t.                               */
#include <sys/types.h>  /* For struct stat.                                  */
#include <sys/stat.h>   /* For struct stat.                                  */
#include <string.h>     /* For strncpy(), memset().                          */
#include <stdarg.h>     /* For va_list, va_start, va_end.                    */
#include <unistd.h>     /* For close().                                      */
#include <sys/socket.h> /* For socket(), connect(), getsockopt().            */
#include <netinet/in.h> /* For struct sockaddr, struct sockaddr_in, htons(). */
#include <netdb.h>      /* For gethostbyname().                              */
#include <endian.h>     /* For __BYTE_ORDER, __LITTLE_ENDIAN                 */
#include <rpc/rpc.h>    /* For XDR.                                          */

/* HACK around HP-UX BUG: rpc.h is not protected against multiple inclusions!*/
/**********************************************************************************
#ifndef __RPC_RPC_H__
#include <rpc/rpc.h>
#define __RPC_RPC_H__
#endif
***************************************************************************/

/* HACK: work-around CRAY / SUN BUG: popen() not defined in stdio.h. */
#if _CRAY || __sun
extern FILE* popen ( const char* command, const char* mode );
extern FILE* fdopen ( int fdesc, const char* mode );
#endif /* _CRAY || __sun */

#include "Assertions.h" /* For macros PRE(), POST(), AND*().                 */
#include "Error.h"      /* For error().                                      */
#include "Memory.h"     /* For NEW(), FREE().                                */
#include "File.h"       /* For public functions.                             */

/*================================= MACROS ==================================*/

/* For read/writeShorts*() optimization: */

#define IS_LITTLE_ENDIAN (__BYTE_ORDER == __LITTLE_ENDIAN)

/********   NON-PORTABLE IDIOCY -- CJC  ****************************************
#if (defined(__alpha)||defined(__i386__)||defined(__i486__)||defined(__i586__))
#define IS_LITTLE_ENDIAN 1
#else
#define IS_LITTLE_ENDIAN 0
#endif
*****************************************************************************/

/* For read/write other To/From long: */

#define SUB_WORD(p,type) ( IS_LITTLE_ENDIAN ? (const char*) (p) \
 : ( (const char*) (p) ) + \
   ( ( sizeof (*p) / sizeof (type) - 1 ) ) * sizeof (type) / sizeof (char) )

/*================================== TYPES ==================================*/

enum { FILE_STREAM, PIPE_STREAM, SOCKET_STREAM }; /* File->type. */

struct File_
    {
    char* name;       /* Pathed file or command or host name.              */
    char* mode;       /* I/O mode: "r", "w", "a", "r+", etc.               */
    int   type;       /* File, pipe or socket.                             */
    int   port;       /* Port number, if socket.                           */
    int   bufferSize; /* Read buffer size (in bytes), if socket.           */
    int   xdrMode;    /* XDR_DECODE when reading, XDR_ENCODE when writing. */
    FILE* file;       /* Stream file structure.                            */
    XDR   xdr;        /* XDR buffer structure for encoding/decoding.       */
    };

static const char SVN_ID[] = "$Id: File.c 83 2018-03-12 19:24:33Z coats $";

/*========================== FORWARD DECLARATIONS ===========================*/

static void zeroFile ( File* file );

static const char* typeName ( const File* file );

static int isReadMode ( const File* file );

static int isWriteMode ( const File* file );

static void ensureReadMode ( File* file );

static void ensureWriteMode ( File* file );

static int requiresBuffering ( const File* file, size_t bytes );

static int readArrayBuffered ( File* file, size_t count, size_t sizeOfItem,
                               const xdrproc_t elproc, void* array );

/*=========================== PUBLIC FUNCTIONS ==============================*/


/******************************************************************************
 * PURPOSE: isValidFile - Checks if a File structure is valid.
 * INPUTS:  const File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:
 *****************************************************************************/

int isValidFile ( const File* file )
    {
    return AND12 ( file, file->name, file->name[ 0 ], file->mode,
                   IN4 ( file->mode[ 0 ], 'r', 'w', 'a' ),
                   IN4 ( file->mode[ 1 ], '\0', 'b', '+' ),
                   IMPLIES ( file->mode[ 1 ],
                             AND3 ( IN4 ( file->mode[ 2 ], '\0', 'b', '+' ),
                                    file->mode[ 2 ] != file->mode[ 1 ],
                                    IMPLIES ( file->mode[ 2 ] != '\0',
                                            file->mode[ 3 ] == '\0' ) ) ),
                   IN4 ( file->type, FILE_STREAM, PIPE_STREAM, SOCKET_STREAM ),
                   IMPLIES ( file->type == SOCKET_STREAM,
                             AND2 ( file->port > 0, file->bufferSize > 0 ) ),
                   IMPLIES ( file->type != SOCKET_STREAM,
                             AND2 ( file->port == 0, file->bufferSize == 0 ) ),
                   IN3 ( file->xdrMode, XDR_ENCODE, XDR_DECODE ),
                   file->file );
    }


/******************************************************************************
 * PURPOSE: isReadableFile - Checks if a File's mode permits reading.
 * INPUTS:  const File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:
 *****************************************************************************/

int isReadableFile ( const File* file )
    {
    return AND2 ( isValidFile ( file ), file->mode[ 0 ] == 'r' );
    }


/******************************************************************************
 * PURPOSE: isWritableFile - Checks if a File's mode permits writing.
 * INPUTS:  const File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:
 *****************************************************************************/

int isWritableFile ( const File* file )
    {
    return AND2 ( isValidFile ( file ),
                  OR2 ( IN3 ( file->mode[ 0 ], 'w', 'a' ),
                        file->mode[ 1 ] == '+' ) );
    }


/******************************************************************************
 * PURPOSE: isSeekableFile - Checks if a File's type permits seeking.
 * INPUTS:  const File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:
 *****************************************************************************/

int isSeekableFile ( const File* file )
    {
    return AND5 ( isValidFile ( file ),
                  file->type == FILE_STREAM,
                  strcmp ( file->name, "-stdin"  ),
                  strcmp ( file->name, "-stdout" ),
                  strcmp ( file->name, "-stderr" ) );
    }


/******************************************************************************
 * PURPOSE: nameOfFile - Get the name (read-only) of a file.
 * INPUTS:  const File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: const char*       Pointer to the file name.
 * NOTES:   Do not attempt to deallocate the returned pointer.
 *****************************************************************************/

const char* nameOfFile ( const File* file )
    {
    PRE ( isValidFile ( file ) );

    return file->name;
    }


/******************************************************************************
 * PURPOSE: sizeOfFile - Get the size (in bytes) of an existing file.
 * INPUTS:  const File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: size_t       Size, in bytes, of the file.
 * NOTES:
 *****************************************************************************/

size_t sizeOfFile ( const File* file )
    {
    PRE ( isSeekableFile ( file ) );

    size_t size = 0;
    const int fileDescriptor = fileno ( file->file );

    if ( fileDescriptor < 0 )
        {
        error ( "Can't get file descriptor number for file '%s'.", file->name );
        }
    else
        {
        struct stat statBuffer;

        if ( fstat ( fileDescriptor, &statBuffer ) != 0 )
            {
            error ( "Can't get size of file '%s'.", file->name );
            }
        else
            {
            size = ( size_t ) statBuffer.st_size;
            }
        }

    return size;
    }


/******************************************************************************
 * PURPOSE: fileOfFile - Get the underlying FILE pointer.
 * INPUTS:  const File* file  The file to examine.
 * OUTPUTS: None
 * RETURNS: FILE*       Underlying FILE pointer.
 * NOTES:   HACK! This violates encapsulation but is needed to interface with
 *          external software. This file pointer should not be closed.
 *****************************************************************************/

FILE* fileOfFile ( const File* file )
    {
    PRE ( isValidFile ( file ) );

    return file->file;
    }


/******************************************************************************
 * PURPOSE: openFile - Open a file for reading and/or writing.
 * INPUTS:  const char* fileName  The name of the file to open or one of:
 *                                "-stdin", "-stdout", "-stderr" or
 *          const char* mode      "r", "w", "a", "r+", etc.
 * OUTPUTS: None
 * RETURNS: File*                 Initialized file structure, or 0 if failed.
 * NOTES:   If unsuccessful then error() is called and 0 is returned.
 *****************************************************************************/

File* openFile ( const char* fileName, const char* mode )
    {
    File* file = 0;

    PRE5 ( fileName, mode, IN4 ( *mode, 'r', 'w', 'a' ),
           IMPLIES ( strcmp ( fileName, "-stdin"  ) == 0, strcmp ( mode, "r" ) == 0 ),
           IMPLIES ( OR2 ( strcmp ( fileName, "-stdout" ) == 0,
                           strcmp ( fileName, "-stderr" ) == 0 ),
                     AND2 ( *mode != 'r', mode[ 1 ] == '\0' ) ) );

    file = NEW ( File, 1 );

    if ( file )
        {
        zeroFile ( file );

        file->name = NEW ( char, strlen ( fileName ) + 1 );
        file->mode = NEW ( char, strlen ( mode     ) + 1 );

        if ( AND2 ( file->name, file->mode ) )
            {
            if      ( strcmp ( fileName, "-stdin"  ) == 0 ) file->file = stdin;
            else if ( strcmp ( fileName, "-stdout" ) == 0 ) file->file = stdout;
            else if ( strcmp ( fileName, "-stderr" ) == 0 ) file->file = stderr;
            else file->file = fopen ( fileName, mode );

            }

        if ( file->file )
            {
            strcpy ( file->name, fileName );
            strcpy ( file->mode, mode );
            file->type = FILE_STREAM;
            file->xdrMode = *mode == 'r' ? XDR_DECODE : XDR_ENCODE;
            xdrstdio_create ( &file->xdr, file->file, file->xdrMode );
            }
        else
            {
            error ( "Can't open file '%s' for %s.", fileName,
                    *mode == 'r' ? "reading" : "writing" );

            FREE ( file->name );
            FREE ( file->mode );
            zeroFile ( file );
            FREE ( file );
            }
        }

    POST ( IMPLIES ( file, isValidFile ( file ) ) );

    return file;
    }


/******************************************************************************
 * PURPOSE: openPipe - Open a pipe for reading or writing.
 * INPUTS:  const char* command   The command used to open a pipe.
 *          const char* mode      "r" or "w".
 * OUTPUTS: None
 * RETURNS: File*                 Initialized file structure, or 0 if failed.
 * NOTES:   If unsuccessful then error() is called and 0 is returned.
 *****************************************************************************/

File* openPipe ( const char* command, const char* mode )
    {
    File* file = 0;

    PRE5 ( command, *command, mode, IN3 ( *mode, 'r', 'w' ), mode[ 1 ] == '\0' );

    file = NEW ( File, 1 );

    if ( file )
        {
        zeroFile ( file );

        file->name = NEW ( char, strlen ( command ) + 1 );
        file->mode = NEW ( char, strlen ( mode    ) + 1 );

        if ( AND2 ( file->name, file->mode ) )
            {
            /*
             * Must flush stdout to sync subsequent output in cases where
             * pipe command outputs to stdout. E.g., "cat", "echo", etc.
             */

            fflush ( stdout );

            file->file = popen ( command, mode );
            }

        if ( file->file )
            {
            strcpy ( file->name, command );
            strcpy ( file->mode, mode );
            file->type = PIPE_STREAM;
            file->xdrMode = *mode == 'r' ? XDR_DECODE : XDR_ENCODE;
            xdrstdio_create ( &file->xdr, file->file, file->xdrMode );
            }
        else
            {
            error ( "Can't open pipe with command '%s' for %s.", command,
                    *mode == 'r' ? "reading" : "writing" );

            FREE ( file->name );
            FREE ( file->mode );
            zeroFile ( file );
            FREE ( file );
            }
        }

    POST ( IMPLIES ( file, isValidFile ( file ) ) );

    return file;
    }

/******************************************************************************
 * PURPOSE: closeFile_ - Close a file.
 * INPUTS:  File* file  The file to close.
 * OUTPUTS: File* file  Destroyed, zeroed and deallocated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *          Use closeFile() macro instead - it zeros-out the argument to avoid
 *          dangling references to deallocated memory.
 *****************************************************************************/

int closeFile_ ( File* file )
    {
    PRE ( isValidFile ( file ) );

    int ok = 0;

    xdr_destroy ( &file->xdr ); /* Free XDR buffer. */

    if ( file->type == PIPE_STREAM )
        {
        const int status = pclose ( file->file );
        ok = status != -1;
        }
    else if ( AND3 ( strcmp ( file->name, "-stdin"  ),
                     strcmp ( file->name, "-stdout" ),
                     strcmp ( file->name, "-stderr" ) ) )
        {
        ok = fclose ( file->file ) == 0;
        }
    else ok = 1;

    if ( ! ok ) error ( "Can't close %s '%s'.", typeName ( file ), file->name );

    FREE ( file->name );
    FREE ( file->mode );
    zeroFile ( file );
    FREE ( file );

    POST ( ! isValidFile ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: seekFile - Seek to a location in a file.
 * INPUTS:  File* file   The file to seek on.
 *          long offset  The offset to seek by.
 *          int whence   SEEK_SET, SEEK_CUR, SEEK_END.
 * OUTPUTS: File* file   Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int seekFile ( File* file, long offset, int whence )
    {
    PRE2 ( isSeekableFile ( file ), IN4 ( whence, SEEK_SET, SEEK_CUR, SEEK_END ) );

    const int ok = fseek ( file->file, offset, whence ) == 0;

    if ( ! ok )
        {
        error ( "Can't seek to byte %d%s in file '%s'.", offset,
                whence == SEEK_CUR ? " from current location" :
                whence == SEEK_END  ? " from end" : "",
                file->name );
        }

    POST ( isValidFile ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: offsetInFile - Get the current offset from the beginning of file.
 * INPUTS:  File* file   The file to check.
 * OUTPUTS: None
 * RETURNS: long the current offset from the beginning of the file.
 * NOTES:
 *****************************************************************************/

long offsetInFile ( File* file )
    {
    PRE ( isSeekableFile ( file ) );

    const long offset = ftell ( file->file ); /* Why isn't ftell() const? */

    POST ( isValidFile ( file ) );

    return offset;
    }


/******************************************************************************
 * PURPOSE: isEndOfFile - Checks if a File pointer has reached the end.
 * INPUTS:  File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: int 1 if EOF, else 0.
 * NOTES:   Will block if used on pipes or sockets.
 *****************************************************************************/

int isEndOfFile ( File* file )
    {
    PRE ( isReadableFile ( file ) );

    /*
     * Implementation note:
     *   feof() is not used since it does not work with fseek().
     *   ftell() (compared with a cached file size) is five times slower than
     *   the fgetc(); ungetc() approach used here!
     */

    const int ch = fgetc ( file->file );
    ungetc ( ch, file->file );

    return ch == EOF;
    }


/******************************************************************************
 * PURPOSE: isNonBlockingFile - Checks if a File will block if read from.
 * INPUTS:  File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: int 1 if non-blocking, else 0.
 * NOTES:
 *****************************************************************************/

int isNonBlockingFile ( const File* file )
    {
    PRE ( isValidFile ( file ) );

    return file->type == FILE_STREAM;
    }


/******************************************************************************
 * PURPOSE: readByte - Read a byte from a file.
 * INPUTS:  File* file  The file to read from.
 * OUTPUTS: void* x     The byte read.
 *          File* file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readByte ( File* file, void* x )
    {
    PRE2 ( isReadableFile ( file ), x );

    int ok = 0;

    ensureReadMode ( file );

    ok = fread ( x, 1, 1, file->file ) == 1;

    if ( ! ok )
        error ( "Can't read a byte from %s '%s'.", typeName ( file ), file->name );

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readChar - Read a char from a file.
 * INPUTS:  File* file  The file to read from.
 * OUTPUTS: char* x     The char read.
 *          File* file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readChar ( File* file, char* x )
    {
    PRE2 ( isReadableFile ( file ), x );

    int ok = 0;

    ensureReadMode ( file );

    ok = xdr_char ( &file->xdr, x );

    if ( ! ok )
        error ( "Can't read a char from %s '%s'.", typeName ( file ), file->name );

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readShort - Read an XDR short from a file.
 * INPUTS:  File* file   The file to read from.
 * OUTPUTS: short* x     The short read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readShort ( File* file, short* x )
    {
    PRE2 ( isReadableFile ( file ), x );

    int ok = 0;

    ensureReadMode ( file );

    ok = xdr_short ( &file->xdr, x );

    if ( ! ok )
        error ( "Can't read a short from %s '%s'.", typeName ( file ), file->name );

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readShort2 - Read a big-endian two-byte short from a file.
 * INPUTS:  File* file   The file to read from.
 * OUTPUTS: short* x     The short read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readShort2 ( File* file, short* x )
    {
    PRE2 ( isReadableFile ( file ), x );

    int ok = 0;
    unsigned char lowByte;
    signed char highByte;

    ensureReadMode ( file );

    ok = AND2 ( fread ( &highByte, 1, 1, file->file ) == 1,
                fread ( &lowByte,  1, 1, file->file ) == 1 );

    if ( ok )
        {
        DEBUG ( printf ( "highByte = %x, lowByte = %x\n", highByte, lowByte ); )

        *x = ( highByte << 8 ) + lowByte;

        DEBUG ( printf ( "value = %d\n", *x ); )
        }

    if ( ! ok )
        error ( "Can't read a short from %s '%s'.", typeName ( file ), file->name );

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readInt - Read an integer from a file.
 * INPUTS:  File* file   The file to read from.
 * OUTPUTS: int* x       The integer read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readInt ( File* file, int* x )
    {
    PRE2 ( isReadableFile ( file ), x );

    int ok = 0;

    ensureReadMode ( file );

    ok = xdr_int ( &file->xdr, x );

    if ( ! ok )
        error ( "Can't read an integer from %s '%s'.", typeName ( file ), file->name );

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readLong - Read a long from a file.
 * INPUTS:  File* file   The file to read from.
 * OUTPUTS: long* x      The long read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readLong ( File* file, long* x )
    {
    PRE2 ( isReadableFile ( file ), x );

    int ok = 0;

    ensureReadMode ( file );

    ok = xdr_long ( &file->xdr, x );

    if ( ! ok )
        error ( "Can't read a long from %s '%s'.", typeName ( file ), file->name );

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readShortToLong - Read a short from a file into a long.
 * INPUTS:  File* file   The file to read from.
 * OUTPUTS: long* x      The short read and expanded to a long.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readShortToLong ( File* file, long* x )
    {
    PRE2 ( isReadableFile ( file ), x );

    short s  = 0;
    int   ok = readShort ( file, &s );
    *x = s;

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readShort2ToLong - Read a big-endian two-byte short from a file
 *          into a long.
 * INPUTS:  File* file   The file to read from.
 * OUTPUTS: long* x      The short read and expanded to a long.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readShort2ToLong ( File* file, long* x )
    {
    PRE2 ( isReadableFile ( file ), x );

    short s  = 0;
    int   ok = readShort2 ( file, &s );
    *x = s;

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readIntToLong - Read an int from a file into a long.
 * INPUTS:  File* file   The file to read from.
 * OUTPUTS: long* x      The int read and expanded to a long.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readIntToLong ( File* file, long* x )
    {
    PRE2 ( isReadableFile ( file ), x );

    int i  = 0;
    int ok = readInt ( file, &i );
    *x = i;

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readFloat - Read a float from a file.
 * INPUTS:  File*  file  The file to read from.
 * OUTPUTS: float* x     The float read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readFloat ( File* file, float* x )
    {
    PRE2 ( isReadableFile ( file ), x );

    int ok = 0;

    ensureReadMode ( file );

    ok = xdr_float ( &file->xdr, x );

    if ( ! ok )
        error ( "Can't read a float from %s '%s'.", typeName ( file ), file->name );

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readDouble - Read a double from a file.
 * INPUTS:  File*   file  The file to read from.
 * OUTPUTS: double* x     The double read.
 *          File*   file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readDouble ( File* file, double* x )
    {
    PRE2 ( isReadableFile ( file ), x );

    int ok = 0;

    ensureReadMode ( file );

    ok = xdr_double ( &file->xdr, x );

    if ( ! ok )
        error ( "Can't read a double from %s '%s'.", typeName ( file ), file->name );

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readBytes - Read a block of bytes from a file.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The number of bytes to read.
 * OUTPUTS: void*  a     The block of bytes read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readBytes ( File* file, void* a, size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    const int ok = readSomeBytes ( file, a, n ) == n;

    if ( ! ok )
        {
        error ( "Can't read %u bytes from %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readSomeBytes - Read a block of up to n bytes from a file or pipe.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The maximum number of bytes to read.
 * OUTPUTS: void*  a     The block of bytes read.
 *          File*  file  Updated file structure.
 * RETURNS: size_t       The number of bytes actually read and stored in 'a'.
 * NOTES:   This is used when the number of bytes expected is unknown, e.g.,
 *          as with pipes, and may be less than a given maximum without
 *          indicating an error. A return value of 0 may indicate an
 *          end-of-file condition for a file or pipe. If used with a socket,
 *          blocking will occur until exactly 'n' bytes are read.
 *****************************************************************************/

size_t readSomeBytes ( File* file, void* a, size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    size_t bytesRead = 0; /* Number of bytes actually read. */

    ensureReadMode ( file );

    if ( ! requiresBuffering ( file, n ) )
        {
        bytesRead = fread ( a, 1, n, file->file );
        }
    else
        {
        unsigned char* const c = ( unsigned char* ) a; /* For pointer arithmetic. */

        int done = 0;

        while ( AND2 ( ! done, bytesRead != n ) )
            {
            const size_t bytesRemaining = n - bytesRead;

            const size_t bytesToRead    = bytesRemaining < file->bufferSize ?
                                          bytesRemaining
                                          : file->bufferSize;

            const size_t charsAlreadyRead = bytesRead / sizeof ( char );

            const size_t newBytesRead = fread ( c + charsAlreadyRead, 1,
                                                bytesToRead, file->file );

            done = newBytesRead == 0;
            bytesRead += newBytesRead;
            }
        }

    POST ( isReadMode ( file ) );

    return bytesRead;
    }


/******************************************************************************
 * PURPOSE: readChars - Read an array of chars from a file.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The number of chars to read.
 * OUTPUTS: char*  a     The array of chars read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readChars ( File* file, char* a, size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    int ok = 0;

    ensureReadMode ( file );

    if ( requiresBuffering ( file, n ) )
        {
        ok = readArrayBuffered ( file, ( size_t ) n, sizeof ( char ), xdr_char, a );
        }
    else
        {
        ok = xdr_vector ( &file->xdr, a, n, sizeof ( char ), xdr_char );

        if ( ! ok )
            {
            error ( "Can't read %u chars from %s '%s'.",
                    n, typeName ( file ), file->name );
            }
        }

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readShorts - Read an array of XDR shorts from a file.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The number of shorts to read.
 * OUTPUTS: short    a[] The array of shorts read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readShorts ( File* file, short a[], size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    int ok = 0;

    ensureReadMode ( file );

    if ( requiresBuffering ( file, n ) )
        {
        ok = readArrayBuffered ( file, n, sizeof ( short ), xdr_short, a );
        }
    else
        {
        ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( short ), xdr_short );

        if ( ! ok )
            {
            error ( "Can't read %u shorts from %s '%s'.",
                    n, typeName ( file ), file->name );
            }
        }

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readShorts2 - Read an array of big-endian two-byte shorts from a
 *          file.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The number of shorts to read.
 * OUTPUTS: short    a[] The array of shorts read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readShorts2 ( File* file, short a[], size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    int ok = 0;

    ensureReadMode ( file );

    if ( AND2 ( sizeof ( short ) == 2, ! IS_LITTLE_ENDIAN ) ) /* Optimize: */
        {
        if ( requiresBuffering ( file, n ) )
            {
            ok = readArrayBuffered ( file, n, sizeof ( short ), 0, a );
            }
        else
            {
            DEBUG ( printf ( "calling fread\n" ); )

            ok = fread ( a, sizeof ( short ), n, file->file ) == n;

            if ( ! ok )
                {
                error ( "Can't read %u shorts from %s '%s'.",
                        n, typeName ( file ), file->name );
                }
            }
        }
    else /* Must loop over each byte pair: */
        {
        size_t i;

        for ( i = 0; i < n; ++i )
            {
            unsigned char lowByte;
            signed char highByte;

            ok = AND2 ( fread ( &highByte, 1, 1, file->file ) == 1,
                        fread ( &lowByte,  1, 1, file->file ) == 1 );

            if ( ok )
                {
                DEBUG ( printf ( "highByte = %x, lowByte = %x\n", highByte, lowByte ); )

                a[ i ] = ( highByte << 8 ) + lowByte;

                DEBUG ( printf ( "value = %d\n", a[ i ] ); )
                }
            }

        if ( ! ok )
            {
            error ( "Can't read %u shorts from %s '%s'.",
                    n, typeName ( file ), file->name );
            }
        }

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readInts - Read an array of integers from a file.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The number of integers to read.
 * OUTPUTS: int    a[]   The array of integers read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readInts ( File* file, int a[], size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    int ok = 0;

    ensureReadMode ( file );

    if ( requiresBuffering ( file, n ) )
        {
        ok = readArrayBuffered ( file, n, sizeof ( int ), xdr_int, a );
        }
    else
        {
        ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( int ), xdr_int );

        if ( ! ok )
            {
            error ( "Can't read %u ints from %s '%s'.",
                    n, typeName ( file ), file->name );
            }
        }

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readLongs - Read an array of longs from a file.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The number of integers to read.
 * OUTPUTS: long   a[]   The array of longs read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readLongs ( File* file, long a[], size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    int ok = 0;

    ensureReadMode ( file );

    if ( requiresBuffering ( file, n ) )
        {
        ok = readArrayBuffered ( file, n, sizeof ( long ), xdr_long, a );
        }
    else
        {
        ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( long ), xdr_long );

        if ( ! ok )
            {
            error ( "Can't read %u longs from %s '%s'.",
                    n, typeName ( file ), file->name );
            }
        }

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readShortsToLongs - Read an array of shorts from a file and expand
 *          them to an array of longs.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The number of integers to read.
 * OUTPUTS: long   a[]   The array of shorts read and expanded to longs.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readShortsToLongs ( File* file, long a[], size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    short* sa = ( short* ) a; /* First read from file into a - stored as shorts. */
    const int ok = readShorts ( file, sa, n );

    if ( ok ) /* Expand shorts into longs: */
        {
        long*        dst = a  + n;
        const short* src = sa + n;

        while ( n )
            {
            *--dst = *--src;
            --n;
            }
        }

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readShorts2ToLongs - Read an array of big-endian two-byte shorts
 *          from a file and expand them to an array of longs.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The number of integers to read.
 * OUTPUTS: long   a[]   The array of shorts read and expanded to longs.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readShorts2ToLongs ( File* file, long a[], size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    short* sa = ( short* ) a; /* First read from file into a, stored as shorts. */
    const int ok = readShorts2 ( file, sa, n );

    if ( ok ) /* Expand shorts into longs: */
        {
        long*        dst = a  + n;
        const short* src = sa + n;

        while ( n )
            {
            *--dst = *--src;
            --n;
            }
        }

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readIntsToLongs - Read an array of ints from a file and expand them
 *          to an array of longs.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The number of integers to read.
 * OUTPUTS: long   a[]   The array of ints read and expanded to longs.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readIntsToLongs ( File* file, long a[], size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    int* ia = ( int* ) a; /* First read from file into a - stored as ints. */
    const int ok = readInts ( file, ia, n );

    if ( ok ) /* Expand ints into longs: */
        {
        long*      dst = a  + n;
        const int* src = ia + n;

        while ( n )
            {
            *--dst = *--src;
            --n;
            }
        }

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readFloats - Read an array of floats from a file.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The number of integers to read.
 * OUTPUTS: float  a[]   The array of floats read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readFloats ( File* file, float a[], size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    int ok = 0;

    ensureReadMode ( file );

    if ( requiresBuffering ( file, n ) )
        {
        ok = readArrayBuffered ( file, n, sizeof ( float ), xdr_float, a );
        }
    else
        {
        ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( float ), xdr_float );

        if ( ! ok )
            {
            error ( "Can't read %u floats from %s '%s'.",
                    n, typeName ( file ), file->name );
            }
        }

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readDoubles - Read an array of doubles from a file.
 * INPUTS:  File*   file  The file to read from.
 *          size_t  n     The number of integers to read.
 * OUTPUTS: double  a[]   The array of doubles read.
 *          File*   file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readDoubles ( File* file, double a[], size_t n )
    {
    PRE3 ( isReadableFile ( file ), a, n );

    int ok = 0;

    ensureReadMode ( file );

    if ( requiresBuffering ( file, n ) )
        {
        ok = readArrayBuffered ( file, n, sizeof ( double ), xdr_double, a );
        }
    else
        {
        ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( double ), xdr_double );

        if ( ! ok )
            {
            error ( "Can't read %u doubles from %s '%s'.",
                    n, typeName ( file ), file->name );
            }
        }

    POST ( isReadMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readString - Read a one-line string from a file.
 * INPUTS:  File*  file  The file to read from.
 *          size_t n     The number of characters to read.
 * OUTPUTS: char   s[]   The string read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *          Reading stops when n - 1 characters are read or a newline character
 *          is read (and stored in s) - like fgets().
 *****************************************************************************/

int readString ( File* file, char s[], size_t n )
    {
    PRE3 ( isReadableFile ( file ), s, n );

    int ok = 0;

    ensureReadMode ( file );

    if ( ! requiresBuffering ( file, n ) )
        {
        ok = fgets ( s, n, file->file ) != 0;

        if ( ! ok )
            {
            error ( "Can't read %u-length string from %s '%s'.",
                    n, typeName ( file ), file->name );
            }
        }
    else
        {
        const size_t maxCharsReadable = file->bufferSize / sizeof ( char );
        size_t charsRead = 0; /* Number of chars already read so far. */
        int done = 0;

        *s = '\0';

        ok = 1;

        while ( AND2 ( ok, ! done ) )
            {
            const size_t maxCharsRemaining = n - charsRead;

            const size_t maxCharsToRead    = maxCharsRemaining < maxCharsReadable ?
                                             maxCharsRemaining
                                             : maxCharsReadable;

            char* const partialString = s + charsRead;

            ok = fgets ( partialString, maxCharsToRead, file->file ) != 0;

            if ( ! ok )
                {
                if ( *s == '\0' ) /* No chars were ever read. */
                    {
                    error ( "Can't read up to %u chars of a partial string from the "
                            "socket '%s'.\n"
                            "(Only read a total of %u chars out of up to %u expected.)",
                            maxCharsToRead, file->name, charsRead, n );
                    }
                else done = ok = 1;  /* False alarm. Return short string w/o '\n'. */
                }
            else
                {
                const size_t partialStringLength = strlen ( partialString );

                if ( partialStringLength ) charsRead += partialStringLength;
                else done = 1;
                }
            }
        }

    if ( ! ok ) *s = '\0';

    POST2 ( isReadMode ( file ), strlen ( s ) <= n );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeByte - Write a byte to a file.
 * INPUTS:  File* file       The file to write to.
 *          unsigned char x  The byte to write.
 * OUTPUTS: File* file       Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeByte ( File* file, unsigned char x )
    {
    PRE ( isWritableFile ( file ) );

    int ok = 0;

    ensureWriteMode ( file );

    ok = fwrite ( &x, 1, 1, file->file ) == 1;

    if ( ! ok )
        {
        error ( "Can't write a byte to %s '%s'.", typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeChar - Write a char to a file.
 * INPUTS:  File* file  The file to write to.
 *          char  x     The char to write.
 * OUTPUTS: File* file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 ******************************************************************************/

int writeChar ( File* file, char x )
    {
    PRE ( isWritableFile ( file ) );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_char ( &file->xdr, &x );

    if ( ! ok )
        {
        error ( "Can't write a char to %s '%s'.", typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeShort - Write a short to a file.
 * INPUTS:  File* file  The file to write to.
 *          short x     The short to write.
 * OUTPUTS: File* file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeShort ( File* file, short x )
    {
    PRE ( isWritableFile ( file ) );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_short ( &file->xdr, &x );

    if ( ! ok )
        {
        error ( "Can't write a short to %s '%s'.", typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeShort2 - Write a short as big-endian two-bytes to a file.
 * INPUTS:  File* file  The file to write to.
 *          short x     The short to write.
 * OUTPUTS: File* file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeShort2 ( File* file, short x )
    {
    PRE ( isWritableFile ( file ) );

    int ok = 0;
    const unsigned char lowByte  = x & 0xff;
    const unsigned char highByte = ( x >> 8 ) & 0xff;

    ensureWriteMode ( file );

    DEBUG ( printf ( "value = %d\n", x ); )
    DEBUG ( printf ( "highByte = %x, lowByte = %x\n", highByte, lowByte ); )

    ok = AND2 ( fwrite ( &highByte, 1, 1, file->file ) == 1,
                fwrite ( &lowByte,  1, 1, file->file ) == 1 );

    if ( ! ok )
        {
        error ( "Can't write a short to %s '%s'.", typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeInt - Write an integer to a file.
 * INPUTS:  File* file  The file to write to.
 *          int   x     The integer to write.
 * OUTPUTS: File* file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeInt ( File* file, int x )
    {
    PRE ( isWritableFile ( file ) );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_int ( &file->xdr, &x );

    if ( ! ok )
        {
        error ( "Can't write an int to %s '%s'.", typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeLong - Write a long to a file.
 * INPUTS:  File* file  The file to write to.
 *          long  x     The long to write.
 * OUTPUTS: File* file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeLong ( File* file, long x )
    {
    PRE ( isWritableFile ( file ) );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_long ( &file->xdr, &x );

    if ( ! ok )
        {
        error ( "Can't write a long to %s '%s'.", typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeFloat - Write a float to a file.
 * INPUTS:  File*  file  The file to write to.
 *          float  x     The float to write.
 * OUTPUTS: File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeFloat ( File* file, float x )
    {
    PRE ( isWritableFile ( file ) );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_float ( &file->xdr, &x );

    if ( ! ok )
        {
        error ( "Can't write a float to %s '%s'.", typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeDouble - Write a double to a file.
 * INPUTS:  File*  file  The file to write to.
 *          double x     The double to write.
 * OUTPUTS: File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeDouble ( File* file, double x )
    {
    PRE ( isWritableFile ( file ) );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_double ( &file->xdr, &x );

    if ( ! ok )
        {
        error ( "Can't write a double to %s '%s'.", typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeBytes - Write a block of bytes to a file.
 * INPUTS:  File*     file  The file to write to.
 *          const void* a   The block of bytes to write.
 *          size_t    n     The number of bytes to write.
 * OUTPUTS: File*     file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeBytes ( File* file, const void* a, size_t n )
    {
    PRE3 ( isWritableFile ( file ), a, n );

    int ok = 0;

    ensureWriteMode ( file );

    ok = fwrite ( a, 1, n, file->file ) == n;

    if ( ! ok )
        {
        error ( "Can't write %u bytes to %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeChars - Write an array of chars to a file.
 * INPUTS:  File*       file  The file to write to.
 *          const char* a     The array  of chars to write.
 *          size_t      n     The number of chars to write.
 * OUTPUTS: File*       file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeChars ( File* file, const char* a, size_t n )
    {
    PRE3 ( isWritableFile ( file ), a, n );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( char ), xdr_char );

    if ( ! ok )
        {
        error ( "Can't write %u chars to %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeShorts - Write an array of shorts in XDR-format to a file.
 * INPUTS:  File*     file   The file to write to.
 *          const short a[]  The array of shorts to write.
 *          size_t    n      The number of shorts to write.
 * OUTPUTS: File*     file   Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeShorts ( File* file, const short a[], size_t n )
    {
    PRE3 ( isWritableFile ( file ), a, n );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( short ), xdr_short );

    if ( ! ok )
        {
        error ( "Can't write %u shorts to %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeShorts2 - Write an array of shorts as big-endian two-bytes to
 *          a file.
 * INPUTS:  File*     file   The file to write to.
 *          const short a[]  The array of shorts to write.
 *          size_t    n      The number of shorts to write.
 * OUTPUTS: File*     file   Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeShorts2 ( File* file, const short a[], size_t n )
    {
    PRE3 ( isWritableFile ( file ), a, n );

    int ok = 0;

    ensureWriteMode ( file );

    if ( AND2 ( sizeof ( short ) == 2, ! IS_LITTLE_ENDIAN ) ) /* Optimize: */
        {
        DEBUG ( printf ( "calling fwrite()\n" ); )

        ok = fwrite ( a, sizeof ( short ), n, file->file ) == n;
        }
    else /* Must loop over each byte pair: */
        {
        size_t i;

        for ( i = 0, ok = 1; AND2 ( ok, i < n ); ++i )
            {
            const unsigned char lowByte  = a[ i ] & 0xff;
            const unsigned char highByte = ( a[ i ] >> 8 ) & 0xff;

            DEBUG ( printf ( "value = %d\n", a[ i ] ); )
            DEBUG ( printf ( "highByte = %x, lowByte = %x\n", highByte, lowByte ); )

            ok = AND2 ( fwrite ( &highByte, 1, 1, file->file ) == 1,
                        fwrite ( &lowByte,  1, 1, file->file ) == 1 );
            }
        }

    if ( ! ok )
        {
        error ( "Can't write %u shorts to %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeInts - Write an array of integers to a file.
 * INPUTS:  File*     file  The file to write to.
 *          const int a[]   The array of integers to write.
 *          size_t    n     The number of integers to write.
 * OUTPUTS: File*     file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeInts ( File* file, const int a[], size_t n )
    {
    PRE3 ( isWritableFile ( file ), a, n );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( int ), xdr_int );

    if ( ! ok )
        {
        error ( "Can't write %u ints to %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeLongs - Write an array of longs to a file.
 * INPUTS:  File*      file  The file to write to.
 *          const long a[]   The array of longs to write.
 *          size_t     n     The number of integers to write.
 * OUTPUTS: File*      file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeLongs ( File* file, const long a[], size_t n )
    {
    PRE3 ( isWritableFile ( file ), a, n );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( long ), xdr_long );

    if ( ! ok )
        {
        error ( "Can't write %u longs to %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeShortsFromLongs - Write an array of shorts to a file
 *          converting on-the-fly the values from an array of longs.
 * INPUTS:  File*      file  The file to write to.
 *          const long a[]   The array of longs to write (as shorts).
 *          size_t     n     The number of integers to write.
 * OUTPUTS: File*      file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeShortsFromLongs ( File* file, const long a[], size_t n )
    {
    PRE3 ( isWritableFile ( file ), a, n );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_vector ( &file->xdr, ( char* ) SUB_WORD ( a, short ), n,
                      sizeof ( long ), xdr_short );

    if ( ! ok )
        {
        error ( "Can't write %u shorts to %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeShorts2FromLongs - Write an array of big-endian two-byte
 *          shorts to a file converting (by copying) the values from an array
 *          of longs.
 * INPUTS:  File*      file  The file to write to.
 *          const long a[]   The array of longs to write (as shorts).
 *          size_t     n     The number of integers to write.
 * OUTPUTS: File*      file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeShorts2FromLongs ( File* file, const long a[], size_t n )
    {
    PRE3 ( isWritableFile ( file ), a, n );

    int    ok = 0;
    short* sa = 0;

    ensureWriteMode ( file );

    /*
     * To avoid modification due to loss of precision (of a const-cast-away
     * approach) we must copy to a compact short array and then write that copy:
     */

    sa = NEW ( short, n );

    if ( sa )
        {
        size_t i;

        for ( i = 0; i < n; ++ i ) sa[ i ] = a[ i ];

        ok = writeShorts2 ( file, sa, n );

        FREE ( sa );
        }
    else
        {
        error ( "Can't allocate temporary buffer to write %u shorts to %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeIntsFromLongs - Write an array of ints to a file converting
 *          on-the-fly the values from an array of longs.
 * INPUTS:  File*      file  The file to write to.
 *          const long a[]   The array of longs to write (as ints).
 *          size_t     n     The number of integers to write.
 * OUTPUTS: File*      file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeIntsFromLongs ( File* file, const long a[], size_t n )
    {
    PRE3 ( isWritableFile ( file ), a, n );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_vector ( &file->xdr, ( char* ) SUB_WORD ( a, int ), n,
                      sizeof ( long ), xdr_int );

    if ( ! ok )
        {
        error ( "Can't write %u ints to %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeFloats - Write an array of floats to a file.
 * INPUTS:  XDR*        xdr  The file to write to.
 *          const float a[]  The array of floats to write.
 *          size_t      n    The number of floats to write.
 * OUTPUTS: XDR*        xdr  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeFloats ( File* file, const float a[], size_t n )
    {
    PRE3 ( isWritableFile ( file ), a, n );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( float ), xdr_float );

    if ( ! ok )
        {
        error ( "Can't write %u floats to %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeDoubles - Write an array of doubles to a file.
 * INPUTS:  XDR*         xdr  The file to write to.
 *          const double a[]  The array of doubles to write.
 *          size_t       n    The number of doubles to write.
 * OUTPUTS: XDR*         xdr  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeDoubles ( File* file, const double a[], size_t n )
    {
    PRE3 ( isWritableFile ( file ), a, n );

    int ok = 0;

    ensureWriteMode ( file );

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( double ), xdr_double );

    if ( ! ok )
        {
        error ( "Can't write %u doubles to %s '%s'.",
                n, typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeString - Write a string to a file.
 * INPUTS:  File*       file     The file to write to.
 *          const char* format   The string to write. May be a printf()-like
 *                               format string like "%d %f\n" followed by args.
 * OUTPUTS: File*       file     Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *          Implemented in terms of vfprintf (man vfprintf).
 *****************************************************************************/

int writeString ( File* file, const char* format, ... )
    {
    PRE2 ( isWritableFile ( file ), format );

    int ok = 0;
    va_list args; /* For stdarg magic. */

    ensureWriteMode ( file );

    va_start ( args, format );                       /* Begin stdarg magic.  */
    ok = vfprintf ( file->file, format, args ) != -1; /* Pass args along.     */
    va_end ( args );                                 /* End of stdarg magic. */

    if ( ! ok )
        {
        error ( "Can't write string to %s '%s'.", typeName ( file ), file->name );
        }

    POST ( isWriteMode ( file ) );

    return ok;
    }


/*=========================== PRIVATE FUNCTIONS =============================*/


/******************************************************************************
 * PURPOSE: zeroFile - Zeros-out a File structure.
 * INPUTS:  None
 * OUTPUTS: File* file  The file to check.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void zeroFile ( File* file )
    {
    PRE ( file );

    memset ( file, 0, sizeof ( File ) );

    POST ( IS_ZERO3 ( file->name, file->mode, file->file ) );
    }


/******************************************************************************
 * PURPOSE: typeName - Name of file type.
 * INPUTS:  const File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: const char*  Name of file type.
 * NOTES:
 *****************************************************************************/

static const char* typeName ( const File* file )
    {
    PRE ( isValidFile ( file ) );

    const char* const result =   file->type == FILE_STREAM   ? "file"
                                 : file->type == PIPE_STREAM   ? "pipe"
                                 : file->type == SOCKET_STREAM ? "socket"
                                 : 0;

    POST2 ( result, *result );

    return result;
    }


/******************************************************************************
 * PURPOSE: isReadMode - Determine if the file is in read mode.
 * INPUTS:  File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: int 1 if xdrMode is XDR_DECODE.
 * NOTES:
 *****************************************************************************/

static int isReadMode ( const File* file )
    {
    PRE ( isReadableFile ( file ) );

    return file->xdrMode == XDR_DECODE;
    }


/******************************************************************************
 * PURPOSE: isWriteMode - Determine if the file is in write mode.
 * INPUTS:  File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: int 1 if xdrMode is XDR_ENCODE.
 * NOTES:
 *****************************************************************************/

static int isWriteMode ( const File* file )
    {
    PRE ( isWritableFile ( file ) );

    return file->xdrMode == XDR_ENCODE;
    }


/******************************************************************************
 * PURPOSE: ensureReadMode - Make sure the file is in read mode.
 * INPUTS:  File* file  The file to put in read mode.
 * OUTPUTS: File* file  The file put in read mode.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void ensureReadMode ( File* file )
    {
    PRE ( isReadableFile ( file ) );

    if ( file->xdrMode != XDR_DECODE )
        {
        fflush ( file->file ); /* Flush the output buffer. */
        xdr_destroy ( &file->xdr ); /* Free XDR buffer. */
        file->xdrMode = XDR_DECODE;
        xdrstdio_create ( &file->xdr, file->file, file->xdrMode );
        }

    POST ( isReadMode ( file ) );
    }


/******************************************************************************
 * PURPOSE: ensureWriteMode - Make sure the file is in write mode.
 * INPUTS:  File* file  The file to put in write mode.
 * OUTPUTS: File* file  The file put in write mode.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void ensureWriteMode ( File* file )
    {
    PRE ( isWritableFile ( file ) );

    if ( file->xdrMode != XDR_ENCODE )
        {
        fflush ( file->file ); /* Flush the input buffer. */
        xdr_destroy ( &file->xdr ); /* Free XDR buffer. */
        file->xdrMode = XDR_ENCODE;
        xdrstdio_create ( &file->xdr, file->file, file->xdrMode );
        }

    POST ( isWriteMode ( file ) );
    }

/******************************************************************************
 * PURPOSE: requiresBuffering - Determine if a read requires buffering.
 * INPUTS:  const File* file    The file to check.
 *          size_t bytes        The number of bytes to read.
 * OUTPUTS: None
 * RETURNS: int 1 if the read requires buffering, else 0.
 * NOTES:
 *****************************************************************************/

static int requiresBuffering ( const File* file, size_t bytes )
    {
    PRE2 ( isReadableFile ( file ), bytes );

    return AND2 ( file->port, bytes > file->bufferSize );
    }


/******************************************************************************
 * PURPOSE: readArrayBuffered - Read an array of data from a buffer-limited
 *          file (e.g., a socket).
 * INPUTS:  File*           file         File to read.
 *          size_t          count        Number of items to read.
 *          size_t          sizeOfItem   Size in bytes of an item.
 *                                       E.g., sizeof (float)
 *          const xdrproc_t elproc       Optional: XDR element reader procedure
 *                                       e.g., xdr_float.
 *                                       If 0 then fread() is used instead of
 *                                       xdr_vector().
 * OUTPUTS: void*           array        Filled array of items.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

static int readArrayBuffered ( File* file, size_t count, size_t sizeOfItem,
                               const xdrproc_t elproc, void* array )
    {
    PRE5 ( isReadableFile ( file ), count, sizeOfItem, array,
           file->bufferSize >= sizeOfItem );

    int ok = 1;

    const size_t sizeOfBuffer = file->bufferSize;

    const size_t maxItemsReadable = sizeOfBuffer / sizeOfItem;

    size_t itemsRead = 0; /* Number of items already read so far. */

    const size_t stride = sizeOfItem / sizeof ( char ); /* Chars per item. */

    char* const c = ( char* ) array; /* To enable pointer arithmetic. */

    CHECK3 ( sizeOfBuffer >= sizeOfItem, stride, maxItemsReadable );

    while ( AND2 ( ok, itemsRead != count ) )
        {
        const size_t itemsRemaining = count - itemsRead;

        const size_t itemsToReadNow = itemsRemaining < maxItemsReadable ?
                                      itemsRemaining
                                      : maxItemsReadable;

        const size_t offset = itemsRead * stride; /* To next item in array. */

        CHECK2 ( count > itemsRead, itemsToReadNow );

        if ( elproc )
            {
            ok = xdr_vector ( &file->xdr, c + offset, itemsToReadNow, sizeOfItem,
                              elproc );
            }
        else
            {
            const int itemsActuallyRead =
                fread ( c + offset, sizeOfItem, itemsToReadNow, file->file );

            ok = itemsActuallyRead == itemsToReadNow;
            }

        if ( ! ok )
            {
            error ( "Can't read %u items of partial data from the socket '%s'.\n"
                    "(Only read a total of %u items out of %u expected.)",
                    itemsToReadNow, file->name, itemsRead, count );
            }
        else itemsRead += itemsToReadNow;
        }

    return ok;
    }

