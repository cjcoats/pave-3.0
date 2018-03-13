#ifndef ERROR_H
#define ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PURPOSE: Error.h - Declares routines for setting the global error handler.
 *
 * USAGE:
 *            #include <Error.h>
 *            static void yourErrorHandler( int errorNumber,
 *                                          const char* message )
 *            {
 *              ...
 *            }
 *            Handler oldHandler = getErrorHandler();
 *            setErrorHandler( yourErrorHandler );
 *            int* p = NEW( int, 100000000 );
 *            setErrorHandler( oldHandler );
 *
 * NOTES:
 *  Source-code control string:
 *  "$Id: Error.h 83 2018-03-12 19:24:33Z coats $"
 *
 * HISTORY: 
 *   Created 03/1993, Todd Plessel, EPA/MMTSI,
 *
 *   Version 11/1994, Todd Plessel, EPA/MMTSI, Modified for use as an example.
 * 
 *   Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 *****************************************************************************/

/*================================= TYPES ===================================*/

typedef void (*ErrorHandler)( int errorNumber, const char* message );

/*=============================== FUNCTIONS =================================*/

extern void error( const char* message, ... );
extern ErrorHandler getErrorHandler( void );
extern void setErrorHandler( ErrorHandler anErrorHandler );
extern unsigned errors( void );

#ifdef __cplusplus
}
#endif

#endif /* ERROR_H */
