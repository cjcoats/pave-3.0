
/******************************************************************************
 * PURPOSE: Error.c - Defines routines for setting user-defined error handlers.
 * NOTES:   The presence of the 'userErrorHandler' static 'global' variable
 *          means that multi-thread applications cannot have thread-specific
 *          handlers.
 * HISTORY: 03/1993, Todd Plessel, EPA/MMTSI, Created.
 *          11/1994, Todd Plessel, EPA/MMTSI, Modified for use as an example.
 *          02/2018, Carlie J. Coats, Jr.:  Cleanups for PAVE-2.4
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include <stdio.h>  /* For fprintf() and vfprintf(). */
#include <string.h> /* For strcat(). */
#include <errno.h>  /* For errno and strerror(). */
#include <limits.h> /* For UINT_MAX. */
#include <stdarg.h> /* For va_args. */

#include "Assertions.h" /* For DEBUG(). */
#include "Error.h"      /* For the routine definitions. */

/*============================ PRIVATE VARIABLES ============================*/

/* Required source-code control string. */
static char* Error_version = "$Id: Error.c 83 2018-03-12 19:24:33Z coats $";

static ErrorHandler userErrorHandler = 0; /* Points to user's handler.   */
static unsigned int errorCount       = 0; /* Count of all errors so far. */

/*=========================== PUBLIC FUNCTIONS ==============================*/


/******************************************************************************
 * PURPOSE: error - Prints the given error message (via strerror()) and then
 *          invokes the user error handler (if any).
 * INPUTS:  const char* message  A string like those used in printf().
 *          ...                  Other arguments implied by '%' in message.
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:   Implemented in terms of vsprintf (man vsprintf).
 *****************************************************************************/

void error ( const char* message, ... )
    {
    /*
     * Holds current value to pass to user after clearing the real errno.
     * to protect against user longjumps.
     */
    int errnoTmp = errno;

    static char expandedMessage[1024]; /* BUG: Hope it's big enough... */

    if ( errorCount == UINT_MAX ) errorCount = 1; /* Skip 0 when wrapping. */
    else                          errorCount++;   /* Update global count. */

    fprintf ( stderr, "\n\n\a" ); /* Print newlines and ring bell. */
    strcpy ( expandedMessage, "ERROR: " ); /* Add a standard prefix. */

    if ( message )
        {
        va_list args;              /* For stdarg magic. */
        va_start ( args, message ); /* Begin stdarg magic. */
        /* Append the expanded message. BUG: May overwrite past end!... */
        vsprintf ( expandedMessage + strlen ( expandedMessage ), message, args );
        va_end ( args );           /* End of stdarg magic. */
        }

    strcat ( expandedMessage, "\nReason: " ); /* Add a standard postfix. */
    if ( errno == 0 ) strcat ( expandedMessage, "User or data error.\n" );
    strcat ( expandedMessage, "(See console window for more details.)\n" );

    CHECK ( strlen ( expandedMessage ) < 1024U )
    /* BUG: Too late! We may have already overwritten memory in vsprintf()! */

    /* Add the expanded error message and reason. */
    if ( errno ) strcat ( expandedMessage, strerror ( errno ) );

    /* Print the expanded message. */
    fprintf ( stderr, expandedMessage );

    /* Print the current error count. */
    fprintf ( stderr, "(program failure # %u)\n\n", errorCount );

    DEBUG ( printf ( "errno = %d\n", errno ); )

    errno = 0; /* Clear the global now in case userErrorHandler never returns. */

    /* Finally, call the user' error handler routine (if it exists). */
    if ( userErrorHandler ) userErrorHandler ( errnoTmp, expandedMessage );
    }


/******************************************************************************
 * PURPOSE: getErrorHandler - Returns the current error handler (if any).
 * INPUTS:  None
 * OUTPUTS: None
 * RETURNS: ErrorHandler  The current error handler (if any).
 * NOTES:
 *****************************************************************************/

ErrorHandler getErrorHandler ( void )
    {
    return userErrorHandler;
    }


/******************************************************************************
 * PURPOSE: setErrorHandler - Sets a new user-specified error handler.
 * INPUTS:  ErrorHandler anErrorHandler  An error handler routine (or 0).
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void setErrorHandler ( ErrorHandler anErrorHandler )
    {
    userErrorHandler = anErrorHandler;
    }


/******************************************************************************
 * PURPOSE: errors - Returns the current error count - the total number
 *          of errors (failures) that have occurred since the process started.
 * INPUTS:  None
 * OUTPUTS: None
 * RETURNS: unsigned The current error count.
 * NOTES:
 *****************************************************************************/

unsigned errors ( void )
    {
    return errorCount;
    }


