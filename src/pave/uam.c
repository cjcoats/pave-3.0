/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: uam.c 83 2018-03-12 19:24:33Z coats $
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
 *      Author:  Kathy Pearson, MCNC, December 14, 1994
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ****************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
/* SRT 950915 for DEC Alpha compilation #include <sys/unistd.h> */
#include <sys/stat.h>
#include <fcntl.h>
#ifndef __OPENNT
#include <values.h>
#endif

#include "nan_incl.h"
#include "vis_data.h"
#include "readuam.h"
#include "parms3.h"

/* SRT 950703 indexing macro, snagged from bts.h */
#define INDEX(col, row, level, step, NCOL, NROW, NLEVEL) \
        ((int)( (col) +                                 \
                ((row) * (NCOL)) +                       \
                ((level) * (NCOL) * (NROW)) +            \
                ((step) * (NCOL) * (NROW) * (NLEVEL))))

void net2julian ( int sdate, int stime, int tstep_sec, int record,
                  int *jdate, int *jtime );
int uam_close();
int uam_fetch_header ( char *filename, UAM_INFO *hdr_info );
int uam_fetch_data ( UAM_INFO *hdr_info, float *buf,
                     int n, int spec, int level, int hour );

/*******************************************************************/
/* uam_open                                                        */
/* Function: open input file and determine filesize and filetype   */
/* On Error: return FAILURE and write Error string into message    */
/* If no Error: return PAVE_SUCCESS                                     */
/*******************************************************************/
int uam_open ( VIS_DATA *info, char *message )
    {
    UAM_INFO uam_info;
    int local_fd;
    struct stat statbuf;      /* UNIX file status buffer */
    register int istat;       /* I/O status fuam UNIX stat call */
    register int status;      /* uam_open function return status */
    /* ------------------------------------------ */
    /* -- file can be BOTH online AND offline! -- */
    int online;           /* requstded data file is online */
    int offline;          /* requstded data file is offline */
    /* ------------------------------------------ */

    uam_info.spec_list = NULL;    /* added 950718 SRT */
    uam_info.file_id = NULL;  /* added 950718 SRT */
    uam_info.sdate = NULL; /*added 960703 ALT */
    uam_info.stime = NULL; /*added 960703 ALT */

    status = FAILURE;     /* assume FAILURE */

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
                if ( uam_fetch_header ( ( *info ).filename, &uam_info ) )
                    {
                    if ( uam_info.uam_type != UNKNOWN )
                        {
                        ( *info ).dataset = UAM_DATA;
                        status = PAVE_SUCCESS;
                        }
                    else
                        {
                        sprintf ( message, "%s",
                                  "File is not recognized as UAM." );
                        }
                    uam_close();
                    }
                else
                    {
                    sprintf ( message, "%s",
                              "Read UAM-IV file ERROR: see the command window for details" );
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
    if ( uam_info.spec_list != NULL )
        {
        free ( uam_info.spec_list );
        uam_info.spec_list = NULL; /* added 950718 SRT */
        }
    if ( uam_info.file_id != NULL )
        {
        free ( uam_info.file_id );
        uam_info.file_id = NULL;  /* added 950718 SRT */
        }
    if ( uam_info.stime != NULL )     /* added 960919 SRT for memory management */
        {
        /* added 960919 SRT for memory management */
        free ( ( char * ) uam_info.stime ); /* added 960919 SRT for memory management */
        uam_info.stime = NULL;      /* added 960919 SRT for memory management */
        }                   /* added 960919 SRT for memory management */
    if ( uam_info.sdate != NULL )     /* added 960919 SRT for memory management */
        {
        /* added 960919 SRT for memory management */
        free ( ( char * ) uam_info.sdate ); /* added 960919 SRT for memory management */
        uam_info.sdate = NULL;      /* added 960919 SRT for memory management */
        }                   /* added 960919 SRT for memory management */
    return ( status );
    }

int uam_inquire ( VIS_DATA *info, char *message, int inquiring )
    {
    UAM_INFO uam_info;
    static char units[32];
    int spos;         /* species list character position */
    int upos;         /* units list character position */
    register int i, j, k, l;
    int hour;
    int min;
    int sec;
    int   gdtyp, ncols, nrows, utm_zone;
    float  llx, lly, urx, ury, xorig, yorig, xcell,
           ycell, xcent, ycent, p_gam, p_bet, p_alp;

    char *coord_str;
    char *species_short_list = NULL;
    char *units_list = NULL;
    enum  { UNUSED = -1 };

    uam_info.spec_list = NULL;    /* added 950718 SRT */
    uam_info.file_id = NULL;  /* added 950718 SRT */
    uam_info.sdate = NULL; /*added 960703 ALT */
    uam_info.stime = NULL; /*added 960703 ALT */

    if ( !uam_fetch_header ( ( *info ).filename, &uam_info ) )
        {
        sprintf ( message, "Error fetching UAM file header." );
        return ( FAILURE );
        }
    uam_close();
    ( *info ).nstep = uam_info.nstep;
    if ( ( *info ).sdate ) free ( ( *info ).sdate );
    ( *info ).sdate = NULL; /* added 960515 SRT to avoid memory leak from malloc in record.c */
    ( *info ).sdate = uam_info.sdate;
    if ( ( *info ).stime ) free ( ( *info ).stime );
    ( *info ).stime = NULL; /* added 960515 SRT to avoid memory leak from malloc in record.c */
    ( *info ).stime = uam_info.stime;
    ( *info ).nlevel = uam_info.ilevel;
    ( *info ).nrow = uam_info.irow;
    ( *info ).ncol = uam_info.icol;
    ( *info ).nspecies = uam_info.ispec;

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
    species_short_list = NULL;    /* added 950718 SRT */
    if ( ( species_short_list =
                ( ( char * ) malloc ( sizeof ( char ) * ( ( *info ).nspecies * 32 ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for list.\n" );
        goto INQUIRE_FAILURE;
        }
    if ( units_list != NULL )
        free ( units_list );
    units_list = NULL;        /* added 950718 SRT */
    if ( ( units_list =
                ( ( char * ) malloc ( sizeof ( char ) * ( ( *info ).nspecies * 32 ) ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for list.\n" );
        goto INQUIRE_FAILURE;
        }

    ( *info ).nspecies = uam_info.ispec;
    if ( uam_info.spec_list == NULL )
        {
        switch ( uam_info.uam_type )
            {
            case UAM_WIND:
                {
                /* KLP 1/31/95 WIND:WIND: --> UWIND:VWIND */
                strcpy ( species_short_list, "UWIND:VWIND:" );
                break;
                }
            case UAM_DIFF:
                {
                strcpy ( species_short_list, "DFBK:" );
                break;
                }
            case UAM_REGT:
                {
                strcpy ( species_short_list, "REGIONTOP:" );
                break;
                }
            case UAM_TEMP:
                {
                strcpy ( species_short_list, "TMPR:" );
                break;
                }
            case UAM_VERT:
                {
                strcpy ( species_short_list, "VERTICAL_VELOCITY:" );
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
        strcpy ( species_short_list, uam_info.spec_list );
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
    for ( i = 0; i < uam_info.ispec; i++ )
        {
        switch ( uam_info.uam_type )
            {
            case UAM_AVER:
            case UAM_AIRQ:
            case UAM_TOPC:
                {
                if ( !strcmp ( ( *info ).species_short_name[i], "AERO" ) )
                    strcpy ( units, "MICRO_GM/M3" );
                else
                    strcpy ( units, "PPM" );
                break;
                }
            case UAM_INST:
                {
                if ( !strcmp ( ( *info ).species_short_name[i], "AERO" ) )
                    strcpy ( units, "MICRO_GM/M3" );
                else
                    strcpy ( units, "uMOL/M3" );
                break;
                }
            case UAM_PTSR:
            case UAM_EMIS:
                {
                if ( !strcmp ( ( *info ).species_short_name[i], "AERO" ) )
                    strcpy ( units, "GM/HR" );
                else
                    strcpy ( units, "GM_MOLES/HR" );
                break;
                }
            case UAM_WIND:
                {
                strcpy ( units, "M/H" );
                break;
                }
            case UAM_DIFF:
                {
                strcpy ( units, "M" );
                break;
                }
            case UAM_REGT:
                {
                strcpy ( units, "M" );
                break;
                }
            case UAM_TEMP:
                {
                strcpy ( units, "K" );
                break;
                }
            case UAM_VERT:
                {
                strcpy ( units, "CM/S" );
                break;
                }
            default:
                break;
            }

        sprintf ( units_list+upos, "%s%c", units, ':' );
        upos += strlen ( units ) + 1;
        }

    if ( ( *info ).units_name != NULL )
        free ( ( *info ).units_name );
    ( *info ).units_name = NULL;  /* added 950718 SRT */

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

    ( *info ).incr_sec = 3600;

    if ( ( *info ).map_info != NULL )
        free ( ( *info ).map_info );
    ( *info ).map_info = NULL; /* added 950718 SRT */
    if ( ( ( *info ).map_info = ( ( char * ) malloc ( sizeof ( char ) * 256 ) ) ) == NULL )
        {
        sprintf ( message, "Error allocating map info string" );
        goto INQUIRE_FAILURE;
        }


    nrows  = uam_info.irow;
    ncols  = uam_info.icol;
    xorig  = uam_info.sw_utmx;
    yorig  = uam_info.sw_utmy;
    urx    = uam_info.ne_utmx;
    ury    = uam_info.ne_utmy;
    xcell  = ( urx-xorig ) /ncols;
    ycell  = ( ury-yorig ) /nrows;

    if ( uam_info.utm_zone > 0 )
        {
        gdtyp  = UTMGRD3;
        xcent  = 0;
        ycent  = 0;
        p_alp  = uam_info.utm_zone;
        p_bet  = UNUSED;
        p_gam  = UNUSED;
        }
    else
        {
        if ( ( yorig < 90. ) && ( yorig>0. ) && ( ury<90. ) && ( ury>0. ) )
            {
            gdtyp  = LATGRD3;
            xcent  = UNUSED;
            ycent  = UNUSED;
            p_alp  = UNUSED;
            p_bet  = UNUSED;
            p_gam  = UNUSED;
            }
        else
            {
            gdtyp  = LAMGRD3;
            coord_str = getenv ( "PAVE_COORD" );
            if ( coord_str )
                {
                sscanf ( coord_str,"%d %f %f %f %f %f",
                         &gdtyp, &p_alp, &p_bet, &p_gam, &xcent, &ycent );
                }
            else
                {
                fprintf ( stderr,"%s\n%s%s\n",
                          "ERROR: UAM file does not have self-describing coordinate info.",
                          "You must setenv PAVE_COORD \"",
                          "grid_type p_alp p_bet p_gam xcent ycent\"" );
                }
            }
        }
    sprintf ( ( *info ).map_info, "%d %g %g %g %g %g %g %g %g %g %d %d",
              gdtyp, xorig, yorig, xcell, ycell, xcent, ycent,
              p_gam, p_bet, p_alp, ncols, nrows );


    if ( ( *info ).data_label != NULL )
        free ( ( *info ).data_label );
    ( *info ).data_label = NULL;  /* added 950718 SRT */
    if ( ( ( *info ).data_label = ( ( char * ) malloc ( sizeof ( char ) * ( strlen ( uam_info.file_id ) + 1 ) ) ) ) == NULL )
        {
        sprintf ( message, "Error allocating map info string" );
        goto INQUIRE_FAILURE;
        }
    strcpy ( ( *info ).data_label, uam_info.file_id );

INQUIRE_SUCCESS:
    if ( species_short_list != NULL )
        free ( species_short_list );
    species_short_list = NULL;    /* added 950718 SRT */
    if ( units_list != NULL )
        free ( units_list );
    units_list = NULL;        /* added 950718 SRT */
    if ( uam_info.spec_list != NULL )
        {
        free ( uam_info.spec_list );
        uam_info.spec_list = NULL; /* added 950718 SRT */
        }
    if ( uam_info.file_id != NULL )
        {
        free ( uam_info.file_id );
        uam_info.file_id = NULL;  /* added 950718 SRT */
        }
    return ( PAVE_SUCCESS );

INQUIRE_FAILURE:
    if ( species_short_list != NULL )
        free ( species_short_list );
    if ( units_list != NULL )
        free ( units_list );
    if ( uam_info.spec_list != NULL )
        {
        free ( uam_info.spec_list );
        uam_info.spec_list = NULL; /* added 950718 SRT */
        }
    if ( uam_info.file_id != NULL )
        {
        free ( uam_info.file_id );
        uam_info.file_id = NULL;  /* added 950718 SRT */
        }
    return ( FAILURE );
    }

int uam_get_info ( VIS_DATA *info, char *message )
    {
    int inquiring = 1;
    if ( uam_inquire ( info, message, inquiring ) )
        return ( PAVE_SUCCESS );
    return ( FAILURE );
    }

int uam_get_data ( VIS_DATA *info, char *message )
    {
    UAM_INFO uam_info;
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
    fprintf ( stderr, "Enter uam.c's uam_get_data()\n" );
#endif

    uam_info.spec_list = NULL;    /* added 950718 SRT */
    uam_info.file_id = NULL;  /* added 950718 SRT */
    uam_info.sdate = NULL; /*added 960703 ALT */
    uam_info.stime = NULL; /*added 960703 ALT */

    if ( !uam_inquire ( info, message, inquiring ) )
        {
        if ( uam_info.spec_list != NULL )
            {
            free ( uam_info.spec_list );
            uam_info.spec_list = NULL; /* added 950718 SRT */
            }
        if ( uam_info.file_id != NULL )
            {
            free ( uam_info.file_id );
            uam_info.file_id = NULL; /* added 950718 SRT */
            }
        return ( FAILURE );
        }
    if ( !uam_fetch_header ( ( *info ).filename, &uam_info ) )
        {
        sprintf ( message, "Error fetching UAM file header." );
        if ( uam_info.spec_list != NULL )
            {
            free ( uam_info.spec_list );
            uam_info.spec_list = NULL; /* added 950718 SRT */
            }
        if ( uam_info.file_id != NULL )
            {
            free ( uam_info.file_id );
            uam_info.file_id = NULL; /* added 950718 SRT */
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
    if ( ( *info ).dataset != UAM_DATA )
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
            if ( uam_fetch_data ( &uam_info, levelbuf, bufsize,
                                  ( *info ).selected_species - 1, iz - 1, it - 1 ) )
                {
                index = 0;
                for ( iy = 0; iy < uam_info.irow; iy++ )
                    {
                    for ( ix = 0; ix < uam_info.icol; ix++ )
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

    info->grid_min = MAXFLOAT;
    info->grid_max = ( -MAXFLOAT );
    for ( i = 0; i < n; i++ )
        {
        if ( isnanf ( info->grid[i] ) ) continue;
        if ( ( *info ).grid[i] < ( *info ).grid_min )
            ( *info ).grid_min = ( *info ).grid[i];
        if ( ( *info ).grid[i] > ( *info ).grid_max )
            ( *info ).grid_max = ( *info ).grid[i];
        }


    if ( levelbuf != NULL )
        free ( levelbuf );
    levelbuf = NULL;      /* added 950718 SRT */
    uam_close();
    if ( uam_info.spec_list != NULL )
        {
        free ( uam_info.spec_list );
        uam_info.spec_list = NULL; /* added 950718 SRT */
        }
    if ( uam_info.file_id != NULL )
        {
        free ( uam_info.file_id );
        uam_info.file_id = NULL;  /* added 950718 SRT */
        }
    if ( uam_info.stime != NULL )     /* added 960919 SRT for memory management */
        {
        /* added 960919 SRT for memory management */
        free ( ( char * ) uam_info.stime ); /* added 960919 SRT for memory management */
        uam_info.stime = NULL;      /* added 960919 SRT for memory management */
        }                   /* added 960919 SRT for memory management */
    if ( uam_info.sdate != NULL )     /* added 960919 SRT for memory management */
        {
        /* added 960919 SRT for memory management */
        free ( ( char * ) uam_info.sdate ); /* added 960919 SRT for memory management */
        uam_info.sdate = NULL;      /* added 960919 SRT for memory management */
        }                   /* added 960919 SRT for memory management */
    return ( PAVE_SUCCESS );

DATA_FAILURE:
    if ( levelbuf != NULL )
        free ( levelbuf );
    levelbuf = NULL;      /* added 950718 SRT */
    uam_close();
    if ( uam_info.spec_list != NULL )
        {
        free ( uam_info.spec_list );
        uam_info.spec_list = NULL; /* added 950718 SRT */
        }
    if ( uam_info.file_id != NULL )
        {
        free ( uam_info.file_id );
        uam_info.file_id = NULL;  /* added 950718 SRT */
        }
    return ( FAILURE );

DIMENSION_ERROR:
    sprintf ( message, "Dimension error." );
    uam_close();
    if ( uam_info.spec_list != NULL )
        {
        free ( uam_info.spec_list );
        uam_info.spec_list = NULL; /* added 950718 SRT */
        }
    if ( uam_info.file_id != NULL )
        {
        free ( uam_info.file_id );
        uam_info.file_id = NULL;  /* added 950718 SRT */
        }
    return ( FAILURE );

DATA_TYPE_ERROR:
    sprintf ( message, "Data is not of type UAM." );
    uam_close();
    if ( uam_info.spec_list != NULL )
        {
        free ( uam_info.spec_list );
        uam_info.spec_list = NULL; /* added 950718 SRT */
        }
    if ( uam_info.file_id != NULL )
        {
        free ( uam_info.file_id );
        uam_info.file_id = NULL;  /* added 950718 SRT */
        }
    return ( FAILURE );
    }

