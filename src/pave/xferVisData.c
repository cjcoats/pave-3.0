/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0 file VisData.c
 *
 *  File: $Id: xferVisData.c 83 2018-03-12 19:24:33Z coats $
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
 *      Author:      Rajini Balay, NCSU, rajini@aristotle.csc.ncsu.edu
 *      Date:        February 25, 1995
 *
 *      Version 02/2018 by Carlie J. Coats, Jr., Ph.D. for PAVE-3.0
 *      replaced gratuitous malloc()s by stack-based local variables.
 *****************************************************************************/

/* bald messes this up in some header file */
#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */

#include <malloc.h>
#include <stdlib.h>
#include <sys/types.h>    /* sys/types.h needed for netinet/in.h */
#include <netinet/in.h>
#include <unistd.h>     /* read, write, sleep  */

#include "visDataClient.h"
#include "busClient.h"
#include "busMsgQue.h"
#include "busError.h"
#include "busDebug.h"
#include "busXtClient.h"
#include "busRW.h"
#include "busVersion.h"
#include "busRpc.h"
#include "busUtil.h"

#define bcopy(s1, s2, len) memcpy(s2, s1, (size_t)len);

/* Function for sending the VIS_DATA structure over the socket
 */
int sendVisData ( int fd, VIS_DATA *info )
    {
    int i;
    int dataset, slice;
    int err;

#ifdef DIAGNOSTICS
    fprintf ( stderr, ":::::::::::::::::::::::::::::::::::::::::::::: \n" );
    fprintf ( stderr, "		SendVisData\n" );
    fprintf ( stderr, ":::::::::::::::::::::::::::::::::::::::::::::: \n" );

    fprintf ( stderr, "xferVisData.c's SendVisData() about to send:\n" );
    fflush ( stderr );
    dump_VIS_DATA ( info, NULL, NULL );
    fflush ( stderr );
    fflush ( stdout );
#endif /* DIAGNOSTICS */
    if ( ( err = sendString ( fd, info->filename, strlen ( info->filename ) ) ) == XFER_ERR )
        return err;

    dataset = info->dataset;
    if ( ( err = sendInteger ( fd, 1, &dataset ) ) == XFER_ERR )
        return err;
    if ( ( err = sendInteger ( fd, 1, &info->nspecies ) ) == XFER_ERR )
        return err;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Sending species information \n" );
#endif /* DIAGNOSTICS */

    for ( i=0; i<info->nspecies; i++ )
        {
        if ( ( err = sendString ( fd, info->species_short_name[i], strlen ( info->species_short_name[i] ) ) ) == XFER_ERR )
            return err;
        }
    for ( i=0; i<info->nspecies; i++ )
        {
        if ( ( err = sendString ( fd, info->species_long_name[i], strlen ( info->species_long_name[i] ) ) ) == XFER_ERR )
            return err;
        }
    for ( i=0; i<info->nspecies; i++ )
        {

        if ( ( info->units_name[i] ) && strlen ( info->units_name[i] ) ) /*added 951020 SRT*/
            {
            if ( ( err = sendString ( fd, info->units_name[i], strlen ( info->units_name[i] ) ) ) == XFER_ERR )
                return err;
            }
        else  /*added 951020 SRT*/
            if ( ( err = sendString ( fd, "\0", 0 ) ) == XFER_ERR ) /*added 951020 SRT*/
                return err;  /*added 951020 SRT*/
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Sending data_level and map_info \n" );
#endif /* DIAGNOSTICS */
    if ( info->map_info != NULL )
        {
        if ( ( err = sendString ( fd, info->map_info, strlen ( info->map_info ) ) ) == XFER_ERR )
            return err;
        }
    else if ( ( err = sendString ( fd, NULL, 0 ) ) == XFER_ERR )
        return err;

    if ( info->data_label != NULL )
        {
        if ( ( err = sendString ( fd, info->data_label, strlen ( info->data_label ) ) ) == XFER_ERR )
            return err;
        }
    else if ( ( err = sendString ( fd, NULL, 0 ) ) == XFER_ERR )
        return err;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Sending integers \n" );
#endif /* DIAGNOSTICS */

    if ( sendInteger ( fd, 1, &info->first_date ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->first_time ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->last_date  ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->last_time  ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->incr_sec   ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->ncol       ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->nrow       ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->nlevel     ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->nstep      ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->col_min    ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->col_max    ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->row_min    ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->row_max    ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->level_min  ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->level_max  ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->step_min   ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->step_max   ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->step_incr  ) == XFER_ERR )  return  XFER_ERR ;

    slice = info->slice;
    if ( sendInteger ( fd, 1, &slice ) == XFER_ERR                  )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->selected_species ) == XFER_ERR )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->selected_col ) == XFER_ERR     )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->selected_row ) == XFER_ERR     )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->selected_level ) == XFER_ERR   )  return  XFER_ERR ;
    if ( sendInteger ( fd, 1, &info->selected_step ) == XFER_ERR    )  return  XFER_ERR ;
    if ( sendFloat   ( fd, 1, &info->grid_min ) == XFER_ERR         )  return  XFER_ERR ;
    if ( sendFloat   ( fd, 1, &info->grid_max ) == XFER_ERR         )  return  XFER_ERR ;

    /* If information present in grid then transfer the fields */
    if ( info->slice != NONESLICE )
        {
        int num_dates, num_gridpts;
        int ncol, nrow, nlevel, nstep;

        if ( ( info->sdate != NULL ) && ( info->stime != NULL ) )
            {
            int one = 1;

#ifdef DIAGNOSTICS
            fprintf ( stderr, "Sending the Sdata Stime and Grid info ... \n" );
#endif /* DIAGNOSTICS */
            num_dates = ( ( info->step_max - info->step_min ) /info->step_incr )+1;

#ifdef DIAGNOSTICS
            fprintf ( stderr, "Number of dates = %d \n", num_dates );
#endif /* DIAGNOSTICS */

            if ( sendInteger ( fd, 1,         &one        ) == XFER_ERR )  return  XFER_ERR ;
            if ( sendInteger ( fd, num_dates, info->sdate ) == XFER_ERR )  return  XFER_ERR ;
            if ( sendInteger ( fd, num_dates, info->stime ) == XFER_ERR )  return  XFER_ERR ;
            }
        else
            {
            int zero;

            zero = 0;
            if ( sendInteger ( fd, 1, &zero ) == XFER_ERR )  return  XFER_ERR ;
            }

        if ( info->grid != NULL )
            {
            int one = 1;

            ncol   = info->col_max - info->col_min + 1;
            nrow   = info->row_max - info->row_min + 1;
            nlevel = info->level_max - info->level_min + 1;
            nstep  = ( ( info->step_max - info->step_min ) /info->step_incr ) + 1;
            num_gridpts = ncol * nrow * nlevel * nstep;

#ifdef DIAGNOSTICS
            fprintf ( stderr, "Number of grid points = %d \n", num_gridpts );
#endif /* DIAGNOSTICS */

            if ( sendInteger ( fd, 1, &one ) == XFER_ERR )  return  XFER_ERR ;
            if ( num_gridpts > 0 )
                {
                if ( sendFloat ( fd, num_gridpts, info->grid ) == XFER_ERR )  return  XFER_ERR ;
                }
            }
        else
            {
            int zero;

            zero = 0;
            if ( sendInteger ( fd, 1, &zero ) == XFER_ERR )  return  XFER_ERR ;
            }
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, ":::::::::::::::::::::::::::::::::::::::::::::: \n" );
#endif /* DIAGNOSTICS */
    return XFER_SUCCESS;
    }

/* Function for geting the VIS_DATA structure over the socket
 */
int getVisData ( int fd, VIS_DATA *info )
    {
    int i;
    int dataset, slice;
    int err;

#ifdef DIAGNOSTICS
    fprintf ( stderr, ":::::::::::::::::::::::::::::::::::::::::::::: \n" );
    fprintf ( stderr, "               GetVisData : \n" );
    fprintf ( stderr, ":::::::::::::::::::::::::::::::::::::::::::::: \n" );
#endif /* DIAGNOSTICS */

    if ( ( err = getString ( fd, &info->filename ) ) == XFER_ERR )
        return err;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Filename got = %s \n", info->filename );
#endif /* DIAGNOSTICS */

    if ( ( err = getInteger ( fd, &dataset ) ) == XFER_ERR )
        return err;
    info->dataset = dataset;
    if ( ( err = getInteger ( fd, &info->nspecies ) ) == XFER_ERR )
        return err;

    if ( info->nspecies != 0 )
        {
        info->species_short_name = ( char ** ) malloc ( info->nspecies*sizeof ( char ** ) );
        info->species_long_name  = ( char ** ) malloc ( info->nspecies*sizeof ( char ** ) );
        if ( info->units_name ) free ( info->units_name );
        info->units_name=NULL; /*SRT 971223*/
        info->units_name = ( char ** ) malloc ( info->nspecies*sizeof ( char ** ) );

        /* added malloc check SRT */
        if ( ( info->species_short_name == NULL ) ||
             ( info->species_long_name == NULL ) ||
             ( info->units_name == NULL ) )
            {
            if ( info->species_short_name ) free ( info->species_short_name );
            if ( info->species_long_name  ) free ( info->species_long_name );
            if ( info->units_name         ) free ( info->units_name );
            return XFER_ERR;
            }

        /* added SRT 951024 */
        for ( i = 0; i < ( *info ).nspecies; i++ )
            {
            ( *info ).species_short_name[i] = NULL;
            ( *info ).species_long_name[i] = NULL;
            ( *info ).units_name[i] = NULL;
            }
        }

    for ( i=0; i<info->nspecies; i++ )
        if ( getString ( fd, &info->species_short_name[i] ) == XFER_ERR )  return  XFER_ERR ;
    for ( i=0; i<info->nspecies; i++ )
        if ( getString ( fd, &info->species_long_name[i] )  == XFER_ERR )  return  XFER_ERR ;
    for ( i=0; i<info->nspecies; i++ )
        if ( getString ( fd, &info->units_name[i]         ) == XFER_ERR )  return  XFER_ERR ;
    if ( getString ( fd, &info->map_info   ) == XFER_ERR )  return  XFER_ERR ;
    if ( getString ( fd, &info->data_label ) == XFER_ERR )  return  XFER_ERR ;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Getting integers \n" );
#endif /* DIAGNOSTICS */
    if ( getInteger ( fd, &info->first_date ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->first_time ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->last_date  ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->last_time  ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->incr_sec   ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->ncol       ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->nrow       ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->nlevel     ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->nstep      ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->col_min    ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->col_max    ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->row_min    ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->row_max    ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->level_min  ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->level_max  ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->step_min   ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->step_max   ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->step_incr  ) == XFER_ERR )  return  XFER_ERR ;

    if ( getInteger ( fd, &slice ) == XFER_ERR )  return  XFER_ERR ;
    info->slice = slice;

    if ( getInteger ( fd, &info->selected_species ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->selected_col     ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->selected_row     ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->selected_level   ) == XFER_ERR )  return  XFER_ERR ;
    if ( getInteger ( fd, &info->selected_step    ) == XFER_ERR )  return  XFER_ERR ;
    if ( getFloat   ( fd, &info->grid_min         ) == XFER_ERR )  return  XFER_ERR ;
    if ( getFloat   ( fd, &info->grid_max         ) == XFER_ERR )  return  XFER_ERR ;


#ifdef DIAGNOSTICS
    fprintf ( stderr, "xferVisData.c's getVisData() just received:\n" );
    fflush ( stderr );
    dump_VIS_DATA ( info, NULL, NULL );
    fflush ( stderr );
    fflush ( stdout );
#endif /* DIAGNOSTICS */

    if ( info->nstep )
        {
#ifdef DIAGNOSTICS
        fprintf ( stderr, "xferVisData.c's getVisData(): allocating Sdate, Stime\n" );
        fflush ( stderr );
#endif /* DIAGNOSTICS */
        info->sdate = ( int * ) malloc ( sizeof ( int ) *info->nstep );
        if ( !info->sdate ) return XFER_ERR; /* added 950801 SRT */
        info->stime = ( int * ) malloc ( sizeof ( int ) *info->nstep );
        if ( !info->stime ) return XFER_ERR; /* added 950801 SRT */
        }
    /* If information present in grid then transfer the fields */
    if ( info->slice != NONESLICE )
        {
        int i;
        int num_dates, num_gridpts;
        int ncol, nrow, nlevel, nstep;
        int coming = 0;

        if ( ( err = getInteger ( fd, &coming ) ) == XFER_ERR )
            return err;
        if ( coming )
            {
#ifdef DIAGNOSTICS
            fprintf ( stderr, "Reading the Sdata Stime and Grid info ... \n" );
#endif /* DIAGNOSTICS */
            num_dates = ( ( info->step_max - info->step_min ) /info->step_incr )+1;

#ifdef DIAGNOSTICS
            fprintf ( stderr, "Number of dates = %d \n",num_dates );
#endif /* DIAGNOSTICS */
            if ( !info->sdate )
                {
                info->sdate = ( int * ) malloc ( sizeof ( int ) *num_dates );
                if ( !info->sdate ) return XFER_ERR; /* added 950801 SRT */
                }
            if ( !info->stime )
                {
                info->stime = ( int * ) malloc ( sizeof ( int ) *num_dates );
                if ( !info->stime ) return XFER_ERR; /* added 950801 SRT */
                }

            if ( ( err = getIntegers ( fd, info->sdate, num_dates ) ) == XFER_ERR )
                return err;
            if ( ( err = getIntegers ( fd, info->stime, num_dates ) ) == XFER_ERR )
                return err;
            }

        if ( ( err = getInteger ( fd, &coming ) ) == XFER_ERR )
            return err;
        if ( coming )
            {
            ncol   = info->col_max - info->col_min + 1;
            nrow   = info->row_max - info->row_min + 1;
            nlevel = info->level_max - info->level_min + 1;
            nstep  = ( ( info->step_max - info->step_min ) /info->step_incr ) + 1;
            num_gridpts = ncol * nrow * nlevel * nstep;
#ifdef DIAGNOSTICS
            fprintf ( stderr, "Number of grid points = %d \n", num_gridpts );
#endif /* DIAGNOSTICS */

            info->grid = ( float * ) malloc ( sizeof ( float ) *num_gridpts );
            if ( !info->grid ) return XFER_ERR; /* added 950801 SRT */
            if ( ( err = getFloats ( fd, info->grid, num_gridpts ) ) == XFER_ERR )
                return err;
            }
        }
#ifdef DIAGNOSTICS
    fprintf ( stderr, ":::::::::::::::::::::::::::::::::::::::::::::: \n" );
#endif /* DIAGNOSTICS */
    return XFER_SUCCESS;
    }

#ifdef IMA_CRAY

/* Function for writing an array of integers on the socket if the
   current machine is a CRAY */
int sendInteger ( int fd, int n, int *nums )
    {
    int code = 1;
    int bitoff = 0;
    char *buffer;
    int err;

    buffer = malloc ( 4*n );
    if ( CRAY2IEG ( &code, &n, buffer, &bitoff, nums ) == 0 )
        err = writeSleepLoop ( fd, buffer, 4*n );
    free ( buffer );
    if ( err < 0 )
        return XFER_ERR;
    else
        return XFER_SUCCESS;
    }

/* Function for writing an array of floats on the socket if the
   current machine is a CRAY */
int sendFloat ( int fd, int n, float *nums )
    {
    int code = 2;
    int bitoff = 0;
    char *buffer;
    int err;

    buffer = malloc ( 4*n );
    if ( CRAY2IEG ( &code, &n, buffer, &bitoff, nums ) == 0 )
        err = writeSleepLoop ( fd, buffer, 4*n );
    free ( buffer );
    if ( err < 0 )
        return XFER_ERR;
    else
        return XFER_SUCCESS;
    }

int getInteger ( int fd, int *i )
    {
    return getIntegers ( fd, i, 1 );
    }

/* Function for receiving an array of integers on the socket if the
   current machine is a CRAY */
int getIntegers ( int fd, int *nums, int n )
    {
    int code = 1;
    int bitoff = 0;
    char *buffer;
    int err;

    buffer = malloc ( 4*n );
    err = readSleepLoop ( fd, buffer, 4*n );
    if ( err < 0 )
        {
        free ( buffer );
        return XFER_ERR;
        }
    if ( IEG2CRAY ( &code, &n, buffer, &bitoff, nums ) != 0 )
        printf ( "ERROR in IEG2CRAY for getIntegers \n" );
    free ( buffer );
    return XFER_SUCCESS;
    }

int getFloat ( int fd, float *fl )
    {
    return getFloats ( fd, fl, 1 );
    }

/* Function for receiving an array of floats on the socket if the
   current machine is a CRAY */
int getFloats ( int fd, float *nums, int n )
    {
    int code = 2;
    int bitoff = 0;
    char *buffer;
    int err;

    buffer = malloc ( 4*n );
    if ( ( err = readSleepLoop ( fd, buffer, 4*n ) ) < 0 )
        {
        free ( buffer );
        return XFER_ERR;
        }
    if ( IEG2CRAY ( &code, &n, buffer, &bitoff, nums ) != 0 )
        printf ( "ERROR in IEG2CRAY for getFloats \n" );
    free ( buffer );
    return XFER_SUCCESS;
    }
#else
/* Function for writing an array of integers on the socket if the
   current machine is NOT a CRAY */
int sendInteger ( int fd, int n, int *nums )
    {
    int i;
    int tmp_array[n];
    int err;

    for ( i=0; i<n; i++ )
        {
        tmp_array[i] = htonl ( nums[i] );
        }
    err = writeSleepLoop ( fd, ( char * ) tmp_array, 4*n );

    if ( err < 0 )
        return XFER_ERR;
    else
        return XFER_SUCCESS;
    }

/* Function for writing an array of floats on the socket if the
   current machine is NOT a CRAY */
int sendFloat ( int fd, int n, float *nums )
    {
    int i;
    int tmp_int;
    int tmp_array[n];
    int err;

    for ( i=0; i<n; i++ )
        {
        bcopy ( ( char * ) &nums[i], ( char * ) &tmp_int, 4 );
        tmp_array[i] = htonl ( tmp_int );
        }
    err = writeSleepLoop ( fd, ( char * ) tmp_array, 4*n );
    if ( err < 0 )
        return XFER_ERR;
    else
        return XFER_SUCCESS;
    }

/* Function for receiving an integer on the socket if the
   current machine is NOT a CRAY */
int getInteger ( int fd, int *i )
    {
    char buf[4];
    int inbuf;
    int err;

    if ( ( err = readSleepLoop ( fd, ( char * ) &inbuf, 4 ) ) < 0 )
        return XFER_ERR;
    *i = ntohl ( inbuf );
    return XFER_SUCCESS;
    }

/* Function for receiving an array of integers on the socket if the
   current machine is NOT a CRAY */
int getIntegers ( int fd, int *nums, int n )
    {
    char *tmp_array;
    int i;
    int err;

    tmp_array = ( char * ) malloc ( n*4 );
    if ( ( err = readSleepLoop ( fd, ( char * ) tmp_array, 4*n ) ) < 0 )
        {
        free ( tmp_array );
        return XFER_ERR;
        }

    for ( i=0; i<n; i++ )
        {
        char buf[4];
        int x;

        buf[0] = tmp_array[i*4];
        buf[1] = tmp_array[i*4+1];
        buf[2] = tmp_array[i*4+2];
        buf[3] = tmp_array[i*4+3];

        nums[i] = ntohl ( * ( ( int * ) buf ) );
        }
    free ( tmp_array );
    return XFER_SUCCESS;
    }

/* Function for receiving an array of floats on the socket if the
   current machine is NOT a CRAY */
int getFloats ( int fd, float *nums, int n )
    {
    char *tmp_array;
    int i, tmp_int;
    int err;

    tmp_array = malloc ( n*4 );
    if ( ( err = readSleepLoop ( fd, ( char * ) tmp_array, 4*n ) ) < 0 )
        {
        free ( tmp_array );
        return XFER_ERR;
        }

    for ( i=0; i<n; i++ )
        {
        char buf[4];
        int x;

        buf[0] = tmp_array[i*4];
        buf[1] = tmp_array[i*4+1];
        buf[2] = tmp_array[i*4+2];
        buf[3] = tmp_array[i*4+3];

        tmp_int = ntohl ( * ( ( int * ) buf ) );
        bcopy ( ( char * ) &tmp_int, ( char * ) &nums[i], 4 );
        }
    free ( tmp_array );
    return XFER_SUCCESS;
    }

int getFloat ( int fd, float *fl )
    {
    char buf[4];
    int x, inbuf;
    int err;

    if ( ( err = readSleepLoop ( fd, ( char * ) &inbuf, 4 ) ) < 0 )
        return XFER_ERR;
    x = ntohl ( inbuf );
    bcopy ( ( char * ) &x, ( char * ) fl, 4 );
    /* printf("Got float : %f \n", *fl); */
    return XFER_SUCCESS; /* added SRT 950706 */
    }

#endif

int readSleepLoop ( int fd, char *inbuf, int length )
    {
    int sum, err;

    sum = 0;
    err = 0;
    do
        {
        if ( sum>0 )
            sleep ( 0 );
        err  = read ( fd, inbuf+sum, length-sum );
        sum += err;
        }
    while ( sum<length && ( err>=0 && sum>0 ) );
    return err;
    }

int writeSleepLoop ( int fd, char *outbuf, int length )
    {
    int sum, err;

    sum = 0;
    err = 0;
    do
        {
        if ( sum>0 )
            sleep ( 0 );
        err = write ( fd, outbuf+sum, length-sum );
        sum += err;
        }
    while ( sum<length && err>=0 && sum>0 );

    return err;
    }

int getString ( int fd, char **buf )
    {

    int len;
    int err;

    if ( ( err = getInteger ( fd, &len ) ) == XFER_ERR )
        return err;
    if ( len > 0 )
        {
        *buf = ( char * ) malloc ( len+1 );
        if ( ( err = readSleepLoop ( fd, *buf, len ) ) == XFER_ERR )
            return err;
        ( *buf ) [len] = '\0';
        }
    return XFER_SUCCESS;
    }

int sendString ( int fd, char *buf, int len )
    {
    int sum, err;

    if ( ( err = sendInteger ( fd, 1, &len ) ) == XFER_ERR )
        return err;
    if ( len > 0 )
        {
        if ( ( err = writeSleepLoop ( fd, buf, len ) ) == XFER_ERR )
            return err;
        }
    return XFER_SUCCESS;
    }
