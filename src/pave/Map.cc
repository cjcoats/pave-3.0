/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Map.cc 83 2018-03-12 19:24:33Z coats $
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
 ****************************************************************************/
/////////////////////////////////////////////////////////////
//
// Map.cc
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 13, 1996
//
/////////////////////////////////////////////////////////////
//
//   Every Map Object:
//
// o Must supply its (in)validity when reqested
//
// o Must retrieve and supply a copy of its data when requested
//
// o Needs to request info from Lockheed/Martin's DrawMap library
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT  960513  Implemented
//
/////////////////////////////////////////////////////////////

#include "Map.h"

static int is_reasonably_equal ( double p, double q );


///////////////////////////////////////////////////
//
//      Map::Map()
//
///////////////////////////////////////////////////
Map::Map    (
    const char *mapFileName,
    const M3IOParameters* parameters
)
    {
    MapLines *mapLines;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Map::Map('%s') with:\n"
              "gdtyp=%d\n"
              "p_alp=%g\n"
              "p_bet=%g\n"
              "p_gam=%g\n"
              "xcent=%g\n"
              "ycent=%g\n"
              "nrows=%d\n"
              "ncols=%d\n"
              "xorig=%g\n"
              "yorig=%g\n"
              "xcell=%g\n"
              "ycell=%g\n"
              "corners[LOWER][LAT]=%g\n"
              "corners[LOWER][LON]=%g\n"
              "corners[UPPER][LAT]=%g\n"
              "corners[UPPER][LON]=%g\n",
              mapFileName,
              parameters->gdtyp,
              parameters->p_alp,
              parameters->p_bet,
              parameters->p_gam,
              parameters->xcent,
              parameters->ycent,
              parameters->nrows,
              parameters->ncols,
              parameters->xorig,
              parameters->yorig,
              parameters->xcell,
              parameters->ycell,
              parameters->corners[LOWER][LAT],
              parameters->corners[LOWER][LON],
              parameters->corners[UPPER][LAT],
              parameters->corners[UPPER][LON] );
#endif // DIAGNOSTICS

    validMap_ = unsure_or_no;
    mapFileName_ = NULL;
    xpts_ = NULL;
    ypts_ = NULL;
    npts_ = NULL;
    npolyline_ = 0;

    assert ( mapFileName );
    assert ( parameters );
    if ( !areValidM3IOParameters ( parameters ) )
        {
        fprintf ( stderr, "Invalid parameters supplied to Map::Map()!\n" );
        return;
        }
    memcpy ( &parameters_, parameters, sizeof ( M3IOParameters ) );
    mapFileName_ = strdup ( mapFileName );
    if ( !mapFileName_ )
        {
        fprintf ( stderr, "strdup() failed in Map::Map()!!\n" );
        return;
        }


    if ( mapLines = createProjectedMapLinesClippedToGrid ( mapFileName_, ( char * ) 0, &parameters_ ) )
        {
        int     polyline, start, length, end, vertex, index;
        double  projectedXOrigin, projectedYOrigin;
        float   *v, origx, origy, dx, dy;

        // allocate space for our copy of the points/lines
        npolyline_ = mapLines->polylineCount;
        xpts_ = ( float * ) malloc ( sizeof ( float ) *mapLines->vertexCount );
        ypts_ = ( float * ) malloc ( sizeof ( float ) *mapLines->vertexCount );
        npts_ = ( int * ) malloc ( sizeof ( int ) *mapLines->polylineCount );
        if ( ( !xpts_ ) || ( !ypts_ ) || ( !npts_ ) )
            {
            if ( xpts_ ) free ( xpts_ );
            xpts_ = NULL;
            if ( ypts_ ) free ( ypts_ );
            ypts_ = NULL;
            if ( npts_ ) free ( npts_ );
            npts_ = NULL;
            npolyline_ = 0;
            fprintf ( stderr, "malloc() failure in Map::Map()!\n" );
            deallocateMapLines ( mapLines );
            return;
            }

        computeProjectedGridOrigin ( &parameters_,
                                     &projectedXOrigin,
                                     &projectedYOrigin );
        origx = ( float ) projectedXOrigin;
        origy = ( float ) projectedYOrigin;
        dx = parameters_.xcell;
        dy = parameters_.ycell;

        // now scale the map so TileWnd.cc can draw it right
        index = 0;
        for ( polyline = 0; polyline < mapLines->polylineCount; ++polyline )
            {
            start  = mapLines->starts[polyline];
            npts_[polyline] = length = mapLines->lengths[polyline];
            end    = start + length;
            v   = mapLines->vertices + 2 * start;
            for ( vertex = start; vertex < end; ++vertex, v += 2 )
                {
                xpts_[index] = ( v[X]-origx ) /dx;
                ypts_[index] = ( v[Y]-origy ) /dy;
                index++;
                }
            }

        validMap_ = yes;
        deallocateMapLines ( mapLines );
#ifdef DIAGNOSTICS
        print ( stderr );
#endif // #ifdef DIAGNOSTICS
        }
    }



///////////////////////////////////////////////////
//
//      Map::~Map()
//
///////////////////////////////////////////////////
Map::~Map()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Map::~Map('%s')\n", mapFileName_ );
#endif // DIAGNOSTICS

    if ( mapFileName_ )   free ( mapFileName_ );
    mapFileName_ = NULL;
    if ( xpts_ )      free ( xpts_ );
    xpts_ = NULL;
    if ( ypts_ )      free ( ypts_ );
    ypts_ = NULL;
    if ( npts_ )      free ( npts_ );
    npts_ = NULL;
    }



///////////////////////////////////////////////////
//
//      Map::match()
//
///////////////////////////////////////////////////
int Map::match ( void *target )
    {
    void        **targetpp;
    char        *fname;
    M3IOParameters  *params;

    targetpp = ( void ** ) target;
    assert ( targetpp );
    fname = ( char * ) targetpp[0];
    params = ( M3IOParameters * ) targetpp[1];

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Map::match() with params:\n" );
    fprintf ( stderr, "params->gdtyp == %d\n", params->gdtyp );
    fprintf ( stderr, "params->p_alp == %g\n", params->p_alp );
    fprintf ( stderr, "params->p_bet == %g\n", params->p_bet );
    fprintf ( stderr, "params->p_gam == %g\n", params->p_gam );
    fprintf ( stderr, "params->xcent == %g\n", params->xcent );
    fprintf ( stderr, "params->ycent == %g\n", params->ycent );
    fprintf ( stderr, "params->nrows == %d\n", params->nrows );
    fprintf ( stderr, "params->ncols == %d\n", params->ncols );
    fprintf ( stderr, "params->xorig == %g\n", params->xorig );
    fprintf ( stderr, "params->yorig == %g\n", params->yorig );
    fprintf ( stderr, "params->xcell == %g\n", params->xcell );
    fprintf ( stderr, "params->ycell == %g\n", params->ycell );
    fprintf ( stderr, "params->corners[0][0] == %g\n", params->corners[0][0] );
    fprintf ( stderr, "params->corners[1][0] == %g\n", params->corners[1][0] );
    fprintf ( stderr, "params->corners[0][1] == %g\n", params->corners[0][1] );
    fprintf ( stderr, "params->corners[1][1] == %g\n", params->corners[1][1] );
#endif // DIAGNOSTICS

    if ( !fname ) return 0;

    if ( !mapFileName_ ) return 0;

    if ( strcmp ( fname, mapFileName_ ) ) return 0;

    if ( params->gdtyp != parameters_.gdtyp ) return 0;

    if ( params->ncols != parameters_.ncols ) return 0;

    if ( params->nrows != parameters_.nrows ) return 0;

    switch ( params->gdtyp )
        {
        case LAMGRD3:
        case ALBGRD3:
            if ( !is_reasonably_equal ( params->p_bet, parameters_.p_bet ) )
                return 0;

            if ( !is_reasonably_equal ( params->p_gam, parameters_.p_gam ) )
                return 0;

        case UTMGRD3:
            if ( !is_reasonably_equal ( params->p_alp, parameters_.p_alp ) )
                return 0;

            if ( !is_reasonably_equal ( params->xcent, parameters_.xcent ) )
                return 0;

            if ( !is_reasonably_equal ( params->ycent, parameters_.ycent ) )
                return 0;

        case LATGRD3:
            if ( !is_reasonably_equal ( params->xorig, parameters_.xorig ) )
                return 0;

            if ( !is_reasonably_equal ( params->yorig, parameters_.yorig ) )
                return 0;

            if ( !is_reasonably_equal ( params->xcell, parameters_.xcell ) )
                return 0;

            if ( !is_reasonably_equal ( params->ycell, parameters_.ycell ) )
                return 0;

            if ( !is_reasonably_equal (    params->corners[0][0],
                                           parameters_.corners[0][0] ) )
                return 0;

            if ( !is_reasonably_equal (    params->corners[0][1],
                                           parameters_.corners[0][1] ) )
                return 0;

            if ( !is_reasonably_equal (    params->corners[1][0],
                                           parameters_.corners[1][0] ) )
                return 0;

            if ( !is_reasonably_equal (    params->corners[1][1],
                                           parameters_.corners[1][1] ) )
                return 0;
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Map::match('%s') about to return 1\n", fname );
#endif // DIAGNOSTICS

    return 1;
    }



///////////////////////////////////////////////////
//
//      Map::getClassName()
//
///////////////////////////////////////////////////
char *Map::getClassName ( void )
    {
    static char *myName = "Map";
    return myName;
    }



///////////////////////////////////////////////////
//
//      Map::print()
//
///////////////////////////////////////////////////
void Map::print ( FILE *output )
    {
    char gdtyp[10];
    int  gdtypi = parameters_.gdtyp;

    switch ( gdtypi )
        {
        case LATGRD3:
            sprintf ( gdtyp, "LATGRD3" );
            break;
        case LAMGRD3:
            sprintf ( gdtyp, "LAMGRD3" );
            break;
        case ALBGRD3:
            sprintf ( gdtyp, "ALBGRD3" );
            break;
        case MERGRD3:
            sprintf ( gdtyp, "MERGRD3" );
            break;
        case STEGRD3:
            sprintf ( gdtyp, "STEGRD3" );
            break;
        case UTMGRD3:
            sprintf ( gdtyp, "UTMGRD3" );
            break;
        case POLGRD3:
            sprintf ( gdtyp, "POLGRD3" );
            break;
        case TRMGRD3:
            sprintf ( gdtyp, "TRMGRD3" );
            break;
        case EQMGRD3:
            sprintf ( gdtyp, "EQMGRD3" );
            break;
        case LEQGRD3:
            sprintf ( gdtyp, "LEQGRD3" );
            break;
        default:
            sprintf ( gdtyp, "UNKNOWN" );
        }
    fprintf ( output, "---------\n" );
    fprintf ( output, "Map print\n" );
    fprintf ( output, "---------\n" );
    fprintf ( output, "parameters_.gdtyp == %d == %s\n", parameters_.gdtyp, gdtyp );
    fprintf ( output, "parameters_.p_alp == %f\n", parameters_.p_alp );
    fprintf ( output, "parameters_.p_bet == %f\n", parameters_.p_bet );
    fprintf ( output, "parameters_.p_gam == %f\n", parameters_.p_gam );
    fprintf ( output, "parameters_.xcent == %f\n", parameters_.xcent );
    fprintf ( output, "parameters_.ycent == %f\n", parameters_.ycent );
    fprintf ( output, "parameters_.nrows == %d\n", parameters_.nrows );
    fprintf ( output, "parameters_.ncols == %d\n", parameters_.ncols );
    fprintf ( output, "parameters_.xorig == %f\n", parameters_.xorig );
    fprintf ( output, "parameters_.yorig == %f\n", parameters_.yorig );
    fprintf ( output, "parameters_.xcell == %f\n", parameters_.xcell );
    fprintf ( output, "parameters_.ycell == %f\n", parameters_.ycell );
    fprintf ( output, "parameters_.corners[LOWER][LAT] == %f\n", parameters_.corners[LOWER][LAT] );
    fprintf ( output, "parameters_.corners[UPPER][LAT] == %f\n", parameters_.corners[UPPER][LAT] );
    fprintf ( output, "parameters_.corners[LOWER][LON] == %f\n", parameters_.corners[LOWER][LON] );
    fprintf ( output, "parameters_.corners[UPPER][LON] == %f\n", parameters_.corners[UPPER][LON] );
    fprintf ( output, "mapFileName_ == '%s'\n", mapFileName_ );
    fprintf ( output, "npolyline_ == %d\n", npolyline_ );
    fprintf ( output, "validMap_ == %s\n", validMap_ == yes ? "YES" : "NO" );
    }



///////////////////////////////////////////////////
//
//      Map::isMapValid()
//
///////////////////////////////////////////////////
int Map::isMapValid ( char *estring )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Map::isMapValid('%s')\n", mapFileName_ );
#endif // DIAGNOSTICS

    if ( ! ( validMap_ == yes ) )
        sprintf ( estring, "Map is invalid !!" );

    return ( validMap_ == yes );
    }



/************************************************************
is_reasonably_equal
************************************************************/
static int is_reasonably_equal ( double p, double q )
    {
    double  e = 10e-10,
            lhs = ( p-q ) * ( p-q ),
            rhs = e* ( p*p+q*q )+e;

    return ( lhs < rhs );
    }


void Map::copyParameters ( M3IOParameters *params )
    {

    params->gdtyp   = parameters_.gdtyp;
    params->p_alp   = parameters_.p_alp;
    params->p_bet   = parameters_.p_bet;
    params->p_gam   = parameters_.p_gam;
    params->xcent   = parameters_.xcent;
    params->ycent   = parameters_.ycent;
    params->nrows   = parameters_.nrows;
    params->ncols   = parameters_.ncols;
    params->xorig   = parameters_.xorig;
    params->yorig   = parameters_.yorig;
    params->xcell   = parameters_.xcell;
    params->ycell   = parameters_.ycell;
    params->corners[0][0] = parameters_.corners[0][0];
    params->corners[0][1] = parameters_.corners[0][1];
    params->corners[1][0] = parameters_.corners[1][0];
    params->corners[1][1] = parameters_.corners[1][1];
    params->ellipse = parameters_.ellipse;
    params->radius  = parameters_.radius;
    }

