/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: map_overlay.c 83 2018-03-12 19:24:33Z coats $
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
 *  REVISION HISTORY
 *      Created 1994?? by ??
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0:
 *      In-line "clip.c"
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#ifdef _AIX
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif /* ifdef _AIX */

#include "vis_data.h"

#define PAVE_SUCCESS (1)
#define FAILURE      (0)
#define YES          (1)
#define NO           (0)

#define LAMBERT_FILENAME  "lambert/mm4d.sized"
#define LAMBERT_FILENAME2 "lambert/mm4d_nolatlon.sized"
#define UTM_CO_FILENAME   "utm/counties.arc"
#define UTM_ST_FILENAME   "utm/states.arc"

static float xminedge;
static float yminedge;
static float xmaxedge;
static float ymaxedge;

static int out1 = 0;
static int out2 = 0;
static float xx1 = 0.;
static float yy1 = 0.;
static float xx2 = 0.;
static float yy2 = 0.;

static char string[80];

/************   From old "clip.c" :    *****************/

static void outcode ( float xmin, float ymin, float xmax, float ymax )
    {
    out1 = out2 = 0;

    if ( yy1 > ymax )
        out1 |= YES << 3;
    if ( yy1 < ymin )
        out1 |= YES << 2;
    if ( xx1 > xmax )
        out1 |= YES << 1;
    if ( xx1 < xmin )
        out1 |= YES << 0;
    if ( yy2 > ymax )
        out2 |= YES << 3;
    if ( yy2 < ymin )
        out2 |= YES << 2;
    if ( xx2 > xmax )
        out2 |= YES << 1;
    if ( xx2 < xmin )
        out2 |= YES << 0;
    return;
    }

static int treject()
    {
    if ( ( out1 & out2 ) != 0 )
        return ( YES );
    else
        return ( NO );
    }

static int taccept()
    {
    if ( ( out1 == 0 ) && ( out2 == 0 ) )
        return ( YES );
    else
        return ( NO );
    }

static void swap()
    {
    float xtemp, ytemp;
    int   otemp;

    xtemp = xx1;
    ytemp = yy1;
    xx1   = xx2;
    yy1   = yy2;
    xx2   = xtemp;
    yy2   = ytemp;
    otemp = out1;
    out1  = out2;
    out2  = otemp;
    return;
    }

static int clip ( float *fx1, float *fy1, float *fx2, float *fy2,
                  float xmin, float ymin, float xmax, float ymax )
    {
    char accept, reject, done;

    xx1 = *fx1;
    yy1 = *fy1;
    xx2 = *fx2;
    yy2 = *fy2;

    accept = reject = done = NO;
    while ( !done )
        {
        outcode ( xmin, ymin, xmax, ymax );
        reject = treject();
        if ( reject )
            done = YES;
        else
            {
            accept = taccept();
            if ( accept )
                done = YES;
            else
                {
                if ( !out1 )
                    swap();
                if ( out1 & 010 )
                    {
                    xx1 = xx1 + ( xx2 - xx1 ) * ( ymax - yy1 ) / ( yy2 - yy1 );
                    yy1 = ymax;
                    }
                else if ( out1 & 04 )
                    {
                    xx1 = xx1 + ( xx2 - xx1 ) * ( ymin - yy1 ) / ( yy2 - yy1 );
                    yy1 = ymin;
                    }
                else if ( out1 & 02 )
                    {
                    yy1 = yy1 + ( yy2 - yy1 ) * ( xmax - xx1 ) / ( xx2 - xx1 );
                    xx1 = xmax;
                    }
                else if ( out1 & 01 )
                    {
                    yy1 = yy1 + ( yy2 - yy1 ) * ( xmin - xx1 ) / ( xx2 - xx1 );
                    xx1 = xmin;
                    }
                }
            }
        }
    if ( accept )
        {
        *fx1 = xx1;
        *fy1 = yy1;
        *fx2 = xx2;
        *fy2 = yy2;
        return ( 1 );
        }
    return ( 0 );
    }


int map_overlay ( VIS_DATA info, float *xpts, float *ypts, int *n,
                  int *npolyline, int maxpoints, int map_options, char *message )
    {

    int lambert_map ( float llx, float lly, float urx, float ury, int ncol, int nrow,
                      int clip_xmin, int clip_xmax, int clip_ymin, int clip_ymax,
                      float *xpts, float *ypts, int *n, int *npolyline, int maxpoints,
                      int map_options );
    int utm_map ( float llx, float lly, float urx, float ury, int utm_zone,
                  int ncol, int nrow,
                  int clip_xmin, int clip_xmax, int clip_ymin, int clip_ymax,
                  float *xpts, float *ypts, int *n, int *npolyline, int maxpoints,
                  int map_options );
    int latlon_map ( float llx, float lly, float urx, float ury, int utm_zone,
                     int ncol, int nrow,
                     int clip_xmin, int clip_xmax, int clip_ymin, int clip_ymax,
                     float *xpts, float *ypts, int *n, int *npolyline, int maxpoints,
                     int map_options );

    float llx, lly, urx, ury, dx, dy;
    int status;
    char tstring[256];

    /* netCDF header variables for map */
    int grid_type, ncol, nrow;
    float xorig, yorig, xcell, ycell, xcent, ycent, p_gam, p_bet, p_alp;

    /* unique variable for UTM map */
    int utm_zone;

    status = PAVE_SUCCESS;
    switch ( info.dataset )
        {
        case UAM_DATA:
        case UAMV_DATA:
            {
            if ( sscanf ( info.map_info, "%g%g%g%g%d%d%d",
                          &llx, &lly, &urx, &ury, &utm_zone, &ncol, &nrow ) == 7 )
                {
                /* cheesy way to figure out LatLon vs UTM */
                if ( ( lly < 90. ) && ( lly>0. ) && ( ury<90. ) && ( ury>0. ) )
                    {
                    if ( !latlon_map ( llx, lly, urx, ury, utm_zone,
                                       ncol, nrow,
                                       info.col_min - 1,
                                       ncol-info.col_max,
                                       info.row_min - 1,
                                       nrow-info.row_max,
                                       xpts, ypts, n, npolyline,
                                       maxpoints, map_options ) )
                        {
                        status = FAILURE;
                        sprintf ( message, "Failure to load map information." );
                        }
                    }

                else
                    {
                    llx /= 1000.0;
                    lly /= 1000.0;
                    urx /= 1000.0;
                    ury /= 1000.0;
                    if ( !utm_map ( llx, lly, urx, ury, utm_zone, ncol, nrow,
                                    info.col_min - 1,
                                    ncol-info.col_max,
                                    info.row_min - 1,
                                    nrow-info.row_max,
                                    xpts, ypts, n, npolyline,
                                    maxpoints, map_options ) )
                        {
                        status = FAILURE;
                        sprintf ( message, "Failure to load map information." );
                        }
                    }
                }
            else
                {
                status = FAILURE;
                sprintf ( message, "Error reading map info string" );
                }
            break;
            }
        case netCDF_DATA:
            {
            if ( sscanf ( info.map_info, "%d%g%g%g%g%g%g%g%g%g%d%d",
                          &grid_type, &xorig, &yorig, &xcell, &ycell, &xcent,
                          &ycent, &p_gam, &p_bet, &p_alp, &ncol, &nrow ) == 12 )
                {
                if ( getenv ( "P_ALP" ) != NULL )
                    {
                    fprintf ( stderr,
                              "\nReplacing your netCDF data's P_ALP==%g\n"
                              "by your environment variable's P_ALP==%g\n",
                              p_alp, atof ( getenv ( "P_ALP" ) ) );
                    p_alp = atof ( getenv ( "P_ALP" ) );
                    }

                if ( getenv ( "P_BET" ) != NULL )
                    {
                    fprintf ( stderr,
                              "\nReplacing your netCDF data's P_BET==%g\n"
                              "by your environment variable's P_BET==%g\n",
                              p_bet, atof ( getenv ( "P_BET" ) ) );
                    p_bet = atof ( getenv ( "P_BET" ) );
                    }

                if ( getenv ( "P_GAM" ) != NULL )
                    {
                    fprintf ( stderr,
                              "\nReplacing your netCDF data's P_GAM==%g\n"
                              "by your environment variable's P_GAM==%g\n",
                              p_gam, atof ( getenv ( "P_GAM" ) ) );
                    p_gam = atof ( getenv ( "P_GAM" ) );
                    }

                if ( getenv ( "XCENT" ) != NULL )
                    {
                    fprintf ( stderr,
                              "\nReplacing your netCDF data's XCENT==%g\n"
                              "by your environment variable's XCENT==%g\n",
                              xcent, atof ( getenv ( "XCENT" ) ) );
                    xcent = atof ( getenv ( "XCENT" ) );
                    }

                if ( getenv ( "YCENT" ) != NULL )
                    {
                    fprintf ( stderr,
                              "\nReplacing your netCDF data's YCENT==%g\n"
                              "by your environment variable's YCENT==%g\n",
                              ycent, atof ( getenv ( "YCENT" ) ) );
                    ycent = atof ( getenv ( "YCENT" ) );
                    }

                if ( getenv ( "XORIG" ) != NULL )
                    {
                    fprintf ( stderr,
                              "\nReplacing your netCDF data's XORIG==%g\n"
                              "by your environment variable's XORIG==%g\n",
                              xorig, atof ( getenv ( "XORIG" ) ) );
                    xorig = atof ( getenv ( "XORIG" ) );
                    }

                if ( getenv ( "YORIG" ) != NULL )
                    {
                    fprintf ( stderr,
                              "\nReplacing your netCDF data's YORIG==%g\n"
                              "by your environment variable's YORIG==%g\n",
                              yorig, atof ( getenv ( "YORIG" ) ) );
                    yorig = atof ( getenv ( "YORIG" ) );
                    }

                if ( getenv ( "XCELL" ) != NULL )
                    {
                    fprintf ( stderr,
                              "\nReplacing your netCDF data's XCELL==%g\n"
                              "by your environment variable's XCELL==%g\n",
                              xcell, atof ( getenv ( "XCELL" ) ) );
                    xcell = atof ( getenv ( "XCELL" ) );
                    }

                if ( getenv ( "YCELL" ) != NULL )
                    {
                    fprintf ( stderr,
                              "\nReplacing your netCDF data's YCELL==%g\n"
                              "by your environment variable's YCELL==%g\n",
                              ycell, atof ( getenv ( "YCELL" ) ) );
                    ycell = atof ( getenv ( "YCELL" ) );
                    }


                if ( ( p_alp != 30 ) ||
                        ( p_bet != 60 ) ||
                        ( p_gam != -90 ) ) /* HACK */
                    {
                    fprintf ( stderr,
                              "Resetting (p_alp, p_bet, p_gam) from "
                              "(%g, %g, %g) to (30, 60, -90) as per Talat Odman\n",
                              p_alp, p_bet, p_gam );
                    p_alp = 30;
                    p_bet = 60;
                    p_gam = -90;
                    }

                if ( ( grid_type != 2 /*1 SRT */ ) ||
                        ( xcent != -90 ) || ( ycent != 40 ) ||
                        ( p_alp != 30 ) || ( p_bet != 60 ) ||
                        ( p_gam != -90 ) )
                    {
                    /*status = FAILURE; */
                    sprintf ( message,
                              "Only Lambert map available is for\n"
                              "type 2=Lambert with cent (-90,40) & angle (30,60,-90),\n"
                              "and you are using\n"
                              "type %d=", grid_type );
                    switch ( grid_type )
                        {
                        case 1:
                            strcat ( message, "Lat/Lon" );
                            break;

                        case 2:
                            strcat ( message, "Lambert" );
                            break;

                        case 3:
                            strcat ( message, "Mercater" );
                            break;

                        case 4:
                            strcat ( message, "Stereographic" );
                            break;

                        case 5:
                            strcat ( message, "UTM" );
                            break;

                        default:
                            strcat ( message, "Unknown Type" );
                            break;
                        }
                    sprintf ( tstring,
                              " with cent (%g,%g) & angle (%g,%g,%g)\n",
                              xcent, ycent, p_alp, p_bet, p_gam );
                    strcat ( message, tstring );
                    fprintf ( stderr, message );
                    message[0]='\0';
                    fprintf ( stderr,
                              "But I'll try it anyway as per Talat Odman,\n"
                              "using PAVE_MAPDIR=%s\n",
                              getenv ( "PAVE_MAPDIR" ) );
                    }
                /*  else  */
                    {
                    llx = xorig;
                    lly = yorig;
                    urx = xorig + xcell * ncol;
                    ury = yorig + ycell * nrow;
                    if ( !lambert_map ( llx, lly, urx, ury,
                                        ncol, nrow,
                                        info.col_min - 1,
                                        ncol-info.col_max,
                                        info.row_min - 1,
                                        nrow-info.row_max,
                                        xpts, ypts, n, npolyline,
                                        maxpoints, map_options ) )
                        {
                        status = FAILURE;
                        sprintf ( message, "Failure to load map information." );
                        }
                    }
                }
            else
                {
                status = FAILURE;
                sprintf ( message, "Error reading map info string" );
                }
            break;
            }
        }
    return ( status );
    }

int lambert_map ( float llx, float lly, float urx, float ury, int ncol, int nrow,
                  int clip_xmin, int clip_xmax, int clip_ymin, int clip_ymax,
                  float *xpts, float *ypts, int *n, int *npolyline,
                  int maxpoints, int map_options )
    {
    register int i, j;
    int   segment;
    float xmin;
    float ymin;
    float xmax;
    float ymax;
    float xcellsize;
    float ycellsize;
    float x0;
    float y0;
    float x1;
    float y1;
    float x;
    float y;
    float tempx0;
    float tempy0;
    float tempx1;
    float tempy1;
    float lastx;
    float lasty;
    float xscale;
    float yscale;
    int   nx;
    int   ny;
    int   nxbase;
    int   nybase;
    FILE *map_file;
    int   npoints;
    int   npoly;
    int   used;
    int   total;
    float clipx0;
    float clipy0;
    float clipx1;
    float clipy1;

    char *dirname ;

    char *cell_base = "cell edge";

    char lambert_filename[ 512 ];

    lambert_filename[0] = NULL;
    xmin = llx;
    ymin = lly;
    xmax = urx;
    ymax = ury;
    nx = ncol;
    ny = nrow;

    xcellsize = ( xmax - xmin ) / ( ( float ) nx );
    ycellsize = ( ymax - ymin ) / ( ( float ) ny );

    if ( !strcmp ( cell_base, "cell center" ) ) /* field to mesh types */
        {
        nxbase = nx - 1;
        nybase = ny - 1;
        }
    else if ( !strcmp ( cell_base, "cell edge" ) ) /* tile types */
        {
        nxbase = nx;
        nybase = ny;
        }
    xscale = ( ( float ) ( nxbase ) ) / ( xmax - xmin );
    yscale = ( ( float ) ( nybase ) ) / ( ymax - ymin );

    dirname = getenv ( "PAVE_MAPDIR" ) ;
    if ( dirname )
        {
        if ( map_options & MAP_NO_LATLON )
            sprintf ( lambert_filename, "%s/%s", dirname, LAMBERT_FILENAME2 );
        else
            sprintf ( lambert_filename, "%s/%s", dirname, LAMBERT_FILENAME );
        }
    else{
        fprintf ( stderr, "WARNING:  Missing environment variable PAVE_MAPDIR\n" );
        if ( map_options & MAP_NO_LATLON )
            sprintf ( lambert_filename, "%s", LAMBERT_FILENAME2 );
        else
            sprintf ( lambert_filename, "%s", LAMBERT_FILENAME );
        }

    if ( ( map_file = fopen ( lambert_filename, "r" ) ) == NULL )
        {
        fprintf ( stderr, "Cannot open map file %s\n", LAMBERT_FILENAME2 );
        free ( lambert_filename );
        return ( FAILURE );
        }

    clipx0 = ( float ) clip_xmin;
    clipy0 = ( float ) clip_ymin;
    clipx1 = ( float ) ( nxbase ) - clip_xmax;
    clipy1 = ( float ) ( nybase ) - clip_ymax;

    npoly = 0;
    used = 0;
    total = 0;
    while ( NULL != fgets ( string, 79, map_file ) )
        {
        sscanf ( string, "%d", &segment );
        fgets ( string, 79, map_file );

        npoints = 0;
        while ( ! ( ( string[0] == 'E' ) && ( string[1] == 'N' ) &&
                    ( string[2] == 'D' ) ) )
            {
            sscanf ( string, "%g%g", &x, &y );
            tempx0 = ( x - xmin ) * xscale;
            tempy0 = ( y - ymin ) * yscale;
            if ( npoints > 0 )
                {
                x0 = tempx1;
                y0 = tempy1;
                x1 = tempx0;
                y1 = tempy0;
                if ( clip ( &x0, &y0, &x1, &y1,
                            clipx0, clipy0, clipx1, clipy1 ) )
                    {
                    if ( !used )
                        {
                        if ( used+total+2 < maxpoints )
                            {
                            xpts[used+total] = x0;
                            ypts[used+total] = y0;
                            xpts[used+1+total] = x1;
                            ypts[used+1+total] = y1;
                            used += 2;
                            }
                        }
                    else
                        {
                        if ( ( x0 == lastx ) && ( y0 == lasty ) )
                            {
                            if ( used+total+1 < maxpoints )
                                {
                                xpts[used+total] = x1;
                                ypts[used+total] = y1;
                                ++used;
                                }
                            }
                        else
                            {
                            n[npoly] = used;
                            ++npoly;
                            total += used;
                            used = 0;
                            if ( used+total+2 < maxpoints )
                                {
                                xpts[used+total] = x0;
                                ypts[used+total] = y0;
                                xpts[used+1+total] = x1;
                                ypts[used+1+total] = y1;
                                used += 2;
                                }
                            }
                        }
                    lastx = x1;
                    lasty = y1;
                    }
                }
            tempx1 = tempx0;
            tempy1 = tempy0;
            ++npoints;
            fgets ( string, 79, map_file );
            }
        if ( used )
            {
            n[npoly] = used;
            ++npoly;
            total += used;
            used = 0;
            }
        }
    fclose ( map_file );
    free ( lambert_filename );

    if ( used+total+5 < maxpoints )
        {
        xpts[used+total] = clipx0;
        ypts[used+total] = clipy0;
        ++used;
        xpts[used+total] = clipx1;
        ypts[used+total] = clipy0;
        ++used;
        xpts[used+total] = clipx1;
        ypts[used+total] = clipy1;
        ++used;
        xpts[used+total] = clipx0;
        ypts[used+total] = clipy1;
        ++used;
        xpts[used+total] = clipx0;
        ypts[used+total] = clipy0;
        ++used;
        }

    if ( used )
        {
        n[npoly] = used;
        ++npoly;
        total += used;
        }
    *npolyline = npoly;
    return ( PAVE_SUCCESS );

    }


int utm_map ( float llx, float lly, float urx, float ury, int utm_zone,
              int ncol, int nrow,
              int clip_xmin, int clip_xmax, int clip_ymin, int clip_ymax,
              float *xpts, float *ypts, int *n, int *npolyline, int maxpoints,
              int map_options )
    {
    int set_utm_position ( int map_fd, int index_fd, int *npoints );
    void geo2utm ( float *utmx, float *utmy, int utm_zone, float lon, float lat );

    register int i, j;
    static float point[2];
    int segment;
    float xmin;
    float ymin;
    float xmax;
    float ymax;
    float xcellsize;
    float ycellsize;
    float x0;
    float y0;
    float x1;
    float y1;
    float x;
    float y;
    float tempx0;
    float tempy0;
    float tempx1;
    float tempy1;
    float lastx;
    float lasty;
    float xscale;
    float yscale;
    int nx;
    int ny;
    int nxbase;
    int nybase;
    int map_fd;
    int index_fd;
    int npoints;
    int npoly;
    int used;
    int total;
    float clipx0;
    float clipy0;
    float clipx1;
    float clipy1;

    char *dirname ;

    char *cell_base = "cell edge";

    char utm_filename[ 512 ];
    char utm_indxname[ 512 ];

    xmin = llx;
    ymin = lly;
    xmax = urx;
    ymax = ury;
    nx = ncol;
    ny = nrow;

    xcellsize = ( xmax - xmin ) / ( ( float ) nx );
    ycellsize = ( ymax - ymin ) / ( ( float ) ny );


    if ( !strcmp ( cell_base, "cell center" ) ) /* field to mesh types */
        {
        nxbase = nx - 1;
        nybase = ny - 1;
        }
    else if ( !strcmp ( cell_base, "cell edge" ) ) /* tile types */
        {
        nxbase = nx;
        nybase = ny;
        }
    xscale = ( ( float ) ( nxbase ) ) / ( xmax - xmin );
    yscale = ( ( float ) ( nybase ) ) / ( ymax - ymin );

    dirname = getenv ( "PAVE_MAPDIR" ) ;
    if ( dirname )
        {
        if ( map_options & MAP_UTM_STATES )
            sprintf ( utm_filename, "%s/%s", dirname, UTM_ST_FILENAME );
        else
            sprintf ( utm_filename, "%s/%s", dirname, UTM_CO_FILENAME );

        sprintf ( utm_indxname, "%s.index%d", utm_filename, utm_zone );
        }
    else{
        fprintf ( stderr, "WARNING:  Missing environment variable PAVE_MAPDIR\n" );
        if ( map_options & MAP_UTM_STATES )
            sprintf ( utm_filename, "%s", UTM_ST_FILENAME );
        else
            sprintf ( utm_filename, "%s", UTM_CO_FILENAME );

        sprintf ( utm_indxname, "%s.index%d", utm_filename, utm_zone );
        }

    if ( ( map_fd = open ( utm_filename, O_RDONLY ) ) == -1 )
        {
        fprintf ( stderr, "Cannot open UTM map file %s\n", utm_filename );
        return ( FAILURE );
        }
    if ( ( index_fd = open ( utm_indxname, O_RDONLY ) ) == -1 )
        {
        fprintf ( stderr, "Cannot open UTM index file %s\n", utm_indxname );
        fprintf ( stderr, "See instructions for creating UTM index files.\n" );
        close ( map_fd );
        return ( FAILURE );
        }

    clipx0 = ( float ) clip_xmin;
    clipy0 = ( float ) clip_ymin;
    clipx1 = ( float ) ( nxbase ) - clip_xmax;
    clipy1 = ( float ) ( nybase ) - clip_ymax;

    xminedge = xmin + xcellsize * clip_xmin;
    yminedge = ymin + ycellsize * clip_ymin;
    xmaxedge = xmax - xcellsize * clip_xmax;
    ymaxedge = ymax - ycellsize * clip_ymax;

    npoly = 0;
    used = 0;
    total = 0;

    while ( set_utm_position ( map_fd, index_fd, &npoints ) )
        {
        for ( i = 0; i < npoints; ++i )
            {
            if ( read ( map_fd, ( char * ) point, 8 ) != 8 )
                {
                close ( map_fd );
                close ( index_fd );
                return ( FAILURE );
                }
            point[0] = ( float ) fabs ( ( double ) point[0] );
            geo2utm ( &x, &y, utm_zone, point[0], point[1] );

            tempx0 = ( x - xmin ) * xscale;
            tempy0 = ( y - ymin ) * yscale;
            if ( i > 0 )
                {
                x0 = tempx1;
                y0 = tempy1;
                x1 = tempx0;
                y1 = tempy0;
                if ( clip ( &x0, &y0, &x1, &y1,
                            clipx0, clipy0, clipx1, clipy1 ) )
                    {
                    if ( !used )
                        {
                        if ( used+total+2 < maxpoints )
                            {
                            xpts[used+total] = x0;
                            ypts[used+total] = y0;
                            xpts[used+1+total] = x1;
                            ypts[used+1+total] = y1;
                            used += 2;
                            }
                        }
                    else
                        {
                        if ( ( x0 == lastx ) && ( y0 == lasty ) )
                            {
                            if ( used+total+2 < maxpoints )
                                {
                                xpts[used+total] = x1;
                                ypts[used+total] = y1;
                                ++used;
                                }
                            }
                        else
                            {
                            n[npoly] = used;
                            ++npoly;
                            total += used;
                            used = 0;
                            if ( used+total+2 < maxpoints )
                                {
                                xpts[used+total] = x0;
                                ypts[used+total] = y0;
                                xpts[used+1+total] = x1;
                                ypts[used+1+total] = y1;
                                used += 2;
                                }
                            }
                        }
                    lastx = x1;
                    lasty = y1;
                    }
                }
            tempx1 = tempx0;
            tempy1 = tempy0;
            }
        if ( used )
            {
            n[npoly] = used;
            ++npoly;
            total += used;
            used = 0;
            }
        }
    close ( map_fd );
    close ( index_fd );

    if ( used+total+5 < maxpoints )
        {
        xpts[used+total] = clipx0;
        ypts[used+total] = clipy0;
        ++used;
        xpts[used+total] = clipx1;
        ypts[used+total] = clipy0;
        ++used;
        xpts[used+total] = clipx1;
        ypts[used+total] = clipy1;
        ++used;
        xpts[used+total] = clipx0;
        ypts[used+total] = clipy1;
        ++used;
        xpts[used+total] = clipx0;
        ypts[used+total] = clipy0;
        ++used;
        }

    if ( used )
        {
        n[npoly] = used;
        ++npoly;
        total += used;
        }

    /*
    printf("Total points = %d\n", total);
    */
    *npolyline = npoly;
    return ( PAVE_SUCCESS );
    }

int set_utm_position ( int map_fd, int index_fd, int *npoints )
    {
    static float corners[4];
    static off_t position;
    int inside;

    while ( read ( index_fd, ( char * ) corners, 16 ) == 16 )
        {
        inside = 0;

        /* check for cases where one corner is inside the clip box */

        if (
            (
                ( ( corners[0] >= xminedge ) && ( corners[0] <= xmaxedge ) ) ||
                ( ( corners[2] >= xminedge ) && ( corners[2] <= xmaxedge ) )
            ) &&
            (
                ( ( corners[1] >= yminedge ) && ( corners[1] <= ymaxedge ) ) ||
                ( ( corners[3] >= yminedge ) && ( corners[3] <= ymaxedge ) )
            )
        )
            inside = 1;

        /* check for other cases where x or y coordinates span the entire clip box */

        if ( !inside )
            {
            if (
                (
                    ( ( corners[0] <= xminedge ) && ( corners[2] >= xmaxedge ) ) &&
                    ( ( corners[1] >= yminedge ) || ( corners[3] <= ymaxedge ) )
                ) ||
                (
                    ( ( corners[1] <= yminedge ) && ( corners[3] >= ymaxedge ) ) &&
                    ( ( corners[0] >= xminedge ) || ( corners[2] <= xmaxedge ) )
                )
            )
                inside = 1;
            }

        if ( inside )
            {
            if ( read ( index_fd, ( char * ) ( &position ), 4 ) != 4 )
                {
                close ( map_fd );
                close ( index_fd );
                return ( FAILURE );
                }

            /* skip to arc node position and 6th element in the
               node header which contains npoints */
            if ( lseek ( map_fd, ( off_t ) ( position + 20 ),
                         SEEK_SET ) == -1 )
                {
                close ( map_fd );
                close ( index_fd );
                return ( FAILURE );
                }
            if ( read ( map_fd, ( char * ) ( npoints ), 4 ) != 4 )
                {
                close ( map_fd );
                close ( index_fd );
                return ( FAILURE );
                }
            }
        else
            {
            /* skip position index information */
            if ( lseek ( index_fd, ( off_t ) 4, SEEK_CUR ) == -1 )
                {
                close ( map_fd );
                close ( index_fd );
                return ( FAILURE );
                }
            }
        if ( inside )
            {
            return ( PAVE_SUCCESS );
            }
        }
    return ( FAILURE );
    }

/***********************************************************************/
/*   The routine below is from William Ivey's "project.c" functions    */
/***********************************************************************/
/* geo2utm: convert coordinates from geographic decimal degrees to utm */
/***********************************************************************/

void geo2utm ( float *utmx, float *utmy, int utm_zone, float lon, float lat )
    {
#define degrad 0.017453

    float utmym, dlong;

    utmym = 2.41 + 110.268 * lat + 0.00903 * ( lat*lat );
    dlong = 180. - ( 6 * utm_zone ) + ( 3 - lon );
    *utmy = 3187 * sin ( degrad * 2 * lat ) * ( 1 - cos ( degrad * dlong ) ) + utmym;
    *utmx = ( 111.226 + 0.0053 * lat ) * cos ( degrad * lat ) * dlong + 500.;
    }

int latlon_map ( float llx, float lly, float urx, float ury, int utm_zone,
                 int ncol, int nrow,
                 int clip_xmin, int clip_xmax, int clip_ymin, int clip_ymax,
                 float *xpts, float *ypts, int *n, int *npolyline, int maxpoints,
                 int map_options )
    {
    int set_utm_position ( int map_fd, int index_fd, int *npoints );

    register int i, j;
    static float point[2];
    int segment;
    float xmin;
    float ymin;
    float xmax;
    float ymax;
    float xcellsize;
    float ycellsize;
    float x0;
    float y0;
    float x1;
    float y1;
    float x;
    float y;
    float tempx0;
    float tempy0;
    float tempx1;
    float tempy1;
    float lastx;
    float lasty;
    float xscale;
    float yscale;
    int nx;
    int ny;
    int nxbase;
    int nybase;
    int map_fd;
    int index_fd;
    int npoints;
    int npoly;
    int used;
    int total;
    float clipx0;
    float clipy0;
    float clipx1;
    float clipy1;

    char *dirname ;
    char *cell_base = "cell edge";

    char utm_filename[ 512];
    char utm_indxname[ 512 ];

    xmin = llx;
    ymin = lly;
    xmax = urx;
    ymax = ury;
    nx = ncol;
    ny = nrow;

    xcellsize = ( xmax - xmin ) / ( ( float ) nx );
    ycellsize = ( ymax - ymin ) / ( ( float ) ny );


    if ( !strcmp ( cell_base, "cell center" ) ) /* field to mesh types */
        {
        nxbase = nx - 1;
        nybase = ny - 1;
        }
    else if ( !strcmp ( cell_base, "cell edge" ) ) /* tile types */
        {
        nxbase = nx;
        nybase = ny;
        }
    xscale = ( ( float ) ( nxbase ) ) / ( xmax - xmin );
    yscale = ( ( float ) ( nybase ) ) / ( ymax - ymin );

    dirname = getenv ( "PAVE_MAPDIR" ) ;
    if ( dirname )
        {
        sprintf ( utm_filename, "%s/%s", dirname, UTM_ST_FILENAME );
        sprintf ( utm_indxname, "%s.latlon.index", utm_filename );
        }
    else{
        fprintf ( stderr, "Missing environment variable PAVE_MAPDIR\n" );
        sprintf ( utm_filename, "%s", dirname, UTM_ST_FILENAME );
        sprintf ( utm_indxname, "%s.latlon.index", utm_filename );
        }

    if ( ( map_fd = open ( utm_filename, O_RDONLY ) ) == -1 )
        {
        fprintf ( stderr, "Cannot open UTM map file %s\n", utm_filename );
        return ( FAILURE );
        }
    if ( ( index_fd = open ( utm_indxname, O_RDONLY ) ) == -1 )
        {
        fprintf ( stderr, "Cannot open UTM index file %s\n", utm_indxname );
        fprintf ( stderr, "See instructions for creating UTM index files.\n" );
        close ( map_fd );
        return ( FAILURE );
        }

    clipx0 = ( float ) clip_xmin;
    clipy0 = ( float ) clip_ymin;
    clipx1 = ( float ) ( nxbase ) - clip_xmax;
    clipy1 = ( float ) ( nybase ) - clip_ymax;

    xminedge = xmin + xcellsize * clip_xmin;
    yminedge = ymin + ycellsize * clip_ymin;
    xmaxedge = xmax - xcellsize * clip_xmax;
    ymaxedge = ymax - ycellsize * clip_ymax;

    npoly = 0;
    used = 0;
    total = 0;

    while ( set_utm_position ( map_fd, index_fd, &npoints ) )
        {
        for ( i = 0; i < npoints; ++i )
            {
            if ( read ( map_fd, ( char * ) point, 8 ) != 8 )
                {
                close ( map_fd );
                close ( index_fd );
                return ( FAILURE );
                }

            x=point[0];
            y=point[1];

            tempx0 = ( x - xmin ) * xscale;
            tempy0 = ( y - ymin ) * yscale;
            if ( i > 0 )
                {
                x0 = tempx1;
                y0 = tempy1;
                x1 = tempx0;
                y1 = tempy0;
                if ( clip ( &x0, &y0, &x1, &y1,
                            clipx0, clipy0, clipx1, clipy1 ) )
                    {
                    if ( !used )
                        {
                        if ( used+total+2 < maxpoints )
                            {
                            xpts[used+total] = x0;
                            ypts[used+total] = y0;
                            xpts[used+1+total] = x1;
                            ypts[used+1+total] = y1;
                            used += 2;
                            }
                        }
                    else
                        {
                        if ( ( x0 == lastx ) && ( y0 == lasty ) )
                            {
                            if ( used+total+2 < maxpoints )
                                {
                                xpts[used+total] = x1;
                                ypts[used+total] = y1;
                                ++used;
                                }
                            }
                        else
                            {
                            n[npoly] = used;
                            ++npoly;
                            total += used;
                            used = 0;
                            if ( used+total+2 < maxpoints )
                                {
                                xpts[used+total] = x0;
                                ypts[used+total] = y0;
                                xpts[used+1+total] = x1;
                                ypts[used+1+total] = y1;
                                used += 2;
                                }
                            }
                        }
                    lastx = x1;
                    lasty = y1;
                    }
                }
            tempx1 = tempx0;
            tempy1 = tempy0;
            }
        if ( used )
            {
            n[npoly] = used;
            ++npoly;
            total += used;
            used = 0;
            }
        }
    close ( map_fd );
    close ( index_fd );

    if ( used+total+5 < maxpoints )
        {
        xpts[used+total] = clipx0;
        ypts[used+total] = clipy0;
        ++used;
        xpts[used+total] = clipx1;
        ypts[used+total] = clipy0;
        ++used;
        xpts[used+total] = clipx1;
        ypts[used+total] = clipy1;
        ++used;
        xpts[used+total] = clipx0;
        ypts[used+total] = clipy1;
        ++used;
        xpts[used+total] = clipx0;
        ypts[used+total] = clipy0;
        ++used;
        }

    if ( used )
        {
        n[npoly] = used;
        ++npoly;
        total += used;
        }

    /*
    printf("Total points = %d\n", total);
    */
    *npolyline = npoly;
    return ( PAVE_SUCCESS );
    }
