/*****************************************************************************
 *         FARB-E-2D    by A.Preusser (translated partly from FORTRAN
 *                               by f2c)
 *  File: $Id: farbe2d.c 83 2018-03-12 19:24:33Z coats $
 * Copyright (C) A.Preusser, Fritz-Haber-Inst. der MPG, 1993
 *
 *   barf  [ba:rf]  2.  "He suggested using FORTRAN, and everybody barfed."
 *  - From The Shogakukan DICTIONARY OF NEW ENGLISH (Second edition)
 *
 *   My answer:
 *  short float longshit - that's   C  -  ap
 ****************************************************************************/

#include <stdio.h>
#include "resources.h"
typedef int integer;
typedef char *address;
typedef short int shortint;
typedef float real;
typedef double doublereal;
typedef struct
    {
    real r, i;
    } complex;
typedef struct
    {
    doublereal r, i;
    } doublecomplex;
typedef long int logical;
typedef short int shortlogical;

#define TRUE_ (1)
#define FALSE_ (0)

#define abs(x) ((x) >= 0 ? (x) : -(x))
#define dabs(x) (doublereal)abs(x)
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define dmin(a,b) (doublereal)min(a,b)
#define dmax(a,b) (doublereal)max(a,b)

#ifndef Skip_f2c_Undefs
#undef cray
#undef gcos
#undef mc68010
#undef mc68020
#undef mips
#undef pdp11
#undef sgi
#undef sparc
#undef sun
#undef sun2
#undef sun3
#undef sun4
#undef u370
#undef u3b
#undef u3b2
#undef u3b5
#undef unix
#undef vax
#endif


static const char SVN_ID[] = "$Id: farbe2d.c 83 2018-03-12 19:24:33Z coats $";

/* Common Block Declarations */

struct
    {
    real zmin, zmax;
    integer imin, jmin, imax, jmax;
    } frbcom_;

#define frbcom_1 frbcom_

struct
    {
    integer nfabu, ncolbu;
    real xfabu[4], yfabu[4];
    } frbcob_;

#define frbcob_1 frbcob_

union
    {
    struct
        {
        real sacmin, cmscal;
        integer maxpol, ncpmax, maxsta, ncmaxs, npp;
        real pi;
        integer maxrid;
        real sigs[4];
        integer ncmax;
        } _1;
    struct
        {
        real sacmin, cmscal;
        integer npmax, ncpmax, maxsta, ncmaxs, npp;
        real pi;
        integer maxrid;
        real sigs[4];
        integer ncmax;
        } _2;
    } frbcoc_;

#define frbcoc_1 (frbcoc_._1)
#define frbcoc_2 (frbcoc_._2)

struct
    {
    integer kk, kse;
    real xx4f, yy4f, sir, cor, cl;
    } frbcof_;

#define frbcof_1 frbcof_

struct
    {
    real p0[4], p1[4], p2[4], p3[4], q0[4], q1[4], q2[4], r0[4], r1[4], p11,
         p12, p13, p21, p22, p23, p31, p32, p33;
    } frbcop_;

#define frbcop_1 frbcop_

struct
    {
    real x0[1024]   /* was [256][4] */, y0[1024]    /* was [256][4] */;
    integer nclzr[1024] /* was [256][4] */;
    real sder[1024] /* was [256][4] */, tzr[1024]   /* was [256][4] */;
    integer nz[4];
    real si[4], co[4], sa[4], se[4], dx[4], dy[4], sl[4], hmin, slmax;
    integer kride, nprec;
    real poserr, dernor;
    integer ndir3;
    real zmax[4], zmin[4];
    integer istatz[1024]    /* was [256][4] */;
    real x3, y3, zsold;
    } frbcrd_;

#define frbcrd_1 frbcrd_

struct
    {
    real rma, rmax, dsmax, dsmin, fstep, thetas[4], racmin;
    } frbcor_;

#define frbcor_1 frbcor_

/* Table of constant values */
#define NXYMAX 513
static real c_b5 = ( float ) 1.;
static real c_b6 = ( float ) 19.6;
static real c_b7 = ( float ).4;
static real c_b8 = ( float ) 0.;
static integer c__24 = 24;
static real c_b11 = ( float ) 15.1;
static integer c__11 = 11;
static integer c__1 = 1;
static real c_b43 = ( float ) 10.;
static integer c__2 = 2;
static integer c__5 = 5;
static integer c__4 = 4;
static integer c__8 = 8;
static integer c__0 = 0;
static integer c__16 = 16;
static integer c__6 = 6;
static integer c_n1 = -1;
static integer c__99 = 99;
static real c_b399 = ( float ) 30.;
static real c_b401 = ( float ) 20.;
static real c_b404 = ( float ).9;
static real c_b406 = ( float ).6;
static real c_b419 = ( float ).85714285714285712;
#if ! defined(__hpux)
extern void ( *gplp ) (), ( *gtextp ) (), ( *gwatchp ) (), ( *gslwscp ) ();
#endif

extern res_data reso;   /* resources ********/
extern FILE *fp_mms;    /* file pointer to Min, Max, Sad -file*/
extern fmms_exist;      /* indicates, if fp_mms has been opened */

float xlen, ylen, xmarg, ymarg;
/* xlen, ... will be passed back and will be used by a XConfigureEvent */


/* Subroutine */ int farbe_ ( z, nxdim, nx, ny, mode, ncol, ratio,
                              cc, lc, xcm, lx, ycm, ly,
                              rxmarg, rymarg, rxlen, rylen,
                              xannincr, yannincr )

real *z, cc[], *ratio;
real xcm[], ycm[];
int *nxdim, *nx, *ny, *mode, *ncol;
int lc,lx,ly, xannincr, yannincr;
float rxmarg,rymarg, rxlen, rylen;  /* in cm */
    {
    /* Format strings */
    static char fmt_9000[] = "(\n0***ERROR***  ONE OF THE FOLLOWING WRON\
G\n/\n NXDIM,NX,NY\n: %d %d %d\n )";
    static char fmt_9001[] = "(\n0***ERROR*** MODE.LT.0 .OR. MODE.GT.3\n\
:%d\n)";
    static char fmt_9002[] = "(\n0***ERROR*** NX.GT.NXDIM\n:%d %d\n)";
    static char fmt_9003[] = "(\n0***ERROR***  NX.GT.MNMAX .OR. NY.GT.MN\
MAX\n/\n MNMAX,NX,NY\n: %d %d %d\n)";
    static char fmt_9010[] = "(\n0***ERROR***  MAX(Z)-MIN(Z).LT.\n %e\n)";
    static char fmt_9020[] = "(\n ***WARNING***   ALL ELEMENTS OF ARRAY Z \
NEARLY SAME VALUE\n  ZMIN= %e,\n   ZMAX= %e\n)";
    static char fmt_9100[] = "%% %d.%2.2df";
    static char fmt_9200[] = "%% %d.%2.2de";

    /* System generated locals */
    integer z_dim1, z_offset, i__1, i__2;
    real r__1, r__2, r__3;

    /* Builtin functions */
    integer s_wsfe(), do_fio(), e_wsfe();
    double r_lg10(), pow_ri(), r_mod();
    integer s_wsfi(), e_wsfi();

    /* Local variables */
    static integer ndig, icol[256];
    static real zdif;
    static char form[16];
    static real xmin, xmax, ymin, ymax;
    extern /* Subroutine */ int far2d_();
    static real c[255];
    static real x[NXYMAX], y[NXYMAX];
    static integer i, j;
    static integer nfeld;
    static real cincr;
    static integer logma, logdi;
    static real small;
    static integer login;
    static real xincr, yincr;
    extern /* Subroutine */ int farb2d_();
    static integer nc;
    /*    extern int frblgd_(); */
    static real cmscal, absmax, xx[5], yy[5], cstart;
    static integer nx1, ny1;
    static real dif;
    static integer ndd;
    extern /* Subroutine */ int gpl_(), legena_(), gslwsc_();
    static integer log1;
    static integer lmod;
    extern void xf_frame();


    /*     F ILL   A R  EA   WITH   B ICUBICS  -  E ASY TO USE */
    /*     *       * *              *             * */

    /*     FARB-E-2D        VERSION 3.0  01/1991 */

    /*     AUTHOR: A. PREUSSER */
    /*             FRITZ-HABER-INSTITUT DER MPG */
    /*             FARADAYWEG 4-6 */
    /*             D-14195 BERLIN (Dahlem) */
    /*             TEL.:  +49-30-8413-3220 */
    /*             E-MAIL: PREUSSER AT FHI-BERLIN.MPG.DE */


    /* *** ESSENTIAL INFORMATION FOR USERS *** */

    /*     INPUT PARAMETERS */
    /*     ---------------- */
    /*     Z            2D- ARRAY WITH DIMENSION (NXDIM,NY) */
    /*                  DEFINING Z-VALUES ON A RECTANGULAR */
    /*                  NX BY NY GRID. */
    /*                  THE FIRST INDEX SPECIFIES THE POSITION */
    /*                  OF THE CORRESPONDING Z-VALUE IN THE */
    /*                  X-DIRECTION, THE SECOND IN THE */
    /*                  Y-DIRECTION OF THE PLOT-SYSTEM. */

    /*                  ALL SCALING IS HANDLED BY THE PROGRAM, */
    /*                  INCLUDING THE SELECTION OF CONTOUR LEVELS */
    /*                  AT ROUND VALUES. */

    /*     NXDIM        FIRST DIMENSION OF Z AS DEFINED IN THE */
    /*                  CALLING PROGRAM */

    /*     NX,NY        NUMBER OF VALUES OF Z TO BE USED IN X- */
    /*                  AND Y-DIRECTION. */

    /*     MODE         0, FILL AREA ONLY */
    /*                  1, LINES ONLY */
    /*                  2, FILL AREA AND LINES */
    /*                  3, DISCRETE DATA, NO INTERPOLATION, */
    /*                     FILL AREA FOR COMPLETE RECTANGLES. */
    /*                  AS A DEFAULT, SET MODE=0 */

    /*     OUTPUT */
    /*     ------ */
    /*     IS A TWO-DIMENSIONAL CONTOUR PLOT WITH FILLED AREAS */
    /*     MAPPING THE 2D-ARRAY Z TO A SQUARE. */
    /*     4 TO 7 CONTOUR LEVELS ARE CHOSEN AT ROUND VALUES, */
    /*     DEPENDING ON THE VALUES OF Z. */
    /*     A LEGEND IS PLOTTED IDENTIFYING THE LEVELS */
    /*     SEPARATING THE AREAS OF DIFFERENT COLOURS. */


    /* *** ADDITIONAL INFORMATION FOR USERS *** */

    /*     PLOT INTERFACE */
    /*     THE PLOTTING OF THE FILLED AREAS AND THE CONTOUR LINES */
    /*     IS CARRIED OUT BY A   *USER SUPPLIED*   SUBROUTINE */
    /*                        USRPLT                          . */
    /*    (INTERNALLY, THE ROUTINES GTEXT FOR PLOTTING OF TEXT, */
    /*     COMPATIBLE WITH CALCOMP-ROUTINE SYMBOL, */
    /*     AND THE ROUTINE GPL (N,X,Y) FOR PLOTTING OF A */
    /*     POLYGON WIH N POINTS WITH COORDINATES X(I),Y(I),I=1,N */
    /*     ARE CALLED IN ADDITION.) */

    /*     A STANDARD USRPLT WILL BE USED IN CASE THE USER DOES */
    /*     NOT SUPPLY HIS OWN. */

    /*     PARAMETERS OF USRPLT */
    /*                SUBROUTINE USRPLT (X,Y,N,NCOL,MODE) */
    /*                X,Y     REAL ARRAYS OF LENGTH N FOR */
    /*                        THE COORDINATES OF THE POLYGON */
    /*                        TO BE PLOTTED. */
    /*                N       NUMBER OF POINTS OF POLYGON */
    /*                NCOL    COLOUR TO BE USED */
    /*                        FOR THE AREA OR THE LINE. */
    /*                MODE    1, LINE DRAWING */
    /*                        0, FILL AREA */

    /*     MINIMUM AND MAXIMUM OF Z */
    /*     THE MIN. AND MAX. VALUES OF Z ARE STORED IN A COMMON */
    /*     BLOCK /FRBCOM/ THAT MAY BE ACCESSED BY A USER. */
    /*     THE ORDER OF VARIABLES IN /FRBCOM/ IS THE FOLLOWING: */
    /*            ZMIN,ZMAX,IMIN,JMIN,IMAX,JMAX */
    /*     (ZMIN= Z(IMIN,JMIN), ZMAX= Z(IMAX,JMAX) ) */
    /*     THE LOCATION OF A CERTAIN VALUE Z(I,J) CAN BE FOUND */
    /*     WITH THE HELP OF THE TICK MARKS AT THE MARGINS OF THE */
    /*     SQUARE. THE LEFT LOWER CORNER OF THE SQUARE (ORIGIN */
    /*     OF THE X-Y-SYSTEM) IS AT 1.0 CM, 1.0 CM. */
    /*     THE SQUARE IS 18.0 CM X 18.0 CM WIDE. */


    /*     EXAMPLE FOR USRPLT  (WITH GKS-CALLS) */
    /*     ------------------ */

    /*     MNMAX        MAX. NUMBER OF GRID LINES IN X- AND Y-DIRECTION */
    /*     CMSCAL       =1.,   IF CENTIMETERS ARE USED WHEN CALLING FARB2D */
    /*                  =2.54, IF INCHES   ARE USED WHEN CALLING FARB2D */
    /*     NCOL         NUMBER OF COLOURS */
    /*     XLEN,YLEN    LENGTH OF DOMAIN IN X- AND Y-DIRECTION */
    /*     XMARG,YMARG  DISTANCES FROM THE LEFT AND LOWER MARGIN */
    /*     SMALL        SMALLEST DIFFERENCE BETWEEN ELEMENTS OF Z */


    /*     INSTALLATION PARAMETERS */
    /* Parameter adjustments */
    z_dim1 = *nxdim;
    z_offset = z_dim1 + 1;
    z -= z_offset;

    /* Function Body */

    xlen = rxlen;
    ylen = rylen;
    if ( *ratio==1. )
        {
        xlen= 20.;
        if ( rxlen<20. ) xlen= 20.;
        ylen= 16.;
        if ( rylen<16. ) ylen= 16.;

        /* take max. xlen, modify ylen */
        if ( ( float ) ( ( *nx ) ) / ( *ny ) >xlen/ylen )
            ylen= ( *ny ) * xlen/ ( *nx );

        /* take max. ylen, modify xlen */
        else xlen= ( *nx ) * ylen/ ( *ny );
        }

    xmarg = rxmarg;
    ymarg = rymarg;
    small = ( float ) 1e-25;

    /*     SET NUMBER OF GRID LINES */
    nx1 = *nx;
    ny1 = *ny;
    if ( *mode == 3 )
        {
        nx1 = *nx + 1;
        }
    if ( *mode == 3 )
        {
        ny1 = *ny + 1;
        }

    /*     SET COLOR TABLE */
    i__1 = *ncol;
    for ( i = 1; i <= i__1; ++i )
        {
        /* L10: */
        icol[i - 1] = i;
        }

    /*     ERROR CHECKS */
    if ( *nxdim <= 0 || *mode < 3 && ( *nx <= 1 || *ny <= 1 ) || *mode == 3 && *
            nx + *ny < 3 || *nx <= 0 || *ny <= 0 )
        {
        fprintf ( stderr,fmt_9000,*nxdim,*nx,*ny );
        }

    if ( *mode < 0 && *mode != -999 || *mode > 3 )
        {
        fprintf ( stderr,fmt_9001,*mode );
        }

    if ( *nx > *nxdim )
        {
        fprintf ( stderr,fmt_9002,*nx,*nxdim );
        }

    if ( nx1 > NXYMAX || ny1 > NXYMAX )
        {
        fprintf ( stderr,fmt_9003,*nx,*ny,NXYMAX );
        }

    nc = *ncol - 1;

    /*     DETERMINE MIN AND MAX OF Z */
    frbcom_1.zmax = z[z_dim1 + 1];
    frbcom_1.zmin = frbcom_1.zmax;
    frbcom_1.imin = 1;
    frbcom_1.jmin = 1;
    frbcom_1.imax = 1;
    frbcom_1.jmax = 1;
    i__1 = *ny;
    for ( j = 1; j <= i__1; ++j )
        {
        i__2 = *nx;
        for ( i = 1; i <= i__2; ++i )
            {
            if ( frbcom_1.zmax >= z[i + j * z_dim1] )
                {
                goto L20;
                }
            frbcom_1.zmax = z[i + j * z_dim1];
            frbcom_1.imax = i;
            frbcom_1.jmax = j;
            goto L40;
L20:
            if ( frbcom_1.zmin <= z[i + j * z_dim1] )
                {
                goto L40;
                }
            frbcom_1.zmin = z[i + j * z_dim1];
            frbcom_1.imin = i;
            frbcom_1.jmin = j;
L40:
            ;
            }
        /* L50: */
        }

    /*     CHECK FOR MINIMAL DIFFERENCE PERMITTED IN Z */
    zdif = frbcom_1.zmax - frbcom_1.zmin;
    if ( abs ( zdif ) > small )
        {
        goto L60;
        }
    fprintf ( stderr,fmt_9010,small );
    return 0;

    /*     COMPUTE INCREMENT CINCR FOR CONTOUR VALUES */
L60:
    cincr = zdif / nc;
    if ( cincr < ( float ) 1. )
        {
        cincr *= ( float ).1;
        }
    log1 = r_lg10 ( &cincr ) + ( float ) 1.;
    cincr = pow_ri ( &c_b43, &log1 );
L100:
    dif = *ncol * cincr - zdif;
    if ( dif > zdif )
        {
        cincr *= ( float ).5;
        }
    if ( dif > zdif )
        {
        goto L100;
        }

    /*     ROUNDED STARTING VALUE */
    cstart = frbcom_1.zmin - r_mod ( &frbcom_1.zmin, &cincr );
    if ( frbcom_1.zmin > ( float ) 0. )
        {
        cstart += cincr;
        }

    nc = ( frbcom_1.zmax - cstart ) / cincr + 1;
    if ( nc > *ncol-1 )  nc = *ncol-1;

    /*       = NUMBER OF CONTOUR LEVELS */

    /*     CONTOUR LEVELS */
    i__1 = nc;
    for ( i = 1; i <= i__1; ++i )
        {
        /* L110: */
        c[i - 1] = cstart + ( i - 1 ) * cincr;
        }

    /* Modification for directly accepting contour levels cc */
    if ( lc!= 0 )  /* if address of cc.ne.0 */
        {
        nc= *ncol - 1;
        for ( i=0; i<nc; i++ )
            c[i]= cc[i];
        }
    else
        for ( i=0; i<nc; i++ )
            cc[i]= c[i];   /* always return levels used */

    /* return number of contour levels +1 = number of colors */
    *ncol= nc+1;

    /*     SET FORMAT FOR OUTPUT OF CONTOUR VALUES IN LEGEND */

    /*     DETERMINE NUMBER OF DIGITS */
    logdi = r_lg10 ( &zdif );
    r__2 = ( r__1 = c[nc - 1], abs ( r__1 ) ), r__3 = abs ( c[0] );
    absmax = max ( r__2,r__3 );
    logma= logdi;
    if ( absmax != 0. ) logma = r_lg10 ( &absmax );
    login = r_lg10 ( &cincr );

    if ( ( r__1 = c[nc - 1], abs ( r__1 ) ) < ( float ) 1. )
        {
        --logma;
        }

    if ( cincr < ( float ) 1. )
        {
        --login;
        }

    if ( zdif < ( float ) 1. )
        {
        --logdi;
        }

    if ( logma - logdi > 5 )
        {
        fprintf ( stderr,fmt_9020,frbcom_1.zmin,frbcom_1.zmax );
        }

    /*     CHOOSE FORMAT SPECIFICATION E OR F */
    if ( ( r__1 = c[nc - 1], abs ( r__1 ) ) < ( float ).1 && c[nc - 1] != ( float ) 0. ||
            abs ( c[0] ) < ( float ).1 && c[0] != ( float ) 0. || c[nc - 1] >= (
                float ) 1e6 || c[0] <= ( float )-1e6 )
        {
        goto L120;
        }

    /*     F-FORMAT */
    ndig = abs ( logma ) + 3;
    if ( login < 0 )
        {
        ndig -= login;
        }
    /* Computing MIN */
    i__1 = ndig + 2;
    nfeld = min ( i__1,15 );
    ndd = 0;
    if ( login < 2 )
        {
        ndd = ( i__1 = login - 2, abs ( i__1 ) );
        }
    /* Computing MIN */
    i__1 = ndd, i__2 = nfeld - 2;
    ndd = min ( i__1,i__2 );
    sprintf ( form,fmt_9100,nfeld,ndd );
    /*    printf (" fmt9100= %s\n", form); */
    goto L140;

    /*     E-FORMAT */
L120:
    ndig = ( i__1 = logma - login, abs ( i__1 ) ) + 3;
    nfeld = ndig + 6;
    if ( c[0] < ( float ) 0. )
        {
        ++nfeld;
        }
    nfeld = min ( nfeld,15 );
    /* Computing MIN */
    i__1 = ndig, i__2 = nfeld - 7;
    ndig = min ( i__1,i__2 );
    sprintf ( form,fmt_9200,nfeld,ndig-1 );
    /*   printf (" fmt9200= %s\n", form); */

L140:

    /*     SCALING FOR X- AND Y-ARRAYS */
    xincr = xlen / ( nx1 - 1 );
    x[0] = xmarg;
    i__1 = nx1;
    for ( i = 2; i <= i__1; ++i )
        x[i - 1] = x[i - 2] + xincr;

    yincr = ylen / ( ny1 - 1 );
    y[0] = ymarg;
    i__1 = ny1;
    for ( i = 2; i <= i__1; ++i )
        y[i - 1] = y[i - 2] + yincr;

    /* Modification for directly accepting x- and y-coordinates in cm */
    if ( lx!= 0 )  /* if address of xcm.ne.0 */
        {
        for ( i=0; i < nx1; i++ )
            x[i]= xcm[i];
        xlen= x[nx1-1];
        }
    else
        for ( i=0; i < nx1; i++ )
            xcm[i]= x[i];

    if ( ly!= 0 )  /* if address of ycm.ne.0 */
        {
        for ( i=0; i < ny1; i++ )
            y[i]= ycm[i];
        ylen= y[ny1-1];
        }
    else
        for ( i=0; i < ny1; i++ )
            ycm[i]= y[i];

    /*****     CALLs TO FARB2D *****/
    if ( *mode > -999 )
        {
        lmod= *mode;
        if ( *mode==2 ) lmod= 0;

        if ( *mode < 3 )
            farb2d_ ( x, nx, y, ny, &z[z_offset], nxdim, c, icol, &nc,
                      &lmod, c__0, c__0 );

        if ( *mode==2 )
            {
            lmod= 1;  /* lines by separate call */
            farb2d_ ( x, nx, y, ny, &z[z_offset], nxdim, c, icol, &nc,
                      &lmod, c__0, c__0 );
            }

        if ( *mode == 3 ) /* no interpolation */
            far2d_ ( x, nx, y, ny, &z[z_offset], nxdim, c, icol, &nc );

        }

    /*** plot frame **/
    xf_frame ( x,y );

    /* mark extrema */
    if ( ( reso.min_height != 0 || reso.sad_height != 0 || reso.max_height !=0 )
            && *mode != 3 )
        {
        gslwscp ( 0 ); /*set line width for extrema symbols */
        lmod= 4;
        farb2d_ ( x, nx, y, ny, &z[z_offset], nxdim, c, icol, &nc,
                  &lmod, c__0, c__0 );

        /*** Close file for Max Min and Saddle points ***/
        if ( fmms_exist==1 )
            {
            int dummy;
            dummy= fclose ( fp_mms );
            }
        }

    /* put labels on contour lines */
    if ( xannincr != 0 && yannincr != 0 && *mode != 3 )
        {
        lmod= -1;
        farb2d_ ( x, nx, y, ny, &z[z_offset], nxdim, c, icol, &nc,
                  &lmod, xannincr, yannincr );
        }

    /**** Legend ***/
    /* set dimension of legend */
    xmin= xmarg + xlen + reso.xdist*0.1;
    if ( xcm!=0 ) xmin= xcm[nx1-1] + reso.xdist*0.1;
    xmax= xmin + 1.;

    ymin= ymarg;
    if ( ycm!=0 ) ymin= ycm[0];
    ymax= ymin + ylen;
    if ( ycm!=0 ) ymax= ycm[ny1-1];

    /* plot legend */
        {
        int incr= 1;
        if ( nc>16 ) incr=2;
        if ( nc>32 ) incr= 4;
        if ( nc>64 ) incr=8;
        legena_ ( &xmin, &xmax, &ymin, &ymax, c, icol, &nc, form,
                  mode, &incr, x,y );
        }

    return 0;
    } /* farbe_ */

/* Subroutine */ int farb2d_ ( x, lx, y, ly, z, nxdim, cn, icol, nc, mode,
                               xannincr, yannincr )
real *x;
integer *lx;
real *y;
integer *ly;
real *z;
integer *nxdim;
real *cn;
integer *icol, *nc, *mode, xannincr, yannincr;
    {
    /* Initialized data */

    static real x4 = ( float ) 0.;
    static real z43 = ( float ) 0.;
    static real x5 = ( float ) 0.;
    static real z53 = ( float ) 0.;
    static real z63 = ( float ) 0.;
    static real z5b1 = ( float ) 0.;
    static real z5b2 = ( float ) 0.;
    static real z5b3 = ( float ) 0.;
    static real z5b4 = ( float ) 0.;
    static real z5b5 = ( float ) 0.;
    static real a5 = ( float ) 0.;
    static real za5b2 = ( float ) 0.;
    static real za5b3 = ( float ) 0.;
    static real za5b4 = ( float ) 0.;
    static real x6 = ( float ) 0.;
    static real z64 = ( float ) 0.;
    static real z6b1 = ( float ) 0.;
    static real z6b2 = ( float ) 0.;
    static real z6b3 = ( float ) 0.;
    static real z6b4 = ( float ) 0.;
    static real z6b5 = ( float ) 0.;
    static real zx[2] = { ( float ) 0., ( float ) 0. };
    static real zy[2] = { ( float ) 0., ( float ) 0. };
    static real zxy[2] = { ( float ) 0., ( float ) 0. };
    static real zab[6]  /* was [2][3] */ = { ( float ) 0., ( float ) 0., ( float ) 0., (
            float ) 0., ( float ) 0., ( float ) 0.
                                           };
    static real za[8]   /* was [4][2] */ = { ( float ) 0., ( float ) 0., ( float ) 0., (
            float ) 0., ( float ) 0., ( float ) 0., ( float ) 0., ( float ) 0.
                                           };
    static real zb[5] = { ( float ) 0., ( float ) 0., ( float ) 0., ( float ) 0., ( float ) 0. };


    /* Format strings */
    static char fmt_99999[] = "FARB2D:   LX = 1 OR LESS.\n";
    static char fmt_99998[] = "FARB2D:   LY = 1 OR LESS.\n";
    static char fmt_99993[] = "FARB2D:   IDENTICAL X VALUES.\n";
    static char fmt_99992[] = "FARB2D:   X VALUES OUT OF SEQUENCE.\n";
    static char fmt_99991[] = "FARB2D:   IX= %d   X(IX) = %e\n";
    static char fmt_99990[] = "FARB2D:   IDENTICAL Y VALUES.\n";
    static char fmt_99989[] = "FARB2D:   Y VALUES OUT OF SEQUENCE.\n";
    static char fmt_99988[] = "FARB2D:   IY= %d      Y(IY) = %e\n";
    static char fmt_99973[] = "FARB2D:   IDENTICAL CN VALUES.\n";
    static char fmt_99972[] = "FARB2D:   CN VALUES OUT OF SEQUENCE.\n";
    static char fmt_99971[] = "FARB2D:   I= %d      CN(I) = %e\n";
    static char fmt_99970[] = "FARB2D:   NC .LE. 0       NC= %d\n";

    /* System generated locals */
    integer z_dim1, z_offset, i__1, i__2;
    real r__1;

    /* Builtin functions */
    /*    integer s_wsfe(), e_wsfe(), do_fio(); */

    /* Local variables */
    static integer ixml, iyml;
    static real zxy33, zxy34, zzxy[4];
    static integer iyml1, i;
    static real b1, b2, b3, b4, b5, w2, w3, x3, y3, y4;
    extern /* Subroutine */ int frbfcl_(), farbrc_();
    static real z33;
    static integer ix, iy;
    static real z54, z62, z65;
    static integer jy;
    static real sw, xx[4], yy[4], zz[4];
    static integer nsides, lx0, ly0, ix6;
    static real wx2, wx3, wy2, wy3;
    static integer ifa;
    static real zx33, zx34, zy33, zy34, zzx[4], zzy[4];
    static integer ixm1, iym2, lxm1, lxm2, lym1, lym2, iym3;
    static integer iyann, lcanno;
    extern int usrpsc_();
    extern int frbcmm_();


    /*     FILL AREA WITH BICUBICS FOR 2D CONTOUR PLOTTING */
    /*     ----------------------------------------------- */
    /*     FARB-E-2D  VERSION 3.0, 01/1991 */

    /*     T R I P   ALGORITHM              A. PREUSSER */

    /*     AUTHOR: A. PREUSSER */
    /*             FRITZ-HABER-INSTITUT DER MPG */
    /*             FARADAYWEG 4-6 */
    /*             D-1000 BERLIN 33 */

    /*     INPUT PARAMETERS */
    /*     X       ARRAY OF LENGTH LX FOR X-COORDINATES OF */
    /*             A REGULAR GRID */
    /*             IN ASCENDING ORDER. */

    /*                     X- AND Y-COORDINATES MUST BE GIVEN */
    /*                             IN CENTIMETERS */
    /*                             ============== */

    /*     LX      NUMBER OF GRID LINES X= X(I), I=1,LX */
    /*             PARALLEL TO Y-AXIS. */
    /*     Y       ARRAY OF LENGTH LY FOR Y-COORDINATES */
    /*             IN ASCENDING ORDER. */
    /*     LY      NUMBER OF GRID LINES Y= Y(I), I=1,LY */
    /*             PARALLEL TO X-AXIS. */
    /*     Z       2-DIMENSIONAL ARRAY DIMENSIONED Z(NXDIM,...) */
    /*             DEFINING THE Z-VALUES AT THE GRID POINTS. */
    /*             THE POINT WITH THE COORDINATES X(K), Y(L) */
    /*             RECEIVES THE VALUE Z(K,L), K=1,LX, L=1,LY. */
    /*     NXDIM   FIRST DIMENSION OF ARRAY Z */
    /*     CN      ARRAY OF LENGTH NC FOR THE Z-VALUES OF */
    /*             THE CONTOURS (CONTOUR LEVELS) */
    /*             IN ASCENDING ORDER */
    /*     ICOL    INTEGER ARRAY OF LENGTH NC+1 FOR */
    /*             THE COLOURS TO BE USED FOR THE LINES OR AREAS. */
    /*             VALUES FROM THIS ARRAY ARE PASSED TO */
    /*             THE USER SUPPLIED SUBROUTINE USRPLT. */
    /*             ICOL(I) IS USED FOR THE AREA, WHERE */
    /*                  Z .GT. CN(I-1)        AND */
    /*                  Z .LE. CN(I), */
    /*             FOR I=2,NC. */
    /*             AREAS, WHERE Z.LE.CN(1) */
    /*             ARE FILLED WITH COLOUR ICOL(1), */
    /*             AND AREAS, WHERE Z.GT.ICOL(NC) */
    /*             ARE FILLED WITH COLOUR ICOL(NC+1). */
    /*     NC      NUMBER OF CONTOUR LEVELS, NC.LE.100 */
    /*     MODE          0, FILL AREA ONLY */
    /*                   1, LINES ONLY */
    /*                   2, FILL AREA AND LINES */


    /*     OUTPUT */
    /*     IS PERFORMED BY CALLS TO THE SUBROUTINE    USRPLT */
    /*     TO BE SUPPLIED BY THE USER (AN EXAMPLE FOR USRPLT */
    /*     IS INCLUDED.) */

    /*     PARAMETERS OF USRPLT */
    /*                SUBROUTINE USRPLT (X,Y,N,NCOL,MODE) */
    /*                X,Y     REAL ARRAYS OF LENGTH N FOR */
    /*                        THE COORDINATES OF THE POLYGON */
    /*                        TO BE PLOTTED. */
    /*                N       NUMBER OF POINTS OF POLYGON */
    /*                NCOL    COLOUR TO BE USED */
    /*                        FOR THE AREA OR THE LINE. */
    /*                        FOR NCOL, THE PROGRAM PASSES */
    /*                        VALUES OF ICOL AS DESCRIBED ABOVE. */
    /*                MODE    1, LINE DRAWING */
    /*                        0, FILL AREA */
    /*     ------------------------------------------------------------- */

    /*     THIS MODULE (FARB2D) IS BASED ON SUBROUTINE SFCFIT OF */
    /*          ACM ALGORITHM 474 BY H.AKIMA */

    /* Parameter adjustments */
    --icol;
    --cn;
    z_dim1 = *nxdim;
    z_offset = z_dim1 + 1;
    z -= z_offset;
    --y;
    --x;

    /* Function Body */


    /*    PRELIMINARY PROCESSING */

    ifa = 0;
    lx0 = *lx;
    lxm1 = lx0 - 1;
    lxm2 = lxm1 - 1;
    ly0 = *ly;
    lym1 = ly0 - 1;
    lym2 = lym1 - 1;

    /*     ERROR CHECK */

    if ( lxm2 < 0 )
        {
        goto L400;
        }
    if ( lym2 < 0 )
        {
        goto L410;
        }
    i__1 = lx0;
    for ( ix = 2; ix <= i__1; ++ix )
        {
        if ( ( r__1 = x[ix - 1] - x[ix] ) < ( float ) 0. )
            {
            goto L20;
            }
        else if ( r__1 == 0 )
            {
            goto L460;
            }
        else
            {
            goto L470;
            }
L20:
        ;
        }

    i__1 = ly0;
    for ( iy = 2; iy <= i__1; ++iy )
        {
        if ( ( r__1 = y[iy - 1] - y[iy] ) < ( float ) 0. )
            {
            goto L70;
            }
        else if ( r__1 == 0 )
            {
            goto L490;
            }
        else
            {
            goto L500;
            }
L70:
        ;
        }

    i__1 = *nc;
    for ( i = 2; i <= i__1; ++i )
        {
        if ( ( r__1 = cn[i - 1] - cn[i] ) < ( float ) 0. )
            {
            goto L80;
            }
        else if ( r__1 == 0 )
            {
            goto L530;
            }
        else
            {
            goto L540;
            }
L80:
        ;
        }

    if ( *nc <= 0 )
        {
        goto L560;
        }


    /*  MAIN DO-LOOPS */
    iyann= 1;
    i__1 = ly0;
    for ( iy = 2; iy <= i__1; ++iy )
        {
        iym2 = iy - 2;
        iym3 = iy - 3;
        iyml = iy - ly0;
        iyml1 = iyml + 1;
        ix6 = 0;
        i__2 = lx0;
        for ( ix = 1; ix <= i__2; ++ix )
            {
            ixm1 = ix - 1;
            ixml = ix - lx0;

            /* ROUTINES TO PICK UP NECESSARY X,Y, AND Z VALUES TO */
            /* COMPUTE THE ZA,ZB, AND ZAB VALUES, AND TO ESTIMATE */
            /* THEM WHEN NECESSARY */
            /* PRELIMINARY WHEN IX.EQ.1 */

            if ( ixm1 != 0 )
                {
                goto L150;
                }
            y3 = y[iy - 1];
            y4 = y[iy];
            b3 = ( float ) 1. / ( y4 - y3 );
            if ( iym2 > 0 )
                {
                b2 = ( float ) 1. / ( y3 - y[iy - 2] );
                }
            if ( iym3 > 0 )
                {
                b1 = ( float ) 1. / ( y[iy - 2] - y[iy - 3] );
                }
            if ( iyml < 0 )
                {
                b4 = ( float ) 1. / ( y[iy + 1] - y4 );
                }
            if ( iyml1 < 0 )
                {
                b5 = ( float ) 1. / ( y[iy + 2] - y[iy + 1] );
                }
            goto L180;

            /*  TO SAVE THE OLD VALUES */

L150:
            za[0] = za[1];
            za[4] = za[5];
            x3 = x4;
            z33 = z43;
            za[1] = za[2];
            za[5] = za[6];
            zab[0] = zab[1];
            zab[2] = zab[3];
            zab[4] = zab[5];
L160:
            x4 = x5;
            z43 = z53;
            zb[0] = z5b1;
            zb[1] = z5b2;
            zb[2] = z5b3;
            zb[3] = z5b4;
            zb[4] = z5b5;
            za[2] = za[3];
            za[6] = za[7];
            zab[1] = za5b2;
            zab[3] = za5b3;
            zab[5] = za5b4;
L170:
            x5 = x6;
            z53 = z63;
            z54 = z64;
            z5b1 = z6b1;
            z5b2 = z6b2;
            z5b3 = z6b3;
            z5b4 = z6b4;
            z5b5 = z6b5;
            /* TO COMPUTE THE ZA, ZB, AND ZAB VALUES AND */
            /* TO ESTIMATE THE ZB VALUES */
            /* WHEN (IY.LE.3).OR.(IY.GE.LY-1) */

L180:
            ++ix6;
            if ( ix6 > lx0 )
                {
                goto L260;
                }
            x6 = x[ix6];
            z63 = z[ix6 + ( iy - 1 ) * z_dim1];
            z64 = z[ix6 + iy * z_dim1];
            z6b3 = ( z64 - z63 ) * b3;
            if ( lym2 == 0 )
                {
                goto L200;
                }
            if ( iym2 == 0 )
                {
                goto L190;
                }
            z62 = z[ix6 + ( iy - 2 ) * z_dim1];
            z6b2 = ( z63 - z62 ) * b2;
            if ( iyml != 0 )
                {
                goto L190;
                }
            z6b4 = z6b3 + z6b3 - z6b2;
            goto L210;
L190:
            z65 = z[ix6 + ( iy + 1 ) * z_dim1];
            z6b4 = ( z65 - z64 ) * b4;
            if ( iym2 != 0 )
                {
                goto L210;
                }
            z6b2 = z6b3 + z6b3 - z6b4;
            goto L210;
L200:
            z6b2 = z6b3;
            z6b4 = z6b3;
L210:
            if ( iym3 <= 0 )
                {
                goto L220;
                }
            z6b1 = ( z62 - z[ix6 + ( iy - 3 ) * z_dim1] ) * b1;
            goto L230;
L220:
            z6b1 = z6b2 + z6b2 - z6b3;
L230:
            if ( iyml1 >= 0 )
                {
                goto L240;
                }
            z6b5 = ( z[ix6 + ( iy + 2 ) * z_dim1] - z65 ) * b5;
            goto L250;
L240:
            z6b5 = z6b4 + z6b4 - z6b3;
L250:
            if ( ix6 == 1 )
                {
                goto L170;
                }
            a5 = ( float ) 1. / ( x6 - x5 );
            za[3] = ( z63 - z53 ) * a5;
            za[7] = ( z64 - z54 ) * a5;
            za5b2 = ( z6b2 - z5b2 ) * a5;
            za5b3 = ( z6b3 - z5b3 ) * a5;
            za5b4 = ( z6b4 - z5b4 ) * a5;
            if ( ix6 == 2 )
                {
                goto L160;
                }
            goto L280;
            /*     TO ESTIMATE THE ZA AND ZAB VALUES */
            /*     WHEN (IX.GE.LX-1).AND.(LX.GT.2) */
L260:
            if ( lxm2 == 0 )
                {
                goto L270;
                }
            za[3] = za[2] + za[2] - za[1];
            za[7] = za[6] + za[6] - za[5];
            if ( ixml == 0 )
                {
                goto L290;
                }
            za5b2 = zab[1] + zab[1] - zab[0];
            za5b3 = zab[3] + zab[3] - zab[2];
            za5b4 = zab[5] + zab[5] - zab[4];
            goto L290;
            /*     TO ESTIMATE THE ZA AND ZAB VALUES */
            /*     WHEN (IX.GE.LX-1).AND.(LX.EQ.2) */
L270:
            za[3] = za[2];
            za[7] = za[6];
            if ( ixml == 0 )
                {
                goto L290;
                }
            za5b2 = zab[1];
            za5b3 = zab[3];
            za5b4 = zab[5];
            /*     TO ESTIMATE THE ZA AND ZAB VALUES WHEN IX EQ 1 */
L280:
            if ( ixm1 != 0 )
                {
                goto L290;
                }
            za[1] = za[2] + za[2] - za[3];
            za[0] = za[1] + za[1] - za[2];
            za[5] = za[6] + za[6] - za[7];
            za[4] = za[5] + za[5] - za[6];
            zab[0] = zab[1] + zab[1] - za5b2;
            zab[2] = zab[3] + zab[3] - za5b3;
            zab[4] = zab[5] + zab[5] - za5b4;
            goto L300;
            /*     NUMERICAL DIFFERENTATION  ---- TO DETERMINE */
            /*     PARTIAL DERIV. ZX,ZY, AND ZXY AS WEIGHTED MEANS OF */
            /*     DIVIDED DIFFERENCES ZA, ZB, AND ZAB, RESPECTIVELY */

            /* TO SAVE THE OLD VALUES WHEN IX.NE.1 */
L290:
            zx33 = zx[0];
            zx34 = zx[1];
            zy33 = zy[0];
            zy34 = zy[1];
            zxy33 = zxy[0];
            zxy34 = zxy[1];
            /* NEW COMPUTATION */
L300:
            for ( jy = 1; jy <= 2; ++jy )
                {
                w2 = ( r__1 = za[ ( jy << 2 ) - 1] - za[ ( jy << 2 ) - 2], abs ( r__1 ) )
                     ;
                w3 = ( r__1 = za[ ( jy << 2 ) - 3] - za[ ( jy << 2 ) - 4], abs ( r__1 ) )
                     ;
                sw = w2 + w3;
                if ( sw == ( float ) 0. )
                    {
                    goto L310;
                    }
                wx2 = w2 / sw;
                wx3 = w3 / sw;
                goto L320;
L310:
                wx2 = ( float ).5;
                wx3 = ( float ).5;
L320:
                zx[jy - 1] = wx2 * za[ ( jy << 2 ) - 3] + wx3 * za[ ( jy << 2 ) - 2]
                             ;
                w2 = ( r__1 = zb[jy + 2] - zb[jy + 1], abs ( r__1 ) );
                w3 = ( r__1 = zb[jy] - zb[jy - 1], abs ( r__1 ) );
                sw = w2 + w3;
                if ( sw == ( float ) 0. )
                    {
                    goto L330;
                    }
                wy2 = w2 / sw;
                wy3 = w3 / sw;
                goto L340;
L330:
                wy2 = ( float ).5;
                wy3 = ( float ).5;
L340:
                zy[jy - 1] = wy2 * zb[jy] + wy3 * zb[jy + 1];
                zxy[jy - 1] = wy2 * ( wx2 * zab[ ( jy << 1 ) - 2] + wx3 * zab[ ( jy
                                      << 1 ) - 1] ) + wy3 * ( wx2 * zab[ ( jy + 1 << 1 ) - 2] +
                                              wx3 * zab[ ( jy + 1 << 1 ) - 1] );
                /* L350: */
                }
            if ( ixm1 == 0 )
                {
                goto L380;
                }


            /*         DEFINITION OF COORDINATES FOR INTERFACE TO FARBRC */
            xx[0] = x4;
            xx[1] = x3;
            xx[2] = x3;
            xx[3] = x4;
            yy[0] = y4;
            yy[1] = y4;
            yy[2] = y3;
            yy[3] = y3;
            zz[0] = z[ix + iy * z_dim1];
            zz[1] = z[ix - 1 + iy * z_dim1];
            zz[2] = z33;
            zz[3] = z43;
            zzx[0] = zx[1];
            zzy[0] = zy[1];
            zzxy[0] = zxy[1];
            zzx[1] = zx34;
            zzy[1] = zy34;
            zzxy[1] = zxy34;
            zzx[2] = zx33;
            zzy[2] = zy33;
            zzxy[2] = zxy33;
            zzx[3] = zx[0];
            zzy[3] = zy[0];
            zzxy[3] = zxy[0];


            nsides = 3;
            if ( ix == 2 )
                {
                nsides = 4;
                }

            /* set lcanno, annotation on or off */
            lcanno= 0;
            if ( xannincr != 0 && yannincr != 0 )
                {
                int ixtemp;
                ixtemp= ix-2;
                if ( iyann%2 ==0 ) ixtemp= ixtemp + xannincr - 1;
                if ( ixtemp%xannincr == 0 && ( iy-2 ) %yannincr ==0 )
                    lcanno= 1;
                /* don't call if lcanno ==0 */
                if ( lcanno == 0 )
                    {
                    ++ifa;
                    goto L380;
                    }
                nsides = 4;
                }

            if ( *mode != 4 )
                {
                ++ifa;
                lcanno= ifa*lcanno;
                farbrc_ ( xx, yy, zz, zzx, zzy, zzxy, &cn[1], &icol[1], nc, mode,
                          &nsides, &lcanno, usrpsc_ );
                }
            else
                farbmm_ ( xx, yy, zz, zzx, zzy, zzxy, &cn[1], &icol[1], &c__1,
                          &c__1, &nsides, frbcmm_ );

L380:
            ;
            }
        frbfcl_ ( &icol[1] );
        if ( yannincr != 0 && ( iy-2 ) %yannincr == 0 )
            iyann= iyann+1;                      /* counts y-annotations */
        /* L390: */
        }
    /*     NORMAL EXIT */
    return 0;

    /*     ERROR EXIT */
L400:
    fprintf ( stderr,fmt_99999 );
    goto L600;

L410:
    fprintf ( stderr,fmt_99998 );
    goto L600;

L460:
    fprintf ( stderr,fmt_99993 );
    goto L480;
L470:
    fprintf ( stderr,fmt_99992 );
L480:
    fprintf ( stderr,fmt_99991,ix,x[ix] );
    goto L600;

L490:
    fprintf ( stderr,fmt_99990 );
    goto L510;
L500:
    fprintf ( stderr,fmt_99989 );
L510:
    fprintf ( stderr,fmt_99988,iy,y[iy] );
    goto L600;
L530:
    fprintf ( stderr,fmt_99973 );
    goto L550;
L540:
    fprintf ( stderr,fmt_99972 );
L550:
    fprintf ( stderr,fmt_99971,i,cn[i] );
    goto L600;

L560:
    fprintf ( stderr,fmt_99970,*nc );

L600:
    return 0;
    } /* farb2d_ */



/* Subroutine */ int far2d_ ( x, lx, y, ly, z, nxdim, cn, icol, nc )
real *x;
integer *lx;
real *y;
integer *ly;
real *z;
integer *nxdim;
real *cn;
integer *icol, *nc;
    {
    /*  FORMAT STATEMENTS */
    static char fmt_99999[] = "FAR2D:   LX = 1 OR LESS.\n";
    static char fmt_99998[] = "FAR2D:   LY = 1 OR LESS.\n";
    static char fmt_99993[] = "FAR2D:   IDENTICAL X VALUES.\n";
    static char fmt_99992[] = "FAR2D:   X VALUES OUT OF SEQUENCE.\n";
    static char fmt_99991[] = "FAR2D:   IX= %d   X(IX) = %e\n";
    static char fmt_99990[] = "FAR2D:   IDENTICAL Y VALUES.\n";
    static char fmt_99989[] = "FAR2D:   Y VALUES OUT OF SEQUENCE.\n";
    static char fmt_99988[] = "FAR2D:   IY= %d      Y(IY) = %e\n";
    static char fmt_99973[] = "FAR2D:   IDENTICAL CN VALUES.\n";
    static char fmt_99972[] = "FAR2D:   CN VALUES OUT OF SEQUENCE.\n";
    static char fmt_99971[] = "FAR2D:   I= %d      CN(I) = %e\n";
    static char fmt_99970[] = "FAR2D:   NC .LE. 0       NC= %d\n";

    /* System generated locals */
    integer z_dim1, z_offset, i__1, i__2, i__3;
    real r__1;

    /* Builtin functions */
    /*    integer s_wsfe(), e_wsfe(), do_fio(); */

    /* Local variables */
    static integer ncol, i;
    extern /* Subroutine */ int frbfcl_();
    static integer ix, iy;
    extern /* Subroutine */ int frbfop_();
    static real xx[4], yy[4];
    extern /* Subroutine */ int frbfup_();


    /*     FILL AREA OF RECTANGLES FOR A 2D-ARRAY */
    /*     -------------------------------------- */
    /*     FARB-E-2D  VERSION 3.0, 01/1991 */

    /*     AUTHOR: A. PREUSSER */
    /*             FRITZ-HABER-INSTITUT DER MPG */
    /*             FARADAYWEG 4-6 */
    /*             D-1000 BERLIN 33 */

    /*     INPUT PARAMETERS */
    /*     X       ARRAY OF LENGTH LX+1 FOR X-COORDINATES OF */
    /*             A REGULAR GRID */
    /*             IN ASCENDING ORDER. */
    /*     LX      NUMBER OF VALUES IN X-DIRECTION */
    /*     Y       ARRAY OF LENGTH LY+1 FOR Y-COORDINATES */
    /*             IN ASCENDING ORDER. */
    /*     LY      NUMBER OF VALUES IN Y-DIRECTION */
    /*     Z       2-DIMENSIONAL ARRAY DIMENSIONED Z(NXDIM,...) */
    /*             DEFINING THE Z-VALUES FOR THE RECTANGLES DEFINED */
    /*             BY THE GRID LINES X= X(K), Y= Y(L), X= X(K+1), */
    /*             Y= Y(L+1),  K=1,LX, L=1,LY. */
    /*             RECTANGLE K,L RECEIVES VALUE Z(K,L). */
    /*     NXDIM   FIRST DIMENSION OF ARRAY Z */
    /*     CN      ARRAY OF LENGTH NC FOR THE Z-VALUES OF */
    /*             THE LEVELS SEPARATING AREAS OF DIFFERENT */
    /*             COLOURS (IN ASCENDING ORDER). */
    /*     ICOL    INTEGER ARRAY OF LENGTH NC+1 FOR */
    /*             THE COLOURS TO BE USED FOR THE LINES OR AREAS. */
    /*             VALUES FROM THIS ARRAY ARE PASSED TO */
    /*             THE USER SUPPLIED SUBROUTINE USRPLT. */
    /*             ICOL(I) IS USED FOR THE RECTANGLE, WHERE */
    /*                  Z(K,L) .GT. CN(I-1)        AND */
    /*                  Z(K,L) .LE. CN(I), */
    /*             FOR I=2,NC. */
    /*             RECTANGLES, WHERE Z(L,K).LE.CN(1) */
    /*             ARE FILLED WITH COLOUR ICOL(1), */
    /*             AND AREAS, WHERE Z(L,K).GT.ICOL(NC) */
    /*             ARE FILLED WITH COLOUR ICOL(NC+1). */
    /*     NC      NUMBER OF CONTOUR LEVELS */
    /*     lcanno  number of rectangle (used for annotation of
                                        contour lines) */


    /*     OUTPUT */
    /*     IS PERFORMED BY CALLS TO THE SUBROUTINE    USRPLT */
    /*     TO BE SUPPLIED BY THE USER (AN EXAMPLE FOR USRPLT */
    /*     IS INCLUDED AND WILL BE USED IN CASE THE USER DOES */
    /*     NOT SUPPLY HIS OWN ROUTINE). */

    /*     PARAMETERS OF USRPLT */
    /*                SUBROUTINE USRPLT (X,Y,N,NCOL,MODE) */
    /*                X,Y     REAL ARRAYS OF LENGTH N FOR */
    /*                        THE COORDINATES OF THE POLYGON */
    /*                        TO BE PLOTTED. */
    /*                N       NUMBER OF POINTS OF POLYGON */
    /*                NCOL    COLOUR TO BE USED */
    /*                        FOR THE AREA OR THE LINE. */
    /*                        FOR NCOL, THE PROGRAM PASSES */
    /*                        VALUES OF ICOL AS DESCRIBED ABOVE. */
    /*                MODE    1, LINE DRAWING */
    /*                        0, FILL AREA */
    /*     ------------------------------------------------------------- */

    /*     INITIALIZE FILL AREA BUFFER (SET TO CLOSED) */
    /* Parameter adjustments */
    --icol;
    --cn;
    z_dim1 = *nxdim;
    z_offset = z_dim1 + 1;
    z -= z_offset;
    --y;
    --x;

    /* Function Body */
    frbcob_1.nfabu = 0;
    /*     INITIALIZE COLOUR OF FILL AREA BUFFER */
    frbcob_1.ncolbu = 0;

    /*     ERROR CHECKS */

    if ( *lx < 1 )
        {
        goto L3400;
        }
    if ( *ly < 1 )
        {
        goto L3410;
        }
    i__1 = *lx + 1;
    for ( ix = 2; ix <= i__1; ++ix )
        {
        if ( ( r__1 = x[ix - 1] - x[ix] ) < ( float ) 0. )
            {
            goto L20;
            }
        else if ( r__1 == 0 )
            {
            goto L3460;
            }
        else
            {
            goto L3470;
            }
L20:
        ;
        }

    i__1 = *ly + 1;
    for ( iy = 2; iy <= i__1; ++iy )
        {
        if ( ( r__1 = y[iy - 1] - y[iy] ) < ( float ) 0. )
            {
            goto L70;
            }
        else if ( r__1 == 0 )
            {
            goto L3490;
            }
        else
            {
            goto L3500;
            }
L70:
        ;
        }

    i__1 = *nc;
    for ( i = 2; i <= i__1; ++i )
        {
        if ( ( r__1 = cn[i - 1] - cn[i] ) < ( float ) 0. )
            {
            goto L80;
            }
        else if ( r__1 == 0 )
            {
            goto L3530;
            }
        else
            {
            goto L3540;
            }
L80:
        ;
        }

    if ( *nc <= 0 )
        {
        goto L3560;
        }

    i__1 = *ly;
    for ( iy = 1; iy <= i__1; ++iy )
        {
        yy[2] = y[iy];
        yy[3] = y[iy];
        yy[0] = y[iy + 1];
        yy[1] = y[iy + 1];

        i__2 = *lx;
        for ( ix = 1; ix <= i__2; ++ix )
            {
            xx[1] = x[ix];
            xx[2] = x[ix];
            xx[0] = x[ix + 1];
            xx[3] = x[ix + 1];
            /*         DETERMINE COLOUR */
            ncol = 1;
            i__3 = *nc;
            for ( i = 1; i <= i__3; ++i )
                {
                if ( z[ix + iy * z_dim1] <= cn[i] )
                    {
                    goto L510;
                    }
                ++ncol;
                /* L500: */
                }
L510:

            if ( ncol == frbcob_1.ncolbu )
                {
                frbfup_ ( xx, yy );
                }
            if ( ncol != frbcob_1.ncolbu )
                {
                frbfop_ ( xx, yy, &icol[1], &ncol );
                }

            /* L1000: */
            }
        frbfcl_ ( &icol[1] );
        frbcob_1.ncolbu = 0;
        /* L2000: */
        }

    return 0;

    /*     ERROR EXIT */
L3400:
    fprintf ( stderr,fmt_99999 );
    goto L3600;

L3410:
    fprintf ( stderr,fmt_99998 );
    goto L3600;

L3460:
    fprintf ( stderr,fmt_99993 );
    goto L3480;
L3470:
    fprintf ( stderr,fmt_99992 );
L3480:
    fprintf ( stderr,fmt_99991,ix,x[ix] );
    goto L3600;

L3490:
    fprintf ( stderr,fmt_99990 );
    goto L3510;
L3500:
    fprintf ( stderr,fmt_99989 );
L3510:
    fprintf ( stderr,fmt_99988,iy,y[iy] );
    goto L3600;
L3530:
    fprintf ( stderr,fmt_99973 );
    goto L3550;
L3540:
    fprintf ( stderr,fmt_99972 );
L3550:
    fprintf ( stderr,fmt_99971,i,cn[i] );
    goto L3600;

L3560:
    fprintf ( stderr,fmt_99970,*nc );

L3600:
    return 0;
    } /* far2d_ */

/* Subroutine */ int farbrc_ ( x, y, z, zx, zy, zxy, cn, icol, nc,
                               mode, nsides, lcanno, usrpsc_ )

real *x, *y, *z, *zx, *zy, *zxy, *cn;
integer *icol, *nc, *mode, *nsides, *lcanno;
int ( *usrpsc_ ) ();
    {
    /* Initialized data */

    static real xstack[400];
    static real ystack[400];
    static integer it = 0;
    static real zs = ( float ) 0.;

    /* Format strings */
    static char fmt_8999[] = "FARBRC:  (NUMBER OF CONTOURLEVELS NC= %d)\
  .GT. (NCMAX= %d)\n";
    static char fmt_9000[] = "FARBRC:  CONTOURLEVEL %d =%e\
  .LE. CONTOURLEVEL %d =%e\n";
    static char fmt_9010[] = "FARBRC:  Y(1).NE.Y(2)  OR   \
 X(2).GE.X(1)\n IN RECTANGLE NO. %d\n VERTICES MUST BE ORDERED\
  COUNTER-CLOCKWISE \n STARTING IN THE UPPER RIGHT CORNER.\n \
 SIDES MUST BE PARALLEL TO X- AND Y-AXIS\n";
    static char fmt_9020[] = "FARBRC: VERTICES OF RECTANGLE NO.\
  %d  NOT IN COUNTER-CLOCKWISE ORDER\n";
    static char fmt_9030[] = "FARBRC: ***WARNING***, CHECK RECTANGLE NO. %d\
 \n DIFFERENCE IN X- OR Y- COORDINATES TOO LARGE OR TOO SMALL\n \
 XYDIF= %e \n SCALE X AND/OR Y TO CM (OR INCH)\n";
    static char fmt_6001[] = "FARBRC: MORE THAN \
  %d CONTOURS CROSSING A SIDE OF A RECTANGLE.\n \
  INCREASE INSTALLATION PARAMETER NCMAXS\n";
    static char fmt_7001[] = "FARBRC: OVERFLOW\
  OF WORKING STORAGE XPOL,YPOL\n MAXPOL= %d, RECT.NO. %d\n";

    /* System generated locals */
    integer i__1, i__2, i__3, i__4;
    real r__1, r__2, r__3;

    /* Builtin functions */
    double atan();
    integer s_wsfe(), do_fio(), e_wsfe();
    double r_sign();

    /* Local variables */
    static integer kcll, nfar, ncol, ndir, jsar[6], jser[6], jpol, jzar[6];
    static real epsn, zx3b3;
    static integer jzer[6];
    static real zx4b3, zy3a3, zy4a3, xpol[300], ypol[300];
    static integer npst[4], npol1;
    static real a, b, c, d, e;
    static integer i, j, k, ncdif;
    static real t;
    static integer iride, nside;
    static real dernr[6], derns[4];
    static integer ndirs, ndirv, jsast[4], jpolr[6];
    static real f1;
    static integer jsest[4], npolr[6], jzast[4], jfrst[6];
    static real f2;
    static integer jzest[4];
    static real t1[16]  /* was [4][4] */, z1[16]    /* was [4][4] */,
           zmaxt, zmint;
    static integer jtost[6];
    static integer kvert, njour, nstop, nfrst;
    static real derno1;
    static integer jpoll1;
    static real cc[1024], fa, fb;
    static integer in[4], ii;
    static real tb;
    extern real frbeva_();
    static real ta, xx[2];
    extern /* Subroutine */ int frbfop_();
    static real zz[4], yy[2];
    extern real frbzer_();
    static real di2, di3, cn1;
    extern /* Subroutine */ int frbfup_();
    static integer ic1, jn, ni;
    static real tperrl;
    extern /* Subroutine */ int frbfcl_();
    static integer is, istart, jj, np1;
    static real sl1[4];
    static integer jstopr[6], np2;
    static real tperrs;
    extern /* Subroutine */ int frbrid_();
    static integer np, journy, jsides, ncdifa;
    static real ts2[4];
    static integer nc1;
    static real sdchek;
    static integer ir, jfstir, js, jr, jz;
    extern /* Subroutine */ int posfil_();
    static real f1f2;
    static integer kcl, icn, jsa;
    static real cnn, z3a3, sl12[4];
    static integer jin, nse;
    static real eps, sl01;
    static integer jza, jse, jze, jjz, jzn, jzr, icn1, ncl1, jsa2, jza2;
    static integer lmode;

    /*                              - 3 -
    /*  VERTEX(2) * ----------------------0------------------* VERTEX(1) */
    /*            (                         .                ) */
    /*            (                           .              ) - 2 -*/
    /*    - 4 -   (                             .            ) */
    /*            (                                .         ) */
    /*  VERTEX(3) * ----------------------------------0----- * VERTEX(4) */
    /*                            SIDE(1)                        X */

    /*     THE SIDES ARE PARALLEL TO THE CARTESIAN X-Y-SYSTEM. */

    /*   ----------------------------------------------------------------- */
    /*   END OF USER DOCUMENTATION */
    /*   ----------------------------------------------------------------- */

    /*           SOME SPECIAL TERMS */

    /*     STATION      ZERO ON A SIDE */
    /*     RIDE         MOVE FROM ONE STATION TO ANOTHER INSIDE RECT. */
    /*     TRANSFER     MOVE FROM ONE STATION TO THE NEXT ON SIDE */
    /*     TRIP         SEQUENCE OF RIDES AND TRANSFERS */
    /*     ROUND TRIP   SUCCESSFUL TRIP THAT ENDED AT ITS START */
    /*     HORROR TRIP  TRIP THAT DOES NOT FIND AN END */
    /*     JOURNEY      SEQUENCE OF TRIPS STARTING FROM THE SAME */
    /*                  TYPE OF STATIONS (SAME VALUE OF ISTATZ) */
    /*                  AND HAVING THE SAME ORIENTATION. */

    /*                  THERE MAY BE THREE JOURNEYS. */
    /*                  THE FIRST TWO ARE COUNTER-CLOCKWISE AND */
    /*                  START AT STATIONS WITH ISTATZ=0 AND =2, */
    /*                  RESPECTIVELY. */
    /*                  THE THIRD JOURNEY IS CARRIED */
    /*                  OUT ONLY IN CASE OF NUMERICAL DIFFICULTIES, */
    /*                  WHEN AREAS ARE UNFILLED AFTER THE FIRST TWO. */
    /*                  IT STARTS AT STATIONS WITH ISTATZ.LE.2 AND */
    /*                  IS CLOCKWISE. */



    /*     NFABU       0, FILL AREA BUFFER CLOSED */
    /*                 1, FILL AREA BUFFER OPEN */
    /*     NCOLBU      COLOUR OF FILL AREA BUFFER */
    /*     XFABU,YFABU X-Y COORDINATES OF FILL AREA BUFFER */

    /*     SACMIN     MINIMAL DISTANCE OF TWO POINTS TO BE STORED */
    /* *** CMSCAL     VARIABLE FOR SWITCHING BETWEEN CM AND INCH */
    /*     MAXPOL     MAXIMUM NUMBER OF POINTS FOR A TRIP */
    /*     NCPMAX     MAXIMUM NUMBER OF POINTS TO BE COMPUTED FOR A RIDE */
    /*     MAXSTA     MAXIMUM NUMBER OF POINTS TO BE STORED FOR A RIDE */
    /*     NCMAXS     MAXIMUM NUMBER OF CONTOURS CROSSING A RECTANGLE SIDE */
    /*     NPP        ACCUMULATED NUMBER OF POINTS FOR A SEQUENCE OF */
    /*                RECTANGLES */
    /*     PI         3.141... */
    /*     MAXRID     MAXIMUM NUMBER OF RIDES FOR A TRIP */
    /*     SIGS       SIGN FOR SIDES (+1 OR -1) */
    /*     NCMAX      MAXIMUM NUMBER OF CONTOUR LEVELS */


    /*     /FRBCOF/ CONTAINS VARIABLES WHICH ARE PASSED TO FUNCTION */
    /*              FRBEVA AS PARAMETERS */

    /*     KK          INDEX  OF FUNCTION TO BE EVALUATED BY FRBEVA */
    /*     KSE         ACTUAL SIDE INDEX */
    /*     XX4F,YY4F   COORDINATES FOR POINT P4F (PRELEMINARY POSITION */
    /*                 OF POINT P4) */
    /*     SIR,COR     COSINUS OF DIRECTION NORMAL TO CURVE DIRECTION */
    /*     CL          ACTUAL CONTOUR LEVEL */


    /*      P0,P1,P2,P3    COEFFICIENTS FOR THE POLYNOMIALS */
    /*                     ON THE 4 SIDES . */
    /*                     VARIABLES ON SIDES 1 AND 2 ARE */
    /*                     COUNTER-CLOCKWISE, ON SIDES 3 AND 4 */
    /*                     CLOCKWISE. */
    /*      Q0,Q1,Q2       COEFFICIENTS FOR THE DERIVATIVES */
    /*                     OF THE POLYNOMIALS ON THE 4 SIDES. */
    /*      R0,R1          COEFF. FOR THE SECOND DERIVATIVES. */
    /*      P11...P33      COEFF. OF POLYNOMIALS USED TOGETHER */
    /*                     WITH P0(I)...P3(I), I=1 AND I=4, FOR THE */
    /*                     REPRESENTATION INSIDE THE RECTANGLE. */


    /*     FRBCRD CONTAINS VARIABLES THAT ARE PASSED TO FRBRID */
    /*            OR THAT ARE RETAINED FOR THE NEXT CALL TO FARBRC (NSIDE=3)
    */
    /*     X0,Y0       X-Y-COORDINATES OF ZEROS ON SIDES */
    /*     NCLZR       CONTOUR LEVEL FOR ZEROS */
    /*     SDER        DERIVATIVE IN DIRECTION OF SIDES */
    /*                 (SIDE DIRECTION= COUNTER CLOCKWISE) */
    /*     TZR         COORDINATES FOR ZEROS (STATIONS) ON SIDES */
    /*     NZ          NUMBER OF ZEROS ON SIDES */
    /*     SI,CO       COSINUS OF DIRECTION FOR SIDES */
    /*     SA,SE       VALUES OF VARIABLES AT START AND END OF SIDES */
    /*     DX,DY       DIFFERENCES OF X AND Y */
    /*     SL          SIDE LENGTHS */
    /*     HMIN        LENGTH OF SHORTEST SIDE OF RECTANGLE */
    /*     SLMAX       LENGTH OF LONGEST SIDE */
    /*     KRIDE       COUNTS THE CALLS TO FRBRID */
    /*     NPREC       NUMBER OF POINTS FOR THE RECTANGLE */
    /*     POSERR      PERMITTED POSITION ERROR */
    /*     DERNOR      SIGN OF NORMAL DERIVATIVE FOR A RIDE (+1 OR -1) */
    /*     NDIR3       1, FOR COUNTER-CLOCKWISE TRIP */
    /*                 -1, FOR CLOCKWISE TRIP */
    /*     ZMIN,ZMAX    MIN. AND MAX. VALUES OF Z ON SIDES */
    /*     ISTATZ       STATUS OF ZEROS ON SIDES */
    /*                  FIRST INDEX :ZERO, SECOND INDEX: SIDE. */
    /*                  THE INITIAL STATUS IS = 0 */
    /*                  WHEN A ZERO HAS SERVED AS START +1 IS ADDED. */
    /*                  WHEN A ZERO HAS SERVED AS END +2 IS ADDED. */
    /*                  SO AFTER TWO JOURNEYS, ISTATZ SHOULD BE 3, */
    /*                  SINCE EVERY STATION SHOULD HAVE BEEN USED */
    /*                  TWICE: ONCE AS START, AND ONCE AS END OF A RIDE. */
    /*     X3,Y3        COORDINATES X(3),Y(3) */
    /*     ZSOLD        ZS OF LAST CALL */


    /*     CC           SCALED CONTOUR LEVELS */
    /*     ZZ           SCALED Z-VALUES AT VERTICES */
    /*     T1           COORDINATES ON SIDES AT ENDPOINTS OF INTERVALS */
    /*     Z1           Z-VALUES AT ENDPOINTS OF INTERVALS */
    /*     TS2          WORKING ARRAY FOR COMPUTING T1 */
    /*     IN           NUMBER OF INTERVALS ON SIDES */
    /*     XPOL,YPOL    X,Y-COORDINATES FOR FILL AREA POLYGON */
    /*     XSTACK,YSTACK X,Y COORDINATES OF STACK */
    /*                  (COORDINATES OF THE FIRST RIDE OF A TRIP) */
    /*     XX,YY        LOCAL COPIES OF X,Y */
    /*     JSAR,JZAR    SIDE AND ZERO FOR START OF RIDES */
    /*     JSER,JZER    SIDE AND ZERO FOR END OF RIDES */
    /*     JSTOPR       STOP MODES OF RIDES */
    /*     JSAST,JZAST  SIDE AND ZERO FOR START OF RIDES IN STACK */
    /*     JSEST,JZEST  SIDE AND ZERO FOR END OF RIDES IN STACK */
    /*     JPOLR        INDEX IN XPOL,YPOL FOR START OF RIDES */
    /*     NPST         NUMBER OF POINTS USED IN STACKS */
    /*     DERNS        NORMAL DERIVATIVE IN STACKS */
    /*     DERNR        NORMAL DERIVATIVE OF RIDES */
    /*     NPOLR        NUMBER OF POINTS OF RIDES */
    /*     JFRST        NO. OF STACK FROM WHICH RIDE IS COPIED */
    /*     JTOST        NO. OF STACK TO WHICH RIDE IS COPIED */
    /*     SL1          1./SIDE LENGTH */
    /*     SL12         SL1**2 */


    /* Parameter adjustments */
    --icol;
    --cn;
    --zxy;
    --zy;
    --zx;
    --z;
    --y;
    --x;

    /* Function Body */


    /*     INITIALISATION FOR FIRST RECTANGLE */
    ++it;
    /* ALT Commented out this stmnt    if (it%100==0) gwatchp(it); * ALT */

    /*     SET INSTALLATION PARAMETERS */

    if ( it != 1 )
        {
        goto L10;
        }
    frbcoc_1.cmscal = ( float ) 1.;
    /* *** SET  CMSCAL= 2.54  FOR INCH CALIBRATED PLOTTERS */
    frbcoc_1.sacmin = ( float ).02 / frbcoc_1.cmscal;
    frbcoc_1.maxpol = 300;
    frbcoc_1.ncmaxs = 256;
    frbcoc_1.ncmax = 1024;
    frbcoc_1.maxsta = 100;
    frbcoc_1.maxrid = 6;
    frbcoc_1.ncpmax = frbcoc_1.maxsta * 10;
    frbcoc_1.npp = 0;
    frbcob_1.nfabu = 0;
    frbcoc_1.pi = atan ( ( float ) 1. ) * ( float ) 4.;
    frbcoc_1.sigs[0] = ( float ) 1.;
    frbcoc_1.sigs[1] = ( float ) 1.;
    frbcoc_1.sigs[2] = ( float )-1.;
    frbcoc_1.sigs[3] = ( float )-1.;
L10:

    /*     CHECK NUMBER OF CONTOUR LEVELS */
    if ( *nc > frbcoc_1.ncmax )
        {
        fprintf ( stderr,fmt_8999,*nc,frbcoc_1.ncmax );

        }

    nfar = 0;
    /*         = NUMBER OF FILL AREA CALLS */
    frbcrd_1.kride = 0;
    /*       = NUMBER OF CALLS TO FRBRID */
    frbcrd_1.nprec = 0;
    /*          = NUMBER OF CURVE POINTS COMPUTED FOR RECTANGLE */

    /*     CHECK CONTOUR VALUES FOR MONOTONY, */
    /*     IF CONTOUR LINE PASSES THROUGH DATA POINT ON */
    /*     SIDE 4, SET NSIDE TO 4 (NO COPY FROM LAST RECTANGLE) */
    /*     SCALE CONTOUR LEVELS */
    nside = *nsides;
    if ( cn[1] == z[2] || cn[1] == z[3] )
        {
        nside = 4;
        }
    frbcrd_1.zsold = zs;
    zs = ( z[1] + z[2] + z[3] + z[4] ) / ( float ) 4.;
    cc[0] = cn[1] - zs;
    i__1 = *nc;
    for ( kcl = 2; kcl <= i__1; ++kcl )
        {
        cc[kcl - 1] = cn[kcl] - zs;
        if ( cn[kcl] == z[2] || cn[kcl] == z[3] )
            {
            nside = 4;
            }
        if ( cn[kcl] - cn[kcl - 1] > ( float ) 0. )
            {
            goto L20;
            }
        i__2= kcl-1;
        fprintf ( stderr,fmt_9000,kcl,cn[kcl],i__2,cn[kcl - 1] );

L20:
        ;
        }


    /*     SOME BASIC GEOMETRY FOR THE RECTANGLE */

    frbcrd_1.sl[0] = x[4] - x[3];
    frbcrd_1.sl[1] = y[1] - y[4];
    frbcrd_1.sl[2] = x[1] - x[2];
    frbcrd_1.sl[3] = y[2] - y[3];
    frbcrd_1.x3 = x[3];
    frbcrd_1.y3 = y[3];
    for ( j = 1; j <= 4; ++j )
        {
        zz[j - 1] = z[j] - zs;
        np1 = ( j + 1 ) % 4 + 1;
        np2 = ( j + 2 ) % 4 + 1;
        frbcrd_1.dx[j - 1] = x[np2] - x[np1];
        frbcrd_1.dy[j - 1] = y[np2] - y[np1];
        sl1[j - 1] = ( float ) 1. / frbcrd_1.sl[j - 1];
        sl12[j - 1] = sl1[j - 1] * sl1[j - 1];
        frbcrd_1.co[j - 1] = frbcrd_1.dx[j - 1] / frbcrd_1.sl[j - 1];
        frbcrd_1.si[j - 1] = frbcrd_1.dy[j - 1] / frbcrd_1.sl[j - 1];
        frbcrd_1.sa[j - 1] = ( float ) 0.;
        if ( j > 2 )
            {
            frbcrd_1.sa[j - 1] = frbcrd_1.sl[j - 1];
            }
        frbcrd_1.se[j - 1] = frbcrd_1.sl[j - 1];
        if ( j > 2 )
            {
            frbcrd_1.se[j - 1] = ( float ) 0.;
            }
        /* L50: */
        }
    frbcrd_1.slmax = max ( frbcrd_1.sl[0],frbcrd_1.sl[1] );
    di2 = - ( frbcrd_1.dy[0] * frbcrd_1.co[1] - frbcrd_1.dx[0] * frbcrd_1.si[1] )
          ;
    di3 = ( r__1 = frbcrd_1.dy[1] * frbcrd_1.co[2] - frbcrd_1.dx[1] *
                   frbcrd_1.si[2], abs ( r__1 ) );

    /*     CHECK COORDINATES OF VERTICES */
    if ( y[1] != y[2] || x[2] >= x[1] )
        {
        fprintf ( stderr, fmt_9010, it );
        }

    /*     CHECK IF VERTICES ARE NUMBERED COUNTER-CLOCKWISE */
    if ( di2 < ( float ) 0. )
        {
        fprintf ( stderr, fmt_9020, it );
        }

    frbcrd_1.hmin = min ( di2,di3 );
    /*         = SHORTEST SIDE LENGTH */

    /*     CHECK HMIN */
    if ( frbcrd_1.hmin < ( float ).01 / frbcoc_1.cmscal || frbcrd_1.hmin > (
                float ) 100. / frbcoc_1.cmscal )
        {
        fprintf ( stderr, fmt_9030, it, frbcrd_1.hmin );
        }
    if ( frbcrd_1.hmin == ( float ) 0. )
        {
        goto L5000;
        }

    /* Computing MIN */
    r__1 = ( float ).0001 / frbcoc_1.cmscal, r__2 = frbcrd_1.hmin * ( float ).0001
            / frbcrd_1.slmax;
    frbcrd_1.poserr = min ( r__1,r__2 );
    tperrl = ( float ).01;
    /*           = PERMITTED POSITION ERROR */

    /*     COPY INFORMATION FOR SIDE 4 */
    if ( nside == 4 )
        {
        goto L80;
        }
    jzn = frbcrd_1.nz[1];
    frbcrd_1.nz[3] = jzn;
    frbcrd_1.zmin[3] = frbcrd_1.zmin[1] + frbcrd_1.zsold - zs;
    frbcrd_1.zmax[3] = frbcrd_1.zmax[1] + frbcrd_1.zsold - zs;
    if ( jzn == 0 )
        {
        goto L80;
        }
    jjz = jzn;
    i__1 = jzn;
    for ( jzr = 1; jzr <= i__1; ++jzr )
        {
        frbcrd_1.x0[jjz + 767] = ( float ) 0.;
        frbcrd_1.y0[jjz + 767] = frbcrd_1.y0[jzr + 255];
        frbcrd_1.nclzr[jjz + 767] = frbcrd_1.nclzr[jzr + 255];
        frbcrd_1.sder[jjz + 767] = -frbcrd_1.sder[jzr + 255];
        frbcrd_1.tzr[jjz + 767] = frbcrd_1.tzr[jzr + 255];
        frbcrd_1.istatz[jjz + 767] = 0;
        --jjz;
        /* L70: */
        }
L80:

    /*     COMPUTE COEFFICIENTS FOR POLYNOMIALS ALONG SIDES */

    z3a3 = ( zz[3] - zz[2] ) * sl1[0];
    frbcop_1.p0[0] = zz[2];
    frbcop_1.p1[0] = zx[3];
    frbcop_1.p2[0] = ( ( z3a3 - zx[3] ) * ( float ) 2. + z3a3 - zx[4] ) * sl1[0];
    frbcop_1.p3[0] = ( z3a3 * ( float )-2. + zx[4] + zx[3] ) * sl12[0];

    z3a3 = ( zz[1] - zz[2] ) * sl1[3];
    frbcop_1.p0[3] = zz[2];
    frbcop_1.p1[3] = zy[3];
    frbcop_1.p2[3] = ( ( z3a3 - zy[3] ) * ( float ) 2. + z3a3 - zy[2] ) * sl1[3];
    frbcop_1.p3[3] = ( z3a3 * ( float )-2. + zy[2] + zy[3] ) * sl12[3];

    z3a3 = ( zz[0] - zz[1] ) * sl1[2];
    frbcop_1.p0[2] = zz[1];
    frbcop_1.p1[2] = zx[2];
    frbcop_1.p2[2] = ( ( z3a3 - zx[2] ) * ( float ) 2. + z3a3 - zx[1] ) * sl1[2];
    frbcop_1.p3[2] = ( z3a3 * ( float )-2. + zx[1] + zx[2] ) * sl12[2];

    z3a3 = ( zz[0] - zz[3] ) * sl1[1];
    frbcop_1.p0[1] = zz[3];
    frbcop_1.p1[1] = zy[4];
    frbcop_1.p2[1] = ( ( z3a3 - zy[4] ) * ( float ) 2. + z3a3 - zy[1] ) * sl1[1];
    frbcop_1.p3[1] = ( z3a3 * ( float )-2. + zy[1] + zy[4] ) * sl12[1];

    /*     DETERMINE POLYNOMIAL COEFF FOR DERIVATIVES ALONG SIDES */
    for ( j = 1; j <= 4; ++j )
        {
        frbcop_1.q0[j - 1] = frbcop_1.p1[j - 1];
        frbcop_1.q1[j - 1] = frbcop_1.p2[j - 1] * ( float ) 2.;
        frbcop_1.q2[j - 1] = frbcop_1.p3[j - 1] * ( float ) 3.;
        frbcop_1.r0[j - 1] = frbcop_1.q1[j - 1];
        frbcop_1.r1[j - 1] = frbcop_1.q2[j - 1] * ( float ) 2.;
        /* L90: */
        }

    /*     SET CONTOUR LEVEL TO BE PASSED TO FRBEVA */
    frbcof_1.cl = ( float ) 0.;


    /*     FIND POINTS ON SIDES OF RECTANGLE, */
    /*     WHERE FIRST DERIVATIVE IS ZERO */

    /*     LOOP OVER SIDES */
    i__1 = nside;
    for ( jsa = 1; jsa <= i__1; ++jsa )
        {
        frbcof_1.kse = jsa;
        tperrs = tperrl * frbcrd_1.sl[frbcof_1.kse - 1];
        /*       SET INITIAL ENDPOINTS OF INTERVALS */
        t1[ ( frbcof_1.kse << 2 ) - 4] = frbcrd_1.sa[frbcof_1.kse - 1];
        t1[ ( frbcof_1.kse << 2 ) - 3] = frbcrd_1.se[frbcof_1.kse - 1];
        i = 2;
        /*       LOOP OVER DERIVATIVES */
        for ( k = 3; k <= 4; ++k )
            {

            /*         SET FUNCTION TO BE EVALUATED BY FRBEVA */
            frbcof_1.kk = k;

            ts2[0] = t1[ ( frbcof_1.kse << 2 ) - 4];
            ii = 2;
            tb = t1[ ( frbcof_1.kse << 2 ) - 4];
            f2 = frbeva_ ( &tb );
            /*         LOOP OVER ENDPOINTS OF INTERVALS */
            i__2 = i;
            for ( j = 2; j <= i__2; ++j )
                {
                ta = tb;
                f1 = f2;
                tb = t1[j + ( frbcof_1.kse << 2 ) - 5];
                f2 = frbeva_ ( &tb );
                if ( f1 * f2 > ( float ) 0. )
                    {
                    goto L100;
                    }
                if ( f1 == ( float ) 0. && f2 == ( float ) 0. )
                    {
                    goto L100;
                    }
                /* L95: */
                ts2[ii - 1] = frbzer_ ( &ta, &tb, &f1, &f2, &tperrs,
                                        frbeva_ );
                ++ii;
L100:
                ;
                }
            ts2[ii - 1] = t1[i + ( frbcof_1.kse << 2 ) - 5];
            i__2 = ii;
            for ( j = 1; j <= i__2; ++j )
                {
                /* L120: */
                t1[j + ( frbcof_1.kse << 2 ) - 5] = ts2[j - 1];
                }
            i = ii;
            tperrs *= ( float ).1;
            /* L150: */
            }
        /*       IN(KSE)= NUMBER OF INTERVALS */
        --i;
        in[frbcof_1.kse - 1] = i;
        /*       (E.G. IF IN(KSE)=1, THERE IS NO POINT FOR WHICH 1ST DER.=0)
        */

        /*       COMPUTE MAXIMA AND MINIMA FOR EACH SIDE */
        np1 = ( frbcof_1.kse + 1 ) % 4 + 1;
        np2 = ( frbcof_1.kse + 2 ) % 4 + 1;
        /* Computing MAX */
        r__1 = zz[np1 - 1], r__2 = zz[np2 - 1];
        frbcrd_1.zmax[frbcof_1.kse - 1] = max ( r__1,r__2 );
        /* Computing MIN */
        r__1 = zz[np1 - 1], r__2 = zz[np2 - 1];
        frbcrd_1.zmin[frbcof_1.kse - 1] = min ( r__1,r__2 );
        z1[ ( frbcof_1.kse << 2 ) - 4] = zz[np1 - 1];
        z1[i + 1 + ( frbcof_1.kse << 2 ) - 5] = zz[np2 - 1];
        if ( i == 1 )
            {
            goto L170;
            }
        frbcof_1.kk = 1;
        i__2 = i;
        for ( j = 2; j <= i__2; ++j )
            {
            z1[j + ( frbcof_1.kse << 2 ) - 5] = frbeva_ ( &t1[j + ( frbcof_1.kse <<
                                                2 ) - 5] );
            if ( z1[j + ( frbcof_1.kse << 2 ) - 5] > frbcrd_1.zmax[frbcof_1.kse
                    - 1] )
                {
                frbcrd_1.zmax[frbcof_1.kse - 1] = z1[j + ( frbcof_1.kse << 2 )
                                                     - 5];
                }
            if ( z1[j + ( frbcof_1.kse << 2 ) - 5] < frbcrd_1.zmin[frbcof_1.kse
                    - 1] )
                {
                frbcrd_1.zmin[frbcof_1.kse - 1] = z1[j + ( frbcof_1.kse << 2 )
                                                     - 5];
                }
            /* L160: */
            }
L170:
        /* L200: */
        ;
        }

    /*     CHECK, IF RECTANGLE HAS ONE COLOUR, */
    /*     BECAUSE THE MINIMUM IS OVER THE MAX. CONTOUR LEVEL, */
    /*     OR MAXIMUM UNDER MIN. CONTOUR LEVEL */
    /* Computing MAX */
    r__1 = max ( frbcrd_1.zmax[0],frbcrd_1.zmax[1] ), r__1 = max ( r__1,
            frbcrd_1.zmax[2] );
    zmaxt = max ( r__1,frbcrd_1.zmax[3] );
    /* Computing MIN */
    r__1 = min ( frbcrd_1.zmin[0],frbcrd_1.zmin[1] ), r__1 = min ( r__1,
            frbcrd_1.zmin[2] );
    zmint = min ( r__1,frbcrd_1.zmin[3] );
    cn1 = cc[0];
    cnn = cc[*nc - 1];
    if ( cnn >= zmint && cn1 < zmaxt )
        {
        goto L500;
        }
    ncol = 1;
    if ( cnn < zmint )
        {
        ncol = *nc + 1;
        }

    /*     RECTANGLE HAS ONE COLOUR ONLY */
L250:
    if ( abs ( *mode ) == 1 )
        {
        goto L400;
        }
    if ( frbcob_1.nfabu == 1 && ncol == frbcob_1.ncolbu )
        {
        goto L300;
        }
    frbfop_ ( &x[1], &y[1], &icol[1], &ncol );
    goto L400;
L300:
    frbfup_ ( &x[1], &y[1] );
L400:
    frbcrd_1.nz[1] = 0;
    goto L5000;
L500:

    /*     FIND MIN. AND MAX. CONTOUR LEVEL FOR RECTANGLE */
    i__1 = *nc;
    for ( kcl = 1; kcl <= i__1; ++kcl )
        {
        ic1 = kcl;
        if ( cc[kcl - 1] >= zmint )
            {
            goto L610;
            }
        /* L600: */
        }
L610:
    icn = *nc + 1;
    i__1 = *nc;
    for ( kcl = 1; kcl <= i__1; ++kcl )
        {
        --icn;
        if ( cc[icn - 1] <= zmaxt )
            {
            goto L660;
            }
        /* L650: */
        }
L660:
    /*     IC1= FIRST CONTOUR LEVEL */
    /*     ICN= LAST CONTOUR LEVEL */
    icn1 = icn - ic1 + 1;
    ncol = icn + 1;
    if ( cc[icn - 1] == zmaxt )
        {
        ncol = icn;
        }
    if ( icn1 == 0 )
        {
        goto L250;
        }

    /*     COMPUTE ZEROS ON SIDES FOR ALL CONTOUR LEVELS */
    /*     IN COUNTER-CLOCKWISE ORDER */

    i__1 = nside;
    for ( jsa = 1; jsa <= i__1; ++jsa )
        {
        frbcof_1.kse = jsa;
        jn = 0;
        if ( cc[icn - 1] < frbcrd_1.zmin[frbcof_1.kse - 1] || cc[ic1 - 1] >
                frbcrd_1.zmax[frbcof_1.kse - 1] )
            {
            goto L850;
            }
        ni = in[frbcof_1.kse - 1];
        i__2 = ni;
        for ( jin = 1; jin <= i__2; ++jin )
            {

            r__1 = z1[jin + 1 + ( frbcof_1.kse << 2 ) - 5] - z1[jin + (
                        frbcof_1.kse << 2 ) - 5];
            ndir = r_sign ( &c_b5, &r__1 );
            kcl = ic1 - 1;
            if ( ndir == -1 )
                {
                kcl = icn + 1;
                }

            i__3 = icn1;
            for ( kcll = 1; kcll <= i__3; ++kcll )
                {
                kcl += ndir;
                frbcof_1.cl = cc[kcl - 1];
                f1 = z1[jin + ( frbcof_1.kse << 2 ) - 5] - frbcof_1.cl;
                f2 = z1[jin + 1 + ( frbcof_1.kse << 2 ) - 5] - frbcof_1.cl;
                if ( f1 == ( float ) 0. || f2 == ( float ) 0. )
                    {
                    goto L665;
                    }
                f1f2 = r_sign ( &c_b5, &f1 ) * r_sign ( &c_b5, &f2 );
                if ( f1f2 > ( float ) 0. )
                    {
                    goto L700;
                    }
                if ( f1f2 < ( float ) 0. )
                    {
                    goto L690;
                    }

                /*           SPECIAL SITUATIONS */

L665:
                if ( ni == 1 && f1 == ( float ) 0. && f2 == ( float ) 0. )
                    {
                    goto L670;
                    }
                /*           IF () THEN CONTOURLINE = SIDE KSE */

                if ( f1 == ( float ) 0. && ( r__1 = t1[jin + ( frbcof_1.kse << 2 ) -
                                                       5] - t1[ ( frbcof_1.kse << 2 ) - 4], abs ( r__1 ) ) <=
                        frbcrd_1.poserr * ( float ) 3. )
                    {
                    goto L700;
                    }
                /*           IF () THEN LINE PASSES THROUGH A VERTEX AT START
                OF SIDE */
                /*                 THIS CASE IS HANDLED ON PREVIOUS SIDE */

                if ( f2 == ( float ) 0. && ( r__1 = t1[jin + 1 + ( frbcof_1.kse <<
                                                       2 ) - 5] - t1[ni + 1 + ( frbcof_1.kse << 2 ) - 5], abs (
                                                 r__1 ) ) <= frbcrd_1.poserr * ( float ) 3. )
                    {
                    goto L680;
                    }
                /*           IF () THEN LINE PASSES THROUGH VERTEX AT END OF S
                IDE */
                goto L690;

                /*           CONTOUR LINE = SIDE JSA */
L670:
                kvert = ( jsa + 1 ) % 4 + 1;
                xx[0] = x[kvert];
                yy[0] = y[kvert];
                kvert = ( jsa + 2 ) % 4 + 1;
                xx[1] = x[kvert];
                yy[1] = y[kvert];
                if ( *mode > -999 && *mode != 0 )
                    {
                    /*          usrplt_(xx, yy, &c__2, &kcl, &c__1); */
                    lmode= 1;
                    if ( *mode== -1 ) lmode= -1;
                    ( *usrpsc_ ) ( xx, yy, &c__2, &icol[kcl], &lmode,
                                   *lcanno,
                                   jpolr, npolr, &iride, jfrst, jtost );

                    }
                goto L850;
L680:

                /*           LINE PASSES THROUGH DATA POINT */
                /*                       (AT END OF SIDE) */

                /*           INHIBIT MULTIPLE ZERO AT VERTEX */
                if ( jn == 0 )
                    {
                    goto L685;
                    }
                if ( kcl != frbcrd_1.nclzr[jn + ( frbcof_1.kse << 8 ) - 257] )
                    {
                    goto L685;
                    }
                if ( ( r__1 = t1[ni + 1 + ( frbcof_1.kse << 2 ) - 5] -
                              frbcrd_1.tzr[jn + ( frbcof_1.kse << 8 ) - 257], abs (
                            r__1 ) ) <= frbcrd_1.poserr * ( float ) 3. )
                    {
                    goto L700;
                    }
L685:

                /*           COMPUTE VALUE ON SIDE JSA */
                frbcof_1.kk = 1;
                /* Computing MIN */
                r__2 = frbcrd_1.sl[frbcof_1.kse - 1] * ( float ).01, r__3 = (
                            r__1 = t1[ni + 1 + ( frbcof_1.kse << 2 ) - 5] - t1[ni +
                                    ( frbcof_1.kse << 2 ) - 5], abs ( r__1 ) ) * ( float ).1;
                eps = min ( r__2,r__3 );
                ta = t1[ni + 1 + ( frbcof_1.kse << 2 ) - 5] - eps *
                     frbcoc_1.sigs[frbcof_1.kse - 1];
                fa = frbeva_ ( &ta );

                /*           COMPUTE VALUE ON NEXT SIDE */
                nse = frbcof_1.kse % 4 + 1;
                frbcof_1.kse = nse;
                /* Computing MIN */
                r__2 = frbcrd_1.sl[nse - 1] * ( float ).01, r__3 = ( r__1 = t1[ (
                            nse << 2 ) - 3] - t1[ ( nse << 2 ) - 4], abs ( r__1 ) ) * (
                            float ).1;
                epsn = min ( r__2,r__3 );
                tb = t1[ ( nse << 2 ) - 4] + epsn * frbcoc_1.sigs[nse - 1];
                fb = frbeva_ ( &tb );

                frbcof_1.kse = jsa;
                if ( fa * fb > ( float ) 0. )
                    {
                    goto L700;
                    }
                if ( fa == ( float ) 0. && fb == ( float ) 0. )
                    {
                    goto L700;
                    }
                /*           IF () THEN CONTOUR LINE IS DEGENERATED TO A POINT
                 */

                /*           CONTOUR LINE STARTS FROM VERTEX INTO RECTANGLE */

                ++jn;
                if ( jn > frbcoc_1.ncmaxs )
                    {
                    goto L6000;
                    }
                frbcrd_1.tzr[jn + ( frbcof_1.kse << 8 ) - 257] = t1[ni + 1 + (
                            frbcof_1.kse << 2 ) - 5];
                frbcrd_1.nclzr[jn + ( frbcof_1.kse << 8 ) - 257] = kcl;
                frbcrd_1.sder[jn + ( frbcof_1.kse << 8 ) - 257] = -fa / eps;
                goto L700;

                /*           COMPUTE ZERO ON SIDE (STATION) */
L690:
                ++jn;
                if ( jn > frbcoc_1.ncmaxs )
                    {
                    goto L6000;
                    }
                frbcof_1.kk = 1;
                frbcrd_1.tzr[jn + ( frbcof_1.kse << 8 ) - 257] = frbzer_ ( &t1[
                            jin + ( frbcof_1.kse << 2 ) - 5], &t1[jin + 1 + (
                                        frbcof_1.kse << 2 ) - 5], &f1, &f2, &frbcrd_1.poserr,
                        frbeva_ );

                /*           COMPUTE DERIVATIVE AT ZERO */
                frbcof_1.kk = 4;
                frbcrd_1.sder[jn + ( frbcof_1.kse << 8 ) - 257] = frbeva_ ( &
                        frbcrd_1.tzr[jn + ( frbcof_1.kse << 8 ) - 257] ) *
                        frbcoc_1.sigs[frbcof_1.kse - 1];
                /*           STORE INDEX OF CONTOUR LEVEL */
                frbcrd_1.nclzr[jn + ( frbcof_1.kse << 8 ) - 257] = kcl;

                /*           CHECK SIGN OF DER., IF SAME LEVEL */
                if ( jn < 2 )
                    {
                    goto L700;
                    }
                if ( kcl != frbcrd_1.nclzr[jn - 1 + ( frbcof_1.kse << 8 ) - 257] )
                    {
                    goto L700;
                    }
                if ( ( r__1 = frbcrd_1.tzr[jn + ( frbcof_1.kse << 8 ) - 257] -
                              frbcrd_1.tzr[jn - 1 + ( frbcof_1.kse << 8 ) - 257], abs (
                            r__1 ) ) > frbcrd_1.poserr * ( float ) 3. )
                    {
                    goto L700;
                    }
                if ( frbcrd_1.sder[jn + ( frbcof_1.kse << 8 ) - 257] *
                        frbcrd_1.sder[jn - 1 + ( frbcof_1.kse << 8 ) - 257] < (
                            float ) 0. )
                    {
                    goto L700;
                    }

                /*           IF SIGN IS WRONG OR =0, COMPUTE DER. BY DIFFERENC
                ES */
                frbcof_1.kk = 1;
                sl01 = frbcrd_1.sl[frbcof_1.kse - 1] * ( float ).01;
                /* Computing MIN */
                r__2 = sl01, r__3 = ( r__1 = frbcrd_1.tzr[jn - 1 + (
                                                 frbcof_1.kse << 8 ) - 257] - t1[jin - 1 + (
                                                         frbcof_1.kse << 2 ) - 5], abs ( r__1 ) ) * ( float ).1;
                eps = min ( r__2,r__3 );
                ta = frbcrd_1.tzr[jn - 1 + ( frbcof_1.kse << 8 ) - 257] - eps *
                     frbcoc_1.sigs[frbcof_1.kse - 1];
                frbcrd_1.sder[jn - 1 + ( frbcof_1.kse << 8 ) - 257] = -frbeva_ ( &
                        ta ) / eps;
                /* Computing MIN */
                r__2 = sl01, r__3 = ( r__1 = frbcrd_1.tzr[jn - 1 + (
                                                 frbcof_1.kse << 8 ) - 257] - t1[jin + 1 + (
                                                         frbcof_1.kse << 2 ) - 5], abs ( r__1 ) ) * ( float ).1;
                eps = min ( r__2,r__3 );
                tb = frbcrd_1.tzr[jn + ( frbcof_1.kse << 8 ) - 257] + eps *
                     frbcoc_1.sigs[frbcof_1.kse - 1];
                frbcrd_1.sder[jn + ( frbcof_1.kse << 8 ) - 257] = frbeva_ ( &tb ) /
                        eps;

L700:
                ;
                }
            /* L800: */
            }
L850:
        frbcrd_1.nz[frbcof_1.kse - 1] = jn;
        /*                = NUMBER OF ZEROS ON SIDE KSE */
        /* L900: */
        }

    /*     EVERY RIDE SHOULD HAVE START AND END */
    if ( frbcrd_1.nz[0] + frbcrd_1.nz[1] + frbcrd_1.nz[2] + frbcrd_1.nz[3] < 2 )
        {
        goto L250;
        }


    /*     CLEAR FILL AREA BUFFER */
    frbfcl_ ( &icol[1] );

    /*     COMPUTE X0,Y0 FOR EACH ZERO (RELATIVE TO X(3),Y(3)), */
    /*     SET STATUS OF ALL ZEROS TO 0 */
    i__1 = nside;
    for ( jsa = 1; jsa <= i__1; ++jsa )
        {
        jn = frbcrd_1.nz[jsa - 1];
        if ( jn == 0 )
            {
            goto L1000;
            }
        np1 = ( jsa + 1 ) % 4 + 1;
        i__2 = jn;
        for ( jza = 1; jza <= i__2; ++jza )
            {
            frbcrd_1.istatz[jza + ( jsa << 8 ) - 257] = 0;
            t = frbcrd_1.tzr[jza + ( jsa << 8 ) - 257];
            frbcrd_1.x0[jza + ( jsa << 8 ) - 257] = x[np1] - frbcrd_1.x3 +
                                                    frbcrd_1.sa[jsa - 1] * frbcrd_1.co[jsa - 1] + t * ( r__1 =
                                                            frbcrd_1.co[jsa - 1], abs ( r__1 ) );
            frbcrd_1.y0[jza + ( jsa << 8 ) - 257] = y[np1] - frbcrd_1.y3 +
                                                    frbcrd_1.sa[jsa - 1] * frbcrd_1.si[jsa - 1] + t * ( r__1 =
                                                            frbcrd_1.si[jsa - 1], abs ( r__1 ) );
            /* L980: */
            }
L1000:
        ;
        }
    /* :    WRITE(*,*)((TZR(J,JJ),J=1,NZ(JJ)),JJ=1,4) */
    /* :    WRITE(*,*)((SDER(J,JJ),J=1,NZ(JJ)),JJ=1,4) */
    /* :    WRITE(*,*)(NZ(J),J=1,4) */

    /*      COMPUTE COEFFICIENTS FOR REPRESENTATION INSIDE RECTANGLE */
    zx3b3 = ( zx[2] - zx[3] ) * sl1[3];
    zx4b3 = ( zx[1] - zx[4] ) * sl1[3];
    zy3a3 = ( zy[4] - zy[3] ) * sl1[0];
    zy4a3 = ( zy[1] - zy[2] ) * sl1[0];
    a = ( zz[0] - zz[3] - zz[1] + zz[2] ) * sl1[3] * sl1[0] - zx3b3 - zy3a3 +
        zxy[3];
    b = zx4b3 - zx3b3 - zxy[4] + zxy[3];
    c = zy4a3 - zy3a3 - zxy[2] + zxy[3];
    d = zxy[1] - zxy[4] - zxy[2] + zxy[3];
    e = a + a - b - c;
    frbcop_1.p11 = zxy[3];
    frbcop_1.p12 = ( ( zx3b3 - zxy[3] ) * ( float ) 2. + zx3b3 - zxy[2] ) * sl1[3];
    frbcop_1.p13 = ( zx3b3 * ( float )-2. + zxy[2] + zxy[3] ) * sl12[3];
    frbcop_1.p21 = ( ( zy3a3 - zxy[3] ) * ( float ) 2. + zy3a3 - zxy[4] ) * sl1[0];
    frbcop_1.p22 = ( ( a + e ) * ( float ) 3. + d ) * sl1[0] * sl1[3];
    frbcop_1.p23 = ( e * ( float )-3. - b - d ) * sl1[0] * sl12[3];
    frbcop_1.p31 = ( zy3a3 * ( float )-2. + zxy[4] + zxy[3] ) * sl12[0];
    frbcop_1.p32 = ( e * ( float )-3. - c - d ) * sl1[3] * sl12[0];
    frbcop_1.p33 = ( d + e + e ) * sl12[0] * sl12[3];

    /*     INITIALIZE STACK */
    for ( is = 1; is <= 4; ++is )
        {
        jfrst[is - 1] = 0;
        jsest[is - 1] = 0;
        jzest[is - 1] = 0;
        jsast[is - 1] = 0;
        jzast[is - 1] = 0;
        npst[is - 1] = 0;
        derns[is - 1] = ( float ) 0.;
        /* L1100: */
        }
    jpolr[0] = 1;


    /*     START 'TRIPS' USING THE ZEROS ON THE SIDES AS 'STATIONS' */

    /*     SET PARAMETERS FOR FIRST JOURNEY */
    /*     START AT UNUSED STATIONS (ISTATZ =0) */
    istart = 0;
    frbcrd_1.ndir3 = 1;
    /*     NDIR3= 1 MEANS COUNTER-CLOCKWISE TRIP */
    ndirv = 2;
    ndirs = 0;

    /*     LOOP OVER JOURNEYS */
    njour = 3;
    if ( abs ( *mode ) == 1 )
        {
        njour = 1;
        }
    i__1 = njour;
    for ( journy = 1; journy <= i__1; ++journy )
        {

        /*       SET PARAMETERS FOR JOURNY=3 */
        if ( journy != 3 )
            {
            goto L1210;
            }
        frbcrd_1.ndir3 = -1;
        ndirv = 1;
        ndirs = 2;
        istart = 1;
L1210:
        /* :      WRITE (*,*) JOURNY*11111 */

        /*       LOOP OVER SIDES, JSA= SIDE INDEX */
        for ( jsa = 1; jsa <= 4; ++jsa )
            {
            jzn = frbcrd_1.nz[jsa - 1];
            if ( jzn == 0 )
                {
                goto L2010;
                }

            /*         LOOP OVER ZEROS, JZA= STARTING ZERO */
            i__2 = jzn;
            for ( jza = 1; jza <= i__2; ++jza )
                {

                /*           FOR THIRD JOURNEY, START ALSO AT ISTATZ.LE.2 */
                if ( journy == 3 && frbcrd_1.istatz[jza + ( jsa << 8 ) - 257] <=
                        2 )
                    {
                    goto L1220;
                    }

                if ( frbcrd_1.istatz[jza + ( jsa << 8 ) - 257] != istart )
                    {
                    goto L2000;
                    }
                /*           IF ()  THEN THIS STATION WILL NOT SERVE AS START
                */
L1220:


                /*           START TRIP FROM STATION  SIDE JSA, ZERO JZA */

                /*           IF JOURNY.NE.1 CHECK STACK FIRST */
                if ( journy == 1 )
                    {
                    goto L1340;
                    }
                for ( is = 1; is <= 4; ++is )
                    {
                    if ( npst[is - 1] != 0 && jsa == jsest[is - 1] && jza ==
                            jzest[is - 1] )
                        {
                        goto L1310;
                        }
                    /* L1300: */
                    }
                goto L1340;
                /*           COORDINATES FOR NEXT RIDE ARE IN STACK */
L1310:
                jfrst[0] = is;
                jj = npst[is - 1];
                i__3 = npst[is - 1];
                for ( j = 1; j <= i__3; ++j )
                    {
                    xpol[j - 1] = xstack[jj + is * 100 - 101];
                    ypol[j - 1] = ystack[jj + is * 100 - 101];
                    --jj;
                    /* L1330: */
                    }
                npol1 = npst[is - 1];
                jsa2 = jsast[is - 1];
                jza2 = jzast[is - 1];
                nstop = 0;
                /*           SET NORMAL DERIV. AND COLOR */
                frbcrd_1.dernor = -derns[is - 1];
                derno1 = frbcrd_1.dernor;
                ncl1 = frbcrd_1.nclzr[jza + ( jsa << 8 ) - 257];
                /* Computing MIN */
                r__1 = -frbcrd_1.dernor * frbcrd_1.ndir3 + ( float ) 1.;
                ncol = min ( r__1, ( float ) 1. ) + ncl1;
                goto L1350;

L1340:

                /*           SET NORMAL DERIVATIVE AND COLOUR FOR FIRST RIDE
                */
                if ( frbcrd_1.sder[jza + ( jsa << 8 ) - 257] == ( float ) 0. )
                    {
                    goto L2000;
                    }
                frbcrd_1.dernor = r_sign ( &c_b5, &frbcrd_1.sder[jza + ( jsa <<
                                           8 ) - 257] );
                derno1 = frbcrd_1.dernor;
                ncl1 = frbcrd_1.nclzr[jza + ( jsa << 8 ) - 257];
                /* Computing MIN */
                r__1 = -frbcrd_1.dernor * frbcrd_1.ndir3 + ( float ) 1.;
                ncol = min ( r__1, ( float ) 1. ) + ncl1;
                /*           NCOL= COLOUR FOR TRIP */
                /*           THE FIRST LINE OF THE LAST STATEMENT IS 1 OR 0,
                */
                /*           DEPENDING ON THE SIGN OF SDER */
                /*           (DERIVATIVE IN DIRECTION OF SIDE) */
                /*           AND NDIR3 */
                /*           NDIR3= 1, FOR JOURNY=1,3 (COUNTER-CLOCKWISE TRIP)
                 */
                /*           NDIR3= -1, FOR JOURNY=2  (CLOCKWISE TRIP) */

                jfrst[0] = 0;
                frbrid_ ( frbeva_,&jsa, &jza, cc, nc, xpol, ypol,
                          &frbcoc_1.maxpol, &jsa2, &jza2, &npol1, &nstop );
                /*           FIRST RIDE ENDED ON STATION SIDE JSA2, ZERO JZA2
                */

                /*           CALL FOR LINE DRAWING */
                if ( abs ( *mode ) != 1 )
                    {
                    goto L1350;
                    }
                /*      usrplt_(xpol, ypol, &npol1, &ncl1, &c__1); */
                lmode= 1;
                if ( *mode== -1 ) lmode= -1;
                ( *usrpsc_ ) ( xpol, ypol, &npol1, &icol[ncl1], &lmode,
                               *lcanno* ( int ) frbcrd_1.dernor,
                               jpolr, npolr, &iride, jfrst, jtost );


                /*           BOOK KEEPING FOR START AND END OF RIDE */
L1350:
                /* :    WRITE (*,*) ' FIRST',JSA,JZA,JSA2,JZA2,NPOL1,NSTOP,JFR
                ST(1) */
                jpol = npol1;
                jpoll1 = 1;
                np = npol1;
                iride = 1;
                jtost[iride - 1] = 0;
                jser[0] = jsa2;
                jzer[0] = jza2;
                jsar[0] = jsa;
                jzar[0] = jza;
                jstopr[0] = nstop;
                npolr[0] = npol1;
                dernr[0] = frbcrd_1.dernor;
                jsides = 0;
                if ( nstop == 2 )
                    {
                    goto L2000;
                    }

                /*           FIND NEXT STATION FOR CONTINUATION OF TRIP (=TRAN
                SFER) */

L1360:
                jza2 += frbcrd_1.ndir3;
                if ( jza2 <= frbcrd_1.nz[jsa2 - 1] && jza2 > 0 )
                    {
                    goto L1400;
                    }

                /*           TAKE NEXT SIDE, ADD VERTEX TO POLYGON */
                /*           SET JZA2 TO FIRST OR LAST ZERO OF THE NEW SIDE */

L1370:
                ++jsides;
                if ( jsides == 5 )
                    {
                    goto L2000;
                    }
                ++jpol;
                kvert = ( jsa2 + ndirv ) % 4 + 1;
                xpol[jpol - 1] = x[kvert];
                ypol[jpol - 1] = y[kvert];
                jsa2 = ( jsa2 + ndirs ) % 4 + 1;
                if ( frbcrd_1.nz[jsa2 - 1] == 0 )
                    {
                    goto L1370;
                    }
                jza2 = 1;
                if ( frbcrd_1.ndir3 == -1 )
                    {
                    jza2 = frbcrd_1.nz[jsa2 - 1];
                    }

                /*           CHECK FOR REGULAR END OF TRIP */
L1400:
                if ( jsa2 == jsa && jza2 == jza )
                    {
                    goto L1900;
                    }

                /*           CHECK DIFFERENCE OF CONTOUR LEVELS */
                /*           BETWEEN JSA,JZA AND JSA2,JZA2, */
                /*           CHECK SIGN OF DERIVATIVE OF STATION JZA2,JSA2, */

                /*           SET NEW DERNOR */

                ncdif = frbcrd_1.nclzr[jza2 + ( jsa2 << 8 ) - 257] - ncl1;
                ncdifa = abs ( ncdif );
                nc1 = -ncdifa;
                if ( nc1 == 0 )
                    {
                    nc1 = 1;
                    }
                frbcrd_1.dernor = derno1 * nc1;
                sdchek = frbcrd_1.sder[jza2 + ( jsa2 << 8 ) - 257] *
                         frbcrd_1.dernor;
                if ( sdchek >= ( float ) 0. && ncdifa <= 1 )
                    {
                    goto L1420;
                    }

                /*           DO NOT STOP AT VERTEX */
                if ( frbcrd_1.tzr[jza2 + ( jsa2 << 8 ) - 257] < frbcrd_1.poserr *
                        ( float ) 3. || frbcrd_1.tzr[jza2 + ( jsa2 << 8 ) - 257]
                        > frbcrd_1.sl[jsa2 - 1] - frbcrd_1.poserr * ( float ) 3. )
                    {
                    goto L1360;
                    }

                /*           STOP TRIP IN ALL OTHER CASES */
                /* :          WRITE (*,*) 'STOP AT',JSA2,JZA2 */
                goto L2000;

L1420:

                /*           START NEW RIDE FROM SIDE JSA2, ZERO JZA2 */

                /*           CHECK STACK FIRST */
                for ( is = 1; is <= 4; ++is )
                    {
                    if ( npst[is - 1] != 0 && jsa2 == jsest[is - 1] && jza2 ==
                            jzest[is - 1] )
                        {
                        goto L1440;
                        }
                    /* L1430: */
                    }
                nfrst = 0;
                goto L1500;
L1440:
                nfrst = is;
                /*           COORDINATES FOR NEXT RIDE ARE IN STACK */
                /*           THE FOLLOWING REPLACES A CALL TO FRBRID */
                if ( jpol + npst[is - 1] > frbcoc_1.maxpol )
                    {
                    goto L7000;
                    }
                jj = npst[is - 1];
                i__3 = npst[is - 1];
                for ( j = 1; j <= i__3; ++j )
                    {
                    xpol[jpol + j - 1] = xstack[jj + is * 100 - 101];
                    ypol[jpol + j - 1] = ystack[jj + is * 100 - 101];
                    --jj;
                    /* L1450: */
                    }
                jse = jsast[is - 1];
                jze = jzast[is - 1];
                np = npst[is - 1];
                nstop = 0;
                goto L1600;

L1500:
                i__3 = frbcoc_1.maxpol - jpol;
                frbrid_ ( frbeva_,&jsa2, &jza2, cc, nc,
                          &xpol[jpol], &ypol[jpol], &i__3,
                          &jse, &jze, &np, &nstop );
                /*           RIDE ENDED AT STATION  SIDE JSE, ZERO JZE */

                /*           CALL FOR LINE DRAWING */
                if ( abs ( *mode ) != 1 || frbcrd_1.istatz[jza2 + ( jsa2 << 8 ) - 257] !=
                        0 || np < 2 )
                    {
                    goto L1595;
                    }
                /*      usrplt_(&xpol[jpol], &ypol[jpol], &np, &frbcrd_1.nclzr[jza2 +
                            (jsa2 << 8) - 257], &c__1); */
                lmode= 1;
                if ( *mode== -1 ) lmode= -1;
                ( *usrpsc_ ) ( &xpol[jpol], &ypol[jpol], &np,
                               &icol[frbcrd_1.nclzr[jza2 + ( jsa2 << 8 ) - 257]], &lmode,
                               *lcanno* ( int ) frbcrd_1.dernor,
                               jpolr, npolr, &iride, jfrst, jtost );

L1595:

                /*           BOOK KEEPING FOR CONTINUATION RIDE */
L1600:
                /* :          WRITE(*,*) JSA2,JZA2,JSE,JZE,NP,NSTOP,NFRST */
                if ( nstop == 2 )
                    {
                    np = 0;
                    }
                jpoll1 = jpol + 1;
                jpol += np;
                ++iride;
                jtost[iride - 1] = 0;
                jsar[iride - 1] = jsa2;
                jzar[iride - 1] = jza2;
                jser[iride - 1] = jse;
                jzer[iride - 1] = jze;
                jstopr[iride - 1] = nstop;
                npolr[iride - 1] = np;
                jpolr[iride - 1] = jpoll1;
                dernr[iride - 1] = frbcrd_1.dernor;
                jfrst[iride - 1] = nfrst;
                jsa2 = jse;
                jza2 = jze;
                if ( iride + 1 > frbcoc_1.maxrid )
                    {
                    goto L2000;
                    }
                /*           CONTINUE TRIP */
                goto L1360;

                /*           POLYGON FOR FILL AREA IS COMPLETE */
                /*           (SUCCESSFULL ROUND TRIP) */

L1900:

                /*           STACK ADMINISTRATION */

                if ( journy > 1 )
                    {
                    goto L1960;
                    }
                /*           IF RIDE WAS TAKEN FROM STACK, EMPTY STACK */
                i__3 = iride;
                for ( ir = 1; ir <= i__3; ++ir )
                    {
                    jfstir = jfrst[ir - 1];
                    if ( jfstir == 0 )
                        {
                        goto L1905;
                        }
                    npst[jfstir - 1] = 0;
L1905:
                    ;
                    }

                /*           FILL STACK */
                i__3 = iride;
                for ( ir = 1; ir <= i__3; ++ir )
                    {
                    if ( jfrst[ir - 1] != 0 )
                        {
                        goto L1950;
                        }
                    if ( npolr[ir - 1] > frbcoc_1.maxsta )
                        {
                        goto L1950;
                        }
                    if ( jstopr[ir - 1] == 1 )
                        {
                        goto L1950;
                        }
                    jj = jpolr[ir - 1];
                    /*             FIND FREE STACK */
                    is = ir;
                    if ( is > 4 )
                        {
                        is = 1;
                        }
                    for ( js = 1; js <= 4; ++js )
                        {
                        if ( npst[is - 1] == 0 )
                            {
                            goto L1930;
                            }
                        is = is % 4 + 1;
                        /* L1910: */
                        }
                    /*             FILL STACK IS */
L1930:
                    i__4 = npolr[ir - 1];
                    for ( j = 1; j <= i__4; ++j )
                        {
                        xstack[j + is * 100 - 101] = xpol[jj - 1];
                        ystack[j + is * 100 - 101] = ypol[jj - 1];
                        ++jj;
                        /* L1940: */
                        }
                    jtost[ir - 1] = is;
                    jsast[is - 1] = jsar[ir - 1];
                    jzast[is - 1] = jzar[ir - 1];
                    jsest[is - 1] = jser[ir - 1];
                    jzest[is - 1] = jzer[ir - 1];
                    derns[is - 1] = dernr[ir - 1];
                    npst[is - 1] = npolr[ir - 1];
L1950:
                    ;
                    }

                /*           CALL FOR FILL AREA */
L1960:
                if ( jpol < 3 )
                    {
                    goto L2000;
                    }
                ++nfar;
                if ( *mode % 2 == 0 )
                    {
                    /*          usrplt_(xpol, ypol, &jpol, &icol[ncol], &c__0); */
                    ( *usrpsc_ ) ( xpol, ypol, &jpol, &icol[ncol], &c__0,
                                   0,   /* only fill operation */
                                   jpolr, npolr, &iride, jfrst, jtost );

                    }

                /*           SET FLAGS FOR START AND END OF RIDES */
                i__3 = iride;
                for ( jr = 1; jr <= i__3; ++jr )
                    {
                    js = jsar[jr - 1];
                    jz = jzar[jr - 1];
                    if ( frbcrd_1.istatz[jz + ( js << 8 ) - 257] % 2 == 0 )
                        {
                        ++frbcrd_1.istatz[jz + ( js << 8 ) - 257];
                        }
                    if ( jstopr[jr - 1] > 0 )
                        {
                        goto L1990;
                        }
                    js = jser[jr - 1];
                    jz = jzer[jr - 1];
                    frbcrd_1.istatz[jz + ( js << 8 ) - 257] += 2;
L1990:
                    ;
                    }

                /*         DRAW LINE IF MODE=2 */
                js = jsar[iride - 1];
                jz = jzar[iride - 1];
                if ( *mode == 2 && np > 1 )
                    {
                    /*          usrplt_(&xpol[jpoll1 - 1], &ypol[jpoll1 - 1], &np, &
                                    frbcrd_1.nclzr[jz + (js << 8) - 257], &c__1); */
                    lmode= 1;
                    if ( *mode== -1 ) lmode= -1;
                    ( *usrpsc_ ) ( &xpol[jpoll1 - 1], &ypol[jpoll1 - 1], &np,
                                   &icol[frbcrd_1.nclzr[jz + ( js << 8 ) - 257]], &lmode,
                                   *lcanno* ( int ) frbcrd_1.dernor,
                                   jpolr, npolr, &iride, jfrst, jtost );

                    }

L2000:
                ;
                }
            /*         END OF LOOP OVER ZEROS */
L2010:
            /* L3000: */
            ;
            }
        /*       END OF LOOP OVER SIDES */

        /*       SET ISTART FOR JOURNY=2 */
        istart = 2;

        /* L4000: */
        }
    frbcoc_1.npp += frbcrd_1.nprec;

    /*     TREAT CASE WHEN WHOLE RECTANGLE HAS TO BE FILLED */
    /*     BECAUSE THERE WAS NO SUCCESSFULL TRIP */
    if ( nfar != 0 || abs ( *mode ) == 1 )
        {
        goto L4050;
        }
    ncol = icn + 1;
    if ( cc[icn - 1] == zmaxt )
        {
        ncol = icn;
        }
    goto L250;
L4050:

L5000:
    return 0;

    /*     ERROR EXIT */
L6000:
    fprintf ( stderr,fmt_6001,frbcoc_1.ncmaxs );
    return 0;

L7000:
    fprintf ( stderr,fmt_7001,frbcoc_1.maxpol,it );
    return 0;
    } /* farbrc_ */



/* Subroutine */ int frbrid_ ( frbeva_,jsa, jza, cn, nc, xpol, ypol, maxpol,
                               jsa2, jza2,
                               np, nstop )
real ( *frbeva_ ) ();
integer *jsa, *jza;
real *cn;
integer *nc;
real *xpol, *ypol;
integer *maxpol, *jsa2, *jza2, *np, *nstop;
    {
    /* Format strings */
    static char fmt_9991[] = "FRBRID: Warning: OVERFLOW OF WORKING\
 STORAGE XPOL,YPOL\n";

    /* System generated locals */
    integer i__1;
    real r__1, r__2;

    /* Builtin functions */
    double atan(), cos(), sin(), sqrt(), r_sign(), atan2();
    integer s_wsfe(), e_wsfe();

    /* Local variables */
    static real racc, sacc;
    static integer nsad, jese;
    static real dxds, dyds;
    static integer nost;
    static real ttzr, dxds1, dyds1, soll1;
    static integer j, n;
    static real r, r30min, s30max, f1, f2;
    static integer jj;
    static real ds;
    static integer jp;
    static real r30;
    static integer jn;
    static real sq;
    /*    extern real frbeva_(); */
    static real thetac, thetsc;
    extern real frbzer_();
    static real pl0, pl1, pl2, xx1, xx2, xx3, yy2, yy3, yy1, xx4, yy4;
    static integer kcl;
    static real ds12, ds23, ds13, dx12, dy12;
    static integer nse;
    static real eps;
    static integer ncp, jpp;
    static real ds34;
    static integer jse;
    static real dx30, dy30, ds30, ugr, ogr;


    /*     TRACE CONTOUR FROM SIDE JSA TO SIDE JSA2 */
    /*     (RIDE FROM JSA,JZA TO JSA2,JZA2) */

    /*     T R I P   ALGORITHM   A.PREUSSER   FARB-E-2D  VERSION 3.0 01/1991
    */


    /*     AUTHOR: A. PREUSSER */
    /*             FRITZ-HABER-INSTITUT DER MPG */
    /*             FARADAYWEG 4-6 */
    /*             D-1000 BERLIN 33 */


    /*     INPUT PARAMETERS */
    /*     JSA         SIDE INDEX  CONTOUR STARTS FROM */
    /*     JZA         ZERO INDEX  CONTOUR STARTS FROM */
    /*     CN          CONTOUR LEVELS */
    /*     NC          NUMBER OF CONTOUR LEVELS */
    /*     MAXPOL      MAXIMUM NUMBER OF POINTS IN XPOL,YPOL */

    /*     OUTPUT PARAMETERS */
    /*     XPOL,YPOL    X-Y-COORDINATES OF THE POINTS OF A RIDE */
    /*     JSA2         SIDE INDEX WHERE CONTOUR ENDS */
    /*     JZA2         ZERO (STATION) INDEX WHERE CONTOUR ENDS */
    /*     NP           NUMBER OF POINTS STORED TO XPOL,YPOL */
    /*     NSTOP        =0, RIDE ENDED AT STATION */
    /*                  =1, RIDE ENDED ON SIDE, NO STATION FOUND */
    /*                  =2, RIDE ENDED INSIDE RECTANGLE */

    /*                  IF NSTOP.EQ.1, JSA2,JZA2 INDICATE THE PREVIOUS */
    /*                  STATION ON THE ENDING SIDE. */
    /*                  (NOTE POSITIVE OR NEGATIVE SENSE OF TRIP */
    /*                  INDICATED BY NDIR3). JZA2 MAY BE ZERO. */

    /*                  IF NSTOP.EQ.2, JSA2= JSA;    JZA2= JZA  . */








    /* Parameter adjustments */
    --ypol;
    --xpol;
    --cn;

    /* Function Body */
    frbcof_1.kse = *jsa;
    ++frbcrd_1.kride;
    nsad = 0;
    *nstop = 0;


    /*     INITIALIZATION, IF FIRST RIDE OF A RECTANGLE */

    if ( frbcrd_1.kride != 1 )
        {
        goto L1400;
        }
    /* Computing MIN */
    r__1 = ( float ).01 / frbcoc_2.cmscal, r__2 = frbcrd_1.hmin * ( float ).01;
    frbcor_1.rmax = min ( r__1,r__2 );
    /*           = DISTANCE NORMAL TO CURVE DIRECTION WITHIN WHICH */
    /*             A ZERO MUST BE FOUND */
    frbcor_1.dsmax = frbcrd_1.hmin * ( float ).2;
    /*           = MAXIMUM STEP SIZE */
    /* Computing MIN */
    r__1 = frbcor_1.rmax * ( float ).03, r__2 = frbcrd_1.poserr * ( float ) 8.;
    frbcor_1.dsmin = min ( r__1,r__2 );
    /*           = MINIMUM STEP SIZE */
    /* Computing MIN */
    r__1 = frbcor_1.rmax * ( float ) 8.;
    frbcor_1.fstep = min ( r__1,frbcor_1.dsmax );
    /*           = STARTING STEP SIZE */
    frbcor_1.racmin = frbcoc_2.sacmin * ( float ).1;
    /*             A POINT IS STORED ONLY IF THE ACCUMULATED R'S (RACC) */
    /*             (CHANGE IN DIRECTION) HAVE REACHED RACMIN */
    /*     SET DIRECTION OF SIDES */
    frbcor_1.thetas[0] = ( float ) 0.;
    frbcor_1.thetas[1] = frbcoc_2.pi * ( float ).5;
    frbcor_1.thetas[2] = frbcoc_2.pi;
    frbcor_1.thetas[3] = frbcoc_2.pi * ( float ) 1.5;
L1400:


    /*     DEFINE CONTOUR LEVEL */
    kcl = frbcrd_1.nclzr[*jza + ( frbcof_1.kse << 8 ) - 257];
    frbcof_1.cl = cn[kcl];

    /*     ESTIMATE STARTING DIRECTION */

    /*     COMPUTE F1 ON SIDE */
    nse = frbcof_1.kse % 4 + 1;
    eps = frbcrd_1.sl[nse - 1] * ( float ).01;
    f1 = frbcrd_1.sder[*jza + ( frbcof_1.kse << 8 ) - 257] * eps;

    /*     COMPUTE F2 NORMAL TO SIDE */
    frbcof_1.kk = 2;
    frbcof_1.xx4f = frbcrd_1.x0[*jza + ( frbcof_1.kse << 8 ) - 257];
    frbcof_1.yy4f = frbcrd_1.y0[*jza + ( frbcof_1.kse << 8 ) - 257];
    frbcof_1.sir = frbcrd_1.co[frbcof_1.kse - 1];
    frbcof_1.cor = -frbcrd_1.si[frbcof_1.kse - 1];
    f2 = ( *frbeva_ ) ( &eps );

    /*     COMPUTE ANGLE FOR STARTING DIRECTION */
    if ( f2 == ( float ) 0. )
        {
        goto L1470;
        }
    thetsc = atan ( -f1 / f2 );
    if ( thetsc < ( float ) 0. )
        {
        thetsc += frbcoc_2.pi;
        }
    if ( thetsc == ( float ) 0. )
        {
        thetsc = ( frbcrd_1.ndir3 - 1 ) * frbcoc_2.pi * ( float ).5;
        }
    goto L1480;
L1470:
    /*     IF (F1.EQ.0.) GOTO 1690 */
    thetsc = frbcoc_2.pi * ( float ).5;
L1480:
    thetac = frbcor_1.thetas[frbcof_1.kse - 1] + thetsc;

    /*     COMPUTE POINTS */

    /*     STORE FIRST POINT */
    jp = 1;
    /*     JP= NUMBER OF POINTS STORED FOR THIS CONTOUR LINE */
    xpol[jp] = frbcrd_1.x0[*jza + ( frbcof_1.kse << 8 ) - 257];
    ypol[jp] = frbcrd_1.y0[*jza + ( frbcof_1.kse << 8 ) - 257];

    /*     INITIALIZE TRACING */
L1490:
    r = ( float ) 0.;
    ds = frbcor_1.fstep;
    dx12 = ds * cos ( thetac );
    dy12 = ds * sin ( thetac );
    xx3 = frbcof_1.xx4f;
    yy3 = frbcof_1.yy4f;
    xx2 = xx3 - dx12;
    yy2 = yy3 - dy12;
    xx1 = xx2 - dx12;
    yy1 = yy2 - dy12;
    ds23 = ds;
    ds12 = ds;
    /*     DS01= DS */
    /*     POINTS P1,P2,P3,P4 WITH COORDINATES XX1...XX4, YY1...YY4 */
    /*     ARE REFERRED TO AS *QUEUE*. DS12...DS34 ARE THE DISTANCES */
    /*     BETWEEN POINTS IN THE QUEUE. */
    /*     XX4F,YY4F ARE PRELEMINARY COORDINATES FOR THE NEXT POINT P4 */
    /*     WHICH WILL BE COMPUTED BY THE REGULA FALSI (FRBZER). */
    /*     FOR A DERIVATION OF THE FORMULAS FOR PL0...PL2 SEE */
    /*     PREUSSER,A. COMPUTING AREA FILLING CONTOURS FOR SURFACES */
    /*                 DEFINED BY PIECEWISE POLYNOMIALS. */
    /*                 COMPUTER AIDED GEOMETRIC DESIGN 3, */
    /*                 (1986), P. 267-279 */
    /*     THERE IS ALSO AN EXPLANATION FOR THE FOLLOWING PART OF */
    /*     THE ALGORITHM. */
    sacc = ( float ) 0.;
    /*         = ACCUMULATED DISTANCES TO LAST POINT STORED */
    racc = frbcor_1.racmin;
    /*         = ACCUMULATED R */
    /*     A POINT IS ONLY STORED TO XPOL,YPOL IF SACC.GE.SACMIN */
    /*                                  AND       RACC.GE.RACMIN */
    ncp = 0;
    /*         = NUMBER OF POINTS COMPUTED */
    nost = 0;
    /*         = NUMBER OF STEPS NORMAL TO CURVE */
    frbcor_1.rma = frbcor_1.rmax;

    /*     COMPUTE NEW POINT FOR CONTOUR LINE */

    /*     COMPUTE CURVE DIRECTION */
L1500:
    ds13 = ds23 + ds12;
    pl0 = ds23 / ( ds12 * ds13 );
    pl1 = -ds13 / ( ds12 * ds23 );
    pl2 = ( ds13 + ds23 ) / ( ds13 * ds23 );
    dxds = pl0 * xx1 + pl1 * xx2 + pl2 * xx3;
    dyds = pl0 * yy1 + pl1 * yy2 + pl2 * yy3;
    sq = sqrt ( dxds * dxds + dyds * dyds );
    dxds /= sq;
    dyds /= sq;
    frbcof_1.cor = -dyds;
    frbcof_1.sir = dxds;

    /*     SEARCH FOR TWO POINTS WITH OPPOSITE SIGN */
    frbcor_1.rma = r_sign ( &frbcor_1.rmax, &frbcor_1.rma );
L1550:
    frbcof_1.xx4f = xx3 + dxds * ds;
    frbcof_1.yy4f = yy3 + dyds * ds;
    f1 = ( *frbeva_ ) ( &c_b8 );
    r__1 = frbcrd_1.dernor * f1;
    frbcor_1.rma = r_sign ( &frbcor_1.rma, &r__1 );
    f2 = ( *frbeva_ ) ( &frbcor_1.rma );
    if ( r_sign ( &c_b5, &f1 ) != r_sign ( &c_b5, &f2 ) )
        {
        goto L1600;
        }
    if ( f1 == ( float ) 0. || f2 == ( float ) 0. )
        {
        goto L1600;
        }

L1560:
    if ( ds * ( float ).5 < frbcor_1.dsmin )
        {
        goto L1570;
        }
    /*     DIVIDE STEPSIZE IN CURVE DIRECTION BY 2. */
    ds *= ( float ).5;
    goto L1550;

    /*     DIVIDE STEPSIZE NORMAL TO CURVE BY 2. */
L1570:
    ++nost;
    frbcor_1.rma *= ( float ).5;
    if ( abs ( frbcor_1.rma ) <= frbcrd_1.poserr )
        {
        goto L1580;
        }
    f2 = ( *frbeva_ ) ( &frbcor_1.rma );
    if ( r_sign ( &c_b5, &f1 ) == r_sign ( &c_b5, &f2 ) && f1 != ( float ) 0. && f2 != (
                float ) 0. )
        {
        goto L1570;
        }
    goto L1600;

    /*     SADDLE POINT */

    /*     SET NEW DIRECTION */
L1580:
    ++nsad;
    if ( nsad > 1 )
        {
        goto L1690;
        }
    dxds1 = dxds;
    dyds1 = dyds;
    thetac = atan2 ( dyds, dxds ) + frbcoc_2.pi * ( float ).5;

    /*     STORE SADDLE POINT */
    frbcof_1.xx4f = xx3;
    frbcof_1.yy4f = yy3;
    jpp = jp + 1;
    if ( jpp > *maxpol )
        {
        goto L4000;
        }
    jp = jpp;
    xpol[jp] = xx3;
    ypol[jp] = yy3;
    goto L1490;


    /*     FIND ZERO FOR NEW POINT */

L1600:
    r = frbzer_ ( &c_b8, &frbcor_1.rma, &f1, &f2, &frbcrd_1.poserr, frbeva_ );
    ++ncp;
    if ( ncp > frbcoc_2.ncpmax )
        {
        goto L1690;
        }
    ds34 = sqrt ( ds * ds + r * r );
    xx4 = frbcof_1.xx4f + frbcof_1.cor * r;
    yy4 = frbcof_1.yy4f + frbcof_1.sir * r;

    /*     CHECK IF POINT IS OUTSIDE THE RECTANGLE */
    *jsa2 = 1;
    if ( yy4 < ( float ) 0. )
        {
        goto L1700;
        }
    *jsa2 = 2;
    if ( xx4 > frbcrd_1.dx[0] )
        {
        goto L1700;
        }
    *jsa2 = 3;
    if ( yy4 > frbcrd_1.dy[1] )
        {
        goto L1700;
        }
    *jsa2 = 4;
    if ( xx4 < ( float ) 0. )
        {
        goto L1700;
        }

    /*     POINT IS INSIDE */

    /*     STORE POINT TO XPOL,YPOL */
    sacc += ds34;
    racc += abs ( r );
    if ( sacc < frbcoc_2.sacmin || racc < frbcor_1.racmin )
        {
        goto L1650;
        }
    jpp = jp + 1;
    if ( jpp > *maxpol )
        {
        goto L4000;
        }
    jp = jpp;
    xpol[jp] = xx4;
    ypol[jp] = yy4;
    sacc = ( float ) 0.;
    racc = ( float ) 0.;

    /*     UPDATE QUEUE */
L1650:
    /*     DS01= DS12 */
    ds12 = ds23;
    ds23 = ds34;
    xx1 = xx2;
    yy1 = yy2;
    xx2 = xx3;
    yy2 = yy3;
    xx3 = xx4;
    yy3 = yy4;

    /*     SET NEW STEP SIZE */
    soll1 = ( float ) 2.;
    if ( abs ( r ) > frbcrd_1.poserr )
        {
        soll1 = ( r__1 = frbcor_1.rma * ( float ).8 / r, abs ( r__1 ) );
        }
    if ( soll1 > ( float ) 1. )
        {
        /* Computing MIN */
        r__1 = frbcor_1.dsmax, r__2 = ds * sqrt ( soll1 );
        ds = min ( r__1,r__2 );
        }

    goto L1500;

    /*     TRACING STOPPED */
L1690:
    *nstop = 1;
    *jsa2 = 1;
    dxds = dxds1;
    dyds = dyds1;

    /*     POINT IS OUTSIDE */

    /*     SEARCH FOR CORRESPONDING ZERO ON SIDES */
    /*     START WITH SIDE JSA2 */
L1700:
    r30min = ( float ) 99999.;
    s30max = frbcor_1.dsmax;
    jse = *jsa2;
    for ( n = 1; n <= 2; ++n )
        {
        for ( jese = 1; jese <= 4; ++jese )
            {
            jj = frbcrd_1.nz[jse - 1];
            if ( jj == 0 )
                {
                goto L1760;
                }
            i__1 = jj;
            for ( j = 1; j <= i__1; ++j )
                {

                /*           LEVEL CHECK */
                if ( frbcrd_1.nclzr[j + ( jse << 8 ) - 257] != frbcrd_1.nclzr[*
                        jza + ( frbcof_1.kse << 8 ) - 257] )
                    {
                    goto L1750;
                    }

                /*           DERIVATIVE CHECK */
                if ( frbcrd_1.sder[j + ( jse << 8 ) - 257] * frbcrd_1.dernor > (
                            float ) 0. )
                    {
                    goto L1750;
                    }

                /*           R-CHECK */
                dx30 = frbcrd_1.x0[j + ( jse << 8 ) - 257] - xx3;
                dy30 = frbcrd_1.y0[j + ( jse << 8 ) - 257] - yy3;
                r30 = dy30 * dxds - dx30 * dyds;
                if ( r30 * r < ( float ) 0. && n == 1 )
                    {
                    goto L1750;
                    }
                r30 = abs ( r30 );
                if ( r30 >= r30min )
                    {
                    goto L1750;
                    }

                /*           S-CHECK */
                ds30 = dx30 * dxds + dy30 * dyds;
                if ( ds30 > s30max || ds30 < -ds23 )
                    {
                    goto L1750;
                    }

                *jza2 = j;
                *jsa2 = jse;
                r30min = r30;
L1750:
                ;
                }
L1760:
            jse = jse % 4 + 1;
            /* L1770: */
            }
        if ( r30min < frbcor_1.rmax )
            {
            goto L1800;
            }
        /*       FOR N=2, DO NOT CHECK SIGN OF R30 */
        /* L1780: */
        }

    /*     NO ACCEPTABLE ZERO ON ALL THREE SIDES */

    /*     REDUCE STEP SIZE */
    if ( *nstop != 1 )
        {
        goto L1560;
        }

    goto L1850;

    /*     STORE END STATION OF THE RIDE */
L1800:
    *nstop = 0;
L1850:
    if ( jp + 1 > *maxpol )
        {
        goto L4000;
        }
    if ( *nstop == 1 )
        {
        goto L3000;
        }
    ++jp;
    xpol[jp] = frbcrd_1.x0[*jza2 + ( *jsa2 << 8 ) - 257];
    ypol[jp] = frbcrd_1.y0[*jza2 + ( *jsa2 << 8 ) - 257];

    /*     ADD COORDINATES OF LOWER LEFT VERTEX */
    /*     NORMAL RETURN */
L2000:
    *np = jp;
    i__1 = *np;
    for ( jp = 1; jp <= i__1; ++jp )
        {
        xpol[jp] += frbcrd_1.x3;
        ypol[jp] += frbcrd_1.y3;
        /* L2050: */
        }
    frbcrd_1.nprec += *np;
    return 0;

    /*     ERROR HANDLING */
L3000:
    if ( jp <= 2 )
        {
        goto L3900;
        }

    /*     CHECK IF LAST POINT IS NEAR BOUNDARY */
    *jsa2 = 1;
    ttzr = xx3;
    if ( abs ( yy3 ) < frbcoc_2.sacmin )
        {
        goto L3300;
        }

    *jsa2 = 2;
    ttzr = yy3;
    if ( ( r__1 = xx3 - frbcrd_1.dx[0], abs ( r__1 ) ) < frbcoc_2.sacmin )
        {
        goto L3300;
        }

    *jsa2 = 3;
    ttzr = xx3;
    if ( ( r__1 = yy3 - frbcrd_1.dy[1], abs ( r__1 ) ) < frbcoc_2.sacmin )
        {
        goto L3300;
        }

    *jsa2 = 4;
    ttzr = yy3;
    if ( abs ( xx3 ) < frbcoc_2.sacmin )
        {
        goto L3300;
        }
    goto L3900;

    /*     TRACING STOPPED NEAR BOUNDARY */
    /*     NSTOP= 1, JSA2 SIDE NUMBER, */
    /*     JZA2 ZERO INDEX  OF LAST OR NEXT ZERO, OR =0 */
    /*     DEPENDING ON NDIR3. */

    /*     FIND JZA2 */
L3300:
    ugr = frbcrd_1.sa[*jsa2 - 1];
    *jza2 = 0;
    jn = frbcrd_1.nz[*jsa2 - 1];
    if ( jn == 0 )
        {
        goto L3500;
        }
    i__1 = jn;
    for ( jj = 1; jj <= i__1; ++jj )
        {
        ++ ( *jza2 );
        ogr = frbcrd_1.tzr[*jza2 + ( *jsa2 << 8 ) - 257];
        if ( ttzr >= ugr && ttzr <= ogr || ttzr >= ogr && ttzr <= ugr )
            {
            goto L3450;
            }
        ugr = ogr;
        /* L3400: */
        }
    ++ ( *jza2 );
L3450:
    if ( frbcrd_1.ndir3 == 1 )
        {
        -- ( *jza2 );
        }
L3500:
    goto L2000;

    /*     TRACING STOPPED WITHIN RECTANGLE */
L3900:
    *nstop = 2;
    *jsa2 = frbcof_1.kse;
    *jza2 = *jza;
    goto L2000;

L4000:
    fprintf ( stderr,fmt_9991 );
    goto L3900;
    } /* frbrid_ */

real frbeva_ ( t )
real *t;
    {
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real s0, s1, s2, s3, xx4, yy4;


    /*     T R I P   ALGORITHM   A.PREUSSER   FARB-E-2D  VERSION 3.0 01/1991
    */

    /*     FUNCTION EVALUATION */

    /*     AUTHOR      : A. PREUSSER */
    /*                   FRITZ-HABER-INSTITUT */
    /*                   DER MAX-PLANCK-GESELLSCHAFT */
    /*                   FARADAYWEG 4-6 */
    /*                   D-1000 BERLIN 33 */


    /*     VARIABELS IN /FRBCOF/ ARE USED AS ARGUMENTS */
    /*     FOR AN EXPLANATION SEE SUBROUTINE FARBRC */

    /*     KK      NUMBER OF FUNCTION TO BE EVALUATED */
    /*     KK=1    ORIGINAL POLYNOMIAL ALONG SIDE KSE */
    /*     KK=2    BIVARIATE POLYNOMIAL INSIDE RECTANGLE */
    /*     KK=3    2ND DERIVATIVE ALONG SIDE KSE */
    /*     KK=4    1ST DERIVATIVE ALONG SIDE KSE */



    if ( frbcof_1.kk == 2 )
        {
        goto L20;
        }
    if ( frbcof_1.kk == 4 )
        {
        goto L40;
        }
    if ( frbcof_1.kk == 3 )
        {
        goto L30;
        }
    ret_val = frbcop_1.p0[frbcof_1.kse - 1] + *t * ( frbcop_1.p1[frbcof_1.kse
              - 1] + *t * ( frbcop_1.p2[frbcof_1.kse - 1] + *t * frbcop_1.p3[
                                frbcof_1.kse - 1] ) ) - frbcof_1.cl;
    return ret_val;
L20:
    xx4 = frbcof_1.xx4f + frbcof_1.cor **t;
    yy4 = frbcof_1.yy4f + frbcof_1.sir **t;
    s0 = frbcop_1.p0[0] + yy4 * ( frbcop_1.p1[3] + yy4 * ( frbcop_1.p2[3] + yy4
                                  * frbcop_1.p3[3] ) );
    s1 = frbcop_1.p1[0] + yy4 * ( frbcop_1.p11 + yy4 * ( frbcop_1.p12 + yy4 *
                                  frbcop_1.p13 ) );
    s2 = frbcop_1.p2[0] + yy4 * ( frbcop_1.p21 + yy4 * ( frbcop_1.p22 + yy4 *
                                  frbcop_1.p23 ) );
    s3 = frbcop_1.p3[0] + yy4 * ( frbcop_1.p31 + yy4 * ( frbcop_1.p32 + yy4 *
                                  frbcop_1.p33 ) );
    ret_val = s0 + xx4 * ( s1 + xx4 * ( s2 + xx4 * s3 ) ) - frbcof_1.cl;
    return ret_val;
L30:
    ret_val = frbcop_1.r0[frbcof_1.kse - 1] + *t * frbcop_1.r1[frbcof_1.kse -
              1];
    return ret_val;
L40:
    ret_val = frbcop_1.q0[frbcof_1.kse - 1] + *t * ( frbcop_1.q1[frbcof_1.kse
              - 1] + *t * frbcop_1.q2[frbcof_1.kse - 1] );
    return ret_val;
    } /* frbeva_ */

/* Subroutine */ int frbfop_ ( x, y, icol, ncol )
real *x, *y;
integer *icol, *ncol;
    {
    static integer j;
    extern /* Subroutine */ int frbfcl_();


    /*     OPEN FILL AREA BUFFER */

    /*     IF ALREADY OPEN, CLEAR BUFFER FIRST */
    /* Parameter adjustments */
    --icol;
    --y;
    --x;

    /* Function Body */
    if ( frbcob_1.nfabu == 1 )
        {
        frbfcl_ ( &icol[1] );
        }
    /*     FILL BUFFER */
    for ( j = 1; j <= 4; ++j )
        {
        frbcob_1.xfabu[j - 1] = x[j];
        frbcob_1.yfabu[j - 1] = y[j];
        /* L100: */
        }
    /*     SET COLOUR OF FILL AREA BUFFER */
    frbcob_1.ncolbu = *ncol;
    /*     DECLARE FILL AREA BUFFER OPEN */
    frbcob_1.nfabu = 1;
    return 0;
    } /* frbfop_ */

/* Subroutine */ int frbfup_ ( x, y )
real *x, *y;
    {

    /*     UPDATE FILL AREA BUFFER */

    /* Parameter adjustments */
    --y;
    --x;

    /* Function Body */
    frbcob_1.xfabu[3] = x[4];
    frbcob_1.yfabu[3] = y[4];
    frbcob_1.xfabu[0] = x[1];
    frbcob_1.yfabu[0] = y[1];
    return 0;
    } /* frbfup_ */

/* Subroutine */ int frbfcl_ ( icol )
integer *icol;
    {
    extern /* Subroutine */ int usrplt_();
    extern /* Subroutine */ int usrpsc_();


    /*     CLEAR FILL AREA BUFFER */

    /*     RETURN, IF FILL AREA BUFFER IS CLOSED */
    /* Parameter adjustments */
    --icol;

    /* Function Body */
    if ( frbcob_1.nfabu != 1 )
        {
        goto L100;
        }
    /*     DECLARE FILL AREA BUFFER CLOSED */
    frbcob_1.nfabu = 0;
    /*     CALL FILL AREA ROUTINE */
    /*    usrplt_(frbcob_1.xfabu, frbcob_1.yfabu, &c__4, &icol[frbcob_1.ncolbu], &
            c__0); */
    usrpsc_ ( frbcob_1.xfabu, frbcob_1.yfabu, &c__4, &icol[frbcob_1.ncolbu], &
              c__0, 0,   /* only fill operation */
              NULL, NULL, &c__0, NULL, NULL );

L100:
    return 0;
    } /* frbfcl_ */

real frbzer_ ( ta, tb, f1, f2, er, func )
real *ta, *tb, *f1, *f2, *er;
real ( *func ) ();
    {
    /* System generated locals */
    real ret_val, r__1, r__2;

    /* Builtin functions */
    double r_sign();

    /* Local variables */
    static real a, b, c, e, g, h, s, y, fa, fb, fc, fg, fs, fy;
    extern real frbeva_();


    /*     T R I P   ALGORITHM   A.PREUSSER   FARB-E-2D  VERSION 3.0 01/1991
    */

    /*     COMPUTE ZERO BETWEEN TA AND TB */

    /*     F1= FUNCTION VALUE AT TA */
    /*     F2= FUNCTION VALUE AT TB */
    /*         F1 AND F2 MUST HAVE OPPOSITE SIGN */
    /*         THIS MUST BE CHECKED BEFORE ENTRY */
    /*     ER= PERMITTED ERROR FOR SOLUTION FRBZER */
    /*     NAME OF FUNCTION = FRBEVA */

    /*     THE METHOD IS A COMBINATION OF THE REGULA FALSI */
    /*     AND THE MIDPOINT METHOD */

    /*     IT IS A MODIFIED VERSION OF THE VIM- (CONTROL DATA */
    /*     USER GROUP) ROUTINE WITH CATALOG IDENTIFICATION */
    /*                C2BKYZERO */
    /*     WRITTEN BY LOREN P. MEISSNER, 1965 */

    a = *ta;
    b = *tb;
    fa = *f1;
    fb = *f2;
    c = a;
    fc = fa;
    s = c;
    fs = fc;

L10:
    h = ( b + c ) * ( float ).5;
    if ( ( r__1 = h - b, abs ( r__1 ) ) <= *er )
        {
        goto L110;
        }
    if ( abs ( fb ) <= abs ( fc ) )
        {
        goto L15;
        }
    y = b;
    fy = fb;
    g = b;
    fg = fb;
    s = c;
    fs = fc;
    goto L20;
L15:
    y = s;
    fy = fs;
    g = c;
    fg = fc;
    s = b;
    fs = fb;
L20:
    if ( fy != fs )
        {
        goto L21;
        }
    b = h;
    goto L29;
L21:
    e = ( s * fy - y * fs ) / ( fy - fs );
    if ( ( r__1 = e - s, abs ( r__1 ) ) <= *er )
        {
        r__2 = g - s;
        e = s + r_sign ( er, &r__2 );
        }
    if ( ( e - h ) * ( s - e ) < ( float ) 0. )
        {
        goto L28;
        }
    b = e;
    goto L29;
L28:
    b = h;

    /* *** FUNCTION CALL */
L29:
    fb = ( *func ) ( &b );

    if ( r_sign ( &c_b5, &fg ) * r_sign ( &c_b5, &fb ) < ( float ) 0. )
        {
        goto L35;
        }
    c = s;
    fc = fs;
    goto L10;
L35:
    c = g;
    fc = fg;
    goto L10;

L110:
    ret_val = h;

    return ret_val;
    } /* frbzer_ */
double pow_ri ( ap, bp )
real *ap;
integer *bp;
    {
    double pow, x;
    integer n;

    pow = 1;
    x = *ap;
    n = *bp;

    if ( n != 0 )
        {
        if ( n < 0 )
            {
            if ( x == 0 )
                {
                return ( pow );
                }
            n = -n;
            x = 1/x;
            }
        for ( ; ; )
            {
            if ( n & 01 )
                pow *= x;
            if ( n >>= 1 )
                x *= x;
            else
                break;
            }
        }
    return ( pow );
    }
#define log10e 0.43429448190325182765

double r_lg10 ( x )
real *x;
    {
    double log();

    return ( log10e * log ( *x ) );
    }
double r_mod ( x,y )
real *x, *y;
    {
#ifdef IEEE_drem
    double drem(), xa, ya, z;
    if ( ( ya = *y ) < 0. )
        ya = -ya;
    z = drem ( xa = *x, ya );
    if ( xa > 0 )
        {
        if ( z < 0 )
            z += ya;
        }
    else if ( z > 0 )
        z -= ya;
    return z;
#else
    double floor(), quotient;
    if ( ( quotient = ( double ) *x / *y ) >= 0 )
        quotient = floor ( quotient );
    else
        quotient = -floor ( -quotient );
    return ( *x - ( *y ) * quotient );
#endif
    }
double r_sign ( a,b )
real *a, *b;
    {
    double x;
    x = ( *a >= 0 ? *a : - *a );
    return ( *b >= 0 ? x : -x );
    }




