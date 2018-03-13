/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: DataImport.c 83 2018-03-12 19:24:33Z coats $
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
 * PURPOSE: DataImport.c - Declares functions for reading and writing Models-3
 *          data files. This library is essentially a supplement to Carlie's
 *          libm3io to provide some convenience routines and work-around some
 *          of its deficiencies.
 * NOTES:
 * HISTORY: 04/1996, Todd Plessel, EPA/MMTSI, Created.
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include <stdio.h>      /* For sprintf().                                    */
#include <stddef.h>     /* For size_t.                                       */
#include <stdlib.h>     /* For putenv(), getenv().                           */
#include <string.h>     /* For strncpy(), strstr(), memset().                */
#include <ctype.h>      /* For isspace().                                    */
#include <math.h>       /* For log().                                        */
//#include <unistd.h>     /* For unlink().                                     */

#include "iodecl3.h"    /* init3c(), shut3c(), open3c(), close3c(), read3c().*/

#include "Assertions.h" /* For PRE(), POST(), CHECK(), DEBUG(), IN*().       */
#include "Error.h"      /* For error().                                      */
#include "Memory.h"     /* For NEW(), FREE().                                */
#include "File.h"       /* For readInts(), writeInts().                      */
#include "DataImport.h" /* For public functions.                             */

/*================================= MACROS  =================================*/

#ifndef MIN
#define MIN( a, b ) ((a) < (b) ? (a) : (b))
#define MAX( a, b ) ((a) > (b) ? (a) : (b))
#endif

/*=========================== PRIVATE VARIABLES =============================*/

static const char SVN_ID[] = "$Id: DataImport.c 83 2018-03-12 19:24:33Z coats $";

static int initialized = 0; /* Is libm3io initialized? */

/* Storage for environment variable assignment strings: */

enum { MAX_ASSIGNMENTS = 1024, MAX_ASSIGNMENT_LENGTH = 300 };

static char assignments[ MAX_ASSIGNMENTS ][ MAX_ASSIGNMENT_LENGTH ];

/*========================== FORWARD DECLARATIONS ===========================*/

static void limitToMin ( int* x, int min )
    {
    if ( *x < min ) *x = min;
    }
static void limitToMax ( int* x, int max )
    {
    if ( *x > max ) *x = max;
    }

static void limitToRange ( int* x, int min, int max )
    {
    limitToMin ( x, min );
    limitToMax ( x, max );
    }

static void limit ( int* low, int* high, int min, int max )
    {
    PRE3 ( low, high, min <= max );

    limitToRange ( low,   min, max );
    limitToRange ( high, *low, max );

    POST2 ( IN_RANGE ( *low, min, *high ), IN_RANGE ( *high, *low, max ) );
    }

static int countInRange ( const int range[ 2 ] )
    {
    PRE2 ( range, range[ FIRST ] <= range[ LAST ] );
    return 1 + range[ LAST ] - range[ FIRST ];
    }


static int equal ( double a, double b, double tolerance )
    {
    PRE ( tolerance >= 0.0 );
    return a > b ? a - b <= tolerance : b - a <= tolerance;
    }

static int gammaIsZero ( double gamma )
    {
    return equal ( gamma, 0.0, 1e-5 );
    }

static int gammaIsPlusOrMinus90 ( double gamma )
    {
    return OR2 ( equal ( gamma, 90.0, 1e-5 ), equal ( gamma, -90.0, 1e-5 ) );
    }

static int warnIfNonZeroGamma ( double gamma )
    {
    /*
     * FIX: Note: gamma is assumed to be zero.
     * The Proj Library offers no support for non-zero gamma anyway...
     */

    if ( ! equal ( gamma, 0.0, 1e-5 ) )
        fprintf ( stderr, "\a\n\nWarning: non-zero gamma ignored.\n\n" );

    return 1;
    }

static int createM3IOLogFile ( void );

static int assignFile ( const char* physicalFileName, const char* mode,
                        char* logicalFileName );

static void unassignFile ( const char* logicalFileName );

static int checkAndFixM3IOFile ( M3IOFile* file );

static void setSubsetDimensions ( const int subset[][ 2 ], M3IOFile* file );

static void setSubsetGrid ( const int subset[][ 2 ], M3IOFile* file );

static void setStartDateAndTime ( int firstTimestep, M3IOFile* file );

static void setSubsetVariables ( const M3IOFile* inputFile,
                                 int numberOfVariables,
                                 const char* variableNames[],
                                 M3IOFile* outputFile );

static void printSubset ( const int subset[][2], const char* variableNames[] );

static void updateRange ( const void* data, size_t numberOfValues,
                          int initializeWithData, size_t variable, int vtype,
                          float* ranges );

static int computeRange ( const void* data, size_t numberOfValues, int vtype,
                          float* minimum, float* maximum );

#ifdef DEBUGGING
static void printIddata ( const Iddata* iddata, int numberOfValues,
                          const int vtype[] );
#endif /* DEBUGGING */


/*=========================== PUBLIC FUNCTIONS ==============================*/


/******************************************************************************
 * PURPOSE: readM3IOFileDescription - Get the description of a data file.
 * INPUTS:  const char* fileName  Name of an M3IO data file. "MET_CROSS_2".
 * OUTPUTS: M3IOFile* file   M3IOFile description structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   Calls initializeM3IO() (and finalizeM3IO())
 *          if ! isInitializedM3IO() already.
 *****************************************************************************/

int readM3IOFileDescription ( const char* fileName, M3IOFile* file )
    {
    PRE2 ( fileName, file );

    int ok = 0;
    const int wasInitialized = isInitializedM3IO();

    if ( OR2 ( wasInitialized, initializeM3IO() ) )
        {
        if ( openM3IOFileForReading ( fileName, file ) ) ok = closeM3IOFile ( file );

        ok = AND2 ( OR2 ( wasInitialized, finalizeM3IO() ), ok );
        }

    if ( ! ok ) error ( "Failed to get info on file '%s'.\n", fileName );

    POST ( IMPLIES ( ok, isValidM3IOFile ( file ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readM3IOFileDescriptionFromFile - Read the description of an M3IO
 *          data file from a file (or pipe). For use with openPipe() calls to
 *          remote processes.
 * INPUTS:  File*     file       The opened file (or pipe) to read from.
 * OUTPUTS: M3IOFile* m3ioFile   Initialized M3IOFile description structure.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

int readM3IOFileDescriptionFromFile ( File* file, M3IOFile* m3ioFile )
    {
    PRE2 ( isValidFile ( file ), m3ioFile );

    int ok = 1;

    /* Read Bdesc: */

    ok = AND2 ( ok, readDouble ( file, &m3ioFile->bdesc.p_alp ) );
    ok = AND2 ( ok, readDouble ( file, &m3ioFile->bdesc.p_bet ) );
    ok = AND2 ( ok, readDouble ( file, &m3ioFile->bdesc.p_gam ) );
    ok = AND2 ( ok, readDouble ( file, &m3ioFile->bdesc.xcent ) );
    ok = AND2 ( ok, readDouble ( file, &m3ioFile->bdesc.ycent ) );
    ok = AND2 ( ok, readDouble ( file, &m3ioFile->bdesc.xorig ) );
    ok = AND2 ( ok, readDouble ( file, &m3ioFile->bdesc.yorig ) );
    ok = AND2 ( ok, readDouble ( file, &m3ioFile->bdesc.xcell ) );
    ok = AND2 ( ok, readDouble ( file, &m3ioFile->bdesc.ycell ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.ftype ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.cdate ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.ctime ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.wdate ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.wtime ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.sdate ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.stime ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.tstep ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.mxrec ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.nvars ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.ncols ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.nrows ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.nlays ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.nthik ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.gdtyp ) );
    ok = AND2 ( ok, readInt (    file, &m3ioFile->bdesc.vgtyp ) );
    ok = AND2 ( ok, readFloat (  file, &m3ioFile->bdesc.vgtop ) );
    ok = AND2 ( ok, readFloats ( file,  m3ioFile->bdesc.vglvs, MXLAYS3 + 1 ) );
    ok = AND2 ( ok, readInts (   file,  m3ioFile->bdesc.vtype, MXVARS3 ) );

    /* Read Cdesc: */

    ok = AND2 ( ok, readChars ( file,  m3ioFile->cdesc.gdnam,
                                sizeof m3ioFile->cdesc.gdnam / sizeof ( char ) ) );

    ok = AND2 ( ok, readChars ( file,  m3ioFile->cdesc.upnam,
                                sizeof m3ioFile->cdesc.upnam / sizeof ( char ) ) );

    ok = AND2 ( ok, readChars ( file,  m3ioFile->cdesc.execn,
                                sizeof m3ioFile->cdesc.execn / sizeof ( char ) ) );

    ok = AND2 ( ok, readChars ( file,  m3ioFile->cdesc.fdesc[ 0 ],
                                sizeof m3ioFile->cdesc.fdesc / sizeof ( char ) ) );

    ok = AND2 ( ok, readChars ( file,  m3ioFile->cdesc.updsc[ 0 ],
                                sizeof m3ioFile->cdesc.updsc / sizeof ( char ) ) );

    ok = AND2 ( ok, readChars ( file,  m3ioFile->cdesc.vname[ 0 ],
                                sizeof m3ioFile->cdesc.vname / sizeof ( char ) ) );

    ok = AND2 ( ok, readChars ( file,  m3ioFile->cdesc.units[ 0 ],
                                sizeof m3ioFile->cdesc.units / sizeof ( char ) ) );

    ok = AND2 ( ok, readChars ( file,  m3ioFile->cdesc.vdesc[ 0 ],
                                sizeof m3ioFile->cdesc.vdesc / sizeof ( char ) ) );

    if ( ! ok )
        error ( "Failed to read description from file '%s'.\n", nameOfFile ( file ) );

    POST ( IMPLIES ( ok, isValidM3IOFile ( m3ioFile ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeM3IOFileDescriptionToFile - Write the description of an M3IO
 *          data file to a file (or pipe). For use with openPipe() calls to
 *          remote processes.
 * INPUTS:  File*           file      The opened file (or pipe) to write to.
 *          const M3IOFile* m3ioFile  The M3IOFile description to write.
 * OUTPUTS: File*           file      The updated file (or pipe) written to.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

int writeM3IOFileDescriptionToFile ( File* file, const M3IOFile* m3ioFile )
    {
    PRE2 ( isValidFile ( file ), isValidM3IOFile ( m3ioFile ) );

    int ok = 1;

    /* Write Bdesc: */

    ok = AND2 ( ok, writeDouble ( file, m3ioFile->bdesc.p_alp ) );
    ok = AND2 ( ok, writeDouble ( file, m3ioFile->bdesc.p_bet ) );
    ok = AND2 ( ok, writeDouble ( file, m3ioFile->bdesc.p_gam ) );
    ok = AND2 ( ok, writeDouble ( file, m3ioFile->bdesc.xcent ) );
    ok = AND2 ( ok, writeDouble ( file, m3ioFile->bdesc.ycent ) );
    ok = AND2 ( ok, writeDouble ( file, m3ioFile->bdesc.xorig ) );
    ok = AND2 ( ok, writeDouble ( file, m3ioFile->bdesc.yorig ) );
    ok = AND2 ( ok, writeDouble ( file, m3ioFile->bdesc.xcell ) );
    ok = AND2 ( ok, writeDouble ( file, m3ioFile->bdesc.ycell ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.ftype ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.cdate ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.ctime ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.wdate ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.wtime ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.sdate ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.stime ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.tstep ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.mxrec ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.nvars ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.ncols ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.nrows ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.nlays ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.nthik ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.gdtyp ) );
    ok = AND2 ( ok, writeInt (    file, m3ioFile->bdesc.vgtyp ) );
    ok = AND2 ( ok, writeFloat (  file, m3ioFile->bdesc.vgtop ) );
    ok = AND2 ( ok, writeFloats ( file, m3ioFile->bdesc.vglvs, MXLAYS3 + 1 ) );
    ok = AND2 ( ok, writeInts (   file, m3ioFile->bdesc.vtype, MXVARS3 ) );

    /* Write Cdesc: */

    ok = AND2 ( ok, writeChars ( file, m3ioFile->cdesc.gdnam,
                                 sizeof m3ioFile->cdesc.gdnam / sizeof ( char ) ) );

    ok = AND2 ( ok, writeChars ( file, m3ioFile->cdesc.upnam,
                                 sizeof m3ioFile->cdesc.upnam / sizeof ( char ) ) );

    ok = AND2 ( ok, writeChars ( file, m3ioFile->cdesc.execn,
                                 sizeof m3ioFile->cdesc.execn / sizeof ( char ) ) );

    ok = AND2 ( ok, writeChars ( file, m3ioFile->cdesc.fdesc[ 0 ],
                                 sizeof m3ioFile->cdesc.fdesc / sizeof ( char ) ) );

    ok = AND2 ( ok, writeChars ( file, m3ioFile->cdesc.updsc[ 0 ],
                                 sizeof m3ioFile->cdesc.updsc / sizeof ( char ) ) );

    ok = AND2 ( ok, writeChars ( file, m3ioFile->cdesc.vname[ 0 ],
                                 sizeof m3ioFile->cdesc.vname / sizeof ( char ) ) );

    ok = AND2 ( ok, writeChars ( file, m3ioFile->cdesc.units[ 0 ],
                                 sizeof m3ioFile->cdesc.units / sizeof ( char ) ) );

    ok = AND2 ( ok, writeChars ( file, m3ioFile->cdesc.vdesc[ 0 ],
                                 sizeof m3ioFile->cdesc.vdesc / sizeof ( char ) ) );

    if ( ! ok )
        error ( "Failed to write description to file '%s'.\n", nameOfFile ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: initializeM3IO - Initialize the M3IO library.
 * INPUTS:  None
 * OUTPUTS: None
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   This function is not multi-thread-safe since it calls init3c().
 *          If M3IO initialization fails then error() is called.
 *          This function also specifies that M3IO headers be checked during
 *          I/O operations and hides all messages generated by M3IO.
 *****************************************************************************/

int initializeM3IO ( void )
    {
    PRE ( ! initialized );

    int ok = 0;

    ok = putenv ( "IOAPI_CHECK_HEADERS=Y" ) == 0; /* Make M3IO check headers! */

    /* Hide all M3IO-generated messages: */

    ok = AND2 ( ok, putenv ( "LOGFILE=/dev/null" ) == 0 );

    /* TEMP HACK: work-around until Carlie delivers updated M3IO for HP. */
#ifdef __hpux
    ok = AND2 ( ok, createM3IOLogFile() ); /* Force usage of ~/.M3IOLOGFILE */
#else
    DEBUG ( ok = AND2 ( ok, createM3IOLogFile() ); ) /* Unless debugging. */
#endif

    initialized = AND2 ( ok, init3c() ); /* Initialize libm3io. */

    if ( ! initialized ) error ( "Failed to initialize libm3io." );

    return initialized;
    }


/******************************************************************************
 * PURPOSE: finalizeM3IO - Required to finalize libm3io.
 * INPUTS:  None
 * OUTPUTS: None
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   This function is not multi-thread-safe since it calls shut3c().
 *          If libm3io finalization fails then error() is called.
 *****************************************************************************/

int finalizeM3IO ( void )
    {
    PRE ( initialized );

    if ( ! shut3c() ) error ( "Failed to finalize libm3io." );

    else initialized = 0;

    return ! initialized;
    }


/******************************************************************************
 * PURPOSE: isInitializedM3IO - Determine if libm3io is initialized.
 * INPUTS:  None
 * OUTPUTS: None
 * RETURNS: int 1 if initialized, else 0.
 * NOTES:
 *****************************************************************************/

int isInitializedM3IO ( void )
    {
    return initialized;
    }


/******************************************************************************
 * PURPOSE: openM3IOFileForReading - Open a Models-3 file for reading.
 * INPUTS:  const char* fileName  Name of the Models-3 file to open.
 * OUTPUTS: M3IOFile* file        Initialized structure.
 * RETURNS: 1 if successful, else 0.
 * NOTES:   If a failure occurs, error() is called.
 *****************************************************************************/

int openM3IOFileForReading ( const char* fileName, M3IOFile* file )
    {
    PRE3 ( isInitializedM3IO(), fileName, file );

    int ok = 0;

    memset ( file, 0, sizeof ( M3IOFile ) );

    strncpy ( file->fileName, fileName, 256 );
    file->fileName[ 256 ] = '\0';

    if ( assignFile ( file->fileName, "r", file->logicalFileName ) )
        {
        if ( open3c ( file->logicalFileName, 0, 0, FSREAD3, "" ) )
            {
            ok = desc3c ( file->logicalFileName, &file->bdesc, &file->cdesc );
            ok = AND2 ( ok, checkAndFixM3IOFile ( file ) );

            if ( ! ok )
                {
                unassignFile ( file->logicalFileName );
                close3c ( file->logicalFileName );
                }
            }
        else unassignFile ( file->logicalFileName );
        }

    if ( ! ok )
        {
        memset ( file, 0, sizeof ( M3IOFile ) );
        error ( "Failed to open file '%s' for reading.", fileName );
        }

    POST ( IMPLIES ( ok, isValidM3IOFile ( file ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: openM3IOFileForWriting - Open a Models-3 file for writing.
 * INPUTS:  const char* fileName     The name of the file to create/open.
 *          M3IOFile* file           The M3IOFile structure to write.
 * OUTPUTS: None
 * RETURNS: 1 if successful, else 0.
 * NOTES:   If a failure occurs, error() is called.
 *****************************************************************************/

int openM3IOFileForWriting ( const char* fileName, M3IOFile* file )
    {
    PRE6 ( isInitializedM3IO(), fileName, *fileName, file,
           isValidIOAPI_Bdesc3 ( &file->bdesc ),
           isValidIOAPI_Cdesc3 ( &file->cdesc, file->bdesc.nvars ) );

    int ok = 0;

    strncpy ( file->fileName, fileName, 256 );
    file->fileName[ 256 ] = '\0';


    if ( assignFile ( file->fileName, "w", file->logicalFileName ) )
        {
        unlink ( file->fileName ); /* HACK: M3IO BUG: Must remove existing file! */

        ok = open3c ( file->logicalFileName, &file->bdesc, &file->cdesc, FSCREA3,"" );

        if ( ! ok ) unassignFile ( file->logicalFileName );
        }

    if ( ! ok ) error ( "Failed to open file '%s' for writing.", file->fileName );

    return ok;
    }


/******************************************************************************
 * PURPOSE: closeM3IOFile - Close a Models-3 file.
 * INPUTS:  M3IOFile* file  M3IOFile to close.
 * OUTPUTS: None
 * RETURNS: 1 if successful, else 0.
 * NOTES:   If a failure occurs, error() is called.
 *****************************************************************************/

int closeM3IOFile ( M3IOFile* file )
    {
    PRE2 ( isInitializedM3IO(), isValidM3IOFile ( file ) );

    const int ok = close3c ( file->logicalFileName );

    unassignFile ( file->logicalFileName );

    file->logicalFileName[ 0 ] = '\0';

    POST ( file->logicalFileName[ 0 ] == '\0' );

    return ok;
    }


/******************************************************************************
 * PURPOSE: isValidM3IOFile - Verify a M3IOFile structure.
 * INPUTS:  const M3IOFile* file  M3IOFile file structure to verify.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:
 *****************************************************************************/

int isValidM3IOFile ( const M3IOFile* file )
    {
    return AND3 ( file,
                  isValidIOAPI_Bdesc3 ( &file->bdesc ),
                  isValidIOAPI_Cdesc3 ( &file->cdesc, file->bdesc.nvars ) );
    }


/******************************************************************************
 * PURPOSE: isValidIOAPI_Bdesc3 - Verify a IOAPI_Bdesc3 structure.
 * INPUTS:  const IOAPI_Bdesc3* bdesc  Bdesc structure to verify.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   FIX: M3IO Library should provide this!
 *****************************************************************************/

int isValidIOAPI_Bdesc3 ( const IOAPI_Bdesc3* bdesc )
    {
    int ok = AND2 ( bdesc != 0, bdesc->nvars > 0 );
    int v;

    for ( v = 0; AND2 ( ok, v < bdesc->nvars ); ++v )
        {
        ok = IN3 ( bdesc->vtype[ v ], M3INT, M3REAL );
        }

    return AND19 ( ok,
                   IN4 ( bdesc->ftype, GRDDED3, PTRFLY3, IDDATA3 ),
                   GT_ZERO8 ( bdesc->mxrec, bdesc->nvars, bdesc->nlays,
                              bdesc->nrows, bdesc->ncols, bdesc->sdate,
                              bdesc->xcell, bdesc->ycell ),
                   ( bdesc->tstep == 0 ) || isValidDate ( bdesc->sdate ),
                   isValidTime ( bdesc->stime ),
                   isValidTimestepSize ( bdesc->tstep ),
                   IMPLIES ( bdesc->mxrec > 1, bdesc->tstep > 0   ),
                   IN12 ( bdesc->gdtyp, LATGRD3, LAMGRD3, ALBGRD3, MERGRD3, STEGRD3, UTMGRD3, POLGRD3, EQMGRD3, TRMGRD3, LEQGRD3, IMISS3 ),
                   IMPLIES ( bdesc->gdtyp == LATGRD3,
                             AND6 ( IN_RANGE ( bdesc->xorig, -180.0, 180.0 ),
                                    IN_RANGE ( bdesc->yorig,  -90.0,  90.0 ),
                                    IN_RANGE ( bdesc->xcell,    0.0, 360.0 ),
                                    IN_RANGE ( bdesc->ycell,    0.0, 180.0 ),
                                    IN_RANGE ( bdesc->xorig +
                                            bdesc->ncols *
                                            bdesc->xcell, -180.0, 540.0 ),
                                    IN_RANGE ( bdesc->yorig +
                                            bdesc->nrows *
                                            bdesc->ycell,  -90.0,  90.0 ) ) ),
                   IMPLIES ( bdesc->gdtyp == LAMGRD3,
                             AND6 ( IN_RANGE ( bdesc->p_alp,  -90.0,  90.0 ),
                                    IN_RANGE ( bdesc->p_bet,  -90.0,  90.0 ),
                                    IN_RANGE ( bdesc->p_gam, -180.0, 180.0 ),
                                    IN_RANGE ( bdesc->xcent, -180.0, 180.0 ),
                                    IN_RANGE ( bdesc->ycent,  -90.0,  90.0 ),
                                    bdesc->p_alp <= bdesc->p_bet ) ),
                   IMPLIES ( bdesc->gdtyp == ALBGRD3,
                             AND6 ( IN_RANGE ( bdesc->p_alp,  -90.0,  90.0 ),
                                    IN_RANGE ( bdesc->p_bet,  -90.0,  90.0 ),
                                    IN_RANGE ( bdesc->p_gam, -180.0, 180.0 ),
                                    IN_RANGE ( bdesc->xcent, -180.0, 180.0 ),
                                    IN_RANGE ( bdesc->ycent,  -90.0,  90.0 ),
                                    bdesc->p_alp <= bdesc->p_bet ) ),
                   IMPLIES ( IN3 ( bdesc->gdtyp, MERGRD3, STEGRD3 ),
                             AND3 ( IN_RANGE ( bdesc->p_alp,  -90.0,  90.0 ),
                                    IN_RANGE ( bdesc->p_bet, -180.0, 180.0 ),
                                    IN_RANGE ( bdesc->p_gam,  -90.0,  90.0 ) ) ),
                   IMPLIES ( bdesc->gdtyp == POLGRD3,
                             AND3 ( IN_RANGE ( bdesc->p_alp, -1.0, 1.0 ),
                                    IN_RANGE ( bdesc->p_bet, -90.0, 90.0 ),
                                    IN_RANGE ( bdesc->p_gam, -180.0, 180.0 ) ) ),
                   IMPLIES ( bdesc->gdtyp == TRMGRD3,
                             AND3 ( IN_RANGE ( bdesc->p_alp, -90.0, 90.0 ),
                                    IN_RANGE ( bdesc->p_bet, 0.0, 1.0 ),
                                    IN_RANGE ( bdesc->p_gam, -180.0, 180.0 ) ) ),
                   IMPLIES ( bdesc->gdtyp == EQMGRD3,
                             AND2 ( IN_RANGE ( bdesc->p_alp, -90.0, 90.0 ),
                                    IN_RANGE ( bdesc->p_gam, -180.0, 180.0 ) ) ),
                   IMPLIES ( bdesc->gdtyp == LEQGRD3,
                             AND2 ( IN_RANGE ( bdesc->p_alp, -90.0, 90.0 ),
                                    IN_RANGE ( bdesc->p_gam, -180.0, 180.0 ) ) ),
                   IMPLIES ( bdesc->gdtyp == UTMGRD3,
                             IN_RANGE ( bdesc->p_alp, 1.0, 60.0 ) ),
                   IN10 ( bdesc->vgtyp,
                          VGSGPH3, VGSGPN3, VGSIGZ3, VGPRES3, VGZVAL3, VGHVAL3, VGWRFEM, VGWRFNM,
                          IMISS3 ),
                   IMPLIES ( bdesc->vgtyp == IMISS3, bdesc->nlays == 1 ) );
    }


/******************************************************************************
 * PURPOSE: isValidIOAPI_Cdesc3 - Verify a IOAPI_Cdesc3 structure.
 * INPUTS:  const IOAPI_Cdesc3* cdesc  Cdesc structure to verify.
 *          int nvars                  Number of variables.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   FIX: M3IO Library should provide this!
 *****************************************************************************/

int isValidIOAPI_Cdesc3 ( const IOAPI_Cdesc3* cdesc, int nvars )
    {
    int ok = AND2 ( cdesc != 0, nvars > 0 );
    int v;
    char blank[ NAMLEN3 + 1 ];

    memset ( blank, ' ', NAMLEN3 * sizeof ( char ) );
    blank[ NAMLEN3 ] = '\0';

    for ( v = 0; AND2 ( ok, v < nvars ); ++v )
        {
        ok = AND4 ( cdesc->vname[ v ][ 0 ],
                    cdesc->units[ v ][ 0 ],
                    strncmp ( cdesc->vname[ v ], blank, NAMLEN3 ),
                    strncmp ( cdesc->units[ v ], blank, NAMLEN3 ) );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: checkM3IOFile - Check a M3IOFile structure and report each invalid
 *          condition.
 * INPUTS:  const M3IOFile* file  M3IOFile file structure to check.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   If invalid then error() is called for each invalid condition.
 *****************************************************************************/

int checkM3IOFile ( const M3IOFile* file )
    {
    return AND3 ( file,
                  checkIOAPI_Bdesc3 ( &file->bdesc ),
                  checkIOAPI_Cdesc3 ( &file->cdesc, file->bdesc.nvars ) );
    }


/******************************************************************************
 * PURPOSE: checkIOAPI_Bdesc3 - Verify a IOAPI_Bdesc3 structure.
 * INPUTS:  const IOAPI_Bdesc3* bdesc  Bdesc structure to check.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   If invalid then error() is called for each invalid condition.
 *****************************************************************************/

int checkIOAPI_Bdesc3 ( const IOAPI_Bdesc3* bdesc )
    {
    int ok = 0;
    const unsigned int errorCount = errors();

    if ( bdesc )
        {
        int v;

        for ( v = 0; v < bdesc->nvars; ++v )
            {
            if ( ! IN3 ( bdesc->vtype[ v ], M3INT, M3REAL ) )
                {
                error ( "Variable #%d has an invalid (or unsupported) vtype: %d",
                        v + 1, bdesc->vtype[ v ] );
                }
            }

        if ( ! IN4 ( bdesc->ftype, GRDDED3, PTRFLY3, IDDATA3 ) )
            {
            error ( "Invalid ftype (%d)", bdesc->ftype );
            }

        if ( bdesc->mxrec <= 0 )
            {
            error ( "Invalid mxrec (%d)", bdesc->mxrec );
            }

        if ( bdesc->nvars <= 0 )
            {
            error ( "Invalid nvars (%d)", bdesc->nvars );
            }

        if ( bdesc->nlays <= 0 )
            {
            error ( "Invalid nlays (%d)", bdesc->nlays );
            }

        if ( bdesc->nrows <= 0 )
            {
            error ( "Invalid nrows (%d)", bdesc->nrows );
            }

        if ( bdesc->ncols <= 0 )
            {
            error ( "Invalid ncols (%d)", bdesc->ncols );
            }

        if ( bdesc->xcell <= 0.0 )
            {
            error ( "Invalid xcell (%f)", bdesc->xcell );
            }

        if ( bdesc->ycell <= 0.0 )
            {
            error ( "Invalid ycell (%f)", bdesc->ycell );
            }

        if ( bdesc->tstep && ( ! isValidDate ( bdesc->sdate ) ) )
            {
            error ( "Invalid sdate (%d)", bdesc->sdate );
            }

        if ( ! isValidTime ( bdesc->stime ) )
            {
            error ( "Invalid stime (%d)", bdesc->stime );
            }

        if ( ! isValidTimestepSize ( bdesc->tstep ) )
            {
            error ( "Invalid tstep (%d)", bdesc->tstep );
            }

        if ( ! IMPLIES ( bdesc->mxrec > 1, bdesc->tstep > 0 ) )
            {
            error ( "Invalid tstep (%d)", bdesc->tstep );
            }

        switch ( bdesc->gdtyp )
            {
            case LATGRD3:

                if ( ! IN_RANGE ( bdesc->xorig, -180.0, 180.0 ) )
                    {
                    error ( "Invalid xorig (%f)", bdesc->xorig );
                    }

                if ( ! IN_RANGE ( bdesc->yorig,  -90.0,  90.0 ) )
                    {
                    error ( "Invalid yorig (%f)", bdesc->yorig );
                    }

                if ( ! IN_RANGE ( bdesc->xcell,    0.0, 360.0 ) )
                    {
                    error ( "Invalid xcell (%f)", bdesc->xcell );
                    }

                if ( ! IN_RANGE ( bdesc->ycell,    0.0, 180.0 ) )
                    {
                    error ( "Invalid ycell (%f)", bdesc->ycell );
                    }

                if ( ! IN_RANGE ( bdesc->xorig + bdesc->ncols * bdesc->xcell,
                                  -180.0, 540.0 ) )
                    {
                    error ( "Invalid xorig (%f) + ncols (%d) * xcell (%f) = %f",
                            bdesc->xorig, bdesc->ncols, bdesc->xcell,
                            bdesc->xorig + bdesc->ncols * bdesc->xcell );
                    }

                if ( ! IN_RANGE ( bdesc->yorig + bdesc->nrows * bdesc->ycell,
                                  -90.0, 90.0 ) )
                    {
                    error ( "Invalid yorig (%f) + nrows (%d) * ycell (%f) = %f",
                            bdesc->yorig, bdesc->nrows, bdesc->ycell,
                            bdesc->yorig + bdesc->nrows * bdesc->ycell );
                    }

                break;

            case LAMGRD3:
            case ALBGRD3:

                if ( ! IN_RANGE ( bdesc->p_alp,  -90.0,  90.0 ) )
                    {
                    error ( "Invalid p_alp (%f)", bdesc->p_alp );
                    }

                if ( ! IN_RANGE ( bdesc->p_bet,  -90.0,  90.0 ) )
                    {
                    error ( "Invalid p_bet (%f)", bdesc->p_bet );
                    }

                if ( ! IN_RANGE ( bdesc->p_gam, -180.0, 180.0 ) )
                    {
                    error ( "Invalid p_gam (%f)", bdesc->p_gam );
                    }

                if ( ! IN_RANGE ( bdesc->xcent, -180.0, 180.0 ) )
                    {
                    error ( "Invalid xcent (%f)", bdesc->xcent );
                    }

                if ( ! IN_RANGE ( bdesc->ycent,  -90.0,  90.0 ) )
                    {
                    error ( "Invalid ycent (%f)", bdesc->ycent );
                    }

                if ( bdesc->p_alp > bdesc->p_bet )
                    {
                    error ( "Invalid p_alp (%f) > p_bet (%f)", bdesc->p_alp, bdesc->p_bet );
                    }

                break;

            case MERGRD3:
            case STEGRD3:

                if ( ! IN_RANGE ( bdesc->p_alp,  -90.0,  90.0 ) )
                    {
                    error ( "Invalid p_alp (%f)", bdesc->p_alp );
                    }

                if ( ! IN_RANGE ( bdesc->p_bet, -180.0, 180.0 ) )
                    {
                    error ( "Invalid p_bet (%f)", bdesc->p_bet );
                    }

                if ( ! IN_RANGE ( bdesc->p_gam,  -90.0,  90.0 ) )
                    {
                    error ( "Invalid p_gam (%f)", bdesc->p_gam );
                    }

                break;

            case UTMGRD3:

                if ( ! IN_RANGE ( bdesc->p_alp, 1.0, 60.0 ) )
                    {
                    error ( "Invalid p_alp (%f)", bdesc->p_alp );
                    }

                break;

            case POLGRD3:

                if ( ! IN_RANGE ( bdesc->p_alp, -1.0, 1.0 ) )
                    {
                    error ( "Invalid p_alp (%f)", bdesc->p_alp );
                    }

                if ( ! IN_RANGE ( bdesc->p_bet, -90.0, 90.0 ) )
                    {
                    error ( "Invalid p_bet (%f)", bdesc->p_bet );
                    }

                if ( ! IN_RANGE ( bdesc->p_gam,  -180.0,  180.0 ) )
                    {
                    error ( "Invalid p_gam (%f)", bdesc->p_gam );
                    }

                break;

            default:

                error ( "Invalid gdtyp (%d)", bdesc->gdtyp );
                break;
            }

        if ( ! IN10 ( bdesc->vgtyp,
                      VGSGPH3, VGSGPN3, VGSIGZ3, VGPRES3, VGZVAL3, VGHVAL3, VGWRFEM, VGWRFNM, IMISS3 ) )
            {
            error ( "Invalid vgtyp (%d)", bdesc->vgtyp );
            }

        /*
        *   BOGUS !! -- CJC

        if ( ! IMPLIES( bdesc->vgtyp == IMISS3, bdesc->nlays == 1 ) )
        {
          error( "Invalid vgtyp (%d) for nlays (%d) > 1",
                 bdesc->vgtyp, bdesc->nlays );
        }

        */
        }

    ok = errors() == errorCount;

    return ok;
    }


/******************************************************************************
 * PURPOSE: checkIOAPI_Cdesc3 - Check a IOAPI_Cdesc3 structure and report each
 *          invalid condition.
 * INPUTS:  const IOAPI_Cdesc3* cdesc  Cdesc structure to check.
 *          int nvars                  Number of variables.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   If invalid then error() is called for each invalid condition.
 *****************************************************************************/

int checkIOAPI_Cdesc3 ( const IOAPI_Cdesc3* cdesc, int nvars )
    {
    int ok = AND2 ( cdesc != 0, nvars > 0 );

    if ( ok )
        {
        int v;
        char blank[ NAMLEN3 + 1 ];

        memset ( blank, ' ', NAMLEN3 * sizeof ( char ) );
        blank[ NAMLEN3 ] = '\0';

        for ( v = 0; v < nvars; ++v )
            {
            if ( OR2 ( ! cdesc->vname[ v ][ 0 ],
                       ! strncmp ( cdesc->vname[ v ], blank, NAMLEN3 ) ) )
                {
                error ( "Varaiable #%d has no name." );
                }

            if ( OR2 ( ! cdesc->units[ v ][ 0 ],
                       ! strncmp ( cdesc->units[ v ], blank, NAMLEN3 ) ) )
                {
                error ( "Varaiable #%d has no units." );
                }
            }
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: readM3IOVariable - Reads from an open Models-3 file a variable of
 *          data at a timestep.
 * INPUTS:  const M3IOFile* file       The Models-3 file to read from.
 *          int timestep               The timestep to read.
 *          int variable               The variable to read.
 * OUTPUTS: void* data                 Buffer to fill. data[L][R][C]
 * RETURNS: 1 if successful, else 0.
 * NOTES:   If a failure occurs, error() is called.
 *****************************************************************************/

int readM3IOVariable ( const M3IOFile* file, int timestep, int variable,
                       void* data )
    {
    PRE5 ( isInitializedM3IO(), isValidM3IOFile ( file ),
           IN_RANGE ( variable, 0, file->bdesc.nvars - 1 ),
           IN_RANGE ( timestep, 0, file->bdesc.mxrec - 1 ), data );

    int ok;
    int jdate, jtime; /* Julian date and time of timestep. */
    char variableName[ NAMLEN3 + 1 ]; /* Variable name without blanks. */

    getM3IODateTime ( file->bdesc.sdate, file->bdesc.stime, file->bdesc.tstep,
                      timestep, &jdate, &jtime );

    compressString ( file->cdesc.vname[ variable ], NAMLEN3, variableName );

    ok = read3c ( file->logicalFileName, variableName, ALLAYS3, jdate, jtime,data );

    if ( ! ok )
        {
        error ( "Failed to read data from Models-3 file '%s', variable = '%s', "
                "timestep = %d.", file->fileName, variableName, timestep + 1 );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: readM3IOVariables - Reads from an open Models-3 file all variables
 *          of data at a timestep.
 * INPUTS:  const M3IOFile* file       The Models-3 file to read from.
 *          int timestep               The timestep to read.
 * OUTPUTS: void* data                 Buffer to fill. data[L][R][C]
 * RETURNS: 1 if successful, else 0.
 * NOTES:   If a failure occurs, error() is called.
 *****************************************************************************/

int readM3IOVariables ( const M3IOFile* file, int timestep, void* data )
    {
    PRE4 ( isInitializedM3IO(), isValidM3IOFile ( file ),
           IN_RANGE ( timestep, 0, file->bdesc.mxrec - 1 ), data );

    int ok;
    int jdate, jtime; /* Julian date and time of timestep. */

    getM3IODateTime ( file->bdesc.sdate, file->bdesc.stime, file->bdesc.tstep,
                      timestep, &jdate, &jtime );

    ok = read3c ( file->logicalFileName, ALLVAR3, ALLAYS3, jdate, jtime, data );

    if ( ! ok )
        {
        error ( "Failed to read data from Models-3 file '%s', timestep = '%d'.",
                file->fileName, timestep + 1 );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeM3IOVariable - Writes to an open Models-3 file a variable of
 *          data at a timestep.
 * INPUTS:  const M3IOFile* file       The Models-3 file to write to.
 *          int timestep               The timestep to write.
 *          int variable               The variable to write.
 *          const void* data           Buffer to write. data[L][R][C].
 * OUTPUTS: None
 * RETURNS: 1 if successful, else 0.
 * NOTES:   If a failure occurs, error() is called.
 *****************************************************************************/

int writeM3IOVariable ( const M3IOFile* file, int timestep, int variable,
                        const void* data )
    {
    PRE5 ( isInitializedM3IO(), isValidM3IOFile ( file ),
           IN_RANGE ( variable, 0, file->bdesc.nvars - 1 ),
           IN_RANGE ( timestep, 0, file->bdesc.mxrec - 1 ), data );

    int ok;
    int jdate, jtime; /* Julian date and time of timestep. */
    char variableName[ NAMLEN3 + 1 ]; /* Variable name without blanks. */

    getM3IODateTime ( file->bdesc.sdate, file->bdesc.stime, file->bdesc.tstep,
                      timestep, &jdate, &jtime );

    compressString ( file->cdesc.vname[ variable ], NAMLEN3, variableName );

    ok = write3c ( file->logicalFileName, variableName, jdate, jtime, data );

    if ( ! ok )
        error ( "Failed to write data to Models-3 file '%s', variable = '%s', "
                "timestep = %d.", file->fileName, variableName, timestep + 1 );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeM3IOVariables - Writes to an open Models-3 file all data
 *          variables for a given timestep.
 * INPUTS:  const M3IOFile* file       The Models-3 file to write to.
 *          int timestep               The timestep to write.
 *          const void* data           Buffer to write. data[V][L][R][C].
 * OUTPUTS: None
 * RETURNS: 1 if successful, else 0.
 * NOTES:   If a failure occurs, error() is called.
 *****************************************************************************/

int writeM3IOVariables ( const M3IOFile* file, int timestep, const void* data )
    {
    PRE4 ( isInitializedM3IO(), isValidM3IOFile ( file ),
           IN_RANGE ( timestep, 0, file->bdesc.mxrec - 1 ), data );

    int ok;
    int jdate, jtime; /* Julian date and time of timestep. */

    getM3IODateTime ( file->bdesc.sdate, file->bdesc.stime, file->bdesc.tstep,
                      timestep, &jdate, &jtime );

    ok = write3c ( file->logicalFileName, ALLVAR3, jdate, jtime, data );

    if ( ! ok )
        {
        error ( "Failed to write data to Models-3 file '%s', timestep = %d.",
                file->fileName, timestep + 1 );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: readM3IOSubset - Reads a subset of data from an open Models-3 file.
 * INPUTS:  const M3IOFile* file       The Models-3 file to read from.
 *          const int subset[ NUMBER_OF_DATA_DIMENSIONS ][ 2 ] Subset spec.
 *          const char* variableNames[] Optional: The selected variable names.
 * OUTPUTS: void* data                Buffer to fill. data[T][V][L][R][C].
 * RETURNS: 1 if successful, else 0.
 * NOTES:   If a failure occurs, error() is called.
 *****************************************************************************/

int readM3IOSubset ( const M3IOFile* file, const int subset[][ 2 ],
                     const char* variableNames[], void* data )
    {
    PRE4 ( isInitializedM3IO(), isValidM3IOFile ( file ),
           isValidSubsetOfM3IOFile ( file, ( const int ( * ) [2] ) subset,
                                     variableNames ), data );

    int ok = 1;
    int timestep, variable = 0; /* Current timestep and variable being read. */
    int jdate, jtime;       /* Julian date and time of the current timestep. */
    size_t chunk = 0;                 /* Which chunk are we reading?         */

    /* Reads occur in chunks of this size: */
    const size_t chunkSize = countInRange ( subset[ LAYER  ] ) *
                             countInRange ( subset[ ROW    ] ) *
                             countInRange ( subset[ COLUMN ] );

    getM3IODateTime ( file->bdesc.sdate, file->bdesc.stime, file->bdesc.tstep,
                      subset[ TIMESTEP ][ FIRST ], &jdate, &jtime );

    for (       timestep  = subset[ TIMESTEP ][ FIRST ];
                AND2 ( timestep <= subset[ TIMESTEP ][ LAST  ], ok ); ++timestep )
        {
        for ( variable = 0; AND2 ( ok, variable < subset[ VARIABLE ][ COUNT ] );
                ++variable )
            {
            DEBUG ( printf ( "timestep = %d, variable = '%s'\n",
                             timestep, variableNames[ variable ] ); )

            if ( file->bdesc.vtype[ variable ] == M3INT )
                {
                int* idata = data;

                if ( file->bdesc.ftype == GRDDED3 )
                    {
                    ok = xtract3c ( file->logicalFileName, variableNames[ variable ],
                                    1 + subset[ LAYER  ][ FIRST ],
                                    1 + subset[ LAYER  ][ LAST  ],
                                    1 + subset[ ROW    ][ FIRST ],
                                    1 + subset[ ROW    ][ LAST  ],
                                    1 + subset[ COLUMN ][ FIRST ],
                                    1 + subset[ COLUMN ][ LAST  ],
                                    jdate, jtime,
                                    idata + chunk * chunkSize );
                    }
                else
                    {
                    ok = read3c ( file->logicalFileName, variableNames[ variable ],
                                  ALLAYS3, jdate, jtime, idata + chunk * chunkSize );
                    }
                }
            else if ( file->bdesc.vtype[ variable ] == M3REAL )
                {
                float* fdata = data;

                if ( file->bdesc.ftype == GRDDED3 )
                    {
                    ok = xtract3c ( file->logicalFileName, variableNames[ variable ],
                                    1 + subset[ LAYER  ][ FIRST ],
                                    1 + subset[ LAYER  ][ LAST  ],
                                    1 + subset[ ROW    ][ FIRST ],
                                    1 + subset[ ROW    ][ LAST  ],
                                    1 + subset[ COLUMN ][ FIRST ],
                                    1 + subset[ COLUMN ][ LAST  ],
                                    jdate, jtime,
                                    fdata + chunk * chunkSize );
                    }
                else
                    {
                    ok = read3c ( file->logicalFileName, variableNames[ variable ],
                                  ALLAYS3, jdate, jtime, fdata + chunk * chunkSize );
                    }
                }

            ++chunk;
            }

        nextimec ( &jdate, &jtime, file->bdesc.tstep );
        }

    if ( ! ok )
        {
        error ( "Failed to read subset data from Models-3 file '%s',\n"
                "  timestep = %d, variable = '%s',\n"
                "  layers   = %d through %d,\n"
                "  rows     = %d through %d,\n"
                "  columns  = %d through %d.\n",
                file->fileName, timestep, variableNames[ variable - 1 ],
                1 + subset[ LAYER  ][ FIRST ],  1 + subset[ LAYER  ][ LAST ],
                1 + subset[ ROW    ][ FIRST ],  1 + subset[ ROW    ][ LAST ],
                1 + subset[ COLUMN ][ FIRST ],  1 + subset[ COLUMN ][ LAST ] );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: sizeOfM3IOSubset - Compute the size (# of values) in a subset.
 * INPUTS:  const int subset[ NUMBER_OF_DATA_DIMENSIONS ][ 2 ] Subset spec.
 * OUTPUTS: None
 * RETURNS: size_t  The number of values in the subset.
 * NOTES:
 *****************************************************************************/

size_t sizeOfM3IOSubset ( const int subset[][ 2 ] )
    {
    PRE5 ( subset[ TIMESTEP ][ FIRST ] <= subset[ TIMESTEP ][ LAST ],
           subset[ LAYER    ][ FIRST ] <= subset[ LAYER    ][ LAST ],
           subset[ ROW      ][ FIRST ] <= subset[ ROW      ][ LAST ],
           subset[ COLUMN   ][ FIRST ] <= subset[ COLUMN   ][ LAST ],
           subset[ VARIABLE ][ COUNT ] >= 0 );

    return subset[ VARIABLE ][ COUNT ] *
           countInRange ( subset[ TIMESTEP ] ) *
           countInRange ( subset[ LAYER    ] ) *
           countInRange ( subset[ ROW      ] ) *
           countInRange ( subset[ COLUMN   ] );
    }


/******************************************************************************
 * PURPOSE: isValidSubsetOfM3IOFile - Determine if the subset is a valid
 *          subrange of an M3IOFile.
 * INPUTS:  const M3IOFile* inputFile   The input M3IO file.
 *          const int subset[ NUMBER_OF_DATA_DIMENSIONS ][ 2 ] Subset spec.
 *          const char* variableNames[] Optional: The selected variable names.
 * OUTPUTS: None
 * RETURNS: int 1 if subset is valid, else 0.
 * NOTES:
 *****************************************************************************/

int isValidSubsetOfM3IOFile ( const M3IOFile* file,
                              const int subset[][ 2 ],
                              const char* variableNames[] )
    {
    PRE2 ( isValidM3IOFile ( file ), subset );

    int ok = 0;

    ok = AND14 ( IN_RANGE ( subset[TIMESTEP][FIRST], 0, file->bdesc.mxrec - 1 ),
                 IN_RANGE ( subset[TIMESTEP][LAST ], 0, file->bdesc.mxrec - 1 ),
                 IN_RANGE ( subset[VARIABLE][COUNT], 1, file->bdesc.nvars     ),
                 IN_RANGE ( subset[LAYER   ][FIRST], 0, file->bdesc.nlays - 1 ),
                 IN_RANGE ( subset[LAYER   ][LAST ], 0, file->bdesc.nlays - 1 ),
                 IN_RANGE ( subset[ROW     ][FIRST], 0, file->bdesc.nrows - 1 ),
                 IN_RANGE ( subset[ROW     ][LAST ], 0, file->bdesc.nrows - 1 ),
                 IN_RANGE ( subset[COLUMN  ][FIRST], 0, file->bdesc.ncols - 1 ),
                 IN_RANGE ( subset[COLUMN  ][LAST ], 0, file->bdesc.ncols - 1 ),
                 subset[ TIMESTEP ][ FIRST ] <= subset[ TIMESTEP ][ LAST ],
                 subset[ LAYER    ][ FIRST ] <= subset[ LAYER    ][ LAST ],
                 subset[ ROW      ][ FIRST ] <= subset[ ROW      ][ LAST ],
                 subset[ COLUMN   ][ FIRST ] <= subset[ COLUMN   ][ LAST ],
                 IMPLIES ( file->bdesc.ftype != GRDDED3,
                           AND3 ( countInRange ( subset[ LAYER  ] ) ==
                                  file->bdesc.nlays,
                                  countInRange ( subset[ ROW    ] ) ==
                                  file->bdesc.nrows,
                                  countInRange ( subset[ COLUMN ] ) ==
                                  file->bdesc.ncols ) ) );

    if ( variableNames )
        {
        int variable;

        for ( variable = 0; AND2 ( ok, variable < subset[ VARIABLE ][ COUNT ] );
                ++variable )
            {
            const int whichVariable =
                numberOfM3IOVariable ( file, variableNames[ variable ] );

            ok = whichVariable != -1;
            }
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: adjustM3IOSubset - Adjust a specified subset to within the valid
 *          range.
 * INPUTS:  const M3IOFile* file                     An M3IO file structure.
 * OUTPUTS: int subset[ NUMBER_OF_DATA_DIMENSIONS ][ 2 ]  Subsets to adjust.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void adjustM3IOSubset ( const M3IOFile* file, int subset[][ 2 ] )
    {
    PRE3 ( file, subset, isValidM3IOFile ( file ) );

    limit ( &subset[ TIMESTEP ][ FIRST ],
            &subset[ TIMESTEP ][ LAST  ], 0, file->bdesc.mxrec - 1 );

    limit ( &subset[ VARIABLE ][ COUNT ],
            &subset[ VARIABLE ][ COUNT ], 1, file->bdesc.nvars     );

    limit ( &subset[ LAYER    ][ FIRST ],
            &subset[ LAYER    ][ LAST  ], 0, file->bdesc.nlays - 1 );

    limit ( &subset[ ROW      ][ FIRST ],
            &subset[ ROW      ][ LAST  ], 0, file->bdesc.nrows - 1 );

    limit ( &subset[ COLUMN   ][ FIRST ],
            &subset[ COLUMN   ][ LAST  ], 0, file->bdesc.ncols - 1 );

    DEBUG ( printSubset ( ( const int ( * ) [ 2 ] ) subset, 0 ); )

    POST2 ( IN_RANGE ( subset[ VARIABLE ][ COUNT ], 1, file->bdesc.nvars ),
            isValidSubsetOfM3IOFile ( file, ( const int ( * ) [2] ) subset, 0 ) );
    }


/******************************************************************************
 * PURPOSE: createSubsetOfM3IOFile - Initialize an output file description
 *          based on an input file description and subset specifications.
 * INPUTS:  const M3IOFile* inputFile   The input M3IO file.
 *          const int subset[ NUMBER_OF_DATA_DIMENSIONS ][ 2 ] Subset spec.
 *          const char* variableNames[] Optional: The selected variable names.
 * OUTPUTS: M3IOFile* outputFile        The output M3IO file to describe.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void createSubsetOfM3IOFile ( const M3IOFile* inputFile,
                              const int subset[][ 2 ],
                              const char* variableNames[],
                              M3IOFile* outputFile )
    {
    PRE4 ( isValidM3IOFile ( inputFile ), subset, outputFile,
           isValidSubsetOfM3IOFile ( inputFile, ( const int ( * ) [2] ) subset,
                                     variableNames ) );

    /* Begin with a copy of the input description: */

#if 0
    /* TEMP HACK: to work-around HP BUG: doesn't allow const structure copying!! */
    outputFile->bdesc = inputFile->bdesc;
    outputFile->cdesc = inputFile->cdesc;
#else
    memcpy ( &outputFile->bdesc, &inputFile->bdesc, sizeof ( IOAPI_Bdesc3 ) );
    memcpy ( &outputFile->cdesc, &inputFile->cdesc, sizeof ( IOAPI_Cdesc3 ) );
#endif

    /* Then reduce it to the specified subset: */

    setStartDateAndTime ( subset[ TIMESTEP ][ FIRST ], outputFile );
    setSubsetDimensions ( ( const int ( * ) [2] ) subset,   outputFile );
    setSubsetGrid (       ( const int ( * ) [2] ) subset,   outputFile );

    if ( variableNames )
        setSubsetVariables ( inputFile, subset[ VARIABLE ][ COUNT ], variableNames,
                             outputFile );

    DEBUG ( printf ( "\ninputFile:\n" );
            printM3IOFileDescription ( inputFile );
            printf ( "\noutputFile:\n" );
            printM3IOFileDescription ( outputFile ); )

    POST2 ( isValidIOAPI_Bdesc3 ( &outputFile->bdesc ),
            isValidIOAPI_Cdesc3 ( &outputFile->cdesc, outputFile->bdesc.nvars ) );
    }


/******************************************************************************
 * PURPOSE: isM3IOCrossGrid - Determine if a given M3IOFile is a 'cross grid'.
 * INPUTS:  const M3IOFile* file      The structure with grid description.
 * OUTPUTS: None
 * RETURNS: int 1 if it is a 'cross grid', else 0.
 * NOTES:
 *****************************************************************************/

int isM3IOCrossGrid ( const M3IOFile* file )
    {
    char gridName[ NAMLEN3 + 1 ];

    compressString ( file->cdesc.gdnam, NAMLEN3, gridName );

    return strstr ( gridName, "_CRO" ) != 0;
    }


/******************************************************************************
 * PURPOSE: numberOfM3IOVariable - Get the number of the named variable.
 * INPUTS:  const M3IOFile* file      The M3IOFile structure to scan.
 *          const char* variableName  The name of the variable to search for.
 * OUTPUTS: None
 * RETURNS: int the number of the named variable in the range [0, nvars - 1]
 *          or -1 if not found.
 * NOTES:
 *****************************************************************************/

int numberOfM3IOVariable ( const M3IOFile* file, const char* variableName )
    {
    PRE2 ( isValidM3IOFile ( file ), variableName );

    int variable;
    int found = 0;

    for ( variable = 0; AND2 ( ! found, variable < file->bdesc.nvars );
            ++variable )
        {
        char vname[ NAMLEN3 + 1 ];

        compressString ( file->cdesc.vname[ variable ], NAMLEN3, vname );

        found = strcmp ( variableName, vname ) == 0;
        }

    if ( ! found ) variable = 0;

    --variable;

    POST2 ( IMPLIES (   found, IN_RANGE ( variable, 0, file->bdesc.nvars - 1 ) ),
            IMPLIES ( ! found, variable == -1 ) );

    return variable;
    }


/******************************************************************************
 * PURPOSE: nameOfM3IOVariable - Get the name of the numbered variable.
 * INPUTS:  const M3IOFile* file  The M3IOFile structure to read.
 *          int variableNumber    The number of the variable to name.
 * OUTPUTS: None
 * RETURNS: const char* Name of numbered variable.
 * NOTES:
 *****************************************************************************/

const char* nameOfM3IOVariable ( const M3IOFile* file, int variableNumber )
    {
    PRE2 ( isValidM3IOFile ( file ),
           IN_RANGE ( variableNumber, 0, file->bdesc.nvars - 1 ) );

    static char vname[ NAMLEN3 + 1 ];

    compressString ( file->cdesc.vname[ variableNumber ], NAMLEN3, vname );

    return vname;
    }


/******************************************************************************
 * PURPOSE: getM3IODateTime - Compute a future date and time from a given
 *          starting date, starting time, timestep size and timestep number.
 * INPUTS:  int startDate  Date in yyyyddd format when timestep was 0.
 *          int startTime  Time in hhmmss  format when timestep was 0.
 *          int timestepSize  Increment size per timestep in *hhmmss.
 *          int timestepNumber  The timestep number to compute the
 *                                       date and time for [0, ...].
 * OUTPUTS: int* theDate         Date in yyyyddd format for the given timestep.
 *          int* theTime         Time in hhmmss  format for the given timestep.
 * RETURNS: None
 * NOTES:   timestepSize hh component may be greater than 23.
 *****************************************************************************/

void getM3IODateTime ( int startDate, int startTime, int timestepSize,
                       int timestepNumber, int* theDate, int* theTime )
    {
    PRE6 ( ( ! timestepSize ) || isValidDate ( startDate ), isValidTime ( startTime ),
           isValidTimestepSize ( timestepSize ), timestepNumber >= 0, theDate,
           theTime );

    *theDate = startDate;
    *theTime = startTime;

    if ( timestepNumber > 0 )
        {
        const int totalTimeInSecondsOfTimestepSize = time2secc ( timestepSize );

        const int totalTimeInSecondsFromStart = totalTimeInSecondsOfTimestepSize *
                                                timestepNumber;

#if 0
        const int totalTimeFromStart = sec2timec ( totalTimeInSecondsFromStart );
#else
        const int totalTimeFromStart = totalTimeInSecondsFromStart % 60 +
                                       100 * ( totalTimeInSecondsFromStart / 60 % 60
                                               + 100 *
                                               ( totalTimeInSecondsFromStart / 3600 ) );
#endif

        DEBUG ( printf ( "startDate = %d, startTime = %d, timestepSize = %d, "
                         "timestepNumber = %d\n",
                         startDate, startTime, timestepSize, timestepNumber );

                printf ( "totalTimeInSecondsOfTimestepSize = %d\n",
                         totalTimeInSecondsOfTimestepSize );

                printf ( "totalTimeInSecondsFromStart = %d\n",
                         totalTimeInSecondsFromStart );

                printf ( "totalTimeFromStart = %d\n", totalTimeFromStart );

                printf ( "before: *theDate = %d, *theTime = %d\n",
                         *theDate, *theTime ); )

        nextimec ( theDate, theTime, totalTimeFromStart );

        DEBUG ( printf ( "after: *theDate = %d, *theTime = %d\n",
                         *theDate, *theTime ); )
        }

    POST2 ( ( ! timestepSize ) || isValidDate ( *theDate ), isValidTime ( *theTime ) );
    }


/******************************************************************************
 * PURPOSE: getM3IODateTimeString - Compute the date and time string of a
 *          timestep.
 * INPUTS:  const M3IOFile* file  The structure to examine.
 *          int timestep          The timestep to compute at.
 *          size_t bufferSize     The size of the buffer.
 * OUTPUTS: char* buffer          The buffer to write to.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void getM3IODateTimeString ( const M3IOFile* file, int timestep,
                             size_t bufferSize, char* buffer )
    {
    PRE5 ( isValidM3IOFile ( file ),
           IN_RANGE ( timestep, 0, file->bdesc.mxrec - 1 ),
           bufferSize, buffer, bufferSize >= 2 * NAMLEN3 );

    int theDate, theTime;

    memset ( buffer, 0, bufferSize * sizeof ( char ) );

    if ( ! timestep )
        {
        getM3IODateTime ( file->bdesc.sdate, file->bdesc.stime, file->bdesc.tstep,
                          timestep, &theDate, &theTime );

        mmddyyc ( theDate, buffer );
        buffer[ NAMLEN3 - 1 ] = '\0'; /* BUG in M3IO Library? */
        strcat ( buffer, "  " );
        hhmmssc ( theTime, buffer + strlen ( buffer ) );
        strcat ( buffer, " GMT" );
        }
    }


/******************************************************************************
 * PURPOSE: isValidDate - Determine if a date is valid.
 * INPUTS:  int yyyyddd  The Date to check in *yyyyddd format.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   The yyyy component may be greater than 9999.
 *****************************************************************************/

int isValidDate ( int yyyyddd )
    {
    const int ddd = yyyyddd % 1000;
    return IN_RANGE ( ddd, 1, 366 );
    }


/******************************************************************************
 * PURPOSE: isValidTime - Determine if a time is valid.
 * INPUTS:  int hhmmss  Time in hhmmss format.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   The hh component may not be greater than 23.
 *****************************************************************************/

int isValidTime ( int hhmmss )
    {
    const int hh = hhmmss / 10000;

    return AND2 ( isValidTimestepSize ( hhmmss ), IN_RANGE ( hh, 0, 23 ) );
    }


/******************************************************************************
 * PURPOSE: isValidTimestepSize - Determine if a timestep size is valid.
 * INPUTS:  int hhmmss  Time in *hhmmss format.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   The hh component may be greater than 23.
 *****************************************************************************/

int isValidTimestepSize ( int hhmmss )
    {
    const int hh     = hhmmss  / 10000;
    const int hh0000 = hh      * 10000;
    const int mmss   = hhmmss - hh0000;
    const int mm     = mmss / 100;
    const int mm00   = mm   * 100;
    const int ss     = mmss - mm00;

    return AND3 ( hh >= 0, IN_RANGE ( mm, 0, 59 ), IN_RANGE ( ss, 0, 59 ) );
    }


/******************************************************************************
 * PURPOSE: compressString - Copy a string while removing all whitespace.
 * INPUTS:  const char* src  The source string to copy.
 *          size_t      n    Maximum # of characters to copy (excluding '\0').
 * OUTPUTS: char*       dst  Copy of src without any whitespace.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void compressString ( const char* src, size_t n, char* dst )
    {
    PRE3 ( dst, src, n );

    size_t i, j;

    for ( i = 0, j = 0; AND2 ( src[ i ], i < n ); ++i )
        if ( ! isspace ( src[ i ] ) ) dst[ j++ ] = src[ i ];

    for ( ; j <= n; ++j ) dst[ j ] = '\0';

    CHECK ( strlen ( dst ) <= n );
    }


/******************************************************************************
 * PURPOSE: expandString - Expand a string with blank padding.
 * INPUTS:  const char* src Source string.
 *          size_t size     Size of source string (including '\0').
 * OUTPUTS: char* dst       Blank-padded result.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void expandString ( const char* src, size_t size, char* dst )
    {
    PRE5 ( src, *src, size, dst, size >= strlen ( src ) );

    const size_t size_1 = size /* - 1 */;
    int i;

    for ( i = 0; AND2 ( src[ i ], i < size_1 ); ++i ) dst[ i ] = src[ i ];

    for ( ; i < size_1; ++i ) dst[ i ] = ' ';

    /* dst[ i ] = '\0'; */

    POST ( strncmp ( src, dst, strlen ( src ) ) == 0 );
    }


/******************************************************************************
 * PURPOSE: computeM3IOGridZ - Computes the grid Z coordinates in meters.
 * INPUTS:  const IOAPI_Bdesc3* bdesc  Structure with info on levels.
 *          int computeZAtLevels       Flag: compute Z coordinate _at_
 *                                     rather than _between_ the given levels.
 *                                     To associate the data with the cells,
 *                                     use 1 to obtain the cell boundaries.
 *                                     Note that this requires that gridZ[]
 *                                     be dimensioned for nlays + 1 values.
 *                                     Use 0 to obtain just the cell centers
 *                                     e.g., for use by Vis5d and other vertex-
 *                                     based systems.
 * OUTPUTS: double gridZ[]             The grid Z coordinates in meters.
 * RETURNS: None
 * NOTES:   The vertical level schemes defined by M3IO are:
 *            VGSGPH3  hydrostatic sigma-P
 *            VGSGPN3  non-h sigma-P
 *            VGSIGZ3  sigma-Z
 *            VGPRES3  pressure (pascals)
 *            VGZVAL3  Z (m) (above sea lvl)
 *            VGHVAL3  H (m) (above ground)
 *            IMISS3   None (becomes a single layer at 0.0).
 *****************************************************************************/

void computeM3IOGridZ ( const IOAPI_Bdesc3* bdesc, int computeZAtLevels,
                        double z[] )
    {
    PRE3 ( isValidIOAPI_Bdesc3 ( bdesc ), IN_RANGE ( computeZAtLevels, 0, 1 ), z );

    const int numberOfLevels = bdesc ? bdesc->nlays + computeZAtLevels: 0;
    const int vgtyp          = bdesc ? bdesc->vgtyp : 0;
    int level;

    for ( level = 0; level < numberOfLevels; ++level )
        {
        const double valueAtLevel = computeZAtLevels ? bdesc->vglvs[ level ]
                                    : ( bdesc->vglvs[ level     ] +
                                        bdesc->vglvs[ level + 1 ] ) / 2;
        const double HEIGHT_OF_TERRAIN_IN_METERS = 0.0; /* Not given in bdesc. */
        double pressure;

        switch ( vgtyp )
            {
            case VGSGPH3: /* hydrostatic sigma-P */
            case VGSGPN3: /* non-h sigma-P */

                pressure = pressureAtSigmaLevel ( valueAtLevel, bdesc->vgtop / 100.0 );
                z[ level ] = heightAtPressure ( pressure );
                break;

            case VGSIGZ3: /* sigma-Z */

                /* vgtop is in meters and valueAtLevel increases for each level. */

                z[ level ] = HEIGHT_OF_TERRAIN_IN_METERS +
                             valueAtLevel *
                             ( bdesc->vgtop - HEIGHT_OF_TERRAIN_IN_METERS );
                break;

            case VGPRES3: /* pressure (pascals) */

                z[ level ] = heightAtPressure ( valueAtLevel / 100.0 );
                break;

            case VGZVAL3: /* Z (m) (above sea lvl) */

                z[ level ] = valueAtLevel;
                break;

            case VGHVAL3: /* H (m) (above ground)  */

                z[ level ] = valueAtLevel + HEIGHT_OF_TERRAIN_IN_METERS;
                break;

            default:

                z[ level ] = level;
                break;
            }
        }
    }


/******************************************************************************
 * PURPOSE: pressureAtSigmaLevel - Compute pressure (in millibars) at a given
 *          sigma level.
 * INPUTS:  double sigmaLevel     Sigma level.
 * *        double pressureAtTop  Pressure (in millibars) at top of the model.
 * OUTPUTS: None
 * RETURNS: double pressure in millibars at the given sigma level.
 * NOTES:   Based on formula in the documentation for Vis5d by Bill Hibbard.
 *****************************************************************************/

double pressureAtSigmaLevel ( double sigmaLevel, double pressureAtTop )
    {
#define SURFACE_PRESSURE_IN_MB 1012.5
    return pressureAtTop + sigmaLevel * ( SURFACE_PRESSURE_IN_MB - pressureAtTop );
    }


/******************************************************************************
 * PURPOSE: heightAtPressure - Compute the height (in meters) at a given
 *          pressure (in millibars).
 * INPUTS:  double pressure   Pressure value in millibars.
 * OUTPUTS: None
 * RETURNS: double height in meters at the given pressure in millibars.
 * NOTES:   Based on formula in the documentation for Vis5d by Bill Hibbard.
 *****************************************************************************/

double heightAtPressure ( double pressure )
    {
    const double pressureToHeightScaleFactor = -7.2 * 1000.0;

    if ( pressure == 0.0 ) pressure = 1e-10; /* HACK: prevent core on non-IEEE.*/

    return pressureToHeightScaleFactor * log ( pressure / SURFACE_PRESSURE_IN_MB );
    }


/******************************************************************************
 * PURPOSE: printM3IOFileDescription - Print a description of an M3IO file.
 * INPUTS:  const M3IOFile* file  M3IOFile structure to print.
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void printM3IOFileDescription ( const M3IOFile* file )
    {
    PRE ( isValidM3IOFile ( file ) );

    const IOAPI_Bdesc3* bdesc = file ? &file->bdesc : 0;
    const IOAPI_Cdesc3* cdesc = file ? &file->cdesc : 0;
    int level, variable;

    printf ( "\nBdesc:\n" );
    printf ( "  p_alp = %f (first, second, third map)\n",           bdesc->p_bet );
    printf ( "  p_bet = %f (projection descriptive)\n",             bdesc->p_bet );
    printf ( "  p_gam = %f (projection parameters)\n",              bdesc->p_gam );
    printf ( "  xcent = %f (lon for coord-system X=0)\n",           bdesc->xcent );
    printf ( "  ycent = %f (lat for coord-system Y=0)\n",           bdesc->ycent );
    printf ( "  xorig = %f (X-coord origin of grid (map units))\n", bdesc->xorig );
    printf ( "  yorig = %f (Y-coord origin of grid (map units))\n", bdesc->yorig );
    printf ( "  xcell = %f (X-coordinate cell dimension)\n",        bdesc->xcell );
    printf ( "  ycell = %f (Y-coordinate cell dimension)\n",        bdesc->ycell );
    printf ( "  ftype = %d (file type)\n",                          bdesc->ftype );
    printf ( "  cdate = %d (creation date   YYYYDDD)\n",            bdesc->cdate );
    printf ( "  ctime = %d (creation time    HHMMSS)\n",            bdesc->ctime );
    printf ( "  wdate = %d (update date     YYYYDDD)\n",            bdesc->wdate );
    printf ( "  wtime = %d (update time      HHMMSS)\n",            bdesc->wtime );
    printf ( "  sdate = %d (file start date YYYYDDD)\n",            bdesc->sdate );
    printf ( "  stime = %d (file start time  HHMMSS)\n",            bdesc->stime );
    printf ( "  tstep = %d (file time step   HHMMSS)\n",            bdesc->tstep );
    printf ( "  mxrec = %d (max time step record number)\n",        bdesc->mxrec );
    printf ( "  nvars = %d (number of species)\n",                  bdesc->nvars );
    printf ( "  ncols = %d (number of grid columns)\n",             bdesc->ncols );
    printf ( "  nrows = %d (number of grid rows)\n",                bdesc->nrows );
    printf ( "  nlays = %d (number of layers)\n",                   bdesc->nlays );
    printf ( "  nthik = %d (perimeter thickness (cells))\n",        bdesc->nthik );
    printf ( "  gdtyp = %d (grid type: 1=LAT-LON, 2=LAM, ...)\n",   bdesc->gdtyp );
    printf ( "  vgtyp = %d (vertical coord type (VGSIGP3, ...))\n", bdesc->vgtyp );
    printf ( "  vgtop = %f (model-top, for sigma coord types)\n",   bdesc->vgtop );
    printf ( "  vglvs[ nlays ] (vertical coord values):\n" );

    for ( level = 0; level < bdesc->nlays + 1; ++level )
        printf ( "    %f\n", bdesc->vglvs[ level ] );

    printf ( "\n" );
    printf ( "  vtype[ nvars ] (variable types):\n" );

    for ( variable = 0; variable < bdesc->nvars; ++variable )
        printf ( "    %d\n", bdesc->vtype[ variable ] );

    printf ( "\n" );
    printf ( "\nCdesc:\n" );
    printf ( "  gdnam = '%s' (grid name)\n",                      cdesc->gdnam );
    printf ( "  upnam = '%s' (last program writing to file)\n",   cdesc->upnam );
    printf ( "  execn = '%s' (value of env vble EXECUTION_ID)\n", cdesc->execn );
    printf ( "  fdesc = '%s' (file description)\n",               cdesc->fdesc[0] );
    printf ( "  updsc = '%s' (update description)\n",             cdesc->updsc[0] );
    printf ( "  vname = '%s' (variable names)\n",                 cdesc->vname[0] );
    printf ( "  units = '%s' (variable units)\n",                 cdesc->units[0] );
    printf ( "  vdesc = '%s' (variable description)\n",           cdesc->vdesc[0] );
    printf ( "\n" );
    }


/******************************************************************************
 * PURPOSE: computeM3IORange - Computes the range of the specified variables
 *          over a subset of an open Models-3 file.
 * INPUTS:  const M3IOFile* file       The opened Models-3 file to read from.
 *          const int subset[ NUMBER_OF_DATA_DIMENSIONS ][ 2 ]
 *                                      Optional: Subset to consider.
 *                                      If 0 then read complete volume.
 *          const char* variableNames[] Optional: The selected variable names.
 *                                      If 0 then use all variables.
 * OUTPUTS: float ranges                Range of each specified variable.
 *                                      range[ variable * 2 + MINIMUM ]
 *                                      range[ variable * 2 + MAXIMUM ]
 * RETURNS: 1 if successful, else 0.
 * NOTES:   If a failure occurs, error() is called.
 *****************************************************************************/

int computeM3IORange ( const M3IOFile* file,
                       const int subset[][ 2 ],
                       const char* variableNames[],
                       float* ranges )
    {
    PRE4 ( isInitializedM3IO(), isValidM3IOFile ( file ), ranges,
           IMPLIES ( subset, isValidSubsetOfM3IOFile ( file,
                     ( const int ( * ) [2] ) subset,
                     variableNames ) ) );

    int     ok        = 0;
    float*  fdata     = 0;     /* The chunk of data read.             */
    int*    idata     = 0;     /* The chunk of data read.             */
    size_t  chunkSize = 0;     /* Reads occur in chunks of this size. */
    Iddata* iddata    = 0;     /* If reading IDDATA3 file.            */

    if ( file->bdesc.ftype == IDDATA3 )
        {
        iddata = allocateIddata ( file->bdesc.nrows,
                                  file->bdesc.nvars * file->bdesc.nlays );
        }
    else
        {
        chunkSize = subset ?
                    countInRange ( subset[ LAYER  ] ) *
                    countInRange ( subset[ ROW    ] ) *
                    countInRange ( subset[ COLUMN ] )
                    : file->bdesc.nlays * file->bdesc.nrows * file->bdesc.ncols;

        fdata = NEW ( float, chunkSize );
        idata = NEW ( int,   chunkSize );
        }

    if ( XOR2 ( AND2 ( idata, fdata ), iddata ) )
        {
        /* Timestep & variable are 0-based and layer, row & column are 1-based. */

        const int firstTimestep = subset ? subset[ TIMESTEP ][ FIRST ] : 0;
        const int firstLayer    = subset ? subset[ LAYER    ][ FIRST ] + 1 : 1;
        const int firstRow      = subset ? subset[ ROW      ][ FIRST ] + 1 : 1;
        const int firstColumn   = subset ? subset[ COLUMN   ][ FIRST ] + 1 : 1;

        const int lastTimestep  = subset ? subset[ TIMESTEP ][ LAST ]
                                  : file->bdesc.mxrec - 1;

        const int numberOfVariables = subset ? subset[ VARIABLE ][ COUNT ]
                                      : file->bdesc.nvars;

        const int lastLayer  = subset ? subset[ LAYER  ][ LAST ] + 1
                               : file->bdesc.nlays;

        const int lastRow    = subset ? subset[ ROW    ][ LAST ] + 1
                               : file->bdesc.nrows;

        const int lastColumn = subset ? subset[ COLUMN ][ LAST ] + 1
                               : file->bdesc.ncols;

        int timestep;     /* Current timestep being read.                  */
        int jdate, jtime; /* Julian date and time of the current timestep. */

        getM3IODateTime ( file->bdesc.sdate, file->bdesc.stime, file->bdesc.tstep,
                          firstTimestep, &jdate, &jtime );

        for ( ok = 1, timestep  = firstTimestep;
                AND2 (   timestep <= lastTimestep, ok ); ++timestep )
            {
            int variable;              /* Current variable being read. */
            char vname[ NAMLEN3 + 1 ]; /* Name of current variable.    */

            for ( variable = 0; AND2 ( ok, variable < numberOfVariables ); ++variable )
                {
                int m3variable = 0;
                int vtype = 0;
                void* data = 0;

                if ( variableNames )
                    {
                    strcpy ( vname, variableNames[ variable ] );
                    }
                else
                    {
                    compressString ( file->cdesc.vname[ variable ], NAMLEN3, vname );
                    }

                m3variable = numberOfM3IOVariable ( file, vname );

                CHECK ( m3variable != -1 );

                vtype = file->bdesc.vtype[ m3variable ];

                if ( vtype == M3INT ) data = idata;
                else                  data = fdata;

                DEBUG ( printf ( "timestep = %d, vname = '%s'\n", timestep, vname ); )

                if ( file->bdesc.ftype == GRDDED3 )
                    {
                    ok = xtract3c ( file->logicalFileName, vname,
                                    firstLayer,  lastLayer,
                                    firstRow,    lastRow,
                                    firstColumn, lastColumn,
                                    jdate, jtime, data );
                    }
                else if ( file->bdesc.ftype == IDDATA3 )
                    {
                    const int iddataChunkSize = file->bdesc.nrows * file->bdesc.nlays;
                    const int whichVariable   = numberOfM3IOVariable ( file, vname );

                    CHECK ( whichVariable != -1 );

                    if ( variable == 0 ) /* All variables are read in each timestep. */
                        {
                        ok = readM3IOIddata ( file, timestep, iddata );
                        chunkSize = *iddata->count * file->bdesc.nlays;
                        }

                    idata = iddata->ivariables + whichVariable * iddataChunkSize;
                    fdata = iddata->fvariables + whichVariable * iddataChunkSize;

                    if ( vtype == M3INT ) data = idata;
                    else                  data = fdata;

                    CHECK ( data );
                    }
                else
                    {
                    ok = read3c ( file->logicalFileName, vname, ALLAYS3, jdate,jtime,data );
                    }

                if ( AND2 ( ok, chunkSize ) )
                    {
                    const int initializeRange = timestep == firstTimestep;

                    updateRange ( data, chunkSize, initializeRange, variable, vtype,
                                  ranges );
                    }
                }

            nextimec ( &jdate, &jtime, file->bdesc.tstep );

            if ( ! ok )
                {
                error ( "Failed to read subset data from Models-3 file '%s',\n"
                        "  timestep = %d, variable = '%s',\n"
                        "  layers   = %d through %d,\n"
                        "  rows     = %d through %d,\n"
                        "  columns  = %d through %d.\n",
                        file->fileName, timestep, vname,
                        firstLayer,   lastLayer,
                        firstRow,     lastRow,
                        firstColumn,  lastColumn );
                }
            }

        if ( file->bdesc.ftype == IDDATA3 )  /* Avoid double-free. */
            {
            idata = 0;
            fdata = 0;
            }

        FREE ( iddata );
        FREE ( idata );
        FREE ( fdata );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: createM3IOIddataFile - Create an M3IO Iddata file.
 * INPUTS:  const char* fileName  Name of the M3IO Iddata file to create.
 *          int numberOfTimesteps Number of timesteps of new file.
 *          int numberOfStations  Total (maximum) number of (unique) stations.
 *          int numberOfVariables Number of variables in new file.
 *          const int   variableTypes[]  Types of variables: M3INT or M3REAL.
 *          const char* variableNames[]  Names of variables.
 *          const char* variableUnits[]  Units of variables.
 *          const float corners[ 2 ][ 2 ] corners[ LOWER | UPPER ][ LAT | LON ]
 * OUTPUTS: M3IOFile* file  Created M3IO Iddata file created.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

int createM3IOIddataFile ( const char* fileName,
                           int numberOfTimesteps,
                           int numberOfStations,
                           int numberOfVariables,
                           int startingDate,
                           int startingTime,
                           int timestepSize,
                           const int   variableTypes[],
                           const char* variableNames[],
                           const char* variableUnits[],
                           const char* variableDescriptions[],
                           const float corners[ 2 ][ 2 ],
                           M3IOFile* file )
    {
    PRE11 ( isInitializedM3IO(), fileName, *fileName,
            GT_ZERO3 ( numberOfTimesteps, numberOfStations, numberOfVariables ),
            variableTypes,
            variableNames, *variableNames[0], variableUnits, *variableUnits[0],
            isValidCorners ( corners ), file );

    int ok       = 0;
    int variable = 0;
    int line     = 0;

    memset ( file, 0, sizeof ( M3IOFile ) );

    file->bdesc.ftype = IDDATA3;
    file->bdesc.sdate = startingDate;
    file->bdesc.stime = startingTime;
    file->bdesc.tstep = timestepSize;
    file->bdesc.mxrec = numberOfTimesteps;
    file->bdesc.nvars = numberOfVariables;
    file->bdesc.ncols = 1;
    file->bdesc.nrows = numberOfStations;
    file->bdesc.nlays = 1;
    file->bdesc.nthik = 0;
    file->bdesc.gdtyp = LATGRD3;
    file->bdesc.p_alp = 0.0;
    file->bdesc.p_bet = 0.0;
    file->bdesc.p_gam = 0.0;
    file->bdesc.xcent = 0.0;
    file->bdesc.ycent = 0.0;
    file->bdesc.xorig = corners[ LOWER ][ LON ];
    file->bdesc.yorig = corners[ LOWER ][ LAT ];

    file->bdesc.xcell = ( corners[ UPPER ][ LON ] - corners[ LOWER ][ LON ] )
                        / ( float ) file->bdesc.ncols;

    file->bdesc.ycell = ( corners[ UPPER ][ LAT ] - corners[ LOWER ][ LAT ] )
                        / ( float ) file->bdesc.nrows;

    file->bdesc.vgtyp = IMISS3;
    file->bdesc.vgtop = IMISS3;
    file->bdesc.vglvs[ 0 ] = file->bdesc.vglvs[ 1 ] = IMISS3;

    for ( variable = 0; variable < numberOfVariables; ++variable )
        file->bdesc.vtype[ variable ] = variableTypes[ variable ];

    expandString ( "LATLON_US", sizeof file->cdesc.gdnam / sizeof ( char ) ,
                   file->cdesc.gdnam );

    file->cdesc.gdnam[ sizeof file->cdesc.gdnam / sizeof ( char ) - 1 ] = '\0';

    for ( line = 0; line < MXDESC3; ++line )
        {
        const char* const desc = line ? " "
                                 : "ROM Surface Meteorology (SURMET) data.";

        expandString ( desc,
                       sizeof file->cdesc.fdesc[ 0 ] / sizeof ( char ),
                       file->cdesc.fdesc[ line ] );
        }

    file->cdesc.fdesc[ MXDESC3 - 1 ][ MXDLEN3 - 1 ] = '\0';

    for ( variable = 0; variable < numberOfVariables; ++variable )
        {
        expandString ( variableNames[ variable ],
                       sizeof file->cdesc.vname[ variable ] / sizeof ( char ),
                       file->cdesc.vname[ variable ] );

        expandString ( variableUnits[ variable ],
                       sizeof file->cdesc.units[ variable ] / sizeof ( char ),
                       file->cdesc.units[ variable ] );

        expandString ( variableDescriptions[ variable ],
                       sizeof file->cdesc.vdesc[ variable ] / sizeof ( char ),
                       file->cdesc.vdesc[ variable ] );
        }

    file->cdesc.vname[ MXVARS3 - 1 ][ NAMLEN3 - 1 ] = '\0';
    file->cdesc.units[ MXVARS3 - 1 ][ NAMLEN3 - 1 ] = '\0';
    file->cdesc.vdesc[ MXVARS3 - 1 ][ NAMLEN3 - 1 ] = '\0';

    if ( checkM3IOFile ( file ) )
        {
        ok = openM3IOFileForWriting ( fileName, file );
        }

    if ( ! ok )
        error ( "Can't create M3IO Iddata file from invalid description." );

    POST ( IMPLIES ( ok, isValidM3IOFile ( file ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readM3IOIddata - Read one timestep-worth of data from an M3IO
 *          Iddata file.
 * INPUTS:  const M3IOFile* file  Opened M3IO Iddata file.
 *          int timestep          Number of the timestep to read.
 * OUTPUTS: Iddata* iddata        Iddata structure read.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

int readM3IOIddata ( const M3IOFile* file, int timestep, Iddata* iddata )
    {
    PRE7 ( isInitializedM3IO(), isValidM3IOFile ( file ),
           IN_RANGE ( timestep, 0, file->bdesc.mxrec - 1 ),
           iddata, iddata->ids, iddata->ivariables, iddata->fvariables );

    int ok = 0;
    int jdate, jtime; /* Julian date and time of timestep. */

    getM3IODateTime ( file->bdesc.sdate, file->bdesc.stime, file->bdesc.tstep,
                      timestep, &jdate, &jtime );

    DEBUG ( printIddata ( iddata, file->bdesc.nvars * file->bdesc.nlays,
                          file->bdesc.vtype ); )

    ok = read3c ( file->logicalFileName, ALLVAR3, ALLAYS3, jdate, jtime,
                  iddata->buffer );

    if ( ! ok )
        {
        error ( "Failed to read Iddata from Models-3 file '%s', timestep = %d.",
                file->fileName, timestep + 1 );
        }

    POST3 ( isValidM3IOFile ( file ), isValidIddata ( iddata ), *iddata->count >= 0 );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeM3IOIddata - Write one timestep-worth of data to an M3IO
 *          Iddata file.
 * INPUTS:  const M3IOFile* file  Opened M3IO Iddata file.
 *          int timestep          Number of the timestep to write.
 *          const Iddata* iddata  Iddata structure to write.
 * OUTPUTS: None
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

int writeM3IOIddata ( const M3IOFile* file, int timestep, const Iddata* iddata )
    {
    PRE4 ( isInitializedM3IO(), isValidM3IOFile ( file ),
           IN_RANGE ( timestep, 0, file->bdesc.mxrec - 1 ),
           isValidIddata ( iddata ) );

    int ok = 0;
    int jdate, jtime; /* Julian date and time of timestep. */

    getM3IODateTime ( file->bdesc.sdate, file->bdesc.stime, file->bdesc.tstep,
                      timestep, &jdate, &jtime );

    DEBUG ( printIddata ( iddata, file->bdesc.nvars * file->bdesc.nlays,
                          file->bdesc.vtype ); )

    ok = write3c ( file->logicalFileName, ALLVAR3, jdate, jtime, iddata->buffer );

    if ( ! ok )
        {
        error ( "Failed to write Iddata to Models-3 file '%s', timestep = %d.",
                file->fileName, timestep + 1 );
        }

    POST ( isValidM3IOFile ( file ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: isValidIddata - Verify an Iddata structure.
 * INPUTS:  const Iddata* iddata  Iddata structure to verify.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:
 *****************************************************************************/

int isValidIddata ( const Iddata* iddata )
    {
    return AND6 ( iddata,
                  iddata->count,
                  iddata->count == iddata->buffer,
                  *iddata->count >= 0,
                  iddata->ids == iddata->count + 1,
                  iddata->ivariables == ( const int* ) iddata->fvariables );
    }


/******************************************************************************
 * PURPOSE: allocateIddata - Allocate an Iddata structure.
 * INPUTS:  int numberOfStations  Total (maximum) number of (unique) stations.
 *          int numberOfVariables Number of variables of data.
 * OUTPUTS: None
 * RETURNS: Iddata* iddata  Allocated Iddata structure to deallocate.
 * NOTES:
 *****************************************************************************/

Iddata* allocateIddata ( int numberOfStations, int numberOfVariables )
    {
    PRE ( GT_ZERO2 ( numberOfStations, numberOfVariables ) );

    Iddata* iddata = 0;
    const size_t sizeOfCount     = 1;
    const size_t sizeOfIds       = numberOfStations;
    const size_t sizeOfVariables = numberOfStations * numberOfVariables;
    /* * nlays == 1 */
    const size_t sizeOfBuffer = sizeOfCount + sizeOfIds + sizeOfVariables;

    CHECK ( sizeof ( int ) == sizeof ( float ) );

    iddata = NEW ( Iddata, 1 );

    if ( iddata )
        {
        memset ( iddata, 0, sizeof ( Iddata ) );
        iddata->buffer = NEW ( int, sizeOfBuffer );

        if ( iddata->buffer )
            {
            memset ( iddata->buffer, 0, sizeOfBuffer * sizeof ( int ) );

            iddata->count      = iddata->buffer;
            iddata->ids        = iddata->count + 1;
            iddata->ivariables = iddata->ids + numberOfStations;
            iddata->fvariables = ( float* ) iddata->ivariables;

            DEBUG ( printf ( "allocated %d ids and %d variables\n",
                             numberOfStations, sizeOfVariables ); )
            }
        else FREE ( iddata );
        }

    POST ( IMPLIES ( iddata, AND2 ( *iddata->count == 0, isValidIddata ( iddata ) ) ) );

    return iddata;
    }


/******************************************************************************
 * PURPOSE: deallocateIddata - Deallocate an Iddata structure.
 * INPUTS:  Iddata* iddata  Allocated Iddata structure to deallocate.
 * OUTPUTS: Iddata* iddata  Deallocated and zeroed Iddata structure.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void deallocateIddata ( Iddata* iddata )
    {
    PRE ( isValidIddata ( iddata ) );

    if ( iddata->buffer ) *iddata->buffer = 0;

    FREE ( iddata->buffer );
    memset ( iddata, 0, sizeof ( Iddata ) );
    FREE ( iddata );

    POST ( iddata == 0 );
    }


/******************************************************************************
 * PURPOSE: isValidCorners - Verify that the domain corners are valid.
 * INPUTS:  const float corners[ 2 ][ 2 ]  [ LOWER | UPPER ][ LAT | LON ].
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   If not valid then error() is called.
 *****************************************************************************/

int isValidCorners ( const float corners[ 2 ][ 2 ] )
    {
    int ok;

    ok = AND5 ( corners,
                isValidLatLon ( corners[ LOWER ][ LAT ],
                                corners[ LOWER ][ LON ] ),
                isValidLatLon ( corners[ UPPER ][ LAT ],
                                corners[ UPPER ][ LON ] ),
                corners[ LOWER ][ LAT ] <= corners[ UPPER ][ LAT ],
                corners[ LOWER ][ LON ] <= corners[ UPPER ][ LON ] );

    return ok;
    }


/******************************************************************************
 * PURPOSE: isValidLatLon - Determine if the given latitude and
 *          longitude coordinates are valid.
 * INPUTS:  float latitude   The latitude  coordinate.
 *          float longitude  The longitude coordinate.
 * OUTPUTS: None
 * RETURNS: int 1 if valid else 0.
 * NOTES:
 *****************************************************************************/

int isValidLatLon ( float latitude, float longitude )
    {
    return AND2 ( IN_RANGE ( latitude,   -90.0,  90.0 ),
                  IN_RANGE ( longitude, -180.0, 180.0 ) );
    }


/*============================ PRIVATE FUNCTIONS ============================*/


/******************************************************************************
 * PURPOSE: createM3IOLogFile - Create a log file for holding messages from
 *          the M3IO Library routines.
 * INPUTS:  None
 * OUTPUTS: None
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   HACK: work-around to hide M3IO messages.
 *****************************************************************************/

static int createM3IOLogFile ( void )
    {
    int ok = 0;

    const char* homePath = getenv ( "HOME" );

    if ( homePath )
        {
        char logFileName[257];
        static char logFileAssignment[300];
        strncpy ( logFileName, homePath, 256 );
        logFileName[256] = '\0';
        strncat ( logFileName, "/.M3IOLOGFILE", 256 - strlen ( logFileName ) );
        logFileName[256] = '\0';
        unlink ( logFileName );
        sprintf ( logFileAssignment, "LOGFILE=%s", logFileName );
        ok = putenv ( logFileAssignment ) == 0;
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: assignFile - Assign a physical Models-3 file name to an environment
 *          variable denoting a logical file name as required by open3c()
 *          and other M3IO routines.
 * INPUTS:  const char* physicalFileName  The physical Models-3 file name.
 *          const char* mode  mode        The mode "r" or "w".
 * OUTPUTS: char* physicalFileName        The name of an environment variable
 *                                        denoting the logical file name.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   This is required to support the extra level of indirection used by
 *          the M3IO library. The 'logical' names are put in this process's
 *          environment (via putenv()) and are later read by open3c(), etc
 *          via getenv(). FIX: I'm still not convinced this is better than
 *          using simple pathed file names...
 *          If failure occurs (due to putenv() failure) then error() is called.
 *****************************************************************************/

static int assignFile ( const char* physicalFileName, const char* mode,
                        char* logicalFileName )
    {
    PRE4 ( physicalFileName, mode, logicalFileName, IN3 ( *mode, 'r', 'w' ) );

    int ok = 0;
    int assignment, found;

    logicalFileName[0] = '\0';

    for ( assignment = 0, found = 0;
            AND2 ( ! found, assignment < MAX_ASSIGNMENTS ) ; )
        {
        found = assignments[ assignment ][ 0 ] == '\0'; /* Found an empty slot. */

        if ( ! found ) ++assignment;
        }

    if ( found )
        {
        CHECK ( assignment < MAX_ASSIGNMENTS );

        sprintf ( logicalFileName, "M3IOFile%08d", assignment );
        sprintf ( assignments[ assignment ],
                  "%s=%s", logicalFileName, physicalFileName );

        CHECK ( strlen ( assignments[ assignment ] ) < ( size_t ) MAX_ASSIGNMENT_LENGTH );

        ok = putenv ( assignments[ assignment ] ) == 0;
        }

    if ( ! ok )
        {
        logicalFileName[0] = '\0';
        error ( "Unable to assign file '%s'.", physicalFileName );
        }

    POST ( IMPLIES ( ok, ! strcmp ( getenv ( logicalFileName ), physicalFileName ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: unassignFile - Removes an assignment made by assignFile().
 * INPUTS:  const char* logicalFileName  The logical Models-3 file name.
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void unassignFile ( const char* logicalFileName )
    {
    PRE ( logicalFileName );

    int assignment, found;
    const size_t length = logicalFileName ? strlen ( logicalFileName ) : 0;

    for ( assignment = 0, found = 0;
            AND2 ( ! found, assignment < MAX_ASSIGNMENTS ) ; )
        {
        found = strncmp ( assignments[ assignment ], logicalFileName, length ) == 0;

        if ( ! found ) ++assignment;
        }

    if ( found )
        {
        CHECK ( assignment < MAX_ASSIGNMENTS );

        sprintf ( assignments[ assignment ], "%s=", logicalFileName );

        CHECK ( ( int ) strlen ( assignments[ assignment ] ) < MAX_ASSIGNMENT_LENGTH );

        putenv ( assignments[ assignment ] );

        assignments[ assignment ][ 0 ] = '\0';
        }

    POST ( IMPLIES ( found, getenv ( logicalFileName ) == 0 ) );
    }


/******************************************************************************
 * PURPOSE: checkAndFixM3IOFile - Checks an M3IOFile structure for correctness
 *          and warns of and repairs some errors.
 * INPUTS:  M3IOFile* file   M3IOFile structure to check/fix.
 * OUTPUTS: M3IOFile* file   M3IOFile structure with fixes.
 * RETURNS: int 1 if file is/has been made valid, else 0.
 * NOTES:   HACK/FIX: M3IO Library should do this!
 *****************************************************************************/

static int checkAndFixM3IOFile ( M3IOFile* file )
    {
    PRE ( file );

    int ok;
    IOAPI_Bdesc3* bdesc = file ? &file->bdesc : 0;

    /* FIX: Add code to check cdesc too! */

    if ( AND2 ( ( bdesc->gdtyp == LAMGRD3 ) || ( bdesc->gdtyp == ALBGRD3 ),
                bdesc->p_alp > bdesc->p_bet ) )
        {
        const double temp = bdesc->p_alp;
        bdesc->p_alp      = bdesc->p_bet;
        bdesc->p_bet      = temp;

        fprintf ( stderr, "\a\n\nWarning: "
                  "p_alp and p_bet are reversed in the file."
                  " M3IO file should be fixed with 'm3edhdr'.\n\n" );
        }

    /*
     * According to the M3IO documentation, for non-lat-lon grids,
     * units for xorig, yorig, xcell, and ycell are supposed to be in meters.
     * However every sample data file encountered thus far has used kilometers.
     * Below is an attempt to detect and convert such bad units.
     * However, a problem with this work-around is that if it encounters a
     * high-resolution grid that actually defines sub-100 meter cells,
     * it will mistake them for 'bad kilometer units' and handle them as such.
     *
     * THIS IS BOGUS -- CJC (e.g., 30-m DEM or land-use

    if ( AND2( bdesc->gdtyp != LATGRD3, bdesc->xcell < 100.0 ) )
    {
      const double KILOMETERS_TO_METERS = 1000.0;

      fprintf( stderr, "\a\n\nWarning: "
                       "scaled probable kilometer units to meters."
                       " M3IO file should be fixed with 'm3edhdr'.\n\n" );

      bdesc->xorig *= KILOMETERS_TO_METERS;
      bdesc->yorig *= KILOMETERS_TO_METERS;
      bdesc->xcell *= KILOMETERS_TO_METERS;
      bdesc->ycell *= KILOMETERS_TO_METERS;
    }
     */

    /*
     * According to the revised M3IO documentation, units for vgtop and vglvls
     * (when vgtyp = VGPRES3) are supposed to be in pascals.
     * However some sample data files encountered have used millibars (the old
     * units) or kilopascals.
     * Below is an attempt to detect and convert such bad units.
     */

    if ( IN4 ( bdesc->vgtyp, VGSGPH3, VGSGPN3, VGPRES3 ) )
        {
        if ( bdesc->vgtop < 99.0 )
            {
            bdesc->vgtop *= 1000.0; /* Convert kilopascals to pascals. */

            fprintf ( stderr, "\a\n\nWarning: "
                      "scaled probable kilopascal units in vgtop to pascals."
                      " M3IO file should be fixed with 'm3edhdr'.\n\n" );
            }
        else if ( bdesc->vgtop < 2000.0 )
            {
            bdesc->vgtop *= 100.0; /* Convert millibars to pascals. */

            fprintf ( stderr, "\a\n\nWarning: "
                      "scaled probable millibars units in vgtop to pascals."
                      " M3IO file should be fixed with 'm3edhdr'.\n\n" );
            }

        if ( bdesc->vgtyp == VGPRES3 )
            {
            const int numberOfLevels = bdesc->nlays + 1;
            int level;

            for ( level = 0; level < numberOfLevels; ++level )
                {
                float* pressure = &bdesc->vglvs[ level ];

                if      ( *pressure <   99.0 ) *pressure *= 1000.0;
                else if ( *pressure < 2000.0 ) *pressure *= 100;
                }
            }
        }

    ok = isValidM3IOFile ( file );

    if ( ! ok )
        {
        checkM3IOFile ( file ); /* Prints more detailed information. */
        error ( "The M3IO file '%s' has an invalid bdesc header. Fix the file.\n",
                file->fileName );
        }

    POST ( IMPLIES ( ok, isValidM3IOFile ( file ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: setSubsetDimensions - Sets the dimensions of a M3IOFile structure
 *          to that specified by a given subset.
 * INPUTS:  const int subset[ NUMBER_OF_DATA_DIMENSIONS ][ 2 ] Subset spec.
 * OUTPUTS: M3IOFile* file  The re-dimensioned M3IOFile structure.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void setSubsetDimensions ( const int subset[][ 2 ], M3IOFile* file )
    {
    PRE2 ( subset, file );

    file->bdesc.mxrec = countInRange ( subset[ TIMESTEP ] );
    file->bdesc.nvars =               subset[ VARIABLE ][ COUNT ];
    file->bdesc.nlays = countInRange ( subset[ LAYER    ] );
    file->bdesc.nrows = countInRange ( subset[ ROW      ] );
    file->bdesc.ncols = countInRange ( subset[ COLUMN   ] );
    }


/******************************************************************************
 * PURPOSE: setSubsetGrid - Sets the grid xorig, yorig and vglvs to that
 *          specified by a given subset.
 * INPUTS:  const int subset[ NUMBER_OF_DATA_DIMENSIONS ][ 2 ] Subset spec.
 * OUTPUTS: M3IOFile* file  The re-gridded M3IOFile structure.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void setSubsetGrid ( const int subset[][ 2 ], M3IOFile* file )
    {
    int level;

    PRE2 ( subset, file );

    file->bdesc.xorig += subset[ COLUMN ][ FIRST ] * file->bdesc.xcell;
    file->bdesc.yorig += subset[ ROW    ][ FIRST ] * file->bdesc.ycell;

    for ( level  = subset[ LAYER ][ FIRST ];
            level <= subset[ LAYER ][ LAST  ] + 1; /* Copy one extra level. */
            ++level )
        {
        file->bdesc.vglvs[ level - subset[ LAYER ][ FIRST ] ] =
            file->bdesc.vglvs[ level ];
        }
    }


/******************************************************************************
 * PURPOSE: setStartDateAndTime - Sets the starting date and time to that
 *          specified by a given timestep.
 * INPUTS:  int firstTimestep     The new first timestep.
 * OUTPUTS: M3IOFile* outputFile  The re-dated M3IOFile structure.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void setStartDateAndTime ( int firstTimestep, M3IOFile* file )
    {
    int sdate, stime; /* New starting data and time in Julian-format. */

    PRE2 ( file, IN_RANGE ( firstTimestep, 0, file->bdesc.mxrec - 1 ) );

    getM3IODateTime ( file->bdesc.sdate, file->bdesc.stime, file->bdesc.tstep,
                      firstTimestep, &sdate, &stime );

    file->bdesc.sdate = sdate;
    file->bdesc.stime = stime;
    }


/******************************************************************************
 * PURPOSE: setSubsetVariables - Sets the variable info based on a subset.
 * INPUTS:  const M3IOFile* inputFile   M3IOFile with all variables.
 *          int numberOfVariables       No. of variables in variableNames[].
 *          const char* variableNames[] Selected variable names.
 * OUTPUTS: M3IOFile* outputFile        M3IOFile with subset of variables.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void setSubsetVariables ( const M3IOFile* inputFile,
                                 int numberOfVariables,
                                 const char* variableNames[],
                                 M3IOFile* outputFile )
    {
    int dst;

    PRE4 ( isValidM3IOFile ( inputFile ),
           IN_RANGE ( numberOfVariables, 1, inputFile->bdesc.nvars ),
           variableNames, outputFile );

    memset ( outputFile->cdesc.vname, 0, sizeof outputFile->cdesc.vname );
    memset ( outputFile->cdesc.units, 0, sizeof outputFile->cdesc.units );
    memset ( outputFile->cdesc.vdesc, 0, sizeof outputFile->cdesc.vdesc );
    memset ( outputFile->bdesc.vtype, 0, sizeof outputFile->bdesc.vtype );

    for ( dst = 0; dst < numberOfVariables; ++dst )
        {
        const int src = numberOfM3IOVariable ( inputFile, variableNames[ dst ] );

        CHECK ( src != -1 );

        strncpy ( outputFile->cdesc.vname[ dst ], inputFile->cdesc.vname[ src ],
                  NAMLEN3 );

        strncpy ( outputFile->cdesc.units[ dst ], inputFile->cdesc.units[ src ],
                  NAMLEN3 );

        strncpy ( outputFile->cdesc.vdesc[ dst ], inputFile->cdesc.vdesc[ src ],
                  MXDLEN3 );

        outputFile->bdesc.vtype[ dst ] = inputFile->bdesc.vtype[ src ];
        }
    }


/******************************************************************************
 * PURPOSE: printSubset - Prints the subset values.
 * INPUTS:  const int subset[ NUMBER_OF_DATA_DIMENSIONS ][ 2 ] Subset spec.
 *          const char* variableNames[] Optional: The selected variable names.
 * OUTPUTS: M3IOFile* outputFile        M3IOFile with subset of variables.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void printSubset ( const int subset[][ 2 ], const char* variableNames[] )
    {
    PRE ( subset );

    printf ( "\n" );

    printf ( "subset[ TIMESTEP ] = [ %d %d ]\n",
             subset[ TIMESTEP ][ FIRST ],
             subset[ TIMESTEP ][ LAST  ] );

    printf ( "subset[ VARIABLE ][ COUNT ] = %d\n",
             subset[ VARIABLE ][ COUNT ] );

    printf ( "subset[ LAYER    ] = [ %d %d ]\n",
             subset[ LAYER    ][ FIRST ],
             subset[ LAYER    ][ LAST  ] );

    printf ( "subset[ ROW      ] = [ %d %d ]\n",
             subset[ ROW      ][ FIRST ],
             subset[ ROW      ][ LAST  ] );

    printf ( "subset[ COLUMN   ] = [ %d %d ]\n",
             subset[ COLUMN   ][ FIRST ],
             subset[ COLUMN   ][ LAST  ] );

    printf ( "\n" );

    if ( variableNames )
        {
        int variable;

        for ( variable = 0; variable < subset[ VARIABLE ][ COUNT ]; ++variable )
            printf ( "variableNames[ %d ] = '%s'\n",
                     variable, variableNames[ variable ] );
        }

    printf ( "\n" );
    }


/******************************************************************************
 * PURPOSE: updateRange - Updates the range with the given data.
 * INPUTS:  const void* data       The data for a single variable.
 *          size_t numberOfValues  The number of data values.
 *          int initializeWithData Flag: just initialize range with this data?
 *          size_t variable        The number of the variable to update.
 *          int    vtype           The variable type M3INT or M3REAL.
 *          float* ranges          The current range.
 * OUTPUTS: float* ranges          The updated range.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void updateRange ( const void* data, size_t numberOfValues,
                          int initializeWithData, size_t variable, int vtype,
                          float* ranges )
    {
    PRE4 ( data, numberOfValues, ranges, IN3 ( vtype, M3INT, M3REAL ) );

    const int indexOfMinimum = 2 * variable + MINIMUM;
    const int indexOfMaximum = indexOfMinimum + 1;
    float minimum, maximum;

    if ( initializeWithData )
        ranges[ indexOfMinimum ] = ranges[ indexOfMaximum ] = 0.0;

    if ( computeRange ( data, numberOfValues, vtype, &minimum, &maximum ) )
        {
        if ( initializeWithData )
            {
            ranges[ indexOfMinimum ] = minimum;
            ranges[ indexOfMaximum ] = maximum;
            }
        else
            {
            ranges[ indexOfMinimum ] = MIN ( ranges[ indexOfMinimum ], minimum );
            ranges[ indexOfMaximum ] = MAX ( ranges[ indexOfMaximum ], maximum );
            }
        }
    }


/******************************************************************************
 * PURPOSE: computeRange - Computes the minimum and maximum m3range of each
 *          variable.
 * INPUTS:  const void* data       The data values to scan.
 *          size_t numberOfValues  The number of data values.
 *          int    vtype           The variable type M3INT or M3REAL.
 * OUTPUTS: float* minimum         The minimum value in data.
 *          float* maximum         The maximum value in data.
 * RETURNS: int 1 if not all values were BADVAL3, else 0.
 * NOTES:
 *****************************************************************************/

static int computeRange ( const void* data, size_t numberOfValues, int vtype,
                          float* minimum, float* maximum )
    {
    PRE5 ( data, numberOfValues, IN3 ( vtype, M3INT, M3REAL ), minimum, maximum );

    int computed = 0;
    size_t value;
    const int*   const idata = data;
    const float* const fdata = data;

    *minimum = *maximum = 0.0;

    for ( value = 0; value < numberOfValues; ++value )
        {
        const float currentValue = vtype == M3INT ? idata[ value ] : fdata[ value ];

        if ( currentValue > AMISS3 )
            {
            if ( ! computed )
                {
                *minimum = *maximum = currentValue;
                computed = 1;
                }
            else
                {
                if      ( currentValue < *minimum ) *minimum = currentValue;
                else if ( currentValue > *maximum ) *maximum = currentValue;
                }
            }
        }

    POST ( *minimum <= *maximum );

    return computed;
    }


#ifdef DEBUGGING

/******************************************************************************
 * PURPOSE: printIddata - Print an Iddata structure.
 * INPUTS:  const Iddata* iddata  Iddata structure to print.
 *          int numberOfValues    nvars * nlays.
 *          const int vtype[]     Variable types: M3INT or M3REAL.
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void printIddata ( const Iddata* iddata, int numberOfValues,
                          const int vtype[] )
    {
    PRE ( isValidIddata ( iddata ) );

    int station = 0;

    printf ( "\n\niddata = %p\n", iddata );
    printf ( "  iddata->count = %p\n",  iddata->count );
    printf ( " *iddata->count = %d\n", *iddata->count );

    for ( station = 0; station < *iddata->count; ++station )
        {
        int variable = 0;
        int indexOfStationValues = station * numberOfValues;

        printf ( "  iddata->ids[       %4d ] = %d\n",
                 station, iddata->ids[ station ] );

        for ( variable = 0; variable < numberOfValues; ++variable )
            {
            const int index = indexOfStationValues + variable;

            if ( vtype[ variable ] == M3INT )
                {
                printf ( "    iddata->ivariables[ %4d ] = %d\n",
                         index, iddata->ivariables[ index ] );
                }
            else
                {
                printf ( "    iddata->fvariables[ %4d ] = %f\n",
                         index, iddata->fvariables[ index ] );
                }
            }
        }

    printf ( "\n" );
    }

#endif /* DEBUGGING */


