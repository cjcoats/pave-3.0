/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: mm.c 83 2018-03-12 19:24:33Z coats $
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

#include <stdio.h>

#include "resources.h"

typedef int   integer;
typedef char *address;
typedef short int shortint;
typedef float  real;
typedef double doublereal;

#define TRUE_ (1)
#define FALSE_ (0)
#define VOID void

#define abs(x) ((x) >= 0 ? (x) : -(x))
#define dabs(x) (doublereal)abs(x)
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define dmin(a,b) (doublereal)min(a,b)
#define dmax(a,b) (doublereal)max(a,b)

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

static const char SVN_ID[] = "$Id: mm.c 83 2018-03-12 19:24:33Z coats $";

/* Common Block Declarations */

struct
    {
    real sacmin, cmscal;
    integer maxpol, ncpmax, maxsta, ncmaxs, npp;
    real pi;
    integer maxrid;
    real sigs[4];
    integer ncmax;
    } frbcoc_;

#define frbcoc_1 frbcoc_

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
    real pp0[4], pp1[4], pp2[4], pp3[4], qq0[4], qq1[4], qq2[4], rr0[4], rr1[
        4];
    } frbddx_;

#define frbddx_1 frbddx_

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

/* Table of constant values */

static integer c__1 = 1;
static real c_b19 = ( float ) 0.;
static real c_b32 = ( float ) 1.;
static integer c__2 = 2;

FILE *fp_mms;  /* file pointer to Min, Max, Sad -file*/
int fmms_exist=0; /* indicates, if fp_mms has been opened */


extern res_data reso;
extern int start_of_symb_colors;

/* Subroutine */ int farbmm_ ( x, y, z, zx, zy, zxy, cn, icol, nc, mode, nsides,
                               usrplt )
real *x, *y, *z, *zx, *zy, *zxy, *cn;
integer *icol, *nc, *mode, *nsides;
/* Subroutine */ int ( *usrplt ) ();
    {
    /* Initialized data */

    static integer it = 0;
    static real zs = ( float ) 0.;

    /* Format strings */
    static char fmt_9010[] = "(\0020***ERROR***\002/\002 Y(1).NE.Y(2)  OR   \
X(2).GE.X(1)\002/\002 IN RECTANGLE NO.\002,i10,\002 VERTICES MUST BE ORDERED\
 COUNTER-CLOCKWISE\002/\002 STARTING IN THE UPPER RIGHT CORNER.\002/\002 SID\
ES MUST BE PARALLEL TO X- AND Y-AXIS\002)";
    static char fmt_9020[] = "(\0020***ERROR***, VERTICES OF RECTANGLE NO\
.\002,i10,\002 NOT IN COUNTER-CLOCKWISE ORDER\002)";
    static char fmt_9030[] = "(\0020***WARNING***, CHECK RECTANGLE NO.\002,i\
10/\002 DIFFERENCE IN X- OR Y- COORDINATES TOO LARGE OR TOO SMALL\002/\002 X\
YDIF=\002,e20.5/\002 SCALE X AND/OR Y TO CM (OR INCH)\002)";
    static char fmt_6001[] = "(\002 ***ERROR IN FARBMM\002/\002 MORE THAN\
\002,i8,\002 CONTOURS CROSSING A\002,\002 SIDE OF A RECTANGLE.\002/\002 INCR\
EASE INSTALLATION PARAMETER NCMAXS\002)";
    static char fmt_7001[] = "(\002 *** ERROR *** IN FARBMM\002/\002 OVERFLO\
W OF WORKING STORAGE XPOL,YPOL\002/\002 MAXPOL= \002,i7/\002 RECT.NO. \002,i\
10)";

    /* System generated locals */
    integer i__1, i__2, i__3;
    real r__1, r__2, r__3;

    /* Builtin functions */
    double atan();
    integer s_wsfe(), do_fio(), e_wsfe();
    double r_sign();

    /* Local variables */
    static integer kcll, ndir;
    static real epsn, zx3b3, zx4b3, zy3a3, zy4a3, xpol[300], ypol[300];
    static integer npol1;
    static real a, b, c, d, e;
    static integer i, j, k;
    static real t;
    static integer nside;
    static real f1, f2;
    static integer kvert, njour;
    static real t1[16]  /* was [4][4] */, z1[16]    /* was [4][4] */,
           zmaxt, zmint;
    static integer nstop;
    static real cc[1024], fa, fb;
    static integer in[4];
    extern real frbedx_();
    static integer ii;
    static real tb, ta, xx[2];
    static integer jn;
    static real zz[4], yy[2];
    extern real frbzer_();
    static integer ni, ic1;
    static real di2, di3, cn1;
    extern /* Subroutine */ int frbrid_();
    static integer istart;
    static real tperrl;
    static integer np1;
    static real ts2[4], sl1[4];
    static integer np2;
    static real tperrs;
    static integer journy;
    static real f1f2;
    static integer icn, kcl, jsa;
    static real cnn, z3a3, sl12[4];
    static integer jin, nse;
    static real eps, sl01;
    static integer jza, jjz, jzn, jzr, icn1, ncl1, jsa2, jza2;

    /* Fortran I/O blocks */
    /*
        static cilist io___78 = { 0, 6, 0, fmt_6001, 0 };
        static cilist io___79 = { 0, 6, 0, fmt_7001, 0 };
    */



    /*     F ILL  AR EA  FOR A  B ICUBIC FUNCTION ON A  R E C TANGLE */
    /*     *      **            *                       *   * */

    /*     T R I P   ALGORITHM   A.PREUSSER   FARB-E-2D  VERSION 3.3, 05/1992
    */

    /*     AUTHOR: A. PREUSSER */
    /*             FRITZ-HABER-INSTITUT DER MPG */
    /*             FARADAYWEG 4-6 */
    /*             D-1000 BERLIN 33 */


    /*     THIS SUBROUTINE COMPUTES THE */
    /*                  DERIVATIVE DZ/DX */
    /*     OF THE BICUBIC FUNCTION DETERMINED BY THE */
    /*     VALUES X,Y,Z,ZX,ZY,ZXY GIVEN AT THE FOUR VERTICES OF */
    /*     A RECTANGLE, AND PASSES LINES OF DZ/DX=0 TO THE EXTERNAL */
    /*     FUNCTION FRBCMM */

    /*     INPUT PARAMETERS */
    /*     ================ */
    /*     X,Y,Z         COORDINATES OF THE VERTICES */
    /*                   IN CENTIMETERS (OR INCHES, IF CMSCAL=2.54). */
    /*                   REAL ARRAYS OF LENGTH (4). */
    /*                   X(I),Y(I),Z(I), I=1,4 DEFINE THE */
    /*                   POSITION OF VERTEX (I). */
    /*                   THE SIDES OF THE RECTANGLE MUST BE PARALLEL */
    /*                   TO THE X- AND Y-AXIS, */
    /*                   AND THE VERTICES MUST BE ORDERED */
    /*                   COUNTER-CLOCKWISE AS IS INDICATED BELOW */
    /*                   (VERTEX 1 IN THE UPPER RIGHT CORNER). */
    /*     ZX,ZY,ZXY     DERIVATIVES OF Z AT THE VERTICES. */
    /*                   REAL ARRAYS OF LENGTH (4). */
    /*     NSIDES        4, COMPUTE ZEROS FOR 4 SIDES. (SHOULD BE USED */
    /*                      AS A DEFAULT) */
    /*                   3, COMPUTE ZEROS FOR 3 SIDES ONLY. */
    /*                      ZEROS FOR SIDE 4 ARE COPIED FROM SIDE 2 */
    /*                      (ONLY APPLICABLE, IF THE RECTANGLE OF */
    /*                       THIS CALL IS THE RIGHT HAND NEIGHBOR OF */
    /*                       THE RECTANGLE OF THE PREVIOUS CALL) */

    /*            DENOMINATION OF THE VERTICES AND SIDES OF THE */
    /*                           RECTANGLE */
    /*            Y */
    /*                            SIDE(3) */
    /*  VERTEX(2) * -------------------------------0-------- * VERTEX(1) */
    /*            (                             .            ) */
    /*            (                           .              ) */
    /*            (                          .               ) */
    /*    SIDE(4) (                          . RIDE          ) SIDE(2) */
    /*            (                           .              ) */
    /*            (                             .            ) */
    /*            (                                .         ) */
    /*  VERTEX(3) * ----------------------------------0----- * VERTEX(4) */
    /*                            SIDE(1)                        X */

    /*     THE SIDES ARE PARALLEL TO THE CARTESIAN X-Y-SYSTEM. */

    /*   ----------------------------------------------------------------- */
    /*   END OF USER DOCUMENTATION */
    /*   ----------------------------------------------------------------- */

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


    /*     FRBDDX CONTAINS COEFFICIENTS FOR DERIVATIVES ON SIDES IN */
    /*     X-DIRECTION */


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
    nside = *nsides;


    /*     INITIALISATION FOR FIRST RECTANGLE */
    ++it;

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
    frbcoc_1.pi = atan ( ( float ) 1. ) * ( float ) 4.;
    frbcoc_1.sigs[0] = ( float ) 1.;
    frbcoc_1.sigs[1] = ( float ) 1.;
    frbcoc_1.sigs[2] = ( float )-1.;
    frbcoc_1.sigs[3] = ( float )-1.;
    frbcrd_1.ndir3 = 1;
L10:

    frbcrd_1.kride = 0;
    /*       = NUMBER OF CALLS TO FRBRID */
    frbcrd_1.nprec = 0;
    /*          = NUMBER OF CURVE POINTS COMPUTED FOR RECTANGLE */


    /*     SOME BASIC GEOMETRY FOR THE RECTANGLE */

    frbcrd_1.sl[0] = x[4] - x[3];
    frbcrd_1.sl[1] = y[1] - y[4];
    frbcrd_1.sl[2] = x[1] - x[2];
    frbcrd_1.sl[3] = y[2] - y[3];
    frbcrd_1.x3 = x[3];
    frbcrd_1.y3 = y[3];
    for ( j = 1; j <= 4; ++j )
        {
        zz[j - 1] = z[j];
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


    frbcrd_1.hmin = min ( di2,di3 );
    /*         = SHORTEST SIDE LENGTH */


    if ( frbcrd_1.hmin == ( float ) 0. )
        {
        goto L5000;
        }

    /* Computing MIN */
    r__1 = ( float ).0001 / frbcoc_1.cmscal, r__2 = frbcrd_1.hmin * ( float ).0001 /
            frbcrd_1.slmax;
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
    frbcrd_1.zmin[3] = frbcrd_1.zmin[1] ;
    frbcrd_1.zmax[3] = frbcrd_1.zmax[1] ;
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
    /*     compute coeff for sides */
    frbddx_1.pp0[0] = frbcop_1.q0[0];
    frbddx_1.pp1[0] = frbcop_1.q1[0];
    frbddx_1.pp2[0] = frbcop_1.q2[0];
    frbddx_1.pp3[0] = ( float ) 0.;
    frbddx_1.pp0[1] = frbcop_1.p1[0] + frbcrd_1.dx[0] * ( frbcop_1.p2[0] * (
                          float ) 2. + frbcrd_1.dx[0] * ( float ) 3. * frbcop_1.p3[0] );
    frbddx_1.pp1[1] = frbcop_1.p11 + frbcrd_1.dx[0] * ( frbcop_1.p21 * ( float )
                      2. + frbcrd_1.dx[0] * ( float ) 3. * frbcop_1.p31 );
    frbddx_1.pp2[1] = frbcop_1.p12 + frbcrd_1.dx[0] * ( frbcop_1.p22 * ( float )
                      2. + frbcrd_1.dx[0] * ( float ) 3. * frbcop_1.p32 );
    frbddx_1.pp3[1] = frbcop_1.p13 + frbcrd_1.dx[0] * ( frbcop_1.p23 * ( float )
                      2. + frbcrd_1.dx[0] * ( float ) 3. * frbcop_1.p33 );
    frbddx_1.pp0[2] = frbcop_1.q0[2];
    frbddx_1.pp1[2] = frbcop_1.q1[2];
    frbddx_1.pp2[2] = frbcop_1.q2[2];
    frbddx_1.pp3[2] = ( float ) 0.;
    frbddx_1.pp0[3] = frbcop_1.p1[0];
    frbddx_1.pp1[3] = frbcop_1.p11;
    frbddx_1.pp2[3] = frbcop_1.p12;
    frbddx_1.pp3[3] = frbcop_1.p13;

    /*     DETERMINE POLYNOMIAL COEFF FOR DERIVATIVES ALONG SIDES */
    for ( j = 1; j <= 4; ++j )
        {
        frbddx_1.qq0[j - 1] = frbddx_1.pp1[j - 1];
        frbddx_1.qq1[j - 1] = frbddx_1.pp2[j - 1] * ( float ) 2.;
        frbddx_1.qq2[j - 1] = frbddx_1.pp3[j - 1] * ( float ) 3.;
        frbddx_1.rr0[j - 1] = frbddx_1.qq1[j - 1];
        frbddx_1.rr1[j - 1] = frbddx_1.qq2[j - 1] * ( float ) 2.;
        /* L92: */
        }
    frbcof_1.kk = 1;
    frbcof_1.kse = 1;
    zz[2] = frbedx_ ( &c_b19 );
    zz[3] = frbedx_ ( frbcrd_1.dx );
    frbcof_1.kse = 3;
    zz[0] = frbedx_ ( frbcrd_1.dx );
    zz[1] = frbedx_ ( &c_b19 );
    cc[0] = ( float ) 0.;

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
            f2 = frbedx_ ( &tb );
            /*         LOOP OVER ENDPOINTS OF INTERVALS */
            i__2 = i;
            for ( j = 2; j <= i__2; ++j )
                {
                ta = tb;
                f1 = f2;
                tb = t1[j + ( frbcof_1.kse << 2 ) - 5];
                f2 = frbedx_ ( &tb );
                if ( f1 * f2 > ( float ) 0. )
                    {
                    goto L100;
                    }
                if ( f1 == ( float ) 0. && f2 == ( float ) 0. )
                    {
                    goto L100;
                    }
                /* L95: */
                ts2[ii - 1] = frbzer_ ( &ta, &tb, &f1, &f2, &tperrs, frbedx_ );
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
            z1[j + ( frbcof_1.kse << 2 ) - 5] = frbedx_ ( &t1[j + ( frbcof_1.kse <<
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
    cn1 = ( float ) 0.;
    cnn = ( float ) 0.;
    if ( cnn >= zmint && cn1 < zmaxt )
        {
        goto L500;
        }

    /*     SET NO. OF ZEROS TO 0, FOR NEIGHBOR RECTANGLE */
    frbcrd_1.nz[1] = 0;
    goto L5000;
L500:

    /*     IC1= FIRST CONTOUR LEVEL */
    /*     ICN= LAST CONTOUR LEVEL */
    ic1 = 1;
    icn = 1;
    icn1 = icn - ic1 + 1;

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
            ndir = r_sign ( &c_b32, &r__1 );
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
                f1f2 = r_sign ( &c_b32, &f1 ) * r_sign ( &c_b32, &f2 );
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
                if ( *mode > 0 )
                    {
                    ( *usrplt ) ( xx, yy, &c__2, &kcl, &c__1 );
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
                fa = frbedx_ ( &ta );

                /*           COMPUTE VALUE ON NEXT SIDE */
                nse = frbcof_1.kse % 4 + 1;
                frbcof_1.kse = nse;
                /* Computing MIN */
                r__2 = frbcrd_1.sl[nse - 1] * ( float ).01, r__3 = ( r__1 = t1[ (
                            nse << 2 ) - 3] - t1[ ( nse << 2 ) - 4], abs ( r__1 ) ) * (
                            float ).1;
                epsn = min ( r__2,r__3 );
                tb = t1[ ( nse << 2 ) - 4] + epsn * frbcoc_1.sigs[nse - 1];
                fb = frbedx_ ( &tb );

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
                        frbedx_ );

                /*           COMPUTE DERIVATIVE AT ZERO */
                frbcof_1.kk = 4;
                frbcrd_1.sder[jn + ( frbcof_1.kse << 8 ) - 257] = frbedx_ ( &
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
                frbcrd_1.sder[jn - 1 + ( frbcof_1.kse << 8 ) - 257] = -frbedx_ ( &
                        ta ) / eps;
                /* Computing MIN */
                r__2 = sl01, r__3 = ( r__1 = frbcrd_1.tzr[jn - 1 + (
                                                 frbcof_1.kse << 8 ) - 257] - t1[jin + 1 + (
                                                         frbcof_1.kse << 2 ) - 5], abs ( r__1 ) ) * ( float ).1;
                eps = min ( r__2,r__3 );
                tb = frbcrd_1.tzr[jn + ( frbcof_1.kse << 8 ) - 257] + eps *
                     frbcoc_1.sigs[frbcof_1.kse - 1];
                frbcrd_1.sder[jn + ( frbcof_1.kse << 8 ) - 257] = frbedx_ ( &tb ) /
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


    /*     START 'TRIPS' USING THE ZEROS ON THE SIDES AS 'STATIONS' */

    /*     SET PARAMETERS FOR FIRST JOURNEY */
    /*     START AT UNUSED STATIONS (ISTATZ =0) */
    istart = 0;

    /*     LOOP OVER JOURNEYS */
    njour = 1;
    i__1 = njour;
    for ( journy = 1; journy <= i__1; ++journy )
        {

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

                if ( frbcrd_1.istatz[jza + ( jsa << 8 ) - 257] != istart )
                    {
                    goto L2000;
                    }
                /*           IF ()  THEN THIS STATION WILL NOT SERVE AS START
                */


                /*           START TRIP FROM STATION  SIDE JSA, ZERO JZA */


                /*           SET NORMAL DERIVATIVE */
                frbcrd_1.dernor = r_sign ( &c_b32, &frbcrd_1.sder[jza + ( jsa <<
                                           8 ) - 257] );

                frbrid_ ( frbedx_, &jsa, &jza, cc, nc, xpol, ypol, &
                          frbcoc_1.maxpol, &jsa2, &jza2, &npol1, &nstop );
                /*           FIRST RIDE ENDED ON STATION SIDE JSA2, ZERO JZA2
                */

                /*           CALL FOR LINE DRAWING */
                ( *usrplt ) ( xpol, ypol, &npol1, &ncl1, &c__1 );

                /*           BOOK KEEPING FOR START AND END OF RIDE */
                if ( nstop != 0 )
                    {
                    goto L2000;
                    }
                frbcrd_1.istatz[jza2 + ( jsa2 << 8 ) - 257] = 2;
                frbcrd_1.istatz[jza + ( jsa << 8 ) - 257] = 1;


L2000:
                ;
                }
            /*         END OF LOOP OVER ZEROS */
L2010:
            /* L3000: */
            ;
            }
        /*       END OF LOOP OVER SIDES */

        /* L4000: */
        }
    frbcoc_1.npp += frbcrd_1.nprec;

L5000:
    return 0;

    /*     ERROR EXIT */
L6000:
    /*    s_wsfe(&io___78);
        do_fio(&c__1, (char *)&frbcoc_1.ncmaxs, (ftnlen)sizeof(integer));
        e_wsfe(); */
    return 0;

L7000:
    /*    s_wsfe(&io___79);
        do_fio(&c__1, (char *)&frbcoc_1.maxpol, (ftnlen)sizeof(integer));
        do_fio(&c__1, (char *)&it, (ftnlen)sizeof(integer));
        e_wsfe(); */
    return 0;
    } /* farbmm_ */

/* Subroutine */ int frbcmm_ ( x, y, n, ncol, mode )
real *x, *y;
integer *n, *ncol, *mode;
    {
    /* System generated locals */
    integer i__1;
    real r__1, r__2;

    /* Builtin functions */
    double r_sign();

    /* Local variables */
    static integer i, symb_sel;
    static real xm, ym;
    extern real frbedy_();
    static real oldzdy, zdynew, fac, dx,dy;
    extern /* Subroutine */ int gmm_symbol();
#if ! defined(__hpux)
    extern void ( *gtextp ) ();
#endif
    extern real frbeva_();
    double sqrt();

    /*     COMPUTE EXTREMA IN A BICUBIC PATCH, GIVEN */
    /*     THE POLYGON OF DZ/DX=0 */

    /*     (COMPUTES DZ/DY AT ALL POINTS. IF DZ/DY CHANGES */
    /*      SIGN BETEWEEN TWO CONSECUTIVE POINTS, THE EXTREMUM */
    /*      IS INTERPOLATED linearly, USING THE VALUES OF DZ/DY) */


    /*     X,Y      COORDINATES FOR POLYGON */
    /*     N        NUMBER OF POINTS FOR POLYGON */
    /*     NCOL     DUMMY */
    /*     MODE     DUMMY */

    /*     (PARAMETERS ARE IDENTICAL WITH USRPLT) */



    /*     CHECK ALL POINTS FOR ALTERNATING SIGN */

    /* Parameter adjustments */
    --y;
    --x;

    /* Function Body */
    r__1 = x[1] - frbcrd_1.x3;
    r__2 = y[1] - frbcrd_1.y3;
    oldzdy = frbedy_ ( &r__1, &r__2 );
    i__1 = *n;
    for ( i = 2; i <= i__1; ++i )
        {
        r__1 = x[i] - frbcrd_1.x3;
        r__2 = y[i] - frbcrd_1.y3;
        zdynew = frbedy_ ( &r__1, &r__2 );
        if ( r_sign ( &c_b32, &oldzdy ) * r_sign ( &c_b32, &zdynew ) > ( float ) 0. )
            {
            goto L20;
            }
        /*       INTERPOLATE X,Y COORDINATE FOR EXTREMUM */
        fac = -oldzdy / ( zdynew - oldzdy );
        dx= x[i] - x[i - 1];
        dy= y[i] - y[i - 1];
        xm = x[i - 1] + dx * fac;
        ym = y[i - 1] + dy * fac;

            {
            real zero=0.;
            real xt,yt;
            real z, z_left, z_back, dr;
            double s;
            int istrl, col_sel;
            char text[16];

            frbcof_1.kk= 2;
            frbcof_1.xx4f= xm - frbcrd_1.x3;
            frbcof_1.yy4f= ym - frbcrd_1.y3;
            s= dx*dx + dy*dy;
            s= sqrt ( s );
            frbcof_1.sir= dx/s;
            frbcof_1.cor= -dy/s;
            dr= frbcrd_1.hmin*0.1; /* distance from extremum */
            frbcof_1.cl= 0.;

            z= frbeva_ ( &zero ); /*value at extremum*/

            z_left= frbeva_ ( &dr ); /*value left of extremum */

            frbcof_1.xx4f= x[i-1] - frbcrd_1.x3;
            frbcof_1.yy4f= y[i-1] - frbcrd_1.y3;
            z_back= frbeva_ ( &zero ); /*value at last point*/

            xt= xm;

            /* MAXIMUM */
            if ( z_back -z < 0. && z_left -z < 0 )
                {
                col_sel= 2;
                symb_sel= 1;
                if ( reso.max_height != 0 )
                    {
                    gmm_symbol ( xm, ym, ( float ) reso.max_height/100., symb_sel );

                    if ( strcmp ( reso.mms_fil,"nofile" ) != 0 )
                        {
                        if ( fmms_exist == 0 )
                            {
                            if ( ( fp_mms= fopen ( reso.mms_fil, "w" ) ) == NULL )
                                {
                                fprintf ( stderr, "Unable to open %s\n", reso.mms_fil );
                                }
                            else fmms_exist = 1;
                            }
                        if ( fmms_exist == 1 )
                            {
                            fprintf ( fp_mms, "MAX %6.2f %6.2f % 7e\n",xm,ym,z );
                            }
                        }

                    if ( reso.max_anno != 0 )
                        {
                        yt= ym + reso.max_height*0.005 + 0.1; /* 0.1 cm above symbol*/
                        sprintf ( text,reso.max_form,z ); /* encode value */
                        istrl= strlen ( text );
                        gtextp ( &xt, &yt, 1, text, 0., 0.5, 0.,
                                 start_of_symb_colors + NDEFCOL+col_sel, /* foreground */
                                 start_of_symb_colors + NDEFCOL+col_sel+1, /*backgr*/
                                 &istrl );
                        }
                    }
                } /*maximum*/

            /* MINIMUM */
            if ( z_back -z > 0. && z_left -z > 0 )
                {
                col_sel= 4;
                symb_sel= 2;
                if ( reso.min_height != 0 )
                    {
                    gmm_symbol ( xm, ym, ( float ) reso.min_height/100., symb_sel );

                    if ( strcmp ( reso.mms_fil,"nofile" ) != 0 )
                        {
                        if ( fmms_exist == 0 )
                            {
                            if ( ( fp_mms= fopen ( reso.mms_fil, "w" ) ) == NULL )
                                {
                                fprintf ( stderr, "Unable to open %s\n", reso.mms_fil );
                                }
                            else fmms_exist = 1;
                            }
                        if ( fmms_exist == 1 )
                            {
                            fprintf ( fp_mms, "MIN %6.2f %6.2f % 7e\n",xm,ym,z );
                            }
                        }


                    if ( reso.min_anno != 0 )
                        {
                        yt= ym + reso.min_height*0.005 + 0.1; /* 0.1 cm above symbol*/
                        sprintf ( text,reso.min_form,z ); /* encode value */
                        istrl= strlen ( text );
                        gtextp ( &xt, &yt, 1, text, 0., 0.5, 0.,
                                 start_of_symb_colors + NDEFCOL+col_sel, /* foreground */
                                 start_of_symb_colors + NDEFCOL+col_sel+1, /*backgr*/
                                 &istrl );
                        }
                    }
                } /*minimum*/


            /* SADDLE */
            if ( z_back -z > 0. && z_left -z < 0 ||
                    z_back -z < 0. && z_left -z > 0 )
                {
                col_sel= 6;
                symb_sel= 3;
                if ( reso.sad_height != 0 )
                    {
                    gmm_symbol ( xm, ym, ( float ) reso.sad_height/100., symb_sel );

                    if ( strcmp ( reso.mms_fil,"nofile" ) != 0 )
                        {
                        if ( fmms_exist == 0 )
                            {
                            if ( ( fp_mms= fopen ( reso.mms_fil, "w" ) ) == NULL )
                                {
                                fprintf ( stderr, "Unable to open %s\n", reso.mms_fil );
                                }
                            else fmms_exist = 1;
                            }
                        if ( fmms_exist == 1 )
                            {
                            fprintf ( fp_mms, "SAD %6.2f %6.2f % 7e\n",xm,ym,z );
                            }
                        }

                    if ( reso.sad_anno != 0 )
                        {
                        yt= ym + reso.sad_height*0.005 + 0.1; /* 0.1 cm above symbol*/
                        sprintf ( text,reso.sad_form,z ); /* encode value */
                        istrl= strlen ( text );
                        gtextp ( &xt, &yt, 1, text, 0., 0.5, 0.,
                                 start_of_symb_colors + NDEFCOL+col_sel, /* foreground */
                                 start_of_symb_colors + NDEFCOL+col_sel+1, /*backgr*/
                                 &istrl );
                        }
                    }
                } /*saddle*/
            }

L20:
        oldzdy = zdynew;
        /* L30: */
        }

    return 0;
    } /* frbcmm_ */

real frbedx_ ( t )
real *t;
    {
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real s1, s2, s3, ss0, ss1, ss2, xx4, yy4;


    /*     T R I P   ALGORITHM   A.PREUSSER   FARB-E-2D  VERSION 3.3, 05/1992
    */

    /*     EVALUATION OF DZ/DXC */

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
    ret_val = frbddx_1.pp0[frbcof_1.kse - 1] + *t * ( frbddx_1.pp1[
                  frbcof_1.kse - 1] + *t * ( frbddx_1.pp2[frbcof_1.kse - 1] + *t *
                          frbddx_1.pp3[frbcof_1.kse - 1] ) );
    return ret_val;
L20:
    xx4 = frbcof_1.xx4f + frbcof_1.cor **t;
    yy4 = frbcof_1.yy4f + frbcof_1.sir **t;
    /*      S0= P0(1) + YY4*(P1(4)+YY4*(P2(4)+YY4*P3(4))) */
    s1 = frbcop_1.p1[0] + yy4 * ( frbcop_1.p11 + yy4 * ( frbcop_1.p12 + yy4 *
                                  frbcop_1.p13 ) );
    s2 = frbcop_1.p2[0] + yy4 * ( frbcop_1.p21 + yy4 * ( frbcop_1.p22 + yy4 *
                                  frbcop_1.p23 ) );
    s3 = frbcop_1.p3[0] + yy4 * ( frbcop_1.p31 + yy4 * ( frbcop_1.p32 + yy4 *
                                  frbcop_1.p33 ) );
    ss0 = s1;
    ss1 = s2 * ( float ) 2.;
    ss2 = s3 * ( float ) 3.;
    /*      FRBEVA= S0 + XX4*(S1+XX4*(S2+XX4*S3)) - CL */
    ret_val = ss0 + xx4 * ( ss1 + xx4 * ss2 );
    return ret_val;
L30:
    ret_val = frbddx_1.rr0[frbcof_1.kse - 1] + *t * frbddx_1.rr1[frbcof_1.kse
              - 1];
    return ret_val;
L40:
    ret_val = frbddx_1.qq0[frbcof_1.kse - 1] + *t * ( frbddx_1.qq1[
                  frbcof_1.kse - 1] + *t * frbddx_1.qq2[frbcof_1.kse - 1] );
    return ret_val;
    } /* frbedx_ */

real frbedy_ ( x, y )
real *x, *y;
    {
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real s1, s2, s3, ss0, ss1, ss2;


    /*     T R I P   ALGORITHM   A.PREUSSER   FARB-E-2D  VERSION 3.3, 05/1992
    */

    /*     EVALUATION OF DZ/DY INSIDE RECTANGLE */

    /*     AUTHOR      : A. PREUSSER */
    /*                   FRITZ-HABER-INSTITUT */
    /*                   DER MAX-PLANCK-GESELLSCHAFT */
    /*                   FARADAYWEG 4-6 */
    /*                   D-1000 BERLIN 33 */


    s1 = frbcop_1.p1[3] + *x * ( frbcop_1.p11 + *x * ( frbcop_1.p21 + *x *
                                 frbcop_1.p31 ) );
    s2 = frbcop_1.p2[3] + *x * ( frbcop_1.p12 + *x * ( frbcop_1.p22 + *x *
                                 frbcop_1.p32 ) );
    s3 = frbcop_1.p3[3] + *x * ( frbcop_1.p13 + *x * ( frbcop_1.p23 + *x *
                                 frbcop_1.p33 ) );
    ss0 = s1;
    ss1 = s2 * ( float ) 2.;
    ss2 = s3 * ( float ) 3.;
    ret_val = ss0 + *y * ( ss1 + *y * ss2 );
    return ret_val;
    } /* frbedy_ */

int gmm_symbol ( x,y,height,symbol )
/*******************************/
float x, y;
float height;
int symbol;
    {
#ifdef ALT
    static float  h2, radius;
    static int c__2=2;
    extern void ( *gplp ) ();
    extern void ( *gsfaip ) ();
    double atan(), sin(), cos();
    double dtmp;

    static int i, j ;
    static int c__17 = 17;
    static int c__0 = 0;
    static float  pi8, xtemp[18], ytemp[18], pi, factor;

    /* set foreground */
    gsfaip ( &c__0 );

    if ( symbol == 1 || symbol ==3 )
        {
        h2= height*0.5;
        xtemp[0]= x - h2;
        xtemp[1]= x + h2;
        ytemp[0]= y - h2;
        ytemp[1]= y + h2;
        gplp ( &c__2,xtemp,ytemp );

        xtemp[0]= x - h2;
        xtemp[1]= x + h2;
        ytemp[0]= y + h2;
        ytemp[1]= y - h2;
        gplp ( &c__2,xtemp,ytemp );
        }

    if ( symbol == 2 || symbol ==3 )
        {
        radius= height*0.5;
        dtmp= 1.;
        pi8 = atan ( dtmp ) / 2.;
        dtmp=0.;
        for ( j = 0; j <= 16; ++j )
            {

            xtemp[j ] = x + radius * sin ( dtmp );
            ytemp[j ] = y + radius* cos ( dtmp );
            dtmp+= pi8;
            }

        gplp ( &c__17, xtemp, ytemp );
        }
#endif
    }
