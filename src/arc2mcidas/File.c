/******************************************************************************
 * PURPOSE: File.c - Define functions for reading and writing
 *          ASCII and platform-independent binary (XDR) files.
 * NOTES:   Required source-code control string:
 *          "$Id: File.c 83 2018-03-12 19:24:33Z coats $"
 * HISTORY: 12/1995, Todd Plessel, EPA/MMTSI, Created.
 *          02/2018, by Carlie J. Coats, Jr.:  Version for PAVE-2.4
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include <stdio.h>      /* For FILE, fopen(), fclose(), vfprintf(), *getc(). */
#include <stddef.h>     /* For typedef size_t.                               */
#include <string.h>     /* For strncpy(), memset().                          */
#include <stdarg.h>     /* For va_list, va_start, va_end.                    */
#include <rpc/rpc.h>    /* For XDR.                                          */

#include "Assertions.h" /* For macros PRE(), POST(), AND*().                */
#include "Error.h"      /* For error().                                     */
#include "File.h"       /* For public functions.                            */

/*================================ DEFINES ==================================*/

/* For read/writeShorts*() optimization: */

#ifdef __alpha
#define IS_LITTLE_ENDIAN 1
#else
#define IS_LITTLE_ENDIAN 0
#endif

/*=========================== PRIVATE VARIABLES =============================*/

/* Required source-code control string. */
static char* File_version = "$Id: File.c 83 2018-03-12 19:24:33Z coats $";

/*========================== FORWARD DECLARATIONS ===========================*/

static void zeroFile ( File* file );

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
    return AND3 ( file, file->name[ 0 ], file->file );
    }


/******************************************************************************
 * PURPOSE: openFile - Open a file for reading or writing.
 * INPUTS:  const char* fileName  The name of the file to open or one of:
 *                                "-stdin", "-stdout", "-stderr".
 *          const char* mode      "r" or "w".
 * OUTPUTS: File*       file      Initialized file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int openFile ( const char* fileName, const char* mode, File* file )
    {
    int ok = 0;

    PRE4 ( fileName, mode, file, IN3 ( *mode, 'r', 'w' ) )

    zeroFile ( file );
    strncpy ( file->name, fileName, 256 ); /* Truncate copy of fileName string. */

    if      ( strcmp ( fileName, "-stdin"  ) == 0 )
        {
        if ( *mode == 'r' ) file->file = stdin;
        }
    else if ( strcmp ( fileName, "-stdout" ) == 0 )
        {
        if ( *mode == 'w' ) file->file = stdout;
        }
    else if ( strcmp ( fileName, "-stderr" ) == 0 )
        {
        if ( *mode == 'w' ) file->file = stderr;
        }
    else file->file = fopen ( fileName, mode ); /* Use full fileName for fopen().*/

    if ( file->file )
        {
        ok = 1;
        xdrstdio_create ( &file->xdr, file->file,
                          *mode == 'r' ? XDR_DECODE : XDR_ENCODE );
        }
    else
        {
        error ( "Can't open file '%s' for %s.", fileName,
                *mode == 'r' ? "reading" : "writing" );

        zeroFile ( file );
        }

    POST2 ( IMPLIES ( ok, isValidFile ( file ) ),
            IMPLIES ( isValidFile ( file ), ok ) )

    return file->file != 0;
    }


/******************************************************************************
 * PURPOSE: openPipe - Open a pipe for reading or writing.
 * INPUTS:  const char* command   The command used to open a pipe.
 *          const char* mode      "r" or "w".
 * OUTPUTS: File*       file      Initialized file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int openPipe ( const char* command, const char* mode, File* file )
    {
    int ok = 0;

    PRE4 ( command, mode, file, IN3 ( *mode, 'r', 'w' ) )

    zeroFile ( file );

    /*
     * Must flush stdout to sync subsequent output in cases where pipe command
     * outputs to stdout. E.g., "cat", "echo", etc.
     */

    fflush ( stdout );

    strncpy ( file->name, command, 256 ); /* Truncate copy of command string. */
    file->file = popen ( command, mode ); /* But use full command for popen(). */

    if ( file->file )
        {
        ok = 1;
        xdrstdio_create ( &file->xdr, file->file,
                          *mode == 'r' ? XDR_DECODE : XDR_ENCODE );
        }
    else
        {
        error ( "Can't open pipe with command '%s' for %s.", command,
                *mode == 'r' ? "reading" : "writing" );

        zeroFile ( file );
        }

    POST2 ( IMPLIES ( ok, isValidFile ( file ) ),
            IMPLIES ( isValidFile ( file ), ok ) )

    return file->file != 0;
    }


/******************************************************************************
 * PURPOSE: closeFile - Close a file.
 * INPUTS:  File* file  The file to close.
 * OUTPUTS: File* file  Destroyed and zeroed file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int closeFile ( File* file )
    {
    int ok = 0;

    PRE ( isValidFile ( file ) )

    xdr_destroy ( &file->xdr ); /* Free XDR buffer. */

    if ( AND3 ( strcmp ( file->name, "-stdin"  ),
                strcmp ( file->name, "-stdout" ),
                strcmp ( file->name, "-stderr" ) ) )
        {
        ok = fclose ( file->file ) == 0;
        }
    else ok = 1;

    if ( ! ok ) error ( "Can't close file '%s'.", file->name );

    zeroFile ( file );

    POST ( ! isValidFile ( file ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: closePipe - Close a pipe.
 * INPUTS:  File* file  The pipe to close.
 * OUTPUTS: File* file  Destroyed and zeroed file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int closePipe ( File* file )
    {
    int ok = 0;

    PRE ( isValidFile ( file ) )

    xdr_destroy ( &file->xdr ); /* Free XDR buffer. */

    ok = pclose ( file->file ) == 0;

    if ( ! ok ) error ( "Can't close pipe '%s'.", file->name );

    zeroFile ( file );

    POST ( ! isValidFile ( file ) )

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
    int ok = 0;

    PRE2 ( isValidFile ( file ),
           IN4 ( whence, SEEK_SET, SEEK_CUR, SEEK_END ) )

    ok = fseek ( file->file, offset, whence ) == 0;

    if ( ! ok )
        {
        error ( "Can't seek to byte %d%s in file '%s'.", offset,
                whence == SEEK_CUR ? " from current location" :
                whence == SEEK_END  ? " from end" : "",
                file->name );
        }

    POST ( isValidFile ( file ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: tellFile - Seek to a location in a file.
 * INPUTS:  File* file   The file to seek on.
 * OUTPUTS: None
 * RETURNS: long the current offset from the beginning of the file.
 * NOTES:
 *****************************************************************************/

long tellFile ( File* file )
    {
    long offset = 0;

    PRE ( isValidFile ( file ) )

    offset = ftell ( file->file );

    POST ( isValidFile ( file ) )

    return offset;
    }


/******************************************************************************
 * PURPOSE: isEndOfFile - Checks if a File pointer has reached the end.
 * INPUTS:  File* file  The file to check.
 * OUTPUTS: None
 * RETURNS: int 1 if EOF, else 0.
 * NOTES:
 *****************************************************************************/

int isEndOfFile ( File* file )
    {
    int ch;

    PRE ( isValidFile ( file ) )

    /*
     * Implementation note:
     *   feof() is not used since it does not work with fseek().
     *   ftell() (compared with a cached file size) is five times slower than
     *   the fgetc(); ungetc() approach used here!
     */

    ch = fgetc ( file->file );
    ungetc ( ch, file->file );

    return ch == EOF;
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
    int ok = 0;

    PRE2 ( isValidFile ( file ), x )

    ok = fread ( x, 1, 1, file->file ) == 1;

    if ( ! ok ) error ( "Can't read a byte from file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE2 ( isValidFile ( file ), x )

    ok = xdr_char ( &file->xdr, x );

    if ( ! ok ) error ( "Can't read a char from file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE2 ( isValidFile ( file ), x )

    ok = xdr_short ( &file->xdr, x );

    if ( ! ok ) error ( "Can't read a short from file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;
    unsigned char lowByte;
    signed char highByte;

    PRE2 ( isValidFile ( file ), x )

    ok = AND2 ( fread ( &highByte, 1, 1, file->file ) == 1,
                fread ( &lowByte,  1, 1, file->file ) == 1 );

    if ( ok )
        {
        DEBUG ( printf ( "highByte = %x, lowByte = %x\n", highByte, lowByte ); )

        *x = ( highByte << 8 ) + lowByte;

        DEBUG ( printf ( "value = %d\n", *x ); )
        }

    if ( ! ok ) error ( "Can't read a short from file '%s'.", file->name );

    POST ( isValidFile ( file ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: readInt - Read an integer from a file.
 * INPUTS:  File* file  The file to read from.
 * OUTPUTS: int* x      The integer read.
 *          File*  file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int readInt ( File* file, int* x )
    {
    int ok = 0;

    PRE2 ( isValidFile ( file ), x )

    ok = xdr_int ( &file->xdr, x );

    if ( ! ok ) error ( "Can't read an integer from file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE2 ( isValidFile ( file ), x )

    ok = xdr_float ( &file->xdr, x );

    if ( ! ok ) error ( "Can't read a float from file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE2 ( isValidFile ( file ), x )

    ok = xdr_double ( &file->xdr, x );

    if ( ! ok ) error ( "Can't read a double from file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = fread ( a, 1, n, file->file ) == n;

    if ( ! ok ) error ( "Can't read %u bytes from file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

    return ok;
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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = xdr_vector ( &file->xdr, a, n, sizeof ( char ), xdr_char );

    if ( ! ok ) error ( "Can't read %u chars from file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( short ), xdr_short );

    if ( ! ok ) error ( "Can't read %u shorts from file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    if ( AND2 ( sizeof ( short ) == 2, ! IS_LITTLE_ENDIAN ) ) /* Optimize: */
        {
        DEBUG ( printf ( "calling fread\n" ); )

        ok = fread ( a, sizeof ( short ), n, file->file ) == n;
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
        }

    if ( ! ok ) error ( "Can't read %u shorts from file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( int ), xdr_int );

    if ( ! ok ) error ( "Can't read %u ints from file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( float ), xdr_float );

    if ( ! ok ) error ( "Can't read %u floats from file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( double ), xdr_double );

    if ( ! ok ) error ( "Can't read %u doubles from file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
 *          Reading stops when a newline character is read (and stored in s) -
 *          like fgets().
 *****************************************************************************/

int readString ( File* file, char s[], size_t n )
    {
    int ok = 0;

    PRE3 ( isValidFile ( file ), s, n )

    ok = fgets ( s, n, file->file ) != 0;

    if ( ! ok )
        error ( "Can't read a %u-length string from file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE ( isValidFile ( file ) )

    ok = fwrite ( &x, 1, 1, file->file ) == 1;

    if ( ! ok ) error ( "Can't write a byte to file '%s'.", file->name );

    POST ( isValidFile ( file ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeChar - Write a char to a file.
 * INPUTS:  File* file  The file to write to.
 *          char  x     The char to write.
 * OUTPUTS: File* file  Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeChar ( File* file, char x )
    {
    int ok = 0;

    PRE ( isValidFile ( file ) )

    ok = xdr_char ( &file->xdr, &x );

    if ( ! ok ) error ( "Can't write a char to file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE ( isValidFile ( file ) )

    ok = xdr_short ( &file->xdr, &x );

    if ( ! ok ) error ( "Can't write a short to file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;
    const unsigned char lowByte  = x & 0xff;
    const unsigned char highByte = ( x >> 8 ) & 0xff;

    PRE ( isValidFile ( file ) )

    DEBUG ( printf ( "value = %d\n", x ); )
    DEBUG ( printf ( "highByte = %x, lowByte = %x\n", highByte, lowByte ); )

    ok = AND2 ( fwrite ( &highByte, 1, 1, file->file ) == 1,
                fwrite ( &lowByte,  1, 1, file->file ) == 1 );

    if ( ! ok ) error ( "Can't write a short to file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE ( isValidFile ( file ) )

    ok = xdr_int ( &file->xdr, &x );

    if ( ! ok ) error ( "Can't write an int to file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE ( isValidFile ( file ) )

    ok = xdr_float ( &file->xdr, &x );

    if ( ! ok ) error ( "Can't write a float to file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE ( isValidFile ( file ) )

    ok = xdr_double ( &file->xdr, &x );

    if ( ! ok ) error ( "Can't write a double to file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = fwrite ( a, 1, n, file->file ) == n;

    if ( ! ok ) error ( "Can't write %u bytes to file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( char ), xdr_char );

    if ( ! ok ) error ( "Can't write %u chars to file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( short ), xdr_short );

    if ( ! ok ) error ( "Can't write %u shorts to file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    if ( AND2 ( sizeof ( short ) == 2, ! IS_LITTLE_ENDIAN ) ) /* Optimize: */
        {
        DEBUG ( printf ( "calling fwrite()\n" ); )

        ok = fwrite ( a, sizeof ( short ), n, file->file ) == n;
        }
    else /* Must loop over each byte pair: */
        {
        size_t i;

        for ( i = 0; i < n; ++i )
            {
            const unsigned char lowByte  = a[ i ] & 0xff;
            const unsigned char highByte = ( a[ i ] >> 8 ) & 0xff;

            DEBUG ( printf ( "value = %d\n", a[ i ] ); )
            DEBUG ( printf ( "highByte = %x, lowByte = %x\n", highByte, lowByte ); )

            ok = AND2 ( fwrite ( &highByte, 1, 1, file->file ) == 1,
                        fwrite ( &lowByte,  1, 1, file->file ) == 1 );
            }
        }

    if ( ! ok ) error ( "Can't write %u shorts to file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( int ), xdr_int );

    if ( ! ok ) error ( "Can't write %u ints to file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( float ), xdr_float );

    if ( ! ok ) error ( "Can't write %u floats to file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

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
    int ok = 0;

    PRE3 ( isValidFile ( file ), a, n )

    ok = xdr_vector ( &file->xdr, ( char* ) a, n, sizeof ( double ), xdr_double );

    if ( ! ok ) error ( "Can't write %u doubles to file '%s'.", n, file->name );

    POST ( isValidFile ( file ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeString - Write a string to a file.
 * INPUTS:  File*      file     The file to read from.
 *          const char* format  The string to write. May be a printf()-like
 *                              format string like "%d %f\n" followed by args.
 *          size_t      n       The number of characters to read.
 * OUTPUTS: File*      file     Updated file structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *          Implemented in terms of vfprintf (man vfprintf).
 *****************************************************************************/

int writeString ( File* file, const char* format, ... )
    {
    int ok = 0;
    va_list args; /* For stdarg magic. */

    PRE2 ( isValidFile ( file ), format )

    va_start ( args, format );                       /* Begin stdarg magic.  */
    ok = vfprintf ( file->file, format, args ) != -1; /* Pass args along.     */
    va_end ( args );                                 /* End of stdarg magic. */

    if ( ! ok )
        error ( "Can't write string to file '%s'.", file->name );

    POST ( isValidFile ( file ) )

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
    PRE ( file )

    memset ( file, 0, sizeof ( File ) );

    POST ( IS_ZERO2 ( file->name[ 0 ], file->file ) )
    }


