/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: MapServer.cc 83 2018-03-12 19:24:33Z coats $
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
///////////////////////////////////////////////////
//
// MapServer.cc
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 13, 1996
//
///////////////////////////////////////////////////
//
//    MapServer Class
//
//        1. Creates map list
//        2. Adds new maps to list when necessary
//        3. Returns pointers to map line info when requested
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  960513  Implemented
// SRT  960514  Added Todd Plessel's (EPA/MMTSI) M3IO parameter checking, which
//       was borrowed from /project/models3/unofficial/development/
//      Models3Vis/code/VisualizationSubsystem/src/libs/DataImport/
//      DataImport.cc
//
///////////////////////////////////////////////////

#include <iostream>

#include "MapServer.h"

///////////////////////////////////////////////////
//
//   MapServer::MapServer()
//
///////////////////////////////////////////////////
MapServer::MapServer()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter MapServer::MapServer()\n" );
#endif // DIAGNOSTICS

    mapList_ = new linkedList;
    }



///////////////////////////////////////////////////
//
//   MapServer::~MapServer()
//
///////////////////////////////////////////////////
MapServer::~MapServer()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter MapServer::~MapServer()\n" );
#endif // DIAGNOSTICS

    if ( mapList_ ) delete mapList_;
    mapList_ = NULL;
    }




///////////////////////////////////////////////////
//
//   MapServer::map_overlay() - returns 1 for success, 0 for failure
//
///////////////////////////////////////////////////
int MapServer::map_overlay (   VIS_DATA *info,
                               float **xpts,
                               float **ypts,
                               int **n,
                               int *npolyline,
                               char *mapName,
                               char *message )
    {
    Map   *map;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter MapServer::map_overlay() with info->dataset == " );
    if ( info->dataset == UAM_DATA ) fprintf ( stderr, "UAM_DATA\n" );
    else if ( info->dataset == UAMV_DATA ) fprintf ( stderr, "UAMV_DATA\n" );
    else if ( info->dataset == netCDF_DATA ) fprintf ( stderr, "netCDF_DATA\n" );
    else
        fprintf ( stderr, "UNKNOWN type\n" );
#endif // DIAGNOSTICS

    map = generateMap ( info, mapName, message );
    if ( !map )
        {
        sprintf ( message, "Cannot generate map in MapServer::map_overlay()!" );
        return 0;
        }


    *xpts = map->get_xpts();
    *ypts = map->get_ypts();
    *n = map->get_npts();
    *npolyline = map->get_npolyline();

    if ( !map->isMapValid ( message ) )
        return 0;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Returning successfully from MapServer::map_overlay()!!\n" );
#endif // DIAGNOSTICS
    return 1;
    }

///////////////////////////////////////////////////
///////////////////////////////////////////////////

Map *MapServer::generateMap ( VIS_DATA *info, char *mapName, char *message )
    {
    enum    { UNUSED = -1 };
    void    *target[2];
    int    grid_type, ncol, nrow, utm_zone;
    float    llx, lly, urx, ury, xorig, yorig, xcell,
             ycell, xcent, ycent, p_gam, p_bet, p_alp;
    M3IOParameters    params;
    Map   *map;

    // verify that we have a mapList to work with
    if ( !mapList_ )
        {
        sprintf ( message, "No mapList in MapServer::map_overlay()!" );
        return ( Map * ) NULL;
        }

    // build the parameter list for this map
    // AME: changed default corners of map
    //params.corners[LOWER][LAT] =   10.0;  // "pre-clip"
    //params.corners[LOWER][LON] = -125.0;  // the map to
    //params.corners[UPPER][LAT] =   80.0;  // the U.S.
    //params.corners[UPPER][LON] =  -60.0;  // domain

    params.corners[LOWER][LAT] =   -90.0;  // "pre-clip"
    params.corners[LOWER][LON] =   -180.0;  // the map to
    params.corners[UPPER][LAT] =   90.0;  // the whole
    params.corners[UPPER][LON] =  180.0;  // world

    /* as per Todd Plessel
       The next two are used to specify the ellipse used for
       the globe.  Typical values to use for these are (for
       data based on MM5, such as MCIP, RADM, etc):
       ELLIPSE          0 (sphere)
       RADIUS     6370997 (meters)

       or for other files, the following is presumed to be
       the most accurate representation (we'll use this one for now)
       ELLIPSE MERIT_1983 (see MapProjections.h for a list of the ellipses)
       RADIUS           0
    */
    params.ellipse = MERIT_1983;
    params.radius = 0;

    if ( sscanf ( info->map_info, "%d%g%g%g%g%g%g%g%g%g%d%d",
                  &grid_type, &xorig, &yorig, &xcell,
                  &ycell, &xcent, &ycent, &p_gam,
                  &p_bet, &p_alp, &ncol, &nrow ) == 12 )
        {
        params.gdtyp  = grid_type;
        params.p_alp  = p_alp;
        params.p_bet  = p_bet;
        params.p_gam  = p_gam;
        params.xcent  = xcent;
        params.ycent  = ycent;
        params.nrows  = nrow;
        params.ncols  = ncol;
        params.xorig  = xorig;
        params.yorig  = yorig;
        params.xcell  = xcell;
        params.ycell  = ycell;
        }
    else if ( sscanf ( info->map_info, "%g%g%g%g%d%d%d",
                       &llx, &lly, &urx, &ury, &utm_zone, &ncol, &nrow ) == 7 )
        {
        // cheesy way to figure out LatLon vs UTM
        if ( ( lly < 90. ) && ( lly>0. ) && ( ury<90. ) && ( ury>0. ) )
            {
            params.gdtyp  = LATGRD3;
            params.p_alp  = UNUSED;
            params.xcent  = UNUSED;
            params.ycent  = UNUSED;
            }
        else
            {
            params.gdtyp  = UTMGRD3;
            params.p_alp  = utm_zone;
            params.xcent  = 0;
            params.ycent  = 0;
            }

        params.p_bet  = UNUSED;
        params.p_gam  = UNUSED;
        params.nrows  = nrow;
        params.ncols  = ncol;
        params.xorig  = llx;
        params.yorig  = lly;
        params.xcell  = ( urx-llx ) /ncol;
        params.ycell  = ( ury-lly ) /nrow;
        }
    else
        {
        sprintf ( message, "Unknown dataset type %d!\n", info->dataset );
        return 0;
        }

    // check to see if user is overriding any of the params with env vars
    environmentVariableCheck ( &params );

    //
    // This row/col increment is done because PAVE draws a map
    // for entire edge cells, while the MapUtilities library
    // returns a map only to the center of the outermost cells
    //
    // NOTE:  if gdtyp is a LATGRD3=lat/lon projection and
    // adding one row would go above 90 degrees,
    // then the nrows is not incremented to avoid an assertion
    // failure in the MapUtilities library - HACK SRT 970918
    //
    // NOTE:  if gdtyp is a LATGRD3=lat/lon projection and
    // adding one col would go more than around the world,
    // then the ncols is not incremented to avoid an assertion
    // failure in the MapUtilities library - HACK SRT 970918
    //
    if (
        ! (
            ( params.gdtyp==LATGRD3 ) &&
            ( params.yorig+params.ycell* ( params.nrows+1 ) >90.0 )
        )
    )
        params.nrows++;
    if (
        ! (
            ( params.gdtyp==LATGRD3 ) &&
            ( params.xcell* ( params.ncols+1 ) >360.0 )
        )
    )
        params.ncols++;

    // see if we already have a copy of this map on the mapList
    target[0] = mapName;
    target[1] = ( void * ) &params;
    if ( ! ( map = ( Map * ) mapList_->find ( target ) ) )
        // we need to create a this map since we haven't done so yet
        {
        if ( ! ( map = new Map ( mapName, &params ) ) )
            {
            sprintf ( message,
                      "Couldn't create Map in MapServer::map_overlay()!\n" );
            return ( Map * ) NULL;
            }
        mapList_->addTail ( map );
        }

#ifdef DIAGNOSTICS
    map->print ( stdout );
#endif // #ifdef DIAGNOSTICS

    return map;
    }


///////////////////////////////////////////////////
//
//   MapServer::environmentVariableCheck()
//   checks whether user wants to override any of the params
//   and if so, writes over those params
//
///////////////////////////////////////////////////
void MapServer::environmentVariableCheck ( M3IOParameters *params )
    {
    if ( !params ) return;

    char newEllipse[30];
    int found = 0;
#ifdef DIAGNOSTICS
    char gdtyp[10];
    int  gdtypi = params->gdtyp;
    fprintf ( stderr, "--------------------------------------------------------\n" );
    fprintf ( stderr, "Enter MapServer::environmentVariableCheck() with params:\n" );
    fprintf ( stderr, "--------------------------------------------------------\n" );
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
    fprintf ( stderr, "params->gdtyp == %d == %s\n", params->gdtyp, gdtyp );
    fprintf ( stderr, "params->p_alp == %f\n", params->p_alp );
    fprintf ( stderr, "params->p_bet == %f\n", params->p_bet );
    fprintf ( stderr, "params->p_gam == %f\n", params->p_gam );
    fprintf ( stderr, "params->xcent == %f\n", params->xcent );
    fprintf ( stderr, "params->ycent == %f\n", params->ycent );
    fprintf ( stderr, "params->nrows == %d\n", params->nrows );
    fprintf ( stderr, "params->ncols == %d\n", params->ncols );
    fprintf ( stderr, "params->xorig == %f\n", params->xorig );
    fprintf ( stderr, "params->yorig == %f\n", params->yorig );
    fprintf ( stderr, "params->xcell == %f\n", params->xcell );
    fprintf ( stderr, "params->ycell == %f\n", params->ycell );
    fprintf ( stderr, "params->corners[LOWER][LAT] == %f\n", params->corners[LOWER][LAT] );
    fprintf ( stderr, "params->corners[UPPER][LAT] == %f\n", params->corners[UPPER][LAT] );
    fprintf ( stderr, "params->corners[LOWER][LON] == %f\n", params->corners[LOWER][LON] );
    fprintf ( stderr, "params->corners[UPPER][LON] == %f\n", params->corners[UPPER][LON] );
#endif // DIAGNOSTICS

    if ( getenv ( "PRECLIP_LLLAT" ) != NULL )
        {
        fprintf ( stderr,
                  "\nUsing your pre-clip [LOWER][LAT]==%g\n"
                  "      instead of the the default==%g\n",
                  atof ( getenv ( "PRECLIP_LLLAT" ) ),
                  params->corners[LOWER][LAT] );
        params->corners[LOWER][LAT]=atof ( getenv ( "PRECLIP_LLLAT" ) );
        }

    if ( getenv ( "PRECLIP_LLLON" ) != NULL )
        {
        fprintf ( stderr,
                  "\nUsing your pre-clip [LOWER][LON]==%g\n"
                  "      instead of the the default==%g\n",
                  atof ( getenv ( "PRECLIP_LLLON" ) ),
                  params->corners[LOWER][LON] );
        params->corners[LOWER][LON]=atof ( getenv ( "PRECLIP_LLLON" ) );
        }

    if ( getenv ( "PRECLIP_URLAT" ) != NULL )
        {
        fprintf ( stderr,
                  "\nUsing your pre-clip [UPPER][LAT]==%g\n"
                  "      instead of the the default==%g\n",
                  atof ( getenv ( "PRECLIP_URLAT" ) ),
                  params->corners[UPPER][LAT] );
        params->corners[UPPER][LAT]=atof ( getenv ( "PRECLIP_URLAT" ) );
        }

    if ( getenv ( "PRECLIP_URLON" ) != NULL )
        {
        fprintf ( stderr,
                  "\nUsing your pre-clip [UPPER][LON]==%g\n"
                  "      instead of the the default==%g\n",
                  atof ( getenv ( "PRECLIP_URLON" ) ),
                  params->corners[UPPER][LON] );
        params->corners[UPPER][LON]=atof ( getenv ( "PRECLIP_URLON" ) );
        }

    if ( getenv ( "P_ALP" ) != NULL )
        {
        fprintf ( stderr,
                  "\nReplacing your data's P_ALP==%g\n"
                  "by your environment variable's P_ALP==%g\n",
                  params->p_alp, atof ( getenv ( "P_ALP" ) ) );
        params->p_alp = atof ( getenv ( "P_ALP" ) );
        }

    if ( getenv ( "P_BET" ) != NULL )
        {
        fprintf ( stderr,
                  "\nReplacing your data's P_BET==%g\n"
                  "by your environment variable's P_BET==%g\n",
                  params->p_bet, atof ( getenv ( "P_BET" ) ) );
        params->p_bet = atof ( getenv ( "P_BET" ) );
        }

    if ( getenv ( "P_GAM" ) != NULL )
        {
        fprintf ( stderr,
                  "\nReplacing your data's P_GAM==%g\n"
                  "by your environment variable's P_GAM==%g\n",
                  params->p_gam, atof ( getenv ( "P_GAM" ) ) );
        params->p_gam = atof ( getenv ( "P_GAM" ) );
        }

    if ( getenv ( "XCENT" ) != NULL )
        {
        fprintf ( stderr,
                  "\nReplacing your data's XCENT==%g\n"
                  "by your environment variable's XCENT==%g\n",
                  params->xcent, atof ( getenv ( "XCENT" ) ) );
        params->xcent = atof ( getenv ( "XCENT" ) );
        }

    if ( getenv ( "YCENT" ) != NULL )
        {
        fprintf ( stderr,
                  "\nReplacing your data's YCENT==%g\n"
                  "by your environment variable's YCENT==%g\n",
                  params->ycent, atof ( getenv ( "YCENT" ) ) );
        params->ycent = atof ( getenv ( "YCENT" ) );
        }

    if ( getenv ( "XORIG" ) != NULL )
        {
        fprintf ( stderr,
                  "\nReplacing your data's XORIG==%g\n"
                  "by your environment variable's XORIG==%g\n",
                  params->xorig, atof ( getenv ( "XORIG" ) ) );
        params->xorig = atof ( getenv ( "XORIG" ) );
        }

    if ( getenv ( "YORIG" ) != NULL )
        {
        fprintf ( stderr,
                  "\nReplacing your data's YORIG==%g\n"
                  "by your environment variable's YORIG==%g\n",
                  params->yorig, atof ( getenv ( "YORIG" ) ) );
        params->yorig = atof ( getenv ( "YORIG" ) );
        }

    if ( getenv ( "XCELL" ) != NULL )
        {
        fprintf ( stderr,
                  "\nReplacing your data's XCELL==%g\n"
                  "by your environment variable's XCELL==%g\n",
                  params->xcell, atof ( getenv ( "XCELL" ) ) );
        params->xcell = atof ( getenv ( "XCELL" ) );
        }

    if ( getenv ( "YCELL" ) != NULL )
        {
        fprintf ( stderr,
                  "\nReplacing your data's YCELL==%g\n"
                  "by your environment variable's YCELL==%g\n",
                  params->ycell, atof ( getenv ( "YCELL" ) ) );
        params->ycell = atof ( getenv ( "YCELL" ) );
        }

    if ( getenv ( "NROWS" ) != NULL ) // added 961002 SRT
        {
        fprintf ( stderr,
                  "\nReplacing your data's NROWS==%d\n"
                  "by your environment variable's NROWS==%d\n",
                  params->nrows, atoi ( getenv ( "NROWS" ) ) );
        params->nrows = atoi ( getenv ( "NROWS" ) );
        }

    if ( getenv ( "NCOLS" ) != NULL ) // added 961002 SRT
        {
        fprintf ( stderr,
                  "\nReplacing your data's NCOLS==%d\n"
                  "by your environment variable's NCOLS==%d\n",
                  params->ncols, atoi ( getenv ( "NCOLS" ) ) );
        params->ncols = atoi ( getenv ( "NCOLS" ) );
        }

    if ( getenv ( "GDTYP" ) != NULL ) // added 961002 SRT
        {
        fprintf ( stderr,
                  "\nReplacing your data's GDTYP==%d\n"
                  "by your environment variable's GDTYP==%d\n",
                  params->gdtyp, atoi ( getenv ( "GDTYP" ) ) );
        params->gdtyp = atoi ( getenv ( "GDTYP" ) );
        }
    if ( getenv ( "SPHERICAL_EARTH" ) != NULL ) // added 000511 ALT
        {
        params->ellipse = 0;
        params->radius = 6370997;
        params->radius = atof ( getenv ( "SPHERICAL_EARTH" ) );
        if ( params->radius == 1 )
            {
            params->radius = 6370997;
            fprintf ( stdout,"Using spherical earth with default radius = %.2f m\n",
                      params->radius );
            }
        else
            {
            fprintf ( stdout,
                      "Using spherical earth with radius = %.2f (default is 6370997m)\n",
                      params->radius );
            }
        }
    else if ( getenv ( "ELLIPSOID" ) != NULL ) // AME
        {
        strcpy ( newEllipse,getenv ( "ELLIPSOID" ) );
        if ( strcmp ( newEllipse,"MERIT_1983" ) == 0 )
            {
            params->ellipse = MERIT_1983;
            found = 1;
            }
        else if ( strcmp ( newEllipse,"SGS_85" ) == 0 )
            {
            params->ellipse = SGS_85;
            found = 1;
            }
        else if ( strcmp ( newEllipse,"GRS_1980" ) == 0 )
            {
            params->ellipse = GRS_1980;
            found = 1;
            }
        else if ( strcmp ( newEllipse,"IAU_1976" ) == 0 )
            {
            params->ellipse = IAU_1976;
            found = 1;
            }
        else if ( strcmp ( newEllipse,"CLARKE_1866" ) == 0 )
            {
            params->ellipse = CLARKE_1866;
            found = 1;
            }
        else if ( strcmp ( newEllipse,"CLARKE_1880" ) == 0 )
            {
            params->ellipse = CLARKE_1880;
            found = 1;
            }
        else if ( strcmp ( newEllipse,"ENGELIS_1985" ) == 0 )
            {
            params->ellipse = ENGELIS_1985;
            found = 1;
            }
        else if ( strcmp ( newEllipse,"EVEREST_1969" ) == 0 )
            {
            params->ellipse = EVEREST_1969;
            found = 1;
            }
        else if ( strcmp ( newEllipse,"WGS_60" ) == 0 )
            {
            params->ellipse = WGS_60;
            found = 1;
            }
        else if ( strcmp ( newEllipse,"WGS_66" ) == 0 )
            {
            params->ellipse = WGS_66;
            found = 1;
            }
        else if ( strcmp ( newEllipse,"WGS_72" ) == 0 )
            {
            params->ellipse = WGS_72;
            found = 1;
            }
        else if ( strcmp ( newEllipse,"WGS_84" ) == 0 )
            {
            params->ellipse = WGS_84;
            found = 1;
            }
        if ( found )
            {
            fprintf ( stdout,
                      "\nReplacing Earth's ellipsoid with %s\n", newEllipse );
            params->radius = 0;
            }
        else
            {
            fprintf ( stderr,
                      "\nCould not find ellipsoid %s, use one of \n", newEllipse );
            fprintf ( stderr,
                      "MERIT_1983, SGS_85, GRS_1980, IAU_1976, CLARKE_1866, CLARKE_1880\n" );
            fprintf ( stderr,
                      "ENGELIS_1985, EVEREST_1969, WGS_60, WGS_66, WGS_72, WGS_84\n" );
            }
        }
    else if ( getenv ( "GRS80" ) != NULL ) // added 000511 ALT
        {
        fprintf ( stderr,
                  "\nReplacing Earth's ellipsoid with GRS80 (%d)\n",GRS_1980 );
        params->ellipse = GRS_1980;
        params->radius = 0;
        }
    }



// C "Helper" functions as per Steve Fine's suggestion.
//
// These enable us to replace an original C map_overlay()
// function with this map_overlay() function, which relies
// on C++ objects rather than C code to do the guts of the
// work.  In this way we don't have to modify any of the
// code that calls map_overlay(), and yet we get the
// benefits of incorporating OO techniques with Lockheed/Martin's
// new DrawMap library.

static   MapServer *MapServer_ = NULL;

///////////////////////////////////////////////////
//
//   init_projmaps() - returns 1 if error
//
///////////////////////////////////////////////////
int init_projmaps ( char *message )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter init_projmaps()\n" );
#endif // DIAGNOSTICS

    MapServer_ = new MapServer;
    if ( !MapServer_ )
        {
        sprintf ( message, "Couldn't allocate MapServer_ in init_projmaps()!" );
        return 1;
        }
    return 0;
    }



///////////////////////////////////////////////////
//
//   projmap_overlay() - returns 1 if success, 0 if failure
//
///////////////////////////////////////////////////
int projmap_overlay ( VIS_DATA *info,
                      float **xpts,   // NOTE:  YOU SHOULDN'T FREE THIS SPACE YOURSELF
                      float **ypts,   // NOTE:  YOU SHOULDN'T FREE THIS SPACE YOURSELF
                      int **n,        // NOTE:  YOU SHOULDN'T FREE THIS SPACE YOURSELF
                      int *npolyline,
                      char *mapName,
                      char *message )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter projmap_overlay()\n" );
#endif // DIAGNOSTICS

    if ( !MapServer_ )
        {
        sprintf ( message, "No MapServer_ in projmap_overlay()!" );
        return 0;
        }

    return ( !MapServer_->map_overlay ( info, xpts, ypts, n, npolyline,
                                        mapName, message ) );
    }

///////////////////////////////////////////////////
//
//   projmap_generate() - returns 1 if success, 0 if failure
//
///////////////////////////////////////////////////
Map *projmap_generate ( VIS_DATA *info,
                        char *mapName,
                        char *message )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter projmap_generate()\n" );
#endif // DIAGNOSTICS

    if ( !MapServer_ )
        {
        sprintf ( message, "No MapServer_ in projmap_generate()!" );
        return ( Map * ) NULL;
        }

    return ( MapServer_->generateMap ( info, mapName, message ) );
    }


///////////////////////////////////////////////////
//
//   cleanup_projmaps()
//
///////////////////////////////////////////////////
int cleanup_projmaps ( char *message )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter cleanup_projmaps()\n" );
#endif // DIAGNOSTICS

    if ( MapServer_ )
        {
        delete MapServer_;
        MapServer_ = ( MapServer * ) NULL;
        }
    else
        {
        sprintf ( message, "No MapServer_ to free in cleanup_projmaps()!" );
        return 1;
        }
    return 0;
    }

