/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: MapProjections.c 83 2018-03-12 19:24:33Z coats $
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
 ******************************************************************************
 * PURPOSE: MapProjections.c - Implement functions for using map projections.
 * NOTES:
 * HISTORY: 06/1995, Todd Plessel, EPA/MMTSI, Created stubs.
 *          09/1995, Mark Bolstad, EPA/MMTSI, Implemented.
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include <stdio.h>              /* For sprintf().                          */
#include <stdarg.h>             /* For va_args.                            */
#include <string.h>             /* For memset(),memcpy()                   */

#include "projects.h"           /* For pj_errno, pj_*() USGS routines.     */

#include "Assertions.h"         /* For macros PRE(),POST(),CHECK(),XOR3(). */
#include "Error.h"              /* For errors().                           */
#include "Memory.h"             /* For macros NEW(), FREE().               */
#include "MapProjectionsInfo.h" /* For getMapProjectionVisibility(), etc.  */
#include "MapProjections.h"     /* For public functions.                   */

/*=========================== PRIVATE VARIABLES =============================*/

static const char SVN_ID[] = "$Id: MapProjections.c 83 2018-03-12 19:24:33Z coats $";

static MapProjection currentProjectionParms;
static PJ*           currentProjection; /* The current map projection.    */

static int pass_thru;         /* HACK for LAT_LON "projection". */

/*========================== FORWARD DECLARATIONS ===========================*/

static int createEllipseStrings (   const MapProjection* mapProjection,
                                    char* projectionStrings[], int* index );

static int createParameterStrings ( const MapProjection* mapProjection,
                                    char* projectionStrings[], int* index );

static int createString ( char** string, const char* format, ... );

static int generateMapProjection ( const MapProjection* mapProjection,
                                   PJ** projObj );


/*=========================== PUBLIC FUNCTIONS ==============================*/


/******************************************************************************
 * PURPOSE: setMapProjection - Establish the 'current' map projection that will
 *          be used during subsequent calls to projectLatLon().
 * INPUTS:  const MapProjection* mapProjection  The map projection to use.
 * OUTPUTS: None
 * RETURNS: 1 if successful, else 0.
 * NOTES:   Calls error() upon failure.
 *****************************************************************************/

int setMapProjection ( const MapProjection* mapProjection )
    {
    PRE ( isValidMapProjection ( mapProjection ) );

    int ok = 1;

    pj_errno = 0;

    if ( currentProjection )
        {
        pj_free ( currentProjection );
        currentProjection = 0;
        memset ( &currentProjectionParms, '\0', sizeof ( currentProjectionParms ) );
        }

    if ( mapProjection->type == LAT_LON ) pass_thru = 1;
    else
        {
        pass_thru = 0;

        ok = generateMapProjection ( mapProjection, &currentProjection );
        if ( ! ok ) error ( "%s", pj_strerrno ( pj_errno ) );
        else memcpy ( &currentProjectionParms, mapProjection,
                          sizeof ( currentProjectionParms ) );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: projectLatLon - Project the give (lat, lon) to (x, y) using the
 *          'current' map projection established with setMapProjection().
 * INPUTS:  double lat  The latitude  of the point to project.
 *          double lon  The longitude of the point to project.
 * OUTPUTS: double* x   The x-coordinate of the projected point.
 *          double* y   The y-coordinate of the projected point.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void projectLatLon ( double lat, double lon, double* x, double* y )
    {
    PRE5 ( x, y, IMPLIES ( ! currentProjection, pass_thru ),
           IN_RANGE ( lat,  -90.0,  90.0 ),
           IN_RANGE ( lon, -180.0, 540.0 ) );

    UV data;

    if ( pass_thru )
        {
        *x = lon;
        *y = lat;
        }
    else
        {
        data.u = lon * DEG_TO_RAD;
        data.v = lat * DEG_TO_RAD;
        data = pj_fwd ( data, currentProjection );
        *x = data.u;
        *y = data.v;
        }
    }


/******************************************************************************
 * PURPOSE: projectXY - Project the given (x, y) to (lat, lon) using the
 *          'current' map projection established with setMapProjection().
 * INPUTS:  double  x    The x-coordinate of the point to project.
 *          double  y    The y-coordinate of the point to project.
 * OUTPUTS: double* lat  The latitude  of the projected point.
 *          double* lon  The longitude of the projected point.

 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void projectXY ( double x, double y, double* lat, double* lon )
    {
    PRE6 ( lat, lon, x != HUGE_VAL, y != HUGE_VAL,
           IMPLIES ( ! currentProjection, pass_thru ),
           IMPLIES (   currentProjection, currentProjection->inv ) );

    UV data;

    if ( pass_thru )
        {
        *lon = x;
        *lat = y;
        }
    else
        {
        data.u = x;
        data.v = y;
        data = pj_inv ( data, currentProjection );
        *lon = data.u * RAD_TO_DEG;
        *lat = data.v * RAD_TO_DEG;
        }

    POST ( IMPLIES ( AND2 ( *lat != HUGE_VAL, *lon != HUGE_VAL ),
                     AND2 ( IN_RANGE ( *lat,  -90.0,  90.0 ),
                            IN_RANGE ( *lon, -180.0, 180.0 ) ) ) );

    }


/******************************************************************************
 * PURPOSE: isValidMapProjection - Check if the given map projection is valid.
 * INPUTS:  const MapProjection* mapProjection  The map projection to check.
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:   Used in PRE().
 *****************************************************************************/

int isValidMapProjection ( const MapProjection* mapProjection )
    {
    int ok = AND3 ( mapProjection != 0,
                    IN_RANGE ( mapProjection->type, 0, NUMBER_OF_PROJECTIONS - 1 ),
                    mapProjection->maxSubdivision >= 0 );

    if ( ok )
        {
        const double* ellParameters = mapProjection->ellParameters;
        const int* visibility = getMapProjectionVisibility ( mapProjection->type,
                                mapProjection->whichParameterSet );
        int param;

        ok = OR2 ( AND2 ( IN_RANGE ( mapProjection->ellipse,
                                     1, NUMBER_OF_ELLIPSES - 1 ),
                          IN_RANGE ( mapProjection->sphereType,
                                     E2S_NONE, NUMBER_OF_E2S_PARAMETERS - 1 ) ),
                   AND3 ( mapProjection->ellipse == 0,
                          ellParameters[A] > 0.0,
                          XOR3 ( ellParameters[ B  ] > 0.0,
                                 ellParameters[ RF ] > 0.0,
                                 ellParameters[ ES ] > 0.0 ) ) );

        for ( param = 0; AND2 ( ok, param < NUMBER_OF_PARAMETERS ); ++param )
            {
            if ( visibility[ param ] != -1 )
                {
                const double* extrema = getParameterExtrema ( mapProjection->type, param,
                                        mapProjection->whichParameterSet );
                ok = IN_RANGE ( mapProjection->parameters[param], extrema[0],extrema[1] );
                }
            }
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: isMapProjectionInvertible - Check if the mapProjection has an
 *                                      inverse.
 * INPUTS:  const MapProjection* mapProjection - the projection to check
 * OUTPUTS: None
 * RETURNS: 1 if the inverse exists for the proj/ellipse/parameter combination.
 * NOTES:
 ******************************************************************************/

int isMapProjectionInvertible ( const MapProjection* mapProjection )
    {
    PRE ( isValidMapProjection ( mapProjection ) );

    PJ* cur = 0;
    int ok = generateMapProjection ( mapProjection, &cur );

    if ( ok ) ok = cur->inv ? 1 : 0;

    pj_free ( cur );

    return ok;
    }


/*=========================== PRIVATE FUNCTIONS =============================*/


/******************************************************************************
 * PURPOSE: createEllipseStrings - Allocates and initializes strings with
 *          ellipse projection commands.
 * INPUTS:  const MapProjection* mapProjection  The map projection to use.
 *          int*  index                *index indicates the string to create.
 * OUTPUTS: char* projectionStrings[]  Allocated and initialized strings.
 *          int*  index        *index now indicates the next unallocated string
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static int createEllipseStrings ( const MapProjection* mapProjection,
                                  char* projectionStrings[], int* index )
    {
    PRE3 ( mapProjection, projectionStrings, index );

    int ok = 1;

    if ( mapProjection->ellipse )
        {
        ok = createString ( &projectionStrings[ *index ], "ellps=%s",
                            getEllipseProjName ( mapProjection->ellipse ) );

        if ( ok ) ++*index;

        if ( AND2 ( ok, mapProjection->sphereType != E2S_NONE ) )
            {
            static const char* const ellipseStrings[ NUMBER_OF_E2S_PARAMETERS ] =
                { 0, "R_A", "R_V", "R_a", "R_g", "R_h" };

            CHECK ( ellipseStrings[ NUMBER_OF_E2S_PARAMETERS - 1 ] );

            ok = createString ( &projectionStrings[ *index ],
                                ellipseStrings[ mapProjection->sphereType ] );

            if ( ok ) ++*index;
            }
        }
    else
        {
        int param;

        for ( param = 0; AND2 ( ok, param < NUMBER_OF_ELL_PARAMETERS ); ++param )
            {
            if ( mapProjection->ellParameters[ param ] )
                {
                ok = createString ( &projectionStrings[ *index ], "%s=%f",
                                    getEllipseParamTag ( param ),
                                    mapProjection->ellParameters[ param ] );

                if ( ok ) ++*index;
                }
            }
        }

    POST ( IMPLIES ( ok, projectionStrings[ *index - 1 ] ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: createParameterStrings - Allocates and initializes strings with
 *          parameter projection commands.
 * INPUTS:  const MapProjection* mapProjection  The map projection to use.
 *          int*  index                *index indicates the string to create.
 * OUTPUTS: char* projectionStrings[]  Allocated and initialized strings.
 *          int*  index        *index now indicates the next unallocated string
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static int createParameterStrings ( const MapProjection* mapProjection,
                                    char* projectionStrings[], int* index )
    {
    PRE3 ( mapProjection, projectionStrings, index );

    int param, ok = 1;
    const int* visibility = getMapProjectionVisibility ( mapProjection->type,
                            mapProjection->whichParameterSet );

    for ( param = 0; AND2 ( ok, param < NUMBER_OF_PARAMETERS ); ++param )
        {
        if ( visibility[ param ] != -1 )
            {
            if ( IN6 ( param, SOUTHERN_HEMI, NO_ROTATION, ROTATION_CONVERSION,
                       NO_U_OFFSET, ONE_HEMISPHERE ) )
                {
                if ( mapProjection->parameters[ param ] )
                    {
                    ok = createString ( &projectionStrings[ *index ], "%s",
                                        getMapProjectionParamTag ( mapProjection->type,
                                                param, mapProjection->whichParameterSet ) );

                    if ( ok ) ++*index;
                    }
                }
            else
                {
                ok = createString ( &projectionStrings[ *index ], "%s=%f",
                                    getMapProjectionParamTag ( mapProjection->type,
                                            param,
                                            mapProjection->whichParameterSet ),
                                    mapProjection->parameters[ param ] );

                if ( ok ) ++*index;
                }
            }
        }

    POST ( IMPLIES ( ok, projectionStrings[ *index - 1 ] ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: createString - Allocates and initializes a string to a
 *          given projection command.
 * INPUTS:  const char* format A printf()-like format string.
 *          ...                Optional arguments implies by the format string.
 * OUTPUTS: char** string      The address of the string that gets allocated
 *                             and initialized.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static int createString ( char** string, const char* format, ... )
    {
    PRE2 ( string, format );

    int ok = 0;
    va_list args; /* For stdarg magic. */

    va_start ( args, format );                         /* Begin stdarg magic.  */

    *string = NEW ( char, 80 );

    if ( *string ) ok = vsprintf ( *string, format, args ) != -1; /* Pass args. */

    va_end ( args );                                   /* End of stdarg magic. */

    return ok;
    }



/******************************************************************************
 * PURPOSE: generateMapProjection - initialize the USGS projection library
 *                                  to the user supplied input.
 * INPUTS:  const MapProjection* mapProjection - the requested projection
 *                PJ**           projObj       - the initialize proj Object
 * OUTPUTS:       PJ**           projObj       - the initialize proj Object
 * RETURNS: 1 if the initialization succeeded
 * NOTES:
 ******************************************************************************/

static int generateMapProjection ( const MapProjection* mapProjection,
                                   PJ** projObj )
    {
    PRE2 ( isValidMapProjection ( mapProjection ), projObj );

    enum { NUMBER_OF_PROJECTION_STRINGS = 24 };

    char* projectionStrings[ NUMBER_OF_PROJECTION_STRINGS ];

    int index = 0;
    int i     = 0;
    int ok    = 0;

    ok = createString ( &projectionStrings[ index ], "proj=%s",
                        getMapProjectionProjName ( mapProjection->type ) );

    if ( ok )
        {
        ++index;

        ok = createEllipseStrings (   mapProjection, projectionStrings, &index );

        ok = AND2 ( ok, createParameterStrings ( mapProjection, projectionStrings,
                    &index ) );

        if ( ok )
            {
            *projObj = pj_init ( index, projectionStrings );
            ok = *projObj != 0;
            }
        }

    CHECK ( index <= NUMBER_OF_PROJECTION_STRINGS );

    for ( i = 0; i < index; i++ ) FREE ( projectionStrings[ i ] );

    return ok;
    }

/******************************************************************************
 * PURPOSE: currentProjectionIsSet - Returns whether an active proj is set.
 * INPUTS:  None
 * OUTPUTS: None
 * RETURNS: int 1 if set, else 0.
 * NOTES:
 ******************************************************************************/

int currentProjectionIsSet ( void )
    {
    return currentProjection != 0;
    }


/******************************************************************************
 * PURPOSE: getCurrentProjection - Returns the current projection
 * INPUTS:  None
 * OUTPUTS: None
 * RETURNS: MapProjection *  A read-only reference to the current projection.
 * NOTES:
 ******************************************************************************/

const MapProjection* getCurrentProjection ( void )
    {
    PRE ( currentProjectionIsSet() );

    return &currentProjectionParms;
    }


