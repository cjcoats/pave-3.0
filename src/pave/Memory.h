#ifndef MEMORY_H
#define MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PURPOSE: Memory.h - Defines macros for memory allocation and declares
 *          related routines.
 * NOTES:   Required source-code control string:
 *          "@(#)Memory.h	2.2 /env/proj/archive/edss/src/pave/pave_drawmap/SCCS/s.Memory.h 11/09/99 14:03:10"
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
 *          02/1999, Todd Plessel, EPA/MMTSI, Added NEW_ZERO(), RESIZE().
 *****************************************************************************/

/*================================ INCLUDES =================================*/

#include <stddef.h> /* For the size_t typedef. */

/*================================= MACROS ==================================*/

/* Allocate memory large enough to hold 'count' items of given 'type': */

#define NEW( type, count ) new_( (count) * sizeof (type), 0 )

/* Allocate memory large enough to hold 'count' items of size 'size': */

#define NEW_SIZE( size, count ) new_( (count) * (size), 0 )

/* Versions that allocate then zero-out the memory: */

#define NEW_ZERO( type, count ) ((type*) new_( (count) * sizeof (type), 1 ))

#define NEW_SIZE_ZERO( size, count ) new_( (count) * (size), 1 )

/*
 * Re-size an existing allocated block by deltaCount:
 * Example:
 *
 *   Foo* fooArray = NEW( Foo, 100 );
 *
 *   if ( fooArray )
 *   {
 *     size_t fooCount = 100;
 *
 *     ...grow by 10 more Foo...
 *
 *     if ( RESIZE( &fooArray, &fooCount, 10 ) )
 *     {
 *       ...fooCount is now 110...
 *     }
 *
 *     FREE( fooArray );
 *   }
 */

#define RESIZE( addressOfExistingAddress, addressOfExistingCount, deltaCount ) \
resize_( sizeof **(addressOfExistingAddress), \
(const void**) (addressOfExistingAddress), \
(addressOfExistingCount), (deltaCount), 0 )

/* Re-size an existing allocated block and zero-out any extra portion: */

#define RESIZE_ZERO( addressOfExistingAddress, addressOfExistingCount, \
deltaCount ) \
resize_( sizeof **(addressOfExistingAddress), \
(const void**) (addressOfExistingAddress), \
(addressOfExistingCount), (deltaCount), 1 )

/*
 * Re-size an existing allocated block by deltaCount:
 * Example:
 *
 *   const size_t elementSize = type == INTEGER ? sizeof (int) : sizeof (float);
 *   void* array = NEW_SIZE( elementSize, 100 );
 *
 *   if ( array )
 *   {
 *     size_t arrayCount = 100;
 *
 *     ...grow by 10 more ints or floats...
 *
 *     if ( RESIZE_SIZE( &array, elementSize, &arrayCount, 10 ) )
 *     {
 *       ...arrayCount is now 110...
 *     }
 *
 *     FREE( array );
 *   }
 */

#define RESIZE_SIZE( addressOfExistingAddress, elementSize, \
addressOfExistingCount, deltaCount ) \
resize_( elementSize, \
(const void**) (addressOfExistingAddress), \
(addressOfExistingCount), (deltaCount), 0 )

#define RESIZE_SIZE_ZERO( addressOfExistingAddress, elementSize, \
addressOfExistingCount, deltaCount ) \
resize_( elementSize, \
(const void**) (addressOfExistingAddress), \
(addressOfExistingCount), (deltaCount), 1 )

/* Deallocate the memory and zero the pointer: */

#define FREE( p ) ( ( (p) ? free_( p ) : (void) 0 ), (p) = 0 )

/*=============================== FUNCTIONS =================================*/

extern void* new_( size_t bytes, int zeroIt );
extern void  free_( void* address );
extern int   resize_( size_t typeSize,
                      const void** existingAddress, size_t* existingCount,
                      long deltaCount, int zeroExtra );

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_H */

