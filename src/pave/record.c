/*#define HEAVY_DIAGNOSTICS*/
/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: record.c 83 2018-03-12 19:24:33Z coats $
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
 *      Authors:  Kathy Pearson, Atanas Trayanov, NCSC, April, 1994
 *      Modified:    January 31, 1995    KLP 1/31/95 malloc/no free
 *      Modified:    May 16, 1995        SRT * - hdr_info->hour1 SRT
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "readuam.h"
#include "vis_data.h"
#include "utils.h"

extern int dump_hdrStruct ( UAM_INFO *u, char *fname /* optional arg, can use NULL */ );

/*
#define DEBUG 1
*/

/*
#define DEBUG 1
#define HEAVY_DIAGNOSTICS 1
*/

/* ... definitions ... */

#define MAX_BUF_SIZE 2048
#define ERROR(x) fprintf(stderr, "%s\n",x)
#define ERROR2(x1,x2) fprintf(stderr, x1, x2)
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define MAX(a,b)    ((a)>(b)?(a):(b))
#define GET_DATA(a,b)   (a+b*WORD_SIZE)

static int hr_nptsr[24];
int rec_read ( char *buf, int n, int offset );

int convert=0;

/********************************************************************/
/* UAM Header Routine                                               */
/********************************************************************/

FILE *fp;

int uam_fetch_header ( char *filename, UAM_INFO *hdr_info )
    {
    int i, j, k;

#ifdef UNICOS
    int unicos_k;
#endif

    static char s_str[MAX_BUF_SIZE];
    int *ibuf;
    float *fbuf, grid_x, grid_y;
    static char str[MAXHEADER];
    static char buf[MAX_BUF_SIZE];
    struct stat statbuf;
    int uam_filesize;
    int hour1, hour2, date, irow, icol, ispec, ilevel;
    int datapos, levellen, speclen, steplen;
    int nptsr;
    static char c;
    int blength;
    time_t t1, t2, hoursSince1970();
    int cw, uam_type;
    void flip();

    char timebuf[4*sizeof ( int )];
    int nstep, need_timeinfo;
    int *sdate, *stime;
    int dummy;
    float hour;
    static struct file_type
        {
        char *name;
        int type;
        } f_type[]=
        {
        "AVERAGE   ", UAM_AVER,
        "INSTANT   ", UAM_INST,
        "EMISSIONS ", UAM_EMIS,
        "WIND      ", UAM_WIND,
        "AIRQUALITY", UAM_AIRQ,
        "DIFFBREAK ", UAM_DIFF,
        "REGIONTOP ", UAM_REGT,
        "TEMPERATUR", UAM_TEMP,
        "TOPCONC   ", UAM_TOPC,
        "PTSOURCE  ", UAM_PTSR,
        "VERTVEL   ", UAM_VERT,
        "BOUNDARY  ", UAM_BNDRY
        };

    sdate=hdr_info->sdate;
    stime=hdr_info->stime;

    if ( ( fp = fopen ( filename,"r" ) ) == NULL )
        {
        ERROR2 ( "Cannot open for read input file %s.\n", filename );
        return ( FAILURE );
        }
    /* get file size */
    if ( stat ( filename, &statbuf ) == IOERROR )
        {
        ERROR ( "Cannot get input file size in UAM_Extract." );
        return ( FAILURE );
        }
    if ( ( uam_filesize= ( int ) statbuf.st_size ) <= 0 )
        {
        ERROR ( "Size of input file is zero in UAM_Extract." );
        return ( FAILURE );
        }
    /* the following modification is needed to help PAVE read ALL files */
    if ( fread ( &cw,sizeof ( cw ), 1, fp ) != 1 )
        {
        ERROR ( "Cannot read the first record control word." );
        return ( FAILURE );
        }
    rewind ( fp ); /* set the file pointer to the beginning of the file */
    convert=0; /* this is the default: read the file as is */

    if ( cw!=FILE_DESC_HEADER*sizeof ( cw ) )
        {
        convert = 1;
        /* if further checks are needed we can do this: */
        flip ( ( char * ) &cw, sizeof ( cw ) );
        if ( cw != FILE_DESC_HEADER*sizeof ( cw ) )
            {
            ERROR ( "UAM-IV internal file format error: Inappropriate first record control word." );
            return ( FAILURE );
            }
        }

    /* get file description header */
    if ( rec_read ( buf, FILE_DESC_HEADER, NO_OFFSET ) <0 )
        {
        ERROR ( "Cannot read file description header in UAM_Extract." );
        return ( FAILURE );
        }
    /* get uam_type -- file name from file description header record */

    for ( i = 0; i < FDH_FILEID; i++ ) str[i] = buf[i * WORD_SIZE + convert*3];
    str[i] = '\0';

#ifdef DEBUG
    fprintf ( stderr,"File type %s\n", str );
#endif
    k=sizeof ( f_type ) / ( sizeof ( f_type[0] ) );
    uam_type = UNKNOWN;
    for ( i=0; i<k; i++ )
        {
        if ( strncmp ( str,f_type[i].name,strlen ( f_type[i].name ) ) == 0 )
            {
            uam_type = f_type[i].type;
            break;
            }
        }
    hdr_info->uam_type = uam_type;

    /* file id in words 11-70 */
    for ( i = 0; i < MAXHEADER-1 /* -1 added 950718 SRT */; i++ ) str[i]=buf[ ( FDH_FILEID+i ) * WORD_SIZE+convert*3];
    str[i] = '\0';
    for ( i = MAXHEADER-1; i >= 0; --i ) /* strip trailing blanks */
        {
        if ( str[i] > ' ' ) break;
        str[i] = '\0';
        }
#ifdef DEBUG
    fprintf ( stderr, "Title string has %d characters\n",strlen ( str ) );
    fprintf ( stderr, "Title = %s\n",str );
#endif

    hdr_info->file_id=strdup ( str );/*str;*/ /* added strdup 950718 SRT */

    ibuf = ( int * )   GET_DATA ( buf, FDH_CHEM ); /* # of chemical species */
    ispec = *ibuf;

    ibuf = ( int * )   GET_DATA ( buf, FDH_BEGD ); /* beginning date */
    date = *ibuf;
    fbuf = ( float * ) GET_DATA ( buf, FHD_BEGH ); /* beginning hour */
    hour1 = ( int ) *fbuf;

#ifdef HEAVY_DIAGNOSTICS
    fprintf ( stderr, "\n------------------\n" );
    fprintf ( stderr, "record.c just read:\n" );
    fprintf ( stderr, "------------------\n" );
    fprintf ( stderr, "ispec == %d (# of chemical species)\n", ispec );
    fprintf ( stderr, " date == %d (beginning date)\n", date );
    fprintf ( stderr, "hour1 == %d (beginning hour)\n", hour1 );
#endif /*#ifdef HEAVY_DIAGNOSTICS */

    dummy = UAM_DATA;
    t1=hoursSince1970 ( &date, *fbuf, &dummy );
    hour1 = dummy;
    hdr_info->begin_date = date;

#ifdef HEAVY_DIAGNOSTICS
    fprintf ( stderr, "Just set hdr_info->begin_date to %d\n", hdr_info->begin_date );
#endif /*#ifdef HEAVY_DIAGNOSTICS */

    ibuf = ( int * )   GET_DATA ( buf, FDH_ENDD ); /* ending date */
    date = *ibuf;
    fbuf = ( float * ) GET_DATA ( buf, FDH_ENDH ); /* ending hour */
    hour2 = ( int ) *fbuf;

#ifdef HEAVY_DIAGNOSTICS
    fprintf ( stderr, " date == %d (ending date)\n", date );
    fprintf ( stderr, "hour2 == %d (ending hour)\n", hour2 );
#endif /*#ifdef HEAVY_DIAGNOSTICS */

    dummy = UAM_DATA;
    t2=hoursSince1970 ( &date, *fbuf, &dummy );
    hdr_info->end_date = date;
    hour2 = dummy;

#ifdef HEAVY_DIAGNOSTICS
    fprintf ( stderr, "Just set hdr_info->end_date to %d\n", hdr_info->end_date );
#endif /*#ifdef HEAVY_DIAGNOSTICS */

    if ( t1<0 || t2<0 )
        {
        ERROR ( "Cannot interpret date/time info to reasonable values" );
        return FAILURE;
        }

#if DEBUG
    fprintf ( stderr, "time step header (%d,%d,%d,%d)\n",
              hdr_info->begin_date, hour1,
              hdr_info->end_date, hour2 );
#endif


    if ( uam_type == UNKNOWN ) return ( PAVE_SUCCESS );

    /* get region description header */

    if ( rec_read ( buf, REGN_DESC_HEADER, NO_OFFSET ) <0 )
        {
        ERROR ( "Cannot read region header in UAM_Extract." );
        return ( FAILURE );
        }

    /* set x and y extents */

    fbuf = ( float * ) GET_DATA ( buf, RGN_XCRD ); /* x-coordinate */
    hdr_info->sw_utmx = *fbuf;
    fbuf = ( float * ) GET_DATA ( buf, RGN_YCRD ); /* y-coordinate */
    hdr_info->sw_utmy = *fbuf;

#ifdef HEAVY_DIAGNOSTICS
    fprintf ( stderr, "hdr_info->sw_utmx == %f (x coord RGN_XCRD)\n", hdr_info->sw_utmx );
    fprintf ( stderr, "hdr_info->sw_utmy == %f (y coord RGN_YCRD)\n", hdr_info->sw_utmy );
#endif /*#ifdef HEAVY_DIAGNOSTICS */

    ibuf = ( int * )   GET_DATA ( buf, RGN_UTMZ ); /* UTM zone */
    hdr_info->utm_zone = *ibuf;
    fbuf = ( float * ) GET_DATA ( buf, RGN_XLOC ); /* x-location */
    hdr_info->sw_utmx += *fbuf;
    fbuf = ( float * ) GET_DATA ( buf, RGN_YLOC ); /* y-location */
    hdr_info->sw_utmy += *fbuf;
    fbuf = ( float * ) GET_DATA ( buf, RGN_GCSX ); /* x-dir. grid cell size */
    grid_x = ( *fbuf );
    fbuf = ( float * ) GET_DATA ( buf, RGN_GCSY ); /* y-dir. grid cell size */
    grid_y = ( *fbuf );
    ibuf = ( int * )   GET_DATA ( buf, RGN_NGCZ ); /* # grid cells in z-dir. */
    ilevel = *ibuf;

#ifdef HEAVY_DIAGNOSTICS
    fprintf ( stderr, "hdr_info->utm_zone == %d (utm zone )\n", hdr_info->utm_zone );
    fprintf ( stderr, "hdr_info->sw_utmx == %f (x loc RGN_XLOC)\n", hdr_info->sw_utmx );
    fprintf ( stderr, "hdr_info->sw_utmy == %f (y loc RGN_YLOC)\n", hdr_info->sw_utmy );
    fprintf ( stderr, "grid_x == %f (x-dir. grid cell size RGN_GCSX)\n", grid_x );
    fprintf ( stderr, "grid_y == %f (y-dir. grid cell size RGN_GCSY)\n", grid_y );
    fprintf ( stderr, "ilevel == %d (# grid cells in z-dir RGN_NGCZ)\n", ilevel );
#endif /*#ifdef HEAVY_DIAGNOSTICS */

    /* get segment description headers */

    if ( rec_read ( buf, SEGM_DESC_HEADER, NO_OFFSET ) <0 )
        {
        ERROR ( "Cannot read segment header in UAM_Extract." );
        return ( FAILURE );
        }

    ibuf = ( int * ) GET_DATA ( buf, SEG_XLOC ); /* segment x-location */

#ifdef HEAVY_DIAGNOSTICS
    fprintf ( stderr, "*ibuf == %d (segment x loc)\n", *ibuf );
#endif /*#ifdef HEAVY_DIAGNOSTICS */

    hdr_info->sw_utmx += ( float ) *ibuf;
    ibuf = ( int * ) GET_DATA ( buf, SEG_YLOC ); /* segment y-location */

#ifdef HEAVY_DIAGNOSTICS
    fprintf ( stderr, "*ibuf == %d (segment y loc)\n", *ibuf );
#endif /*#ifdef HEAVY_DIAGNOSTICS */

    hdr_info->sw_utmy += ( float ) *ibuf;
    ibuf = ( int * ) GET_DATA ( buf, SEG_NGCX ); /* # segment grid cells in x-dir */
    icol = hdr_info->icol = *ibuf;
    ibuf = ( int * ) GET_DATA ( buf, SEG_NGCY ); /* # segment grid cells in y-dir */
    irow = hdr_info->irow = *ibuf;

    hdr_info->ne_utmx = hdr_info->sw_utmx + icol*grid_x;
    hdr_info->ne_utmy = hdr_info->sw_utmy + irow*grid_y;

#ifdef HEAVY_DIAGNOSTICS
    fprintf ( stderr, "hdr_info->sw_utmx now == %f\n", hdr_info->sw_utmx );
    fprintf ( stderr, "hdr_info->sw_utmy now == %f\n", hdr_info->sw_utmy );
    fprintf ( stderr, "hdr_info->icol == %d\n", hdr_info->icol );
    fprintf ( stderr, "hdr_info->irow == %d\n", hdr_info->irow );
    fprintf ( stderr, "set hdr_info->ne_utmx to %f ()\n", hdr_info->ne_utmx );
    fprintf ( stderr, "set hdr_info->ne_utmy to %f ()\n", hdr_info->ne_utmy );
#endif /*#ifdef HEAVY_DIAGNOSTICS */

    if ( ispec != 0 )
        {
        /* get species description header */
        if ( rec_read ( buf, ispec*SPC_SPEC, NO_OFFSET ) <0 )
            {
            ERROR ( "Cannot read species header in UAM_Extract." );
            return ( FAILURE );
            }
        /* KLP malloc/no free */
        /* remove this statement */
        /*
                if ((s_str=(char *) malloc(SPC_SPEC*ispec))==NULL) return (FAILURE);
        */
        for ( i=k=0; i<ispec; i++ )
            {
            for ( j=0; j<SPC_SPEC; j++ )
                {
                c=buf[ ( SPC_SPEC*i+j ) *WORD_SIZE+convert*3];
                if ( c==' ' ) break;
                s_str[k++]=c;
                }
            s_str[k++]=':';
            }
        /*
                s_str[--k]='\0';
        */
        /* -- for this application, add an extra colon at the end of the string */
        s_str[k]='\0';
#ifdef DEBUG
        fprintf ( stderr, "species header = *%s*\n", s_str );
#endif
        hdr_info->spec_list = strdup ( s_str ); /* s_str added strdup 950718 SRT */
        }
    else    hdr_info->spec_list = NULL;
    if ( uam_type == UAM_WIND ) ispec=2;

    datapos  = FILE_DESC_HEADER + REC_CW
               + REGN_DESC_HEADER + REC_CW
               + SEGM_DESC_HEADER + REC_CW;

    if ( ( ispec != 0 ) && ( uam_type != UAM_WIND ) )
        datapos += SPC_SPEC*ispec + REC_CW;

    switch ( uam_type )
        {
        case UAM_DIFF:
        case UAM_TEMP:
        case UAM_REGT:
            ilevel = 1;
            ispec = 1;
            break;
        case UAM_EMIS:
        case UAM_TOPC:
            ilevel = 1;
            break;
        }

    if ( ispec == 0 ) ispec = 1;
    levellen = UAM_REC_OFFSET + icol * irow;
    speclen = ilevel * ( levellen + REC_CW );
    if ( uam_type == UAM_BNDRY )
        {
        blength=2* ( icol+irow );
        datapos+=blength*4+12+4*REC_CW;
        speclen=4* ( REC_CW + 12 ) + blength*ilevel;
        }
    steplen = TIME_STEP_HEADER + REC_CW
              + ispec * speclen;
    if ( uam_type == UAM_WIND )
        {
        steplen +=  WIND_SCAL_HEADER + REC_CW;
        speclen = UAM_REC_OFFSET + icol * irow + REC_CW;
        levellen = ispec * speclen - REC_CW;
        }
    if ( uam_type == UAM_PTSR )
        {
        /* get the time-invariant info from the point source file */
#ifdef DEBUG
        printf ( "FTELL before Time-Invariant %d\n", ftell ( fp ) );
        printf ( "DATAPOS(words) for same record %d\n", datapos );
#endif
        if ( rec_read ( buf, TIME_INV_CNTR_REC, NO_OFFSET ) <0 )
            {
            ERROR ( "Cannot read counter record in UAM_Extract." );
            return ( FAILURE );
            }
        ibuf = ( int * ) GET_DATA ( buf, TIC_NPTS ); /* number of point sources in segment */
        nptsr = *ibuf;
#ifdef DEBUG
        fprintf ( stderr, "Header info: # of point sources=%d\n", nptsr );
#endif
        datapos += TIME_INV_CNTR_REC + REC_CW
                   +  nptsr * POINT_SOURCE_DEF + REC_CW;
        }

#ifdef DEBUG
    fprintf ( stderr,"datapos=%d\n",datapos );
    fprintf ( stderr,"levellen=%d\n",levellen );
    fprintf ( stderr,"steplen=%d\n",steplen );
    fprintf ( stderr,"speclen=%d\n",speclen );
#endif

    /*******************************************
        hour2 = hour1 +t2-t1-1;
        if (hour2 < hour1) hour2 = hour1;
    ********************************************/
    if ( uam_type == UAM_PTSR )
        {
        k = datapos;
        nstep = t2 - t1;
#ifdef DEBUG
        printf ( "NSTEP=%d hour1=%d, hour2=%d, t1=%d, t2=%d\n",
                 nstep, hour1, hour2, t1,t2 );
#endif
        need_timeinfo = 0;
        if ( ( sdate == NULL ) || ( stime==NULL ) )
            {
            need_timeinfo = 1;
            sdate= ( int * ) malloc ( nstep*sizeof ( sdate[0] ) );
            if ( sdate==NULL )
                {
                ERROR ( "Allocation error: sdate" );
                return ( FAILURE );
                }

            stime= ( int * ) malloc ( nstep*sizeof ( stime[0] ) );
            if ( stime==NULL )
                {
                ERROR ( "Allocation error: stime" );
                return ( FAILURE );
                }
            }
        for ( i = 0; i < nstep; i++ )
            {
            if ( i > 23 )
                break;

            /* read counter record to see how many point sources in this step */

#ifdef UNICOS
            unicos_k = k + ( k/511 );
            if ( fseek ( fp, unicos_k*WORD_SIZE, SEEK_SET ) != 0 )
#else
            if ( fseek ( fp, k*WORD_SIZE, SEEK_SET ) != 0 )
#endif
                {
                ERROR ( "File seek error 1" );
                return ( IOERROR );
                }
#ifdef DEBUG
            printf ( "FTELL before Time-Variant %d\n", ftell ( fp ) );
#endif
            if ( rec_read ( ( char * ) timebuf, 4, 0 ) <0 ) /* get time info */
                {
                ERROR ( "Error on read timeinfo in UAMV_Extract." );
                return ( FAILURE );
                }
            ibuf = ( int   * ) GET_DATA ( timebuf,0 );
            sdate[i] = *ibuf;
            fbuf = ( float * ) GET_DATA ( timebuf,1 );
            hour = *fbuf;
            stime[i] = UAM_DATA;
            hoursSince1970 ( &sdate[i], hour, &stime[i] );
#ifdef DEBUG
            printf ( "DATE info: %d %d\n",sdate[i], stime[i] );
#endif
            if ( rec_read ( ( char * ) buf, 2, NO_OFFSET ) <0 )
                {
                ERROR ( "Error on read data in UAM_Extract." );
                return ( FAILURE );
                }
            ibuf = ( int * ) GET_DATA ( buf, 1 );
            hr_nptsr[i] = *ibuf;
            k += TIME_STEP_HEADER + REC_CW
                 + TIME_VAR_CNTR_REC + REC_CW
                 + hr_nptsr[i] * POINT_SOURCE_LOC + REC_CW
                 + ispec * ( UAM_REC_OFFSET + hr_nptsr[i] + REC_CW );
#if DEBUG
            fprintf ( stderr, "nptsr[%d] = %d\n", i, hr_nptsr[i] );
#endif
            }

        }
    else
        {
#ifdef UNICOS
        k = uam_filesize/ ( WORD_SIZE );
        k -= ( k/512 + 1 + REC_CW );
        k = ( k - datapos ) /steplen;
#else
        k= ( uam_filesize/ ( WORD_SIZE ) - datapos ) /steplen;
#endif
        if ( k < 1 )
            {
            ERROR ( "No time steps in the file" );
            return ( FAILURE );
            }
        /*
                if (k < hour2 - hour1 + 1)
                  {
                hour2 = hour1 + k -1;
                  }
        */
        nstep=k;
        need_timeinfo = 0;
        if ( ( sdate == NULL ) || ( stime==NULL ) )
            {
            need_timeinfo = 1;
            sdate= ( int * ) malloc ( nstep*sizeof ( sdate[0] ) );
            if ( sdate==NULL )
                {
                ERROR ( "Allocation error: sdate" );
                return ( FAILURE );
                }

            stime= ( int * ) malloc ( nstep*sizeof ( stime[0] ) );
            if ( stime==NULL )
                {
                ERROR ( "Allocation error: stime" );
                return ( FAILURE );
                }

            k=datapos*WORD_SIZE;
            /* Scanning file to fill-in the time info for every timestep */
            for ( i=0; i<nstep; i++ )
                {
                if ( fseek ( fp, k, SEEK_SET ) != 0 )
                    {
                    ERROR ( "File seek error" );
                    return ( IOERROR );
                    }
                if ( rec_read ( ( char * ) timebuf, 4, 0 ) <0 ) /* get time info */
                    {
                    ERROR ( "Error on read timeinfo in UAMV_Extract." );
                    return ( FAILURE );
                    }
                ibuf = ( int   * ) GET_DATA ( timebuf,0 );
                sdate[i] = *ibuf;
                fbuf = ( float * ) GET_DATA ( timebuf,1 );
                hour = *fbuf;
                if ( uam_type==UAM_INST )
                    {
                    hour= ( float ) ( ( int ) ( hour+0.5 ) );
                    }
                stime[i] = UAM_DATA;
                hoursSince1970 ( &sdate[i], hour, &stime[i] );
                k+=steplen*WORD_SIZE;
                }
            }
        }

    if ( need_timeinfo )
        {
        hour1 = stime[0]/10000;
        hour2 = stime[nstep-1]/10000;
        hdr_info->begin_date = sdate[0];
        hdr_info->end_date   = sdate[nstep-1];

        hdr_info->nstep=nstep;
        hdr_info->sdate=sdate;
        hdr_info->stime=stime;
        }
    if ( uam_type == UAM_WIND )
        {
        datapos += WIND_SCAL_HEADER + REC_CW;
        }
    hdr_info->hour1 = hour1;
    hdr_info->hour2 = hour2;
    hdr_info->ispec = ispec;
    hdr_info->ilevel = ilevel;
    hdr_info->datapos = datapos;
    hdr_info->levellen = levellen;
    hdr_info->speclen = speclen;
    hdr_info->steplen = steplen;

#ifdef DEBUG
    fprintf ( stderr, "filesize=%d\n",uam_filesize );
    fprintf ( stderr, "TimeSteps in the file %s = %d\n",filename,nstep );
    fprintf ( stderr, "hour1=%d\n",hour1 );
    fprintf ( stderr, "hour2=%d\n",hour2 );
#endif

#ifdef DIAGNOSTICS
    dump_hdrStruct ( hdr_info, filename );
#endif /* #ifdef DIAGNOSTICS */

    return ( PAVE_SUCCESS );
    }
/* ========================================================================== */
#define DEFAULT_MISSING_VALUE (-999.0)
int uam_fetch_data ( UAM_INFO *hdr_info, float *buf,
                     int n, int spec, int level, int hour )
    {
    int k;

#ifdef UNICOS
    int unicos_k;
#endif

    int i, j, xindex, yindex, zindex, index;
    float emis;
    float *fbuf;
    int *ibuf;
    char *locbuf = NULL;
    char *emisbuf = NULL;
    float missing_value;
    char *missing_value_str;

    if ( ( missing_value_str=getenv ( "UAM_MISSING_VALUE" ) ) == NULL )
        {
        missing_value = DEFAULT_MISSING_VALUE;
        }
    else
        {
        missing_value = atof ( missing_value_str );
        }

    if ( hdr_info->uam_type==UAM_BNDRY )
        return get_bndry_data ( hdr_info, buf, n, spec, level, hour );
    if ( spec >= hdr_info->ispec )
        {
        ERROR ( "spec too big" );
        return ( FAILURE );
        }
    if ( level >= hdr_info->ilevel )
        {
        ERROR ( "level too big" );
        return ( FAILURE );
        }

    if ( ( n + UAM_REC_OFFSET ) > hdr_info->levellen )
        {
        ERROR ( "too much data requested" );
        return ( FAILURE );
        }

    /*
     * Time Step Header Record
     * word 1:  integer     Julian beginning date
     * word 2:  float       beginning time
     * word 3:  integer     Julian ending date
     * word 4:  float       ending time
     */

    /*
     * ... within each time step
     * .......... within each layer
     * ................. Average ConCentration Record
     *
     * Average Concentration Records
     * word 1:  integer     segment number (must be 1 )
     * word 2-11:   character   species name (10 characters, 1 character/word)
     * word 12-N:   float       concentrations averaged over time interval
     */

    if ( hdr_info->uam_type == UAM_PTSR )
        {
        if ( hour > 23 )
            {
            ERROR ( "pt source hours 0-23 only accessible" );
            return ( FAILURE );
            }
        /* zero input buffer */
        for ( i = 0; i < n; i++ )
            buf[i] = 0.0;

        if ( ( locbuf = ( ( char * ) malloc ( sizeof ( int ) *
                                              hr_nptsr[hour] * POINT_SOURCE_LOC ) ) ) == NULL )
            return ( FAILURE );
        k = hdr_info->datapos;
        for ( i = 0; i < hour; i++ )
            {
            k += TIME_STEP_HEADER + REC_CW;
            k += TIME_VAR_CNTR_REC + REC_CW
                 + hr_nptsr[hour] * POINT_SOURCE_LOC + REC_CW
                 + hdr_info->ispec * ( UAM_REC_OFFSET + hr_nptsr[hour] + REC_CW );
            }
        k += TIME_STEP_HEADER + REC_CW;
        k += TIME_VAR_CNTR_REC + REC_CW;

        /* seek point source location record for requested hour */

#ifdef UNICOS
        unicos_k = k + ( k/511 );
        if ( fseek ( fp, unicos_k*WORD_SIZE, SEEK_SET ) != 0 )
#else
        if ( fseek ( fp, k*WORD_SIZE, SEEK_SET ) != 0 )
#endif
            {
            ERROR ( "File seek error 2" );
            free ( locbuf );
            return ( IOERROR );
            }

        /* read point source location records for each point source in segment */

        if ( rec_read ( ( char * ) locbuf,
                        hr_nptsr[hour] * POINT_SOURCE_LOC, NO_OFFSET ) < 0 )
            {
            ERROR ( "Error on read data in UAM_Extract." );
            free ( locbuf );
            return ( FAILURE );
            }

        if ( ( emisbuf = ( ( char * ) malloc ( sizeof ( float ) *
                                               hr_nptsr[hour] ) ) ) == NULL )
            {
            free ( locbuf );
            return ( FAILURE );
            }

        k += hr_nptsr[hour] * POINT_SOURCE_LOC + REC_CW;
        for ( j = 0; j < spec; j++ )
            k += ( UAM_REC_OFFSET + hr_nptsr[hour] + REC_CW );

        /* seek emissions record for requested hour and requested species */

#ifdef UNICOS
        unicos_k = k + ( k/511 );
        if ( fseek ( fp, unicos_k*WORD_SIZE, SEEK_SET ) != 0 )
#else
        if ( fseek ( fp, k*WORD_SIZE, SEEK_SET ) != 0 )
#endif
            {
            ERROR ( "File seek error 3" );
            free ( locbuf );
            free ( emisbuf );
            return ( IOERROR );
            }

        /* read emissions data for this point source */

        if ( rec_read ( ( char * ) emisbuf, hr_nptsr[hour],
                        UAM_REC_OFFSET ) <0 )
            {
            ERROR ( "Error on read data in UAM_Extract." );
            free ( locbuf );
            free ( emisbuf );
            return ( FAILURE );
            }
        i = j = 0;
        while ( i < hr_nptsr[hour] )
            {
            ibuf = ( int * ) GET_DATA ( locbuf, j );
            xindex = *ibuf;
            ibuf = ( int * ) GET_DATA ( locbuf, ( j+1 ) );
            yindex = *ibuf;
            ibuf = ( int * ) GET_DATA ( locbuf, ( j+2 ) );
            zindex = *ibuf;
            if ( zindex-1 == level )
                {
                fbuf = ( float * ) GET_DATA ( emisbuf, i );
                emis = *fbuf;
                if ( emis==missing_value )
                    {
                    emis=setNaNf();
                    }
                index = ( xindex-1 ) +
                        ( yindex-1 ) * hdr_info->icol;
                if ( ( index > 0 ) && ( index < n ) )
                    buf[index] += emis;
                }
            j += 5;
            ++i;
            }

        free ( locbuf );
        free ( emisbuf );
        }

    else
        {
        k = hdr_info->datapos
            + TIME_STEP_HEADER + REC_CW
            + ( hour /* - hdr_info->hour1 SRT */ ) * ( hdr_info->steplen )
            + spec * ( hdr_info->speclen )
            + level * ( REC_CW + hdr_info->levellen );

#ifdef UNICOS
        unicos_k = k + ( k/511 );
        if ( fseek ( fp, unicos_k*WORD_SIZE, SEEK_SET ) != 0 )
#else
        if ( fseek ( fp, k*WORD_SIZE, SEEK_SET ) != 0 )
#endif
            {
            ERROR ( "File seek error 4" );
            return ( IOERROR );
            }

        if ( rec_read ( ( char * ) buf, n, UAM_REC_OFFSET ) <0 ) /* get level */
            {
            ERROR ( "Error on read data in UAM_Extract." );
            return ( FAILURE );
            }
        for ( i=0; i<n; i++ )
            {
            if ( buf[i]==missing_value ) /* missing data in UAM-IV */
                {
                buf[i]=setNaNf();
                }
            }
        }
    return ( PAVE_SUCCESS );
    }
/* ========================================================================== */
int uam_close()
    {
    if ( fclose ( fp ) == 0 ) return ( PAVE_SUCCESS );
    ERROR ( "File closing error" );
    return ( IOERROR );
    }
/* ========================================================================== */
#ifdef UNICOS
int rec_read ( char *buf, int n, int offset )
    {
    int i, k, first_cw, cw;
    int type, skipbytes, gotbytes, getbytes;
    char tmp[512*WORD_SIZE];

#ifdef DEBUG
    fprintf ( stderr, "n = %d\n", n );
#endif
    n *= WORD_SIZE;
    offset *= WORD_SIZE;
    gotbytes = 0;

    skipbytes = 0;
    i = 0;
    while ( gotbytes < n + offset )
        {
        if ( fread ( &cw, sizeof ( cw ), 1, fp ) != 1 )
            {
            ERROR ( "Control word reading error" );
            return ( IOERROR );
            }
#ifdef DEBUG
        fprintf ( stderr, "cw:0%o\n", cw );
#endif
        type=cw>> ( WORD_SIZE*8-4 );
        if ( ( type == CW_EOF ) || ( type == CW_EOD ) )
            return ( FAILURE );
        getbytes = ( cw & 0777 ) * WORD_SIZE;
#ifdef DEBUG
        fprintf ( stderr, "get %d words\n", ( cw & 0777 ) );
        fprintf ( stderr, "getbytes = %d, 0%o\n", getbytes, getbytes );
#endif
        if ( fread ( tmp, sizeof ( tmp[0] ), getbytes, fp ) != getbytes )
            {
            ERROR ( "Blocked record read failed" );
            return ( IOERROR );
            }
        for ( k=0; k<getbytes; k++ )
            {
            if ( skipbytes < offset )
                ++skipbytes;
            else
                {
                buf[i++]=tmp[k];
                if ( i >= n )
                    break;
                }
            }
        gotbytes+=getbytes;
        }
    return ( PAVE_SUCCESS );
    }
#else
int rec_read ( char *buf, int n, int offset )
    {
    int cw, eor;
    void flip();

    n *= WORD_SIZE;
    offset *= WORD_SIZE;
    if ( fread ( &cw, sizeof ( cw ), 1, fp ) != 1 )
        {
        ERROR ( "Cannot read FORTRAN record control word" );
        return ( IOERROR );
        }
    if ( convert ) flip ( ( char * ) &cw,sizeof ( cw ) );
    if ( cw < n+offset )
        {
        ERROR ( "Record does not contain this much data" );
        fprintf ( stderr, "cw=%d n=%d offset=%d\n",cw,n,offset );
        return ( IOERROR );
        }
    if ( offset != 0 )
        {
        if ( fseek ( fp, offset, SEEK_CUR ) != 0 )
            {
            ERROR ( "File seek error 5" );
            return ( IOERROR );
            }
        }
    if ( fread ( buf, sizeof ( buf[0] ), n, fp ) != n )
        {
        ERROR ( "Record read failed" );
        return ( IOERROR );
        }
    if ( convert ) flip ( buf,n );
    offset += n;
    if ( cw > offset ) if ( fseek ( fp, ( cw-offset ), SEEK_CUR ) != 0 )
            {
            ERROR ( "File seek error 6" );
            return ( IOERROR );
            }
    if ( fread ( &eor, sizeof ( eor ), 1, fp ) != 1 )
        {
        ERROR ( "Cannot read FORTRAN end-of-record word" );
        return ( IOERROR );
        }
    if ( convert ) flip ( ( char * ) &eor,sizeof ( eor ) );
    if ( cw != eor )
        {
        ERROR ( "Record headers mismatch" );
        return ( IOERROR );
        }
    return ( PAVE_SUCCESS );
    }
#endif

/* ========================================================================= */
int get_bndry_data ( UAM_INFO *hdr_info, float *buf,
                     int n, int spec, int level, int hour )
    {
    int k;
    int i, size, wsize, esize, ssize, nsize;
    int nx, ny, nz;
    float *locbuf;

    /* make the entire region empty */
    for ( i=0; i<n; i++ ) buf[i]=setNaNf();

    nx=hdr_info->icol;
    ny=hdr_info->irow;
    nz=hdr_info->ilevel;
    nsize = ssize = nx*nz;
    wsize = esize = ny*nz;
    size = MAX ( nsize,wsize );

    locbuf = ( float * ) malloc ( size*sizeof ( locbuf[0] ) );
    if ( locbuf==NULL )
        {
        ERROR ( "Temp array allocation failure" );
        return FAILURE;
        }

    k = hdr_info->datapos
        + TIME_STEP_HEADER + REC_CW
        + hour * hdr_info->steplen
        + spec * hdr_info->speclen;

#ifdef UNICOS
    k += ( k/511 );
#endif

    if ( fseek ( fp, k*WORD_SIZE, SEEK_SET ) != 0 )
        {
        ERROR ( "File seek error 4" );
        free ( locbuf );
        return ( IOERROR );
        }

    if ( rec_read ( ( char * ) locbuf, wsize, 12 ) <0 ) /* get level */
        {
        ERROR ( "Error on read data in UAM_Extract." );
        free ( locbuf );
        return ( FAILURE );
        }
    for ( i=0; i<ny; i++ )
        {
        buf[i*nx]=locbuf[i*nz+level];
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr,
              "Just got W boundary data for spec %d, level %d, hour %d:\n",
              spec, level, hour );
    for ( i=0; i<ny; i++ )
        fprintf ( stderr,"locbuf[%d*nz+level]==%f\n", i, locbuf[i*nz+level] );
#endif

    if ( rec_read ( ( char * ) locbuf, esize, 12 ) <0 ) /* get level */
        {
        ERROR ( "Error on read data in UAM_Extract." );
        free ( locbuf );
        return ( FAILURE );
        }
    for ( i=0; i<ny; i++ )
        {
        buf[i*nx+ ( nx-1 )]=locbuf[i*nz+level];
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr,
              "Just got E boundary data for spec %d, level %d, hour %d:\n",
              spec, level, hour );
    for ( i=0; i<ny; i++ )
        fprintf ( stderr,"locbuf[%d*nz+level]==%f\n", i, locbuf[i*nz+level] );
#endif


    if ( rec_read ( ( char * ) locbuf, ssize, 12 ) <0 ) /* get level */
        {
        ERROR ( "Error on read data in UAM_Extract." );
        free ( locbuf );
        return ( FAILURE );
        }
    for ( i=0; i<nx; i++ )
        {
        buf[i]=locbuf[i*nz+level];
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr,
              "Just got S boundary data for spec %d, level %d, hour %d:\n",
              spec, level, hour );
    for ( i=0; i<nx; i++ )
        fprintf ( stderr,"locbuf[%d*nz+level]==%f\n", i, locbuf[i*nz+level] );
#endif


    if ( rec_read ( ( char * ) locbuf, nsize, 12 ) <0 ) /* get level */
        {
        ERROR ( "Error on read data in UAM_Extract." );
        free ( locbuf );
        return ( FAILURE );
        }
    for ( i=0; i<nx; i++ )
        {
        buf[ ( ny-1 ) *nx+i]=locbuf[i*nz+level];
        }


#ifdef DIAGNOSTICS
    fprintf ( stderr,
              "Just got N boundary data for spec %d, level %d, hour %d:\n",
              spec, level, hour );
    for ( i=0; i<nx; i++ )
        fprintf ( stderr,"locbuf[%d*nz+level]==%f\n", i, locbuf[i*nz+level] );
#endif

    free ( locbuf );

    return PAVE_SUCCESS;
    }
/* ========================================================================== */
void flip ( buffer, n )
char *buffer;
int n;
    {
    register int i;
    char b0, b1, b2, b3;

    for ( i = 0; i < n; i += 4 )
        {
        b0 = buffer[i];
        b1 = buffer[i+1];
        b2 = buffer[i+2];
        b3 = buffer[i+3];
        buffer[i] = b3;
        buffer[i+1] = b2;
        buffer[i+2] = b1;
        buffer[i+3] = b0;
        }
    }




/************************************************************
DUMP_HDRSTRUCT - used for diagnostic purposes;  added 961002 SRT
                 returns 1 if error
************************************************************/
int dump_hdrStruct ( UAM_INFO *u, char *fname /* optional arg, can use NULL */ )
    {
    int i;

    if ( !u ) return 1;

    fprintf ( stderr, "---------------------------------\n" );
    fprintf ( stderr, "Enter dump_hdrStruct(UAM_INFO *u)\n" );
    fprintf ( stderr, "---------------------------------\n" );
    if ( fname ) fprintf ( stderr, "file == '%s'\n", fname );
    fprintf ( stderr, "icol == %d\n", u->icol );
    fprintf ( stderr, "irow == %d\n", u->irow );
    fprintf ( stderr, "ilevel == %d\n", u->ilevel );
    fprintf ( stderr, "ispec == %d\n", u->ispec );
    fprintf ( stderr, "spec_list == '%s'\n", u->spec_list ? u->spec_list : "<null string>" );
    fprintf ( stderr, "sw_utmx == %g\n", u->sw_utmx );
    fprintf ( stderr, "sw_utmy == %g\n", u->sw_utmy );
    fprintf ( stderr, "ne_utmx == %g\n", u->ne_utmx );
    fprintf ( stderr, "ne_utmy == %g\n", u->ne_utmy );
    fprintf ( stderr, "utm_zone == %d\n", u->utm_zone );
    fprintf ( stderr, "hour1 == %d\n", u->hour1 );
    fprintf ( stderr, "hour2 == %d\n", u->hour2 );
    fprintf ( stderr, "begin_date == %d\n", u->begin_date );
    fprintf ( stderr, "end_date == %d\n", u->end_date );
    fprintf ( stderr, "nstep == %d\n", u->nstep );
    if ( u->nstep )
        if ( u->sdate )
            for ( i = 0; i < u->nstep; i++ )
                fprintf ( stderr, "sdate[%d] == %d\n", i, u->sdate[i] );
    if ( u->nstep )
        if ( u->stime )
            for ( i = 0; i < u->nstep; i++ )
                fprintf ( stderr, "stime[%d] == %d\n", i, u->stime[i] );
    fprintf ( stderr, "uam_type == %d\n", u->uam_type );
    fprintf ( stderr, "file_id == '%s'\n", u->file_id ? u->file_id : "<null string>" );
    fprintf ( stderr, "datapos == %d\n", u->datapos );
    fprintf ( stderr, "steplen == %d\n", u->steplen );
    fprintf ( stderr, "speclen == %d\n", u->speclen );
    fprintf ( stderr, "levellen == %d\n", u->levellen );

    return 0;
    }

