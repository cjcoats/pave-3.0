/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: retrieveData.c 83 2018-03-12 19:24:33Z coats $
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
 *      
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ****************************************************************************
 *FILE:           retrieveData.c
 *
 *AUTHOR:         Steve Thorpe
 *                MCNC Environmental Programs
 *                thorpe@ncsc.org
 *
 *DATE:           12/22/94
 *
 *PURPOSE:    This program will be used to implement
 *        a processor for a postfix form arithmetic expression;
 *        it produces an array of data corresponding to the
 *        postfix formula.  The data will then be returned
 *        to PAVE for visual and/or statistical analysis.
 *
 *        An operator will either be '+', '-', '/', '*', '**',
 *        '<=', '>=', '<', '>', '==', '!=', '&&', '||',
 *        'abs', 'sqr', 'sqrt', 'exp', 'ln', 'sin', 'cos', 'tan',
 *        'sind', 'cosd', 'tand', or 'log'.  An operand may be
 *        suffixed with an optional colon followed by an hour
 *        (":24", for example), and will have four possible cases:
 *
 *        a)  it may be a floating point constant, which
 *            will be denoted by CX, where X is the
 *            constant.  'pi' and 'e' are also accepted
 *            as constants and translated appropriately.
 *
 *        b)  it may indicate an array of RADM data, which
 *            will be denoted by SXi, where X is the specie
 *            index, and i is a character
 *            denoting which of the selected cases that
 *                    data will come from.
 *
 *        c)  it may indicate an array of RADM depositions,
 *            which will be denoted by DX.Yi, where X is the
 *            species index, Y is the level index for that
 *            deposition within that species, and i is a
 *            character denoting which of the
 *            selected cases that data will come from.
 *
 *        d)  it may indicate sigma value for the middle
 *            of the layer as computed using the HC global
 *            variable thickList. this will be denoted T.
 *            the sigma value T for a given level X will
 *            be determined using the following scheme:
 *            E = the summation of thickList[i], i = 1 to KMAX
 *            T[KMAX] = 0.5 * thickList[KMAX]/E
 *            T[X] = T[X+1] + 0.5 *
 *                (thickList[X+1] + thickList[X]) / E
 *
 *        This program will access the actual data from
 *        EDDS data file(s).
 *
 *        This code development originally began on 2/14/89
 *        at the Atmospheric Sciences Research Center's (ASRC)
 *        Acid Deposition Modeling Project (ADMP), based at
 *        the State University of New York at Albany (SUNYA)
 *        (otherwise affectionately known as the alphabet place ;).
 *        It was originally developed under the direction of
 *        Dr. Julius Chang as part of the Regional Acid Deposition
 *        Model Analysis Package (RAP), a Macintosh based
 *        analysis and visualization system which runs under
 *        a HyperCard front end.
 *
 ****************************************************************************
 * MODIFICATION HISTORY:
 * 
 * WHO  WHEN   WHAT
 * ---  ----       ----
 * SRT  02/14/89   Implemented
 * 
 * HPC  03/29/89   Retrieve unit flag, convert ppm->g
 * 
 * HPC  05/01/89   getData now doesn't get deposition data
 * 
 * SRT  05/03/89   whichScale ignored if doing mass units
 * 
 * SRT  05/05/89   MSF and PSTAR now resources not files
 *         divide by zero now returns an error message
 * 
 * SRT  05/18/89   getData cleans up better on error detection
 * 
 * SRT  06/15/89   now get data for only the selected domain;
 *         uses mac i/o calls not stdio calls
 * 
 * SRT  06/28/89   Will now compile either with/without coprocessor;
 *         uses caseList, refNumList, not caseA, refNumA, etc.
 * 
 * SRT  08/08/89   PSTAR array is now only 2D not 3D array
 * 
 * SRT  08/10/89   Processes MAC binary data not VAX binary
 * 
 * SRT  09/06/89   Handles deposition data again; no longer supports
 *         mass units; will now modify "TSDATA" resource if this
 *         is a time series.
 * 
 * SRT  09/25/89   Now does straight averaging for total/time, rather
 *         than weighted according to layer height.
 * 
 * SRT  12/13/89   Added modification history.
 * 
 * SRT  02/01/90   Make use of global thickList variable.
 * 
 * SRT  03/13/90   Now handles unary operators log, sqr, and sqrt.
 * 
 * SRT  03/22/90   Now handles getting data averaged over multiple hours.
 * 
 * SRT  05/07/90   If floor[0]=='1', abs(denominator) <= abs(floorCut) will make
 *         the answer 0 for that quotient.
 * 
 * SRT  05/29/90   Allowed to attach a specific hour to a species within
 *         a formula, by adding a colon and the hour on the end.
 * 
 * SRT  07/02/90   Added a check for unitx/unitx make units empty!
 * 
 * SRT  07/03/90   getData now accepts unary abs and binary * *  operators
 * 
 * SRT  07/18/90   Allowed to compute the change of a species per hour within
 *         a formula, denoted by :dt after the species in postFixQueue
 * 
 * DAH  08/06/90   All changes preceded by "dh". changed to allow 2d and
 *         3d species in same formula. If 2d just uses first level.
 *         Now this is getting from the parser the format:
 *         SXi or DXi where S or D denotes 3d or 2d respectively,
 *         X is the absolute starting level, and i is the indicator
 *         of the case name. further description of X if the second
 *         3d species is chosen, X = (species_number - 1) *  Maxlevels
 *                   ex:   so2  0   X=0
 *                          1
 *                          2
 *                          3
 *                          4
 *                          5
 *                     so4  6   X=6
 *                          7
 *                          8
 *                          9
 *                          10
 *                          11
 *                     no2  12  X=12
 *                          13
 *                          ...
 *                     dSO2 96  X=96
 *                     dSO4 97  X=97
 * 
 *         old files depositions will have format of DX.Li
 *         where X is the absolute start level of the kmax
 *         data block and .L is the level in that block of data.
 * 
 * DAH  09/04/90   add mixed to tell if is mixed 2d and 3d then need to do
 *         operation on all levels using the 2d data
 * 
 * DAH  09/05/90   make dtype tell not just if array or constant, but
 *         if 2d array, 3d array, or constant
 * 
 * SRT  09/13/90   KMAX is now allowed to a maximum of 50 not 20
 * 
 * SRT  09/17/90   modified HC variable IO list above, sorted cGetGlobals below
 * 
 * DAH  10/22/90   modified to get an extra grid cell in the x and y direction
 *         so fluxes will work.
 * 
 * SRT  11/07/90   Modified to handle intermediate hours in get_chem_data(),
 *         so the user will be able to make smoother animations, given a
 *         whichHour with a decimal portion.
 * 
 * SRT  11/12/90   Now uses ttlLevelsList rather than ttlLevels, since every
 *         available case has its own ttlLevels.  Changed the code
 *         if [(type1 == DTYPECONC)] to [if (type1 == SARRPTR)] and
 *         if [(type2 == DTYPECONC)] to [if (type2 == SARRPTR)].
 * 
 * SRT  11/15/90   Removed "Warning - mixing 2d and 3d species!" Putandwait
 *         message, since with generation of animations this would
 *         be a problem.
 * 
 * SRT  11/16/90   Changed how thisKMAX is assigned, in order to have the
 *         2d/3d mixing work correctly.  No longer need dataType,
 *         mixed, and thisKMAX global variables, since everything
 *         necessary can be determined using stack->dtype.
 * 
 * SRT  11/20/90   Completed (we hope!) changes to get 2d and 3d arrays
 *         in one formula working correctly.  Changed a few global
 *         variables into local variables to save XCMD size.
 * 
 * SRT  11/26/90   If the user holds down command and period, the
 *         XCMD will be exited, and errormsg will be set to "cancel".
 *         Removed globalC variable and setting of plotCancel.
 * 
 * SRT  11/28/90   Added the option to handle T = sigma(half) as an atom.
 * 
 * SRT  11/29/90   Z vs T getData option (integrate == 10) incorporated.
 * 
 * SRT  12/12/90   Changed tsdata from an automatic variable to a dynamically
 *         allocated array, in order to save on stack space usage, which
 *         may have been causing problems before.
 * 
 * SRT  12/22/94   Created PAVE's retrieveData.c from RAP's getData.c
 * 
 * SRT  12/28/94   Removed multiple hours option - if want to go back to
 *                 it, check out the original RAP's getData.c code.
 *         Changed name of get_chem_data() to get_spec_data().
 * 
 * SRT  01/12/95   Added exp, ln, sin, cos, tan, sind, cosd, and
 *                 tand operators, as well as pi and e constants.
 *         Also put in an error check for x* * y -
 *         if ((x<0) && (y is non-integral)) then x* * y is invalid.
 * 
 * SRT  01/26/95   modified to NOT get the extra grid cell in the x and y
 *         direction (which used to be used so fluxes will work)
 * 
 * SRT  01/27/95   Modified to no longer handle interpolating intermediate
 *         time steps in get_spec_data().
 * 
 * SRT  02/22/95   Add capability to handle minx, miny, minz, mint, maxx,
 *                 maxy, maxz, maxt, mean, min, max, sum, nrows, ncols, nlevels
 * 
 * SRT  02/23/95   Add capability to retrieve "scatter data", ie a 1D array
 *         to be used for a correlation plot
 * 
 * SRT  02/24/95   Incorporate ability to override hour setting in a
 *         species within a formula by adding :<hour> after species
 * 
 * SRT  02/27/95   If a cell is not in the domain, then that cell's data
 *         will be stuffed with a NaN.  The routine isnanf()
 *         should now be used to test every data point within a
 *         a VIS_DATA struct's grid for validity, if isnanf() returns
 *         1 then that cell's data is invalid.  Also - retrieveData()
 *         now only tests for division/log/ln errors in a cell if
 *         that cell & level is in the domain.
 * 
 * SRT  08/30/95   Allow mixing variables w/ different map_info strings
 * 
 * SRT  11/16/95   KMAX is now allowed to a maximum of 512 not 50
 * 
 * SRT  10/07/96   Added support for <, <=, >, >=, !=, ==, &&, and || operators
 * 
 * SRT  10/21/96   Added makeSureIts_netCDF() calls, added get_data()
 * 
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 *************************************************************/
#include <math.h>

#include "bts.h"


/* struct data types for this file only */

struct stack_item
    {
    int      dtype;              /* a constant or an array? */
    double   constant;
    VIS_DATA vdata;
    struct   stack_item *sptr;   /* pointer to the item below on stack */
    };


/* #defines for this file only */

#define TMAX         ((long)((*hrMax)-(*hrMin)+1))
#define ARRSIZE      ((long)((long)IMAX*JMAX*KMAX*TMAX*sizeof(float)))
#define DEPARRSIZE   ((long)((long)IMAX*JMAX*TMAX*sizeof(float)))

/* types of "atoms" entered by the user */
#define         FLTPTR          0
#define         SARRPTR         1
#define         DARRPTR         2

/* new operators added 2/95 - SRT */
#define     NO_TYPE     0
#define     MINX        1
#define     MINY        2
#define     MINZ        3
#define     MAXX        4
#define     MAXY        5
#define     MAXZ        6
#define     MINT        7
#define     MAXT        8
#define     MEAN        9
#define     MINIMUM     10
#define     MAXIMUM     11
#define     SUM     12

/* map_info is formatted "%g%g%g%g%d%d%d" or "%d%g%g%g%g%g%g%g%g%g%d%d",
   so a BOGUS map_info should take this into account */
#define     BOGUS_MAP_INFO  "-1 -1.0 -1.0 -1.0 -1.0 -1.0 -1.0 -1.0 -1.0 -1.0 -1 -1"

/* global (ugh!) variables for this file only */

static VIS_DATA *caseInfo;

static struct BusData *bd;

static float floorCut,
            *thickVals,
            *sigmaVals;

static int  IMAX,
            JMAX,
            KMAX,
            selectedStep,
            fullIMAX,
            fullJMAX,
            fullKMAX,
            fpos,           /* current parsing position in formula */
            ncases,
            colMin,
            colMax,
            rowMin,
            rowMax,
            levelMin,
            levelMax,
           *hrMin,
           *hrMax,
           *whichLevel,
            dt,
            selectCol,
            selectRow,
            selectLevel,
           *stepMin,
           *stepMax,
           *stepIncr,
            thisHour;
static int  sliceType;

static char  atom[25],      /* the current atom being processed */
            *formula,
            *percents,
            *caseList,
            *hostList,        /* list of hosts separated by commas */
            *infixFormula,
             mem_msg[] = "retrieveData() needs more memory!",
             flor;

static
struct  stack_item  *stack;



/* function prototypes for routines for this file only */

static int               push            ( void );

static struct stack_item *pop            ( void );

static int               advance         ( void );

static int               free_up         ( void );

static int               fillZlevels     ( float *sdata,
                                           float *zdata,
                                           int    step );

static double            totalIntegration ( float *sdata,
        int thisKMAX,
        int step );

static int       get_spec_data   ( char     *fname,
                                   char     *hname,
                                   char     caseChar,
                                   int      spec_index,
                                   VIS_DATA *sdata,
                                   int      thisKMAX );


static int               process         ( void );

static void          freeCaseInfo    ( void );


static int       my_get_data ( struct BusData *bd, VIS_DATA *info, char *message );



/************************************************************
MY_GET_INFO -   "Wraps" functionality around get_info()
        to make sure UAM* data is recast into netCDF data.

            Returns 1 if success, 0 on failure with an error message
            printed into the message argument.
************************************************************/
int my_get_info ( struct BusData *bd, VIS_DATA *info, char *message )
    {
    if ( !get_info ( bd, info, message ) )
        return 0;

    /* convert to netCDF (really, IO/API map_info
       information) data if it isn't already */
    if ( makeSureIts_netCDF ( info, message ) )
        return 0;

    return 1;
    }



/************************************************************
MY_GET_DATA -   Makes a get_data() call, while "wrapping" functionality
            around get_data() to make sure UAM* data is
            recast into netCDF data.  This enables users to
            mix and match data from UAM* and netCDF/Models-3 IO/API
            files within the same formula and plots.

        This routine assumes that all get_info calls were
        followed by a call to makeSureIts_netCDF().

            Returns 1 if success, 0 on failure with an error message
            printed into the message argument.
************************************************************/
static int my_get_data ( struct BusData *bd, VIS_DATA *info, char *message )
    {
    char *map_info;
    enum dataset_type dataset = info->dataset;

    map_info = info->map_info;
    info->map_info = NULL;

    if ( !get_data ( bd, info, message ) )
        return 0;

    if ( info->map_info ) free ( info->map_info );
    info->map_info = map_info;
    info->dataset = dataset;
    return 1;
    }



/************************************************************
FREECASEINFO -  frees up storage used for case information structs
************************************************************/
static
void    freeCaseInfo ( void )
    {
    int     i;

    if ( caseInfo != NULL )
        {
        for ( i = 0; i < ncases; i++ )
            if ( caseInfo[i].filename != NULL )
                myFreeVis ( &caseInfo[i] );

        free ( caseInfo );
        caseInfo = NULL;
        }
    }





/************************************************************
PUSH -  pushes a data (constant or array pointer) onto stack;
    returns 1 if there was a failure
************************************************************/
static int push ( void )
    {
    struct  stack_item *tstack;

    if ( ( tstack = ( struct stack_item * )
                    malloc ( ( size_t ) sizeof ( struct stack_item ) ) ) == NULL )
        return errmsg ( mem_msg );

    /* set that stack item's VIS_DATA to all NULL values */
    memset ( ( void * ) &tstack->vdata, 0, ( size_t ) sizeof ( VIS_DATA ) );

    tstack->sptr = stack;
    stack = tstack;

#ifdef DIAGNOSTICS
    printf ( "PUSH\n" );
#endif /* DIAGNOSTICS */

    return 0;
    }



/************************************************************
POP -   pops a stack item off of the stack, *WITHOUT*
    freeing any space allocated to that stack item;
    returns NULL if there was a failure, otherwise
    returns a pointer to that stack item.
************************************************************/
static struct stack_item *pop ( void )
    {
    struct stack_item *tstack;

#ifdef DIAGNOSTICS
    printf ( "POP\n" );
    if ( stack == NULL )
        printf ( " (NULL stack!)" );
    printf ( "\n" );
#endif /* DIAGNOSTICS */

    if ( stack == NULL )
        return ( struct stack_item * ) NULL;
    tstack = stack;
    stack = stack->sptr;
    return tstack;
    }



/************************************************************
ADVANCE - advances the input pointer, grabbing the next atom
************************************************************/
static int advance ( void )
    {
    int     atompos = 0;  /* current position in the current atom */

    while ( formula[fpos] == ' ' ) fpos++;
    while ( ( formula[fpos] != ' ' ) &&
            ( formula[fpos] != '\0' ) )
        atom[atompos++] = formula[fpos++];
    atom[atompos] = '\0';

    if ( !strcmp ( atom,"**" ) ) strcpy ( atom, "p" ); /* replace "**" by "p" = power operator */
    if ( !strcmp ( atom,"<=" ) ) strcpy ( atom, "l" ); /* replace "<=" by "l" = lte   operator */
    if ( !strcmp ( atom,">=" ) ) strcpy ( atom, "g" ); /* replace ">=" by "g" = gte   operator */
    if ( !strcmp ( atom,"!=" ) ) strcpy ( atom, "!" ); /* replace "!=" by "!" = neq   operator */
    if ( !strcmp ( atom,"==" ) ) strcpy ( atom, "=" ); /* replace "==" by "=" = eq    operator */

#ifdef DIAGNOSTICS
    printf ( "ADVANCED to atom '%s'\n", atom );
#endif /* DIAGNOSTICS */

    return 0;
    }



/************************************************************
FREE_UP - frees up any remaining allocated space;
      returns 0
************************************************************/
static int free_up ( void )
    {
    struct stack_item *tstack;

    while ( ( tstack = pop() ) != NULL )
        {
        if ( tstack->dtype ) myFreeVis ( &tstack->vdata );
        free ( tstack );
        tstack = NULL;
        }
    return 0;
    }



/************************************************************
fillZlevels -   computes the vertical profile for the
        given species data.  Note: this should
        be the *same* algorithm as vertical profile
        in integrate.c!

        Note:  step is *0* based

               if (step >= 0) then only that
               time step is computed for;

               if (step < 0) then fillZlevels
               is performed for all time steps
               that are there, integrated down
               to just KMAX numbers

        returns 1 if error.
************************************************************/
static int fillZlevels ( float *sdata, float *zdata, int step )
    {
    int     i, j, k, p, t, tmin, tmax;
    double      total, cells_on, cells_fac ;

    if ( step >= 0 )
        tmin = tmax = step;
    else
        {
        tmin = 0;
        tmax = *hrMax - *hrMin;
        }

    cells_on = 0.0;
    for ( j = 0; j < JMAX; j++ )
        for ( i = 0; i < IMAX; i++ )
            cells_on += ( double ) percents[INDEX ( i,j,0,0,IMAX,JMAX,1 )];

    if ( cells_on == 0.0 )
        {
        for ( k = 0; k < KMAX; k++ )
            zdata[k] = 0.0;

        return 0;
        } 

    cells_fac = 100.0 / ( (double)( tmax-tmin+1 ) * cells_on ) ;

    for ( k = 0; k < KMAX; k++ )
        {
        zdata[k] = 0.0;
        if ( whichLevel[k] )
            {
            if ( cancelKeys() ) return errmsg ( "cancel" );
            total = 0.0;
            for ( t = 0; t <= tmax; t++ )
                for ( j = 0; j < JMAX; j++ )
                    for ( i = 0; i < IMAX; i++ )
                        {
                        p = percents[INDEX ( i,j,0,0,IMAX,JMAX,1 )] ;
                        total += ( sdata[INDEX ( i,j,k,t,IMAX,JMAX,KMAX )] * p );
                        }
            zdata[k] = cells_fac * total ;
            }
        }

    return 0;
    }

/************************************************************
totalIntegration -  computes the total integration for the
            given species data.

            Note:  step is *0* based

                   if (step >= 0) then only that
                   time step is computed for;

                   if (step < 0) then totalIntegration
                   is performed for all time steps
                   that are there, integrated down
                   to one number

            Returns 1 if error is encountered.
************************************************************/
static double totalIntegration ( float *sdata, int thisKMAX, int step )
    {
    double  total, cells_on, cells_fac, tsum;
    int i, j, k, t, tmin, tmax;
    long    index;

    if ( step >= 0 )
        tmin = tmax = step;
    else
        {
        tmin = 0;
        tmax = *hrMax - *hrMin;
        }

    cells_on = 0.0;
    for ( j = 0; j < JMAX; j++ )
        {
        for ( i = 0; i < IMAX; i++ )
            {
            cells_on += ( double ) percents[INDEX ( i,j,0,0,IMAX,JMAX,1 )] ;
            }
        }

    if ( cells_on == 0.0 ) 
        {
        return 0.0 ;
        }

    cells_fac = 100.0 / ( cells_on * (double)( tmax-tmin+1 ) ) ;
    total     = 0.0;
    for ( t = tmin; t <= tmax; t++ )
        {
        if ( thisKMAX == KMAX )
            {
            for ( k = 0, tsum = 0.0; k < KMAX; k++ )
                {
                if ( whichLevel[k] )    /* this level is selected; compute an average */
                    {
                    for ( tsum = 0.0, j = 0; j < JMAX; j++ )
                        {
                        for ( i = 0; i < IMAX; i++ )
                            {
                            tsum += ( sdata[index] * (float)percents[INDEX ( i,j,0,0,IMAX,JMAX,1 )] ) ;
                            }
                        }
                    total += tsum * thickVals[k] ;
                    }
                }
            }
        else{
            for ( j = 0; j < JMAX; j++ )
                {
                for ( i = 0; i < IMAX; i++ )
                    {
                    total += ( sdata[index] * ( float ) ( percents[INDEX ( i,j,0,0,IMAX,JMAX,1 )] ) );
                    }
                }
            }
        }

    return total * cells_fac;
    }




/************************************************************
GET_SPEC_DATA - retrieves data for a particular species,
        storing it in sdata.

        Returns 1 if an error is encountered.
************************************************************/
static int get_spec_data (
                         char            *fname,
                         char            *hname,
                         char            caseChar,
                         int         spec_index,
                         VIS_DATA        *sdata,
                         int         thisKMAX
                         )
    {
    int         i, j, k, s, smin, smax, h, t;
    char            tstring[512], tstring2[512];
    int         HACK = 0;

    if ( sdata == NULL ) return errmsg ( "NULL sdata arg to get_spec_data()!!" );

    /* set sdata's info to all NULL values */
    memset ( ( void * ) sdata, 0, ( size_t ) sizeof ( VIS_DATA ) );

    /* put filename in VIS_DATA struct */
    if ( ( sdata->filename = strdup ( fname ) ) == NULL )
        {
        myFreeVis ( sdata );
        return errmsg ( mem_msg );
        }

    /* put hostname in VIS_DATA struct */
    if ( ( sdata->filehost.name = strdup ( hname ) ) == NULL )
        {
        myFreeVis ( sdata );
        return errmsg ( mem_msg );
        }

    /* fill up VIS_DATA struct with template info using get_info */
    if ( !get_info ( bd, sdata, errorString ) )
        {
        myFreeVis ( sdata );
        return 1;
        }

    /* convert to netCDF (really, IO/API map_info information) data if it isn't already */
    if ( makeSureIts_netCDF ( sdata, errorString ) )
        {
        myFreeVis ( sdata );
        return 1;
        }


#ifdef DIAGNOSTICS
    printf ( "Just after get_info() call in get_spec_data() !\n" );
    if ( dump_VIS_DATA ( sdata, NULL, NULL ) ) return 2;
#endif /* DIAGNOSTICS */


    /* set the type of slice and miscellaneous clamps
       NOTE:  kathy's routines are *1* based */
    sdata->selected_species = spec_index+1;
    sdata->col_min = colMin+1;
    sdata->col_max = colMax+1;
    sdata->row_min = rowMin+1;
    sdata->row_max = rowMax+1;
    sdata->level_min = levelMin+1;
    sdata->level_max = levelMax+1;
    sdata->step_incr = stepIncr[caseChar-'A'];
    if ( ( thisHour >= 0 ) && ( !dt ) ) /* then we have a case of '<spec>:<step>', in
                     which case we want to get data for time step
                     thisHour regardless of what the hour settings are */
        {
        s = stepMin[caseChar-'A'] + thisHour - 1;
        if ( sliceType == XYTSLICE )  sdata->slice = XYSLICE;
        else if ( sliceType == YZTSLICE )  sdata->slice = YZSLICE;
        else if ( sliceType == XZTSLICE )  sdata->slice = XZSLICE;
        else if ( sliceType == XYZTSLICE ) sdata->slice = XYZSLICE;
        else
            sdata->slice = sliceType;
        }
    else
        {
        s = stepMin[caseChar-'A'] + selectedStep - 1;
        sdata->slice = sliceType;
        }

    smin = stepMin[caseChar-'A'] + *hrMin;
    smax = stepMin[caseChar-'A'] + *hrMax;

    /* HACK HACK HACK */
    /* Removing this limitation AME 4/15/2004
     *if (s > 30000 || smin > 30000 || smax > 30000 )
     *  {
     *  fprintf(stderr, "==================================================\n");
     *  fprintf(stderr, "You've GOT to be kidding with these time specs !\n");
     *  fprintf(stderr, "Now invoking HACK in get_spec_data() because\n");
     *  fprintf(stderr, "s==%d, smin==%d, smax==%d, I am\n",
     *           s, smin, smax);
     *  fprintf(stderr, "resetting them to s=1, smin=1, smax=24\n");
     *  fprintf(stderr, "And also I'll set sdata->step_incr to 1.\n");
     *  fprintf(stderr, "This painful, awful HACK gets around problems\n");
     *  fprintf(stderr, "with receiving PAVE instructions over the bus.\n");
     *  fprintf(stderr, "These instructions are not yet integrated\n");
     *  fprintf(stderr, "with the newly designed PAVE Objects\n");
     *  fprintf(stderr, "==================================================\n");
     *  s=1;
     *  smin=1;
     *  smax=24;
     *  sdata->step_incr = 1;
     *  HACK=1;
     *  }
     */

    if ( smax > sdata->step_max )
        {
        fprintf ( stderr, "File '%s' only goes to step %d"
                  " so I'm resetting hrMax to %d\n",
                  sdata->filename, sdata->step_max,
                  sdata->step_max-smin );
        smax = sdata->step_max;
        *hrMax = smax-smin;
        }

    if ( (    s < sdata->step_min       ) ||
            (    s > sdata->step_max       ) ||
            ( smin < sdata->step_min       ) ||
            ( smax > sdata->step_max       ) ||
            ( smin > smax                  ) ||
            ( (   s < stepMin[caseChar-'A'] ) && ( HACK!=1 ) ) ||
            ( (   s > stepMax[caseChar-'A'] ) && ( HACK!=1 ) ) ||
            ( ( smin < stepMin[caseChar-'A'] ) && ( HACK!=1 ) ) ||
            ( ( smin > stepMax[caseChar-'A'] ) && ( HACK!=1 ) ) ||
            ( ( smax < stepMin[caseChar-'A'] ) && ( HACK!=1 ) ) ||
            ( ( smax > stepMax[caseChar-'A'] ) && ( HACK!=1 ) ) )
        {
        sprintf ( tstring, "Bad hours in get_spec_data()!\n"
                  "  sdata->filename == '%s'\n"
                  "     selectedStep == %3d (1 based) - from calling process\n"
                  "                s == %3d (offset from step_min, 0 based)\n"
                  "             smin == %3d (offset from step_min, 0 based)\n"
                  "             smax == %3d (offset from step_min, 0 based)\n"
                  "  sdata->step_min == %3d (1 based) - created internally\n"
                  "  sdata->step_max == %3d (1 based) - created internally\n"
                  "     stepMin['%c'] == %3d (1 based) - from calling process\n"
                  "     stepMax['%c'] == %3d (1 based) - from calling process\n",
                  sdata->filename,
                  selectedStep,
                  s,
                  smin,
                  smax,
                  sdata->step_min,
                  sdata->step_max,
                  ( int ) ( caseChar ), stepMin[caseChar-'A'],
                  ( int ) ( caseChar ), stepMax[caseChar-'A'] );
        myFreeVis ( sdata );
        return errmsg ( tstring );
        }
    sdata->selected_step = s;
    sdata->selected_row = selectRow;
    sdata->selected_col = selectCol;
    sdata->selected_level = selectLevel;

    if ( ( sliceType == XYTSLICE ) ||
            ( sliceType == YZTSLICE ) ||
            ( sliceType == XZTSLICE ) ||
            ( sliceType == XYZTSLICE ) )
        {
        sdata->step_min = smin;
        sdata->step_max = smax;
        }
    else
        {
        sdata->step_min = s;
        sdata->step_max = s;
        }
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Just before get_data() call in get_spec_data !\n" );
    fflush ( stderr );
    dump_VIS_DATA ( sdata, NULL, NULL );
    fflush ( stderr );
    fflush ( stdout );
#endif /* DIAGNOSTICS */

    t = get_data ( bd, sdata, errorString );
#ifdef DIAGNOSTICS
    fprintf ( stderr, "get_data returned %d\n", t ); /* SRT 951026 */
#endif /* DIAGNOSTICS */
    if ( !t )
        {
        myFreeVis ( sdata );
        if ( !errorString[0] )
            sprintf ( errorString, "get_data() failed in retrieveData.c!\n" );
        return 1;
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Just after get_data() call in get_spec_data !\n" );
    fflush ( stderr );
    if ( dump_VIS_DATA ( sdata, NULL, NULL ) )
        {
        myFreeVis ( sdata );
        return 1;
        }
    fflush ( stderr );
    fflush ( stdout );
#endif /* DIAGNOSTICS */

    if ( ( thisHour >= 0 ) && ( !dt ) )
        /* then we have a case of '<spec>:<step>', in
                   which case we have gotten data for time step
                   thisHour regardless of what the hour settings
                   are.  Now we have to replicate that single
           hour's worth of data (*hrMax-*hrMin+1) times
           if the entire formula is getting data for
           multiple hours, in order to have enough data
           to operate on with the rest of the formula */
        {
        sdata->selected_step = stepMin[caseChar-'A'] + selectedStep - 1;
        sdata->step_min = smin;
        sdata->step_max = smax;

#ifdef DIAGNOSTICS
        fprintf ( stderr, "Just reset sdata->selected_step to %d\n",
                  sdata->selected_step );
        fprintf ( stderr, "Just reset sdata->step_min to %d\n",
                  sdata->step_min );
        fprintf ( stderr, "Just reset sdata->step_max to %d\n",
                  sdata->step_max );
#endif /* DIAGNOSTICS */

        if ( ( sliceType == XYTSLICE ) ||
                ( sliceType == YZTSLICE ) ||
                ( sliceType == XZTSLICE ) ||
                ( sliceType == XYZTSLICE ) )
            {
            char *tf;
            size_t msize;
            int *newsdate = ( int * ) NULL, *newstime = ( int * ) NULL;

            if ( smin < smax )
                {
                int dateorig = sdata->sdate ? sdata->sdate[0] : 0,
                    timeorig = sdata->stime ? sdata->stime[0] : 0;
                msize = sizeof ( int ) * ( smax-smin+1 );
                if ( ( ! ( newsdate = malloc ( msize ) ) ) ||
                        ( ! ( newstime = malloc ( msize ) ) ) )
                    {
                    if ( newsdate ) free ( ( void * ) newsdate );
                    if ( newstime ) free ( ( void * ) newstime );
                    myFreeVis ( sdata );
                    return errmsg ( mem_msg );
                    }
                for ( i = smin; i <= smax; i++ )
                    {
                    newsdate[i-smin] = dateorig;
                    newstime[i-smin] = timeorig;
                    }
                if ( sdata->sdate ) free ( sdata->sdate );
                sdata->sdate = newsdate;
                if ( sdata->stime ) free ( sdata->stime );
                sdata->stime = newstime;
                }
            sdata->slice = sliceType;
            msize = ( stack->vdata.col_max - stack->vdata.col_min + 1 ) *
                    ( stack->vdata.row_max - stack->vdata.row_min + 1 ) *
                    ( stack->vdata.level_max - stack->vdata.level_min + 1 ) *
                    ( smax - smin + 1 ) *
                    sizeof ( float );
            if ( ( tf = ( char * ) malloc ( msize ) ) == NULL )
                {
                myFreeVis ( sdata );
                return errmsg ( mem_msg );
                }
            msize = ( stack->vdata.col_max - stack->vdata.col_min + 1 ) *
                    ( stack->vdata.row_max - stack->vdata.row_min + 1 ) *
                    ( stack->vdata.level_max - stack->vdata.level_min + 1 ) *
                    sizeof ( float );
            for ( i = smin; i <= smax; i++ )
                memcpy ( &tf[msize* ( i-smin )], ( char * ) sdata->grid, msize );
            free ( sdata->grid );
            sdata->grid = ( float * ) tf;
            tf = NULL;
            }
        }

    /*  if we're doing change with respect to time... */
    if ( dt )
        {
        int   ni, nj, nk, nsteps ;
        float newmin, newmax, val1, val0, val;

        /* calculate the deltas and the new grid_min and grid_max*/
        ni     = sdata->col_max-sdata->col_min+1 ;
        nj     = sdata->row_max-sdata->row_min+1 ;
        nk     = sdata->level_max-sdata->level_min+1 ;
        nsteps = sdata->step_max-sdata->step_min+1 ;
        newmin = sdata->grid[INDEX ( 0,0,0,1,ni,nj,nk )] -
                 sdata->grid[INDEX ( 0,0,0,0,ni,nj,nk )] ;
        newmax = newmin ;
        for ( h = 0; h < nsteps-1; h++ )
            for ( k = 0; k < nk; k++ )
                for ( j = 0; j < nj; j++ )
                    for ( i = 0; i < ni; i++ )
                        {
                        val0 = sdata->grid[INDEX ( i,j,k,h,  ni,nj,nk )];
                        val1 = sdata->grid[INDEX ( i,j,k,h+1,ni,nj,nk )];
                        sdata->grid[INDEX ( i,j,k,h,  ni,nj,nk )] = val = val1 - val0;
                        if ( val<newmin ) newmin=val;
                        if ( val>newmax ) newmax=val;
                        }

        /* adjust the appropriate variables in the *sdata structure */
        sdata->grid_min = newmin;
        sdata->grid_max = newmax;
        sdata->nstep-1;
        sdata->step_max--;
        ( *hrMax )--;
        }

    return 0;
    }



/************************************************************
PROCESS -   the main loop for the expression processor.
        This returns:

      0 if processing should continue

      1 if it has completed processing successfully;
        in which case the finished value will be
        pointed to by the top of the stack's
        stack_item

      2 if processing has encountered an error

    The basic algorithm for each iteration of process is to get
    the next atom (if there is one) from the postfix formula
    string.  If it is an operand, it allocates the necessary
    space for that operand, and pushes a stack_item for that
    operand onto the stack.  If it is an operator, it pops the
    top two items off of the stack, performs the operation, and
    pushes the result back onto the stack.  If there are no
    unprocessed atoms remaining in the formula and it was a
    correct postfix string, there will be a single stack_item on
    the stack corresponding to the result of the formula.
************************************************************/
static int process ( void )
    {
    char    atomType, c, tstring[512], tstring2[512],
            a1[255], *tcp2, caseName[255], *mp, hostName[255];
    float   *fp1, *fp2, *fp3, f, f1, f2, tf;
    struct  stack_item  *op1, *op2;
    int len, mylen, type1, type2, backflag = 0, i, sindex, thisKMAX,
                                  j, k, nCaseSpecs,
                                  ks, ke, ndts, t;
    long    ind, index, index1, index2;
    int     maxi, maxj, maxk, maxt, mini, minj, mink, mint, atom_type;
    float   mean, var, std_dev, grid_min, grid_max, sum;


    if ( cancelKeys() ) return ( 1+errmsg ( "cancel" ) );
    advance();
    len = strlen ( atom );
    if ( atom[0] == '\0' )
        if ( ( stack != NULL ) &&
                ( stack->sptr == NULL ) )
            {
            if ( stack->dtype )
                {
                char unit[255];

                /* Before returning, set the units,
                   species short name, species
                   long name, and nspecies.

                   Also get rid of the filename, if
                   it exists, because its really not
                   valid in the context of formulas */

                if ( ( stack->vdata.units_name != NULL ) &&
                        ( stack->vdata.units_name
                          [stack->vdata.selected_species-1] != NULL ) )
                    strcpy ( unit, stack->vdata.units_name
                             [stack->vdata.selected_species-1] );
                else
                    unit[0] = '\0';

                for ( i = 0; i < stack->vdata.nspecies; i++ )
                    {
                    if ( stack->vdata.species_short_name != NULL )
                        if ( stack->vdata.species_short_name[i] != NULL )
                            {
#ifdef MDIAGS
                            /*SRT*/printf ( "PROCESS free of spec_short_name[%d] == %s at %lu\n",
                                            i, stack->vdata.species_short_name[i],
                                            ( unsigned long ) stack->vdata.species_short_name[i] ); /*SRT*/
#endif /* MDIAGS */
                            free ( stack->vdata.species_short_name[i] );
                            stack->vdata.species_short_name[i] = NULL;
                            }

                    if ( stack->vdata.species_long_name != NULL )
                        if ( stack->vdata.species_long_name[i] != NULL )
                            {
                            free ( stack->vdata.species_long_name[i] );
                            stack->vdata.species_long_name[i] = NULL;
                            }

                    if ( stack->vdata.units_name != NULL )
                        if ( stack->vdata.units_name[i] != NULL )
                            {
                            free ( stack->vdata.units_name[i] );
                            stack->vdata.units_name[i] = NULL;
                            }
                    }

                if ( stack->vdata.species_short_name != NULL )
                    {
                    free ( stack->vdata.species_short_name );
                    stack->vdata.species_short_name = NULL;
                    }

                if ( stack->vdata.units_name != NULL )
                    {
                    free ( stack->vdata.units_name );
                    stack->vdata.units_name = NULL;
                    }

                if ( stack->vdata.species_long_name != NULL )
                    {
                    free ( stack->vdata.species_long_name );
                    stack->vdata.species_long_name = NULL;
                    }

                stack->vdata.selected_species = 1;
                stack->vdata.nspecies = 1;

                if ( stack->vdata.filename != NULL )
                    {
                    free ( stack->vdata.filename );
                    stack->vdata.filename = NULL;
                    }

                if ( stack->vdata.filehost.name != NULL )
                    {
                    free ( stack->vdata.filehost.name );
                    stack->vdata.filehost.name = NULL;
                    }

                if (
                    ( ( stack->vdata.units_name = ( char ** )
                                                  malloc ( sizeof ( char * ) ) ) == NULL )
                    ||
                    ( ( stack->vdata.species_short_name = ( char ** )
                            malloc ( sizeof ( char * ) ) ) == NULL )
                    ||
                    ( ( stack->vdata.species_long_name = ( char ** )
                            malloc ( sizeof ( char * ) ) ) == NULL )
                    ||
                    ( ( stack->vdata.units_name[0] =
                            strdup ( unit ) ) == NULL )
                    ||
                    ( ( stack->vdata.species_short_name[0] =
                            strdup ( infixFormula ) ) == NULL )
                    ||
                    ( ( stack->vdata.species_long_name[0] =
                            strdup ( infixFormula ) ) == NULL )
                )
                    return ( 1 + errmsg ( mem_msg ) );

                return 1;
                }

            /* SRT 961010 return (1 + errmsg("A formula must produce a grid of data !")); */
            sprintf ( tstring, "%s == %g ", infixFormula, stack->constant ); /* SRT 961010 */
            return ( 1 + errmsg ( tstring ) ); /* SRT 961010 */
            }
        else
            return ( 1 + errmsg ( "?? incorrect postfixqueue ??" ) );

    if ( ( ( strncasecmp ( atom, "sqrt", 4 ) == 0 ) && ( len == 4 ) ) ||
         ( ( strncasecmp ( atom, "sqr",  3 ) == 0 ) && ( len == 3 ) ) ||
         ( ( strncasecmp ( atom, "log",  3 ) == 0 ) && ( len == 3 ) ) ||
         ( ( strncasecmp ( atom, "minx", 4 ) == 0 ) && ( len == 4 ) ) ||
         ( ( strncasecmp ( atom, "miny", 4 ) == 0 ) && ( len == 4 ) ) ||
         ( ( strncasecmp ( atom, "minz", 4 ) == 0 ) && ( len == 4 ) ) ||
         ( ( strncasecmp ( atom, "maxx", 4 ) == 0 ) && ( len == 4 ) ) ||
         ( ( strncasecmp ( atom, "maxy", 4 ) == 0 ) && ( len == 4 ) ) ||
         ( ( strncasecmp ( atom, "maxz", 4 ) == 0 ) && ( len == 4 ) ) ||
         ( ( strncasecmp ( atom, "mean", 4 ) == 0 ) && ( len == 4 ) ) ||
         ( ( strncasecmp ( atom, "min",  3 ) == 0 ) && ( len == 3 ) ) ||
         ( ( strncasecmp ( atom, "max",  3 ) == 0 ) && ( len == 3 ) ) ||
         ( ( strncasecmp ( atom, "sum",  3 ) == 0 ) && ( len == 3 ) ) ||
         ( ( strncasecmp ( atom, "abs",  3 ) == 0 ) && ( len == 3 ) ) )
        {
        if ( stack->dtype == SARRPTR )
            {
            thisKMAX = KMAX;
            ks = 0;
            ke = KMAX-1;
            }
        else
            {
            thisKMAX = 1;
            ks = 0;
            ke = 0;
            }
        }

    if      ( ( strncasecmp ( atom, "minx", 4 ) == 0 ) && ( len == 4 ) ) atom_type = MINX;
    else if ( ( strncasecmp ( atom, "miny", 4 ) == 0 ) && ( len == 4 ) ) atom_type = MINY;
    else if ( ( strncasecmp ( atom, "minz", 4 ) == 0 ) && ( len == 4 ) ) atom_type = MINZ;
    else if ( ( strncasecmp ( atom, "maxx", 4 ) == 0 ) && ( len == 4 ) ) atom_type = MAXX;
    else if ( ( strncasecmp ( atom, "maxy", 4 ) == 0 ) && ( len == 4 ) ) atom_type = MAXY;
    else if ( ( strncasecmp ( atom, "maxz", 4 ) == 0 ) && ( len == 4 ) ) atom_type = MAXZ;
    else if ( ( strncasecmp ( atom, "mint", 4 ) == 0 ) && ( len == 4 ) ) atom_type = MINT;
    else if ( ( strncasecmp ( atom, "maxt", 4 ) == 0 ) && ( len == 4 ) ) atom_type = MAXT;
    else if ( ( strncasecmp ( atom, "mean", 4 ) == 0 ) && ( len == 4 ) ) atom_type = MEAN;
    else if ( ( strncasecmp ( atom, "sum",  3 ) == 0 ) && ( len == 3 ) ) atom_type = SUM;
    else
        atom_type = NO_TYPE;

    if ( atom_type != NO_TYPE )
        {
        if ( stack == NULL )
            return ( 1 + errmsg (
                         "Nothing to take minx/maxx/miny/maxy/minz/maxz/mint/maxt/mean/"
                         "min/max/sum of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            {
            switch ( atom_type )
                {
                case MINX:
                case MINY:
                case MINZ:
                case MINT:
                case MAXX:
                case MAXY:
                case MAXZ:
                case MAXT:
                    stack->constant = 1.0;
                    break;

                    /* otherwise its MEAN MIN, MAX, or SUM so it can stay the same */
                default:
                    break;
                }
            }
        else
            {
            if ( calc_stats ( &stack->vdata, percents, whichLevel,
                              -1, ( int ) TMAX,
                              &maxi, &maxj, &maxk, &maxt,
                              &mini, &minj, &mink, &mint,
                              &grid_min, &grid_max,
                              &mean, &var, &std_dev, &sum ) )
                return 1;
            myFreeVis ( &stack->vdata );
            stack->dtype = FLTPTR;
            switch ( atom_type )
                {
                case MINX:
                    stack->constant = mini;
                    break;
                case MINY:
                    stack->constant = minj;
                    break;
                case MINZ:
                    stack->constant = mink;
                    break;
                case MINT:
                    stack->constant = mint;
                    break;
                case MAXX:
                    stack->constant = maxi;
                    break;
                case MAXY:
                    stack->constant = maxj;
                    break;
                case MAXZ:
                    stack->constant = maxk;
                    break;
                case MAXT:
                    stack->constant = maxt;
                    break;
                case MEAN:
                    stack->constant = mean;
                    break;
                case SUM:
                    stack->constant = sum;
                    break;
                default:
                    return errmsg ( "Unknown atom_type in process()" );
                }
            }
        }
    else if ( ( strncasecmp ( atom, "min", 3 ) == 0 ) && ( len == 3 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take max of on stack!" ) );
        if ( stack->dtype != FLTPTR ) /* if it IS a FLTPTR, we do nothing */
            {
            float   val, vmin ;
            float   v2d[JMAX][IMAX] ;
            long    ind ;
            int     m ;
            for ( k = ks; k <= ke; k++ )
                {
                for ( m = 0, t = 0; t < TMAX; t++ )     /* find MAX{ grid(t,k,,j,i): t = 0:TMAX-1 } */
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            if ( percents[ INDEX( i,j,0,0,IMAX,JMAX,1 ) ] )
                                {
                                ind = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX ) ;
                                val = stack->vdata.grid[ ind ] ;
                                if ( isnanf ( val ) ) continue;
                                if ( !m )
                                    {
                                    v2d[j][i] = val ;
                                    m = 1 ;
                                    }
                                else if ( val < v2d[j][i] )
                                    {
                                    v2d[j][i] = val ;
                                    }
                                }

                if ( !m ) return 1;

                for ( t = 0; t < TMAX; t++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            if ( percents[ INDEX( i,j,0,0,IMAX,JMAX,1 ) ] )
                                {
                                ind = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX ) ;
                                stack->vdata.grid[ ind ] = v2d[j][i] ;
                                }
                }
            }
        }
    else if ( ( strncasecmp ( atom, "max", 3 ) == 0 ) && ( len == 3 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take max of on stack!" ) );
        if ( stack->dtype != FLTPTR ) /* if it IS a FLTPTR, we do nothing */
            {
            float   val, vmax ;
            float   v2d[JMAX][IMAX] ;
            long    ind ;
            int     m ;
            for ( k = ks; k <= ke; k++ )
                {
                for ( m = 0, t = 0; t < TMAX; t++ )     /* find MAX{ grid(t,k,j,i): t = 0:TMAX-1 } */
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            if ( percents[ INDEX( i,j,0,0,IMAX,JMAX,1 ) ] )
                                {
                                ind = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX ) ;
                                val = stack->vdata.grid[ ind ] ;
                                if ( isnanf ( val ) ) continue;
                                if ( !m )
                                    {
                                    v2d[j][i] = val ;
                                    m = 1 ;
                                    }
                                else if ( val > v2d[j][i] )
                                    {
                                    v2d[j][i] = val ;
                                    }
                                }

                if ( !m ) return 1 ;

                for ( t = 0; t < TMAX; t++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            if ( percents[ INDEX( i,j,0,0,IMAX,JMAX,1 ) ] )
                                {
                                ind = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX ) ;
                                stack->vdata.grid[ ind ] = v2d[j][i] ;
                                }
                }
            }
        }
    else if ( ( strncasecmp ( atom, "sqrt", 4 ) == 0 ) && ( len == 4 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take sqrt of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            {
            if ( stack->constant < 0.0 )
                return ( 1 + errmsg ( "Can't take sqrt of a negative!" ) );
            else
                stack->constant = sqrt ( ( double ) stack->constant );
            }
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );

                            if ( stack->vdata.grid[index] < 0.0 )
                                {
                                if ( percents[INDEX ( i,j,1,0,IMAX,JMAX,1 )] )
                                    return ( 1 + errmsg ( "Can't take sqrt of a negative!" ) );
                                }
                            else
                                stack->vdata.grid[index] =
                                    sqrt ( ( double ) stack->vdata.grid[index] );
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "sqr", 3 ) == 0 ) && ( len == 3 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take sqr of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            stack->constant *= stack->constant;
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                            stack->vdata.grid[index] *= stack->vdata.grid[index];
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "sin", 3 ) == 0 ) && ( len == 3 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take sin of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            stack->constant = sin ( ( double ) stack->constant );
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                            stack->vdata.grid[index] =
                                sin ( ( double ) stack->vdata.grid[index] );
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "cos", 3 ) == 0 ) && ( len == 3 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take cos of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            stack->constant = cos ( ( double ) stack->constant );
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                            stack->vdata.grid[index] =
                                cos ( ( double ) stack->vdata.grid[index] );
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "tan", 3 ) == 0 ) && ( len == 3 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take tan of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            stack->constant = tan ( ( double ) stack->constant );
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                            stack->vdata.grid[index] =
                                tan ( ( double ) stack->vdata.grid[index] );
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "sind", 4 ) == 0 ) && ( len == 4 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take sind of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            stack->constant = sin ( ( double ) stack->constant* ( M_PI/180.0 ) );
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                            stack->vdata.grid[index] =
                                sin ( ( double ) stack->vdata.grid[index]* ( M_PI/180.0 ) );
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "cosd", 4 ) == 0 ) && ( len == 4 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take cosd of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            stack->constant = cos ( ( double ) stack->constant* ( M_PI/180.0 ) );
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                           {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                            stack->vdata.grid[index] =
                                cos ( ( double ) stack->vdata.grid[index]* ( M_PI/180.0 ) );
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "tand", 4 ) == 0 ) && ( len == 4 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take tand of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            stack->constant = tan ( ( double ) stack->constant* ( M_PI/180.0 ) );
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                            stack->vdata.grid[index] =
                                tan ( ( double ) stack->vdata.grid[index]* ( M_PI/180.0 ) );
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "exp", 3 ) == 0 ) && ( len == 3 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take exp of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            stack->constant = exp ( ( double ) stack->constant );
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                            stack->vdata.grid[index] =
                                exp ( ( double ) stack->vdata.grid[index] );
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "log", 3 ) == 0 ) && ( len == 3 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take log of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            {
            if ( stack->constant <= 0.0 )
                return ( 1 + errmsg ( "Can't take log of a nonpositive!" ) );
            stack->constant = log10 ( ( double ) stack->constant );
            }
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                            if ( stack->vdata.grid[index] <= 0.0 )
                                {
#ifdef DIAGNOSTICS
                                fprintf ( stderr,
                                          "i=%d,j=%d,k=%d,t=%d,"
                                          "IMAX=%d,JMAX=%d,KMAX=%d,"
                                          "TMAX=%d,index=%ld"
                                          "grid[index]=%f\n",
                                          i,j,k,t,IMAX,JMAX,KMAX,TMAX,
                                          ( long ) index,stack->vdata.grid[index] );
#endif /* DIAGNOSTICS */
                                if ( percents[INDEX ( i,j,1,0,IMAX,JMAX,1 )] )
                                    return ( 1 +
                                             errmsg ( "Can't take log of a nonpositive!" ) );
                                }
                            else
                                stack->vdata.grid[index] =
                                    log10 ( ( double ) stack->vdata.grid[index] );
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "ln", 2 ) == 0 ) && ( len == 2 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take ln of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            {
            if ( stack->constant <= 0.0 )
                return ( 1 + errmsg ( "Can't take ln of a nonpositive!" ) );
            stack->constant = log ( ( double ) stack->constant );
            }
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                            if ( stack->vdata.grid[index] <= 0.0 )
                                {
                                if ( percents[INDEX ( i,j,1,0,IMAX,JMAX,1 )] )
                                    return ( 1 + errmsg ( "Can't take ln of a nonpositive!" ) );
                                }
                            else
                                stack->vdata.grid[index] =
                                    log ( ( double ) stack->vdata.grid[index] );
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "abs", 3 ) == 0 ) && ( len == 3 ) )
        {
        if ( stack == NULL )
            return ( 1 + errmsg ( "Nothing to take abs of on stack!" ) );
        if ( stack->dtype == FLTPTR )
            stack->constant = fabs ( ( double ) stack->constant );
        else
            {
            for ( t = 0; t < TMAX; t++ )
                for ( k = ks; k <= ke; k++ )
                    for ( j = 0; j < JMAX; j++ )
                        for ( i = 0; i < IMAX; i++ )
                            {
                            index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                            stack->vdata.grid[index] =
                                fabs ( ( double ) stack->vdata.grid[index] );
                            }
            }
        }
    else if ( ( strncasecmp ( atom, "pi", 2 ) == 0 ) && ( len == 2 ) )
        {
        if ( push() ) return 2;
        stack->dtype = FLTPTR;
        stack->constant = M_PI;
        }
    else if ( ( strncasecmp ( atom, "e", 1 ) == 0 ) && ( len == 1 ) )
        {
        if ( push() ) return 2;
        stack->dtype = FLTPTR;
        stack->constant = M_E;
        }
    else if ( ( strncasecmp ( atom, "ncols", 5 ) == 0 ) && ( len == 5 ) )
        {
        if ( push() ) return 2;
        stack->dtype = FLTPTR;
        stack->constant = IMAX;
        }
    else if ( ( strncasecmp ( atom, "nrows", 5 ) == 0 ) && ( len == 5 ) )
        {
        if ( push() ) return 2;
        stack->dtype = FLTPTR;
        stack->constant = JMAX;
        }
    else if ( ( strncasecmp ( atom, "nlevels", 7 ) == 0 ) && ( len == 7 ) )
        {
        if ( push() ) return 2;
        stack->dtype = FLTPTR;
        stack->constant = KMAX;
        }
    else
        {
        switch ( atomType = atom[0] )
            {
            case 'T':
                if ( push() ) return 2;
                stack->dtype = SARRPTR;
                stack->vdata.selected_species = 1;
                stack->vdata.nspecies = 1;
                stack->vdata.ncol = IMAX;
                stack->vdata.nrow = JMAX;
                stack->vdata.nlevel = KMAX;
                stack->vdata.slice = XYZSLICE;
                if (
                    ( ( stack->vdata.units_name = ( char ** )
                                                  malloc ( sizeof ( char * ) ) ) == NULL )
                    ||
                    ( ( stack->vdata.species_short_name = ( char ** )
                            malloc ( sizeof ( char * ) ) ) == NULL )
                    ||
                    ( ( stack->vdata.species_long_name = ( char ** )
                            malloc ( sizeof ( char * ) ) ) == NULL )
                    ||
                    ( ( stack->vdata.units_name[0] =
                            strdup ( "\0" ) ) == NULL )
                    ||
                    ( ( stack->vdata.species_short_name[0] =
                            strdup ( "sigmaVals" ) ) == NULL )
                    ||
                    ( ( stack->vdata.species_long_name[0] =
                            strdup ( "sigmaVals" ) ) == NULL )
                    ||
                    ( ( stack->vdata.grid = ( float * )
                                            malloc ( ( size_t ) ARRSIZE ) ) == NULL )
                )
                    return ( 1 + errmsg ( mem_msg ) );

                for ( t = 0; t < TMAX; t++ )
                    for ( k = ks; k <= ke; k++ )
                        for ( j = 0; j < JMAX; j++ )
                            for ( i = 0; i < IMAX; i++ )
                                stack->vdata.grid[INDEX ( i,j,k,t,IMAX,JMAX,KMAX )] =
                                    sigmaVals[k];
                break;


            case 'C':
                if ( push() ) return 2;
                stack->dtype = FLTPTR;
                stack->constant = atof ( &atom[1] );
                break;

            case 'D':
            case 'S':   /* put an item on the stack for this species */
                if ( push() ) return 2;

                /* check for special case of rate of change or
                   a specific hour attached to this species  */
                dt = 0;
                if ( ( tcp2 = strchr ( atom, ( int ) ':' ) ) == NULL )
                    thisHour = -1;
                else if ( ( toupper ( atom[len-3] ) == ':' ) &&
                          ( toupper ( atom[len-2] ) == 'D' ) &&
                          ( toupper ( atom[len-1] ) == 'T' ) &&
                          ( toupper ( atom[len] ) == '\0' ) )
                    {
                    thisHour = selectedStep;
                    dt = 1;
                    atom[len-3] = '\0';
                    len -= 3;
                    }
                else
                    {
                    thisHour = ( int ) atof ( tcp2+1 );
                    *tcp2 = '\0';
                    }

                /* some quick setting up */
                strcpy ( a1, &atom[1] );
                thisKMAX = KMAX;
                if ( atomType == 'D' )
                    thisKMAX = 1;
                sindex = atoi ( a1 );

                /* figure out which case index we're using */
                c = toupper ( atom[strlen ( atom ) - 1] );
                if ( ( c < 'A' ) || ( c >= ( 'A' + ncases ) ) )
                    {
                    strcpy ( tstring, "Case   Not Available" );
                    tstring[5] = tolower ( c );
                    return 1+errmsg ( tstring );
                    }

                /* get the information for that case if
                   we don't already have it */
                if ( caseInfo[c-'A'].filename == NULL )
                    {
                    if ( ( getNthItem ( ( int ) ( c-'A'+1 ), caseList, caseName ) )
                            ||
                            ( ! ( caseInfo[c-'A'].filename =
                                      ( char * ) malloc ( strlen ( caseName )+1 ) ) ) )
                        {
                        sprintf ( tstring,"Can't get fname for case %c !", c );
                        return 1+errmsg ( tstring );
                        }
                    strcpy ( caseInfo[c-'A'].filename, caseName );

                    if ( ( getNthItem ( ( int ) ( c-'A'+1 ), hostList, hostName ) )
                            ||
                            ( ! ( caseInfo[c-'A'].filehost.name =
                                      ( char * ) malloc ( strlen ( hostName )+1 ) ) ) )
                        {
                        sprintf ( tstring, "Can't get hostName for case %c !", c );
                        return 1+errmsg ( tstring );
                        }
                    strcpy ( caseInfo[c-'A'].filehost.name, hostName );

                    if ( !get_info ( bd, &caseInfo[c-'A'], errorString ) )
                        return 2;

#ifdef DIAGNOSTICS
                    printf ( "Just after get_info() call in process() !\n" );
                    if ( dump_VIS_DATA ( &caseInfo[c-'A'], NULL, NULL ) )
                        return 2;
#endif /* DIAGNOSTICS */
                    /* convert to netCDF (really, IO/API map_info
                       information) data if it isn't already */
                    if ( makeSureIts_netCDF ( &caseInfo[c-'A'], errorString ) )
                        return 2;

                    if ( ( caseInfo[c-'A'].ncol   != fullIMAX ) ||
                            ( caseInfo[c-'A'].nrow   != fullJMAX ) )
                        {
                        sprintf ( tstring, "Case %c dims don't match IMAX, JMAX, KMAX !", c );
                        return 1+errmsg ( tstring );
                        }
                    }

                /* make sure caseName and hostName are
                   right for this species */
                strcpy ( caseName, caseInfo[c-'A'].filename );
                strcpy ( hostName, caseInfo[c-'A'].filehost.name );

                /* allocate space for this species data */
                /* change to allocate only dep if needed */
                if ( atomType == 'S' )
                    stack->dtype = SARRPTR;
                else /* if (atomType == 'D') */
                    stack->dtype = DARRPTR;

                /* Get number of specs in case's file */
                nCaseSpecs = caseInfo[c-'A'].nspecies;
                if ( ( nCaseSpecs <= 0 ) || ( nCaseSpecs > MAXPAVESPECS ) )
                    {
                    sprintf ( tstring, "%d invalid NSPECS for '%s' !!",
                              nCaseSpecs, caseInfo[c-'A'].filename );
                    return ( 1 + errmsg ( tstring ) );
                    }
                if ( ( sindex < 0 ) || ( sindex >= nCaseSpecs ) )
                    {
                    itoa ( sindex, tstring );
                    strcat ( tstring, " is a bad spec index for '" );
                    strcat ( tstring, caseInfo[c-'A'].filename );
                    strcat ( tstring, "'!!" );
                    return ( 1 + errmsg ( tstring ) );
                    }

                if ( get_spec_data  ( caseName,
                                      hostName,
                                      c,
                                      sindex,
                                      &stack->vdata,
                                      thisKMAX ) )
                    return 2;

                break;

            case '|':
            case '&':
            case '>':
            case '<':
            case 'g':
            case 'l':
            case '=':
            case '!':
            case 'p':
            case '+':
            case '-':
            case '/':
            case '*':
                if ( ( op2 = pop() ) == NULL )
                    return ( 1+errmsg ( "retrieveData couldn't find expected stack item" ) );
                if ( ( op1 = pop() ) == NULL )
                    {
                    if ( op2->dtype ) myFreeVis ( &op2->vdata );
                    free ( op2 );
                    op2 = NULL;
                    return ( 1 + errmsg ( "retrieveData couldn't find expected stack item" ) );
                    }
                if ( push() )
                    {
                    if ( ( op1->dtype ) ) myFreeVis ( &op1->vdata );
                    if ( ( op2->dtype ) ) myFreeVis ( &op2->vdata );
                    free ( op1 );
                    op1 = NULL;
                    free ( op2 );
                    op2 = NULL;
                    return 2;
                    }
                type1 = op1->dtype;
                type2 = op2->dtype;
                if ( ( type1 == SARRPTR ) || ( type2 == SARRPTR ) )
                    stack->dtype = SARRPTR;
                else
                    stack->dtype = DARRPTR;

                if ( ( type1 == FLTPTR ) && ( type2 == FLTPTR ) )   /* we're dealing with two constants */
                    {
                    stack->dtype = FLTPTR;
                    switch ( atomType )
                        {
                        case 'p':
                            if ( ( op1->constant < 0.0 ) &&
                                    ( ! ( integral ( op2->constant ) ) ) )
                                {
                                sprintf ( tstring,
                                          "Can't do %g**%g since %g<0.0"
                                          " and %g is non-integral!",
                                          op1->constant, op2->constant,
                                          op1->constant, op2->constant );
                                return 1+errmsg ( tstring );
                                }
                            stack->constant =
                                pow ( ( double ) op1->constant,
                                      ( double ) op2->constant );
                            break;

                        case '*':
                            stack->constant = op1->constant
                                              * op2->constant;
                            break;

                        case '|':
                            stack->constant = ( op1->constant != 0.0 ) ||
                                              ( op2->constant != 0.0 );
                            break;

                        case '&':
                            stack->constant = ( op1->constant != 0.0 ) &&
                                              ( op2->constant != 0.0 );
                            break;

                        case '>':
                            stack->constant = op1->constant > op2->constant;
                            break;

                        case '<':
                            stack->constant = op1->constant < op2->constant;
                            break;

                        case 'g':
                            stack->constant = op1->constant >= op2->constant;
                            break;

                        case 'l':
                            stack->constant = op1->constant <= op2->constant;
                            break;

                        case '=':
                            stack->constant = op1->constant == op2->constant;
                            break;

                        case '!':
                            stack->constant = op1->constant != op2->constant;
                            break;

                        case '/':
                            if ( flor )
                                {
                                tf = op2->constant;
                                if ( tf < 0.0 ) tf *= -1.0;
                                if ( tf <= floorCut )
                                    stack->constant = 0.0;
                                else
                                    stack->constant = op1->constant /
                                                      op2->constant;
                                }
                            else if ( op2->constant == 0.0 )
                                {
                                free ( op1 );
                                op1 = NULL;
                                free ( op2 );
                                op2 = NULL;
                                return 1+errmsg (
                                           "Divide by zero error!" );
                                }
                            else
                                stack->constant = op1->constant
                                                  / op2->constant;
                            break;

                        case '+':
                            stack->constant = op1->constant +
                                              op2->constant;
                            break;

                        case '-':
                            stack->constant = op1->constant -
                                              op2->constant;
                            break;
                        }
                    }       /*  we're dealing with just 2 constants */

                else /* we're NOT dealing with just 2 constants */
                    {

                    /* SRT 960924 memory
                       SRT 960924 memory
                       SRT 960924 memory    INSTEAD OF:
                       SRT 960924 memory    setting up the current stack's VIS_DATA with
                       SRT 960924 memory    template info from one of the ops, doing a malloc
                       SRT 960924 memory    and then putting the results in the malloc'ed space
                       SRT 960924 memory
                       SRT 960924 memory    NOW WE:
                       SRT 960924 memory    do the arithmetic in place on one of hte op's grids,
                       SRT 960924 memory    so we don't have to do yet another huge malloc.
                       SRT 960924 memory    Then we simply memcpy the op's info to the stack,
                       SRT 960924 memory    and then memset the op's bytes to all NULL.
                       SRT 960924 memory
                       SRT 960924 memory    */

                    /* SRT 960924 memory      long mallocSize;          */
                    VIS_DATA *tv;

                    if ( stack->dtype == SARRPTR )
                        {
                        /* SRT 960924 memory        mallocSize = ARRSIZE;       */
                        thisKMAX = KMAX;
                        ks = 0;
                        ke = KMAX-1;
                        /* SRT 960924 memory */     tv = ( type1 == SARRPTR ) ? &op1->vdata : &op2->vdata;
                        }
                    else
                        {
                        /* SRT 960924 memory        mallocSize = DEPARRSIZE;    */
                        thisKMAX = 1;
                        ks = ke = 0;
                        /* SRT 960924 memory */     tv = ( type1 == DARRPTR ) ? &op1->vdata : &op2->vdata;
                        }

#ifdef MDIAGS
                    printf ( "IMAX=%d,JMAX=%d,KMAX=%d,hrMin=%d,hrMax=%d,TMAX=%d\n",
                             ( int ) IMAX, ( int ) JMAX, ( int ) KMAX, ( int ) *hrMin,
                             ( int ) *hrMax, ( int ) TMAX );
#endif /* MDIAGS */

#ifdef SQUAT    /* SRT 960924 memory */
                    if ( type1 ) tv = &op1->vdata;
                    else tv = &op2->vdata;
                    memcpy ( &stack->vdata, tv, sizeof ( VIS_DATA ) );
                    stack->vdata.selected_species = 1;
                    stack->vdata.nspecies = 1;
                    stack->vdata.filename = NULL;
                    stack->vdata.filehost.name = NULL;
                    stack->vdata.species_short_name = NULL;
                    stack->vdata.species_long_name = NULL;
                    stack->vdata.units_name = NULL;
                    stack->vdata.map_info = NULL;
                    stack->vdata.data_label = NULL;
                    stack->vdata.grid = NULL;
                    stack->vdata.sdate = NULL;
                    stack->vdata.stime = NULL;
                    mylen = 1+strlen ( tv->map_info );
                    mp = ( char * ) malloc ( ( size_t ) mylen );
                    stack->vdata.map_info = mp;
                    mylen = ( tv->data_label ) ? 1+strlen ( tv->data_label ) :1;
                    mp = ( char * ) malloc ( ( size_t ) mylen );
                    stack->vdata.data_label = mp;
                    stack->vdata.units_name = ( char ** )
                                              malloc ( sizeof ( char * ) );
                    ndts = ( int ) TMAX;
                    stack->vdata.sdate = ( int * ) malloc ( ndts*sizeof ( int ) );
                    stack->vdata.stime = ( int * ) malloc ( ndts*sizeof ( int ) );
                    if ( ( stack->vdata.units_name == NULL ) ||
                         ( stack->vdata.sdate == NULL ) ||
                         ( stack->vdata.map_info == NULL ) ||
                         ( stack->vdata.data_label == NULL ) ||
                         ( stack->vdata.stime == NULL ) )
                        {
                        if ( op1->dtype ) myFreeVis ( &op1->vdata );
                        if ( op2->dtype ) myFreeVis ( &op2->vdata );
                        free ( op1 );
                        op1 = NULL;
                        free ( op2 );
                        op2 = NULL;
                        return ( 1 + errmsg ( mem_msg ) );
                        }
                    memcpy ( stack->vdata.sdate, tv->sdate, ndts*sizeof ( int ) );
                    memcpy ( stack->vdata.stime, tv->stime, ndts*sizeof ( int ) );
                    strcpy ( stack->vdata.map_info, tv->map_info );
                    strcpy ( stack->vdata.data_label,
                             tv->data_label ? tv->data_label : "\0" );
#endif /* ifdef SQUAT */    /* SRT 960924 memory */
                    /* SRT 960924 memory */   ndts = ( int ) TMAX;

                    if ( ( type1 ) && ( type2 ) )
                        {
                        /* take care of units name */
                        if ( atomType == '/' )
                            {
                             if ( tv->units_name[0] ) free ( tv->units_name[0] );
                             tv->units_name[0] = strdup ( "\0" );
                            }
                        else if ( strcasecmp ( op1->vdata.units_name[ op1->vdata.selected_species-1 ],
                                               op2->vdata.units_name[ op2->vdata.selected_species-1 ] ) )
                            {
                            if ( tv->units_name[0] ) free ( tv->units_name[0] );
                            tv->units_name[0] = strdup ( "\0" );
                            }
                        else
                            {
                            strcpy ( tstring, tv->units_name[tv->selected_species-1] );
                            if ( tv->units_name[0] ) free ( tv->units_name[0] );
                            tv->units_name[0] = strdup ( tstring );
                            }

                        /* verify map_info's both match */
                        if ( ! map_infos_areReasonablyEquivalent( op1->vdata.map_info, op2->vdata.map_info, tstring ) )
                            {
                            fprintf ( stderr,
                                      "\nWARNING: '%s'\n"
                                      "Combining variables with map_info\n"
                                      "strings that are not equivalent.  This may not\n"
                                      "make sense, but I'll give it a try with no map.\n"
                                      "The two different map_info's are\n'%s'\nand\n'%s'\n",
                                      op1->vdata.map_info, op2->vdata.map_info );
                            free ( op1->vdata.map_info );
                            op1->vdata.map_info=NULL;
                            free ( op2->vdata.map_info );
                            op2->vdata.map_info=NULL;
                            op1->vdata.map_info = strdup ( BOGUS_MAP_INFO );
                            op2->vdata.map_info = strdup ( BOGUS_MAP_INFO );
                            if ( ( !op1->vdata.map_info ) || ( !op2->vdata.map_info ) )
                                {
                                if ( op1->dtype ) myFreeVis ( &op1->vdata );
                                if ( op2->dtype ) myFreeVis ( &op2->vdata );
                                free ( op1 );
                                op1 = NULL;
                                free ( op2 );
                                op2 = NULL;
                                return ( 1 + errmsg ( mem_msg ) );
                                }
                            }

                        /* do data_label's both match? */
                        if ( ( !op1->vdata.data_label ) ||
                             ( !op2->vdata.data_label ) ||
                             ( strcasecmp ( op1->vdata.data_label,
                                            op2->vdata.data_label ) ) )
                            {
                            if ( op1->vdata.data_label ) free ( op1->vdata.data_label );
                            op1->vdata.data_label = strdup ( "\0" );
                            if ( op2->vdata.data_label ) free ( op2->vdata.data_label );
                            op2->vdata.data_label = strdup ( "\0" );
                            }

                        /* do the start date/time's match? */
                        if ( memcmp ( op1->vdata.sdate, op2->vdata.sdate,
                                      ( size_t ) ndts*sizeof ( int ) )
                                ||
                                memcmp ( op1->vdata.stime, op2->vdata.stime,
                                         ( size_t ) ndts*sizeof ( int ) ) )
                            {
                            fprintf ( stderr, "Start date/times don't match;"
                                      " will label using time step number\n" );
                            memset ( ( void * ) tv->stime, 0, ( size_t ) ndts*sizeof ( int ) );
                            memset ( ( void * ) tv->sdate, 0, ( size_t ) ndts*sizeof ( int ) );
                            }
                        }
                    else
                        {
                        strcpy ( tstring, tv->units_name[tv->selected_species-1] );
                        if ( tv->units_name[0] ) free ( tv->units_name[0] );
                        tv->units_name[0] = strdup ( tstring );
                        }


                    fp3 = tv->grid;

                    if ( ( type1 && !type2 ) || ( !type1 && type2 ) )
                        {
                        if ( type2 == FLTPTR ) /* we hope to do FLT <operator>
                                                    ARR, but this is reversed */
                            {
                            if (  ( atomType == '-' ) ||
                                  ( atomType == '/' ) ||
                                  ( atomType == 'p' ) ||
                                  ( atomType == '>' ) ||
                                  ( atomType == '<' ) ||
                                  ( atomType == 'g' ) ||
                                  ( atomType == 'l' )
                               )
                                backflag = 1;
                            f   = op2->constant;
                            fp2 = op1->vdata.grid;
                            }
                        else
                            {
                            f   = op1->constant;
                            fp2 = op2->vdata.grid;
                            }

                        /* actually compute the new array */                        

                        switch ( atomType )
                            {
                            case '-':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = f;
                                                    }
                                                else
                                                    {
                                                    f1 = f;
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = f1 - f2;
                                                }                                
                                break;

                            case '+':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                fp3[index] = f + fp2[index] ;
                                                }
                                break;

                            case '*':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                fp3[index] = f * fp2[index] ;
                                                }
                                break;

                            case '/':
                                if ( flor )
                                    {
                                    tf = f2;
                                    if ( tf < 0.0 ) tf *= -1.0;
                                    if ( tf <= floorCut )
                                        fp3[index] = 0.0;
                                    else
                                        fp3[index] = f1 / f2;
                                    }
                                else if ( f2 == 0 )
                                    {
                                    if ( op1->dtype )
                                        myFreeVis ( &op1->vdata );
                                    if ( op2->dtype )
                                        myFreeVis ( &op2->vdata );
                                    free ( op1 );
                                    op1 = NULL;
                                    free ( op2 );
                                    op2 = NULL;
                                    return 1+errmsg
                                           ( "Divide by zero error!" );
                                    }
                                else
                                    fp3[index] = f1 / f2;
                                break;

                            case '&':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                f1 = fp2[index];
                                                f2 = f;
                                                fp3[index] = ( f1 != 0.0 ) && ( f2 != 0.0 );
                                                }  
                                break;

                            case '|':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                f1 = fp2[index];
                                                f2 = f;
                                                fp3[index] = ( f1 != 0.0 ) || ( f2 != 0.0 );
                                                }
                                break;

                            case 'p':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = f;
                                                    }
                                                else
                                                    {
                                                    f1 = f;
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = ( f1 != 0.0 ) || ( f2 != 0.0 );
                                                if ( ( f1 < 0.0 ) &&
                                                        ( ! ( integral ( ( double ) f2 ) ) ) )
                                                    {
                                                    sprintf ( tstring,
                                                              "Can't do %g**%g since %g<0.0 "
                                                              "and %g is non-integral!",
                                                              ( double ) f1, ( double ) f2,
                                                              ( double ) f1, ( double ) f2 );
                                                    return 1+errmsg ( tstring );
                                                    }
                                                fp3[index] = pow ( ( double ) f1,
                                                                   ( double ) f2 );
                                                }
                                break;

                            case '>':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = f;
                                                    }
                                                else
                                                    {
                                                    f1 = f;
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = ( f1 > f2 );
                                                }
                                break;

                            case '<':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = f;
                                                    }
                                                else
                                                    {
                                                    f1 = f;
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = ( f1 < f2 );
                                            }
                                break;

                            case 'g':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = f;
                                                    }
                                                else
                                                    {
                                                    f1 = f;
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = ( f1 >= f2 );
                                                }
                                break;

                            case 'l':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = f;
                                                    }
                                                else
                                                    {
                                                    f1 = f;
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = ( f1 <= f2 );
                                                }
                                break;

                            case '=':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                f1 = fp2[index];
                                                f2 = f;
                                                fp3[index] = ( f1 == f2 );
                                                }
                                break;

                            case '!':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                f1 = fp2[index];
                                                f2 = f;
                                                fp3[index] = ( f1 != f2 );
                                                }
                                break;

                            } /* switch */


                        } /* if ( (type1 && !type2) || (!type1 && type2) ) */

                    else                    /* else both types are arrays */
                        {
                        fp1 = op1->vdata.grid;
                        fp2 = op2->vdata.grid;

                        /* Added 960404 as per Alison Eyth */
                        if ( op1->vdata.nlevel != op2->vdata.nlevel )
                            {
                            if ( ( op1->vdata.level_max-op1->vdata.level_min )
                                    !=
                                 ( op2->vdata.level_max-op2->vdata.level_min ) )
                                {
                                sprintf ( tstring,
                                          "ERROR: You are trying to mix 2 variables"
                                          "with (level_max-level_min) not equal"
                                          " (%d and %d)\n",
                                          op1->vdata.level_max-op1->vdata.level_min,
                                          op2->vdata.level_max-op2->vdata.level_min );
                                if ( op1->dtype ) myFreeVis ( &op1->vdata );
                                if ( op2->dtype ) myFreeVis ( &op2->vdata );
                                free ( op1 );
                                op1 = NULL;
                                free ( op2 );
                                op2 = NULL;
                                return 1+errmsg ( tstring );
                                }
                            else
                                {
                                fprintf ( stderr,
                                          "WARNING: You are mixing 2 variables"
                                          "from datasets with different KMAX "
                                          " (%d and %d)\n"
                                          "Are you sure you want to do this?\n\n",
                                          op1->vdata.nlevel, op2->vdata.nlevel );
                                }
                            }
                        if ( type2 == FLTPTR ) /* we hope to do FLT <operator>
                                                    ARR, but this is reversed */
                            {
                            if (  ( atomType == '-' ) ||
                                  ( atomType == '/' ) ||
                                  ( atomType == 'p' ) ||
                                  ( atomType == '>' ) ||
                                  ( atomType == '<' ) ||
                                  ( atomType == 'g' ) ||
                                  ( atomType == 'l' )
                               )
                                backflag = 1;
                            f   = op2->constant;
                            fp2 = op1->vdata.grid;
                            }
                        else
                            {
                            f   = op1->constant;
                            fp2 = op2->vdata.grid;
                            }

                        /* actually compute the new array */                        

                        switch ( atomType )
                            {
                            case '-':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = fp1[index];
                                                    }
                                                else
                                                    {
                                                    f1 = fp1[index];
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = f1 - f2;
                                                }                                
                                break;

                            case '+':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                f1 = fp2[index];
                                                f2 = fp1[index];
                                                fp3[index] = f1 + f2 ;
                                                }
                                break;

                            case '*':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                f1 = fp2[index];
                                                f2 = fp1[index];
                                                fp3[index] = f1 * f2 ;
                                                }
                                break;

                            case '/':
                                if ( flor )
                                    {
                                    tf = f2;
                                    if ( tf < 0.0 ) tf *= -1.0;
                                    if ( tf <= floorCut )
                                        fp3[index] = 0.0;
                                    else
                                        fp3[index] = f1 / f2;
                                    }
                                else if ( f2 == 0 )
                                    {
                                    if ( op1->dtype )
                                        myFreeVis ( &op1->vdata );
                                    if ( op2->dtype )
                                        myFreeVis ( &op2->vdata );
                                    free ( op1 );
                                    op1 = NULL;
                                    free ( op2 );
                                    op2 = NULL;
                                    return 1+errmsg
                                           ( "Divide by zero error!" );
                                    }
                                else
                                    fp3[index] = f1 / f2;
                                break;

                            case '&':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                f1 = fp2[index];
                                                f2 = fp1[index];
                                                fp3[index] = ( f1 != 0.0 ) && ( f2 != 0.0 );
                                                }  
                                break;

                            case '|':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                f1 = fp2[index];
                                                f2 = fp1[index];
                                                fp3[index] = ( f1 != 0.0 ) || ( f2 != 0.0 );
                                                }
                                break;

                            case 'p':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = fp1[index];
                                                    }
                                                else
                                                    {
                                                    f1 = fp1[index];
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = ( f1 != 0.0 ) || ( f2 != 0.0 );
                                                if ( ( f1 < 0.0 ) &&
                                                        ( ! ( integral ( ( double ) f2 ) ) ) )
                                                    {
                                                    sprintf ( tstring,
                                                              "Can't do %g**%g since %g<0.0 "
                                                              "and %g is non-integral!",
                                                              ( double ) f1, ( double ) f2,
                                                              ( double ) f1, ( double ) f2 );
                                                    return 1+errmsg ( tstring );
                                                    }
                                                fp3[index] = pow ( ( double ) f1,
                                                                   ( double ) f2 );
                                                }
                                break;

                            case '>':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = fp1[index];
                                                    }
                                                else
                                                    {
                                                    f1 = fp1[index];
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = ( f1 > f2 );
                                                }
                                break;

                            case '<':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = fp1[index];
                                                    }
                                                else
                                                    {
                                                    f1 = fp1[index];
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = ( f1 < f2 );
                                            }
                                break;

                            case 'g':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = fp1[index];
                                                    }
                                                else
                                                    {
                                                    f1 = fp1[index];
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = ( f1 >= f2 );
                                                }
                                break;

                            case 'l':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                if ( backflag )
                                                    {
                                                    f1 = fp2[index];
                                                    f2 = fp1[index];
                                                    }
                                                else
                                                    {
                                                    f1 = fp1[index];
                                                    f2 = fp2[index];
                                                    }
                                                fp3[index] = ( f1 <= f2 );
                                                }
                                break;

                            case '=':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                f1 = fp2[index];
                                                f2 = fp1[index];
                                                fp3[index] = ( f1 == f2 );
                                                }
                                break;

                            case '!':
                                for ( t = 0; t < TMAX; t++ )
                                    for ( k = ks; k <= ke; k++ )
                                        for ( j = 0; j < JMAX; j++ )
                                            for ( i = 0; i < IMAX; i++ )
                                                {
                                                index = INDEX ( i,j,k,t,IMAX,JMAX,thisKMAX );
                                                f1 = fp2[index];
                                                f2 = fp1[index];
                                                fp3[index] = ( f1 != f2 );
                                                }
                                break;

                            } /* switch */


                        }       /* if one is a scalar; else if both are arrays  */

                    /* SRT 960924 memory      Now move the in-place data to the stack */
                    /* SRT 960924 memory */   tv->selected_species = 1;
                    /* SRT 960924 memory */   memcpy ( &stack->vdata, tv, sizeof ( VIS_DATA ) );
                    /* SRT 960924 memory */   memset ( tv, 0, sizeof ( VIS_DATA ) );

                    } /* we're NOT dealing with just 2 constants */


                /* free the popped stack items' space */
                if ( op1->dtype )
                    myFreeVis ( &op1->vdata );
                if ( op2->dtype )
                    myFreeVis ( &op2->vdata );
                free ( op1 );
                op1 = NULL;
                free ( op2 );
                op2 = NULL;
                break;


            default:
                strcpy ( tstring, "atomType=' ' in process()" );
                tstring[9] = atomType;
                return ( 1 + errmsg ( tstring ) );
            }
        }

    return 0;
    }




/************************************************************
RETRIEVEDATA - returns 1 if error
************************************************************/
/*
   NOTES ON TIME STEP ARGUMENTS TO retrieveData():

         step_min[], step_max[], & step_incr[] are
         *1* based, and are specific to each
         *dataset* == *case* (NOT a formula).
         Typically these would be set to the maximum
         range for each case, although for some
         situations the user may wish to clamp the
         range down somewhat by adjusting these.

         selectedStepP is *1* based, and is specific
         to this *formula* (which may have multiple
         cases within it). It is used as an offset to
         the step_min for each case in a formula, and
         matters only for slice_type's XYSLICE, YZSLICE, XZSLICE,
         and XYZSLICE with integration NOT TIME_INT or ZvsT_INT.

         hrMin & hrMax are *0* based, and
         are specific to this *formula* (which may
         have multiple cases within it).  hrMin == 0
         refers to time_step step_min[<that case>],
         hrMin == 1 to time_step step_min[<that case>]+1,
         and so on.

         for each species within a formula,
         hrMin & hrMax are used as clamps within
         that case's step_min & step_max

         hrMin & hrMax are used for slice_types
         XYTSLICE, YZTSLICE, XZTSLICE, and XYZTSLICE and for integration
         values of TIME_INT or ZvsT_INT.

         Typically hrMin & hrMax could be set up this way
         (although you may want to allow the user to
          "clamp down" the range calculated by this algorithm):

         Assuming you have n cases used by a formula
         [the cases used by a formula can be determined
         by the caseUsed argument which is filled up by
         parseFormula()].  A good choice for hrMin and hrMax
         would be to allow the maximum time step range
         possible.  For each case(i)  used in a formula
         check out the step_min & step_max for that case, and
         determine nsteps(i)= step_max-step_min+1 for that case.
         Now determine nsteps(small) = the smallest of
         the nsteps(i).  Set hrMin = 0, and
         hrMax = nsteps(small)-1
*/

int retrieveData    (

        /* INPUTS to retrieveData */

        int  I_MAX,           /* imax for formula returned
                                             by parseFormula() */

        int  J_MAX,           /* jmax for formula returned
                                             by parseFormula() */

        int  K_MAX,           /* kmax for formula returned
                                             by parseFormula() */

        char *infixFormulaP,  /* infix formula - will be
                                             stuffed into the VIS_DATA
                                             struct returned by
                                             retrieveData() */

        char *postFixQueueP,  /* postfix version of the formula -
                                             required to retrieve the data;
                                             should be in the form returned
                                             by parseFormula() */

        char *caseListP,      /* a list of cases (ie data file
                                             names) in order (a..?),
                                             separated by commas */

        char *hostListP,      /* a list of hosts (ie
                                             "todd.hpcc.epa.gov,ozone,
                                             flyer.ncsc.org") separated by
                                             commas, one for each case in
                                             caseList */

        struct BusData *bdP,  /* needed to communicate with
                                             the SW Bus; this should already
                                             have been initialized with
                                             initVisDataClient() */

        int   selectedStepP,  /* what timestep we are getting
                         data for?

                         NOTES:  this is 1 based !!

                         This is used as
                         an offset to the step_min
                         for each case.

                         This matters only for slice_type's
                         XYSLICE, YZSLICE, XZSLICE, and XYZSLICE */

        int   integration,    /* should be one of the following:
                                             NO_INT, ALL_INT, TIME_INT, ZvsT_INT,
                         or SCATTER_INT (all are defined
                                             in retrieveData.h) */

        float thickValues[],  /* NOTE:  retrieveData MODIFIES THE
                         CONTENTS OF THICKVALUES in place !

                         this is an array of K_MAX floats
                         indicating the relative weights
                                             to give each of the 0..K_MAX-1
                                             layers when doing an integration
                                             (such as time series).  For
                                             example, if you have a 15 layer
                                             RADM run, you might stuff the
                                             following numbers into thickValues:
                                             (.01,.01,.02,.03,.04,.05,.06,.08,
                                              .1,.1,.1,.1,.1,.1,.1)

                For formulas with "sigma" in it, thickValues will
                be used to calculate sigma using the scheme:

                    E = the summation of thickList[i], i = 0 to K_MAX-1

                sigma[K_MAX-1] = 0.5 * thickValues[K_MAX-1]/E

                        sigma[X] = sigma[X+1] + 0.5 *
                       (thickValues[X+1] + thickValues[X]) / E */


        int   whichLevelP[],  /* this is an array of K_MAX integers,
                                             each element should be non-zero
                                             if that level is in the domain
                                             of interest, otherwise 0
                         NOTE: RETRIEVE DATA MODIFIES
                         THE WHICHLEVELP IN PLACE !!!! */

        int   use_floor,      /* if use_floor is non-zero,
                         then divisions will be checked
                                             to avoid divide by zero conditions.
                                             if (use_floor) then for each
                         division, if the denonimator is
                         less than or equal to floorCut
                         then the result of the divide is
                         set to 0 */

        float floorCut_,      /* see use_floor description above */

        char  percentsP[],    /* this is an array indicating which
                         cells are in the domain and which
                         are turned off.  It is indexed
                         [i+j*I_MAX] (where i ranges 0..I_MAX-1,
                         and j ranges 0..J_MAX-1).  For a
                         grid cell completely in the domain,
                         the corresponding percent's value
                         should be 100, for a cell completely
                         NOT in the domain, it should be 0.
                         For a cell which should only count
                         partially, use a value between 0
                         and 100 */

        int   selected_col,   /* which column selected, 1-based */

        int   selected_row,   /* which row selected, 1-based */

        int   selected_level, /* which level selected, 1-based */

        int   step_min[],     /* one step_min for each dataset
                         listed in caseListP argument,
                         1-based (these will probably be
                         passed as 1's most of the time);

                         NOTES: (selectedStep == 1) corresponds
                         to a given dataset's step_min;
                         (hrMin == 0) also corresponds
                         to a given dataset's step_min

                         This will probably be set in
                         the UI to "clamp" datasets down
                         to time periods of interest */

        int   step_max[],     /* one step_max for each dataset
                         listed in caseListP argument,
                         1-based (most of the time these
                         will probably be passed as the
                         max number of steps in each
                         dataset)

                         NOTE: This will probably be set in
                                             the UI to "clamp" datasets down
                                             to time periods of interest */

        int   step_incr[],    /* one step_incr for each dataset
                         listed in caseListP argument,
                         1-based (most of the time these
                         will probably be passed as 1's)

                                             NOTE: This will probably be set in
                                             the UI to "clamp" datasets down
                                             to time increments of interest,
                         for example, to synchronize two
                         datasets in a formula whose
                         step increments differ */

        int  slice_type,      /* should be one of:

                             XYSLICE - selected_level at step hourf

                             YZSLICE - selected_column at step hourf

                             XZSLICE - selected_row at step hourf

                             XYZSLICE - all levels at step hourf

                             XYTSLICE - selected_level for steps
                               *hrMinP thru *hrMaxP

                             YZTSLICE - selected_col for steps
                               *hrMinP thru *hrMaxP

                             XZTSLICE - selected_row for steps
                               *hrMinP thru *hrMaxP

                             XYZTSLICE - all levels for steps
                               *hrMinP thru *hrMaxP

                         NOTE: these are defined in vis_data.h
                          */



        /* INPUTS to retrieveData, but possibly
           modified by retrieveData */

        int   *hrMinP,        /* first timestep (if time series data);
                         this is used as an offset to the
                         step_min for each case.  So in
                         effect, step_min[] and step_max[]
                         clamp a dataset down once, then
                         *hrMinP and *hrMaxP clamp it down
                         again, *within* the step_min-
                         step_max clamp.

                                             NOTES : this is *0* based and
                         and only matters for slice_type's
                                             XYTSLICE, YZTSLICE, XZTSLICE, and XYZTSLICE */

        int   *hrMaxP,        /* last timestep (if time series data);
                                             this may be modified to the
                                             highest available hour if data
                                             files don't go up this high. So in
                                             effect, step_min[] and step_max[]
                                             clamp a dataset down once, then
                                             *hrMinP and *hrMaxP clamp it down
                                             again, *within* the step_min-
                                             step_max clamp.

                                             NOTES : this is *0* based and
                                             and only matters for slice_type's
                                             XYTSLICE, YZTSLICE, XZTSLICE, and XYZTSLICE */

        /* MODIFIED BY retrieveData, in addition to
           whichLevelP (see above) */

        float  *tsdata,      /*  if doing time series or ZvsT
                                             integration (integration ==
                                             TIME_INT or ZvsT_INT) then the
                                             resulting data will be stuffed here.
                                             There should be enough space already
                                             allocated to hold this many floats:

                                               *hrMaxP - *hrMinP + 1
                                                    (for time series)

                                             or

                                                K_MAX * (*hrMaxP - *hrMinP + 1)
                                                    (for ZvsT)

                                             if doing scatter integration
                                             (integration == SCATTER_INT) then
                                             the data will be stuffed here,
                                             and tsdata[] should have at least
                                             1+I_MAX*J_MAX*K_MAX*(*hrMax - *hrMin+1)
                                             points in it.  The format of the
                                             data returned will be tsdata[0] ==
                                             npoints, and tsdata[1]...
                                             tsdata[npoints] holds the scatter
                                             data.

                                             if NOT doing timeSeries, ZvsT, or
                                             scatter integration, then this
                                             argument is ignored */

        VIS_DATA *vdata,      /* resulting data will be placed
                         here; this code will allocate
                         the necessary space for the
                         dynamically allocated values
                         pointed to by elements of
                         VIS_DATA structs - ITS UP
                         TO THE CALLING ROUTINE TO FREE
                         THESE !!!!! Note, if doing timeSeries,
                         ZvsT, or total integration
                         if (integration == ALL_INT), then
                         this argument is ignored  */

        float *tot_value,     /* if integration == ALL_INT then
                         this will receive the result.
                         If integration != ALL_INT then
                         this argument is ignored */

        char *errString       /* space for errormsg, if any */
    )

    {
    int     returnval = 1,
            i, j, k, n, t, h,
            pos = -1,
            thisKMAX = K_MAX;
    float       sigmaValues[MAXPAVESPECS],
                tf,
                E;

    char        totalInt = 0,
                timeSeries = 0,
                ZvsT = 0,
                scatterInt = 0,
                someLevel=0,
                tstring[512],
                tstring2[512],
                numOfCases[256];

    errorString = errString;  /* need to set errorString up for errmsg routine */
    if ( errString <= ( char * ) NULL )
        return errmsg ( "INVALID errString passed to retrieveData()!" );

#ifdef DIAGNOSTICS
    printf ( "Enter retrieveData\n" );
    printf ( "I_MAX == %d\n",I_MAX );
    printf ( "J_MAX == %d\n",J_MAX );
    printf ( "K_MAX == %d\n",K_MAX );
    printf ( "infixFormulaP == %s\n",infixFormulaP );
    printf ( "postFixQueueP == %s\n",postFixQueueP );
    printf ( "caseListP == %s\n",caseListP );
    printf ( "hostListP == %s\n",hostListP );
    printf ( "bdP == %ld\n", ( long ) bdP );
    printf ( "selectedStepP == %d\n", selectedStepP );
    printf ( "   integration == %d\n", integration );
    printf ( " thickValues[] == %lu\n", ( unsigned long ) thickValues );
    printf ( " whichLevelP[] == %lu\n", ( unsigned long ) whichLevelP );
    printf ( "   use_floor = %d\n", use_floor );
    printf ( " floorCut_ = %f\n", floorCut_ );
    printf ( "  percentsP[] == %lu\n", ( unsigned long ) percentsP );
    printf ( "  selected_col == %d\n", selected_col );
    printf ( "  selected_row == %d\n", selected_row );
    printf ( "  selected_level == %d\n", selected_level );
    printf ( "  step_min[] == %lu\n", ( unsigned long ) step_min );
    printf ( "  step_max[] == %lu\n", ( unsigned long ) step_max );
    printf ( "  step_incr[] == %lu\n", ( unsigned long ) step_incr );
    printf ( "  slice_type == %d == ", slice_type );
    switch ( slice_type )
        {
        case XYTSLICE:
            printf ( "XYT\n" );
            break;
        case YZTSLICE:
            printf ( "YZT\n" );
            break;
        case XYZTSLICE:
            printf ( "XYZT\n" );
            break;
        case XZTSLICE:
            printf ( "XZT\n" );
            break;
        case XYSLICE:
            printf ( "XY\n" );
            break;
        case YZSLICE:
            printf ( "YZ\n" );
            break;
        case XZSLICE:
            printf ( "XZ\n" );
            break;
        case XYZSLICE:
            printf ( "XYZ\n" );
            break;
        default:
            printf ( "???\n" );
            break;
        };
    printf ( "   *hrMinP == %d\n", *hrMinP );
    printf ( "   *hrMaxP = %d\n", *hrMaxP );
    printf ( "  tsdata[] == %lu\n", ( unsigned long ) tsdata );
    printf ( " *vdata == %lu\n", ( unsigned long ) vdata );
    printf ( " *tot_value == %lu\n", ( unsigned long ) tot_value );
    printf ( " *errString == %lu\n", ( unsigned long ) errString );
#endif /* DIAGNOSTICS */

    if ( I_MAX < 1 ) return errmsg ( "Bad I_MAX ( < 1 ) in retrieveData() !" );
    if ( J_MAX < 1 ) return errmsg ( "Bad J_MAX ( < 1 ) in retrieveData() !" );
    if ( K_MAX < 1 ) return errmsg ( "Bad K_MAX ( < 1 ) in retrieveData() !" );
    if ( infixFormulaP <= ( char * ) NULL )
        return errmsg ( "INVALID infixFormulaP passed to retrieveData()!" );
    if ( postFixQueueP <= ( char * ) NULL )
        return errmsg ( "INVALID postFixQueueP passed to retrieveData()!" );
    if ( postFixQueueP[0] == '\0' )
        return errmsg ( "INVALID (empty) postFixQueueP passed to retrieveData()!" );
    if ( caseListP <= ( char * ) NULL )
        return errmsg ( "INVALID caseListP passed to retrieveData()!" );
    if ( hostListP <= ( char * ) NULL )
        return errmsg ( "INVALID hostListP passed to retrieveData()!" );
    if ( bdP < ( struct BusData * ) NULL )
        return errmsg ( "INVALID bdP passed to retrieveData()!" );
    if ( bdP == NULL )
        fprintf ( stderr, "NULL bdP in retrieveData() !! Assuming local data\n" );
    if ( thickValues <= ( float * ) NULL )
        return errmsg ( "INVALID thickValues passed to retrieveData()!" );
    if ( whichLevelP <= ( int * ) NULL )
        return errmsg ( "INVALID whichLevelP passed to retrieveData()!" );
    if ( percentsP <= ( char * ) NULL )
        return errmsg ( "INVALID percentsP passed to retrieveData()!" );
    if ( step_min <= ( int * ) NULL )
        return errmsg ( "INVALID step_min passed to retrieveData()!" );
    if ( step_max <= ( int * ) NULL )
        return errmsg ( "INVALID step_max passed to retrieveData()!" );
    if ( step_incr <= ( int * ) NULL )
        return errmsg ( "INVALID stepIncr passed to retrieveData()!" );
    if ( hrMinP <= ( int * ) NULL )
        return errmsg ( "INVALID hrMinP passed to retrieveData()!" );
    if ( hrMaxP <= ( int * ) NULL )
        return errmsg ( "INVALID hrMaxP passed to retrieveData()!" );

    /* SRT removed 061495 - only needed for certain types of data retrieval,
       in which case its checked below anyway...
    if (tsdata <= (float *) NULL)
            return errmsg("INVALID tsdata passed to retrieveData()!");
    */

    if ( vdata <= ( VIS_DATA * ) NULL )
        return errmsg ( "INVALID vdata passed to retrieveData()!" );
    if ( ( integration != TIME_INT ) && ( integration != ZvsT_INT ) )
        if (    ( slice_type != XYSLICE ) &&
                ( slice_type != YZSLICE ) &&
                ( slice_type != XZSLICE ) &&
                ( slice_type != XYZSLICE ) &&
                ( slice_type != XYTSLICE ) &&
                ( slice_type != YZTSLICE ) &&
                ( slice_type != XZTSLICE ) &&
                ( slice_type != XYZTSLICE ) )
            return errmsg ( "slice_type argument INVALID in retrieveData()!" );

    whichLevel = whichLevelP;
    formula = postFixQueueP;
    infixFormula = infixFormulaP;
    errString[0] = '\0';
    fullIMAX = IMAX = I_MAX;
    fullJMAX = JMAX = J_MAX;
    fullKMAX = KMAX = K_MAX;
    caseList = caseListP;
    hostList = hostListP;
    bd = bdP;
    selectedStep = selectedStepP; /* which time step selected, 1-based */
    hrMin = hrMinP;
    hrMax = hrMaxP;
    selectCol = selected_col;     /* which column selected, 1-based */
    selectRow = selected_row;     /* which row selected, 1-based */
    selectLevel = selected_level; /* which level selected, 1-based */
    stepMin = step_min;           /* one step_min for each dataset */
    stepMax = step_max;           /* one step_max for each dataset */
    stepIncr = step_incr;         /* one step_incr for each dataset */
    sliceType = slice_type;       /* XYSLICE, YZSLICE, XZSLICE, XYZSLICE, XYTSLICE, YZTSLICE, XZTSLICE, or XYZTSLICE */

    if ( ( IMAX < 1 ) || ( JMAX < 1 ) || ( KMAX < 1 ) || ( KMAX > 512 ) )
        return errmsg ( "Bad IMAX, JMAX, or KMAX!" );

    /* make my own space for percents and copy percentsP there */
    if ( ( percents = ( char * ) malloc ( IMAX*JMAX ) ) == NULL )
        return errmsg ( "percents malloc() failed in retrieveData()!" );
    memcpy ( percents, percentsP, IMAX*JMAX );

    /* get the domain and range */
    if ( range_get ( percentsP, IMAX, JMAX, &colMin, &colMax, &rowMin, &rowMax ) )
        {
        free ( percents );
        percents = NULL;
        return 1;
        }

    levelMin = KMAX;
    levelMax = -1;
    for ( k = 0; k < KMAX; k++ )
        if ( whichLevel[k] )
            {
            someLevel = 1;
            if ( k < levelMin ) levelMin = k;
            if ( k > levelMax ) levelMax = k;
            }
    if ( someLevel != 1 )
        {
        free ( percents );
        percents = NULL;
        return errmsg ( "No levels selected !" );
        }

    /* get the time right */
    if ( ( sliceType != XYTSLICE ) &&
         ( sliceType != YZTSLICE ) &&
         ( sliceType != XZTSLICE ) &&
         ( sliceType != XYZTSLICE ) &&
         ( integration != TIME_INT ) &&
         ( integration != ZvsT_INT ) )
        *hrMinP = *hrMaxP = selectedStepP-1;

    if ( integration==TIME_INT || integration==ZvsT_INT )
        {
        if ( ( sliceType != XYTSLICE ) &&
             ( sliceType != YZTSLICE ) &&
             ( sliceType != XZTSLICE ) &&
             ( sliceType != XYZTSLICE ) )
            {
            free ( percents );
            percents = NULL;
            return errmsg ( "With integration==TIME_INT or ZvsT_INT "
                            "slice_type must be XYTSLICE, YZTSLICE, XZTSLICE, or XYZTSLICE !" );
            }
        }

    switch ( sliceType )
        {
            /* single level  */
        case XYTSLICE:
        case XYSLICE:
            if ( selectLevel < 1 || selectLevel > KMAX )
                {
                free ( percents );
                percents = NULL;
                return errmsg (
                           "select_level argument invalid in retrieveData()!" );
                }
            KMAX = 1;
            whichLevel[0] = 1;
            thickValues[0] = thickValues[selectLevel-1];
            IMAX = colMax-colMin+1;
            JMAX = rowMax-rowMin+1;
            for ( j = 0; j < JMAX; j++ )
                for ( i = 0; i < IMAX; i++ )
                    percents[INDEX ( i,j,0,0,IMAX,JMAX,1 )] =
                        percentsP[INDEX ( colMin+i,rowMin+j,0,0,
                                          fullIMAX,fullJMAX,1 )];
            break;

            /* single column */
        case YZTSLICE:
        case YZSLICE:
            if ( selectCol < 1 || selectCol > IMAX )
                {
                free ( percents );
                percents = NULL;
                return errmsg (
                           "select_column argument invalid in retrieveData()!" );
                }
            KMAX = levelMax-levelMin+1;
            for ( k = 0; k < KMAX; k++ )
                {
                whichLevel[k]  = whichLevel[levelMin+k];
                thickValues[k] = thickValues[levelMin+k];
                }
            IMAX = 1;
            JMAX = rowMax-rowMin+1;
            for ( j = 0; j < JMAX; j++ )
                percents[INDEX ( 0,j,0,0,IMAX,JMAX,1 )] =
                    percentsP[INDEX ( selectCol-1,j,0,0,fullIMAX,fullJMAX,1 )];
            break;


            /* single row  */
        case XZTSLICE:
        case XZSLICE:
            if ( selectRow < 1 || selectRow > JMAX )
                {
                free ( percents );
                percents = NULL;
                return errmsg (
                           "select_row argument invalid in retrieveData()!" );
                }
            KMAX = levelMax-levelMin+1;
            for ( k = 0; k < KMAX; k++ )
                {
                whichLevel[k]  = whichLevel[levelMin+k];
                thickValues[k] = thickValues[levelMin+k];
                }
            IMAX = colMax-colMin+1;
            JMAX = 1;
            for ( i = 0; i < IMAX; i++ )
                percents[INDEX ( i,0,0,0,IMAX,JMAX,1 )] =
                    percentsP[INDEX ( i,selectRow-1,0,0,fullIMAX,fullJMAX,1 )];
            break;


            /* all rows, columns, & levels */
        case XYZTSLICE:
        case XYZSLICE:
        default:
            KMAX = levelMax-levelMin+1;
            for ( k = 0; k < KMAX; k++ )
                {
                whichLevel[k]  = whichLevel[levelMin+k];
                thickValues[k] = thickValues[levelMin+k];
                }
            IMAX = colMax-colMin+1;
            JMAX = rowMax-rowMin+1;
            for ( j = 0; j < JMAX; j++ )
                for ( i = 0; i < IMAX; i++ )
                    percents[INDEX ( i,j,0,0,IMAX,JMAX,1 )] =
                        percentsP[INDEX ( colMin+i,rowMin+j,0,0,
                                          fullIMAX,fullJMAX,1 )];
            break;
        }


    /* how many cases do we have available? */
    ncases = 0;
    while ( !getNthItem ( ncases+1, caseList, tstring ) ) ncases++;

#ifdef DIAGNOSTICS
    for ( i = 0; i < ncases; i++ )
        {
        printf ( " stepMin['%c'] == %d ", 'A'+i, stepMin[i] );
        printf ( " stepMax['%c'] == %d ", 'A'+i, stepMax[i] );
        printf ( "stepIncr['%c'] == %d\n", 'A'+i, stepIncr[i] );
        }
#endif /* DIAGNOSTICS */


    /* allocate space to hold info on each case */
    if ( ( caseInfo = ( VIS_DATA * ) malloc ( ncases * sizeof ( VIS_DATA ) ) ) == NULL )
        {
        free ( percents );
        percents = NULL;
        return errmsg ( "caseInfo malloc() failed in retrieveData!" );
        }

    /* set case info records to all NULL values */
    memset ( ( void * ) caseInfo, 0, ( size_t ) ( ncases * sizeof ( VIS_DATA ) ) );


    totalInt = ( integration == ALL_INT );
    timeSeries = ( integration == TIME_INT );
    ZvsT = ( integration == ZvsT_INT );
    scatterInt = ( integration == SCATTER_INT );
    if ( use_floor )
        {
        flor = 1;
        if ( floorCut_ < 0.0 ) floorCut_ *= -1.0;
        }
    else
        {
        flor = 0;
        }
    floorCut = floorCut_;

    /*
    Now set up thickVals to be the proportion of each level to be
    used in integrations, and set up sigmaVals to be the correct
    sigma values, using the scheme, with array indeces in the HC
    user's point of view, starting at 1 not 0:

         E = the summation of thickList[i], i = 1 to KMAX

         sigma[KMAX] = 0.5 * thickList[KMAX]/E

         sigma[X] = sigma[X+1] + 0.5 * (thickList[X+1] + thickList[X]) / E
    */
    thickVals = &thickValues[0];
    sigmaVals = &sigmaValues[0];
    E = tf = 0.0;
    for ( i = 0; i < KMAX; i++ )
        {
        E += thickValues[i];
        if ( thickValues[i] < 0.0 ) thickValues[i] = 0.0;
        if ( whichLevel[i] ) tf += thickValues[i];
        }
    if ( tf <= 0.0 )
        {
        freeCaseInfo();
        free ( percents );
        percents = NULL;
        return errmsg ( "Bad thickValues and/or no levels selected!" );
        }
    sigmaValues[KMAX-1] = 0.5 * thickValues[KMAX-1] / E;
    for ( i = KMAX-2; i >= 0; i-- )
        sigmaValues[i] = sigmaValues[i+1] + 0.5 *
                         ( thickValues[i+1] + thickValues[i] ) / E;
    for ( i = 0; i < KMAX; i++ )
        if ( whichLevel[i] )
            thickValues[i] /= tf;
        else
            thickValues[i] = 0;

    if ( timeSeries || ZvsT )
        if ( ( *hrMin < 0 ) || ( *hrMin >= 9000 ) ||
                ( *hrMax < 0 ) || ( *hrMax >= 9000 ) ||
                ( *hrMin >= *hrMax ) )
            {
            freeCaseInfo();
            free ( percents );
            percents = NULL;
            return errmsg ( "Bad hour range!" );
            }


    /* dh 10/22/90 add one to max so one extra grid cell will be retrieved for
        use in fluxes
    if (imax<(IMAX-1)) ++imax;
    if (jmax<(JMAX-1)) ++jmax; SRT removed 1/26/95 */

    /* process until done with formula */
    fpos = 0;   /* set the initial position in the formula */
    stack = NULL;   /* initialize stack */

    if ( timeSeries || ZvsT )
        {
        if ( tsdata == NULL )
            {
            freeCaseInfo();
            free ( percents );
            percents = NULL;
            return errmsg (
                       "No tsdata space given for retrieveData() to store timeSeries/ZvsT data!" );
            }

        /* get all the data */
        while ( ! ( returnval = process() ) );

        if ( returnval == 1 ) /* we've successfully gotten data */
            for ( h = 0; ( ( h <= *hrMax - *hrMin ) && ( returnval == 1 ) ); h++ )
                {

                if ( stack->dtype == SARRPTR )
                    thisKMAX = KMAX;
                else
                    thisKMAX = 1;

                if ( timeSeries )
                    tsdata[h] = totalIntegration ( stack->vdata.grid,
                                                   thisKMAX, h );
                else if ( ZvsT )
                    {
                    if ( thisKMAX == 1 )
                        {
                        free_up();
                        freeCaseInfo();
                        free ( percents );
                        percents = NULL;
                        return errmsg ( "No Z vs T with 2d horizontal fields !" );
                        }
                    if ( fillZlevels ( stack->vdata.grid,
                                       &tsdata[KMAX* ( ( int ) h )], h ) )
                        {
                        free_up();
                        freeCaseInfo();
                        free ( percents );
                        percents = NULL;
                        return 1;
                        }
                    }
                }

        myFreeVis ( vdata );
        free_up();
        freeCaseInfo();
        free ( percents );
        percents = NULL;
        return ( returnval!=1 );
        }
    else
        {
        while ( ! ( returnval = process() ) );

        if ( stack != NULL )
            if ( scatterInt )

                {
                int   tdif, kdif, jdif, idif, indij, indijkt ;
                
                /* let's put the scatter data into tsdata */

                if ( tsdata == NULL )
                    {
                    freeCaseInfo();
                    free ( percents );
                    percents = NULL;
                    return errmsg ( "No tsdata space given for retrieveData() to store scatter data!" );
                    }


                tdif = stack->vdata.step_max  - stack->vdata.step_min  + 1 ;
                kdif = stack->vdata.level_max - stack->vdata.level_min + 1 ;
                jdif = stack->vdata.row_max   - stack->vdata.row_min   + 1 ;
                idif = stack->vdata.col_max   - stack->vdata.col_min   + 1 ;
                tsdata[0] = 0.0; /* tsdata[0] will hold npoints */

                for ( t = 0; t < tdif ; t++ )
                    {
                    for ( k = 0 ; k < kdif ; k++ )
                        {
                        if ( whichLevel[k] )
                            {
                            for ( j=0 ; j < jdif ; j++ )
                                for ( i=0; i < idif ; i++ )
                                    {
                                    indij = INDEX ( i, j, 0, 0, idif,jdif, 1 ) ;
                                    if ( percents[ indij ] )
                                        {
                                        indijkt = INDEX ( i,j,k,t, idif,jdif,kdif ) ;
                                        tsdata[0] += 1.0;
                                        tsdata[ ( int ) tsdata[0]] = stack->vdata.grid[ indijkt ];
                                        }
                                    }
                                }
                            }
                        }
                }

            else
                {
                /* let's put the answer into VIS_DATA argument */
                int   maxi, maxj, maxk, maxt, mini, minj, mink, mint;
                int   tdif, kdif, jdif, idif, indij, indijkt ;
                float mean, var, std_dev, sum;

                if ( stack->dtype == SARRPTR )
                    thisKMAX = KMAX;
                else
                    thisKMAX = 1;

#ifdef SQUAT
                /* one final pass over the data to put NaN's
                   in all the places where the percents array
                   was 0.  NOTE:  to check for this any
                   routines using this VIS_DATA struct
                   should use the isnanf() routine, and
                   also use #include <ieeefp.h>.  If isnanf()
                   returns non-zero then that data point should
                   be disregarded as it is not in the domain  */

                tdif = stack->vdata.step_max  - stack->vdata.step_min  + 1 ;
                kdif = stack->vdata.level_max - stack->vdata.level_min + 1 ;
                jdif = stack->vdata.row_max   - stack->vdata.row_min   + 1 ;
                idif = stack->vdata.col_max   - stack->vdata.col_min   + 1 ;
                for ( t = 0; t < tdif ; t++ )
                    {
                    for ( k = 0 ; k < kdif ; k++ )
                        {
                        if ( whichLevel[k] )
                            {
                            for ( j=0 ; j < jdif ; j++ )
                                for ( i=0; i < idif ; i++ )
                                    {
                                    indij = INDEX ( i, j, 0, 0, idif,jdif, 1 ) ;
                                    if ( ! percents[ indij ] )
                                        {
                                        indijkt = INDEX ( i,j,k,t, idif,jdif,kdif ) ;
                                        stack->vdata.grid[ indijkt ] = ieee_nan();
                                        }
                                    }
                                }
                            }
                        }
                        
#endif /* SQUAT */

                memcpy ( vdata, &stack->vdata, sizeof ( VIS_DATA ) );
                memset ( &stack->vdata, 0, sizeof ( VIS_DATA ) );
                if ( returnval == 1 )
                    returnval += calc_stats ( vdata, percents, whichLevel,
                                              -1, ( int ) TMAX,
                                              &maxi, &maxj, &maxk, &mint,
                                              &mini, &minj, &mink, &maxt,
                                              &vdata->grid_min, &vdata->grid_max,
                                              &mean, &var, &std_dev, &sum );
                }

#ifdef DIAGNOSTICS
        fprintf ( stderr,"About to return from retreiveData.c\n" );
        dump_VIS_DATA ( vdata, NULL, NULL );
#endif /* DIAGNOSTICS */

        free_up();
        }

    /*  if returnval is != 1 then there was an error */
    if ( returnval != 1 )
        {
        myFreeVis ( vdata );
        freeCaseInfo();
        free ( percents );
        percents = NULL;
        return 1;
        }

#ifndef USE_OLDMAP
    /*  now make sure M3IO params are correct; if not fixing them */
    /* SRT 9607120 bogus data shouldn't be "fixed" */  M3IO_parameter_fix ( vdata );
#endif /* USE_OLDMAP */

    if ( totalInt )
        {
        /* compute the total then exit */
        if ( tot_value <= ( float * ) NULL )
            {
            myFreeVis ( vdata );
            freeCaseInfo();
            free ( percents );
            percents = NULL;
            return errmsg ( "INVALID tot_value passed to retrieveData()!" );
            }

        *tot_value = totalIntegration ( vdata->grid, thisKMAX, -1 );
        myFreeVis ( vdata );
        freeCaseInfo();
        free ( percents );
        percents = NULL;
        return 0;
        }

    freeCaseInfo();
    free ( percents );
    percents = NULL;
    return 0;
    }
