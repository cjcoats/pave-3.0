/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: util.c 83 2018-03-12 19:24:33Z coats $
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
 *      c 1993?  Steve Thorpe, MCNC ?
 *      
 ****************************************************************************/

#include <stdio.h>
#include "resources.h"
#include "contour.h"

static const char SVN_ID[] = "$Id: util.c 83 2018-03-12 19:24:33Z coats $";

res_data reso=
    {
    707,500,
    16, 32, 1, 1, -999, -1,
        {20, 40, 60}, {25, 60, 80},
    0, 0, 0,
    10, 5, 1,
    1, 10, 15,
    1, 1,
    2, 0, 6,
    25, 20, 160, 160, 0, 0, 10,
    100,
    10, 990,
    "xfarbe.dat", "xfarbe.ps",
    "xfarbe.pal", "nofile",
    "xfarbe.prof", "nofile",
    "default", " %.3g ",
    " %.3g ", " %.3g ",
    " %.2f ", " %.2f ",
    " %.2f ",
    "xfarbe  -  Fill Area with Bicubics  -  A. PREUSSER ",
    16777215, 0,
        {
        0, 16777192, 15203071, 16772286, 12645119,
        16764337, 9234175, 16755351, 6148863, 16746320, 4434687, 16734728,
        2652415, 14427904, 18386, 8912896, 116, 15990770, 16772084, 12517355,
        15717119, 8585176, 13157610, 3342293, 11512520, 2021335, 10844611,
        1877413, 10501300, 30308, 8388768, 13312, 983057, 16777215, 0, 16777215,
        0, 16724770, 0, 2096927, 0, 7829367, 16777192, 15203071, 16772286,
        12645119, 16764337, 9234175, 16755351, 6148863, 16746320, 4434687,
        16734728, 2652415, 14427904, 18386, 8912896, 116, 15990770, 16772084,
        12517355, 15717119, 8585176, 13157610, 3342293, 11512520, 2021335,
        10844611, 1877413, 10501300, 30308, 8388768, 13312, 983057, 16777215, 0,
        16777215, 0, 16724770, 0, 2096927, 0, 7829367, 215*0
        },
    /* pat */
    /* many more follow; currently assumed NULL and ignored */
    };

int start_of_symb_colors=33;

/* ======================================================== */
/* ======================================================== */


int compute_contour ( float *z, float *x, float *y, int nx, int ny, int numcol,
                      int lc, float *ctrlvl )
    {
    int nxdim;
    int ncol = numcol;
    int mode;
    float ratio;
    float rxmarg, rymarg, rxlen, rylen;
    int lx,ly, xannincr, yannincr;
    int i;
    float zmin,zmax;
#define MIN(x,y) ((x)<(y) ? (x): (y))
#define MAX(x,y) ((x)>(y) ? (x): (y))

    /* Function prototypes */
    extern int farbe_ (
#if 0
        float *, int *, int *, int *, int *, int *,
        float *,
        float *, int, float *, int, float *, int,
        float, float, float, float,
        int, int
#endif
    );

    nxdim = nx;
    mode = 1;
    lx = 1;
    ly = 1;
    ratio = 1.0;
    rxmarg=2.5;
    rymarg=2.0;
    rxlen=16.0;
    rylen=16.0;
    xannincr = 0;
    yannincr = 0;


    farbe_ ( z, &nxdim, &nx, &ny, &mode, &ncol,
             &ratio,
             ctrlvl, lc, x, lx, y, ly,
             rxmarg, rymarg, rxlen, rylen,
             xannincr, yannincr );

    return 0;
    }


/* ======================================================== */
/* ======================================================== */
void gtextp ( x, y, h, text, w, centerx, centery, color,
              back_color, n )
float *x, *y;
char text[];
float w, centerx, centery;
int *n, h, color, back_color;

    {
    }

/***********************************************************/

int usrpsc_ ( float *x, float *y, int *n, int *ncol, int *mode,
              int irect,
              int *jpolr, int *npolr, int *iride, int *jfrst, int *jtost )
    {
    int i;
    extern Cntr_line *ctr_line;
    Cntr_line *cline;
    Cntr_line *new_cntr_line();


    cline = new_cntr_line ( *ncol, *n, x, y );

    cline->next = ctr_line;
    ctr_line = cline;

    return 0;

    }

/***********************************************************/
/* ======================================================== */

Cntr_line *new_cntr_line ( int color, int n, float *x, float *y )
    {
    Cntr_line *cline;
    int i;

    cline = ( Cntr_line * ) malloc ( sizeof ( Cntr_line ) );
    if ( cline )
        {
        cline->color = color;
        cline->npoints = n;
        cline->next = NULL;
        cline->x = ( float * ) malloc ( n*sizeof ( float ) );
        cline->y = ( float * ) malloc ( n*sizeof ( float ) );
        for ( i=0; i<n; i++ )
            {
            cline->x[i] = x[i];
            cline->y[i] = y[i];
            }
        }
    return cline;
    }

/* ======================================================== */
/*start of legena_ */
/*     legend and annotation for axes */

int legena_ ( xmin, xmax, ymin, ymax, c, icol, nc,
              form, mode, incr, xt, yt )
float *xmin, *xmax, *ymin, *ymax, *c;
int *icol, *nc;
char *form;
int *mode;
int *incr;
float *xt, *yt;
    {
    return 0;
    }

/* ======================================================== */
int xf_frame ( xt, yt )
float *xt, *yt;
    {
    /*     PLOT FRAME */
    return 0;
    }

/* ======================================================== */
void gslwscp ( iw )
int iw;
    {
    }
/* ======================================================== */
void gwatchp ( it )
int it;
    {
    printf ( "." );
    }
/* ======================================================== */
void auto_cntr_levels ( float zmin, float zmax,
                        int ncol, float *cntr_lvl, int *nc )
    {

    float cincr, cstart;
    float zdif, dif;
    int log1;
    int i;
    float ten = 10.0;
    double r_lg10 ( float * ), r_mod ( float *, float * ), pow_ri ( float *, int * );

    *nc = ncol - 1;

    /*     COMPUTE INCREMENT CINCR FOR CONTOUR VALUES */
    zdif = zmax - zmin;
    cincr = zdif / ( *nc );
    if ( cincr < ( double ) 1. )
        {
        cincr *= ( double ).1;
        }
    log1 = r_lg10 ( &cincr ) + ( double ) 1.;
    cincr = pow_ri ( &ten, &log1 );
L100:
    dif = ncol * cincr - zdif;
    if ( dif > zdif )
        {
        cincr *= ( float ).5;
        }
    if ( dif > zdif )
        {
        goto L100;
        }

    /*     ROUNDED STARTING VALUE */
    cstart = zmin - r_mod ( &zmin,&cincr );
    if ( zmin > ( double ) 0. )
        {
        cstart += cincr;
        }

    *nc = ( zmax - cstart ) / cincr + 1;
    if ( ( *nc ) > ncol-1 )  *nc = ncol-1;

    /*       = NUMBER OF CONTOUR LEVELS */

    /*     CONTOUR LEVELS */
    for ( i = 0; i < ( *nc ); ++i )
        {
        cntr_lvl[i] =  cstart + i * cincr;
        }

    }



