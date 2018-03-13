/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: MapUtilities.c 83 2018-03-12 19:24:33Z coats $
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
 * PURPOSE: MapUtilities.c - Declares functions for reading and writing Models-3
 *          data files. This library is essentially a supplement to Carlie's
 *          libm3io to provide some convenience routines and work-around some
 *          of its deficiencies.
 * NOTES:
 * HISTORY: 05/1996, Todd Plessel, EPA/MMTSI, Created.
 *          06/1996, Mark Bolstad, EPA/MMTSI, Added lower-level projection code
 *                                            moved from DrawMap Library.
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include <stdio.h>              /* For sprintf().                           */
#include <stddef.h>             /* For typedef size_t.                      */
#include <string.h>             /* For memcpy(), memset().                  */
#include <limits.h>             /* For HUGE_VAL.                            */
#include <float.h>              /* For DBL_MAX.                             */
#include <math.h>               /* For sqrt(), pow(), fabs().               */

#include "Assertions.h"         /* For PRE(), POST(), CHECK(), DEBUG().     */
#include "Error.h"              /* For error().                             */
#include "Memory.h"             /* For NEW(), FREE().                       */
#include "DataImport.h"         /* For *M3IO*().                            */
#include "MapFile.h"            /* For MapLines, *MapLines().               */
#include "MapProjections.h"     /* For MapProjection, WGS_84.               */
#include "MapProjectionsInfo.h" /* For getMapProjectionVisibility().        */
#include "MapUtilities.h"       /* For M3IOParameters and public functions. */

/*================================ TYPES ====================================*/

typedef struct PolylineList
    {
    int vertexCount;
    float* vertices;
    struct PolylineList* next;
    } PolylineList;

typedef struct
    {
    double x, y;
    } Point;

/*=========================== PRIVATE VARIABLES =============================*/

/* Required source-code control string. */

static const char SVN_ID[] = "$Id: MapUtilities.c 83 2018-03-12 19:24:33Z coats $";

static const int maximumSplittingSubdivisions = 8;

static const double UTM_FALSE_EASTING = 500000.0; /* As used in PROJ Library.*/

/*========================== FORWARD DECLARATIONS ===========================*/

static int equal ( double a, double b )
    {
    const double tolerance = 1e-5;
    return a > b ? a - b <= tolerance : b - a <= tolerance;
    }

static int gammaIsZero ( double gamma )
    {
    return equal ( gamma, 0.0 );
    }

static int gammaIsPlusOrMinus90 ( double gamma )
    {
    return OR2 ( equal ( gamma, 90.0 ), equal ( gamma, -90.0 ) );
    }

static int warnIfNonZeroGamma ( double gamma )
    {
    /*
     * FIX: Note: gamma is assumed to be zero.
     * The Proj Library offers no support for non-zero gamma anyway...
     */

    if ( ! equal ( gamma, 0.0 ) )
        fprintf ( stderr, "\a\n\nWarning: non-zero gamma ignored.\n\n" );

    return 1;
    }

static int createBdesc ( const M3IOParameters* parameters, IOAPI_Bdesc3* bdesc );

static double gridCellOffset ( int dimension, double point, double origin,
                               double cellSize );

static void updateMinmax ( double value, double* minimum, double* maximum );

static void updateCorners ( double lon, double lat, double corners[ 2 ][ 2 ] );

static void refine ( double lon0, double lat0,
                     double lon1, double lat1,
                     float* vertices,
                     int* index, int depth, int maxDepth,
                     double corners[ 2 ][ 2 ] );

static void projectAndRefine ( const Point* p1, const Point* p2,
                               int* index, float* vertices,
                               int maxSubdivision, double lonOffset,
                               double mapCorners[ 2 ][ 2 ] );

static int clipLine ( const Point* p0, const Point* p1,
                      Point* p2,       Point* p3,
                      const double corners[ 2 ][ 2 ] );

static int testPoint ( const Point* p, const double corners[ 2 ][ 2 ] );

static int clipTest ( double denom, double num, double* tE, double* tL );

static double adjustLon ( double longitude );

static int flatEnough ( double px0, double py0,
                        double px1, double py1,
                        double pxm, double pym );

static int splitPolyline ( const Point* p1, const Point* p2,
                           double lonOffset, const double corners[ 2 ][ 2 ],
                           Point* p3, Point* p4 );

static void splitBadEndpoint ( const Point* p1, const Point* p2, const Point* pp1,
                               double lonOffset,
                               Point* p3, Point* p4 );

static int splitEndpoint ( const Point* p1, const Point* p2,
                           const Point* pp1, const Point* pp2,
                           double lonOffset,
                           Point* p3, Point* p4 );

static int splitCrossover ( const Point* p1, const Point* p2,
                            Point* p3, Point* p4, double lonOffset );

static int splitCornerWrap ( const Point* p1, const Point* p2,
                             Point* p3, Point* p4,
                             const double corners[ 2 ][ 2 ] );
static int splitMidpoint ( const Point* p1, const Point* p2,
                           const Point* pp1, const Point* pp2,
                           double lonOffset, int depth,
                           Point* p3, Point* p4 );

static int areValidMapLines ( const MapLines* map[], size_t n );

static double norm ( double x, double y )
    {
    return x * x + y * y;
    }

static int additionalPoints ( int maxSubdivision )
    {
    return ( int ) pow ( 2.0, ( double ) maxSubdivision ) - 1;
    }

static double clamp ( double a, double b )
    {
    static const double epsilon = 1e-5;
    return a < b ? -epsilon : epsilon;
    }

static int equalPoints ( const Point* a, const Point* b )
    {
    return AND2 ( equal ( a->x, b->x ), equal ( a->y, b->y ) );
    }

static Point makePoint ( double x, double y )
    {
    Point p;
    p.x = x;
    p.y = y;
    return p;
    }

static double ratio ( double n1, double n2, double d1, double d2 )
    {
    return ( n1 - n2 ) / ( d1 - d2 );
    }

static double dot ( double ux, double uy, double vx, double vy )
    {
    return ux * vx + uy * vy;
    }

static double cellSize ( const GridInfo* gridInfo, int coordinate );

static void copyCorners ( double destination[ 2 ][ 2 ],
                          const double source[ 2 ][ 2 ],
                          double longitudeShift );

static void updateClippingCorners ( double lonOffset,
                                    const double corners[ 2 ][ 2 ],
                                    double mapCorners[ 2 ][ 2 ] );

static void projectLongitudeLines ( const GridInfo* gridInfo,
                                    MapLines* projectedGridLines );

static void projectLatitudeLines (  const GridInfo* gridInfo,
                                    MapLines* projectedGridLines );

static PolylineList* allocatePolyLine ( int numVertices, int additionalPts );

static void copyAndDeallocatePolylines ( PolylineList* list,
        MapLines* projectedMapLines );

static int projectPoint ( const Point* p, int* index,
                          double lonOffset, float* vertices,
                          double mapCorners[ 2 ][ 2 ],
                          const double clipCorners[ 2 ][ 2 ] );

static void projectMap_ ( const MapProjection* mapProjection,
                          const MapLines*      mapLines,
                          double               mapCorners[ 2 ][ 2 ],
                          PolylineList*        head );

static void gridToMapLines ( const GridInfo* gridInfo,
                             MapLines*       projectedGridLines );


/*=========================== PUBLIC FUNCTIONS ==============================*/


/******************************************************************************
 * PURPOSE: createProjectedMapLinesClippedToGrid - Allocate, create and return
 *          a projected MapLines clipped to the grid boundary described by the
 *          parameters.
 * INPUTS:  const char* mapFileName  Name of an (unprojected) McIDAS map file.
 *          const char* dataFileName          Optional M3IO file name or NULL
 *                                            to use parameters instead.
 *          const M3IOParameters* parameters  Parameters describing projection
 *                                            and grid and/or lat-lon clipping
 *                                            domain.
 * OUTPUTS: None
 * RETURNS: MapLines* - MapLines structure with projected lines clipped to the
 *          grid boundary if successful, else 0.
 * NOTES:   The returned result should be freed with:
 *          deallocateMapLines( mapLines ); FREE( mapLines );
 *          when no longer needed.
 *****************************************************************************/

MapLines* createProjectedMapLinesClippedToGrid ( const char* mapFileName,
        const char* dataFileName,
        const M3IOParameters* parameters )
    {
    PRE3 ( mapFileName,
           IMPLIES ( dataFileName,
                     AND2 ( parameters,
                            areValidDomainCorners ( ( const double ( * ) [2] )
                                    parameters->corners ) ) ),
           IMPLIES ( dataFileName == 0, areValidM3IOParameters ( parameters ) ) );

    int ok = 1;
    MapLines  unprojectedMapLines;
    MapLines* clippedProjectedMapLines = 0;

    if ( readMapLines ( mapFileName, &unprojectedMapLines ) )
        {
        M3IOParameters parameters2; /* Copy of parameters with complete info. */

        if ( dataFileName )
            {
            ok = readM3IOParameters ( dataFileName, &parameters2 );

            memcpy ( parameters2.corners, parameters->corners,
                     sizeof parameters2.corners );
            }
        else memcpy ( &parameters2, parameters, sizeof ( M3IOParameters ) );

        if ( ok )
            {
            MapProjection mapProjection;

            ok = 0;

            createMapProjection ( &parameters2, &mapProjection );

            if ( setMapProjection ( &mapProjection ) )
                {
                MapLines projectedMapLines;

                if ( createProjectedMapLines ( &mapProjection, &unprojectedMapLines,
                                               &projectedMapLines ) )
                    {
                    GridInfo gridInfo;

                    if ( createGridInfo ( &parameters2, &gridInfo ) )
                        {
                        MapLines projectedGridLines;

                        if ( createProjectedGridLines ( &gridInfo, &mapProjection,
                                                        &projectedGridLines ) )
                            {
                            clippedProjectedMapLines = NEW ( MapLines, 1 );

                            if ( clippedProjectedMapLines )
                                {
                                if ( createClippedProjectedMapLines ( &projectedMapLines,
                                                                      &projectedGridLines,
                                                                      clippedProjectedMapLines ) )
                                    {
                                    ok = 1;
                                    }
                                else
                                    {
                                    FREE ( clippedProjectedMapLines );
                                    }
                                }

                            deallocateMapLines ( &projectedGridLines );
                            }
                        }

                    deallocateMapLines ( &projectedMapLines );
                    }
                }
            }

        deallocateMapLines ( &unprojectedMapLines );
        }

    POST ( IMPLIES ( clippedProjectedMapLines,
                     AND2 ( isValidMapLines ( clippedProjectedMapLines ),
                            clippedProjectedMapLines->vertexCount > 0 ) ) );

    return clippedProjectedMapLines;
    }


/******************************************************************************
 * PURPOSE: readMapLines - Read a map file into a MapLines structure.
 * INPUTS:  const char* mapFileName   Name of a McIDAS map file. "OUTLUSAM".
 * OUTPUTS: MapLines*   mapLines      Allocated and initialized MapLines.
 * RETURNS: int
 * NOTES:   mapLines should be freed with deallocateMapLines() when no longer
 *          needed.
 *****************************************************************************/

int readMapLines ( const char* mapFileName, MapLines* mapLines )
    {
    PRE2 ( mapFileName, mapLines );

    int ok, numberOfPolylines, numberOfVertices;

    memset ( mapLines, 0, sizeof ( mapLines ) );

    ok = sizeOfMapFile ( mapFileName, &numberOfPolylines, &numberOfVertices );

    if ( ok )
        {
        if ( allocateMapLines ( numberOfPolylines, numberOfVertices, mapLines ) )
            {
            ok = readMapFile ( mapFileName, mapLines );

            if ( ! ok ) deallocateMapLines ( mapLines );
            }
        }

    POST ( IMPLIES ( ok, isValidMapLines ( mapLines ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: printMapLines - Prints a MapLines.
 * INPUTS:  const MapLines* mapLines  The MapLines structure to print.
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void printMapLines ( const MapLines* mapLines )
    {
    PRE ( isValidMapLines ( mapLines ) );

    int polyline;

    for ( polyline = 0; polyline < mapLines->polylineCount; ++polyline )
        {
        const int start  = mapLines->starts[  polyline ];
        const int length = mapLines->lengths[ polyline ];
        const int end    = start + length;
        const float* v   = mapLines->vertices + 2 * start;
        int vertex;

        printf ( "polyline #%6d\n", polyline + 1 );

        for ( vertex = start; vertex < end; ++vertex, v += 2 )
            printf ( " vertex #%6d: (x = %15f, y = %15f)\n", vertex + 1, v[X], v[Y] );
        }
    }


/******************************************************************************
 * PURPOSE: areValidM3IOParameters - Verify that M3IO parameters are valid.
 * INPUTS:  const M3IOParameters* parameters
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   If non-NULL parameters are invalid then error() is called.
 *****************************************************************************/

int areValidM3IOParameters ( const M3IOParameters* parameters )
    {
    int ok;
    IOAPI_Bdesc3 bdesc;

    ok = AND4 ( parameters, createBdesc ( parameters, &bdesc ),
                areValidDomainCorners ( ( const double ( * ) [2] )
                                        parameters->corners ),
                isValidEllipse ( parameters->ellipse, parameters->radius ) );

    if ( AND2 ( ! ok, parameters ) )
        {
        error ( "Invalid parameters:\n"
                "  gdtyp = %d\n"
                "  p_alp = %f, p_bet = %f, p_gam = %f\n"
                "  xcent = %f, ycent = %f\n"
                "  nrows = %d, ncols = %d\n"
                "  xorig = %f, yorig = %f\n"
                "  xcell = %f, ycell = %f\n"
                "  corners[ LOWER ][ LAT ] = %f\n"
                "  corners[ LOWER ][ LON ] = %f\n"
                "  corners[ UPPER ][ LAT ] = %f\n"
                "  corners[ UPPER ][ LON ] = %f\n"
                "  ellipse = %d, radius = %f\n",
                parameters->gdtyp,
                parameters->p_alp, parameters->p_bet, parameters->p_gam,
                parameters->xcent, parameters->ycent,
                parameters->nrows, parameters->ncols,
                parameters->xorig, parameters->yorig,
                parameters->xcell, parameters->ycell,
                parameters->corners[ LOWER ][ LAT ],
                parameters->corners[ LOWER ][ LON ],
                parameters->corners[ UPPER ][ LAT ],
                parameters->corners[ UPPER ][ LON ],
                parameters->ellipse, parameters->radius );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: areValidDomainCorners - Verify that the domain corners are valid.
 * INPUTS:  const double corners[ 2 ][ 2 ]  [ LOWER | UPPER ][ LAT | LON ].
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   If not valid then error() is called.
 *****************************************************************************/

int areValidDomainCorners ( const double corners[ 2 ][ 2 ] )
    {
    int ok;

    ok = AND5 ( corners,
                isValidLatitudeLongitude ( corners[ LOWER ][ LAT ],
                                           corners[ LOWER ][ LON ] ),
                isValidLatitudeLongitude ( corners[ UPPER ][ LAT ],
                                           corners[ UPPER ][ LON ] ),
                corners[ LOWER ][ LAT ] <= corners[ UPPER ][ LAT ],
                corners[ LOWER ][ LON ] <= corners[ UPPER ][ LON ] );

    if ( AND2 ( ! ok, corners ) )
        {
        error ( "Invalid domain corners:\n  "
                "latitude  range  = [%f, %f]"
                "longitude range  = [%f, %f]\n",
                corners[ LOWER ][ LAT ],
                corners[ UPPER ][ LAT ],
                corners[ LOWER ][ LON ],
                corners[ UPPER ][ LON ] );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: isValidEllipse - Verify an ellipse/radius specification.
 *          int            ellipse  CUSTOM_ELLIPSE, WGS_84, etc.
 *          double         radius   Of sphere iff ellipse == CUSTOM_ELLIPSE.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   If not valid then error() is called.
 *****************************************************************************/

int isValidEllipse ( int ellipse, double radius )

    {
    int ok = AND2 ( IN_RANGE ( ellipse, 0, NUMBER_OF_ELLIPSES - 1 ),
                    IMPLIES ( ellipse == CUSTOM_ELLIPSE, radius > 0.0 ) );

    if ( ! ok )
        error ( "Invalid ellipse (%d) and/or radius (%lf).\n", ellipse, radius );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readM3IOParameters - Get the required parameters from a data file.
 * INPUTS:  const char* fileName        Name of an M3IO file. "MET_CROSS_2".
 * OUTPUTS: M3IOParameters* parameters  Initialized parameters.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   Calls initializeM3IO() (and finalizeM3IO()) if not already
 *          initialized. Note: since ellipse, radius and corners are not
 *          obtainable from the M3IO file, these components of parameters are
 *          initialized to the default values: WGS_84, 0.0, -90 -180, 90,
 *          180.
 *****************************************************************************/

int readM3IOParameters ( const char* fileName, M3IOParameters* parameters )
    {
    PRE2 ( fileName, parameters );

    int ok = 0;
    int wasInitialized = isInitializedM3IO();

    memset ( parameters, 0, sizeof ( M3IOParameters ) );

    if ( OR2 ( wasInitialized, initializeM3IO() ) )
        {
        M3IOFile file;

        if ( openM3IOFileForReading ( fileName, &file ) )
            {
            parameters->gdtyp = file.bdesc.gdtyp;
            parameters->p_alp = file.bdesc.p_alp;
            parameters->p_bet = file.bdesc.p_bet;
            parameters->p_gam = file.bdesc.p_gam;
            parameters->xcent = file.bdesc.xcent;
            parameters->ycent = file.bdesc.ycent;
            parameters->nrows = file.bdesc.nrows;
            parameters->ncols = file.bdesc.ncols;
            parameters->xorig = file.bdesc.xorig;
            parameters->yorig = file.bdesc.yorig;
            parameters->xcell = file.bdesc.xcell;
            parameters->ycell = file.bdesc.ycell;

            parameters->corners[ LOWER ][ LAT ] =  -90.0;
            parameters->corners[ UPPER ][ LAT ] =   90.0;
            parameters->corners[ LOWER ][ LON ] = -180.0;
            parameters->corners[ UPPER ][ LON ] =  180.0;

            parameters->ellipse = WGS_84;

            ok = closeM3IOFile ( &file );
            }

        ok = AND2 ( OR2 ( wasInitialized, finalizeM3IO() ), ok );
        }

    if ( ! ok ) error ( "Failed to get info on file '%s'\n", fileName );

    POST ( IMPLIES ( ok, areValidM3IOParameters ( parameters ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: createGridInfo - Create a GridInfo from an M3IOParameters.
 * INPUTS:  const M3IOParameters* parameters   Valid parameters.
 * OUTPUTS: GridInfo*             gridInfo     Initialized GridInfo.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If parameters->gdtyp == UTMGRD3 then this routine also sets the
 *          current projection (via setMapProjection()) to UTM.
 *****************************************************************************/

int createGridInfo ( const M3IOParameters* parameters, GridInfo* gridInfo )
    {
    PRE2 ( areValidM3IOParameters ( parameters ), gridInfo );

    int ok = 1;
    const int x = AND2 ( parameters, parameters->gdtyp == LATGRD3 ) ? LON : X;
    const int y = AND2 ( parameters, parameters->gdtyp == LATGRD3 ) ? LAT : Y;

    if ( parameters->gdtyp == LATGRD3 ) gridInfo->gridType = LAT_LON_GRID;
    else                                gridInfo->gridType = PHYSICAL_GRID;

    if ( parameters->gdtyp == UTMGRD3 )
        {
        MapProjection mapProjection;

        createMapProjection ( parameters, &mapProjection );

        if ( setMapProjection ( &mapProjection ) )
            {
#define UTM_LON( zone ) ( -180.0 + ( (zone) - 0.5 ) * 6.0 )
            const double longitudeOfCenterOfUTMZone = UTM_LON ( parameters->p_alp );
            const double  latitudeOfCenterOfUTMZone = 0.0;
            double   xOfCenterOfUTMZone;
            double   yOfCenterOfUTMZone;
            double   projectedXCenter,   projectedYCenter;
            double unprojectedXCenter, unprojectedYCenter;
#undef UTM_LON

            projectLatLon ( latitudeOfCenterOfUTMZone, longitudeOfCenterOfUTMZone,
                            &xOfCenterOfUTMZone, &yOfCenterOfUTMZone );

            projectedXCenter = xOfCenterOfUTMZone + parameters->xcent;
            projectedYCenter = yOfCenterOfUTMZone + parameters->ycent;

            projectXY (    projectedXCenter,    projectedYCenter,
                           &unprojectedYCenter, &unprojectedXCenter );

            gridInfo->center[ LON ] = unprojectedXCenter;
            gridInfo->center[ LAT ] = unprojectedYCenter;
            }
        else ok = 0;
        }
    else
        {
        gridInfo->center[ LON ] = parameters->xcent;
        gridInfo->center[ LAT ] = parameters->ycent;
        }

    gridInfo->origin[     x   ] = parameters->xorig;
    gridInfo->origin[     y   ] = parameters->yorig;
    gridInfo->delta[      x   ] = parameters->xcell;
    gridInfo->delta[      y   ] = parameters->ycell;
    gridInfo->dimensions[ x   ] = parameters->ncols;
    gridInfo->dimensions[ y   ] = parameters->nrows;

    if ( ! ok ) memset ( gridInfo, 0, sizeof ( GridInfo ) );

    POST ( IMPLIES ( ok, isValidGridInfo ( gridInfo ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: createMapProjection - Initialize a MapProjection based on an
 *          M3IOParameters.
 * INPUTS:  const M3IOParameters* parameters   Valid parameters.
 * OUTPUTS: MapProjection* mapProjection       Initialized mapProjection.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void createMapProjection ( const M3IOParameters* parameters,
                           MapProjection* mapProjection )
    {
    PRE2 ( areValidM3IOParameters ( parameters ), mapProjection );

    /*
     * The table below maps from M3IO projection types and parameters to
     * MapProjection types and parameters.
     * The gammaTest is required to further discriminate some mappings.
     */

#define NUMBER_OF_PROJECTION_MAPPINGS 9

    enum { UNUSED = -1 };

    typedef struct
        {
        int m3type;
        int ( *gammaTest ) ( double );
        int type;
        int alp, bet, gam, ycent;
        } ProjectionMap;

    static const ProjectionMap projectionMap[ NUMBER_OF_PROJECTION_MAPPINGS ] =
        {
            { LATGRD3, 0, LAT_LON, UNUSED, UNUSED, UNUSED, UNUSED },
            {
            LAMGRD3, 0, LAMBERT_CONFORMAL_CONIC,
            MIN_LATITUDE, MAX_LATITUDE, CENTRAL_MERIDIAN, CENTRAL_PARALLEL
            },
            {
            MERGRD3, gammaIsZero, MERCATOR,
            LAT_OF_TRUE_SCALE, CENTRAL_MERIDIAN, UNUSED, UNUSED
            },
            {
            MERGRD3, gammaIsPlusOrMinus90, TRANSVERSE_MERCATOR,
            CENTRAL_PARALLEL, CENTRAL_MERIDIAN, UNUSED, UNUSED
            },
            {
            MERGRD3, 0, OBLIQUE_MERCATOR,
            CENTRAL_PARALLEL, CENTRAL_LON, AZIMUTH, UNUSED
            },
            {
            STEGRD3, warnIfNonZeroGamma, STEREOGRAPHIC,
            CENTRAL_PARALLEL, CENTRAL_MERIDIAN, UNUSED, UNUSED
            },
            { UTMGRD3, 0, UNIVERSAL_TRANSVERSE_MERCATOR, ZONE, UNUSED, UNUSED, UNUSED },
            { POLGRD3, 0, STEREOGRAPHIC, UNUSED, LAT_OF_TRUE_SCALE, CENTRAL_MERIDIAN, CENTRAL_PARALLEL },
            {
            ALBGRD3, 0, ALBERS_EQUAL_AREA,
            MIN_LATITUDE, MAX_LATITUDE, CENTRAL_MERIDIAN, CENTRAL_PARALLEL
            },
        };

    ProjectionMap map; /* The selected mapping. */

    int index = 0, matched = 0;

    while ( ! matched ) /* Search the table for the appropriate mapping. */
        {
        if ( AND2 ( projectionMap[ index ].m3type == parameters->gdtyp,
                    OR2 ( projectionMap[ index ].gammaTest == 0,
                          projectionMap[ index ].gammaTest ( parameters->p_gam ) ) ) )
            {
            matched = 1;
            }
        else ++index;

        CHECK ( index < NUMBER_OF_PROJECTION_MAPPINGS );
        }

    map = projectionMap[ index ]; /* Copy the mapping. */

    initializeMapProjectionToDefaults ( mapProjection, parameters->ellipse,
                                        parameters->radius, map.type,
                                        map.type == UNIVERSAL_TRANSVERSE_MERCATOR ? 1 : 0,
                                        ( const double ( * ) [2] ) parameters->corners );

    if ( map.alp  != UNUSED )
        mapProjection->parameters[ map.alp ] = parameters->p_alp;

    if ( map.bet  != UNUSED )
        mapProjection->parameters[ map.bet ] = parameters->p_bet;

    if ( map.gam  != UNUSED )
        mapProjection->parameters[ map.gam ] = parameters->p_gam;

    if ( map.ycent != UNUSED )
        mapProjection->parameters[ map.ycent] = parameters->ycent;

    POST ( isValidMapProjection ( mapProjection ) );
    }


/******************************************************************************
 * PURPOSE: createProjectedGridLines - Allocate, and initialize a projected
 *          MapLines representation of a grid.
 * INPUTS:  const GridInfo* gridInfo            GridInfo descriptive structure.
 *          const MapProjection* mapProjection  MapProjection to apply.
 * OUTPUTS: MapLines* projectedGridLines        Allocated and initialized
 *                                              MapLines representation of grid
 * RETURNS: int 1 if successful,  else 0.
 * NOTES:   The output result should be freed with deallocateMapLines() when
 *          no longer needed.
 *****************************************************************************/

int createProjectedGridLines ( const GridInfo* gridInfo,
                               const MapProjection* mapProjection,
                               MapLines* projectedGridLines )
    {
    PRE3 ( isValidGridInfo ( gridInfo ), isValidMapProjection ( mapProjection ),
           projectedGridLines );

    int ok = 0;
    int numberOfPolylinesInProjectedGrid, numberOfVerticesInProjectedGrid;

    memset ( projectedGridLines, 0, sizeof ( MapLines ) );

    sizeOfProjectedGrid ( gridInfo, mapProjection,
                          &numberOfPolylinesInProjectedGrid,
                          &numberOfVerticesInProjectedGrid );

    if ( ! GT_ZERO2 ( numberOfPolylinesInProjectedGrid,
                      numberOfVerticesInProjectedGrid ) )
        {
        fprintf ( stderr, "\a\n\nWarning: "
                  "None of the grid lines projected within the "
                  "specified domain.\n" );
        }
    else if ( allocateMapLines ( numberOfPolylinesInProjectedGrid,
                                 numberOfVerticesInProjectedGrid,
                                 projectedGridLines ) )
        {
        projectGrid ( mapProjection, gridInfo, projectedGridLines );
        ok = 1;
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: createProjectedMapLines - Allocate, and initialize a projected
 *          MapLines.
 * INPUTS:  const MapProjection* mapProjection        MapProjection to apply.
 *          const MapLines*      unprojectedMapLines  Unprojected MapLines.
 * OUTPUTS: MapLines* projectedMapLines Allocated and initialized MapLines.
 * RETURNS: int 1 if successful,  else 0.
 * NOTES:   The output result should be freed with deallocateMapLines() when
 *          no longer needed.
 *****************************************************************************/

int createProjectedMapLines ( const MapProjection* mapProjection,
                              const MapLines* unprojectedMapLines,
                              MapLines* projectedMapLines )
    {
    PRE3 ( isValidMapProjection ( mapProjection  ),
           isValidMapLines ( unprojectedMapLines ),
           unprojectedMapLines->vertexCount > 0 );

    int ok = 0;
    int numberOfPolylinesInProjectedMap, numberOfVerticesInProjectedMap;

    memset ( projectedMapLines, 0, sizeof ( MapLines ) );

    sizeOfProjectedMap ( unprojectedMapLines, mapProjection,
                         &numberOfPolylinesInProjectedMap,
                         &numberOfVerticesInProjectedMap );

    if ( ! GT_ZERO2 ( numberOfPolylinesInProjectedMap,
                      numberOfVerticesInProjectedMap ) )
        {
        fprintf ( stderr, "\a\n\nWarning: "
                  "None of the map lines projected within the "
                  "specified domain.\n" );
        }
    else if ( allocateMapLines ( numberOfPolylinesInProjectedMap,
                                 numberOfVerticesInProjectedMap,
                                 projectedMapLines ) )
        {
        projectMap ( mapProjection, unprojectedMapLines, projectedMapLines );
        ok = 1;
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: createClippedProjectedMapLines - Allocate, and initialize a
 *          projected MapLines clipped to a grid.
 * INPUTS:  const MapLines*      projectedMapLines   Projected map MapLines.
 *          const MapLines*      projectedGridLines  Projected grid MapLines.
 * OUTPUTS: MapLines* clippedProjectedMapLines Allocated and initialized
 *                                             MapLines of map clipped to grid.
 * RETURNS: int 1 if successful,  else 0.
 * NOTES:   The output result should be freed with deallocateMapLines() when
 *          no longer needed.
 *****************************************************************************/

int createClippedProjectedMapLines ( const MapLines* projectedMapLines,
                                     const MapLines* projectedGridLines,
                                     MapLines* clippedProjectedMapLines )
    {
    PRE3 ( isValidMapLines ( projectedMapLines  ),
           isValidMapLines ( projectedGridLines ),
           GT_ZERO2 ( projectedMapLines->vertexCount,
                      projectedGridLines->vertexCount ) );

    int ok = 0;
    int numberOfPolylinesInClippedProjectedMap;
    int numberOfVerticesInClippedProjectedMap;

    memset ( clippedProjectedMapLines, 0, sizeof ( MapLines ) );

    sizeOfClippedMapLines ( projectedMapLines,
                            ( const double ( * ) [2] ) projectedGridLines->corners,
                            &numberOfPolylinesInClippedProjectedMap,
                            &numberOfVerticesInClippedProjectedMap );

    if ( ! GT_ZERO2 ( numberOfPolylinesInClippedProjectedMap,
                      numberOfVerticesInClippedProjectedMap ) )
        {
        fprintf ( stderr, "\a\n\nWarning: "
                  "None of the map lines projected within the "
                  "specified grid.\n" );
        }
    else if ( allocateMapLines ( numberOfPolylinesInClippedProjectedMap,
                                 numberOfVerticesInClippedProjectedMap,
                                 clippedProjectedMapLines ) )
        {
        clipMapLines ( projectedMapLines,
                       ( const double ( * ) [2] ) projectedGridLines->corners,
                       clippedProjectedMapLines );
        ok = 1;
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: initializeMapProjectionToDefaults - Initialize a MapProjection to
 *          some reasonable defaults.
 * INPUTS:  MapProjection* mapProjection
 *          int            ellipse  CUSTOM_ELLIPSE, WGS_84, etc.
 *          double         radius   Of sphere iff ellipse == CUSTOM_ELLIPSE.
 *          int            projType
 *          int            alternative
 *          const double corners[ 2 ][ 2 ]   [ LOWER | UPPER ][ LAT | LON ]
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:   The recommended default values for ellipse and radius are:
 *          WGS_84 and 0.0.
 *****************************************************************************/

void initializeMapProjectionToDefaults ( MapProjection* mapProjection,
        int ellipse,
        double radius,
        int projType,
        int alternative,
        const double corners[ 2 ][ 2 ] )
    {
    PRE7 ( mapProjection, corners,
           IN_RANGE ( projType,    0, NUMBER_OF_PROJECTIONS - 1 ),
           IN_RANGE ( ellipse,     0, NUMBER_OF_ELLIPSES    - 1 ),
           IMPLIES (  ellipse == CUSTOM_ELLIPSE, radius > 0.0 ),
           IN_RANGE ( alternative, 0, MAX_ALTERNATIVES      - 1 ),
           areValidDomainCorners ( ( const double ( * ) [2] ) corners ) );

    int i;
    const int* vis = 0;

    memset ( mapProjection, 0, sizeof ( MapProjection ) );
    memcpy ( mapProjection->corners, corners, sizeof mapProjection->corners );

    vis = getMapProjectionVisibility ( projType, alternative );

    mapProjection->type              = projType;
    mapProjection->ellipse           = ellipse;
    mapProjection->sphereType        = E2S_NONE;
    mapProjection->whichParameterSet = alternative;

    for ( i = 0; i < NUMBER_OF_PARAMETERS; i++ )
        if ( vis[ i ] != -1 )
            mapProjection->parameters[ i ] =
                getMapProjectionParamIniter ( projType, i, alternative );

    for ( i = 0; i < NUMBER_OF_ELL_PARAMETERS; i++ )
        mapProjection->ellParameters[ i ] = getEllipseParamIniter ( ellipse, i );

    if ( ellipse == CUSTOM_ELLIPSE )
        {
        mapProjection->ellParameters[ A ] = radius;
        mapProjection->ellParameters[ B ] = radius;
        }

    POST ( isValidMapProjection ( mapProjection ) );
    }


/******************************************************************************
 * PURPOSE: computeProjectedGridOrigin - Calculates the projected grid origin
 *          in meters from the center of the map projection.
 * INPUTS:  M3IOParameters* parameters  Required grid/projection parameters.
 * OUTPUTS: double* projectedXOrigin   The projected x origin in meters.
 *          double* projectedYOrigin   The projected y origin in meters.
 * RETURNS: None
 * NOTES:   The projected values may be different from the original values
 *          since the center of the grid (xcent, ycent) and the center of the
 *          projection (e.g., p_gam, ycent) may differ. This projection is
 *          required to ensure that the grid will 'line-up' with projected
 *          maps.
 *****************************************************************************/

int computeProjectedGridOrigin ( const M3IOParameters* parameters,
                                 double* projectedXOrigin,
                                 double* projectedYOrigin )
    {
    PRE3 ( areValidM3IOParameters ( parameters ),
           projectedXOrigin, projectedYOrigin );

    int ok = 0;

    if ( parameters->gdtyp == LATGRD3 )
        {
        *projectedXOrigin = parameters->xorig;
        *projectedYOrigin = parameters->yorig;
        ok = 1;
        }
    else
        {
        GridInfo gridInfo;

        if ( createGridInfo ( parameters, &gridInfo ) )
            {
            MapProjection mapProjection;

            createMapProjection ( parameters, &mapProjection );

            if ( setMapProjection ( &mapProjection ) )
                {
                double projectedXCenter, projectedYCenter;

                projectLatLon ( gridInfo.center[ LAT ], gridInfo.center[ LON ],
                                &projectedXCenter, &projectedYCenter );

                *projectedXOrigin = projectedXCenter + parameters->xorig;
                *projectedYOrigin = projectedYCenter + parameters->yorig;

                if ( mapProjection.type == UNIVERSAL_TRANSVERSE_MERCATOR )
                    *projectedXOrigin -= UTM_FALSE_EASTING;

                ok = 1;
                }
            }
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: computeGridCell - Compute the fractional grid column and row
 *          numbers of a given point in (projected) meters.
 * INPUTS:  M3IOParameters* parameters  Required grid/projection parameters.
 *          const double projectedPoint[]   Projected point [X,Y].
 * OUTPUTS: double gridCell[]               Grid cell [X,Y].
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If the point is not within the grid then gridCell[] is set to
 *          [-1, -1].
 *****************************************************************************/

int computeGridCell ( const M3IOParameters* parameters,
                      const double projectedPoint[], double gridCell[] )
    {
    PRE3 ( areValidM3IOParameters ( parameters ), projectedPoint, gridCell );

    int ok = 0;
    MapProjection mapProjection;

    gridCell[ X ] = gridCell[ Y ] = -1.0;

    createMapProjection ( parameters, &mapProjection );

    if ( setMapProjection ( &mapProjection ) )
        {
        GridInfo gridInfo;

        if ( createGridInfo ( parameters, &gridInfo ) )
            {
            ok = metersToGridCell ( &gridInfo,
                                    projectedPoint[ X ], projectedPoint[ Y ],
                                    &gridCell[ X ], &gridCell[ Y ] );
            }
        }

    POST ( IMPLIES ( ! AND2 ( gridCell[ X ] == -1.0, gridCell[ Y ] == -1.0 ),
                     AND2 ( IN_RANGE ( gridCell[ X ],
                                       0.0, ( double ) parameters->ncols ),
                            IN_RANGE ( gridCell[ Y ],
                                       0.0, ( double ) parameters->nrows ) ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: createIOAPI_Bdesc3 - Create a bdesc structure based on parameters.
 * INPUTS:  const M3IOParameters* parameters  Required grid/proj parameters.
 * OUTPUTS: IOAPI_Bdesc3* bdesc         Initialized IOAPI_Bdesc3 description.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void createIOAPI_Bdesc3 ( const M3IOParameters* parameters, IOAPI_Bdesc3* bdesc )
    {
    PRE2 ( areValidM3IOParameters ( parameters ), bdesc );

    createBdesc ( parameters, bdesc );

    POST ( isValidIOAPI_Bdesc3 ( bdesc ) );
    }


/******************************************************************************
 * PURPOSE: createM3IOParameters - Create M3IOParameters based on an
 *          IOAPI_Bdesc3 and domain corners.
 * INPUTS:  const IOAPI_Bdesc3* bdesc   Initialized IOAPI_Bdesc3 description.
 *          const double corners[ 2 ][ 2 ]  [ LOWER | UPPER ][ LAT | LON ]
 *          int ellipse                 CUSTOM_ELLIPSE, WGS_84, etc.
 *          double radius               Iff CUSTOM_ELLIPSE use for sphere.
 * OUTPUTS: M3IOParameters* parameters  Initialized grid/projection parameters.
 * RETURNS: None
 * NOTES:   The since the ellipse and radius are not obtainable from bdesc,
 *          the recommended default values for these are WGS_84 and 0.0.
 *****************************************************************************/

void createM3IOParameters ( const IOAPI_Bdesc3* bdesc,
                            const double corners[ 2 ][ 2 ],
                            int ellipse,
                            double radius,
                            M3IOParameters* parameters )
    {
    PRE5 ( isValidIOAPI_Bdesc3 ( bdesc ),
           areValidDomainCorners ( ( const double ( * ) [2] ) corners ),
           IN_RANGE ( ellipse, 0, NUMBER_OF_ELLIPSES - 1 ),
           IMPLIES ( ellipse == CUSTOM_ELLIPSE, radius > 0.0 ),
           parameters );

    memset ( parameters, 0, sizeof ( M3IOParameters ) );

    parameters->gdtyp = bdesc->gdtyp;
    parameters->p_alp = bdesc->p_alp;
    parameters->p_bet = bdesc->p_bet;
    parameters->p_gam = bdesc->p_gam;
    parameters->xcent = bdesc->xcent;
    parameters->ycent = bdesc->ycent;
    parameters->nrows = bdesc->nrows;
    parameters->ncols = bdesc->ncols;
    parameters->xorig = bdesc->xorig;
    parameters->yorig = bdesc->yorig;
    parameters->xcell = bdesc->xcell;
    parameters->ycell = bdesc->ycell;

    memcpy ( parameters->corners, corners, sizeof parameters->corners );

    parameters->ellipse = ellipse;
    parameters->radius  = radius;

    POST ( areValidM3IOParameters ( parameters ) );
    }


/******************************************************************************
 * PURPOSE: projectMap - Project map border lines.
 * INPUTS:  const MapProjection* mapProjection     The projection to use.
 *          const MapLines*      mapLines          The map border lines.
 *                MapLines*      projectedMapLines The map border lines.
 * OUTPUTS:       MapLines*      projectedMapLines The map border lines.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void projectMap ( const MapProjection* mapProjection,
                  const MapLines*      mapLines,
                  MapLines*      projectedMapLines )
    {
    PRE3 ( isValidMapProjection ( mapProjection ), 
           isValidMapLines ( mapLines ),
           isValidMapLines ( projectedMapLines ) );

    PolylineList* head = allocatePolyLine ( 1, 0 );

    if ( head )
        {
        projectMap_ ( mapProjection, mapLines, projectedMapLines->corners, head );

        copyAndDeallocatePolylines ( head->next, projectedMapLines );

        FREE ( head->vertices );
        FREE ( head );
        }
    else error ( "Not enough memory to project map." );

    POST ( isValidMapLines ( projectedMapLines ) );
    }


/******************************************************************************
 * PURPOSE: projectGrid - Project grid lines into a MapLines structure.
 * INPUTS:  const MapProjection* mapProjection   The projection to use.
 *          const GridInfo*      gridInfo        The grid to draw.
 *                MapLines*      projectedGridLines The grid lines.
 * OUTPUTS:       MapLines*      projectedGridLines The grid lines.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void projectGrid ( const MapProjection* mapProjection,
                   const GridInfo*      gridInfo,
                   MapLines*      projectedGridLines )
    {
    PRE3 ( isValidMapProjection ( mapProjection      ),
           isValidGridInfo (      gridInfo           ),
           isValidMapLines (      projectedGridLines ) );

    const int x = gridInfo->gridType == LAT_LON_GRID ? LON : X;
    const int y = gridInfo->gridType == LAT_LON_GRID ? LAT : Y;
    const int numberOfVerticalLines   = gridInfo->dimensions[ x ] + 1;
    const int numberOfHorizontalLines = gridInfo->dimensions[ y ] + 1;

    int numPolylines, numVertices;
    MapLines gridLines;
    GridInfo grid = *gridInfo;

    if ( grid.gridType == PHYSICAL_GRID )
        {
        if ( setMapProjection ( mapProjection ) )
            {
            projectLatLon ( gridInfo->center[ LAT ],
                            gridInfo->center[ LON ] ,
                            &grid.origin[ X ], &grid.origin[ Y ] );

            grid.origin[ X ] += gridInfo->origin[ X ];
            grid.origin[ Y ] += gridInfo->origin[ Y ];

            if ( mapProjection->type == UNIVERSAL_TRANSVERSE_MERCATOR )
                grid.origin[ X ] -= UTM_FALSE_EASTING;
            }
        }

    numPolylines = numberOfVerticalLines + numberOfHorizontalLines;

    /* Multiply by 2 since grid verticies are duplicated: */

    numVertices  = numberOfVerticalLines * numberOfHorizontalLines * 2;

    if ( grid.gridType == LAT_LON_GRID )
        {
        if ( allocateMapLines ( numPolylines, numVertices, &gridLines ) )
            {
            gridToMapLines ( &grid, &gridLines );
            projectMap ( mapProjection, &gridLines, projectedGridLines );
            deallocateMapLines ( &gridLines );
            }
        }
    else gridToMapLines ( &grid, projectedGridLines );

    POST ( isValidMapLines ( projectedGridLines ) );
    }


/******************************************************************************
 * PURPOSE: sizeOfProjectedMap - Calculate the size needed for projected
 *                               maplines.
 * INPUTS:  const MapLines*      map            The map for calculation.
 *          const MapProjection* mapProjection  The map projection.
 * OUTPUTS: int*  numberOfPolylines             The number of polylines needed.
 *          int*  numberOfVertices              The number of vertices needed.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void sizeOfProjectedMap ( const MapLines* map,
                          const MapProjection* mapProjection,
                          int*  numberOfPolylines,
                          int*  numberOfVertices )
    {
    PRE4 ( isValidMapProjection ( mapProjection ), isValidMapLines ( map ),
           numberOfPolylines, numberOfVertices );

    double corners[ 2 ][ 2 ];
    PolylineList* head = 0;
    PolylineList* tmp  = 0;

    *numberOfPolylines = *numberOfVertices = 0;

    head = allocatePolyLine ( 1, 0 );

    if ( head )
        {
        projectMap_ ( mapProjection, map, corners, head );

        tmp  = head;
        head = head->next;
        FREE ( tmp->vertices );
        FREE ( tmp );

        while ( head )
            {
            if ( head->vertexCount > 0 )
                {
                ++*numberOfPolylines;
                *numberOfVertices += head->vertexCount;
                }

            tmp  = head;
            head = head->next;
            FREE ( tmp->vertices );
            FREE ( tmp );
            }
        }
    else error ( "Not enough memory to project map for sizing.\n" );
    }


/******************************************************************************
 * PURPOSE: sizeOfProjectedGrid - Calculate the size needed for conversion of
 *                                a grid into maplines.
 * INPUTS:  const GridInfo*      gridInfo      The grid for calculation.
 *          const MapProjection* mapProjection The map projection.
 * OUTPUTS: int*  numberOfPolylines            The number of polylines needed.
 *          int*  numberOfVertices             The number of vertices needed.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void sizeOfProjectedGrid ( const GridInfo* gridInfo,
                           const MapProjection* mapProjection,
                           int* numberOfPolylines,
                           int* numberOfVertices )
    {
    PRE4 ( isValidMapProjection ( mapProjection ), isValidGridInfo ( gridInfo ),
           numberOfPolylines, numberOfVertices );

#ifdef MAX
#undef MAX
#endif
#define MAX( a, b ) ( (a) > (b) ? (a) : (b) )

    const int numberOfVerticalLines   = gridInfo ?
                                        gridInfo->dimensions[ LON ] + 1 : 0;
    const int numberOfHorizontalLines = gridInfo ?
                                        gridInfo->dimensions[ LAT ] + 1 : 0;
    int numPolylines, numVertices;
    MapLines gridLines;

    numPolylines = numberOfVerticalLines + numberOfHorizontalLines;

    /* Multiply by 2 since grid verticies are duplicated: */

    numVertices = numberOfVerticalLines * numberOfHorizontalLines * 2;

    *numberOfPolylines = 0;
    *numberOfVertices  = 0;

    if ( gridInfo->gridType == LAT_LON_GRID )
        {
        if ( allocateMapLines ( numPolylines, numVertices, &gridLines ) )
            {
            gridToMapLines ( gridInfo, &gridLines );
            sizeOfProjectedMap ( &gridLines, mapProjection,
                                 numberOfPolylines, numberOfVertices );

            deallocateMapLines ( &gridLines );

            *numberOfPolylines = MAX ( *numberOfPolylines, numPolylines );
            *numberOfVertices  = MAX ( *numberOfVertices,  numVertices  );
            }
        }
    else
        {
        *numberOfPolylines = numPolylines;
        *numberOfVertices  = numVertices;
        }
    }


/******************************************************************************
 * PURPOSE: sizeOfClippedMapLines - Determine the number of polylines and
 *          number of vertices in a MapLines after clipping it to a rectangle.
 * INPUTS:  const MapLines* mapLines           The source lines to scan.
 *          const double    corners[ 2 ][ 2 ]  The clipping rectangle.
 * OUTPUTS: int* numberOfPolylines             # of polylines left after clip.
 *          int* numberOfVertices              # of vertices  left after clip.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void sizeOfClippedMapLines ( const MapLines* mapLines,
                             const double corners[ 2 ][ 2 ],
                             int* numberOfPolylines,
                             int* numberOfVertices )
    {
    PRE7 ( mapLines, corners, numberOfPolylines, numberOfVertices,
           corners[ LOWER ][ LAT ] <= corners[ UPPER ][ LAT ],
           corners[ LOWER ][ LON ] <= corners[ UPPER ][ LON ],
           isValidMapLines ( mapLines ) );

    int polyline;          /* Count each source polyline in mapLines.   */

    /* Initialize outputs: */

    *numberOfPolylines = *numberOfVertices = 0;

    for ( polyline = 0; polyline < mapLines->polylineCount; ++polyline )
        {
        const int start  = mapLines->starts[  polyline ];
        const int length = mapLines->lengths[ polyline ];
        const int last   = start + length - 1;
        const float* v   = mapLines->vertices + 2 * start;
        int vertex;
        Point lastVertexCounted = makePoint ( HUGE_VAL, HUGE_VAL ); /* Sentinel. */

        for ( vertex = start; vertex < last; ++vertex, v += 2 )
            {
            Point e1 = makePoint ( v[ 0 ], v[ 1 ] ); /* 1st endpoint to clip. */
            Point e2 = makePoint ( v[ 2 ], v[ 3 ] ); /* 2nd endpoint to clip. */
            Point c1;                               /* 1st clipped enpoint. */
            Point c2;                               /* 2nd clipped enpoint. */

            if ( clipLine ( &e1, &e2, &c1, &c2, corners ) )
                {
                /* If continuation just add 2nd vertex to the current polyline: */

                if ( equalPoints ( &c1, &lastVertexCounted ) ) ++*numberOfVertices;
                else /* New polyline started. Count both vertices, c1 and c2. */
                    {
                    ++*numberOfPolylines;
                    *numberOfVertices += 2;
                    }

                lastVertexCounted = c2;
                }
            }
        }

    POST (  OR2 ( GT_ZERO2 ( *numberOfPolylines, *numberOfVertices ),
                  IS_ZERO2 ( *numberOfPolylines, *numberOfVertices ) ) );
    }


/******************************************************************************
 * PURPOSE: isValidGridInfo - Verify the validity of a GridInfo struct.
 * INPUTS:  const GridInfo* gridInfo The struct to verify.
 * OUTPUTS: None
 * RETURNS: 1 if the struct is ok, else 0.
 * NOTES:
 *****************************************************************************/

int isValidGridInfo ( const GridInfo* gridInfo )
    {
    return AND5 ( gridInfo != 0,
                  GT_ZERO4 ( gridInfo->dimensions[ 0 ], gridInfo->dimensions[ 1 ],
                             gridInfo->delta[ 0 ], gridInfo->delta[ 1 ] ),
                  IN_RANGE ( gridInfo->gridType, LAT_LON_GRID, PHYSICAL_GRID ),
                  IMPLIES ( gridInfo->gridType == LAT_LON_GRID,
                            AND2 ( isValidLatitudeLongitude ( gridInfo->origin[LAT],
                                    gridInfo->origin[LON] ),
                                   isValidLatitudeLongitude (
                                       gridInfo->origin[     LAT ] +
                                       gridInfo->dimensions[ LAT ] *
                                       gridInfo->delta[      LAT ],
                                       gridInfo->origin[     LON ] +
                                       gridInfo->dimensions[ LON ] *
                                       gridInfo->delta[      LON ] ) ) ),
                  IMPLIES ( gridInfo->gridType == PHYSICAL_GRID,
                            isValidLatitudeLongitude ( gridInfo->center[ LAT ],
                                    gridInfo->center[ LON ] ) ) );
    }


/******************************************************************************
 * PURPOSE: clipMapLines - Clip a set of map lines to a rectangle.
 * INPUTS:  const MapLines* mapLines           The source lines to scan.
 *          const double    corners[ 2 ][ 2 ]  The clipping rectangle.
 * OUTPUTS: MapLines*       clippedMapLines    The clipped copy.
 * RETURNS: None
 * NOTES:   The output clippedMapLines is assumed to have been allocated
 *          (e.g., via allocateMapLines()) large enough to hold the clipped
 *          copy. (The size of this clipped copy can be / should have been
 *          determined using sizeOfClippedMapLines().)
 *****************************************************************************/

void clipMapLines ( const MapLines* mapLines, const double corners[ 2 ][ 2 ],
                    MapLines* clippedMapLines )
    {
    PRE10 ( mapLines, corners, clippedMapLines, clippedMapLines->starts,
            clippedMapLines->lengths, clippedMapLines->vertices,
            corners[ LOWER ][ LAT ] <= corners[ UPPER ][ LAT ],
            corners[ LOWER ][ LON ] <= corners[ UPPER ][ LON ],
            isValidMapLines ( mapLines ), mapLines != clippedMapLines );

    int    polyline; /* Count each source polyline in mapLines. */
    int*   clippedStart  = clippedMapLines ? clippedMapLines->starts   : 0;
    int*   clippedLength = clippedMapLines ? clippedMapLines->lengths  : 0;
    float* clippedVertex = clippedMapLines ? clippedMapLines->vertices : 0;

    /* Initialize output: */

    clippedMapLines->polylineCount = 0;
    clippedMapLines->vertexCount   = 0;

    *clippedStart  = 0;
    *clippedLength = 0;

    for ( polyline = 0; polyline < mapLines->polylineCount; ++polyline )
        {
        const int start  = mapLines->starts[  polyline ];
        const int length = mapLines->lengths[ polyline ];
        const int last   = start + length - 1;
        const float* v   = mapLines->vertices + 2 * start;
        int vertex;
        Point lastVertexCounted = makePoint ( HUGE_VAL, HUGE_VAL ); /* Sentinel. */

        for ( vertex = start; vertex < last; ++vertex, v += 2 )
            {
            Point e1 = makePoint ( v[ 0 ], v[ 1 ] ); /* 1st endpoint to clip. */
            Point e2 = makePoint ( v[ 2 ], v[ 3 ] ); /* 2nd endpoint to clip. */
            Point c1;                               /* 1st clipped enpoint.  */
            Point c2;                               /* 2nd clipped enpoint.  */

            if ( clipLine ( &e1, &e2, &c1, &c2, corners ) )
                {
                if ( equalPoints ( &c1, &lastVertexCounted ) )
                    {
                    /* Continuation so just add 2nd vertex to the current polyline: */

                    clippedVertex[ X ] = ( float ) c2.x; /* Copy 2nd vertex. */
                    clippedVertex[ Y ] = ( float ) c2.y;
                    clippedVertex += 2;        /* Point to next vertex. */

                    ++*clippedLength; /* Count that vertex in this polyline's length. */

                    ++clippedMapLines->vertexCount; /* And also in the total count. */
                    }
                else /* New polyline started. Add both vertices, c1 and c2. */
                    {
                    /*
                     * Set the start of the new polyline to either
                     * zero if it is the first polyline, or
                     * the sum of the previous start and the current length otherwise.
                     */

                    if ( clippedMapLines->polylineCount )
                        {
                        CHECK ( IMPLIES ( clippedMapLines->polylineCount == 1,
                                          * ( clippedStart - 1 ) == 0 ) );

                        CHECK ( IMPLIES ( clippedMapLines->polylineCount >  1,
                                          * ( clippedStart - 1 ) >  0 ) );

                        CHECK ( *clippedLength > 0 );

                        *clippedStart = * ( clippedStart - 1 ) + *clippedLength;

                        ++clippedLength;    /* Point to new polyline's length. */
                        *clippedLength = 0; /* Initialize new length to zero.  */
                        }

                    ++clippedStart;  /* Point to next polyline's start. */

                    clippedVertex[ X ] = ( float ) c1.x; /* Copy 1st vertex. */
                    clippedVertex[ Y ] = ( float ) c1.y;
                    clippedVertex += 2;        /* Point to next vertex. */

                    clippedVertex[ X ] = ( float ) c2.x; /* Copy 2nd vertex. */
                    clippedVertex[ Y ] = ( float ) c2.y;
                    clippedVertex += 2;        /* Point to next vertex. */

                    updateCorners ( c2.x, c2.y, clippedMapLines->corners );

                    *clippedLength = 2; /* Count both vertices in this polyline length. */

                    ++clippedMapLines->polylineCount;  /* Count this new polyline.      */
                    clippedMapLines->vertexCount += 2; /* Count both vertices in total. */
                    }

                lastVertexCounted = c2; /* The 2nd vertex becomes the new last point. */
                }
            }
        }

    computeMapLinesCorners ( clippedMapLines );

    POST ( isValidMapLines ( clippedMapLines ) );
    }


/******************************************************************************
 * PURPOSE: latLonToGridCell - Determine the fractional grid column and row
 *          location of a point in latitude-longitude.
 * INPUTS:  const GridInfo*      gridInfo       The grid.
 *          double               latitude       The latitude  of the point.
 *          double               longitude      The longitude of the point.
 * OUTPUTS: double*              column         The fractional grid column
 *                                              where the point lies or -1.0
 *                                              if the point is not within the
 *                                              grid.
 *          double*              row            The fractional grid row
 *                                              where the point lies or -1.0
 *                                              if the point is not within the
 *                                              grid.
 * RETURNS: int 1 if the point is within the grid, else 0.
 * NOTES: 1) column and row are offsets from the grid origin (xorig, yorig),
 *           i.e., column == 0 is xorig and row == 0 is yorig.
 *        2) Assumes the projection has been set via setProjection()
 *****************************************************************************/

int latLonToGridCell ( const GridInfo*      gridInfo,
                       double latitude, double longitude,
                       double* column,  double* row )
    {
    PRE5 ( currentProjectionIsSet(), isValidGridInfo ( gridInfo ),
           isValidLatitudeLongitude ( latitude, longitude ), column, row );

    const int x = AND2 ( gridInfo, gridInfo->gridType == LAT_LON_GRID ) ? LON : X;
    const int y = AND2 ( gridInfo, gridInfo->gridType == LAT_LON_GRID ) ? LAT : Y;
    int ok = 0;
    double xPoint, yPoint;

    *column = *row = -1.0;

    if ( gridInfo->gridType == PHYSICAL_GRID )
        projectLatLon ( latitude, longitude, &xPoint, &yPoint );
    else
        {
        xPoint = longitude;
        yPoint = latitude;
        }

    ok = metersToGridCell ( gridInfo, xPoint, yPoint, column,row );

    POST2 ( IMPLIES ( ! ok, AND2 ( *column == -1.0, *row == -1.0 ) ),
            IMPLIES (   ok,
                        AND2 ( IN_RANGE ( *column,
                                          0.0, ( double ) gridInfo->dimensions[ x ] ),
                               IN_RANGE ( *row,
                                          0.0, ( double ) gridInfo->dimensions[ y ] ) ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: metersToGridCell - Determine the fractional grid column and row
 *          location of a point in (projected) X-Y meters.
 * INPUTS:  const GridInfo*      gridInfo       The grid.
 *          double               xPoint         The X-coordinate of the point.
 *          double               yPoint         The Y-coordinate of the point.
 * OUTPUTS: double*              column         The fractional grid column
 *                                              where the point lies or -1.0
 *                                              if the point is not within the
 *                                              grid.
 *          double*              row            The fractional grid row
 *                                              where the point lies or -1.0
 *                                              if the point is not within the
 *                                              grid.
 * RETURNS: int 1 if the point is within the grid, else 0.
 * NOTES:   column and row are offsets from the grid origin (xorig, yorig),
 *          i.e., column == 0 is xorig and row == 0 is yorig.
 *****************************************************************************/

int metersToGridCell ( const GridInfo*      gridInfo,
                       double xPoint,   double yPoint,
                       double* column,  double* row )
    {
    PRE4 ( currentProjectionIsSet(), isValidGridInfo ( gridInfo ),
           column, row );

    const int x = AND2 ( gridInfo, gridInfo->gridType == LAT_LON_GRID ) ? LON : X;
    const int y = AND2 ( gridInfo, gridInfo->gridType == LAT_LON_GRID ) ? LAT : Y;
    int ok = 0;

    *column = *row = -1.0;

    if ( gridInfo->gridType == PHYSICAL_GRID )
        {
        double projectedGridCenterLatitude, projectedGridCenterLongitude;
        double gridXOrigin, gridYOrigin;

        projectLatLon ( gridInfo->center[ LAT ], gridInfo->center[ LON ],
                        &projectedGridCenterLongitude,
                        &projectedGridCenterLatitude );

        gridXOrigin = gridInfo->origin[ X ] + projectedGridCenterLongitude;
        gridYOrigin = gridInfo->origin[ Y ] + projectedGridCenterLatitude;

        if ( getCurrentProjection()->type == UNIVERSAL_TRANSVERSE_MERCATOR )
            gridXOrigin -= UTM_FALSE_EASTING;

        *column = gridCellOffset ( gridInfo->dimensions[ X ], xPoint,
                                   gridXOrigin, gridInfo->delta[ X ] );

        *row    = gridCellOffset ( gridInfo->dimensions[ Y ], yPoint,
                                   gridYOrigin, gridInfo->delta[ Y ] );
        }
    else if ( gridInfo->gridType == LAT_LON_GRID )
        {
        *column = gridCellOffset ( gridInfo->dimensions[ LON ], xPoint,
                                   gridInfo->origin[ LON ], gridInfo->delta[ LON] );

        *row    = gridCellOffset ( gridInfo->dimensions[ LAT ], yPoint,
                                   gridInfo->origin[ LAT ], gridInfo->delta[ LAT] );
        }

    if   ( AND2 ( *column != -1.0, *row != -1.0 ) ) ok = 1;
    else
        {
        *column  =       *row  = -1.0;
        ok = 0;
        }

    POST2 ( IMPLIES ( ! ok, AND2 ( *column == -1.0, *row == -1.0 ) ),
            IMPLIES (   ok,
                        AND2 ( IN_RANGE ( *column,
                                          0.0, ( double ) gridInfo->dimensions[ x ] ),
                               IN_RANGE ( *row,
                                          0.0, ( double ) gridInfo->dimensions[ y ] ) ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: computeCornerMinMax - Compute the extent of all map lines.
 * INPUTS:  const MapLines* map[]      The array of map lines.
 *          size_t n                   The number of map lines.
 * OUTPUTS: double corners[ 2 ][ 2 ]   The extents.
 * RETURNS: int 1 if the maps were not all empty, else 0.
 * NOTES:
 *****************************************************************************/

int computeCornerMinMax ( const MapLines* map[], size_t n,
                          double corners[ 2 ][ 2 ] )
    {
    PRE ( areValidMapLines ( map, n ) );

    size_t i;
    int valid = 0;

    corners[ LOWER ][ LAT ] = map[ 0 ]->corners[ LOWER ][ LAT ];
    corners[ UPPER ][ LAT ] = map[ 0 ]->corners[ UPPER ][ LAT ];
    corners[ LOWER ][ LON ] = map[ 0 ]->corners[ LOWER ][ LON ];
    corners[ UPPER ][ LON ] = map[ 0 ]->corners[ UPPER ][ LON ];

    for ( i = 0; i < n; ++i )
        {
        if ( map[ i ]->vertexCount != 0 )
            {
            updateCorners ( map[ i ]->corners[ LOWER ][ LON ],
                            map[ i ]->corners[ LOWER ][ LAT ],
                            corners );

            updateCorners ( map[ i ]->corners[ UPPER ][ LON ],
                            map[ i ]->corners[ UPPER ][ LAT ],
                            corners );
            valid = 1;
            }
        }

    return valid;
    }


/******************************************************************************
 * PURPOSE: isValidLatitudeLongitude - Determine if the given latitude and
 *          longitude point is valid.
 * INPUTS:  double latitude   The latitude  coordinate.
 *          double longitude  The longitude coordinate.
 * OUTPUTS: None
 * RETURNS: int 1 if valid else 0.
 * NOTES:
 *****************************************************************************/

int isValidLatitudeLongitude ( double latitude, double longitude )
    {
    return AND2 ( IN_RANGE ( latitude,   -90.0,  90.0 ),
                  IN_RANGE ( longitude, -180.0, 540.0 ) );
    }


/*============================ PRIVATE FUNCTIONS ============================*/


/******************************************************************************
 * PURPOSE: createBdesc - Create a bdesc structure based on parameters.
 * INPUTS:  const M3IOParameters* parameters  Grid/projection parameters.
 * OUTPUTS: IOAPI_Bdesc3* bdesc         Initialized IOAPI_Bdesc3 description.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

static int createBdesc ( const M3IOParameters* parameters, IOAPI_Bdesc3* bdesc )
    {
    PRE2 ( parameters, bdesc );

    int ok;

    memset ( bdesc, 0, sizeof ( IOAPI_Bdesc3 ) );

    bdesc->gdtyp = parameters->gdtyp;
    bdesc->p_alp = parameters->p_alp;
    bdesc->p_bet = parameters->p_bet;
    bdesc->p_gam = parameters->p_gam;
    bdesc->xcent = parameters->xcent;
    bdesc->ycent = parameters->ycent;
    bdesc->nrows = parameters->nrows;
    bdesc->ncols = parameters->ncols;
    bdesc->xorig = parameters->xorig;
    bdesc->yorig = parameters->yorig;
    bdesc->xcell = parameters->xcell;
    bdesc->ycell = parameters->ycell;

    /* Initialize other fields to reasonable default values: */

    bdesc->ftype = GRDDED3;
    bdesc->cdate = 1996100;
    bdesc->ctime = 0;
    bdesc->wdate = 1996100;
    bdesc->wtime = 0;
    bdesc->sdate = 1996100;
    bdesc->stime = 0;
    bdesc->tstep = 0;
    bdesc->mxrec = 1;
    bdesc->nvars = 1;
    bdesc->nlays = 1;
    bdesc->nthik = 0;
    bdesc->vgtyp = IMISS3;
    bdesc->vtype[0] = M3REAL;

    ok = isValidIOAPI_Bdesc3 ( bdesc );

    if ( ! ok )
        {
        error ( "Invalid parameters:\n"
                "  gdtyp = %d, p_alp = %f, p_bet = %f, p_gam = %f\n"
                "  xcent = %f, ycent = %f, nrows = %d, ncols = %d\n"
                "  xorig = %f, yorig = %f, xcell = %f, ycell = %f\n",
                parameters->gdtyp,
                parameters->p_alp, parameters->p_bet, parameters->p_gam,
                parameters->xcent, parameters->ycent,
                parameters->nrows, parameters->ncols,
                parameters->xorig, parameters->yorig,
                parameters->xcell, parameters->ycell );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: gridCellOffset - Compute the grid cell offset of a point.
 * INPUTS:  int dimension    Number of cells in the coordinate direction.
 *          double point     The point  coordinate.
 *          double origin    The origin coordinate.
 *          double cellSize  The size of a grid cell.
 * OUTPUTS: None
 * RETURNS: double offset from the origin or -1.0 if the point is outside the
 *          grid.
 * NOTES:
 *****************************************************************************/

static double gridCellOffset ( int dimension, double point, double origin,
                               double cellSize )
    {
    double offset;

    offset = ( point - origin ) / cellSize;

    if ( ! IN_RANGE ( offset, 0.0, dimension ) ) offset = -1.0;

    return offset;
    }


/******************************************************************************
 * PURPOSE: updateMinmax - Compares a value to a range and widens the range if
 *          necessary.
 * INPUTS:  double  value    The value to compare.
 *          double* minimum  The current minimum.
 *          double* maximum  The current maximum.
 * OUTPUTS: double* minimum  The updated minimum.
 *          double* maximum  The updated maximum.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void updateMinmax ( double value, double* minimum, double* maximum )
    {
    PRE2 ( minimum, maximum ) /* Mark? && *minimum <= *maximum */;
    if      ( value < *minimum ) *minimum = value;
    /* Mark? else */ if ( value > *maximum ) *maximum = value;
    POST ( IN_RANGE ( value, *minimum, *maximum ) );
    }


/******************************************************************************
 * PURPOSE: updateCorners - Compares a point to a bounding box and expands the
 *          bounding box if the point lies outside it.
 * INPUTS:  double  lon               The longitude of the point.
 *          double  lat               The latittude of the point.
 *          double corners[ 2 ][ 2 ]  The current bounding box.
 * OUTPUTS: double corners[ 2 ][ 2 ]  The updated bounding box.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void updateCorners ( double lon, double lat, double corners[ 2 ][ 2 ] )
    {
    PRE ( corners );
    updateMinmax ( lat, &corners[ LOWER ][ LAT ], &corners[ UPPER ][ LAT ] );
    updateMinmax ( lon, &corners[ LOWER ][ LON ], &corners[ UPPER ][ LON ] );
    }


/******************************************************************************
 * PURPOSE: splitPolyline - Check to see if the line should be split
 * INPUTS:  const Point*    p1    XY-coordinates of the endpoint of the split.
 *          const Point*    p2    XY-coordinates of the endpoint of the split.
 *          double    lonOffset - offset from 0 degrees
 * OUTPUTS:       Point*    p3    XY-coordinates of the left split point.
 *                Point*    p4    XY-coordinates of the right split point.
 * RETURNS: 1 if the line segment needs to be split, 0 otherwise
 * NOTES:   This function will create a discontinuous line segment by
 *          creating an open region around the split point. So,
 *          p1         split       p2
 *           o-----------o----------o
 * becomes            p3  p4
 *           o---------o  o---------o
 *****************************************************************************/

static int splitPolyline ( const Point* p1, const Point* p2, double lonOffset,
                           const double corners[ 2 ][ 2 ],
                           Point* p3, Point* p4 )
    {
    PRE4 ( p1, p2, p3, p4 );

    int split = 0;
    Point pp1, pp2;

    projectLatLon ( p1->y,  p1->x + lonOffset , &pp1.x, &pp1.y );
    projectLatLon ( p2->y,  p2->x + lonOffset , &pp2.x, &pp2.y );

    /* Check for endpoint failure: */

    if ( AND2 ( pp1.x == HUGE_VAL, pp2.x == HUGE_VAL ) )
        {
        split = 0; /* Both failed. */
        }
    else if ( ! AND2 ( pp1.x < HUGE_VAL, pp2.x < HUGE_VAL ) ) /* Only one failed.*/
        {
        split = 1;
        splitBadEndpoint ( p1, p2, &pp1, lonOffset, p3, p4 );
        }
    /*  else if ( splitCrossover( p1, p2, p3, p4, lonOffset ) ) split = 1; */
    else if ( splitCornerWrap ( p1, p2, p3, p4, corners ) ) split = 1;
    else /* Both endpoints are valid so check the interior: */
        {
        if      ( splitEndpoint ( p1, p2, &pp1, &pp2, lonOffset, p3, p4 ) ) split =1;
        else if ( splitMidpoint ( p1, p2, &pp1, &pp2, lonOffset,0,p3,p4 ) ) split =1;
        }

    return split;
    }


/******************************************************************************
 * PURPOSE: splitBadEndpoint - Splits a polyline if one endpoint is out of
 *                             bounds for the current projection
 * INPUTS:  const Point*    p1    XY-coordinates of the endpoint of the split.
 *          const Point*    p2    XY-coordinates of the endpoint of the split.
 *          const Point*    pp1   Projected XY-coordinates of the left endpoint
 *          double    lonOffset - offset from 0 degrees
 * OUTPUTS:       Point*    p3    XY-coordinates of the left split point.
 *                Point*    p4    XY-coordinates of the right split point.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void splitBadEndpoint ( const Point* p1, const Point* p2,
                               const Point* pp1, double lonOffset,
                               Point* p3, Point* p4 )
    {
    PRE5 ( p1, p2, p3, p4, pp1 );

    int depth = 1;
    double fx, fy, gx, gy;
    double xm, ym;
    double pxm, pym;

    xm = ( p2->x + p1->x ) / 2.0;
    ym = ( p2->y + p1->y ) / 2.0;

    projectLatLon ( ym,  xm + lonOffset , &pxm, &pym );

    if ( pp1->x < HUGE_VAL )
        {
        if ( pxm < HUGE_VAL )
            {
            gx = xm;
            gy = ym;
            fx = p2->x;
            fy = p2->y;
            }
        else
            {
            fx = xm;
            fy = ym;
            gx = p1->x;
            gy = p1->y;
            }
        }
    else
        {
        if ( pxm < HUGE_VAL )
            {
            gx = xm;
            gy = ym;
            fx = p1->x;
            fy = p1->y;
            }
        else
            {
            fx = xm;
            fy = ym;
            gx = p2->x;
            gy = p2->y;
            }
        }

    while ( depth < maximumSplittingSubdivisions )
        {
        xm = ( fx + gx ) / 2.0;
        ym = ( fy + gy ) / 2.0;

        projectLatLon ( ym,  xm + lonOffset , &pxm, &pym );

        if ( pxm < HUGE_VAL )
            {
            gx = xm;
            gy = ym;
            }
        else
            {
            fx = xm;
            fy = ym;
            }

        depth++;
        }

    if ( pp1->x < HUGE_VAL )
        {
        p3->x = gx;
        p3->y = gy;
        p4->x = fx;
        p4->y = fy;
        }
    else
        {
        p3->x = fx;
        p3->y = fy;
        p4->x = gx;
        p4->y = gy;
        }
    }


/******************************************************************************
 * PURPOSE: splitEndpoint - Splits a polyline if one endpoint is near a
 *                          projection discontinuity
 * INPUTS:  const Point*    p1    XY-coordinates of the endpoint of the split.
 *          const Point*    p2    XY-coordinates of the endpoint of the split.
 *          const Point*    pp1   Projected XY-coordinates of the left endpoint
 *          const Point*    pp2   Projected XY-coordinates of right endpoint.
 *          double    lonOffset - offset from 0 degrees
 * OUTPUTS:       Point*    p3    XY-coordinates of the left split point.
 *                Point*    p4    XY-coordinates of the right split point.
 * RETURNS: 1 if the line segment needs to be split, 0 otherwise
 * NOTES:   This test for the case where a point near one endpoint projects
 *          closer to the other endpoint, indicating a discontinuity at the
 *          initial endpoint.
 *****************************************************************************/

static int splitEndpoint ( const Point* p1, const Point* p2, const Point* pp1,
                           const Point* pp2, double lonOffset,
                           Point* p3, Point* p4 )
    {
    PRE6 ( p1, p2, p3, p4, pp1, pp2 );

    double xm, ym;
    int split = 0;
    double pxm, pym;

    /* Check the left endpoint: */

    if ( equal ( p2->x, p1->x ) )
        {
        xm = p1->x;

        if ( equal ( p2->y, p1->y ) ) ym = p1->y;
        else                         ym = p1->y + clamp ( p2->y, p1->y );
        }
    else if ( equal ( p2->y, p1->y ) )
        {
        xm = p1->x + clamp ( p2->x, p1->x );
        ym = p1->y;
        }
    else
        {
        const double delta = clamp ( p2->x, p1->x );
        xm = p1->x + delta;
        ym = p1->y + delta * ratio ( p2->y, p1->y, p2->x, p1->x );
        }

    projectLatLon ( ym,  xm + lonOffset , &pxm, &pym );

    /* The projected endpoint + delta is closer to the other endpoint. */

    if ( norm ( pxm - pp1->x, pym - pp1->y ) > norm ( pp2->x - pxm, pp2->y - pym ) )
        {
        p3->x = p1->x;
        p3->y = p1->y;
        p4->x = xm;
        p4->y = ym;
        split = 1;
        }

    /* Check the right endpoint: */

    if ( ! split )
        {
        if ( equal ( p2->x, p1->x ) )
            {
            xm = p2->x;

            if ( equal ( p2->y, p1->y ) ) ym = p1->y;
            else                         ym = p2->y - clamp ( p2->y, p1->y );
            }
        else
            {
            const double delta = clamp ( p2->x, p1->x );
            xm = p2->x - delta;
            ym = p2->y + delta * ratio ( p2->y, p1->y, p2->x, p1->x );
            }

        projectLatLon ( ym,  xm + lonOffset , &pxm, &pym );

        /* The projected endpoint + delta is closer to the other endpoint. */

        if ( norm ( pxm - pp1->x, pym - pp1->y ) < norm ( pp2->x - pxm, pp2->y - pym ) )
            {
            p3->x = xm;
            p3->y = ym;
            p4->x = p2->x;
            p4->y = p2->y;
            split = 1;
            }
        }

    return split;
    }


/******************************************************************************
 * PURPOSE: splitCrossover - Split the polyline if it crosses +-180
 * INPUTS:  const Point*    p1    XY-coordinates of the endpoint of the split.
 *          const Point*    p2    XY-coordinates of the endpoint of the split.
 *          double    lonOffset - offset from 0 degrees
 * OUTPUTS:       Point*    p3    XY-coordinates of the left split point.
 *                Point*    p4    XY-coordinates of the right split point.
 * RETURNS:
 * NOTES:
 ******************************************************************************/

static int splitCrossover ( const Point* p1, const Point* p2,
                            Point* p3, Point* p4, double lonOffset )
    {
    PRE4 ( p1, p2, p3, p4 );

    double d1;
    int split = 0;

    if ( OR2 ( AND2 ( p1->x + lonOffset < 0.0, p2->x + lonOffset > 0.0 ),
               AND2 ( p2->x + lonOffset < 0.0, p1->x + lonOffset > 0.0 ) ) )
        {
        const double sign = p1->x + lonOffset < 0.0 ? -1.0 : 1.0;
        d1 = fabs ( sign * 180.0 - ( p1->x + lonOffset ) );

        if ( d1 < fabs ( p1->x + lonOffset ) )
            {
            double d2;
            split = 1;

            d2 = fabs ( -sign * 180.0 - ( p2->x + lonOffset ) );
            p3->x =     sign * 180.0 - lonOffset;
            p4->x =    -sign * 180.0 - lonOffset;
            p3->y = p1->y + sign * d1 * ( ( p2->y - p1->y ) / ( sign * ( d2 + d1 ) ) );
            p4->y = p3->y;
            }
        }

    return split;
    }


/******************************************************************************
 * PURPOSE: splitCornerWrap - Split the polyline if it wraps the corners
 * INPUTS:  const Point*    p1    XY-coordinates of the endpoint of the split.
 *          const Point*    p2    XY-coordinates of the endpoint of the split.
 *          double    lonOffset - offset from 0 degrees
 * OUTPUTS:       Point*    p3    XY-coordinates of the left split point.
 *                Point*    p4    XY-coordinates of the right split point.
 * RETURNS:
 * NOTES:
 ******************************************************************************/

static int splitCornerWrap ( const Point* p1, const Point* p2,
                             Point* p3, Point* p4,
                             const double corners[ 2 ][ 2 ] )
    {
    PRE4 ( p1, p2, p3, p4 );

    double slope;
    int split = 0;

    if ( fabs ( p2->x - p1->x ) > 180.0 )
        {
        split = 1;
        if (  p2->x > p1->x )
            {
            slope = ( p2->y - p1->y ) / ( ( p1->x + 360.0 ) - p2->x );
            p3->x = corners[LOWER][X];
            p3->y = p1->y + ( p1->x - p3->x ) * slope;
            p4->x = corners[UPPER][X];
            p4->y = p3->y;
            }
        else
            {
            slope = ( p2->y - p1->y ) / ( ( p2->x + 360.0 ) - p1->x );
            p3->x = corners[UPPER][X];
            p3->y = p2->y + ( p3->x - p1->x ) * slope;
            p4->x = corners[LOWER][X];
            p4->y = p3->y;
            }
        }

    return split;
    }


/******************************************************************************
 * PURPOSE: splitMidpoint - Splits a polyline if a projection discontinuity
 *                          exist in the interior of a polyline
 * INPUTS:  const Point*    p1    XY-coordinates of the endpoint of the split.
 *          const Point*    p2    XY-coordinates of the endpoint of the split.
 *          const Point*    pp1   Projected XY-coordinates of the left endpoint.
 *          const Point*    pp2   Projected XY-coordinates of right endpoint.
 *          double    lonOffset - offset from 0 degrees
 *          int       depth     - Current level of recursion
 * OUTPUTS:       Point*    p3    XY-coordinates of the left split point.
 *                Point*    p4    XY-coordinates of the right split point.
 * RETURNS: 1 if the line segment needs to be split, 0 otherwise
 * NOTES:   Terminates when the intervals are 1/2^8 of the original size
 *****************************************************************************/

static int splitMidpoint ( const Point* p1,  const Point* p2,
                           const Point* pp1, const Point* pp2,
                           double lonOffset, int depth,
                           Point* p3, Point* p4 )
    {
    PRE6 ( p1, p2, p3, p4, pp1, pp2 );

    Point pm;
    Point ppm;
    int split = 0;
    double theRatio;

    *p3 = *p1;
    *p4 = *p2;

    /*
     * Terminate when the intervals are 1 / 2 ^ maximumSplittingSubdivisions
     * of the original size.
     */

    if ( equalPoints ( p1, p2 ) )
        {
        if ( depth == 0 ) split = 0;
        else split = 1;
        }
    else if ( depth == maximumSplittingSubdivisions ) split = 1;
    else
        {
        pm.x = ( p2->x + p1->x ) / 2.0;
        pm.y = ( p2->y + p1->y ) / 2.0;

        projectLatLon ( pm.y,  pm.x + lonOffset , &ppm.x, &ppm.y );

        theRatio = norm ( ppm.x  - pp1->x, ppm.y  - pp1->y ) /
                   norm ( pp2->x - ppm.x,  pp2->y - ppm.y  );

        if ( theRatio < 0.5 )
            {
            if ( splitMidpoint ( &pm, p2, &ppm, pp2, lonOffset, depth+1, p3, p4 ) )
                split = 1;
            }
        else if ( theRatio > 2.0 )
            {
            if ( splitMidpoint ( p1, &pm, pp1, &ppm, lonOffset, depth+1, p3, p4 ) )
                split = 1;
            }
        else if ( depth > 0 )
            {
            if ( theRatio < 1.0 )
                {
                if ( splitMidpoint ( &pm, p2, &ppm, pp2, lonOffset, depth+1, p3, p4 ) )
                    split = 1;
                }
            else
                {
                if ( splitMidpoint ( p1, &pm, pp1, &ppm, lonOffset, depth+1, p3, p4 ) )
                    split = 1;
                }
            }
        }

    return split;
    }


/******************************************************************************
 * PURPOSE: projectAndRefine - Clip, Project, and (if necessary) refine the
 *                             line segment.
 * INPUTS:  const Point*   x1                 - XY-coordinates of an endpoint
 *          const Point*   x2                 - XY-coordinates of an endpoint
 *          int*           index              - The current index into vertices
 *          float*         vertices           - Vertices for this line segment
 *          int            maxSubdivision     - The maximum refinement level
 *          double         lonOffset          - offset from 0 degrees
 *          double         corners[ 2 ][ 2 ]  - the clipping bounds
 * OUTPUTS: float*         vertices           - Vertices for this line segment
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void projectAndRefine ( const Point* p1, const Point* p2, int* index,
                               float* vertices, int maxSubdivision,
                               double lonOffset,
                               double mapCorners[ 2 ][ 2 ] )

    {
    PRE ( maxSubdivision >= 0 );

    refine ( p1->x + lonOffset, p1->y, p2->x + lonOffset, p2->y,
             vertices, index, 1, maxSubdivision, mapCorners );
    }


/******************************************************************************
 * PURPOSE: refine - Recursively refine a line segment.
 * INPUTS:  double    x0          The x-coordinate of the lower left  endpoint.
 *          double    y0          The y-coordinate of the lower left  endpoint.
 *          double    x1          The x-coordinate of the upper right endpoint.
 *          double    y1          The y-coordinate of the upper right endpoint.
 *          float*    vertices    The line set.
 *          int*      index       The location in mapLines to store the point.
 *          int       depth       The current subdivision level.
 *          int       maxDepth    Maximum subdivision level.
 *          double corners[ 2 ][ 2 ]  The bounds of the region.
 * OUTPUTS: float*    vertices        Updated line set.
 *          int*      index           Updated index.
 *          double corners[ 2 ][ 2 ]  Updated corners.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void refine ( double lon0, double lat0,
                     double lon1, double lat1,
                     float* vertices,
                     int* index,
                     int  depth, int maxDepth,
                     double corners[ 2 ][ 2 ] )
    {
    PRE3 ( index, depth > 0, maxDepth >= 0 );

    double px0, py0, px1, py1, pxm, pym;
    double mLon, mLat;

    mLon = ( lon1 + lon0 ) / 2.0;
    mLat = ( lat1 + lat0 ) / 2.0;

    projectLatLon ( lat0,  lon0 , &px0, &py0 );

    if ( depth > maxDepth )
        {
        /*
         * if we've recursed to this depth, then the line segment still isn't
         * flat enough, but we store it anyway.
         */

        if ( px0 < HUGE_VAL )
            {
            vertices[ 2 * ( *index ) + Y ] = ( float ) py0;
            vertices[ 2 * ( *index ) + X ] = ( float ) px0;
            updateCorners ( px0, py0, corners );
            ++*index;
            }

        return;
        }

    projectLatLon ( lat1,  lon1 , &px1, &py1 );
    projectLatLon ( mLat,  mLon , &pxm, &pym );

    if ( flatEnough ( px0, py0, px1, py1, pxm, pym ) )
        {
        vertices[ 2 * ( *index ) + Y ] = ( float ) py0;
        vertices[ 2 * ( *index ) + X ] = ( float ) px0;
        updateCorners ( px0, py0, corners );
        ++*index;
        }
    else
        {
        refine ( lon0, lat0, mLon, mLat, vertices, index, depth + 1, maxDepth,
                 corners );
        refine ( mLon, mLat, lon1, lat1, vertices, index, depth + 1, maxDepth,
                 corners );
        }
    }


/******************************************************************************
 * PURPOSE: flatEnough - Test if the line segment needs to be refined
 * INPUTS:  double px0
 *          double py0
 *          double px1
 *          double py1 - The endpoints of the line segment
 *          double pxm
 *          double pym - The location for subdivision (midpoint in lat-lon)
 * OUTPUTS: None
 * RETURNS: int 1 if the midpoint is within 5% of the line segment
 *          joining the endpoints, else 0.
 * NOTES:
 *****************************************************************************/

static int flatEnough ( double px0, double py0,
                        double px1, double py1,
                        double pxm, double pym )
    {
    int isFlat = 0;
    const double threshold = 0.001;

    /* If either endpoint projects out of range, then force refinement. */

    if      ( OR2 ( px0 == HUGE_VAL, px1 == HUGE_VAL        ) ) isFlat = 0;
    else if ( AND2 ( px1 - px0 < 0.0001, py1 - py0 < 0.0001 ) ) isFlat = 1;
    else
        {
        const double ux = px1 - px0;
        const double uy = py1 - py0;
        const double vx = pxm - px0;
        const double vy = pym - py0;
        const double dp = dot ( ux, uy, vx, vy );
        const double length = sqrt ( dp );
        const double w = dp / norm ( ux, uy ); /* Length of v projected onto u. */
        double x, y;

        /* Check the bounds of the parametric length: */

        if      ( w >= 1.0 )
            {
            x = px1;
            y = py1;
            }
        else if ( w <= 0.0 )
            {
            x = px0;
            y = py0;
            }
        else
            {
            x = px0 + w * ux;
            y = py0 + w * uy;
            }

        x -= pxm;
        y -= pym;

        isFlat = sqrt ( norm ( x, y ) ) < threshold * length;
        }

    return isFlat;
    }


/******************************************************************************
 * PURPOSE: clipLine - Clip a line against a rectangle
 * INPUTS:  Point* p0                      - The lower left endpoint
 *          Point* p1                      - The upper right endpoint
 *          const double corners[ 2 ][ 2 ] - The clipping rectangle
 * OUTPUTS: Point* y0                      - The lower left clipped endpoint
 *          Point* y1                      - The upper right clipped endpoint
 * RETURNS: 1 if the line is at least partially contained in the rectangle,
 *          else 0 (the line is completely outside the rectangle).
 * NOTES:   Based on Liang-Barsky parametric line-clipping algorithm in
 *          _Computer Graphics Principles and Practice, 2nd Edition_
 *          by Fole, vanDam, Feiner and Hughes, p 123, Fig 3.45.
 *****************************************************************************/

static int clipLine ( const Point* p0, const Point* p1,
                      Point* p2,       Point* p3,
                      const double corners[ 2 ][ 2 ] )
    {
    PRE2 ( p0, p1 );

    double tE = 0, tL = 1;
    double dx, dy;
    int visible = 0;

    p2->x = p0->x;
    p2->y = p0->y;
    p3->x = p1->x;
    p3->y = p1->y;

    dx = p1->x - p0->x;
    dy = p1->y - p0->y;

    if ( AND3 ( dx == 0, dy == 0, testPoint ( p0, corners ) ) ) visible = 1;
    else
        {
        if ( AND4 ( clipTest (  dx, corners[ LOWER ][ X ] - p0->x, &tE, &tL ),
                    clipTest ( -dx, p0->x - corners[ UPPER ][ X ], &tE, &tL ),
                    clipTest (  dy, corners[ LOWER ][ Y ] - p0->y, &tE, &tL ),
                    clipTest ( -dy, p0->y - corners[ UPPER ][ Y ], &tE, &tL ) ) )
            {
            visible = 1;

            if ( tL < 1.0 )
                {
                p3->x = p0->x + tL * dx;
                p3->y = p0->y + tL * dy;
                }

            if ( tE > 0.0 )
                {
                p2->x = p0->x + tE * dx;
                p2->y = p0->y + tE * dy;
                }
            }
        }

    return visible;
    }


/******************************************************************************
 * PURPOSE: testPoint - See if a point is contained within a rectangle.
 * INPUTS:  Point x                         XY-coordinate of the point.
 *          const double corners[ 2 ][ 2 ]  The bounding rectangle.
 * OUTPUTS: None
 * RETURNS: 1 if the point is contained in or on the rectangle, else 0.
 * NOTES:
 *****************************************************************************/

static int testPoint ( const Point* p, const double corners[ 2 ][ 2 ] )
    {
    return AND2 ( IN_RANGE ( p->x, corners[ LOWER ][ X ], corners[ UPPER ][ X ] ),
                  IN_RANGE ( p->y, corners[ LOWER ][ Y ], corners[ UPPER ][ Y ] ) );
    }


/******************************************************************************
 * PURPOSE: clipTest - Adjust the parametric weights of a line
 * INPUTS:  double denom
 *          double num
 * OUTPUTS: double* tE
 *          double* tL
 * RETURNS: int 1 if the parametric weight is within bounds, else 0.
 * NOTES:   Based on Liang-Barsky parametric line-clipping algorithm in
 *          _Computer Graphics Principles and Practice, 2nd Edition_
 *          by Fole, vanDam, Feiner and Hughes, p 122, Fig 3.45.
 *****************************************************************************/

static int clipTest ( double denom, double num, double* tE, double* tL )
    {
    double t;
    int accept = 1;

    if ( denom > 0.0 )
        {
        t = num / denom;
        if      ( t > *tL ) accept = 0;
        else if ( t > *tE ) *tE = t;
        }
    else if ( denom < 0.0 )
        {
        t = num / denom;
        if      ( t < *tE ) accept = 0;
        else if ( t < *tL ) *tL = t;
        }
    else if ( num > 0.01 ) accept = 0;

    return accept;
    }


/******************************************************************************
 * PURPOSE: areValidMapLines - Checks all mapLines in an array for validity
 * INPUTS:  const MapLines* map[] - the array of line sets
 *                size_t    n     - the number of line sets
 * OUTPUTS:
 * RETURNS: int - 1 if all of the line sets are ok, else 0
 * NOTES:
 *****************************************************************************/

static int areValidMapLines ( const MapLines* map[], size_t n )
    {
    int i, ok = AND2 ( map, n );

    for ( i = 0; AND2 ( ok, i < n ); ++i ) ok = isValidMapLines ( map[i] );

    return ok;
    }


/******************************************************************************
 * PURPOSE: cellSize - Compute the horizontal or vertical size of a grid cell.
 * INPUTS:  const GridInfo* gridInfo    The grid info structure.
 *          int             coordinate  LAT or LON.
 * OUTPUTS: None
 * RETURNS: double The size of that dimension of a grid cell.
 * NOTES:
 *****************************************************************************/

static double cellSize ( const GridInfo* gridInfo, int coordinate )
    {
    PRE2 ( isValidGridInfo ( gridInfo ), IN3 ( coordinate, LAT, LON ) );
    return gridInfo->delta[ coordinate ];
    }


/******************************************************************************
 * PURPOSE: copyCorners - Copy the corners with a shift.
 * INPUTS:  const double source[ 2 ][ 2 ]    The source corners to copy.
 *          double       longitudeShift      The longitude shift amount.
 * OUTPUTS: double destination[ 2 ][ 2 ]     The destination corners.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void copyCorners ( double destination[ 2 ][ 2 ],
                          const double source[ 2 ][ 2 ],
                          double longitudeShift )
    {
    PRE2 ( source, destination );
    destination[ LOWER ][ Y ] = source[ LOWER ][ LAT ];
    destination[ UPPER ][ Y ] = source[ UPPER ][ LAT ];
    destination[ LOWER ][ X ] = source[ LOWER ][ LON ] - longitudeShift;
    destination[ UPPER ][ X ] = source[ UPPER ][ LON ] - longitudeShift;
    }


/******************************************************************************
 * PURPOSE: updateClippingCorners - Update the grid corners with the projected
 *                                  corners from the clipping window.
 * INPUTS:  double   lonOffset      - offset from 0 degrees
 *          const double corners[ 2 ][ 2 ]
 * OUTPUTS: double mapCorners[ 2 ][ 2 ]
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void updateClippingCorners ( double lonOffset,
                                    const double corners[ 2 ][ 2 ],
                                    double mapCorners[ 2 ][ 2 ] )
    {
    double dx, dy;

    /*
    * Update the grid corners with the projected corners
    * from the clipping window.
    */

    projectLatLon ( corners[ LOWER ][ Y ],
                    corners[ LOWER ][ X ] + lonOffset , &dx, &dy );

    if ( dx < HUGE_VAL ) updateCorners ( dx, dy, mapCorners );

    projectLatLon ( corners[ UPPER ][ Y ],
                    corners[ UPPER ][ X ] + lonOffset , &dx, &dy );

    if ( dx < HUGE_VAL ) updateCorners ( dx, dy, mapCorners );
    }


/******************************************************************************
 * PURPOSE: projectLongitudeLines - Projects lines of longitude.
 * INPUTS:  const GridInfo* gridInfo            The grid info structure.
 * OUTPUTS: MapLines*       projectedGridLines  The projected grid lines.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void projectLongitudeLines ( const GridInfo* gridInfo,
                                    MapLines* projectedGridLines )
    {
    PRE4 ( isValidGridInfo ( gridInfo ),
           projectedGridLines->starts, projectedGridLines->lengths,
           projectedGridLines->vertices );

    const int x = gridInfo->gridType == LAT_LON_GRID ? LON : X;
    const int y = gridInfo->gridType == LAT_LON_GRID ? LAT : Y;
    const double dx = cellSize ( gridInfo, x );
    const double dy = cellSize ( gridInfo, y );
    const int numberOfVerticalLines   = gridInfo->dimensions[ x ] + 1;
    const int numberOfHorizontalLines = gridInfo->dimensions[ y ] + 1;
    int lon;

    /* Lines of constant longitude: */

    for ( lon = 0; lon < numberOfVerticalLines; ++lon )
        {
        int vertex;
        int lat;
        double longitude = gridInfo->origin[ x ] + lon * dx;

        if ( gridInfo->gridType == LAT_LON_GRID )
            longitude = ( longitude > 180.0 ) ? longitude - 360.0 : longitude;

        projectedGridLines->starts[  lon ] = numberOfHorizontalLines * lon;
        projectedGridLines->lengths[ lon ] = numberOfHorizontalLines;

        vertex = projectedGridLines->starts[ lon ];

        for ( lat = 0; lat < numberOfHorizontalLines; ++lat, ++vertex )
            {
            double latitude = gridInfo->origin[ y ] + lat * dy;
            projectedGridLines->vertices[ 2 * vertex + y ] = ( float ) latitude;
            projectedGridLines->vertices[ 2 * vertex + x ] = ( float ) longitude;
            }
        }
    }


/******************************************************************************
 * PURPOSE: projectLatitudeLines - Projects lines of latitude.
 * INPUTS:  const GridInfo* gridInfo            The grid info structure.
 * OUTPUTS: MapLines*       projectedGridLines  The projected grid lines.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void projectLatitudeLines ( const GridInfo* gridInfo,
                                   MapLines* projectedGridLines )
    {
    PRE4 ( isValidGridInfo ( gridInfo ),
           projectedGridLines->starts, projectedGridLines->lengths,
           projectedGridLines->vertices );

    const int x = AND2 ( gridInfo, gridInfo->gridType == LAT_LON_GRID ) ? LON : X;
    const int y = AND2 ( gridInfo, gridInfo->gridType == LAT_LON_GRID ) ? LAT : Y;
    const double dx = cellSize ( gridInfo, x );
    const double dy = cellSize ( gridInfo, y );
    const int numberOfVerticalLines   = gridInfo ?
                                        gridInfo->dimensions[ x ] + 1 : 0;
    const int numberOfHorizontalLines = gridInfo ?
                                        gridInfo->dimensions[ y ] + 1 : 0;
    int start = numberOfVerticalLines;
    int lat;

    /* Lines of constant latitude */

    for ( lat = 0; lat < numberOfHorizontalLines; ++lat )
        {
        const int start_1 = start - 1;
        double latitude  = gridInfo->origin[ y ] + lat * dy;
        int length       = 0;
        int lon;
        int vertex;

        vertex = projectedGridLines->starts[ start ] =
                     projectedGridLines->starts[  start_1 ] +
                     projectedGridLines->lengths[ start_1 ];

        for ( lon = 0; lon < numberOfVerticalLines; ++lon, ++vertex )
            {
            double longitude = gridInfo->origin[ x ] + lon * dx;

            if ( gridInfo->gridType == LAT_LON_GRID )
                longitude = ( longitude > 180.0 ) ? longitude - 360.0 : longitude;

            ++length;
            projectedGridLines->vertices[ 2 * vertex + y ] = ( float ) latitude;
            projectedGridLines->vertices[ 2 * vertex + x ] = ( float ) longitude;
            }

        projectedGridLines->lengths[ start ] = length;
        ++start;
        }
    }


/******************************************************************************
 * PURPOSE: allocatePolyLine - Allocate a new PolylineList node.
 * INPUTS:  int numVertices   - how many points in the vertex array
 *          int additionalPts - Additional points from refinement
 * OUTPUTS: None
 * RETURNS: PolylineList*
 * NOTES:
 *****************************************************************************/

static PolylineList* allocatePolyLine ( int numVertices, int additionalPts )
    {
    PRE2 ( numVertices > 0, additionalPts >= 0 );

    int ok = 0;
    int count;
    PolylineList* node = 0;

    node = NEW ( PolylineList, 1 );

    if ( node )
        {
        node->vertexCount = 0;
        node->next = 0;
        count = numVertices + ( numVertices - 1 ) *additionalPts;
        node->vertices = NEW ( float, 2 * count );

        if ( node->vertices ) ok = 1;
        }

    if ( ! ok ) FREE ( node );

    POST ( IMPLIES ( ok, AND3 ( node, node->vertices,
                                IS_ZERO2 ( node->next, node->vertexCount ) ) ) );

    return node;
    }


/******************************************************************************
 * PURPOSE: copyAndDeallocatePolylines - copy the polylines to the mapLines
 *                                       structure and free the polylines
 * INPUTS:  PolylineList* list          - The list of polylines
 * OUTPUTS: MapLines* projectedMapLines - The destination mapLines object
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void copyAndDeallocatePolylines ( PolylineList* list,
        MapLines* projectedMapLines )
    {
    PRE ( isValidMapLines ( projectedMapLines ) );

    PolylineList* tmp = 0;
    int start, polyCount;

    start = polyCount = 0;

    while ( list )
        {
        if ( list->vertexCount > 0 )
            {
            projectedMapLines->starts[  polyCount ] = start;
            projectedMapLines->lengths[ polyCount ] = list->vertexCount;

            memcpy ( &projectedMapLines->vertices[ 2 * start ], list->vertices,
                     2 * list->vertexCount * sizeof ( float ) );

            start += list->vertexCount;
            polyCount++;
            }

        FREE ( list->vertices );
        tmp = list->next;
        FREE ( list );
        list = tmp;
        }

    projectedMapLines->vertexCount   = start;
    projectedMapLines->polylineCount = polyCount;

    if ( projectedMapLines->vertexCount )
        computeMapLinesCorners ( projectedMapLines );

    POST4 ( ! list, isValidMapLines ( projectedMapLines ),
            projectedMapLines->vertexCount   == start,
            projectedMapLines->polylineCount == polyCount );
    }


/******************************************************************************
 * PURPOSE: projectPoint - project and clip a single point
 * INPUTS:  const Point* p                 - XY cooridinates of the point
 *          int index                      - index into vertices
 *          double lonOffset               - fudge factor for recentering proj.
 *          float* vertices                - the current polyline's vertices
 *          double mapCorners[ 2 ][ 2 ]        - the current map limits
 *          const double clipCorners[ 2 ][ 2 ] - the clipping window
 * OUTPUTS: float* vertices
 *          int* index
 *          double mapCorners[ 2 ][ 2 ]
 * RETURNS: 1 if the point wasn't clipped, 0 if it was
 * NOTES:
 ******************************************************************************/

static int projectPoint ( const Point* p, int* index,
                          double lonOffset, float* vertices,
                          double mapCorners[ 2 ][ 2 ],
                          const double clipCorners[ 2 ][ 2 ] )
    {
    PRE2 ( fabs ( p->y ) <= 90.0, vertices );

    int ok = 0;

    if ( testPoint ( p, ( const double ( * ) [2] ) clipCorners ) )
        {
        double x, y;
        projectLatLon ( p->y,  p->x + lonOffset , &x, &y );

        if ( x < HUGE_VAL )
            {
            vertices[ 2 * ( *index ) + X ] = ( float ) x;
            vertices[ 2 * ( *index ) + Y ] = ( float ) y;
            updateCorners ( x, y, mapCorners );
            ++*index;
            ok = 1;
            }
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: projectMap_ - Project map border lines.
 * INPUTS:  const MapProjection* mapProjection     The projection to use.
 *          const MapLines*      mapLines          The map border lines.
 *                PolylineList*  head              The internal map list
 * OUTPUTS:       PolylineList*  head
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void projectMap_ ( const MapProjection* mapProjection,
                          const MapLines*      mapLines,
                          double               mapCorners[ 2 ][ 2 ],
                          PolylineList*        head )
    {
    PRE4 ( isValidMapProjection ( mapProjection ), isValidMapLines ( mapLines ),
           mapCorners, head );

    double corners[ 2 ][ 2 ];
    const double lonOffset = mapProjection->parameters[ CENTRAL_MERIDIAN ];

    if ( setMapProjection ( mapProjection ) )
        {
        int i;
        int addedPoints = additionalPoints ( mapProjection->maxSubdivision );
        PolylineList* cur = head;

        /* Shift the clipping corners with the offset: */

        copyCorners ( corners, ( const double ( * ) [2] ) mapProjection->corners,
                      lonOffset );

        /* Initialize the projected corners: */

        mapCorners[ LOWER ][ LAT ] =  DBL_MAX;
        mapCorners[ UPPER ][ LAT ] = -DBL_MAX;
        mapCorners[ LOWER ][ LON ] =  DBL_MAX;
        mapCorners[ UPPER ][ LON ] = -DBL_MAX;

        /* For all polylines: */

        for ( i = 0; i < mapLines->polylineCount; ++i )
            {
            int length = 0;
            int j;
            const int s    = 2 * mapLines->starts[ i ];
            const int jend =     mapLines->starts[ i ] + mapLines->lengths[ i ] - 1;
            Point p1;

            p1.x = mapLines->vertices[ s + LON ] - lonOffset;
            p1.y = mapLines->vertices[ s + LAT ];

            cur->next = allocatePolyLine ( mapLines->lengths[ i ], addedPoints );

            if ( cur->next ) cur = cur->next;

            /* For all vertices in this polyline: */

            for ( j = mapLines->starts[ i ]; j < jend; ++j )
                {
                const int jj = 2 * ( j + 1 );
                Point p2, p3, p4;

                p2.x = mapLines->vertices[ jj + LON ] - lonOffset;
                p2.y = mapLines->vertices[ jj + LAT ];

                if ( p1.x < corners[LOWER][X] &&
                        p1.x+360.0 < corners[UPPER][X] )
                    p1.x += 360.0;

                if ( p2.x < corners[LOWER][X] &&
                        p2.x+360.0 < corners[UPPER][X] )
                    p2.x += 360.0;

                if ( clipLine ( &p1, &p2, &p3, &p4, ( const double ( * ) [2] ) corners ) )
                    {
                    Point tpt = p4;

                    p1 = p3;

                    if ( splitPolyline ( &p1, &tpt, lonOffset, ( const double ( * ) [2] ) corners,
                                         &p3, &p4 ) )
                        {
                        projectAndRefine ( &p1, &p3, &length, cur->vertices,
                                           mapProjection->maxSubdivision, lonOffset,
                                           mapCorners );
                        p1 = p4;

                        if ( ! projectPoint ( &p3, &length, lonOffset, cur->vertices,
                                              mapCorners, ( const double ( * ) [2] ) corners ) )
                            {
                            length--;
                            }

                        cur->vertexCount = length;
                        length = 0;

                        cur->next = allocatePolyLine ( mapLines->lengths[ i ], addedPoints );

                        if ( cur->next ) cur = cur->next;
                        }

                    projectAndRefine ( &p1, &tpt, &length, cur->vertices,
                                       mapProjection->maxSubdivision, lonOffset,
                                       mapCorners );

                    if ( ! equalPoints ( &p2, &tpt ) )
                        {
                        if ( ! projectPoint ( &tpt, &length, lonOffset, cur->vertices,
                                              mapCorners, ( const double ( * ) [2] ) corners ) )
                            {
                            length--;
                            }

                        cur->vertexCount = length;
                        length = 0;

                        cur->next = allocatePolyLine ( mapLines->lengths[ i ],
                                                       addedPoints );

                        if ( cur->next ) cur = cur->next;
                        }
                    }

                p1 = p2;
                }

            /* Test and project the last vertex of the polyline: */

            if ( ! projectPoint ( &p1, &length, lonOffset, cur->vertices, mapCorners,
                                  ( const double ( * ) [2] ) corners ) )
                {
                length--;
                }

            cur->vertexCount = length;
            }

        updateClippingCorners ( lonOffset, ( const double ( * ) [2] ) corners, mapCorners );
        }
    }


/******************************************************************************
 * PURPOSE: gridToMapLines - Create a (projected) grid represented as a
 *          MapLines from a GridInfo.
 * INPUTS:  const GridInfo*      gridInfo        The grid to draw.
 *                MapLines*      projectedGridLines The grid lines.
 * OUTPUTS:       MapLines*      projectedGridLines The grid lines.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

static void gridToMapLines ( const GridInfo* gridInfo,
                             MapLines* projectedGridLines )
    {
    PRE4 ( isValidGridInfo ( gridInfo ),
           projectedGridLines->starts, projectedGridLines->lengths,
           projectedGridLines->vertices );

    const int x = gridInfo->gridType == LAT_LON_GRID ? LON : X;
    const int y = gridInfo->gridType == LAT_LON_GRID ? LAT : Y;
    const int numberOfVerticalLines   = gridInfo->dimensions[ x ] + 1;
    const int numberOfHorizontalLines = gridInfo->dimensions[ y ] + 1;

    projectLongitudeLines ( gridInfo, projectedGridLines );

    projectLatitudeLines (  gridInfo, projectedGridLines );

    projectedGridLines->polylineCount = numberOfHorizontalLines +
                                        numberOfVerticalLines;

    projectedGridLines->vertexCount   = numberOfHorizontalLines *
                                        numberOfVerticalLines * 2;

    computeMapLinesCorners ( projectedGridLines );

    POST ( isValidMapLines ( projectedGridLines ) );
    }


/******************************************************************************
 * PURPOSE: adjustLon - Translates a longitude back into the range [-180, 180].
 * INPUTS:  double longitude  The longitude value to translate.
 * OUTPUTS: None
 * RETURNS: double The longitude translated into the range [-180, 180].
 * NOTES:
 *****************************************************************************/

static double adjustLon ( double longitude )
    {
    while ( longitude >  180.0 ) longitude -= 360.0;
    while ( longitude < -180.0 ) longitude += 360.0;

    POST ( IN_RANGE ( longitude, -180.0, 180.0 ) );

    return longitude;
    }

