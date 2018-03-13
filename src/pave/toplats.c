/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: toplats.c 83 2018-03-12 19:24:33Z coats $
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
 *      Author"  Atanas Trayanov, MCNC? 1997?
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ****************************************************************************/

static const char SVN_ID[] = "$Id: toplats.c 83 2018-03-12 19:24:33Z coats $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "vis_data.h"
#include "netcdf.h"
#include "nan_incl.h"

#define MAXLINE 256
#define ERROR(x) fprintf(stderr,"%s\n",x)
#define UNKNOWN1 (-1)

#define PAVE_SUCCESS 1
#define FAILURE 0
#define IOERROR -1


static int SCALE_FACTOR;
static int nrow_full;
static int ncol_full;

#include "utils.h"
#include "toplats.h"

#include "parms3.h"

int toplats_open ( TOPLATS_INFO *tinfo, VIS_DATA *info, char *message )
    {

    int i;
    char *meta_hdr="#! TOPLATS DESCRIPTION FILE";
    char first_line[MAXLINE];
    char dummy[MAXLINE];
    FILE *mtf;
    char *scale_str;

    mtf = fopen ( info->filename,"r" );
    if ( mtf==NULL )
        {
        sprintf ( message,"Cannot open TOPLATS METAFILE %s",info->filename );
        return ( FAILURE );
        }

    /* check header ID string */
    i=strlen ( meta_hdr );
    fgets ( first_line,i+1, mtf );
    if ( strncmp ( first_line, meta_hdr,i ) )
        {
        fclose ( mtf );
        return FAILURE;
        }

    /* Read the metafile */

    i = read_item ( mtf,"TOPLATS_INDEX_FILE %s",tinfo->indexfn );
    if ( !i )
        {
        fclose ( mtf );
        sprintf ( message,"TOPLATS_INDEX_FILE keyword is missing" );
        return FAILURE;
        }

    i = read_item ( mtf,"TOPLATS_DATA_FILE %s",tinfo->datafn );
    if ( !i )
        {
        fclose ( mtf );
        sprintf ( message,"TOPLATS_DATA_FILE keyword is missing" );
        return FAILURE;
        }

    fclose ( mtf );

    scale_str=getenv ( "TOPLATS_SCALE_FACTOR" );
    if ( scale_str != NULL ) SCALE_FACTOR = atoi ( scale_str );
    else SCALE_FACTOR = 1;

    return PAVE_SUCCESS;
    }

/* =========================================================== */

int toplats_get_info ( TOPLATS_INFO *tinfo,
                       VIS_DATA *info, char *message )
    {

    int i,j, nspecies;
    long sdate;           /* start date */
    long stime;           /* start time */
    long tstep;           /* time step increment */
    int tstep_sec;
    int hour;
    int min;
    int sec;


    int fd = NC_SYSERR;
    int ndims;        /* # dimensions in netCDF file */
    int nvars;        /* # variables in netCDF file */
    int ngatts;       /* # global attributes in netCDF file */
    int recdim;       /* id of unlimited dimension in netCDF file */
    int nvardims;     /* # dims for single variable in netCDF file */
    int nvaratts;     /* # atts for single variable in netCDF file */

    long dimsize;     /* size of an individual dimension */
    char dimname[32]; /* name of netCDF dimension */
    char varname[32]; /* name of netCDF variable */
    char units[32];   /* units applied to current variable */
    int vardim[32];   /* dimensions of netCDF variables */
    nc_type datatype;     /* netCDF data type */
    char *org_filename;

    int grid_type, ncol, nrow;
    float xorig, yorig, xcell, ycell, xcent, ycent, p_gam, p_bet, p_alp;


    /* save the original filename from info */
    org_filename = info->filename;
    info->filename = tinfo->indexfn;

    i=alpha_get_info ( info, message );
    if ( !i )
        {
        sprintf ( message,"Error in get_alpha_info; file %s", info->filename );
        goto INQUIRE_FAILURE;
        }

    nrow_full = info->nrow;
    ncol_full = info->ncol;

    info->nrow /= SCALE_FACTOR;
    info->ncol /= SCALE_FACTOR;

    info->col_max = info->ncol;
    info->row_max = info->nrow;

    if ( sscanf ( info->map_info, "%d%g%g%g%g%g%g%g%g%g%d%d",
                  &grid_type, &xorig, &yorig, &xcell, &ycell, &xcent,
                  &ycent, &p_gam, &p_bet, &p_alp, &ncol, &nrow ) == 12 )
        {
        xcell *= SCALE_FACTOR;
        ycell *= SCALE_FACTOR;
        sprintf ( info->map_info, "%d %g %g %g %g %g %g %g %g %g %d %d",
                  grid_type, xorig, yorig, xcell, ycell, xcent,
                  ycent, p_gam, p_bet, p_alp, ncol, nrow );
        }


    nspecies = info->nspecies;
    for ( i=0; i<nspecies; i++ )
        {
        free ( info->units_name[i] );
        free ( info->species_short_name[i] );
        free ( info->species_long_name[i] );
        }
    free ( info->units_name );
    free ( info->species_short_name );
    free ( info->species_long_name );



    if ( ( fd = ncopen ( tinfo->datafn,NC_NOWRITE ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncopen" );
        goto INQUIRE_FAILURE;
        }

    if ( ( ncinquire ( fd, &ndims, &nvars, &ngatts, &recdim ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncinquire" );
        goto INQUIRE_FAILURE;
        }

    nspecies = nvars;     /* we may include TFLAG variables when malloc */
    info->units_name = ( char ** ) malloc ( nspecies*sizeof ( char * ) );
    info->species_short_name = ( char ** ) malloc ( nspecies*sizeof ( char * ) );
    info->species_long_name = ( char ** ) malloc ( nspecies*sizeof ( char * ) );
    if ( ( info->units_name == NULL ) || ( info->species_short_name == NULL ) ||
            ( info->species_long_name == NULL ) )
        {
        sprintf ( message,"%s","Memory allocation error in toplats_get_info" );
        goto INQUIRE_FAILURE;
        }

    nspecies = 0;
    for ( i = 0; i < nvars; ++i )
        {
        if ( ( ncvarinq ( fd, i, varname, &datatype, &nvardims, vardim, &nvaratts ) )
                == NC_SYSERR )
            {
            sprintf ( message, "%s", "Error calling ncvarinq" );
            goto INQUIRE_FAILURE;
            }
        if ( !strcmp ( varname, "TFLAG" ) ) continue; /* ignore TFLAG variables */
        info->species_short_name[nspecies] = strdup ( varname );
        info->species_long_name[nspecies] = strdup ( varname );
        if ( ( ncattget ( fd, i, "units", units ) ) == NC_SYSERR )
            {
            sprintf ( message, "%s",  "Error calling ncattget" );
            goto INQUIRE_FAILURE;
            }
        info->units_name[nspecies] = strdup ( units );
        if ( ( info->units_name[nspecies] == NULL ) ||
                ( info->species_short_name[nspecies] == NULL ) ||
                ( info->species_long_name[nspecies] == NULL ) )
            {
            sprintf ( message,"%s","Memory allocation error in toplats_get_info" );
            goto INQUIRE_FAILURE;
            }
        nspecies++;
        }

    /* copy species info for the datafile */
    info->nspecies = nspecies;


    /********************* SDATE & STIME ********************/
    if ( ( ( ncattget ( fd, NC_GLOBAL, "SDATE", &sdate ) ) == NC_SYSERR ) ||
            ( ( ncattget ( fd, NC_GLOBAL, "STIME", &stime ) ) == NC_SYSERR ) ||
            ( ( ncattget ( fd, NC_GLOBAL, "TSTEP", &tstep ) ) == NC_SYSERR )
       )
        {
        sprintf ( message, "%s", "Error getting date attributes" );
        goto INQUIRE_FAILURE;
        }

    /* tstep is in HHMMSS form -- convert to seconds */
    hour = ( ( int ) tstep ) /10000;
    min = ( ( ( int ) tstep ) - ( hour * 10000 ) ) /100;
    sec = ( ( int ) tstep ) - ( hour * 10000 ) - ( min * 100 );
    tstep_sec = hour * 3600 + min * 60 + sec;
    info->incr_sec = tstep_sec;


    if ( ( ncinquire ( fd, &ndims, &nvars, &ngatts, &recdim ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncinquire" );
        goto INQUIRE_FAILURE;
        }

    for ( i = 0; i < ndims; i++ )
        {
        if ( ( ncdiminq ( fd, i, dimname, &dimsize ) ) == NC_SYSERR )
            {
            sprintf ( message, "%s", "Error calling ncdiminq" );
            goto INQUIRE_FAILURE;
            }
        if ( !strcmp ( dimname, "TSTEP" ) ) info->nstep = ( int ) dimsize;
        }

    info->step_incr = 1;
    info->step_min = 1;
    info->step_max = info->nstep;

    /* get Julian time and date for first record and last record */

    net2julian ( ( int ) sdate, ( int ) stime, tstep_sec, 1,
                 & ( info->first_date ), & ( info->first_time ) );
    net2julian ( ( int ) sdate, ( int ) stime, tstep_sec, info->nstep,
                 & ( info->last_date ), & ( info->last_time ) );


    if ( info->sdate != NULL ) free ( info->sdate );
    if ( ( info->sdate = ( ( int * ) malloc ( sizeof ( int ) * info->nstep ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for dates.\n" );
        goto INQUIRE_FAILURE;
        }
    if ( info->stime != NULL ) free ( info->stime );
    if ( ( info->stime = ( ( int * ) malloc ( sizeof ( int ) * info->nstep ) ) ) == NULL )
        {
        sprintf ( message, "Cannnot allocate memory for times.\n" );
        goto INQUIRE_FAILURE;
        }
    j = 0;
    for ( i = info->step_min; i <= info->step_max; i += info->step_incr )
        {
        net2julian ( info->first_date, info->first_time, info->incr_sec,
                     i, ( info->sdate )+j, ( info->stime )+j );
        ++j;
        }

    info->slice = NONESLICE;
    info->selected_species = 1;
    info->selected_col = 1;
    info->selected_row = 1;
    info->selected_level = 1;
    info->selected_step = 1;


    /* clean up */
    info->filename = org_filename;

    ncclose ( fd );
    return PAVE_SUCCESS;


INQUIRE_FAILURE:
    fprintf ( stderr,"ERROR: %s\n", message );
    if ( fd != NC_SYSERR ) ncclose ( fd );
    return FAILURE;
    }

/* =========================================================== */

int toplats_get_data ( TOPLATS_INFO *tinfo,
                       VIS_DATA *info, char *message )
    {

    int i, j, n, n2d;
    int k, t, item;
    int nrow, ncol;
    float *grid;
    float *data;
    int *index, *igrid;
    char *org_name, *varname;
    nc_type datatype; /* netCDF data type */

    int *ibin, idatatype;
    int m, jj, kk;
    int ir, ic, idx, ndata;
    float w, s, sw, val;
    float no_datapoint;

    no_datapoint=setNaNf();
    /* save the original filename from info */
    org_name = info->filename;
    nrow = info->nrow;
    ncol = info->ncol;
    varname = info->species_short_name[info->selected_species-1];

    n2d = nrow * ncol;
    n = n2d * info->nstep; /* needs change for selected time */


    info->filename = tinfo->indexfn;
    if ( ! get_ncd_GriddedIndex ( message, info, "IPIX", &index ) )
        {
        goto DATA_FAILURE;
        }

    data = ( float * ) malloc ( n * sizeof ( data[0] ) );
    if ( ! ( data ) )
        {
        sprintf ( message,"malloc error in toplats_get_data" );
        goto DATA_FAILURE;
        }


    k=0;
    for ( t = 0; t<info->nstep; t++ )
        {
        if ( ! get_ncd_Vector ( message, tinfo->datafn, varname,
                                "LAY", &grid, &datatype,t ) )
            {
            goto DATA_FAILURE;
            }
        igrid = ( int * ) grid;

        if ( datatype == M3INT ) /* handle separately ints and floats */
            {
            for ( i=0; i<n2d; i++ )
                {
                ir = i / ncol;
                ic = i % ncol;
                idx = ( ir*SCALE_FACTOR ) *ncol_full + ic*SCALE_FACTOR;
                j=index[idx]-1;
                if ( j < 0 ) data[k]=no_datapoint;
                else data[k] = ( float ) igrid[j];
                k++;
                }
            }
        else if ( datatype == M3REAL )
            {
            for ( i=0; i<n2d; i++ )
                {
                ir = i / ncol;
                ic = i % ncol;
                idx = ( ir*SCALE_FACTOR ) *ncol_full + ic*SCALE_FACTOR;
                j=index[idx]-1;
                if ( j < 0 ) data[k]=no_datapoint;
                else data[k] = grid[j];
                k++;
                }
            }
        else
            {
            sprintf ( message,"Unsupported datatype (NetCDF type=%d)",datatype );
            free ( grid );
            goto DATA_FAILURE;
            }
        free ( grid );
        }
    free ( index );

    /* restore some fields of info to original settings */

    info->grid = data;
    info->filename = org_name;


    info->grid_min = -AMISS3;
    info->grid_max =  AMISS3;
    for ( i = 0; i < n; i++ )
        {
        if ( isnanf ( info->grid[i] ) ) continue;
        if ( info->grid[i] < info->grid_min )
            info->grid_min = info->grid[i];
        if ( info->grid[i] > info->grid_max )
            info->grid_max = ( *info ).grid[i];
        }

#ifdef DEBUG
    printf ( "TPL: DEBUG - returning from toplats_get_data\n" );
#endif
    return PAVE_SUCCESS;

DATA_FAILURE:
    fprintf ( stderr,"ERROR: %s\n", message );
    return FAILURE;
    }

/* =========================================================== */
int get_ncd_GriddedIndex ( char *message, VIS_DATA *info,
                           char *varname, int **index )
    {

    int n, varid;
    long start[32];       /* starting positions for extracting data */
    long count[32];       /* number of each dimension to extract */
    int ncol, nrow, nlevel, nstep;
    int col0, row0, level0, step0;
    int fd = NC_SYSERR;


    ncol = info->col_max - info->col_min + 1;
    nrow = info->row_max - info->row_min + 1;

    nrow = nrow_full;
    ncol = ncol_full;

    /* very cheesy fix for scaling */
    /*
      info->row_min = 1;
      info->col_min = 1;
    */
    nlevel = info->level_max - info->level_min + 1;
    nstep = ( ( info->step_max - info->step_min ) /info->step_incr ) + 1;

    col0 = info->col_min - 1;
    row0 = info->row_min - 1;
    level0 = info->level_min - 1;
    step0 = info->step_min - 1;


    /*  n  = ncol * nrow * nlevel * nstep; */
    n  = ncol * nrow;

    *index = ( int * ) malloc ( n*sizeof ( *index[0] ) );
    if ( ! ( *index ) )
        {
        sprintf ( message,"malloc error in toplats_get_data" );
        goto INDEX_FAILURE;
        }

    if ( ( fd = ncopen ( info->filename,NC_NOWRITE ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s: file %s", "Error calling ncopen",info->filename );
        goto INDEX_FAILURE;
        }

    if ( ( varid = ncvarid ( fd,varname ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s for var %s", "Error calling ncvarid",varname );
        goto INDEX_FAILURE;
        }

    count[0] = 1; /* nstep; */
    count[1] = nlevel;
    count[2] = nrow;
    count[3] = ncol;
    start[0] = step0;
    start[1] = level0;
    start[2] = row0;
    start[3] = col0;

    if ( ( ncvarget ( fd, varid, start, count, ( void * ) *index ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncvarget" );
        goto INDEX_FAILURE;
        }
    ncclose ( fd );
    return PAVE_SUCCESS;

INDEX_FAILURE:
    fprintf ( stderr,"ERROR: %s\n", message );
    if ( fd != NC_SYSERR ) ncclose ( fd );
    return FAILURE;
    }


/* =========================================================== */

int get_ncd_Vector ( char *message, char *filename, char *spc_name,
                     char *dim_name, float **grid, nc_type *datatype, int tstep )
    {


    int i, varid;


    int fd = NC_SYSERR;

    long start[32];       /* starting positions for extracting data */
    long count[32];       /* number of each dimension to extract */
    int nindex, idim;

    int ndims;        /* # dimensions in netCDF file */
    int nvars;        /* # variables in netCDF file */
    int ngatts;       /* # global attributes in netCDF file */
    int recdim;       /* id of unlimited dimension in netCDF file */
    int nvardims;     /* # dims for single variable in netCDF file */
    int nvaratts;     /* # atts for single variable in netCDF file */
    long dimsize;     /* size of an individual dimension */

    char dimname[32]; /* name of netCDF dimension */
    char varname[32]; /* name of netCDF variable */
    int vardim[32];   /* dimensions of netCDF variables */


    if ( ( fd = ncopen ( filename,NC_NOWRITE ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncopen" );
        goto VFAILURE;
        }

    if ( ( ncinquire ( fd, &ndims, &nvars, &ngatts, &recdim ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncinquire" );
        goto VFAILURE;
        }


    idim = -1;
    for ( i = 0; i < ndims; i++ )
        {
        count[i] = 1;
        start[i] = 0;
        if ( ( ncdiminq ( fd, i, dimname, &dimsize ) ) == NC_SYSERR )
            {
            sprintf ( message, "%s", "Error calling ncdiminq" );
            goto VFAILURE;
            }
        if ( !strcmp ( dimname, dim_name ) )
            {
            nindex = ( int ) dimsize;
            idim = i;
            }
        }

    start[0] += tstep;

    if ( idim == -1 )
        {
        sprintf ( message,"Cannot find dimension %s in the file %s",
                  dim_name, filename );
        goto VFAILURE;
        }

    *grid = ( float * ) malloc ( nindex*sizeof ( *grid[0] ) );
    if ( ! ( *grid ) )
        {
        sprintf ( message,"malloc error in toplats_get_data" );
        goto VFAILURE;
        }

    if ( ( varid = ncvarid ( fd, spc_name ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncvarid" );
        goto VFAILURE;
        }

    if ( ( ncvarinq ( fd, varid, varname, datatype, &nvardims,
                      vardim, &nvaratts ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncvarinq" );
        goto VFAILURE;
        }

    for ( i=0; i<nvardims; i++ )
        {
        if ( vardim[i] == idim ) count[i] = nindex;
        }

    if ( ( ncvarget ( fd, varid, start, count, *grid ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncvarget" );
        goto VFAILURE;
        }
    ncclose ( fd );

    return nindex;

VFAILURE:
    fprintf ( stderr,"ERROR: %s\n", message );
    if ( fd != NC_SYSERR ) ncclose ( fd );
    return FAILURE;
    }

/* =========================================================== */

int get_ncd_Slice ( char *message, char *filename, char *spc_name,
                    char *dimname1, char *dimname2,
                    void **grid, nc_type *datatype, int tstep )
    {


    int i, j, k, varid;
    char *dim_name[2];
    int dim2;

    int fd = NC_SYSERR;

    long start[32];       /* starting positions for extracting data */
    long count[32];       /* number of each dimension to extract */
    int nindex, idim;

    int ndims;        /* # dimensions in netCDF file */
    int nvars;        /* # variables in netCDF file */
    int ngatts;       /* # global attributes in netCDF file */
    int recdim;       /* id of unlimited dimension in netCDF file */
    int nvardims;     /* # dims for single variable in netCDF file */
    int nvaratts;     /* # atts for single variable in netCDF file */
    long dimsize;     /* size of an individual dimension */

    char dimname[32]; /* name of netCDF dimension */
    char varname[32]; /* name of netCDF variable */
    int vardim[32];   /* dimensions of netCDF variables */


    dim_name[0]=dimname1;
    dim_name[1]=dimname2;

    if ( ( fd = ncopen ( filename,NC_NOWRITE ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncopen" );
        goto VFAILURE;
        }

    if ( ( varid = ncvarid ( fd, spc_name ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncvarid" );
        goto VFAILURE;
        }

    if ( ( ncinquire ( fd, &ndims, &nvars, &ngatts, &recdim ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncinquire" );
        goto VFAILURE;
        }


    if ( ( ncvarinq ( fd, varid, varname, datatype, &nvardims,
                      vardim, &nvaratts ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncvarinq" );
        goto VFAILURE;
        }

    for ( i = 0; i < ndims; i++ )
        {
        count[i] = 1;
        start[i] = 0;
        if ( ( ncdiminq ( fd, i, dimname, &dimsize ) ) == NC_SYSERR )
            {
            sprintf ( message, "%s", "Error calling ncdiminq" );
            goto VFAILURE;
            }
        for ( k=0; k<2; k++ )
            {
            if ( !strcmp ( dimname, dim_name[k] ) )
                {
                nindex = ( int ) dimsize;
                idim = i;
                for ( j=0; j<nvardims; j++ )
                    {
                    if ( vardim[j] == idim ) count[j] = nindex;
                    }
                }
            } /* end-of-k-loop */
        }
    dim2 = nindex;

    start[0] += tstep;

    nindex = 1;
    for ( i = 0; i < ndims; i++ )
        {
        nindex *= count[i];
        }

    if ( *datatype == M3REAL )
        {
        *grid = ( float * ) malloc ( nindex*sizeof ( float ) );
        if ( ! ( *grid ) )
            {
            sprintf ( message,"malloc error in toplats_get_data" );
            goto VFAILURE;
            }
        }
    else if ( *datatype == M3INT )
        {
        *grid = ( int * ) malloc ( nindex*sizeof ( int ) );
        if ( ! ( *grid ) )
            {
            sprintf ( message,"malloc error in toplats_get_data" );
            goto VFAILURE;
            }
        }

    if ( ( ncvarget ( fd, varid, start, count, *grid ) ) == NC_SYSERR )
        {
        sprintf ( message, "%s", "Error calling ncvarget" );
        goto VFAILURE;
        }
    ncclose ( fd );

    return dim2;

VFAILURE:
    fprintf ( stderr,"ERROR: %s\n", message );
    if ( fd != NC_SYSERR ) ncclose ( fd );
    return FAILURE;
    }

/* =========================================================== */

