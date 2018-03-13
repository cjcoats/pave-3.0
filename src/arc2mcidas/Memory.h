#ifndef MEMORY_H
#define MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PURPOSE: Memory.h - Defines macros for memory allocation and declares
 *          related routines.
 * NOTES:   Required source-code control string:
 *          "$Id: Memory.h 83 2018-03-12 19:24:33Z coats $"
 *          Implementation is based on calls the standard malloc() and free()
 *          routines so clients should use -lmalloc when linking.
 *          Usage:
 *            instead of:
 *              #include <errno.h>
 *              #include <malloc.h>
 *              int* p = (int*) malloc( n * sizeof (int) );
 *              if ( p ) { free( p ); p = 0; }
 *              else myErrorHandler( errno );
 *            use:
 *              #include <Error.h>
 *              #include <Memory.h>
 *              errorHandler( myErrorHandler );
 *              int* p = NEW( int, n );
 *              FREE( p );
 *          Uses the error() routine from Error.c
 * HISTORY: 03/93, Todd Plessel, EPA/MMTSI, Created.
 *          11/94, Todd Plessel, EPA/MMTSI, Modified for use as an example.
 *          02/2018, by Carlie J. Coats, Jr.:  Version for PAVE-2.4
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include <stddef.h> /* For the size_t typedef. */

/*=============================== DEFINES ===================================*/

#define NEW( type, count ) ((type*) new_( (count) * sizeof (type) ))
#define FREE( p ) ( (p) ? free_( p ) : (void) 0 ), (p) = 0

/*=============================== FUNCTIONS =================================*/

extern void* new_ ( size_t bytes );
extern void  free_ ( void* address );

#ifdef __cplusplus
    }
#endif

#endif /* MEMORY_H */

