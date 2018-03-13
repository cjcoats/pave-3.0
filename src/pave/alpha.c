/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: alpha.c 83 2018-03-12 19:24:33Z coats $
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
 *      Author:  Kathy Pearson, MCNC December 1, 1994
 *      KLP  01/31/95
 *      SRT  04/06/95   Added #ifdef __cplusplus lines
 *      CJC  02/27/2018 Version for PAVE-3.0
 *****************************************************************************/


/* in order to get the linker to resolve Kathy's
   subroutines when using CC to compile */
#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
/* SRT 950915 for DEC Alpha compilation #include <sys/unistd.h> */
#include <sys/stat.h>
#include <fcntl.h>

#include "netcdf.h"
#include "readuam.h"
#include "vis_data.h"
#include "utils.h"
#include "parms3.h"

/* in order to get the linker to resolve Kathy's
   subroutines when using CC to compile */
#ifdef __cplusplus
    }
#endif /* #ifdef __cplusplus */


/* void net2julian(); */
/* void date2julian(); */
void net2julian ( int sdate, int stime, int tstep_sec, int record,
                  int *jdate, int *jtime );
void date2julian ( int inputyear, int inputday,
                   int inputhour, int inputmin, int inputsec, int *jdate, int *jtime );


/*******************************************************************/
/* alpha_open                                                      */
/* Function: open input file and determine filesize and filetype   */
/* On Error: return FAILURE and write Error string into message    */
/* If no Error: return PAVE_SUCCESS                                     */
/*******************************************************************/
int alpha_open ( int *vis_fd, VIS_DATA *info, char *message )
    {
    int local_fd;
    struct stat statbuf;            /* UNIX file status buffer */
    register int istat;         /* I/O status falpha UNIX stat call */
    register int status;            /* alpha_open function return status */
    static char buffer[32];
    /* ------------------------------------------ */
    /* -- file can be BOTH online AND offline! -- */
    int online;                     /* requstded data file is online */
    int offline;                    /* requstded data file is offline */
    /* ------------------------------------------ */

    status = FAILURE;           /* assume FAILURE */

    /* check file migration status */

    if ( !get_migrate_state ( ( *info ).filename, &online, &offline ) )
        {
        sprintf ( message, "Error! Cannot get migration state of file %s.\n",
                  ( *info ).filename );
        return ( FAILURE );
        }
    if ( !online )
        fprintf ( stdout, "Retrieving migrated file ... expect a delay!\n" );


    /*
    The installation of netcdf by default sets ncopts to include NC_FATAL,
    which causes an abort if the caller tries to open a file that is not in
    netCDF format.  To avoid that, ncopts is reset here to include only the
    NC_VERBOSE flag.
    */

    ncopts = ( NC_VERBOSE );

    if ( ( local_fd = open ( ( *info ).filename, O_RDONLY ) ) != IOERROR )
        {
        /*
           If the file was migrated, let the user know that the file has been
           migrated back in and is available.
        */

        if ( !online )
            fprintf ( stdout,
                      "Migrated file %s now available\n", ( *info ).filename );

        if ( ( istat = stat ( ( *info ).filename, &statbuf ) ) != IOERROR )
            {
            if ( statbuf.st_size > 0 )
                {
                if ( read ( local_fd, buffer, 3 ) == 3 )
                    {
                    if ( (
                                ( buffer[0] == 'C' ) &&
                                ( buffer[1] == 'D' ) &&
                                ( buffer[2] == 'F' ) ) )
                        {
                        if ( ( *vis_fd = ncopen ( ( *info ).filename,
                                                  NC_NOWRITE ) ) != NC_SYSERR )
                            {
                            ( *info ).dataset = netCDF_DATA;
                            status = PAVE_SUCCESS;
                            }
                        else
                            {
                            sprintf ( message, "%s",
                                      "Cannot access as netCDF file." );
                            }
                        }
                    else
                        {
                        sprintf ( message, "%s",
                                  "File is not recognized as netCDF." );
                        }
                    }
                else
                    {
                    sprintf ( message, "%s",
                              "Cannot read first 3 characters of file." );
                    }
                }
            else
                {
                sprintf ( message, "%s", "File size of zero" );
                }
            }
        else
            {
            sprintf ( message, "%s", "Cannot get file status" );
            }
        }
    close ( local_fd );
    return ( status );
    }

int alpha_inquire ( VIS_DATA *info, char *message, int inquiring )
    {
    static char dimname[32];    /* name of netCDF dimension */
    static char varname[32];    /* name of netCDF variable */
    static char units[32];      /* units applied to current variable */
    static int vardim[32];      /* dimensions of netCDF variables */
    int ndims;      /* # dimensions in netCDF file */
    int nvars;      /* # variables in netCDF file */
    int ngatts;     /* # global attributes in netCDF file */
    int recdim;     /* id of unlimited dimension in netCDF file */
    int nvardims;       /* # dims for single variable in netCDF file */
    int nvaratts;       /* # atts for single variable in netCDF file */
    long dimsize;       /* size of an individual dimension */
    int spos;           /* species list character position */
    int upos;           /* units list character position */
    nc_type datatype;           /* netCDF data type */
    register int i, j, k, l;
    int vis_fd;
    int sdate;          /* start date */
    int stime;          /* start time */
    int tstep;          /* time step increment */
    int tstep_sec;
    int hour;
    int min;
    int sec;

    char *species_short_list = NULL;
    char *units_list = NULL;

    /* netCDF map attributes */

    int grid_type;
    double xcell;
    double ycell;
    double xorig;
    double yorig;
    double xcent;
    double ycent;
    double p_alp;
    double p_bet;
    double p_gam;

    int ftype, nrows, ncols, nthik;

    if ( !alpha_open ( &vis_fd, info, message ) )
        {
        sprintf ( message, "Error opening file %s\n", ( *info ).filename );
        goto INQUIRE_FAILURE;
        }
    if ( ( ncinquire ( vis_fd, &ndims, &nvars, &ngatts, &recdim ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncinquire" );
        goto INQUIRE_FAILURE;
        }
    if ( ( ncattget ( vis_fd, NC_GLOBAL, "FTYPE", &ftype ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s",  "Error calling ncattget" );
        goto INQUIRE_FAILURE;
        }
    if ( ftype == 2 ) /* Boundary data */
        {
        for ( i = 0; i < ndims; i++ )
            {
            if ( ( ncdiminq ( vis_fd, i, dimname, &dimsize ) ) == NC_SYSERR )
                {
                sprintf ( message, "%s", "Error calling ncdiminq" );
                goto INQUIRE_FAILURE;
                }
            if ( !strcmp ( dimname, "TSTEP" ) )
                ( *info ).nstep = ( int ) dimsize;
            else if ( !strcmp ( dimname, "LAY" ) )
                ( *info ).nlevel = ( int ) dimsize;
            }
        if ( ncattget ( vis_fd, NC_GLOBAL, "NTHIK", &nthik ) == NC_SYSERR )
            {
            sprintf ( message, "%s",  "Error calling ncattget" );
            goto INQUIRE_FAILURE;
            }
        if ( ncattget ( vis_fd, NC_GLOBAL, "NCOLS", &ncols ) == NC_SYSERR )
            {
            sprintf ( message, "%s",  "Error calling ncattget" );
            goto INQUIRE_FAILURE;
            }
        if ( ncattget ( vis_fd, NC_GLOBAL, "NROWS", &nrows ) == NC_SYSERR )
            {
            sprintf ( message, "%s",  "Error calling ncattget" );
            goto INQUIRE_FAILURE;
            }
        info->ncol = ncols + 2*nthik;
        info->nrow = nrows + 2*nthik;
        }
    else
        {
        for ( i = 0; i < ndims; i++ )
            {
            if ( ( ncdiminq ( vis_fd, i, dimname, &dimsize ) ) == NC_SYSERR )
                {
                sprintf ( message, "%s", "Error calling ncdiminq" );
                goto INQUIRE_FAILURE;
                }
            if ( !strcmp ( dimname, "TSTEP" ) )
                ( *info ).nstep = ( int ) dimsize;
            else if ( !strcmp ( dimname, "LAY" ) )
                ( *info ).nlevel = ( int ) dimsize;
            else if ( !strcmp ( dimname, "ROW" ) )
                ( *info ).nrow = ( int ) dimsize;
            else if ( !strcmp ( dimname, "COL" ) )
                ( *info ).ncol = ( int ) dimsize;
            }
        }

    if ( ftype == -1 && info->nrow==0 )
        {
        info->nrow=info->ncol;
        info->ncol=1;
        info->dataset = netCDF_OBS;
        }
    if ( ( ( *info ).nstep == 0 ) || ( ( *info ).nlevel == 0 ) ||
            ( ( *info ).nrow == 0 ) || ( ( *info ).ncol == 0 ) )
        {
        sprintf ( message,
                  ":-(Unexpected dimensions! Expecting non-zero ROW-COL-LEVEL-STEP\n"
                  "grid values!  Got TSTEP=%d, LAY=%d, ROW=%d, COL=%d\n",
                  ( *info ).nstep, ( *info ).nlevel, ( *info ).nrow, ( *info ).ncol
                );
        goto INQUIRE_FAILURE;
        }
    if ( !inquiring )
        goto INQUIRE_SUCCESS;

    ( *info ).col_min = 1;
    ( *info ).col_max = ( *info ).ncol;
    ( *info ).row_min = 1;
    ( *info ).row_max = ( *info ).nrow;
    ( *info ).level_min = 1;
    ( *info ).level_max = ( *info ).nlevel;
    ( *info ).step_min = 1;
    ( *info ).step_max = ( *info ).nstep;
    ( *info ).step_incr = 1;

    ( *info ).slice = NONESLICE;
    ( *info ).selected_species = 1;
    ( *info ).selected_col = 1;
    ( *info ).selected_row = 1;
    ( *info ).selected_level = 1;
    ( *info ).selected_step = 1;

    /* Don't restuff pointers that are not NULL!!!! */
    if (
        ( ( *info ).species_long_name != NULL ) ||
        ( ( *info ).species_short_name != NULL ) ||
        ( ( *info ).units_name != NULL ) ||
        ( ( *info ).sdate != NULL ) ||
        ( ( *info ).stime != NULL ) ||
        ( ( *info ).map_info != NULL ) ||
        ( ( *info ).data_label != NULL )
    )
        goto INQUIRE_SUCCESS;

    if ( species_short_list != NULL )
        free ( species_short_list );
    if ( ( species_short_list =
                ( ( char * ) malloc ( sizeof ( char ) * ( nvars * 32 ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for list.\n" );
        goto INQUIRE_FAILURE;
        }
    if ( units_list != NULL )
        free ( units_list );
    if ( ( units_list =
                ( ( char * ) malloc ( sizeof ( char ) * ( nvars * 32 ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for list.\n" );
        goto INQUIRE_FAILURE;
        }

    spos = 0;
    upos = 0;
    ( *info ).nspecies = 0;
    for ( i = 0; i < nvars; ++i )
        {
        if ( ( ncvarinq ( vis_fd, i, varname, &datatype, &nvardims,
                          vardim, &nvaratts ) ) == NC_SYSERR )
            {
            sprintf ( message, "%s", "Error calling ncvarinq" );
            goto INQUIRE_FAILURE;
            }
        if ( strcmp ( varname, "TFLAG" ) ) /* ignore TFLAG variables */
            {
            sprintf ( species_short_list+spos, "%s%c", varname, ':' );
            spos += strlen ( varname ) + 1;
            ++ ( *info ).nspecies;

            if ( ( ncattget ( vis_fd, i, "units", units ) ) == NC_SYSERR )
                {
                sprintf ( message, "%s",  "Error calling ncattget" );
                goto INQUIRE_FAILURE;
                }

            /* strip trailing blanks from units string */

            j = strlen ( units );
            for ( k = j-1; k >= 0; --k )
                {
                if ( units[k] == ' ' )
                    units[k] = '\0';
                else
                    break;
                }

            sprintf ( units_list+upos, "%s%c", units, ':' );
            upos += strlen ( units ) + 1;
            }
        }

    if ( ( *info ).species_short_name != NULL )
        free ( ( *info ).species_short_name );
    if ( ( ( *info ).species_short_name =
                ( ( char ** ) malloc ( sizeof ( char * ) * ( *info ).nspecies ) ) ) == NULL )
        {
        sprintf ( message, "Error allocating short species list string" );
        goto INQUIRE_FAILURE;
        }

    if ( ( *info ).species_long_name != NULL )
        free ( ( *info ).species_long_name );
    if ( ( ( *info ).species_long_name =
                ( ( char ** ) malloc ( sizeof ( char * ) * ( *info ).nspecies ) ) ) == NULL )
        {
        sprintf ( message, "Error allocating long species list string" );
        goto INQUIRE_FAILURE;
        }
    /* KLP 1/31/95 default individual elements of double char pointers to NULL */

    for ( i = 0; i < ( *info ).nspecies; i++ )
        ( *info ).species_short_name[i] = NULL;
    for ( i = 0; i < ( *info ).nspecies; i++ )
        ( *info ).species_long_name[i] = NULL;
    /* * * */


    l = 0;
    k = 0;
    j = strlen ( species_short_list );
    for ( i = 0; i < j; i++ )
        {
        if ( species_short_list[i] == ':' )
            {
            species_short_list[i] = '\0';
            if ( ( ( *info ).species_short_name[l] =
                        ( ( char * ) malloc ( sizeof ( char ) * 32 ) ) ) == NULL )
                {
                sprintf ( message,
                          "Error allocating memory for short species %d", i );
                goto INQUIRE_FAILURE;
                }
            strcpy ( ( *info ).species_short_name[l], species_short_list+k );

            /* For now, set long species name list = short species name list */

            if ( ( ( *info ).species_long_name[l] =
                        ( ( char * ) malloc ( sizeof ( char ) * 32 ) ) ) == NULL )
                {
                sprintf ( message,
                          "Error allocating memory for long species %d", i );
                goto INQUIRE_FAILURE;
                }
            strcpy ( ( *info ).species_long_name[l], species_short_list+k );
            ++l;
            k = i + 1;
            }
        }

    if ( ( *info ).units_name != NULL )
        free ( ( *info ).units_name );
    if ( ( ( *info ).units_name =
                ( ( char ** ) malloc ( sizeof ( char * ) * ( *info ).nspecies ) ) ) == NULL )
        {
        sprintf ( message, "Error allocating units list string" );
        goto INQUIRE_FAILURE;
        }
    /* KLP 1/31/95 default individual elements of double char pointers to NULL */
    for ( i = 0; i < ( *info ).nspecies; i++ )
        ( *info ).units_name[i] = NULL;


    l = 0;
    k = 0;
    j = strlen ( units_list );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "alpha.c about to assign units !\n" );
#endif /* DIAGNOSTICS */

    for ( i = 0; i < j; i++ )
        {
        if ( units_list[i] == ':' )
            {
            units_list[i] = '\0';

            if ( ( ( *info ).units_name[l] =
                        ( ( char * ) malloc ( sizeof ( char ) * 32 ) ) ) == NULL )
                {
                sprintf ( message,
                          "Error allocating memory for units for species %d", i );
                goto INQUIRE_FAILURE;
                }
            strcpy ( ( *info ).units_name[l], units_list+k );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "alpha.c just set units_name[%d]='%s'\n", l, ( *info ).units_name[l] );
#endif /* DIAGNOSTICS */
            ++l;
            k = i + 1;
            }
        }
    if
    (
        ( ( ncattget ( vis_fd, NC_GLOBAL, "SDATE", &sdate ) ) == NC_SYSERR ) ||
        ( ( ncattget ( vis_fd, NC_GLOBAL, "STIME", &stime ) ) == NC_SYSERR ) ||
        ( ( ncattget ( vis_fd, NC_GLOBAL, "TSTEP", &tstep ) ) == NC_SYSERR )
    )
        {
        sprintf ( message, "%s", "Error getting date attributes" );
        goto INQUIRE_FAILURE;
        }
    else
        {

        /* tstep is in HHMMSS form -- convert to seconds */

        hour = ( ( int ) tstep ) /10000;
        min = ( ( ( int ) tstep ) - ( hour * 10000 ) ) /100;
        sec = ( ( int ) tstep ) - ( hour * 10000 ) - ( min * 100 );
        tstep_sec = hour * 3600 + min * 60 + sec;
        ( *info ).incr_sec = tstep_sec;

        /* get Julian time and date for first record and last record */

        net2julian ( ( int ) sdate, ( int ) stime, tstep_sec, 1,
                     & ( ( *info ).first_date ), & ( ( *info ).first_time ) );
        net2julian ( ( int ) sdate, ( int ) stime, tstep_sec, ( *info ).nstep,
                     & ( ( *info ).last_date ), & ( ( *info ).last_time ) );
        }


    if ( ( *info ).sdate != NULL )
        free ( ( *info ).sdate );
    if ( ( ( *info ).sdate = ( ( int * ) malloc ( sizeof ( int ) * ( info->nstep ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for dates.\n" );
        goto INQUIRE_FAILURE;
        }
    if ( ( *info ).stime != NULL )
        free ( ( *info ).stime );
    if ( ( ( *info ).stime = ( ( int * ) malloc ( sizeof ( int ) * ( info->nstep ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for times.\n" );
        goto INQUIRE_FAILURE;
        }



    for ( i = 0; i < info->nstep; i ++ )
        {
        net2julian ( info->first_date, info->first_time, tstep_sec,
                     i+1, ( info->sdate )+i, ( info->stime )+i );
        }





    /* Turn off NetCDF gripes about attributes that aren't present */

    ncopts = 0;
    if ( ( ncattget ( vis_fd, NC_GLOBAL, "GDTYP", &grid_type ) ) == NC_SYSERR )
        grid_type = 1;
    if ( ( ncattget ( vis_fd, NC_GLOBAL, "XCELL", &xcell ) ) == NC_SYSERR )
        xcell = 80.0;
    if ( ( ncattget ( vis_fd, NC_GLOBAL, "YCELL", &ycell ) ) == NC_SYSERR )
        ycell = 80.0;
    if ( ( ncattget ( vis_fd, NC_GLOBAL, "XORIG", &xorig ) ) == NC_SYSERR )
        xorig = -800.0;
    if ( ( ncattget ( vis_fd, NC_GLOBAL, "YORIG", &yorig ) ) == NC_SYSERR )
        yorig = -1640.0;
    if ( ( ncattget ( vis_fd, NC_GLOBAL, "XCENT", &xcent ) ) == NC_SYSERR )
        xcent = -90.0;
    if ( ( ncattget ( vis_fd, NC_GLOBAL, "YCENT", &ycent ) ) == NC_SYSERR )
        ycent = 40.0;
    if ( ( ncattget ( vis_fd, NC_GLOBAL, "P_ALP", &p_alp ) ) == NC_SYSERR )
        p_alp = 30.0;
    if ( ( ncattget ( vis_fd, NC_GLOBAL, "P_BET", &p_bet ) ) == NC_SYSERR )
        p_bet = 60.0;
    if ( ( ncattget ( vis_fd, NC_GLOBAL, "P_GAM", &p_gam ) ) == NC_SYSERR )
        p_gam = -90.0;
    ncopts = ( NC_VERBOSE );

    if ( grid_type == 0 )
        grid_type = 1;
    if ( ( *info ).map_info != NULL )
        free ( ( *info ).map_info );
    if ( ( ( *info ).map_info = ( ( char * ) malloc ( sizeof ( char ) * 256 ) ) ) == NULL )
        {
        sprintf ( message, "Error allocating map info string" );
        goto INQUIRE_FAILURE;
        }
    if ( ftype == 2 )
        {
        xorig -= xcell;
        yorig -= ycell;
        }
    sprintf ( ( *info ).map_info, "%d %g %g %g %g %g %g %g %g %g %d %d",
              grid_type, xorig, yorig, xcell, ycell, xcent, ycent, p_gam, p_bet, p_alp,
              ( *info ).ncol, ( *info ).nrow );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "@(#)alpha.c	2.4 just set map_info to '%s'\n", ( *info ).map_info );
#endif /* DIAGNOSTICS */

    if ( ( *info ).data_label != NULL )
        free ( ( *info ).data_label );
    if ( ( ( *info ).data_label = ( ( char * ) malloc ( sizeof ( char ) * 256 ) ) ) == NULL )
        {
        sprintf ( message, "Error allocating map info string" );
        goto INQUIRE_FAILURE;
        }
    strcpy ( ( *info ).data_label, "" /* "netCDF LABEL" SRT 951005 */ );


INQUIRE_SUCCESS:
    ncclose ( vis_fd );
    if ( species_short_list != NULL )
        free ( species_short_list );
    if ( units_list != NULL )
        free ( units_list );
    return ( PAVE_SUCCESS );

INQUIRE_FAILURE:
    if ( species_short_list != NULL )
        free ( species_short_list );
    if ( units_list != NULL )
        free ( units_list );
    ncclose ( vis_fd );
    return ( FAILURE );
    }

int alpha_get_info ( VIS_DATA *info, char *message )
    {
    int inquiring = 1;
    if ( alpha_inquire ( info, message, inquiring ) )
        return ( PAVE_SUCCESS );
    return ( FAILURE );
    }

int alpha_get_data ( VIS_DATA *info, char *message )
    {
    static long start[4];       /* starting positions for extracting data */
    static long count[4];       /* number of each dimension to extract */
    register int i, j, k, m;

    int n, nslice;
    int ncol, nrow, nlevel, nstep;
    int col0, row0, level0, step0;
    int vis_fd;
    
    float val, vmin, vmax ;

    nc_type datatype;       /* type of netCDF variable */
    char varname[32];   /* name of netCDF variable */
    int vardim[32];         /* dimensions of netCDF variables */
    int nvaratts;       /* # atts for single variable in netCDF file */
    int nvardims;       /* # dims for single variable in netCDF file */

    int inquiring = 0;

    int ftype;
    int get_ncf_bdry_data();

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter alpha.c's alpha_get_data()\n" );
#endif

    if ( !alpha_inquire ( info, message, inquiring ) )
        {
        return ( FAILURE );
        }
    if ( ( *info ).slice == NONESLICE )
        {
        goto DATA_SUCCESS;
        }
    if ( !alpha_open ( &vis_fd, info, message ) )
        {
        sprintf ( message, "Error opening file %s\n", ( *info ).filename );
        return ( FAILURE );
        }
    if ( ( *info ).dataset != netCDF_DATA )
        goto DATA_TYPE_ERROR;

    if ( ( ncattget ( vis_fd, NC_GLOBAL, "FTYPE", &ftype ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s",  "Error calling ncattget" );
        goto DATA_FAILURE;
        }

    if ( ftype == 2 ) /* Boundary data */
        {
        return get_ncf_bdry_data ( vis_fd, info, message );
        }

    switch ( ( *info ).slice )
        {
        case XYSLICE:
            {
            ( *info ).level_min = ( *info ).level_max = ( *info ).selected_level;
            ( *info ).step_min = ( *info ).step_max = ( *info ).selected_step;
            break;
            }
        case YZSLICE:
            {
            ( *info ).col_min = ( *info ).col_max = ( *info ).selected_col;
            ( *info ).step_min = ( *info ).step_max = ( *info ).selected_step;
            break;
            }
        case XZSLICE:
            {
            ( *info ).row_min = ( *info ).row_max = ( *info ).selected_row;
            ( *info ).step_min = ( *info ).step_max = ( *info ).selected_step;
            break;
            }
        case XYZSLICE:
            {
            ( *info ).step_min = ( *info ).step_max = ( *info ).selected_step;
            break;
            }
        case XYTSLICE:
            {
            ( *info ).level_min = ( *info ).level_max = ( *info ).selected_level;
            break;
            }
        case YZTSLICE:
            {
            ( *info ).col_min = ( *info ).col_max = ( *info ).selected_col;
            break;
            }
        case XZTSLICE:
            {
            ( *info ).row_min = ( *info ).row_max = ( *info ).selected_row;
            break;
            }
        case XYZTSLICE:
            {
            /* no restrictions! */
            break;
            }
        case NONESLICE:
            {
            break;
            }
        }

    if ( ( ( *info ).col_min < 1 ) || ( ( *info ).col_max > ( *info ).ncol ) )
        goto DIMENSION_ERROR;
    if ( ( ( *info ).row_min < 1 ) || ( ( *info ).row_max > ( *info ).nrow ) )
        goto DIMENSION_ERROR;
    if ( ( ( *info ).level_min < 1 ) || ( ( *info ).level_max > ( *info ).nlevel ) )
        goto DIMENSION_ERROR;
    if ( ( ( *info ).step_min < 1 ) || ( ( *info ).step_max > ( *info ).nstep ) )
        goto DIMENSION_ERROR;
    if ( ( ( *info ).selected_species > ( *info ).nspecies ) )
        goto DIMENSION_ERROR;
    if ( ( *info ).step_incr < 1 )
        goto DIMENSION_ERROR;

    ncol   = ( *info ).col_max - ( *info ).col_min + 1;
    nrow   = ( *info ).row_max - ( *info ).row_min + 1;
    nlevel = ( *info ).level_max - ( *info ).level_min + 1;
    nstep  = ( ( ( *info ).step_max - ( *info ).step_min ) / ( *info ).step_incr ) + 1;

    col0   = ( *info ).col_min - 1;
    row0   = ( *info ).row_min - 1;
    level0 = ( *info ).level_min - 1;
    step0  = ( *info ).step_min - 1;

    count[0] = nstep;
    count[1] = nlevel;
    count[2] = nrow;
    count[3] = ncol;
    start[0] = step0;
    start[1] = level0;
    start[2] = row0;
    start[3] = col0;

    n = ncol * nrow * nlevel * nstep;
    nslice = ncol * nrow * nlevel;

    if ( ( i = ncvarid ( vis_fd,
                         ( *info ).species_short_name[ ( *info ).selected_species-1] ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncvarid" );
        return ( FAILURE );
        }
    if ( ( *info ).grid != NULL )
        free ( ( *info ).grid );
    if ( ( ( *info ).grid = ( ( float * ) malloc ( sizeof ( float ) * ( n ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for data grid.\n" );
        goto DATA_FAILURE;
        }
    if ( ( *info ).step_incr == 1 )
        {
        if ( ( ncvarget ( vis_fd, i, start, count, ( void * ) ( ( *info ).grid ) ) )
                == NC_SYSERR )
            {
            sprintf ( message, "%s", "Error calling ncvarget" );
            goto DATA_FAILURE;
            }
        }
    else
        {
        count[0] = 1;
        j = 0;
        for ( k = step0; k < ( *info ).step_max; k += ( *info ).step_incr )
            {
            start[0] = k;
            if ( ( ncvarget ( vis_fd, i, start, count,
                              ( void * ) ( ( *info ).grid+j ) ) ) == NC_SYSERR )
                {
                sprintf ( message, "Error calling ncvarget on step %d",
                          k );
                goto DATA_FAILURE;
                }
            j += nslice;
            }
        }


    /* If the data type is INTEGER, we need to recast the grid array */
    if ( ( ncvarinq ( vis_fd, i, varname, &datatype, &nvardims,
                      vardim, &nvaratts ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncvarinq" );
        goto DATA_FAILURE;
        }
    if ( datatype==NC_LONG )        /*  Process "missing", grid_max and grid_min: */
        {
        char *keep_int_asis;
        keep_int_asis = getenv ( "PAVE_NO_INT2FLOAT" );
        if ( keep_int_asis==NULL || atoi ( keep_int_asis ) !=1 )
            {
            int *igrid = ( int * ) info->grid;

            for ( j=0, m=0; j<n; j++ )
                {
                if ( igrid[j] == IMISS3 )
                    {
                    info->grid[j] = setNaNf();
                    }
                else{
                    val = ( float ) igrid[j];
                    info->grid[j] = ( float ) igrid[j];
                    if ( !m )
                        {
                        vmin = val ;
                        vmax = val ;
                        m    = 1 ;
                        }
                    else if ( val < vmin )
                        vmin = val ;
                    else if ( val > vmax )
                        vmax = val ;
                    
                    }
                }
            }
        }

    else{       /*  float:  */

        for ( j=0, m=0; j<n; j++ )
            {
            if ( info->grid[j] < AMISS3 )
                info->grid[j] = setNaNf();
            else{
                val = info->grid[j] ;
                if ( isnanf( val ) )
                    continue ;
                else if ( ! m )
                    { 
                    vmin = val ;
                    vmax = val ;
                    m    = 1 ;
                    }
                else if ( val < vmin )
                    vmin = val ;
                else if ( val > vmax )
                    vmax = val ;
                }
            }
        }
    
    if ( isnanf( vmin ) )
        {
        sprintf ( message, "All data is *MISSING*" );
        return ( FAILURE );
        }

    info->grid_min = vmin ;
    info->grid_max = vmax ;
    
    /*
    fprintf ( stderr,"File %s\nSpecies \"%s\" MIN=%f  MAX=%f", 
              info->filename, 
              info->species_short_name[ info->selected_species-1],
              vmin, vmax );
    */

    j = 0;
    for ( i = ( *info ).step_min; i <= ( *info ).step_max; i += ( *info ).step_incr )
        {
        net2julian ( ( *info ).first_date, ( *info ).first_time, ( *info ).incr_sec,
                     /*      i, ((*info).sdate)+j, ((*info).stime)+j); SRT 971223 */
                     i, &info->sdate[j], &info->stime[j] );
        ++j;
        }

DATA_SUCCESS:
    ncclose ( vis_fd );
    return ( PAVE_SUCCESS );

DATA_FAILURE:
    sprintf ( message, "netCDF error." );
    return ( FAILURE );

DIMENSION_ERROR:
    sprintf ( message, "Dimension error." );
    return ( FAILURE );

DATA_TYPE_ERROR:
    sprintf ( message, "Data is not of type netCDF." );
    return ( FAILURE );
    }


#define grd_data(c,r) c-(1-nthik) + ncol*(r-(1-nthik))
int get_ncf_bdry_data ( int vis_fd, VIS_DATA *info, char *message )
    {
    int i, r, c;
    int n, t;
    int ncol, nrow, nlevel, nstep;
    int perim, nthik, nslice, spc;
    int icol, irow;
    int level0, step0;

    long start[4];        /* starting positions for extracting data */
    long count[4];        /* number of each dimension to extract */

    float *buf;
    float *spntr, *epntr, *npntr, *wpntr;
    float *data;

    nthik = 1; /* currently all BNDRY files have NTHIK=1 */
    level0 = info->level_min - 1;
    step0 = info->step_min - 1;

    ncol = info->ncol;
    nrow = info->nrow;
    nlevel = info->level_max - level0;
    nstep = info->step_max-step0;

    n = ncol * nrow * nlevel * nstep;

    /*
    sprintf(message,"BDRY_DATA: n=%d, col=%d, row=%d, lvl=%d, steps=%d\n",
      n,ncol, nrow, nlevel, nstep);
    return FAILURE;
    */
    if ( info->grid != NULL )
        free ( info->grid );
    if ( ( info->grid = ( ( float * ) malloc ( sizeof ( float ) * ( n ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for data grid.\n" );
        return FAILURE;
        }

    if ( ( spc = ncvarid ( vis_fd,
                           info->species_short_name[info->selected_species-1] ) )
            == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncvarid" );
        return ( FAILURE );
        }

    for ( i=0; i < n; i++ )
        {
        info->grid[i]=setNaNf();
        }

    perim = 2* ( ncol + nrow - 2*nthik ); /* nthisk is already included */
    nslice = perim * nthik;

    buf = ( float * ) malloc ( sizeof ( float ) * nslice );
    if ( buf == NULL )
        {
        sprintf ( message, "%s", "Buffer allocation error" );
        return FAILURE;
        }

    count[1] = nlevel;
    count[2] = perim;
    count[3] = nthik;

    start[1] = level0;
    start[2] = 0;
    start[3] = 0;

    icol = ncol - 2*nthik;
    irow = nrow - 2*nthik;

    spntr = buf - 1;
    epntr = buf - 1 +   icol        +   nthik;
    npntr = buf - 1 +   icol + irow + 3*nthik;
    wpntr = buf - 1 + 2*icol + irow + 4*nthik;

    for ( t=step0; t<nstep; t++ )
        {
        start[0]=t;
        count[0]=1;
        if ( ncvarget ( vis_fd, spc, start, count, buf ) == NC_SYSERR )
            {
            sprintf ( message, "%s", "NetCDF ncvarget error" );
            return FAILURE;
            }

        data = info->grid + ( t-step0 ) *ncol*nrow*nlevel;

        r = 0;
        for ( c=1; c<=icol+nthik; c++ )  data[grd_data ( c,r )]=spntr[c];
        r = irow+nthik;
        for ( c=0; c< icol+nthik; c++ )  data[grd_data ( c,r )]=npntr[c];
        c = icol+nthik;
        for ( r=1; r<=irow+nthik; r++ )  data[grd_data ( c,r )]=epntr[r];
        c=0;
        for ( r=0; r< irow+nthik; r++ )  data[grd_data ( c,r )]=wpntr[r];
        }

    free ( buf );


    return PAVE_SUCCESS;

    }

/*******************************************************************/
