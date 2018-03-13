/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: uamv.c 83 2018-03-12 19:24:33Z coats $
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
 *      Author:  Atanas Trayanov, NCSC, c 1994?
 *      
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
/* SRT 950915 for DEC Alpha compilation #include <sys/unistd.h> */
#include <sys/stat.h>
#include <fcntl.h>

#include "vis_data.h"
#include "uamv.h"
#define uamv_close uam_close

/* SRT 950703 indexing macro, snagged from bts.h */
#define INDEX(col, row, level, step, NCOL, NROW, NLEVEL) \
        ((int)( (col) +                                 \
                ((row) * (NCOL)) +                       \
                ((level) * (NCOL) * (NROW)) +            \
                ((step) * (NCOL) * (NROW) * (NLEVEL))))

void net2julian ( int sdate, int stime, int tstep_sec, int record,
                  int *jdate, int *jtime );
int uamv_close();
int uamv_fetch_header ( char *filename, UAMV_INFO *hdr_info );
int uamv_fetch_data ( UAMV_INFO *hdr_info, float *buf,
                      int n, int spec, int level, int hour );

/*******************************************************************/
/* uamv_open                                                        */
/* Function: open input file and determine filesize and filetype   */
/* On Error: return FAILURE and write Error string into message    */
/* If no Error: return PAVE_SUCCESS                                     */
/*******************************************************************/
int uamv_open ( VIS_DATA *info, char *message )
    {
    UAMV_INFO uamv_info;
    int local_fd;
    struct stat statbuf;            /* UNIX file status buffer */
    register int istat;         /* I/O status fuam UNIX stat call */
    register int status;            /* uamv_open function return status */
    /* ------------------------------------------ */
    /* -- file can be BOTH online AND offline! -- */
    int online;                     /* requstded data file is online */
    int offline;                    /* requstded data file is offline */
    /* ------------------------------------------ */

    uamv_info.spec_list = NULL; /* added 950718 SRT */
    uamv_info.file_id = NULL;   /* added 950718 SRT */

    status = FAILURE;           /* assume FAILURE */

    /* check file migration status */

    if ( !get_migrate_state ( ( *info ).filename, &online, &offline ) )
        {
        sprintf ( message, "Error! Cannot access file %s.\n",
                  ( *info ).filename );
        return ( FAILURE );
        }
    if ( !online )
        fprintf ( stdout, "Retrieving migrated file ... expect a delay!\n" );


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
                if ( uamv_fetch_header ( ( *info ).filename, &uamv_info ) )
                    {
                    if ( uamv_info.uamv_type != UNKNOWN )
                        {
                        ( *info ).dataset = UAMV_DATA;
                        status = PAVE_SUCCESS;
                        }
                    else
                        {
                        sprintf ( message, "%s",
                                  "File is not recognized as UAM-V." );
                        }
                    uamv_close();
                    }
                else
                    {
                    sprintf ( message, "%s",
                              "Read UAM-V file ERROR: see the command window for details" );
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
    if ( uamv_info.spec_list != NULL )
        {
        free ( uamv_info.spec_list );
        uamv_info.spec_list = NULL;   /* added 950718 SRT */
        }
    if ( uamv_info.file_id != NULL )
        {
        free ( uamv_info.file_id );
        uamv_info.file_id = NULL;   /* added 950718 SRT */
        }
    return ( status );
    }

int uamv_inquire ( VIS_DATA *info, char *message, int inquiring )
    {
    UAMV_INFO uamv_info;
    static char units[32];
    int spos;           /* species list character position */
    int upos;           /* units list character position */
    register int i, j, k, l;
    int sdate;          /* start date */
    int stime;          /* start time */
    int tstep;          /* time step increment */
    int tstep_sec;
    int hour;
    int min;
    int sec;

    char *species_short_list = NULL;
    char *units_list = NULL;

    uamv_info.spec_list = NULL; /* added 950718 SRT */
    uamv_info.file_id = NULL;   /* added 950718 SRT */

    uamv_info.sdate = info->sdate;
    uamv_info.stime = info->stime;

    if ( !uamv_fetch_header ( ( *info ).filename, &uamv_info ) )
        {
        sprintf ( message, "Error fetching UAM file header." );
        return ( FAILURE );
        }
    uamv_close();
    ( *info ).nstep = uamv_info.nstep;
    ( *info ).sdate = uamv_info.sdate;
    ( *info ).stime = uamv_info.stime;
    ( *info ).nlevel = uamv_info.ilevel;
    ( *info ).nrow = uamv_info.irow;
    ( *info ).ncol = uamv_info.icol;
    ( *info ).nspecies = uamv_info.ispec;

    if ( ( ( *info ).nstep == 0 ) || ( ( *info ).nlevel == 0 ) ||
            ( ( *info ).nrow == 0 ) || ( ( *info ).ncol == 0 ) )
        {
        sprintf ( message, "%s",
                  "Unexpected dimensions! Expecting ROW-COL-LEVEL-STEP grid values!\n" );
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
        /*
                ((*info).sdate != NULL) ||
                ((*info).stime != NULL) ||
        */
        ( ( *info ).map_info != NULL ) ||
        ( ( *info ).data_label != NULL )
    )
        goto INQUIRE_SUCCESS;

    if ( species_short_list != NULL )
        free ( species_short_list );
    species_short_list = NULL; /* added 950718 SRT */
    if ( ( species_short_list =
                ( ( char * ) malloc ( sizeof ( char ) * ( ( *info ).nspecies * 32 ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for list.\n" );
        goto INQUIRE_FAILURE;
        }
    if ( units_list != NULL )
        free ( units_list );
    units_list = NULL; /* added 950718 SRT */
    if ( ( units_list =
                ( ( char * ) malloc ( sizeof ( char ) * ( ( *info ).nspecies * 32 ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for list.\n" );
        goto INQUIRE_FAILURE;
        }

    ( *info ).nspecies = uamv_info.ispec;
    if ( uamv_info.spec_list == NULL )
        {
        switch ( uamv_info.uamv_type )
            {
            case UAMV_WIND:
                {
                /* KLP 1/31/95 WIND:WIND: --> UWIND:VWIND */
                strcpy ( species_short_list, "UWIND:VWIND:" );
                break;
                }
            case UAMV_TEMP:
                {
                strcpy ( species_short_list, "TMPR:" );
                break;
                }
            case UAMV_H2O:
                {
                strcpy ( species_short_list, "H2O:" );
                break;
                }
            case UAMV_HEIGHT:
                {
                strcpy ( species_short_list, "HGHT:PRES:" );
                break;
                }
            case UAMV_VDIF:
                {
                strcpy ( species_short_list, "VDIF:" );
                break;
                }
            case UAMV_RAIN:
                {
                strcpy ( species_short_list, "RAIN:" );
                break;
                }
            case UAMV_CLOUD:
                {
                strcpy ( species_short_list, "CLOUD:CWATER:" );
                break;
                }
            default:
                {
                break;
                }
            }
        }
    else
        {
        strcpy ( species_short_list, uamv_info.spec_list );
        }
    if ( ( *info ).species_short_name != NULL )
        free ( ( *info ).species_short_name );
    ( *info ).species_short_name = NULL; /* added 950718 SRT */

    if ( ( ( *info ).species_short_name =
                ( ( char ** ) malloc ( sizeof ( char * ) * ( *info ).nspecies ) ) ) == NULL )
        {
        sprintf ( message, "Error allocating short species list string" );
        goto INQUIRE_FAILURE;
        }

    if ( ( *info ).species_long_name != NULL )
        free ( ( *info ).species_long_name );
    ( *info ).species_long_name = NULL; /* added 950718 SRT */

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
    upos = 0;
    switch ( uamv_info.uamv_type )
        {
        case UAMV_WIND:
            {
            strcpy ( units, "M/S:M/S" );
            break;
            }
        case UAMV_TEMP:
            {
            strcpy ( units, "K" );
            break;
            }
        case UAMV_FAVER:
            {
            strcpy ( units, "PPM" );
            break;
            }
        case UAMV_FINST:
            {
            strcpy ( units, "uMol/M3" );
            break;
            }
        case UAMV_H2O:
            {
            strcpy ( units, "PPMV" );
            break;
            }
        case UAMV_HEIGHT:
            {
            strcpy ( units, "M:mb" );
            break;
            }
        case UAMV_VDIF:
            {
            strcpy ( units, "M2/S" );
            break;
            }
        case UAMV_CLOUD:
            {
            strcpy ( units, "N/A:GM/M3" );
            break;
            }
        case UAMV_RAIN:
            {
            strcpy ( units, "IN/HR" );
            break;
            }
        default:
            break;
        }

    sprintf ( units_list+upos, "%s%c", units, ':' );
    upos += strlen ( units ) + 1;

    if ( ( *info ).units_name != NULL )
        free ( ( *info ).units_name );
    ( *info ).units_name = NULL; /* added 950718 SRT */

    if ( ( ( *info ).units_name =
                ( ( char ** ) malloc ( sizeof ( char * ) * ( *info ).nspecies ) ) ) == NULL )
        {
        sprintf ( message, "Error allocating units list string" );
        goto INQUIRE_FAILURE;
        }

    /* KLP 1/31/95 default individual elements of double char pointers to NULL */

    for ( i = 0; i < ( *info ).nspecies; i++ )
        ( *info ).units_name[i] = NULL;
    /* * * */

    l = 0;
    k = 0;
    j = strlen ( units_list );
    for ( i = 0; i < j; i++ )
        {
        if ( units_list[i] == ':' )
            {
            units_list[i] = '\0';

            if ( ( *info ).units_name[l] != NULL )
                free ( ( *info ).units_name[l] );
            ( *info ).units_name[l] = NULL; /* added 950718 SRT */
            if ( ( ( *info ).units_name[l] =
                        ( ( char * ) malloc ( sizeof ( char ) * 32 ) ) ) == NULL )
                {
                sprintf ( message,
                          "Error allocating memory for units for species %d", i );
                goto INQUIRE_FAILURE;
                }
            strcpy ( ( *info ).units_name[l], units_list+k );
            ++l;
            k = i + 1;
            }
        }


    if ( uamv_info.uamv_type == UAMV_FAVER )
        {
        for ( i = 1; i < ( *info ).nspecies; i++ )
            {
            info->units_name[i] = strdup ( info->units_name[0] );
            }
        }

    tstep = 10000;
    ( *info ).incr_sec = tstep_sec = 3600;


    if ( ( *info ).map_info != NULL )
        free ( ( *info ).map_info );
    ( *info ).map_info = NULL; /* added 950718 SRT */
    if ( ( ( *info ).map_info = ( ( char * ) malloc ( sizeof ( char ) * 256 ) ) ) == NULL )
        {
        sprintf ( message, "Error allocating map info string" );
        goto INQUIRE_FAILURE;
        }
    sprintf ( ( *info ).map_info, "%g %g %g %g %d %d %d",
              uamv_info.sw_utmx, uamv_info.sw_utmy,
              uamv_info.ne_utmx, uamv_info.ne_utmy, uamv_info.utm_zone,
              uamv_info.icol, uamv_info.irow );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "@(#)uamv.c	2.3 just set map_info to '%s'\n", ( *info ).map_info );
#endif /* DIAGNOSTICS */

    if ( ( *info ).data_label != NULL )
        free ( ( *info ).data_label );
    ( *info ).data_label = NULL; /* added 950718 SRT */
    if ( ( ( *info ).data_label = ( ( char * ) malloc ( sizeof ( char ) * ( strlen ( uamv_info.file_id ) + 1 ) ) ) ) == NULL )
        {
        sprintf ( message, "Error allocating map info string" );
        goto INQUIRE_FAILURE;
        }
    strcpy ( ( *info ).data_label, uamv_info.file_id );

INQUIRE_SUCCESS:
    if ( species_short_list != NULL )
        free ( species_short_list );
    species_short_list = NULL; /* added 950718 SRT */
    if ( units_list != NULL )
        free ( units_list );
    units_list = NULL; /* added 950718 SRT */
    if ( uamv_info.spec_list != NULL )
        {
        free ( uamv_info.spec_list );
        uamv_info.spec_list = NULL;   /* added 950718 SRT */
        }
    if ( uamv_info.file_id != NULL )
        {
        free ( uamv_info.file_id );
        uamv_info.file_id = NULL;   /* added 950718 SRT */
        }
    return ( PAVE_SUCCESS );

INQUIRE_FAILURE:
    if ( species_short_list != NULL )
        free ( species_short_list );
    if ( units_list != NULL )
        free ( units_list );
    if ( uamv_info.spec_list != NULL )
        {
        free ( uamv_info.spec_list );
        uamv_info.spec_list = NULL;   /* added 950718 SRT */
        }
    if ( uamv_info.file_id != NULL )
        {
        free ( uamv_info.file_id );
        uamv_info.file_id = NULL;   /* added 950718 SRT */
        }
    return ( FAILURE );
    }

int uamv_get_info ( VIS_DATA *info, char *message )
    {
    int inquiring = 1;
    if ( uamv_inquire ( info, message, inquiring ) )
        return ( PAVE_SUCCESS );
    return ( FAILURE );
    }

int uamv_get_data ( VIS_DATA *info, char *message )
    {
    UAMV_INFO uamv_info;
    int ix, iy, iz, it, index;
    register int i, j;

    float *levelbuf = NULL;
    int bufsize;
    int n;
    int ncol, nrow, nlevel, nstep;
    int col0, row0, level0, step0;

    int inquiring = 0;
    int *sdate, *stime;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter uam.c's uamv_get_data()\n" );
#endif

    uamv_info.spec_list = NULL;   /* added 950718 SRT */
    uamv_info.file_id = NULL; /* added 950718 SRT */

    if ( !uamv_inquire ( info, message, inquiring ) )
        {
        if ( uamv_info.spec_list != NULL )
            {
            free ( uamv_info.spec_list );
            uamv_info.spec_list = NULL; /* added 950718 SRT */
            }
        if ( uamv_info.file_id != NULL )
            {
            free ( uamv_info.file_id );
            uamv_info.file_id = NULL; /* added 950718 SRT */
            }
        return ( FAILURE );
        }
    if ( !uamv_fetch_header ( ( *info ).filename, &uamv_info ) )
        {
        sprintf ( message, "Error fetching UAM file header." );
        if ( uamv_info.spec_list != NULL )
            {
            free ( uamv_info.spec_list );
            uamv_info.spec_list = NULL; /* added 950718 SRT */
            }
        if ( uamv_info.file_id != NULL )
            {
            free ( uamv_info.file_id );
            uamv_info.file_id = NULL; /* added 950718 SRT */
            }
        return ( FAILURE );
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
    if ( ( *info ).dataset != UAMV_DATA )
        goto DATA_TYPE_ERROR;
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

    ncol = ( *info ).col_max - ( *info ).col_min + 1;
    nrow = ( *info ).row_max - ( *info ).row_min + 1;
    nlevel = ( *info ).level_max - ( *info ).level_min + 1;
    nstep = ( ( ( *info ).step_max - ( *info ).step_min ) / ( *info ).step_incr ) + 1;

    n = ncol * nrow * nlevel * nstep;
    bufsize = ( *info ).ncol * ( *info ).nrow; /* SRT 950703  ncol * nrow; */

    if ( ( *info ).grid != NULL )
        free ( ( *info ).grid );
    ( *info ).grid = NULL;    /* added 950718 SRT */

    if ( ( ( *info ).grid = ( ( float * ) malloc ( sizeof ( float ) * ( n ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for data grid.\n" );
        goto DATA_FAILURE;
        }
    if ( levelbuf != NULL )
        free ( levelbuf );
    levelbuf = NULL;      /* added 950718 SRT */
    if ( ( levelbuf = ( ( float * ) malloc ( sizeof ( float ) * ( bufsize ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for data buffer.\n" );
        goto DATA_FAILURE;
        }

    /* i = 0; SRT 950703 */
    sdate=info->sdate;
    stime=info->stime;
    for ( it = info->step_min; it <= info->step_max; it += info->step_incr )
        {
        sdate[it-info->step_min] = sdate[it-1];
        stime[it-info->step_min] = stime[it-1];

        for ( iz = info->level_min; iz <= info->level_max; iz++ )
            {
            if ( uamv_fetch_data ( &uamv_info, levelbuf, bufsize,
                                   ( *info ).selected_species - 1, iz - 1, it - 1 ) )
                {
                index = 0;
                for ( iy = 0; iy < uamv_info.irow; iy++ )
                    {
                    for ( ix = 0; ix < uamv_info.icol; ix++ )
                        {
                        if ( ( iy >= ( *info ).row_min - 1 ) &&
                                ( iy <= ( *info ).row_max - 1 ) &&
                                ( ix >= ( *info ).col_min - 1 ) &&
                                ( ix <= ( *info ).col_max - 1 ) )
                            {
                            /*
                               fprintf(stderr, "grid[%d] = levelbuf[%d] = %g\n",
                               i, index, levelbuf[index]);
                               */
                            ( *info ).grid
                            [
                                /* i++ SRT 950703 */
                                INDEX
                                (
                                    /* added +1 950718 SRT */       ix+1- ( *info ).col_min,
                                    /* added +1 950718 SRT */       iy+1- ( *info ).row_min,
                                    iz- ( *info ).level_min,
                                    ( it- ( *info ).step_min ) /
                                    ( *info ).step_incr,
                                    ncol,
                                    nrow,
                                    nlevel
                                )
                            ]
                                = levelbuf[index];
                            }
                        ++index;
                        }
                    }
                }
            }
        }

    ( *info ).grid_min = ( *info ).grid[0];
    ( *info ).grid_max = ( *info ).grid[0];
    for ( i = 1; i < n; i++ )
        {
        if ( ( *info ).grid[i] < ( *info ).grid_min )
            ( *info ).grid_min = ( *info ).grid[i];
        if ( ( *info ).grid[i] > ( *info ).grid_max )
            ( *info ).grid_max = ( *info ).grid[i];
        }


    if ( levelbuf != NULL )
        free ( levelbuf );
    levelbuf = NULL;      /* added 950718 SRT */
    uamv_close();
    if ( uamv_info.spec_list != NULL )
        {
        free ( uamv_info.spec_list );
        uamv_info.spec_list = NULL; /* added 950718 SRT */
        }
    if ( uamv_info.file_id != NULL )
        {
        free ( uamv_info.file_id );
        uamv_info.file_id = NULL; /* added 950718 SRT */
        }
    return ( PAVE_SUCCESS );

DATA_FAILURE:
    if ( levelbuf != NULL )
        free ( levelbuf );
    levelbuf = NULL;      /* added 950718 SRT */
    uamv_close();
    if ( uamv_info.spec_list != NULL )
        {
        free ( uamv_info.spec_list );
        uamv_info.spec_list = NULL; /* added 950718 SRT */
        }
    if ( uamv_info.file_id != NULL )
        {
        free ( uamv_info.file_id );
        uamv_info.file_id = NULL; /* added 950718 SRT */
        }
    return ( FAILURE );

DIMENSION_ERROR:
    sprintf ( message, "Dimension error." );
    uamv_close();
    if ( uamv_info.spec_list != NULL )
        {
        free ( uamv_info.spec_list );
        uamv_info.spec_list = NULL; /* added 950718 SRT */
        }
    if ( uamv_info.file_id != NULL )
        {
        free ( uamv_info.file_id );
        uamv_info.file_id = NULL; /* added 950718 SRT */
        }
    return ( FAILURE );

DATA_TYPE_ERROR:
    sprintf ( message, "Data is not of type UAM." );
    uamv_close();
    if ( uamv_info.spec_list != NULL )
        {
        free ( uamv_info.spec_list );
        uamv_info.spec_list = NULL; /* added 950718 SRT */
        }
    if ( uamv_info.file_id != NULL )
        {
        free ( uamv_info.file_id );
        uamv_info.file_id = NULL; /* added 950718 SRT */
        }
    return ( FAILURE );
    }
