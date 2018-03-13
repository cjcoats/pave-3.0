/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: get_info_and_data.c 83 2018-03-12 19:24:33Z coats $
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
 * Author:  Kathy Pearson, MCNC, kathyp@mcnc.org, (919) 248-9240
 * Date:    December 12, 1994
 * Modified by : Rajini Balay
 *        Date : Feb 26, 1995
 *****************************************************************************/
 
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include "netcdf.h"
#include "vis_data.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#ifdef DO_TIMING
/* the next 3 include files are needed only for timing */
#include <sys/types.h>
#include <sys/times.h>
#include <limits.h>
#endif

#include <string.h>
#define PAVE_SUCCESS 1
#define FAILURE 0
#define MAXLINE 256

typedef  struct visdatalist
    {
    VIS_DATA *vdata;
    time_t mtime;
    struct visdatalist *next;
    } VisDataList;

typedef  struct metalist
    {
    char *filename;
    time_t mtime;
    VisDataList *vdlist;
    struct metalist *next;
    } MetaList;

#include "toplats.h"

/*********************** GLOBAL VARIABLES *********************/

int metameta;
static MetaList *mlhead=NULL;

/*********************** FUNCTION PROTOTYPES ******************/

int alpha_open ( int *vis_fd, VIS_DATA *info, char *message );
int alpha_get_data ( VIS_DATA *info, char *message );
int alpha_get_info ( VIS_DATA *info, char *message );

int uam_open     ( VIS_DATA *info, char *message );
int uam_get_data ( VIS_DATA *info, char *message );
int uam_get_info ( VIS_DATA *info, char *message );

VIS_DATA *VIS_DATA_dup ( VIS_DATA *, char * );





int get_info_local ( VIS_DATA *info, char *message )
    {

    int i, ret;
    int nstep, nstep_old;
    int first=1;
    char *meta_hdr = "#! LIST_OF_CHAINED_FILES";
    char line[MAXLINE];
    char filename[MAXLINE];
    char tmpfname[160]; /* SRT memory 960924 changed from char *tmpfname */
    VIS_DATA *vdata, *tmpvdata;
    VisDataList *vdlist;
    FILE *fp;

    MetaList *mlist, *tail=NULL;
    struct stat stat_str;
    time_t mtime;

    mlist = mlhead;
    vdlist = NULL;
    metameta = 0;
    stat ( info->filename, &stat_str );
    while ( mlist != NULL )
        {
        if ( !strcmp ( mlist->filename,info->filename ) )
            {
            metameta=1;    /* this is a META-meta file */
            vdlist=mlist->vdlist;
            mtime=mlist->mtime;
            break;
            }
        tail=mlist;
        mlist=mlist->next;
        }

    if ( metameta && stat_str.st_mtime > mtime )
        {
        sprintf ( message,"The METAMETAFILE %s has been modified since last time PAVE has read it\n", info->filename );
        return FAILURE;
        }

    if ( metameta == 0 )
        {
        fp = fopen ( info->filename, "r" );
        if ( fp==NULL )
            {
            sprintf ( message,
                      "ERROR: cannot open file %s for reading\n",
                      info->filename );
            return FAILURE;
            }

        i=strlen ( meta_hdr );
        fgets ( line,MAXLINE, fp );
        if ( !strncmp ( line, meta_hdr,i ) )
            {
            metameta=1;       /* this is a META-meta file */
            mtime=time ( NULL );
            mlist= ( MetaList * ) malloc ( sizeof ( MetaList ) );
            mlist->filename=strdup ( info->filename );
            mlist->next=NULL;
            mlist->mtime=mtime;
            /* insert the new MetaList info the linked list */
            if ( tail==NULL ) tail=mlhead=mlist;
            else tail->next=mlist;

            first=1;
            while ( fgets ( line, MAXLINE, fp ) != NULL )
                {
                sscanf ( line, "%s", filename );
                if ( first )
                    {
                    first=0;
                    vdlist= ( VisDataList * ) malloc ( sizeof ( VisDataList ) );
                    mlist->vdlist=vdlist;
                    }
                else
                    {
                    vdlist=vdlist->next= ( VisDataList * ) malloc ( sizeof ( VisDataList ) );
                    }
                vdata = ( VIS_DATA * ) malloc ( sizeof ( VIS_DATA ) );
                memset ( ( void * ) vdata, 0, ( size_t ) sizeof ( VIS_DATA ) );
                vdata->filename=strdup ( filename );
                ret=get_info_local1 ( vdata, message );
                if ( ret==FAILURE )
                    {
                    fclose ( fp );
                    return FAILURE;
                    }
                vdlist->vdata=vdata;
                vdlist->next=NULL;
                }
            }
        fclose ( fp );
        }

    first=1;
    if ( metameta )
        {
        vdlist=mlist->vdlist;
        while ( vdlist!=NULL )
            {
            vdata=vdlist->vdata;
            if ( first )
                {
                first=0;
                /*  tmpfname = info->filename;  SRT 960924 memory */
                strcpy ( tmpfname, info->filename ); /* SRT 960924 memory */
                free ( info->filename );        /* SRT 960924 memory */
                info->filename = strdup ( vdata->filename );
                ret=get_info_local1 ( info, message );
                if ( info->filename )       /* added 960919 SRT for memory management */
                    {
                    /* added 960919 SRT for memory management */
                    free ( info->filename ); /* added 960919 SRT for memory management */
                    info->filename = NULL;  /* added 960919 SRT for memory management */
                    }           /* added 960919 SRT for memory management */
                /*  info->filename = tmpfname;  SRT 960924 memory */
                info->filename = strdup ( tmpfname ); /* SRT 960924 memory */
                }
            else
                {
                /* make some checks for consistency */
                if ( ( info->dataset != vdata->dataset ) ||
                     ( info->ncol != vdata->ncol ) ||
                     ( info->nrow != vdata->nrow ) ||
                     ( info->nlevel != vdata->nlevel ) ||
                     ( info->nspecies != vdata->nspecies ) )
                    {
                    sprintf ( message,"%s\n%s\n%s",
                              "ERROR (parsing): inconsistent data in the chained file:",
                              "all files must have identical number of columns, rows, layers, species, ",
                              "and same datatypes" );
                    fprintf ( stderr,"%s\n",message );
                    return FAILURE;
                    }
                /* update info struct */
                nstep = vdata->nstep;
                nstep_old = info->nstep;
                info->nstep += nstep;
                info->step_max += vdata->step_max;
                info->sdate= ( int * ) realloc ( info->sdate,info->nstep*sizeof ( int ) );
                info->stime= ( int * ) realloc ( info->stime,info->nstep*sizeof ( int ) );
                if ( ( info->sdate==NULL ) || ( info->stime==NULL ) )
                    {
                    sprintf ( message, "Realloc error in get_info_local\n" );
                    return FAILURE;
                    }
                for ( i=0; i<nstep; i++ )
                    {
                    info->sdate[i+nstep_old]=vdata->sdate[i];
                    info->stime[i+nstep_old]=vdata->stime[i];
                    }
                }
            vdlist=vdlist->next;
            } /* end of while ... */
        }
    else
        {
        metameta = 0;
        ret = get_info_local1 ( info, message );
        }

    return ret;

    }




int get_info_local1 ( VIS_DATA *info, char *message )
    {
    int ret;

#ifdef DO_TIMING
    /* timing stuff */
    struct tms tm1, tm2;

    clock_t t1, t2;
    float etime, utime, stime;
#endif

    static VisDataList *vdlist, *head=NULL;
    VisDataList *tmpvdlist, *tail;
    VIS_DATA *tmpvdata;
    struct stat stat_str;

#ifdef DIAGNOSTICS
    if ( info ) if ( info->filename ) fprintf ( stderr, "Enter get_info_local1() with '%s'\n",
                    info->filename );
#endif /* #ifdef DIAGNOSTICS */

    vdlist = head;
    stat ( info->filename, &stat_str );
    while ( vdlist != NULL )
        {
        if ( !strcmp ( vdlist->vdata->filename,info->filename ) )
            {
            /* && !strcmp(vdlist->vdata->filehost.name, info->filehost.name)) {*/
            if ( stat_str.st_mtime >= vdlist->mtime )
                {
                printf ( "PAVE action: file %s is newer then the last read\n\
FILE time = %s, DATABASE time = %s\n",
                         info->filename,
                         asctime ( localtime ( &stat_str.st_mtime ) ),
                         asctime ( localtime ( &vdlist->mtime ) ) );

                free_vis ( vdlist->vdata );

                vdlist->mtime=time ( NULL );
                ret=get_info_local2 ( info, message );
                vdlist->vdata = VIS_DATA_dup ( info,message );

                return ( vdlist->vdata ? ret : FAILURE );
                }

            tmpvdata = VIS_DATA_dup ( vdlist->vdata,message );
            if ( tmpvdata==NULL )
                return FAILURE;

            /* Save some fields from the current info struct */
            tmpvdata->filehost = info->filehost;

            if ( info->filename )
                {
                free ( info->filename );
                info->filename=NULL; /* SRT 960924 memory */
                }

            /* copy the VIS_DATA from the linked list to info */
            memcpy ( info, tmpvdata, sizeof ( VIS_DATA ) );
            free ( tmpvdata );
            return PAVE_SUCCESS;
            }
        tail=vdlist;
        vdlist=vdlist->next;
        }
    tmpvdlist = ( VisDataList * ) malloc ( sizeof ( VisDataList ) );
    if ( tmpvdlist==NULL )
        {
        strcpy ( message,"Malloc error - VisDataList in get_local_info" );
        return FAILURE;
        }
    if ( head==NULL )
        {
        head=tmpvdlist;
        }
    else
        {
        tail->next=tmpvdlist;
        }
    vdlist = tmpvdlist;
    vdlist->next = NULL;

#ifdef DO_TIMING
    t1=times ( &tm1 );
#endif

    vdlist->mtime=time ( NULL );
    ret=get_info_local2 ( info, message );

#ifdef DO_TIMING
    t2=times ( &tm2 );
    etime= ( float ) ( t2-t1 ) /CLK_TCK;
    utime= ( float ) ( tm2.tms_utime-tm1.tms_utime ) /CLK_TCK;
    stime= ( float ) ( tm2.tms_stime-tm1.tms_stime ) /CLK_TCK;
    printf ( "GET_INFO_TIMING:\n Elapsed: %f\n CPU(user): %f\n CPU(sys): %f\n",
             etime, utime, stime );
#endif

    vdlist->vdata = VIS_DATA_dup ( info,message );

    return ( vdlist->vdata ? ret : FAILURE );
    }


int get_info_local2 ( VIS_DATA *info, char *message )
    {
    int fd;
    FILE *mfp;
    int ret;
    TOPLATS_INFO tinfo;

    /* check for memory leaks!!! */
    if ( info->sdate!=NULL )
        {
        fprintf ( stderr,"%s\n",
                  "WARNING: possible memory leak -> SDATE is not NULL" );
        /* good place to free the pointer */
        info->sdate=NULL;
        }
    if ( info->stime!=NULL )
        {
        fprintf ( stderr,"%s\n",
                  "WARNING: possible memory leak -> STIME is not NULL" );
        /* good place to free the pointer */
        info->stime=NULL;
        }

    if ( alpha_open ( &fd, info, message ) )
        {
        ncclose ( fd );
        return ( alpha_get_info ( info, message ) );
        }
    else if ( toplats_open ( &tinfo,info,message ) )
        {
        return toplats_get_info ( &tinfo,info,message );
        }
    else if ( uamv_open ( info, message ) )
        {
        return ( uamv_get_info ( info, message ) );

        }
    else if ( uam_open ( info, message ) )
        {
        return ( uam_get_info ( info, message ) );
        }
    fprintf ( stderr,"%s\n",
              "File FMT not recognized. Not NETCDF, UAM-IV, UAM-V nor TOPLATS" );
    return FAILURE;
    }

#define min(a,b) (a)<(b) ? (a):(b)
#define max(a,b) (a)>(b) ? (a):(b)

int get_data_local ( VIS_DATA *info, char *message )
    {
    int ret;
    int i, n, nfloats;
    MetaList *mlist;
    VisDataList *vdlist=NULL;
    VIS_DATA *vdata, *vdatadup = ( VIS_DATA * ) NULL;
    int *sdate, *stime;
    int smin, smax, step_min, step_max;


    nfloats = ( info->col_max-info->col_min+1 ) *
              ( info->row_max-info->row_min+1 ) *
              ( info->level_max-info->level_min+1 ) *
              ( info->step_max-info->step_min+1 );


    info->grid= ( float * ) malloc ( ( nfloats ) *sizeof ( float ) );
    if ( info->grid==NULL )
        {
        sprintf ( message, "malloc failure in get_data_local!" );
        return FAILURE;
        }

    if ( metameta )
        {
        mlist = mlhead;
        while ( mlist != NULL )
            {
            if ( !strcmp ( mlist->filename,info->filename ) )
                {
                vdlist=mlist->vdlist;
                break;
                }
            mlist=mlist->next;
            }

        step_min = info->step_min;
        step_max = info->step_max;
        sdate = info->sdate;
        stime = info->stime;
        n = 0;
        while ( vdlist != NULL )
            {
            vdata = vdlist->vdata;
            vdlist=vdlist->next;
            smin = max ( vdata->step_min, step_min );
            smax = min ( vdata->step_max, step_max );
            if ( smin <= smax )
                {
                if ( vdatadup )              /* added 960919 SRT for memory management */
                    {
                    /* added 960919 SRT for memory management */
                    free_vis ( vdatadup );       /* added 960919 SRT for memory management */
                    free ( ( char * ) vdatadup ); /* added 960920 SRT for memory management */
                    vdatadup = ( VIS_DATA * ) NULL; /* added 960919 SRT for memory management */
                    }                /* added 960919 SRT for memory management */
                vdatadup = VIS_DATA_dup ( vdata,message );
                if ( vdatadup==NULL ) return FAILURE;

                vdatadup->slice = info->slice;
                vdatadup->selected_species = info->selected_species;
                vdatadup->selected_col = info->selected_col;
                vdatadup->selected_row = info->selected_row;
                vdatadup->selected_level = info->selected_level;
                vdatadup->col_min = info->col_min;     /*SRT added 961025 prevents crash*/
                vdatadup->col_max = info->col_max;     /*SRT added 961025 prevents crash*/
                vdatadup->row_min = info->row_min;     /*SRT added 961025 prevents crash*/
                vdatadup->row_max = info->row_max;     /*SRT added 961025 prevents crash*/
                vdatadup->level_min = info->level_min; /*SRT added 961025 prevents crash*/
                vdatadup->level_max = info->level_max; /*SRT added 961025 prevents crash*/
                vdatadup->step_min=smin;
                vdatadup->step_max=smax;
                nfloats = ( vdatadup->col_max-vdatadup->col_min+1 ) *
                          ( vdatadup->row_max-vdatadup->row_min+1 ) *
                          ( vdatadup->level_max-vdatadup->level_min+1 ) *
                          ( smax-smin+1 );

                ret = get_data_local1 ( vdatadup, message );
                memcpy ( info->grid+n, vdatadup->grid, ( size_t ) ( nfloats*sizeof ( float ) ) );
                n += nfloats;
                for ( i=0; i<=smax-smin; i++ )
                    {
                    *sdate++=vdatadup->sdate[i];
                    *stime++=vdatadup->stime[i];
                    }
                free_vis ( vdatadup );
                free ( ( char * ) vdatadup );
                vdatadup= ( VIS_DATA * ) NULL;
                }
            step_min -= vdata->nstep;
            step_max -= vdata->nstep;
            }
        }
    else
        {
        ret = get_data_local1 ( info, message );
        }
    if ( vdatadup )             /* added 960919 SRT for memory management */
        {
        /* added 960919 SRT for memory management */
        free_vis ( vdatadup );       /* added 960919 SRT for memory management */
        free ( ( char * ) vdatadup ); /* added 960920 SRT for memory management */
        vdatadup = ( VIS_DATA * ) NULL; /* added 960919 SRT for memory management */
        }                /* added 960919 SRT for memory management */
    return ret;
    }


int get_data_local1 ( VIS_DATA *info, char *message )
    {
    int fd;
    FILE *mfp;
    int ret;
    TOPLATS_INFO tinfo;

#ifdef DIAGNOSTICS
    if ( info ) if ( info->filename ) fprintf ( stderr, "Enter get_data_local1() with '%s'\n",
                    info->filename );
#endif /* #ifdef DIAGNOSTICS */

    fflush ( stdout );
    if ( alpha_open ( &fd, info, message ) )
        {
        ncclose ( fd );
        return ( alpha_get_data ( info, message ) );
        }
    else if ( toplats_open ( &tinfo,info,message ) )
        {
        return toplats_get_data ( &tinfo,info,message );
        }
    else if ( uamv_open ( info, message ) )
        {
        return ( uamv_get_data ( info, message ) );
        }
    else if ( uam_open ( info, message ) )
        {
        return ( uam_get_data ( info, message ) );
        }
    return FAILURE;
    }

