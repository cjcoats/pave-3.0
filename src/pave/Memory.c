/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Memory.c 83 2018-03-12 19:24:33Z coats $
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
 * PURPOSE: Memory.c - Defines memory allocation routines.
 * NOTES:   Implementation is based on calls the standard malloc() and free()
 *          routines so clients should use -lmalloc when linking.
 * HISTORY: 03/93, Todd Plessel, EPA/MMTSI, Created.
 *          11/94, Todd Plessel, EPA/MMTSI, Modified for use as an example.
 *          Required source-code control string:
 *          "%W% %P% %G% %U%"
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include <stddef.h>     /* For size_t.                     */
#include <malloc.h>     /* For malloc() and free().        */
#include <stdlib.h>     /* For malloc() and free() on DEC. */

#include "Assertions.h" /* For DEBUG().                    */
#include "Error.h"      /* For error().                    */
#include "Memory.h"     /* For the routine definitions.    */

static const char SVN_ID[] = "$Id: Memory.c 83 2018-03-12 19:24:33Z coats $";

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
    PRE ( address );

    char* p = ( char* ) address;

    DEBUG ( printf ( "free_( address = %p )\n", address ); )

    *p = '\0'; /* Zero first byte to reveal dangling pointer errors in client. */

    free ( address );
    }


/******************************************************************************
 * PURPOSE: new_ - Implements the NEW() macro by calling the standard malloc()
 *          and, upon failure error() is called.
 *          This provides a checking and debugging wrapper around malloc().
 * INPUTS:  size_t bytes     The number of bytes to allocate.
 *          int    zeroIt    Zero-out the memory?
 * OUTPUTS: None
 * RETURNS: void*  The resulting address, or 0 if unsuccessful.
 * NOTES:   If malloc() returns 0 then error() is invoked.
 *****************************************************************************/

void* new_ ( size_t bytes, int zeroIt )
    {
    PRE2 ( bytes, IS_BOOL ( zeroIt ) );

    void* address = 0;

    DEBUG ( printf ( "new_( bytes = %u, zeroIt = %d )", bytes, zeroIt ); )

    address = malloc ( bytes );

    DEBUG ( printf ( " yields address = %p\n", address ); )

    if ( address == 0 )
        error ( "Can't allocate %u bytes to complete the requested action.", bytes );
    else if ( zeroIt ) memset ( address, 0, bytes );

    return address;
    }


/******************************************************************************
 * PURPOSE: resize_ - Implements the RESIZE() macro by calling the standard
 *          realloc() and, upon failure error() is called.
 *          This provides an easier-to-use substitute for the notoriously
 *          problematic realloc().
 * INPUTS:  size_t typeSize         Size of type in bytes.
 *          const void** existingAddress  Pointer to pointer to a block
 *                                  to be reallocated (or pointer to 0 to
 *                                  allocate a new block).
 *          size_t* existingCount   Pointer to # of items in existing block.
 *          long deltaCount         The extra (or fewer) number of items to
 *                                  grow (or shink) the existing block by.
 *                                  If deltaCount == -existingCount then the
 *                                  existing block is freed (and
 *                                  *existingAddress == 0).
 *          int zeroDelta           Zero-out extra memory (if deltaCount > 0)?
 * OUTPUTS: const void** existingAddress  Possibly changed address of a block of
 *                                        size existingCount + deltaCount items.
 *          size_t* existingCount   Pointer to new # of items in existing block.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If realloc() fails then error() is invoked and existingAddress and
 *          existingCount are unchanged.
 *
 *          Example:
 *
 *            const size_t initialEmployeeCount = 100;
 *            Employee* employees = NEW_ZERO( Employee, initialEmployeeCount );
 *
 *            if ( employees )
 *            {
 *              size_t employeeCount = initialEmployeeCount;
 *
 *              ... do stuff with employees ...
 *
 *              if ( RESIZE_NEW( Employee, employees, employeeCount, 10 ) )
 *              {
 *                (employeeCount is now 110 and
 *                employees may be a different address)
 *
 *                ... do stuff with additional employees ...
 *
 *                if ( RESIZE_NEW( Employee, employees, employeeCount,
 *                                 employeeCount / -2 ) )
 *                {
 *                  (after 50% down-sizing, employeeCount is now 55)
 *
 *                  ... do stuff with remaining employees ...
 *                }
 *
 *              }
 *            }
 *
 *            FREE( employees );
 *
 *****************************************************************************/

int resize_ ( size_t typeSize,
              const void** existingAddress, size_t* existingCount,
              long deltaCount, int zeroDelta )
    {
    PRE7 ( typeSize,
           existingAddress,
           existingCount,
           IMPLIES ( *existingAddress,      *existingCount ),
           IMPLIES ( *existingAddress == 0, *existingCount == 0 ),
           ( long ) *existingCount + deltaCount >= 0,
           IS_BOOL ( zeroDelta ) );

    CHECKING ( const void* const OLD ( existingAddress ) = *existingAddress; )
    CHECKING ( const size_t      OLD ( existingCount   ) = *existingCount; )

    int ok = 1;
    const size_t oldBytes =   *existingCount * typeSize;
    const size_t newBytes = ( *existingCount + deltaCount ) * typeSize;

    DEBUG ( printf ( "resize_( existingAddress = %p (%p), existingCount = %p (%u),"
                     " deltaCount = %ld, zeroDelta = %d )\n",
                     existingAddress, *existingAddress,
                     existingCount, *existingCount,
                     deltaCount, zeroDelta ); )

    DEBUG ( printf ( " newBytes = %u\n", newBytes ); )

    if ( newBytes == 0 )
        {
        /* Must use a temporary to free the const-cast-awayed pointer. */
        void* theExistingAddress = ( void* ) *existingAddress;
        FREE ( theExistingAddress );
        *existingAddress = 0;
        *existingCount   = 0;
        }
    else if ( newBytes != oldBytes )
        {
        /* HACK: avoid calling realloc with pointer-to-0 - it is not portable. */

        void* newAddress = *existingAddress == 0 ? malloc ( newBytes )
                           : realloc ( ( void* ) *existingAddress, newBytes );

        DEBUG ( printf ( " yields newAddress = %p\n", newAddress ); )

        if ( newAddress == 0 )
            {
            error ( "Can't re-allocate %u bytes to complete"
                    " the requested action.", newBytes );

            ok = 0;
            }
        else
            {
            *existingAddress = newAddress;
            newAddress = 0;

            if ( AND2 ( deltaCount > 0, zeroDelta ) )
                {
                const size_t deltaBytes    = deltaCount     * typeSize;
                const size_t existingBytes = *existingCount * typeSize;
                char* const  baseAddress   = ( char* ) *existingAddress;
                char* const  extraPortion  = baseAddress +
                                             existingBytes / sizeof ( char );

                DEBUG ( printf ( "Zeroing %u bytes at extraPortion = %p\n",
                                 deltaBytes, extraPortion ); )

                memset ( extraPortion, 0, deltaBytes );
                }

            *existingCount += deltaCount;
            }
        }

    POST4 ( IMPLIES ( *existingAddress,      *existingCount      ),
            IMPLIES ( *existingAddress == 0, *existingCount == 0 ),
            IMPLIES ( OR2 ( deltaCount == 0, ! ok ),
                      AND2 ( *existingAddress == OLD ( existingAddress ),
                             *existingCount   == OLD ( existingCount   ) ) ),
            IMPLIES ( ok, *existingCount == OLD ( existingCount ) + deltaCount ) );

    return ok;
    }

