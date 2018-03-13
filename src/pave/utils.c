/*#define DIAGNOSTICS*/
/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: utils.c 83 2018-03-12 19:24:33Z coats $
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
 *                           C R E D I T S
 *
 *   Development of this Software was supported in part through the
 *   MCNC-EPA cooperative agreement number CR822066-01.
 *
 *   Portions of this software were developed by the State University
 *   of New York at Albany, and funded by the United States Environmental
 *   Protection Agency under Contract 68D80016 with the Research Foundation
 *   of the State University of New York.
 ****************************************************************************
 *  FILE:           utils.c
 *  
 *  AUTHOR:         Steve Thorpe, MCNC Environmental Programs, 12/12/94
 *  
 *  PURPOSE:        Miscellaneous routines which will be used
 *                  by PAVE Analysis and Visualization code.
 *
 *  REVISION HISTORY
 *      950908 SRT added removeWhiteSpace() routine
 *      951218 SRT added registerCurrentTime() and verifyElapsedClockTime() routines
 *      960515 SRT added M3IO_parameter_fix()
 *      960517 SRT added dump_VIS_DATA_to_netCDF_file()
 *      961021 SRT added makeSureIts_netCDF()
 *      961021 SRT added is_reasonably_equal() and map_infos_areReasonablyEquivalent()
 *  
 ****************************************************************************/

#include <string.h>
#include <math.h>
#include "nan_incl.h"

#include "bts.h"

#include "iodecl3.h"            /* M3IO Library (Carlie Coats, MCNC) */


static int is_reasonably_equal ( double p, double q );
static  char e[256];
char    *errorString = e;



/************************************************************/
float setNaNf ( void )
    {
    union
        {
        struct
            {
#if defined(MIPSEL) || defined(__alpha) || defined(linux) || defined(__OPENNT)
            unsigned bits     :23;
            unsigned qnan_bit : 1;
            unsigned exponent : 7;
            unsigned sign     : 1;
#else
            unsigned sign     : 1;
            unsigned exponent : 7;
            unsigned qnan_bit : 1;
            unsigned bits     :23;
#endif
            } nan_parts;
        float f;
        } fnan;


    fnan.nan_parts.sign=0x0;
    fnan.nan_parts.exponent=0x7f;
    fnan.nan_parts.qnan_bit=0x1;
    fnan.nan_parts.bits=0xffff;

    return fnan.f;
    }

/************************************************************
CANCELKEYS - test for depressed control keys to cancel
         the current command.  returns 1 if time
         to cancel, otherwise 0

         NOTE:  FOR NOW ALWAYS RETURNS 0 !
************************************************************/
int cancelKeys ( void )
    {
    return 0;
    }



/************************************************************
ITOA - integer to ASCII
************************************************************/
void itoa ( int i, char *s )
    {
    sprintf ( s, "%d", i );
    }


/************************************************************
removeWhiteSpace - removes white space from a string, in place
           returns non-zero if error, else 0
           [also removes the " and ' characters]
************************************************************/
int removeWhiteSpace ( char *s )
    {
    int currentPos=0, stuffPos = 0;
    char c;

    if ( !s ) return errmsg ( "No string to remove white space from !" );
    if ( !s[0] ) return 0;

    while ( ( c = s[currentPos] ) != '\0' )
        {
        if     ( ( c != '\n' ) &&
                 ( c != '\t' ) &&
                 ( c != '\'' ) &&
                 ( c != '"' ) &&
                 ( c != '\'' ) &&
                 ( c != ' ' ) )
            {
            s[stuffPos] = c;
            stuffPos++;
            }
        currentPos++;
        }
    s[stuffPos] = '\0';
    return 0;
    }


/************************************************************
FTOA - double to ASCII
************************************************************/
void ftoa ( double d, char *s )
    {
    sprintf ( s, "%f", d );
    }



/************************************************************
ERRMSG - if being tested from an application

     NOTE:  it is the responsibility of the
        calling routine to make sure
        errorString points to actual memory !!!!
************************************************************/
int errmsg ( char *s )
    {
#ifdef DIAGNOSTICS
    diagmsg ( s );
#endif /* DIAGNOSTICS */

    if ( errorString != 0 )
        sprintf ( errorString, "%s", s );
    else
        fprintf ( stderr,
                  "utils.c's errmsg() can't fill NULL errorString with '%s'\n", s );

    return 1;
    }



/***********************************************************
DIAGMSG - basically handle a diagnostic or warning msg
************************************************************/
void diagmsg ( char *s )
    {
    char tstring[512];

    sprintf ( tstring, "DIAGMSG: '%s'", s );

#ifdef DIAGNOSTICS
    printf ( "%s\n", s );
    /*fprintf(fp, "%s\n", s);*/
#endif /* DIAGNOSTICS */
    }



/************************************************************
GETNTHITEM -    This routine will copy the nth item in
        inputString into outPutString.  Items in
        inputString should be separated by spaces and/or
        commas, which will be treated as spaces.

        Returns 1 if error, otherwise 0.
************************************************************/
int getNthItem ( int n, char *inputStr, char *outputString )
    {
    int i=0, j=0;
    char    *inputString;

    if ( n < 1 ) return 1;
    if ( !inputStr ) return 1;
    if ( !inputStr[0] ) return 1;
    inputString = strdup ( inputStr );
    if ( !inputString ) return 1;
    while ( inputString[i]!='\0' )
        {
        if ( inputString[i]==',' ) inputString[i] = ' ';
        i++;
        }
    i = 0;
    while ( inputString[i]==' ' ) i++;
    while ( n > 1 )
        {
        n--;
        while ( inputString[i]!=' ' )
            {
            if ( inputString[i] == '\0' )
                {
                free ( inputString );
                inputString = NULL;
                return 1;
                }
            i++;
            }
        while ( inputString[i]==' ' ) i++;
        }
    while ( ( inputString[i]!=' ' ) && ( inputString[i]!='\0' ) )
        outputString[j++] = inputString[i++];
    outputString[j] = '\0';
    free ( inputString );
    inputString = NULL;
    return 0;
    }



/************************************************************
dump_VIS_DATA_to_tabbed_ascii_file - returns 1 if error
                   - regardless it frees up vdata before leaving
************************************************************/
int dump_VIS_DATA_to_tabbed_ascii_file ( VIS_DATA *vdata, char *fname, char *estring )
    {
    FILE    *fp;
    int     i, j, k, t;
    int ni, nj, nk, nt;
    char    sep[2];

    if ( ( !vdata ) || ( !fname ) || ( !estring ) )
        {
        sprintf ( estring, "Bad args to dump_VIS_DATA_to_tabbed_ascii_file()!" );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    if ( !vdata->grid )
        {
        sprintf ( estring, "No grid values for dump_VIS_DATA_to_tabbed_ascii_file()!" );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    ni = vdata->col_max - vdata->col_min + 1;
    nj = vdata->row_max - vdata->row_min + 1;
    nk = vdata->level_max - vdata->level_min + 1;
    nt = vdata->step_max - vdata->step_min + 1;

    if ( ( ni < 1 ) || ( nj < 1 ) || ( nk < 1 ) || ( nt < 1 ) )
        {
        sprintf ( estring, "Can't find ni, nj, and/or nt for tabbed file!" );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    if ( ! ( fp=fopen ( fname, "w" ) ) )
        {
        sprintf ( estring, "Couldn't open file '%s' for writing !", fname );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    if     ( fprintf ( fp, "Tabbed Field file %s\n", fname ) <= 0 )
        {
        fclose ( fp );
        sprintf ( estring, "Error writing header info to tabbed data file '%s'!", fname );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    /* write out variable name and units name only if we have them */
    if  ( ( vdata->species_short_name != NULL ) &&
            ( vdata->units_name != NULL ) &&
            ( vdata->selected_species > 0 ) &&
            ( vdata->species_short_name[vdata->selected_species-1] != NULL ) &&
            ( vdata->units_name[vdata->selected_species-1] != NULL ) )
        if ( fprintf ( fp, "%s in %s", vdata->species_short_name[vdata->selected_species-1],
                       vdata->units_name[vdata->selected_species-1] ) <= 0 )
            {
            fclose ( fp );
            sprintf ( estring, "Error writing header info to tabbed data file '%s'!", fname );
            free_vis ( vdata );
            return errmsg ( estring );
            }

    /* now write out the actual ascii data */

    for ( t = 0; t < nt; t++ )
        {
        if ( fprintf ( fp, "\nTime Step %d\n", t+vdata->step_min ) <= 0 )
            {
            fclose ( fp );
            sprintf ( estring, "Error writing data values to tabbed data file '%s'!", fname );
            free_vis ( vdata );
            return errmsg ( estring );
            }

        for ( k = nk-1; k >= 0; k-- )
            {
            if ( fprintf ( fp, "Layer %d\n\t", k+vdata->level_min ) <= 0 )
                {
                fclose ( fp );
                sprintf ( estring, "Error writing data values to tabbed data file '%s'!", fname );
                free_vis ( vdata );
                return errmsg ( estring );
                }

            for ( i = 0; i < ni; i++ )
                {
                if ( fprintf ( fp, "Col %d\t", i+vdata->col_min ) <= 0 )
                    {
                    fclose ( fp );
                    sprintf ( estring, "Error writing data values to tabbed data file '%s'!", fname );
                    free_vis ( vdata );
                    return errmsg ( estring );
                    }
                }

            for ( j = nj-1; j >= 0; j-- )
                {
                if ( fprintf ( fp, "\nRow %d\t", j+vdata->row_min ) <= 0 )
                    {
                    fclose ( fp );
                    sprintf ( estring, "Error writing data values to tabbed data file '%s'!", fname );
                    free_vis ( vdata );
                    return errmsg ( estring );
                    }

                for ( i = 0; i < ni; i++ )
                    {
                    if ( fprintf ( fp, "%f\t", vdata->grid[INDEX ( i,j,k,t,ni,nj,nk )] ) <= 0 )
                        {
                        fclose ( fp );
                        sprintf ( estring, "Error writing data values to tabbed data file '%s'!", fname );
                        free_vis ( vdata );
                        return errmsg ( estring );
                        }
                    } /* i */

                }   /* j */

            }     /* k */

        }       /* t */

    fclose ( fp );
    return 0;
    }



/************************************************************
dump_VIS_DATA_to_AVS_file - returns 1 if error
              - regardless frees the vdata struct
************************************************************/
int dump_VIS_DATA_to_AVS_file ( VIS_DATA *vdata, char *fname, char *estring )
    {
    FILE    *fp;
    int     i, j, k, t;
    int ni, nj, nk, nt;
    char    sep[2];

    if ( ( !vdata ) || ( !fname ) || ( !estring ) )
        {
        sprintf ( estring, "Bad args to dump_VIS_DATA_to_AVS_file()!" );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    if ( !vdata->grid )
        {
        sprintf ( estring, "No grid values for dump_VIS_DATA_to_AVS_file()!" );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    /*
    if (vdata->level_min != vdata->level_max)
        {
        sprintf(estring, "Multiple layer AVS data output not yet supported!");
        free_vis(vdata);
        return errmsg(estring);
        }
    */

    ni = vdata->col_max - vdata->col_min + 1;
    nj = vdata->row_max - vdata->row_min + 1;
    nk = vdata->level_max - vdata->level_min + 1;
    nt = vdata->step_max - vdata->step_min + 1;

    if ( ( ni < 1 ) || ( nj < 1 ) || ( nk < 1 ) || ( nt < 1 ) )
        {
        sprintf ( estring, "Can't find ni, nj, nk, and/or nt for AVS file!" );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    if ( ! ( fp=fopen ( fname, "w" ) ) )
        {
        sprintf ( estring, "Couldn't open file '%s' for writing !", fname );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    if     ( ( fprintf ( fp, "# AVS Field file %s\n", fname ) <= 0 ) ||
             ( fprintf ( fp, "ndim = %d\n", ( 3+ ( nk!=1 ) ) ) <= 0 ) ||
             ( fprintf ( fp, "dim1 = %d\n", ni ) <= 0 ) ||
             ( fprintf ( fp, "dim2 = %d\n", nj ) <= 0 ) ||
             ( ( nk!=1 ) && ( fprintf ( fp, "dim3 = %d\n", nk ) <= 0 ) ) ||
             ( fprintf ( fp, "dim%d = %d\n", ( 3+ ( nk!=1 ) ), nt ) <= 0 ) ||
             ( fprintf ( fp, "nspace = 3\n" ) <= 0 ) ||
             ( fprintf ( fp, "veclen = 1\n" ) <= 0 ) ||
             ( fprintf ( fp, "data = float\n" ) <= 0 ) ||
             ( fprintf ( fp, "field = uniform\n" ) <= 0 ) ||
             ( fprintf ( fp, "min_ext = %.1f %.1f %.1f\n", ( float ) vdata->col_min, ( float ) vdata->row_min, ( float ) vdata->level_min ) <= 0 ) ||
             ( fprintf ( fp, "max_ext = %.1f %.1f %.1f\n", ( float ) vdata->col_max, ( float ) vdata->row_max, ( float ) vdata->level_max ) <= 0 ) )
        {
        fclose ( fp );
        sprintf ( estring, "Error writing header info to AVS data file '%s'!", fname );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    /* write out variable name and units name only if we have them */
    if  ( ( vdata->species_short_name != NULL ) &&
            ( vdata->units_name != NULL ) &&
            ( vdata->selected_species > 0 ) &&
            ( vdata->species_short_name[vdata->selected_species-1] != NULL ) &&
            ( vdata->units_name[vdata->selected_species-1] != NULL ) )
        if ( ( fprintf ( fp, "label = %s\n", vdata->species_short_name[vdata->selected_species-1] ) <= 0 ) ||
                ( fprintf ( fp, "unit = %s\n", vdata->units_name[vdata->selected_species-1] ) <= 0 ) )
            {
            fclose ( fp );
            sprintf ( estring, "Error writing header info to AVS data file '%s'!", fname );
            free_vis ( vdata );
            return errmsg ( estring );
            }

    if     ( ( fprintf ( fp, "min_val = %f\n", vdata->grid_min ) <= 0 ) ||
             ( fprintf ( fp, "max_val = %f\n", vdata->grid_max ) <= 0 ) )
        {
        fclose ( fp );
        sprintf ( estring, "Error writing header info to AVS data file '%s'!", fname );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    /* now write out two ctrl-l separator characters between ascii header and binary data */
    sep[0] = sep[1] = 12;
    if ( fwrite ( sep, 1, 2, fp ) != 2 )
        {
        fclose ( fp );
        sprintf ( estring, "Error writing header info to AVS data file '%s'!", fname );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    /* now write out the actual binary data */
    for ( t = 0; t < nt; t++ )    /* AVS varies T slowest when reading fields */
        for ( k = 0; k < nk; k++ )
            for ( j = 0; j < nj; j++ )
                for ( i = 0; i < ni; i++ ) /* AVS varies I fastest when reading fields */
                    if ( fwrite ( &vdata->grid[INDEX ( i,j,k,t,ni,nj,nk )], sizeof ( float ), 1, fp ) != 1 )
                        {
                        fclose ( fp );
                        sprintf ( estring, "Error writing data values to AVS data file '%s'!", fname );
                        free_vis ( vdata );
                        return errmsg ( estring );
                        }

    free_vis ( vdata );
    fclose ( fp );
    return 0;
    }



/************************************************************
RANGE_GET - returns 1 if error (or if no cell is on)
************************************************************/
int range_get (  char *percents, /* array to denote which cells in domain */
                 int IMAX,
                 int JMAX,
                 int *imin,   /* minimum column w/ percents on */
                 int *imax,   /* maximum column w/ percents on */
                 int *jmin,   /* minimum row w/ percents on */
                 int *jmax )  /* maximum row w/ percents on */
    {
    int i,j,t,on=0;

    if ( percents == NULL ) return ( errmsg ( "NULL percents!" ) );

    /*  (reverse of) the maximum bounding box allowed */
    *imin = IMAX-1;
    *imax = 0;
    *jmin = JMAX-1;
    *jmax = 0;


    for ( j = 0; j < JMAX; j++ )
        for ( i = 0; i < IMAX; i++ )
            if ( percents[INDEX ( i,j,0,0,IMAX,JMAX,1 )] )
                {
                if ( i < *imin ) *imin = i;
                if ( i > *imax ) *imax = i;
                if ( j < *jmin ) *jmin = j;
                if ( j > *jmax ) *jmax = j;
                on = 1;
                }

    if ( *imin > *imax )
        {
        t = *imin;
        *imin = *imax;
        *imax = t;
        }

    if ( *jmin > *jmax )
        {
        t = *jmin;
        *jmin = *jmax;
        *jmax = t;
        }

    if ( !on ) return ( errmsg ( "There are no selected cells in this formula's region of interest!" ) );

    return 0;
    }



/************************************************************
CALC_STATS - returns 1 if error
************************************************************/
int calc_stats  (
    /* INPUTS to calc_stats */

    VIS_DATA *vdata,

    char percents[],    /* This is an array indicating which
                                       cells are in the domain and which
                                       are turned off.  Where
                                       IMAX=vdata->col_max-vdata->col_min+1,
                                       JMAX=vdata->row_max-vdata->row_min+1,
                                       KMAX=vdata->level_max-vdata->level_min+1,
                                       percents is indexed
                                       [i+j*IMAX] (where i ranges 0..IMAX-1
                                       and j ranges 0..JMAX-1).  For a
                                       grid cell completely in the domain,
                                       the corresponding percent's value
                                       should be 100, for a cell completely
                                       NOT in the domain, it should be 0.
                                       For a cell which should only count
                                       partially, use a value between 0
                                       and 100 */

    int layers[],       /* vdata->levelMax-vdata->levelMin+1
                       items telling whether to use each
                       layer.  0 means don't use it,
                       non-zero means use it */

    int step,       /* Note:  step is *0* based

                                   if (step >= 0) then only that
                                   time step is computed for;

                                   if (step < 0) then stats
                                   are calculated for all time steps
                                   that are there */


    int TMAX,       /* the number of time steps for which
                       data exists within vdata */

    /* OUTPUT by calc_stats */

    int *maxi, /* 1 based */
    int *maxj, /* 1 based */
    int *maxk, /* 1 based */
    int *maxt, /* 1 based */
    int *mini, /* 1 based */
    int *minj, /* 1 based */
    int *mink, /* 1 based */
    int *mint, /* 1 based */
    float *min,
    float *max,
    float *mean,
    float *var,
    float *std_dev,
    float *sumf
)
    {
    int   i, j, k, n=0, IMAX, JMAX, KMAX, tmin, tmax, t, vtmin, vtmax;
    double val, sum, ssq, meand, div;

    if ( step >= 0 )
        {
        if ( step >= TMAX )
            return errmsg ( "Bad step arg to calc_stats() !" );
        tmin = tmax = step;
        }
    else
        {
        tmin = 0;
        tmax = TMAX-1;
        }

    /* a bit of error checking */
    if ( ( vdata       == NULL ) ||
         ( percents    == NULL ) ||
         ( layers      == NULL ) ||
         ( vdata->grid == NULL ) ||
         ( vdata->ncol <= 0 ) ||
         ( vdata->nrow <= 0 ) ||
         ( vdata->nlevel <= 0 ) )
        return errmsg ( "Bad arguments to calc_stats() !" );

    /* some more verification on the arguments */
    if ( ( vdata->slice == XYTSLICE ) ||
         ( vdata->slice == YZTSLICE ) ||
         ( vdata->slice == XZTSLICE ) ||
         ( vdata->slice == XYZTSLICE ) )
        {
        vtmin = vdata->step_min-1;
        vtmax = vdata->step_max-1;
        }
    else
        {
        vtmin = vdata->selected_step-1;
        vtmax = vdata->selected_step-1;
        }
    if ( ( tmin < 0 ) ||
         ( tmin > vtmax-vtmin+1 ) ||
         ( tmax < 0 ) ||
         ( tmax > vtmax-vtmin+1 ) )
        return errmsg ( "Invalid step/time args to calc_stats() !" );

    /* set up some vars */
    IMAX = vdata->col_max - vdata->col_min + 1;
    JMAX = vdata->row_max - vdata->row_min + 1;
    KMAX = vdata->level_max - vdata->level_min + 1;

    /* now loop over the data itself to find the min & max */
    sum = 0.0 ;
    ssq = 0.0 ;
    for ( n=0, t=tmin; t<=tmax; t++ )
        {
        for ( k=0; k<KMAX; k++ )
            {
              if ( layers[k] )
                {
                for ( j=0; j<JMAX; j++ )
                    {
                    for ( i=0; i<IMAX; i++ )
                        {
                        if ( percents[ INDEX( i,j,0,0,IMAX,JMAX,1 ) ] )
                            {
                            val = vdata->grid[INDEX ( i,j,k,t,IMAX,JMAX,KMAX )];
                            if ( isnanf ( val ) ) continue;
                            if ( !n )
                                {
                                *min  = *max = val;
                                *mini = *maxi = vdata->col_min+i;
                                *minj = *maxj = vdata->row_min+j;
                                *mink = *maxk = vdata->level_min+k;
                                *mint = *maxt = vdata->step_min+t;
                                }
                            else if ( val < *min )
                                {
                                *mini = vdata->col_min+i;
                                *minj = vdata->row_min+j;
                                *mink = vdata->level_min+k;
                                *mint = vdata->step_min+t;
                                *min = val;
                                }
                            else if ( val > *max )
                                {
                                *maxi = vdata->col_min+i;
                                *maxj = vdata->row_min+j;
                                *maxk = vdata->level_min+k;
                                *maxt = vdata->step_min+t;
                                *max = val;
                                }
                            sum += val ;
                            ssq += val*val ;
                            n++;
                            }
                        }
                    }
                }
            }
        }

    if ( !n ) return errmsg ( "No grid cells in the domain in calc_stats() !" );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "%d grid cells on in each time step in util.c's calc_stats()\n", ( int ) ( n/ ( tmax-tmin+1 ) ) );
#endif /* DIAGNOSTICS */

    /* set the sum argument */
    *sumf = sum;

    /* find the mean, standard deviation  */
    div   = 1.0 / (double)n ;
    sum = sum * div ;
    ssq = ssq * div - sum*sum ;
    ssq = ( ssq > 0.0 ? ssq : 0.0 ) ;

    *mean    = sum ;
    *var     = ssq ;
    *std_dev = sqrt( ssq ) ;

    return 0;
    }




/************************************************************
DUMP_VIS_DATA - used for diagnostic purposes;
        returns 1 if error
************************************************************/
int dump_VIS_DATA ( VIS_DATA *vdata,

                    char *percents,   /* This is an array indicating which
                                       cells are in the domain and which
                                       are turned off.  Where IMAX==
                       vdata->ncol and JMAX==vdata->nrow,
                       percents is indexed
                                       [i+j*IMAX] (where i ranges 0..IMAX-1
                                       and j ranges 0..JMAX-1).  For a
                                       grid cell completely in the domain,
                                       the corresponding percent's value
                                       should be 100, for a cell completely
                                       NOT in the domain, it should be 0.
                                       For a cell which should only count
                                       partially, use a value between 0
                                       and 100 */

                    int *layers )      /* vdata->levelMax-vdata->levelMin+1
                                       items telling whether to use each
                                       layer.  0 means don't use it,
                                       non-zero means use it
                       NOTE - THIS IS 0 BASED */

    {
    int i, TMAX;

    if ( vdata == NULL )
        return errmsg ( "Can't dump_VIS_DATA with a NULL VIS_DATA struct !" );

    switch ( vdata->slice )
        {
        case XYTSLICE:
        case YZTSLICE:
        case XYZTSLICE:
        case XZTSLICE:
            TMAX = vdata->step_max - vdata->step_min + 1;
            break;

        case XYSLICE:
        case YZSLICE:
        case XZSLICE:
        case XYZSLICE:
        default:
            TMAX = 1;
            break;
        };

    fprintf ( stderr, "\n" );
    fprintf ( stderr, "----------------------------------------------------------------\n" );
    fprintf ( stderr, "        *** Info for file " );
    if ( vdata->filename == NULL )
        fprintf ( stderr, "<N/A> ***\n" );
    else
        {
        if ( vdata->filehost.name != NULL )
            fprintf ( stderr, "%s:", vdata->filehost.name );
        fprintf ( stderr, "%s ***        \n", vdata->filename );
        }
    if ( vdata->dataset == netCDF_DATA )
        fprintf ( stderr, "              Data Set Type:  %s\n", "netCDF" );
    else if ( vdata->dataset == UAM_DATA )
        fprintf ( stderr, "              Data Set Type:  %s\n", "UAM" );
    else if ( vdata->dataset == UAMV_DATA )
        fprintf ( stderr, "              Data Set Type:  %s\n", "UAMV" );
    else if ( vdata->dataset == netCDF_OBS )
        fprintf ( stderr, "              Data Set Type:  %s\n", "netCDF_OBS" );
    else
        fprintf ( stderr, "              Data Set Type:  UNKNOWN !\n" );
    fprintf ( stderr, "          Number of Species:  %d\n", vdata->nspecies );
    if ( vdata->species_short_name &&
            vdata->species_long_name &&
            vdata->units_name )
        for ( i = 0; i < vdata->nspecies; i++ )
            fprintf ( stderr, "                Species(%02d):  *%s*%s*%s*\n", i+1, vdata->species_short_name[i], vdata->species_long_name[i], vdata->units_name[i] );
    else
        fprintf ( stderr, "nothing there for species_short_name, long_name, &/or units !\n" );
    fprintf ( stderr, "                    MapInfo:  %s\n", vdata->map_info ?
              vdata->map_info : "NULL" );
    fprintf ( stderr, "                 Data_Label:  %s\n", vdata->data_label ?
              vdata->data_label : "NULL" );
    fprintf ( stderr, "          First Julian Date:  %d\n", vdata->first_date );
    fprintf ( stderr, "          First Julian Time:  %d\n", vdata->first_time );
    fprintf ( stderr, "           Last Julian Date:  %d\n", vdata->last_date );
    fprintf ( stderr, "           Last Julian Time:  %d\n", vdata->last_time );
    fprintf ( stderr, "Julian Time Increment(secs):  %d\n", vdata->incr_sec );
    fprintf ( stderr, "          Number of Columns:  %d\n", vdata->ncol );
    fprintf ( stderr, "             Number of Rows:  %d\n", vdata->nrow );
    fprintf ( stderr, "           Number of Layers:  %d\n", vdata->nlevel );
    fprintf ( stderr, "            Number of Steps:  %d\n", vdata->nstep );
    fprintf ( stderr, "                 Column Min:  %d\n", vdata->col_min );
    fprintf ( stderr, "                 Column Max:  %d\n", vdata->col_max );
    fprintf ( stderr, "                    Row Min:  %d\n", vdata->row_min );
    fprintf ( stderr, "                    Row Max:  %d\n", vdata->row_max );
    fprintf ( stderr, "                  Level Min:  %d\n", vdata->level_min );
    fprintf ( stderr, "                  Level Max:  %d\n", vdata->level_max );
    fprintf ( stderr, "                   Step Min:  %d\n", vdata->step_min );
    fprintf ( stderr, "                   Step Max:  %d\n", vdata->step_max );
    fprintf ( stderr, "                  Step Incr:  %d\n", vdata->step_incr );
    fprintf ( stderr, "               Grid Minimum:  %.15f\n", vdata->grid_min );
    fprintf ( stderr, "               Grid Maximum:  %.15f\n", vdata->grid_max );
    fprintf ( stderr, "           Selected Species:  %d\n", vdata->selected_species );
    fprintf ( stderr, "            Selected Column:  %d\n", vdata->selected_col );
    fprintf ( stderr, "               Selected Row:  %d\n", vdata->selected_row );
    fprintf ( stderr, "             Selected Level:  %d\n", vdata->selected_level );
    fprintf ( stderr, "              Selected Step:  %d\n", vdata->selected_step );

    switch ( vdata->slice )
        {
        case XYTSLICE:
            fprintf ( stderr, "                 Slice Type:  XYT\n" );
            break;
        case YZTSLICE:
            fprintf ( stderr, "                 Slice Type:  YZT\n" );
            break;
        case XYZTSLICE:
            fprintf ( stderr, "                 Slice Type:  XYZT\n" );
            break;
        case XZTSLICE:
            fprintf ( stderr, "                 Slice Type:  XZT\n" );
            break;
        case XYSLICE:
            fprintf ( stderr, "                 Slice Type:  XY\n" );
            break;
        case YZSLICE:
            fprintf ( stderr, "                 Slice Type:  YZ\n" );
            break;
        case XZSLICE:
            fprintf ( stderr, "                 Slice Type:  XZ\n" );
            break;
        case XYZSLICE:
            fprintf ( stderr, "                 Slice Type:  XYZ\n" );
            break;
        case NONESLICE:
            fprintf ( stderr, "                 Slice Type:  NONE\n" );
            break;
        default:
            fprintf ( stderr, "                 Slice Type:  ???\n" );
            break;
        };

    if ( vdata->selected_species <= 0 )
        return errmsg ( "Invalid vdata->selected_species in dump_VIS_DATA()!\n" );

    if ( vdata->species_short_name )
        fprintf ( stderr, "                    Species:  %s\n",
                  vdata->species_short_name[vdata->selected_species-1] );
    else
        fprintf ( stderr, "           (0-based)Species:  %d\n", vdata->selected_species );


    if ( vdata->grid != NULL )
        {
        int   maxi, maxj, maxk, maxt, mini, minj, mink, mint;
        float min, max, mean, var, std_dev, sum;

        if ( percents == NULL )
            {
            fprintf ( stderr,
                      "Can't check stats in dump_VIS_DATA with a NULL percents array !\n" );
            fprintf ( stderr, "----------------------------------------------------------------\n" );
            return 0;
            }

        if ( layers == NULL )
            {
            fprintf ( stderr,
                      "Can't check stats in dump_VIS_DATA with a NULL layers array !\n" );
            fprintf ( stderr, "----------------------------------------------------------------\n" );
            return 0;
            }

        fprintf ( stderr, "        *** Stats for file " );
        if ( vdata->filename == NULL )
            fprintf ( stderr, "<N/A> ***\n" );
        else
            fprintf ( stderr, "%s ***        \n", vdata->filename );

        if ( calc_stats  ( vdata, percents, layers, -1, TMAX,
                           &maxi, &maxj, &maxk, &maxt,
                           &mini, &minj, &mink, &mint,
                           &min, &max, &mean, &var, &std_dev, &sum ) )
            return 1;


        fprintf ( stderr, "              Minimum Value:  %.15f at (%2d, %2d, %2d, %2d)\n",
                  min, mini, minj, mink, mint );
        fprintf ( stderr, "              Maximum Value:  %.15f at (%2d, %2d, %2d, %2d)\n",
                  max, maxi, maxj, maxk, maxt );
        fprintf ( stderr, "                       Mean:  %.15f\n", mean );
        fprintf ( stderr, "                   Variance:  %.15f\n", var );
        fprintf ( stderr, "         Standard Deviation:  %.15f\n", std_dev );
        fprintf ( stderr, "                        Sum:  %.15f\n", sum );
        }
    else
        fprintf ( stderr, "NOTE: there is no grid in this struct !!\n" );

    fprintf ( stderr, "----------------------------------------------------------------\n" );

    return 0;
    }



/************************************************************
strip_VIS_DATA_operator_chars - removes all operator
    characters (+,-,/,*) from all of a VIS_DATA struct's
    short specie names, in order to allow parsing
        of these specie names correctly in a formula.

        Returns 1 if error.
************************************************************/
int strip_VIS_DATA_operator_chars ( VIS_DATA *info, char *estring )
    {
    int i, j;
    char *cp;

    /* a little error checking on the args */

    if ( !info )
        {
        sprintf ( estring, "NULL info struct passed to strip_VIS_DATA_operator_chars()!" );
        return 1;
        }
    if ( !estring )
        {
        sprintf ( estring, "NULL estring passed to strip_VIS_DATA_operator_chars()!" );
        return 1;
        }

    if ( ( *info ).species_short_name != NULL )
        if ( ( *info ).nspecies > 0 )
            for ( i = 0; i < ( *info ).nspecies; i++ )
                if ( ( cp = ( *info ).species_short_name[i] ) != NULL )
                    {
                    j = 0;
                    while ( ( *cp ) != '\0' )
                        {
                        if ( *cp == '+' ||
                                *cp == '-' ||
                                *cp == '*' ||
                                *cp == '/' )
                            strcpy ( cp, cp+1 );
                        cp++;
                        }
                    }

    estring[0] = '\0';
    return 0;
    }



/************************************************************
VIS_DATA_dup -  returns a copies a VIS_DATA struct and also of all of its
        various tendrills of allocated memory; returns NULL if error
************************************************************/
VIS_DATA *VIS_DATA_dup ( VIS_DATA *info, char *estring )
    {
    VIS_DATA *ans;
    int i;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "\nEnter VIS_DATA_dup()\n" );
#endif /* ifdef DIAGNOSTICS */

    /* a little error checking on the args */

    if ( !info )
        {
        sprintf ( estring, "NULL info struct passed to VIS_DATA_dup()!" );
        return ( VIS_DATA * ) NULL;
        }
    if ( !estring )
        {
        sprintf ( estring, "NULL estring passed to VIS_DATA_dup()!" );
        return ( VIS_DATA * ) NULL;
        }


    /* allocate space for the VIS_DATA to be returned and set
       all bytes therein to NULL */

    if ( ! ( ans = ( VIS_DATA * ) malloc ( sizeof ( VIS_DATA ) ) ) )
        {
        sprintf ( estring, "malloc failure (1) in VIS_DATA_dup()!" );
        return ( VIS_DATA * ) NULL;
        }
    memset ( ( void * ) ans, 0, ( size_t ) sizeof ( VIS_DATA ) );


    /* now copy over all of the fields, allocating space when necessary */

    if ( info->filename )
        if ( ! ( ans->filename = strdup ( info->filename ) ) )
            {
            sprintf ( estring, "malloc failure (2a) in VIS_DATA_dup()!" );
            free_vis ( ans );
            free ( ans );
            ans = NULL;
            return ( VIS_DATA * ) NULL;
            }

    if ( info->filehost.ip )
        if ( ! ( ans->filehost.ip = strdup ( info->filehost.ip ) ) )
            {
            sprintf ( estring, "malloc failure (2b) in VIS_DATA_dup()!" );
            free_vis ( ans );
            free ( ans );
            ans = NULL;
            return ( VIS_DATA * ) NULL;
            }

    if ( info->filehost.name )
        if ( ! ( ans->filehost.name = strdup ( info->filehost.name ) ) )
            {
            sprintf ( estring, "malloc failure (2c) in VIS_DATA_dup()!" );
            free_vis ( ans );
            free ( ans );
            ans = NULL;
            return ( VIS_DATA * ) NULL;
            }

    ans->filehost.port = info->filehost.port;

    ans->dataset        = info->dataset;

    ans->nspecies       = info->nspecies;

    if ( ( *info ).nspecies )
        {
        if ( ( *info ).species_short_name != NULL )
            if ( ! ( ans->species_short_name = ( char ** ) malloc ( ( *info ).nspecies * sizeof ( char** ) ) ) )
                {
                sprintf ( estring, "malloc failure (3) in VIS_DATA_dup()!" );
                free_vis ( ans );
                free ( ans );
                ans = NULL;
                return ( VIS_DATA * ) NULL;
                }
        if ( ( *info ).species_long_name != NULL )
            if ( ! ( ans->species_long_name = ( char ** ) malloc ( ( *info ).nspecies * sizeof ( char** ) ) ) )
                {
                sprintf ( estring, "malloc failure (4) in VIS_DATA_dup()!" );
                free_vis ( ans );
                free ( ans );
                ans = NULL;
                return ( VIS_DATA * ) NULL;
                }
        if ( ( *info ).units_name != NULL )
            if ( ! ( ans->units_name = ( char ** ) malloc ( ( *info ).nspecies * sizeof ( char** ) ) ) )
                {
                sprintf ( estring, "malloc failure (5) in VIS_DATA_dup()!" );
                free_vis ( ans );
                free ( ans );
                ans = NULL;
                return ( VIS_DATA * ) NULL;
                }
        for ( i = 0; i < ( *info ).nspecies; i++ )
            {
            if ( ( *info ).species_short_name != NULL )
                if ( ( *info ).species_short_name[i] != NULL )
                    if ( ! ( ans->species_short_name[i] = strdup ( ( *info ).species_short_name[i] ) ) )
                        {
                        sprintf ( estring, "malloc failure (6) in VIS_DATA_dup()!" );
                        free_vis ( ans );
                        free ( ans );
                        ans = NULL;
                        return ( VIS_DATA * ) NULL;
                        }
            if ( ( *info ).species_long_name != NULL )
                if ( ( *info ).species_long_name[i] != NULL )
                    if ( ! ( ans->species_long_name[i] = strdup ( ( *info ).species_long_name[i] ) ) )
                        {
                        sprintf ( estring, "malloc failure (7) in VIS_DATA_dup()!" );
                        free_vis ( ans );
                        free ( ans );
                        ans = NULL;
                        return ( VIS_DATA * ) NULL;
                        }
            if ( ( *info ).units_name != NULL )
                if ( ( *info ).units_name[i] != NULL )
                    if ( ! ( ans->units_name[i] = strdup ( ( *info ).units_name[i] ) ) )
                        {
                        sprintf ( estring, "malloc failure (8) in VIS_DATA_dup()!" );
                        free_vis ( ans );
                        free ( ans );
                        ans = NULL;
                        return ( VIS_DATA * ) NULL;
                        }
            }
        }

    if ( info->map_info )
        if ( ! ( ans->map_info = strdup ( info->map_info ) ) )
            {
            sprintf ( estring, "malloc failure (8) in VIS_DATA_dup()!" );
            free_vis ( ans );
            free ( ans );
            ans = NULL;
            return ( VIS_DATA * ) NULL;
            }

    if ( info->data_label )
        if ( ! ( ans->data_label = strdup ( info->data_label ) ) )
            {
            sprintf ( estring, "malloc failure (9) in VIS_DATA_dup()!" );
            free_vis ( ans );
            free ( ans );
            ans = NULL;
            return ( VIS_DATA * ) NULL;
            }


    ans->first_date     = info->first_date;

    ans->first_time     = info->first_time;

    ans->last_date      = info->last_date;

    ans->last_time      = info->last_time;

    ans->incr_sec       = info->incr_sec;

    ans->ncol           = info->ncol;

    ans->nrow           = info->nrow;

    ans->nlevel         = info->nlevel;

    ans->nstep          = info->nstep;

    ans->col_min        = info->col_min;

    ans->col_max        = info->col_max;

    ans->row_min        = info->row_min;

    ans->row_max        = info->row_max;

    ans->level_min      = info->level_min;

    ans->level_max      = info->level_max;

    ans->step_min       = info->step_min;

    ans->step_max       = info->step_max;

    ans->step_incr      = info->step_incr;

    ans->slice          = info->slice;

    ans->selected_species   = info->selected_species;

    ans->selected_col   = info->selected_col;

    ans->selected_row   = info->selected_row;

    ans->selected_level = info->selected_level;

    ans->selected_step  = info->selected_step;

    if ( ( *info ).grid )
        {
        /* NOTE:  I HOPE THIS IS CORRECT !!!  THERE PROBABLY
           SHOULD BE SOME CHECKING HERE OF slice VALUE, AND
           APPROPRIATE CALCULATIONS DONE BASED ON THAT VALUE - SRT 061495 */

        long nfloats =  ( ( long ) ( info->col_max-info->col_min+1 ) ) *
                        ( ( long ) ( info->row_max-info->row_min+1 ) ) *
                        ( ( long ) ( info->level_max-info->level_min+1 ) ) *
                        ( ( long ) ( info->step_max-info->step_min+1 ) );

        if ( ! ( ans->grid = ( float * ) malloc ( ( size_t ) ( nfloats*sizeof ( float ) ) ) ) )
            {
            sprintf ( estring, "malloc failure (10) in VIS_DATA_dup()!" );
            free_vis ( ans );
            free ( ans );
            ans = NULL;
            return ( VIS_DATA * ) NULL;
            }

        memcpy ( ( void * ) ans->grid, ( *info ).grid, ( size_t ) ( nfloats*sizeof ( float ) ) );
        }

    if ( ( *info ).nstep )
        {
        if ( ( *info ).sdate )
            {
            if ( ! ( ans->sdate = ( int * ) malloc ( ( *info ).nstep * sizeof ( int ) ) ) )
                {
                sprintf ( estring, "malloc failure (11) in VIS_DATA_dup()!" );
                free_vis ( ans );
                free ( ans );
                ans = NULL;
                return ( VIS_DATA * ) NULL;
                }
            for ( i = 0; i < ( *info ).nstep; i++ )
                ans->sdate[i] = ( *info ).sdate[i];
            }
        if ( ( *info ).stime )
            {
            if ( ! ( ans->stime = ( int * ) malloc ( ( *info ).nstep * sizeof ( int ) ) ) )
                {
                sprintf ( estring, "malloc failure (12) in VIS_DATA_dup()!" );
                free_vis ( ans );
                free ( ans );
                ans = NULL;
                return ( VIS_DATA * ) NULL;
                }
            for ( i = 0; i < ( *info ).nstep; i++ )
                ans->stime[i] = ( *info ).stime[i];
            }
        }

    ans->grid_min       = info->grid_min;

    ans->grid_max       = info->grid_max;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Exiting VIS_DATA_dup()\n" );
#endif /* ifdef DIAGNOSTICS */

    return ans;
    }



/************************************************************
MYFREEVIS - frees up storage used for VIS_DATA struct,
            and also resets all the contents of that struct
            to be NULL values
************************************************************/
void myFreeVis ( VIS_DATA *info )
    {
    if ( info != NULL )
        {
#ifdef MDIAGS
        printf ( "ENTER MYFREEVIS(info == %lu)\n", ( unsigned long ) info ); /*SRT*/
#endif /* MDIAGS */
        free_vis ( info );
        memset ( ( void * ) info, 0, ( size_t ) sizeof ( VIS_DATA ) );
        }
    }




/************************************************************
INTEGRAL - returns 1 if argument is an integral value,
       otherwise returns 0
************************************************************/
int integral ( double d )
    {
    long l = d;

    return ( d-l == 0.0 );
    }


/************************************************************
getPointerToBaseName - returns NULL if not there

    char *fullname is a dataset specified in one string
    and formatted [<hostname>:][<pathname>/]<filename>

    this function returns a pointer to the beginning of
    filename
************************************************************/
char *getPointerToBaseName ( char *fullname ) /* full name to be parsed */
    {
    char *q;

    q = ( char * ) strrchr ( fullname, ( int ) '/' ); /* find the last '/' */
    if ( q )
        /* we've got a path name here */
        return q+1;
    else
        /* there is no path name specified in tstring */
        return fullname;
    }



/************************************************************
PARSELONGDATASETNAME - returns nonzero if error

    char *long is a dataset specified in one string
    and formatted [<hostname>:][<pathname>/]<filename>

    this function splits out the various components
    and make copies of them into hname, pname, and fname
************************************************************/
int parseLongDataSetName ( char *lname, /* full name to be parsed */
                           char *hname,   /* extracted host name goes here */
                           char *pname,   /* extracted path name goes here */
                           char *fname )  /* extracted fname name goes here */
    {
    char    tstring[512], *p, *q;

    assert ( lname && hname && pname && fname );
    strcpy ( tstring, lname );

    /* find the three individual components */

    p = strchr ( tstring, ( int ) ':' ); /* find the first ':' */
    if ( p )
        {
        /* we've got a host name here */
        *p = '\0';
        strcpy ( hname, tstring );
        p++;
        }
    else
        {
        /* there is no host name specified in tstring */
        p = tstring;
        hname[0] = '\0';
        }

    q = ( char * ) strrchr ( p, ( int ) '/' ); /* find the last '/' */
    if ( q )
        {
        /* we've got a path name here */
        *q = '\0';
        q++;
        strcpy ( pname, p );
        strcpy ( fname, q );
        }
    else
        {
        /* there is no path name specified in tstring */
        pname[0] = '\0';
        strcpy ( fname, p );
        }

    return 0;
    }



/************************************************************
IEEE_NAN - returns an IEEE NaN float
************************************************************/
float ieee_nan ( void )
    {
    /* union struct used to make an IEEE NaN for float - SRT;
       IEEE float format determined using Gerry Kane's
       "mips RISC Architecture", pp 6-8 thru 6-11 */
    union
        {
        struct
            {
            unsigned sign     :  1;
            unsigned exponent :  8;
            unsigned fraction : 23;
            }
        f_parts;

        float       f;

        unsigned long   l;
        }
    ieee_nanf;

    ieee_nanf.l = 0x7fbfffffU;  /* see p. E-2 of Kane's book for this # */
    return ieee_nanf.f;
    }



#ifdef MDIAGS
#undef malloc
#undef free

/************************************************************
THORPE_MALLOC - returns 1 if argument is an integral value,
************************************************************/
void *thorpe_malloc ( size_t size )
    {
    void *t;
    printf ( "               MALLOC(%12ld)", ( long ) size );
    fflush ( NULL );
    t = malloc ( size );
    printf ( "=%ld\n", ( long ) t );
    return t;
    }

/************************************************************
THORPE_FREE - returns 1 if argument is an integral value,
************************************************************/
void thorpe_free ( void *ptr )
    {
    printf ( "                               FREE %ld\n", ( long ) ptr );
    free ( ptr );
    }

#endif /* #ifdef MDIAGS */



/************************************************************
getLocalHostName -  returns a pointer to a char
            string with the local host name
************************************************************/
#include <unistd.h>

#ifdef __sparc
#include <sys/systeminfo.h>
#endif /* __sparc */

char *getLocalHostName ( void )
    {
    static char localHostName[65];
    localHostName[64] = '\0';

#ifdef __sparc

    sysinfo ( SI_HOSTNAME, localHostName, 64 );

#else

    if ( gethostname ( localHostName, 64 ) )
        localHostName[0] = '\0';

#endif /* __sparc */

    return localHostName;
    }



/************************************************************
registerCurrentTime
    records the current time in preparation for a later
    call to verifyElapsedClockTime
************************************************************/
struct timeval tpstart;
static int registeredTimeYet = 0;
void registerCurrentTime ( void )
    {
    struct timezone tzp;
    gettimeofday ( &tpstart, &tzp );
    registeredTimeYet = 1;
    }


/************************************************************
verifyElapsedClockTime
        verifies that secsSinceRegistered wall clock seconds
    have passed since registerCurrentTime() was last called.
    If not, this routine will wait until secsSinceRegistered
    secs have passed before returning
************************************************************/
void verifyElapsedClockTime ( float secsSinceRegistered )
    {
    struct timezone tzp;
    struct timeval tpnow;
    long   elapsedMicroSeconds;

#ifdef DIAGNOSTICS
    fprintf ( stderr,"Enter verifyElapsedClockTime(%g)\n", secsSinceRegistered );
#endif /* DIAGNOSTICS */
    if ( !registeredTimeYet ) return;
    while ( 1 )
        {
        gettimeofday ( &tpnow, &tzp );
        elapsedMicroSeconds = ( tpnow.tv_sec-tpstart.tv_sec ) *1000000+
                              tpnow.tv_usec-tpstart.tv_usec;

        if ( secsSinceRegistered*1000000.0 <= ( double ) elapsedMicroSeconds )
            {

#ifdef DIAGNOSTICS
            fprintf ( stderr,"Exit verifyElapsedClockTime(%g)\n",
                      secsSinceRegistered );
#endif /* DIAGNOSTICS */

            return;
            }
        }
    }


/************************************************************
M3IO_parameter_fix
************************************************************/
void   M3IO_parameter_fix ( VIS_DATA *info )
    {
#ifndef   LAMGRD3
#define   LAMGRD3    (2)
#endif

    int     grid_type, ncol, nrow, fixed = 0;
    float   xorig, yorig, xcell, ycell, xcent, ycent, p_gam, p_bet, p_alp, tf;
    char    map_info[256];

    /*
    int utm_zone,
    float   llx, lly, urx, ury;
    */

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter M3IO_parameter_fix()\n" );
#endif /* DIAGNOSTICS */

    switch ( info->dataset )
        {
        case UAMV_DATA:
            break;


        case UAM_DATA:
        case netCDF_DATA:
        case netCDF_OBS:
            sscanf ( info->map_info, "%d%g%g%g%g%g%g%g%g%g%d%d",
                     &grid_type, &xorig, &yorig, &xcell,
                     &ycell, &xcent, &ycent, &p_gam,
                     &p_bet, &p_alp, &ncol, &nrow );

            /* are P_ALP and P_BET improperly reversed? */
            if ( ( grid_type == LAMGRD3 ) || ( grid_type == ALBGRD3 ) )
                if ( p_alp > p_bet )
                    {
                    fixed = 1;
                    tf    = p_bet;
                    p_bet = p_alp;
                    p_alp = tf;
                    fprintf ( stderr, "\n\nM3IO_parameter_fix() WARNING:\n"
                              "p_alp and p_bet are reversed in the file."
                              "  M3IO file should be fixed.\n\n" );
                    }

            /* ************************************************************
            GROTESQUE MISBEHAVIOR!! --  CJC
                           According to the M3IO documentation, for
                           non-lat-lon grids, units for xorig, yorig,
                           xcell, and ycell are supposed to be in
                           meters.  However every sample data file
                           encountered thus far has used kilometers.
                           Below is an attempt to detect and convert
                           such bad units.  However, a problem with
                           this work-around is that if it encounters
                           a high-resolution grid that actually
                           defines sub-100 meter cells, it will
                           mistake them for 'bad kilometer units'
                           and handle them as such.
                    if (grid_type != LAMGRD3)
                              if (xcell < 100.0)
                        {
                        fixed = 1;
                            xorig *= 1000.0;
                            yorig *= 1000.0;
                            xcell *= 1000.0;
                            ycell *= 1000.0;
                            fprintf( stderr, "\n\nM3IO_parameter_fix() WARNING:\n"
                                 "Scaled probable kilometer units to meters."
                                 "  M3IO file should be fixed.\n\n" );
                        }
             ************************************************************ */

            if ( fixed )
                {
                sprintf ( map_info, "%d %g %g %g %g %g %g %g %g %g %d %d",
                          grid_type, xorig, yorig, xcell,
                          ycell, xcent, ycent, p_gam,
                          p_bet, p_alp, ncol, nrow );

                free ( info->map_info );
                info->map_info = strdup ( map_info );
                if ( !info->map_info )
                    fprintf ( stderr,
                              "Memory problem in M3IO_parameter_fix()!\n" );
                }

            break;

        case UNDETERMINED:
        default:
            fprintf ( stderr, "Unknown dataset type %d!\n", info->dataset );
        }
    }




/************************************************************
dump_VIS_DATA_to_netCDF_file - returns 1 if error
                 - regardless it frees up vdata before leaving
************************************************************/
int dump_VIS_DATA_to_netCDF_file ( VIS_DATA *vdata, char *fname, char *estring )
    {
    int   ni, nj, nk, nt;
    IOAPI_Bdesc3  ib3;
    IOAPI_Cdesc3  ic3;
    int       hour;
    char      *specname, netcdf_file[256];
    float         *data;
    static char   logicalName[256];
    int           gdtyp, utm_zone;
    float         xorig, yorig, xcell, ycell;
    float         xcent, ycent, p_gam, p_bet, p_alp;
    float         ne_x, ne_y;
    FILE *fp;
    enum dataset_type dataset;
    int i;
    char *export_varname;
#define DEFAULT_EXPORT_VARNAME "VAR"
#define EXPORT_VARNAME_ENV "PAVE_EXPORT_VARNAME"
    /* do a bunch of error checking on the arguments */
    if ( ( !vdata ) || ( !fname ) || ( !estring ) )
        {
        sprintf ( estring, "Bad args to dump_VIS_DATA_to_netCDF_file()!" );
        free_vis ( vdata );
        return errmsg ( estring );
        }

        {
        int k;
        printf ( "DEBUG: exporting netCDF: nspecies=%d\n",vdata->nspecies );
        for ( k = 0; k< vdata->nspecies; k++ )
            {
            printf ( "DEBUG: exporting netCDF: %s\n",vdata->species_short_name[k] );
            }
        }


    if ( !vdata->grid )
        {
        sprintf ( estring, "No grid values for dump_VIS_DATA_to_netCDF_file()!" );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    ni = vdata->col_max - vdata->col_min + 1;
    nj = vdata->row_max - vdata->row_min + 1;
    nk = vdata->level_max - vdata->level_min + 1;
    nt = vdata->step_max - vdata->step_min + 1;
    data = vdata->grid;

    if ( ( ni < 1 ) || ( nj < 1 ) || ( nk < 1 ) || ( nt < 1 ) )
        {
        sprintf ( estring, "Can't find ni, nj, and/or nt for netCDF_file!" );
        free_vis ( vdata );
        return errmsg ( estring );
        }

    /* actually write the file out */
        {
        init3c();
        sprintf ( netcdf_file, "NETCDF_FILE" );

        memset ( ( void * ) &ib3, 0, ( size_t ) sizeof ( IOAPI_Bdesc3 ) );
        memset ( ( void * ) &ic3, 0, ( size_t ) sizeof ( IOAPI_Cdesc3 ) );

        /* parse the map_info string */
        if ( sscanf ( vdata->map_info, "%d %g %g %g %g %g%g %g %g %g",
                      &gdtyp, &xorig, &yorig, &xcell, &ycell,
                      &xcent, &ycent, &p_gam, &p_bet, &p_alp ) == 10 )
            dataset = netCDF_DATA;
        else if ( sscanf ( vdata->map_info, "%g %g %g %g %d", &xorig, &yorig,
                           &ne_x, &ne_y, &utm_zone ) == 5 )
            dataset = UAM_DATA;
        else
            dataset = UNDETERMINED;

        switch ( dataset )
            {
            case netCDF_DATA:
                /*sscanf(vdata->map_info, "%d %g %g %g %g %g%g %g %g %g",
                   &gdtyp, &xorig, &yorig, &xcell, &ycell,
                   &xcent, &ycent, &p_gam, &p_bet, &p_alp);*/
                ib3.xcent = xcent;
                ib3.ycent = ycent;
                ib3.p_alp = p_alp;
                ib3.p_bet = p_bet;
                ib3.p_gam = p_gam;
                break;
            case UAM_DATA:
            case UAMV_DATA:
                gdtyp = UTMGRD3;
                /*sscanf(vdata->map_info, "%g %g %g %g %d",
                   &xorig, &yorig, &ne_x, &ne_y, &utm_zone);*/
                if ( yorig<90.0 ) gdtyp=LATGRD3;
                xcell = ( ne_x-xorig ) /vdata->ncol;
                ycell = ( ne_y-yorig ) /vdata->nrow;
                break;
            }
        /* make correction to the map_info if this was clipped region */
        xorig += xcell* ( vdata->col_min-1 );
        yorig += ycell* ( vdata->row_min-1 );
        ib3.xorig = xorig;
        ib3.yorig = yorig;
        ib3.xcell = xcell;
        ib3.ycell = ycell;
        ib3.gdtyp = gdtyp;
        ib3.ftype = GRDDED3;

        GETDTTIME ( &ib3.cdate, &ib3.ctime );
        ib3.sdate = vdata->sdate[0];
        ib3.stime = vdata->stime[0];
        ib3.tstep = sec2timec ( vdata->incr_sec );
        ib3.mxrec = nt;
        ib3.nvars = 1;
        ib3.ncols = ni;
        ib3.nrows = nj;
        ib3.nlays = nk;
        ib3.vtype[0] = M3REAL;
        export_varname = getenv ( EXPORT_VARNAME_ENV );
        if ( export_varname == NULL )
            {
            strcpy ( ic3.vname[0], DEFAULT_EXPORT_VARNAME );
            }
        else
            {
            for ( i=0; i<strlen ( export_varname ); i++ )
                {
                if ( !isalnum ( export_varname[i] ) ) export_varname[i]='_';
                }
            strncpy ( ic3.vname[0], export_varname, MXDESC3-2 );
            }

        strcpy ( ic3.units[0], vdata->units_name[0] );
        sprintf ( ic3.fdesc[0], "Original formula:%s", vdata->species_short_name[0] );

        /*
           logicalName is declared static as per the man pages
           for putenv's mention of a potential error:

           int putenv(char *string)

           A potential error is to  call  putenv()  with
           an  automatic variable  as  the  argument,
           then exit the calling function while string is
           still part of the environment.
           */
        sprintf ( logicalName, "%s=%s", netcdf_file, fname );
        putenv ( logicalName );
        /* check whether the file exists */
        fp = fopen ( fname,"r" );
        if ( fp != NULL )
            {
            fclose ( fp );
            sprintf ( estring,
                      "WARNING! The file %s already exists! PAVE will overwrite it\n",
                      fname );
            errmsg ( estring );
            fprintf ( stderr,"%s",estring );
            remove ( fname );
            }

        for ( hour=0; hour<nt; hour++ )
            {
            ib3.wdate = vdata->sdate[hour];
            ib3.wtime = vdata->stime[hour];
            if ( hour == 0 )
                {
                if ( !open3c ( netcdf_file, &ib3, &ic3, FSNEW3, "pave_exe" ) )
                    {
                    sprintf ( estring, "Couldn't open %s !", fname );
                    free_vis ( vdata );
                    return errmsg ( estring );
                    }
                }
            if ( !write3c ( netcdf_file, "ALL",
                            ib3.wdate, ib3.wtime, data ) )
                {
                sprintf ( estring, "Couldn't write to %s !", fname );
                free_vis ( vdata );
                return errmsg ( estring );
                }
            data += ni*nj*nk;
            }

        close3c ( netcdf_file );

        /*
        shut3c();
        */

        }

    /* clean up and return */
    free_vis ( vdata );
    return 0;
    }



/************************************************************
get_vdata_SelectedCellRange - added 961014 SRT
************************************************************/
char *get_vdata_SelectedCellRange ( VIS_DATA *vdata )
    {
    static char selectedCellRange[64];

    if ( !vdata )
        {
        fprintf ( stderr,
                  "\007NULL VIS_DATA *vdata passed to get_vdata_SelectedCellRange()!\n" );
        return "???";
        }
    if ( ( vdata->col_min == vdata->col_max ) &&
            ( vdata->row_min == vdata->row_max ) &&
            ( vdata->level_min == vdata->level_max ) )
        {
        sprintf ( selectedCellRange, "Cell (%d,%d,%d)",
                  vdata->col_min, vdata->row_min, vdata->level_min );
        }
    else
        {
        sprintf ( selectedCellRange, "Cells (%d,%d,%d)->(%d,%d,%d)",
                  vdata->col_min, vdata->row_min, vdata->level_min,
                  vdata->col_max, vdata->row_max, vdata->level_max );
        }
    return selectedCellRange;
    }



/************************************************************
get_vdata_TimeMinString - added 961014 SRT
************************************************************/
char *get_vdata_TimeMinString ( VIS_DATA *vdata )
    {
    static char timeMinString[64];

    if ( !vdata )
        {
        fprintf ( stderr,
                  "\007NULL VIS_DATA *vdata passed to get_vdata_TimeMinString()!\n" );
        return "???";
        }

    if ( ! ( vdata->sdate ) )
        {
        fprintf ( stderr,
                  "\007NULL vdata->sdate passed to get_vdata_TimeMinString()!\n" );
        return "???";
        }

    if ( ! ( vdata->stime ) )
        {
        fprintf ( stderr,
                  "\007NULL vdata->stime passed to get_vdata_TimeMinString()!\n" );
        return "???";
        }

    julian2shorttext ( timeMinString, vdata->sdate[0], vdata->stime[0] );
    return timeMinString;
    }


/************************************************************
get_vdata_TimeMaxString - added 961014 SRT
************************************************************/
char *get_vdata_TimeMaxString ( VIS_DATA *vdata )
    {
    static char timeMaxString[64];

    if ( !vdata )
        {
        fprintf ( stderr,
                  "\007NULL VIS_DATA *vdata passed to get_vdata_TimeMaxString()!\n" );
        return "???";
        }

    if ( ! ( vdata->sdate ) )
        {
        fprintf ( stderr,
                  "\007NULL vdata->sdate passed to get_vdata_TimeMaxString()!\n" );
        return "???";
        }

    if ( ! ( vdata->stime ) )
        {
        fprintf ( stderr,
                  "\007NULL vdata->stime passed to get_vdata_TimeMaxString()!\n" );
        return "???";
        }

    julian2shorttext ( timeMaxString, vdata->sdate[vdata->step_max-vdata->step_min],
                       vdata->stime[vdata->step_max-vdata->step_min] );
    return timeMaxString;
    }



/************************************************************
julian2shorttext
************************************************************/
void julian2shorttext ( char *string, int date, int time ) /* SRT 101496 added */
    {
    static int days_per_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int days_per_year;
    int year, day, hour, min, sec;
    register int i, j, k, found;

    year = date/1000;
    day = date - year * 1000;

    hour = time/10000;
    min = ( time - ( hour * 10000 ) ) /100;
    sec = time - ( hour * 10000 ) - ( min * 100 );

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    /* From Kernighan & Ritchie, p. 37
    "... a year is a leap year if it is divisible by 4 but not by 400,
         except that years divisible by 400 ARE leap years."
    */

    days_per_year = 365;
    days_per_month[1] = 28;
    if ( ( ! ( year % 4 ) ) && ( year % 100 ) ) /* leap year? */
        {
        days_per_month[1] = 29;
        days_per_year = 366;
        }
    if ( ! ( year % 400 ) )
        {
        days_per_month[1] = 29;
        days_per_year = 366;
        }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    found = j = k = 0;
    for ( i = 0; ( ( i < 12 ) && ( !found ) ); ++i )
        {
        j += days_per_month[i];
        if ( day <= j )
            {
            sprintf ( string, "%d/%d/%d %d:%02d:%02d", /* SRT removed 2 spaces before the time 960412 */
                      i+1, day - k, year%100, hour, min, sec );
            found = 1;
            }
        k = j;
        }
    if ( !found )
        sprintf ( string, "%d%d %2d:%02d:%02d:02d:02d JULIAN",
                  year, day, hour, min, sec );
    }




/************************************************************
makeSureIts_netCDF() -  If a VIS_DATA struct is of type UAM*_DATA,
            this converts it to netCDF_DATA, by
            adjusting the map_info and dataset fields
            appropriately.

                Returns 1 if error, placing an error
            message into estring.  Returns 0 otherwise.
************************************************************/
int makeSureIts_netCDF ( VIS_DATA *vdata, char *estring )
    {
    enum    { UNUSED = -1 };
    int     grid_type, ncol, nrow, utm_zone;
    float   llx, lly, urx, ury, xorig, yorig, xcell,
            ycell, xcent, ycent, p_gam, p_bet, p_alp;
    char    map_info[192];


    /* do a bunch of error checking on the arguments */
    if ( ( !vdata ) || ( !estring ) || ( !vdata->map_info ) )
        {
        sprintf ( estring, "Bad args to makeSureIts_netCDF()!" );
        return 1;
        }

    /* convert it, if necessary */
    switch ( vdata->dataset )
        {
        case UAMV_DATA:
            sscanf ( vdata->map_info, "%g%g%g%g%d%d%d",
                     &llx, &lly, &urx, &ury, &utm_zone, &ncol, &nrow );

            /* cheesy way to figure out LatLon vs UTM */
            if ( ( lly<90.0 ) && ( lly>0.0 ) && ( ury<90.0 ) && ( ury>0.0 ) )
                {
                grid_type  = LATGRD3;
                p_alp  = UNUSED;
                xcent  = UNUSED;
                ycent  = UNUSED;
                }
            else
                {
                grid_type  = UTMGRD3;
                p_alp  = utm_zone;
                xcent  = 0;
                ycent  = 0;
                }
            p_bet  = UNUSED;
            p_gam  = UNUSED;
            xorig  = llx;
            yorig  = lly;
            xcell  = ( urx-llx ) /ncol;
            ycell  = ( ury-lly ) /nrow;
            sprintf ( map_info, "%d %g %g %g %g %g %g %g %g %g %d %d",
                      grid_type, xorig, yorig, xcell,
                      ycell, xcent, ycent, p_gam,
                      p_bet, p_alp, ncol, nrow );
            free ( vdata->map_info );
            vdata->map_info = strdup ( map_info );
            if ( !vdata->map_info )
                {
                sprintf ( estring,
                          "Not enough memory in makeSureIts_netCDF()!" );
                return 1;
                }
            vdata->dataset = netCDF_DATA;
            break;

        case netCDF_DATA:
        case netCDF_OBS:
        case UAM_DATA:
            break;

        default:
            sprintf ( estring,
                      "Unknown dataset type in makeSureIts_netCDF()!" );
            return 1;
        }

    return 0;
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



/************************************************************
map_infos_areReasonablyEquivalent()

Tests to see whether the domains are equivalent enough to
use the same map background.  The need for this routine
arises because occasionally different programs write datasets
to disk, using a different precision to specify the domain.
Testing for "reasonable equality" rather than exact equality
allows PAVE to draw a map with data derived from multiple datasets
with the same domains but of slightly different precisions.

The alrogithm used here is, for two domains to be "reasonably
equal", the following conditions must be met

    1) the projection type (LATGRD3, LAMGRD3, UTMGRD3) must be the same

    2) NROWS and NCOLS must be the same

    3) all other parameters must be "reasonably equal", as defined
       by the is_reasonably_equal(p,q) macro in utils.h

Returns 1 if the two map_info strings are deemed
reasonably equivalent, otherwise returns 0 with an
error message in estring.
************************************************************/
int map_infos_areReasonablyEquivalent ( char *p, char *q, char *estring )
    {
    enum    { UNUSED = -1 };
    int     i, grid_type[2], ncol[2], nrow[2], utm_zone[2];
    float   llx[2], lly[2], urx[2], ury[2], xorig[2], yorig[2], xcell[2],
            ycell[2], xcent[2], ycent[2], p_gam[2], p_bet[2], p_alp[2];
    char    *v[2];

    if ( p == q ) return 1;

    if ( ( !p ) || ( !q ) ) return 0;

    if ( !strcmp ( p, q ) ) return 1;


    v[0] = p;
    v[1] = q;
    for ( i = 0; i < 2; i++ )
        {
        if ( sscanf ( v[i], "%d%f%f%f%f%f%f%f%f%f%d%d",
                      &grid_type[i], &xorig[i], &yorig[i], &xcell[i],
                      &ycell[i], &xcent[i], &ycent[i], &p_gam[i],
                      &p_bet[i], &p_alp[i], &ncol[i], &nrow[i] ) == 12 )
            {
#ifdef DIAGNOSTICS
            fprintf ( stderr, "'%s' is netCDF format!\n", v[i] );
            fprintf ( stderr, "I scanned %d %g %g %g %g %g %g %g %g %g %d %d\n",
                      grid_type[i], xorig[i], yorig[i], xcell[i],
                      ycell[i], xcent[i], ycent[i], p_gam[i],
                      p_bet[i], p_alp[i], ncol[i], nrow[i] );
#endif /* #ifdef DIAGNOSTICS */
            }
        else

            if ( sscanf ( v[i], "%f%f%f%f%d%d%d", &llx[i], &lly[i],
                          &urx[i], &ury[i], &utm_zone[i], &ncol[i], &nrow[i] ) == 7 )
                {
#ifdef DIAGNOSTICS
                fprintf ( stderr, "'%s' is UAM* format!\n", v[i] );
                fprintf ( stderr, "I scanned %g %g %g %g %d %d %d\n",
                          llx[i], lly[i], urx[i], ury[i], utm_zone[i], ncol[i], nrow[i] );
#endif /* #ifdef DIAGNOSTICS */

                /* cheesy way to figure out LatLon vs UTM */
                if ( ( lly[i]<90.0 ) && ( lly[i]>0.0 ) && ( ury[i]<90.0 ) && ( ury[i]>0.0 ) )
                    {
#ifdef DIAGNOSTICS
                    fprintf ( stderr, "'%s' is LATGRD3 projection!\n",
                              v[i] );
#endif /* #ifdef DIAGNOSTICS */
                    grid_type[i]  = LATGRD3;
                    p_alp[i]  = UNUSED;
                    xcent[i]  = UNUSED;
                    ycent[i]  = UNUSED;
                    }
                else
                    {
#ifdef DIAGNOSTICS
                    fprintf ( stderr, "'%s' is UTMGRD3 projection!\n",
                              v[i] );
#endif /* #ifdef DIAGNOSTICS */
                    grid_type[i]  = UTMGRD3;
                    p_alp[i]  = utm_zone[i];
                    xcent[i]  = 0;
                    ycent[i]  = 0;
                    }
                p_bet[i] = UNUSED;
                p_gam[i] = UNUSED;
                xorig[i] = llx[i];
                yorig[i] = lly[i];
                xcell[i] = ( urx[i]-llx[i] ) /ncol[i];
                ycell[i] = ( ury[i]-lly[i] ) /nrow[i];
                }
            else
                {
                sprintf ( estring, "Unknown dataset type in map_infos_areReasonablyEquivalent()!" );
                return 0;
                }

#ifdef DIAGNOSTICS
        fprintf ( stderr, "\nv[%d] == '%s'\n", i, v[i] );
        fprintf ( stderr, "grid_type == %d\n", grid_type[i] );
        fprintf ( stderr, "    p_alp == %g\n", p_alp[i] );
        fprintf ( stderr, "    p_bet == %g\n", p_bet[i] );
        fprintf ( stderr, "    p_gam == %g\n", p_gam[i] );
        fprintf ( stderr, "    xcent == %g\n", xcent[i] );
        fprintf ( stderr, "    ycent == %g\n", ycent[i] );
        fprintf ( stderr, "     nrow == %d\n", nrow[i] );
        fprintf ( stderr, "     ncol == %d\n", ncol[i] );
        fprintf ( stderr, "    xorig == %g\n", xorig[i] );
        fprintf ( stderr, "    yorig == %g\n", yorig[i] );
        fprintf ( stderr, "    xcell == %g\n", xcell[i] );
        fprintf ( stderr, "    ycell == %g\n", ycell[i] );
#endif /* #ifdef DIAGNOSTICS */
        }

    if ( grid_type[0] != grid_type[1] )
        {
        sprintf ( estring, "grid_type's of %d and %d are not equal!",
                  grid_type[0], grid_type[1] );
        return 0;
        }

    if ( ncol[0] != ncol[1] )
        {
        sprintf ( estring, "ncol's of %d and %d are not equal!", ncol[0], ncol[1] );
        return 0;
        }

    if ( nrow[0] != nrow[1] )
        {
        sprintf ( estring, "nrow's of %d and %d are not equal!", nrow[0], nrow[1] );
        return 0;
        }
    if ( !is_reasonably_equal ( xorig[0],xorig[1] ) )
        {
        sprintf ( estring,
                  "xorig's of %g and %g differ too much!",
                  xorig[0], xorig[1] );
        return 0;
        }
    if ( !is_reasonably_equal ( yorig[0],yorig[1] ) )
        {
        sprintf ( estring,
                  "yorig's of %g and %g differ too much!",
                  yorig[0], yorig[1] );
        return 0;
        }
    if ( !is_reasonably_equal ( xcell[0],xcell[1] ) )
        {
        sprintf ( estring,
                  "xcell's of %g and %g differ too much!",
                  xcell[0], xcell[1] );
        return 0;
        }
    if ( !is_reasonably_equal ( ycell[0],ycell[1] ) )
        {
        sprintf ( estring,
                  "ycell's of %g and %g differ too much!",
                  ycell[0], ycell[1] );
        return 0;
        }

    switch ( grid_type[0] )
        {
        case LAMGRD3:
        case ALBGRD3:
        case MERGRD3:
        case STEGRD3:
        case TRMGRD3:
            if ( !is_reasonably_equal ( p_alp[0],p_alp[1] ) )
                {
                sprintf ( estring,
                          "p_alp's of %g and %g differ too much!",
                          p_alp[0], p_alp[1] );
                return 0;
                }
            if ( !is_reasonably_equal ( p_bet[0],p_bet[1] ) )
                {
                sprintf ( estring,
                          "p_bet's of %g and %g differ too much!",
                          p_bet[0], p_bet[1] );
                return 0;
                }
            if ( !is_reasonably_equal ( p_gam[0],p_gam[1] ) )
                {
                sprintf ( estring,
                          "p_gam's of %g and %g differ too much!",
                          p_gam[0], p_gam[1] );
                return 0;
                }
            if ( !is_reasonably_equal ( xcent[0],xcent[1] ) )
                {
                sprintf ( estring,
                          "xcent's of %g and %g differ too much!",
                          xcent[0], xcent[1] );
                return 0;
                }
            if ( !is_reasonably_equal ( ycent[0],ycent[1] ) )
                {
                sprintf ( estring,
                          "ycent's of %g and %g differ too much!",
                          ycent[0], ycent[1] );
                return 0;
                }

        case EQMGRD3:
        case LEQGRD3:
            if ( !is_reasonably_equal ( p_alp[0],p_alp[1] ) )
                {
                sprintf ( estring,
                          "p_alp's of %g and %g differ too much!",
                          p_alp[0], p_alp[1] );
                return 0;
                }
            if ( !is_reasonably_equal ( p_gam[0],p_gam[1] ) )
                {
                sprintf ( estring,
                          "p_gam's of %g and %g differ too much!",
                          p_gam[0], p_gam[1] );
                return 0;
                }
            if ( !is_reasonably_equal ( xcent[0],xcent[1] ) )
                {
                sprintf ( estring,
                          "xcent's of %g and %g differ too much!",
                          xcent[0], xcent[1] );
                return 0;
                }
            if ( !is_reasonably_equal ( ycent[0],ycent[1] ) )
                {
                sprintf ( estring,
                          "ycent's of %g and %g differ too much!",
                          ycent[0], ycent[1] );
                return 0;
                }

        case UTMGRD3:
            if ( !is_reasonably_equal ( p_alp[0],p_alp[1] ) )
                {
                sprintf ( estring,
                          "p_alp's of %g and %g differ too much!",
                          p_alp[0], p_alp[1] );
                return 0;
                }

            if ( !is_reasonably_equal ( xcent[0],xcent[1] ) )
                {
                sprintf ( estring,
                          "xcent's of %g and %g differ too much!",
                          xcent[0], xcent[1] );
                return 0;
                }
            if ( !is_reasonably_equal ( ycent[0],ycent[1] ) )
                {
                sprintf ( estring,
                          "ycent's of %g and %g differ too much!",
                          ycent[0], ycent[1] );
                return 0;
                }

        case SINUGRD3:
            \
            if ( !is_reasonably_equal ( p_gam[0],p_gam[1] ) )
                {
                sprintf ( estring,
                          "p_gam's of %g and %g differ too much!",
                          p_gam[0], p_gam[1] );
                return 0;
                }
            if ( !is_reasonably_equal ( xcent[0],xcent[1] ) )
                {
                sprintf ( estring,
                          "xcent's of %g and %g differ too much!",
                          xcent[0], xcent[1] );
                return 0;
                }
            if ( !is_reasonably_equal ( ycent[0],ycent[1] ) )
                {
                sprintf ( estring,
                          "ycent's of %g and %g differ too much!",
                          ycent[0], ycent[1] );
                return 0;
                }

        case LATGRD3:
            break;

        default:
            sprintf ( estring, "Unknown grid type!" );
            return 0;
        }

    return 1;
    }
