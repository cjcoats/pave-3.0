#ifndef FILE_H
#define FILE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PURPOSE: File.h - Declare functions for reading and writing
 *          ASCII and platform-independent binary (XDR) files.
 * NOTES:   Required source-code control string:
 *          "$Id: File.h 83 2018-03-12 19:24:33Z coats $"
 * HISTORY: 12/95, Todd Plessel, EPA/MMTSI, Created.
 *          02/2018, by Carlie J. Coats, Jr.:  Version for PAVE-2.4
 *****************************************************************************/

/*================================ INCLUDES =================================*/

#include <stdio.h>  /* For FILE and to export SEEK_SET, SEEK_CUR, SEEK_END. */
#include <stddef.h> /* For size_t. */
#include <rpc/rpc.h>    /* For XDR.    */

/*================================= TYPES ===================================*/

typedef struct
    {
    char  name[257];
    XDR   xdr;
    FILE* file;
    } File;

/*=============================== FUNCTIONS =================================*/

extern int  isValidFile ( const File* file );
extern int  openFile ( const char* fileName, const char* mode, File* file );
extern int  openPipe ( const char* command,  const char* mode, File* file );
extern int  closeFile (   File* file );
extern int  closePipe (   File* file );
extern int  seekFile (    File* file, long offset, int whence );
extern long tellFile (    File* file );
extern int  isEndOfFile ( File* file );

extern int readByte (    File* file, void*   x );
extern int readChar (    File* file, char*   x );
extern int readShort (   File* file, short*  x );
extern int readShort2 (  File* file, short*  x );
extern int readInt (     File* file, int*    x );
extern int readFloat (   File* file, float*  x );
extern int readDouble (  File* file, double* x );
extern int readBytes (   File* file, void*   a,   size_t n );
extern int readChars (   File* file, char*   a,   size_t n );
extern int readShorts (  File* file, short*  a,   size_t n );
extern int readShorts2 ( File* file, short*  a,   size_t n );
extern int readInts (    File* file, int*    a,   size_t n );
extern int readFloats (  File* file, float*  a,   size_t n );
extern int readDoubles ( File* file, double* a,   size_t n );
extern int readString (  File* file, char    s[], size_t n );

extern int writeByte (    File* file, unsigned char x );
extern int writeChar (    File* file, char   x );
extern int writeShort (   File* file, short  x );
extern int writeShort2 (  File* file, short  x );
extern int writeInt (     File* file, int    x );
extern int writeFloat (   File* file, float  x );
extern int writeDouble (  File* file, double x );
extern int writeBytes (   File* file, const void*   a, size_t n );
extern int writeChars (   File* file, const char*   a, size_t n );
extern int writeShorts (  File* file, const short*  a, size_t n );
extern int writeShorts2 ( File* file, const short*  a, size_t n );
extern int writeInts (    File* file, const int*    a, size_t n );
extern int writeFloats (  File* file, const float*  a, size_t n );
extern int writeDoubles ( File* file, const double* a, size_t n );
extern int writeString (  File* file, const char*   s, ... );


#ifdef __cplusplus
    }
#endif

#endif /* FILE_H */
