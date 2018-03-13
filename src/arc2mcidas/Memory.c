
/******************************************************************************
 * PURPOSE: Memory.c - Defines memory allocation routines.
 * NOTES:   Implementation is based on calls the standard malloc() and free()
 *          routines so clients should use -lmalloc when linking.
 * HISTORY: 03/93, Todd Plessel, EPA/MMTSI, Created.
 *          11/94, Todd Plessel, EPA/MMTSI, Modified for use as an example.
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include <stddef.h> /* For size_t. */
#include <malloc.h> /* For malloc() and free(). */

#include "Assertions.h" /* For DEBUG(). */
#include "Error.h"      /* For error(). */
#include "Memory.h"     /* For the routine definitions. */

/*============================ PRIVATE VARIABLES ============================*/

/* Required source-code control string. */
static char* Memory_version = "$Id: Memory.c 83 2018-03-12 19:24:33Z coats $";

/*=========================== PUBLIC FUNCTIONS ==============================*/


/******************************************************************************
 * PURPOSE: free_ - Implements the FREE() macro by calling free().
 *          This provides a checking and debugging wrapper around free().
 * INPUTS:  void* address  The address to free.
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void free_ ( void* address )
    {
    char* p = ( char* ) address;

    PRE ( address )

    DEBUG ( printf ( "free_( address = %p )\n", address ); )

    *p = '\0'; /* Zero first byte to reveal dangling pointer errors in client. */

    free ( address );
    }


/******************************************************************************
 * PURPOSE: new_ - Implements the NEW() macro by calling the standard malloc()
 *          and, upon failure error() is called.
 *          This provides a checking and debugging wrapper around malloc().
 * INPUTS:  size_t bytes  The number of bytes to allocate.
 * OUTPUTS: None
 * RETURNS: void*  The resulting address, or 0 if unsuccessful.
 * NOTES:   If malloc() returns 0 then error() is invoked.
 *****************************************************************************/

void* new_ ( size_t bytes )
    {
    void* address = 0;

    PRE ( bytes )

    DEBUG ( printf ( "new_( bytes = %u )", bytes ); )

    address = ( void* ) malloc ( bytes ); /* (void*) cast required on DEC!? */

    DEBUG ( printf ( " yields address = %p\n", address ); )

    if ( address == 0 )
        error ( "Can't allocate %u bytes to complete the requested action.", bytes );

    return address;
    }

