#ifndef FILE_H
#define FILE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PURPOSE: File.h - Declare functions for reading and writing to
 *          ASCII and portable binary (XDR) files, pipes and sockets.
 * NOTES:   Required source-code control string:
 *          "@(#)File.h	2.2 /env/proj/archive/edss/src/pave/pave_drawmap/SCCS/s.File.h 11/09/99 14:03:06"
 * HISTORY: 12/1995, Todd Plessel, EPA/MMTSI, Created.
 *****************************************************************************/

/*================================ INCLUDES =================================*/

#include <stdio.h>  /* For FILE and to export SEEK_SET, SEEK_CUR, SEEK_END. */
#include <stddef.h> /* For size_t. */

/* HACK around HP-UX BUG: rpc.h is not protected against multiple inclusions!*/
#ifndef __RPC_RPC_H__
#include <rpc/rpc.h>    /* For XDR.    */
#define __RPC_RPC_H__
#endif /* __RPC_RPC_H__ */

/*================================= MACROS =================================*/

/* Macro version of destructor zeros pointer to avoid dangling references: */

#define closeFile(f) ((f) && closeFile_(f) ? ! (int)((f) = 0) : (int)((f) = 0))

/*================================= TYPES ===================================*/

/* Except for the implementation, File is just an opaque structure: */

typedef struct File_ File;

/*=============================== FUNCTIONS =================================*/

/* Constructors: */

extern File* openFile(   const char* fileName, const char* mode );
extern File* openPipe(   const char* command,  const char* mode );
extern File* openSocket( int port, const char* host );

/* Destructor: */

extern int closeFile_( File* file ); /* Used via closeFile() macro. */

/* Queries: */

extern int  isValidFile(       const File* file ); /* Invariant. */
extern int  isReadableFile(    const File* file );
extern int  isWritableFile(    const File* file );
extern int  isSeekableFile(    const File* file );
extern int  isNonBlockingFile( const File* file );
extern int  isEndOfFile(             File* file );
extern long offsetInFile(            File* file );
extern const char* nameOfFile( const File* file );
extern size_t sizeOfFile(      const File* file );
extern FILE* fileOfFile(       const File* file ); /* HACK: violate Demeter! */

/* Commands: */

extern int seekFile( File* file, long offset, int whence );

extern int readByte(              File* file, void*   x );
extern int readChar(              File* file, char*   x );
extern int readShort(             File* file, short*  x );
extern int readShort2(            File* file, short*  x );
extern int readInt(               File* file, int*    x );
extern int readLong(              File* file, long*   x );
extern int readShortToLong(       File* file, long*   x );
extern int readShort2ToLong(      File* file, long*   x );
extern int readIntToLong(         File* file, long*   x );
extern int readFloat(             File* file, float*  x );
extern int readDouble(            File* file, double* x );
extern int readBytes(             File* file, void*   a, size_t n );
extern int readChars(             File* file, char*   a, size_t n );
extern int readShorts(            File* file, short*  a, size_t n );
extern int readShorts2(           File* file, short*  a, size_t n );
extern int readInts(              File* file, int*    a, size_t n );
extern int readLongs(             File* file, long*   a, size_t n );
extern int readShortsToLongs(     File* file, long*   a, size_t n );
extern int readShorts2ToLongs(    File* file, long*   a, size_t n );
extern int readIntsToLongs(       File* file, long*   a, size_t n );
extern int readFloats(            File* file, float*  a, size_t n );
extern int readDoubles(           File* file, double* a, size_t n );
extern int readString(            File* file, char*   s, size_t n );

extern size_t readSomeBytes(      File* file, void* a, size_t n );

extern int writeByte(             File* file, unsigned char x );
extern int writeChar(             File* file, char   x );
extern int writeShort(            File* file, short  x );
extern int writeShort2(           File* file, short  x );
extern int writeInt(              File* file, int    x );
extern int writeLong(             File* file, long   x );
extern int writeFloat(            File* file, float  x );
extern int writeDouble(           File* file, double x );
extern int writeBytes(            File* file, const void*   a, size_t n );
extern int writeChars(            File* file, const char*   a, size_t n );
extern int writeShorts(           File* file, const short*  a, size_t n );
extern int writeShorts2(          File* file, const short*  a, size_t n );
extern int writeInts(             File* file, const int*    a, size_t n );
extern int writeLongs(            File* file, const long*   a, size_t n );
extern int writeShortsFromLongs(  File* file, const long*   a, size_t n );
extern int writeShorts2FromLongs( File* file, const long*   a, size_t n );
extern int writeIntsFromLongs(    File* file, const long*   a, size_t n );
extern int writeFloats(           File* file, const float*  a, size_t n );
extern int writeDoubles(          File* file, const double* a, size_t n );
extern int writeString(           File* file, const char*   s, ... );


#ifdef __cplusplus
}
#endif

#endif /* FILE_H */
