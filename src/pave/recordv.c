/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: recordv.c 83 2018-03-12 19:24:33Z coats $
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
 *      Author:  Atanas Trayanov, NCSC, 1994?
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ****************************************************************************/

/* bald messes this up in some header file */
#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "uamv.h"
#include "vis_data.h"

/* ... definitions ... */

#define MAX_BUF_SIZE    2048
#define MAXHEADER       1024
#define MAXLINE         255
#define ERROR(x) fprintf(stderr, "%s\n",x)
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define GET_DATA(a,b)   (a+b*WORD_SIZE)

#define uamv_close  uam_close

/* ... function prototypes ... */
int read_item ( FILE *, char *, void * );

/********************************************************************/
/* UAM Header Routine                                               */
/********************************************************************/

int uamv_fetch_header ( char *metafile, UAMV_INFO *hdr_info )
    {
    int i, j, k;

#ifdef UNICOS
    int unicos_k;
#endif

    static char str[MAXHEADER];
    struct stat statbuf;
    int uamv_filesize;
    time_t t1, t2, hoursSince1970();
    int hour1, hour2, irow, icol, ispec, ilevel;
    int datapos, levellen, speclen, steplen, domainlen;
    int nptsr, uamv_type, begin_date, end_date, utm_zone;
    float sw_utmx, sw_utmy, ne_utmx, ne_utmy;

    char filename[255];
    char filetype[MAXLINE];
    float xorg, yorg, dx, dy;

    FILE *mtf;
    static struct file_type
        {
        char *name;
        int type;
        } f_type[]=
        {
        "Wind",             UAMV_WIND,
        "Temp",             UAMV_TEMP,
        "Cloud",            UAMV_CLOUD,
        "H2O",              UAMV_H2O,
        "Rain",             UAMV_RAIN,
        "Vdif",             UAMV_VDIF,
        "Height",           UAMV_HEIGHT,
        "FineGridAverage",  UAMV_FAVER,
        "FineGridInstant",  UAMV_FINST
        };

    int finegrid;
    int latlon;
    char dummy[MAXLINE];
    char *meta_hdr="#! UAMV DESCRIPTION FILE";
    char first_line[MAXLINE];
    extern FILE *fp;
    extern int convert;
    int cw, first_rec_len;
    void flip();

    char timebuf[8];
    int nstep;
    int *sdate, *stime, *ibuf;
    float *fbuf;
    float hour;
    char errmsg[512];

    sdate=hdr_info->sdate;
    stime=hdr_info->stime;

    mtf = fopen ( metafile,"r" );
    if ( mtf==NULL )
        {
        ERROR ( "UAM-V error: Cannot open METAFILE" );
        return ( FAILURE );
        }
    /* check header ID string */
    i=strlen ( meta_hdr );
    fgets ( first_line,i+1, mtf );
    if ( strncmp ( first_line, meta_hdr,i ) )
        {
        fclose ( mtf );
#ifdef DEBUG
        ERROR ( "UAM-V metafile error: Cannot recognize first line. Not a METAFILE" );
#endif
        return FAILURE;
        }
    /* Read the metafile */
    i=1;
    i &= j = read_item ( mtf,"UAMV_FILE %s",filename );
    if ( !j ) ERROR ( "UAMV_FILE keyword is missing" );
    i &= j = read_item ( mtf,"UAMV_TYPE %s",filetype );
    if ( !j ) ERROR ( "UAMV_TYPE keyword is missing" );
    i &= j = read_item ( mtf,"NCOLS %d",&icol );
    if ( !j ) ERROR ( "NCOLS keyword is missing" );
    i &= j = read_item ( mtf,"NROWS %d",&irow );
    if ( !j ) ERROR ( "NROWS keyword is missing" );
    i &= j = read_item ( mtf,"LEVELS %d",&ilevel );
    if ( !j ) ERROR ( "LEVELS keyword is missing" );
    i &= j = read_item ( mtf,"XORG %f",&xorg );
    if ( !j ) ERROR ( "XORG keyword is missing" );
    i &= j = read_item ( mtf,"YORG %f",&yorg );
    if ( !j ) ERROR ( "YORG keyword is missing" );
    if ( ( yorg>=0.0 ) && ( yorg<90.0 ) )
        {
        latlon = 1;
        i &= j = read_item ( mtf,"DLON %f",&dx );
        if ( !j ) ERROR ( "DLON keyword is missing" );
        i &= j = read_item ( mtf,"DLAT %f",&dy );
        if ( !j ) ERROR ( "DLAT keyword is missing" );
        utm_zone=16; /* just an arbitrary value, in case we need to pass it */
        }
    else
        {
        latlon = 0;
        i &= j = read_item ( mtf,"DX %f",&dx );
        if ( !j ) ERROR ( "DX keyword is missing" );
        i &= j = read_item ( mtf,"DY %f",&dy );
        if ( !j ) ERROR ( "DY keyword is missing" );
        i &= j = read_item ( mtf,"UTM_ZONE %d",&utm_zone );
        if ( !j ) ERROR ( "UTM_ZONE keyword is missing" );
        }
    i &= j = read_item ( mtf,"FINE_GRID %d",&finegrid );
    if ( !j ) ERROR ( "FINE_GRID keyword is missing" );
    i &= j = read_item ( mtf,"TITLE %[ -z]",str );
    if ( !j ) ERROR ( "TITLE keyword is missing" );
    fclose ( mtf );
    if ( !i )
        {
        ERROR ( "One or more required KEYWORDs is missing, see above, or or the KEYWORDs are not in the correct order." );
        return FAILURE;
        }

    uamv_type=UNKNOWN;
    k=sizeof ( f_type ) / ( sizeof ( f_type[0] ) );
    for ( i=0; i<k; i++ )
        {
        if ( strncmp ( filetype,f_type[i].name,strlen ( f_type[i].name ) ) == 0 )
            {
            uamv_type = f_type[i].type;
            break;
            }
        }
    if ( uamv_type==UNKNOWN )
        {
        ERROR ( "Unknown UAM-V datatype, check the keyword UAMV_TYPE" );
        return FAILURE;
        }

    if ( uamv_type==UAMV_RAIN ) ilevel=1;

    if ( ( fp = fopen ( filename,"r" ) ) == NULL )
        {
        sprintf ( errmsg,
                  "The input file %s, specified in the METAFILE %s cannot be open to READ",
                  filename, metafile );
        ERROR ( errmsg );
        return ( FAILURE );
        }
    /* get file size */
    if ( stat ( filename, &statbuf ) == IOERROR )
        {
        ERROR ( "Cannot get input file size in UAMV_Extract." );
        return ( FAILURE );
        }
    if ( ( uamv_filesize=statbuf.st_size ) <= 0 )
        {
        ERROR ( "Size of input file is zero in UAMV_Extract." );
        return ( FAILURE );
        }
    hdr_info->uamv_type = uamv_type;
    hdr_info->fine_grid = finegrid;

    /* the following modification is needed to help PAVE read
       ALL files */
    if ( fread ( &cw,sizeof ( cw ), 1, fp ) != 1 )
        {
        ERROR ( "Cannot read the first record control word." );
        return ( FAILURE );
        }
    rewind ( fp ); /* set the file pointer to the beginning of the file */
    convert=0; /* this is the default: read the file as is */

    if ( uamv_type==UAMV_FAVER || uamv_type==UAMV_FINST )
        {
        if ( cw!=FAVG_MSG )
            {
            convert = 1;
            /* if further checks are needed we can do this: */
            flip ( &cw, sizeof ( cw ) );
            if ( cw != FAVG_MSG )
                {
                ERROR ( "Inappropriate first record control word." );
                return ( FAILURE );
                }
            }
        if ( !get_faver_header ( hdr_info ) )
            {
            ERROR ( "ERROR: failed to recongnize the header as fine grid average file" );
            return FAILURE;
            }
        /*
              sw_utmx=xorg+(hdr_info->ixfb-1-1.0/hdr_info->nhf)*dx;
              sw_utmy=yorg+(hdr_info->iyfb-1-1.0/hdr_info->nvf)*dy;
              ne_utmx=xorg+(hdr_info->ixfe+1.0/hdr_info->nhf)*dx;
              ne_utmy=yorg+(hdr_info->iyfe+1.0/hdr_info->nvf)*dx;
        */
        sw_utmx=xorg;
        sw_utmy=yorg;
        ne_utmx= ( xorg+icol*dx );
        ne_utmy= ( yorg+irow*dy );
        ispec=hdr_info->ispec;
        }
    else
        {
        sw_utmx=xorg;
        sw_utmy=yorg;
        ne_utmx= ( xorg+icol*dx );
        ne_utmy= ( yorg+irow*dy );

        if ( finegrid && uamv_type != UAMV_WIND )
            {
            sw_utmx -= dx;
            sw_utmy -= dy;
            ne_utmx += dx;
            ne_utmy += dy;
            icol += 2;
            irow += 2;
            }

        /* fill-in uamv struct */

        hdr_info->icol=icol;
        hdr_info->irow=irow;
        hdr_info->ilevel=ilevel;
        }
    /*
       for now we will ignore the TITLE string from the FineGridAverage or
       FineGridInstant file and we will use the TITLE from the metafile.
       If this is not desired just move the next line inside the else-block above.
    */
    hdr_info->file_id=strdup ( str );

    if ( !latlon )
        {
        sw_utmx *= 1000.0;
        sw_utmy *= 1000.0;
        ne_utmx *= 1000.0;
        ne_utmy *= 1000.0;
        }

    hdr_info->sw_utmx = sw_utmx;
    hdr_info->sw_utmy = sw_utmy;
    hdr_info->ne_utmx = ne_utmx;
    hdr_info->ne_utmy = ne_utmy;

    hdr_info->utm_zone=utm_zone;
    hdr_info->datapos=0;

    switch ( uamv_type )
        {
        case UAMV_TEMP:
            first_rec_len=UAMV_REC_OFFSET + icol * irow;
            levellen = first_rec_len + REC_CW;
            steplen = ( ilevel+1 ) * levellen;
            hdr_info->levellen=levellen;
            hdr_info->steplen = steplen;
            ispec=1;
            break;
        case UAMV_HEIGHT:
        case UAMV_CLOUD:
            first_rec_len=UAMV_REC_OFFSET + icol * irow;
            speclen = first_rec_len + REC_CW;
            levellen = 2 * speclen; /* HNEW+PRESS or CLOUD+CWATER*/
            steplen = ilevel * levellen;
            hdr_info->levellen=levellen;
            hdr_info->steplen = steplen;
            hdr_info->speclen = speclen;
            ispec=2;
            break;
        case UAMV_H2O:
        case UAMV_RAIN:
        case UAMV_VDIF:
            first_rec_len=UAMV_REC_OFFSET + icol * irow;
            levellen = first_rec_len + REC_CW;
            steplen = ilevel * levellen;
            hdr_info->levellen=levellen;
            hdr_info->steplen = steplen;
            ispec=1;
            break;
        case UAMV_WIND:
            ispec=2;
            first_rec_len=2;
            hdr_info->ispec=ispec;
            speclen = icol * irow + REC_CW;
            levellen = 2*speclen;
            steplen = UAMV_REC_OFFSET + REC_CW +
                      ilevel * levellen +
                      speclen /* wsurf */;
            hdr_info->speclen = speclen;
            hdr_info->levellen=levellen;
            hdr_info->steplen = steplen;
            break;
        case UAMV_FAVER:
        case UAMV_FINST:
            datapos=ftell ( fp );
            hdr_info->datapos = datapos;

            if ( uamv_type==UAMV_FAVER ) ilevel=1;
            else ilevel=hdr_info->ilevel;
            hdr_info->ilevel=ilevel;
            icol = hdr_info->nfx;
            irow = hdr_info->nfy;
            hdr_info->icol=icol;
            hdr_info->irow=irow;

            levellen = REC_CW + icol*irow;
            speclen = ilevel*levellen;
            domainlen = ispec * speclen;
            steplen = UAMV_TIME_STEP_HEADER + REC_CW +
                      ispec * ilevel * ( hdr_info->gridlen );
            hdr_info->steplen = steplen;
            hdr_info->domainlen = domainlen;
            hdr_info->levellen = levellen;
            hdr_info->speclen = speclen;
            hdr_info->grid_offset *= ispec*ilevel;
            break;
        }
    hdr_info->ispec=ispec;

    if ( uamv_type!=UAMV_FAVER && uamv_type!=UAMV_FINST )
        {
        if ( cw != first_rec_len*sizeof ( cw ) )
            {
            convert = 1;
            /* if further checks are needed we can do this: */
            flip ( &cw, sizeof ( cw ) );
            if ( cw != first_rec_len*sizeof ( cw ) )
                {
                ERROR ( "Inappropriate first record control word." );
                fprintf ( stderr,"STEPLEN=%d, CW=%d\n",steplen,cw );
                return ( FAILURE );
                }
            }
        }

    datapos = hdr_info->datapos;
    nstep= ( uamv_filesize - datapos ) / ( steplen*WORD_SIZE );

    if ( ( sdate == NULL ) || ( stime==NULL ) )
        {
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

        k=datapos;
        /* Scanning the file to fill-in the time info for every timestep */
        for ( i=0; i<nstep; i++ )
            {
            if ( fseek ( fp, k, SEEK_SET ) != 0 )
                {
                ERROR ( "File seek error" );
                return ( IOERROR );
                }
            if ( rec_read ( ( char * ) timebuf, 2, 0 ) <0 ) /* get time info */
                {
                ERROR ( "Error on read timeinfo in UAMV_Extract." );
                return ( FAILURE );
                }
            fbuf = ( float * ) GET_DATA ( timebuf,0 );
            /* convert the military time to hours */
            hour = ( *fbuf ) /100;
            if ( uamv_type==UAMV_FAVER ) hour -= 1.0;
            ibuf = ( int   * ) GET_DATA ( timebuf,1 );
            sdate[i] = *ibuf;
            stime[i] = UAMV_DATA; /* just to be passed to the next function, it will be reset inside */
            hoursSince1970 ( &sdate[i], hour, &stime[i] );
            k+=steplen*WORD_SIZE;
            }

        hour1 = stime[0]/10000;
        hour2 = stime[nstep-1]/10000;
        begin_date = sdate[0];
        end_date = sdate[nstep-1];

        }
    hdr_info->begin_date=begin_date;
    hdr_info->end_date=end_date;
    hdr_info->hour1=hour1;
    hdr_info->hour2=hour2;
    hdr_info->nstep=nstep;
    hdr_info->sdate=sdate;
    hdr_info->stime=stime;

    return ( PAVE_SUCCESS );
    }
/* ========================================================================== */
/* Addition to read Fine Grid Average files */
int get_faver_header ( UAMV_INFO *hdr_info )
    {
    int i, j, k;
    int numfin, ixfb, iyfb, ixfe, iyfe, nhf, nvf, nfx, nfy, nfz,
        ifgptr, ifglvl;
    int ispec;
    char *buf;
    int *ibuf;
    int nx, ny, size, grid_offset, gridlen;
    int qq;
    char *s_str, c;
    static char str[MAXHEADER];

    if ( ( buf=malloc ( MAX_BUF_SIZE*sizeof ( buf[0] ) ) ) == NULL )
        {
        ERROR ( "Buffer allocation error in UAMV_Extract" );
        return ( FAILURE );
        }
    if ( rec_byte_read ( buf, FAVG_MSG ) <0 )
        {
        ERROR ( "Cannot read fine grid file ID string." );
        return ( FAILURE );
        }
    strncpy ( str, buf, FAVG_MSG );
    for ( i = FAVG_MSG; i >= 0; --i ) /* strip trailing blanks */
        {
        if ( str[i] > ' ' ) break;
        str[i] = '\0';
        }
    hdr_info->file_id=strdup ( str );
    if ( rec_read ( buf, FAVG_NFIN_NSPC, NO_OFFSET ) <0 )
        {
        ERROR ( "Cannot read fine grid file NUMFIN record" );
        return ( FAILURE );
        }
    ibuf = ( int * ) GET_DATA ( buf, FAVG_NFIN );
    numfin = *ibuf;
    ibuf = ( int * ) GET_DATA ( buf, FAVG_NSPC );
    ispec = *ibuf;

    if ( hdr_info->fine_grid > numfin )
        {
        ERROR ( "File contains fewer fine grids than requested" );
        return FAILURE;
        }

    /* get species name record */
    qq=1;
    if ( hdr_info->uamv_type==UAMV_FINST ) qq=sizeof ( int );
    if ( rec_byte_read ( buf, ispec*qq*SPC_SPEC ) <0 )
        {
        ERROR ( "Cannot read header in UAMV_Extract." );
        return ( FAILURE );
        }
    if ( ( s_str=malloc ( SPC_SPEC*ispec ) ) ==NULL ) return ( FAILURE );
    for ( i=k=0; i<ispec; i++ )
        {
        for ( j=0; j<qq*SPC_SPEC; j+=qq )
            {
            c=buf[qq*SPC_SPEC*i+j];
            if ( c==' ' ) break;
            s_str[k++]=c; /* Read until the first blank */
            }
        /* Use ":" as separator between species names */
        s_str[k++]=':';
        }
    /*s_str[--k]='\0';*/
    s_str[k]='\0';
    hdr_info->spec_list = strdup ( s_str );
    /* NUMFIN LOOP */
    gridlen = 0;
    for ( i=1; i<=numfin; i++ )
        {
        if ( rec_read ( buf, FAVG_GRID_INFO, NO_OFFSET ) <0 )
            {
            ERROR ( "Cannot read fine grid file GRID_INFO record" );
            return ( FAILURE );
            }
        ibuf = ( int * ) GET_DATA ( buf, FAVG_IXFB );
        nx  = ibuf[6];
        ny  = ibuf[7];
        size = nx*ny + REC_CW;
        if ( i==hdr_info->fine_grid )
            {
            ibuf = ( int * ) GET_DATA ( buf, FAVG_IXFB );
            ixfb = ibuf[0];
            iyfb = ibuf[1];
            ixfe = ibuf[2];
            iyfe = ibuf[3];
            nhf  = ibuf[4];
            nvf  = ibuf[5];
            nfx  = ibuf[6];
            nfy  = ibuf[7];
            nfz  = ibuf[8];
            ifgptr=ibuf[9];
            ifglvl=ibuf[10];
            grid_offset=gridlen;
            }
        gridlen += size;
        }
    hdr_info->ispec=ispec;
    hdr_info->ilevel=nfz;
    hdr_info->numfin=numfin;
    hdr_info->nfx=nfx;
    hdr_info->nfy=nfy;
    hdr_info->ixfb=ixfb;
    hdr_info->iyfb=iyfb;
    hdr_info->ixfe=ixfe;
    hdr_info->iyfe=iyfe;
    hdr_info->nhf=nhf;
    hdr_info->nvf=nvf;
    hdr_info->gridlen = gridlen;
    hdr_info->grid_offset = grid_offset;
    free ( buf );
    free ( s_str );
    return PAVE_SUCCESS;
    }
/* ========================================================================== */
int uamv_fetch_data ( hdr_info, buf, n, spec, level, hour )
UAMV_INFO *hdr_info;
float *buf;
int n, spec, level, hour;
    {
    int k;
    int offset;
    extern FILE *fp;

#ifdef UNICOS
    int unicos_k;
#endif

    offset=UAMV_REC_OFFSET;
    if ( hdr_info->uamv_type == UAMV_WIND ) offset=0;
    if ( spec >= hdr_info->ispec )
        {
        ERROR ( "UAM-V internal file error: spec too big" );
        return ( FAILURE );
        }
    if ( level >= hdr_info->ilevel )
        {
        ERROR ( "UAM-V internal file error: level too big" );
        return ( FAILURE );
        }

    if ( ( n + offset ) > hdr_info->levellen )
        {
        ERROR ( "UAM-V internal file error: too much data requested" );
        return ( FAILURE );
        }

    switch ( hdr_info->uamv_type )
        {
        case UAMV_WIND:
            /* ignoring WSURF */
            k = hour * ( hdr_info->steplen )
                + level * ( hdr_info->levellen )
                + spec * ( hdr_info->speclen )
                + UAMV_REC_OFFSET + REC_CW;
            break;
        case UAMV_TEMP:
            /*
             * for now, ignore TSURF record by effectively
             * going one level higher
             */
            k = hour * ( hdr_info->steplen )
                + ( level+1 ) * ( hdr_info->levellen );
            break;
        case UAMV_HEIGHT:
        case UAMV_CLOUD:
        case UAMV_H2O:
        case UAMV_RAIN:
        case UAMV_VDIF:
            k = hour * ( hdr_info->steplen )
                + spec * ( hdr_info->speclen )
                + level * ( hdr_info->levellen );
            break;
        case UAMV_FAVER:
        case UAMV_FINST:
            offset=0;
            k = hdr_info->datapos + WORD_SIZE* (
                    UAMV_TIME_STEP_HEADER + REC_CW
                    + hour * ( hdr_info->steplen )
                    + spec * ( hdr_info->speclen )
                    + level * ( hdr_info->levellen )
                    + hdr_info->grid_offset );
            /* the level parameter here selects the nested grid */
            break;
        default:
            ERROR ( "UAM-V internal file error: inappropriate uam-v type" );
            return ( FAILURE );
        }


    if ( hdr_info->uamv_type != UAMV_FAVER &&
            hdr_info->uamv_type != UAMV_FINST )
        {
#ifdef UNICOS
        unicos_k = k + ( k/511 );
        if ( fseek ( fp, unicos_k*WORD_SIZE, SEEK_SET ) != 0 )
#else
        if ( fseek ( fp, k*WORD_SIZE, SEEK_SET ) != 0 )
#endif
            {
            ERROR ( "File seek error" );
            return ( IOERROR );
            }
        }
    else
        {
        if ( fseek ( fp, k, SEEK_SET ) != 0 )
            {
            ERROR ( "File seek error" );
            return ( IOERROR );
            }
        }

    if ( rec_read ( ( char * ) buf, n, offset ) <0 ) /* get level */
        {
        ERROR ( "Error on read data in UAMV_Extract." );
        return ( FAILURE );
        }
    return ( PAVE_SUCCESS );
    }
/* ========================================================================== */
int read_item ( fp,format,pointer_to_variable )
FILE *fp;
char * format;
void *pointer_to_variable;
    {
    char buffer[MAXLINE];

    do
        {
        if ( fgets ( buffer,MAXLINE,fp ) == NULL )
            {
#ifdef DIAGNOSTICS  /* added 951205 SRT */
            sprintf ( buffer,"No match found for %s", format );
            ERROR ( buffer );
#endif /* DIAGNOSTICS   added 951205 SRT */
            return FAILURE;
            }
        }
    while ( sscanf ( buffer,format,pointer_to_variable ) != 1 );
    return PAVE_SUCCESS;
    }

/* ========================================================================== */

#define nint(x) ((x)>0)?(int)((x)+0.5):(int)((x)-0.5)

time_t hoursSince1970 ( sdate, hour, stime )
int *sdate;
float hour;
int *stime;
    {
    struct tm tm;
    time_t t;
    char date[10];
    char date6[7];
    char first_digit[2];
    char *timestr;
    int year;
    int julian;
    int ndigits;
    int i;


    sprintf ( date,"%d", ( *sdate ) );
    ndigits=strlen ( date );
    switch ( ndigits )
        {
        case 1: /* YDDD -> year 2000 is represented by no digits */
        case 2: /* YDDD -> year 2000 is represented by no digits */
        case 3: /* YDDD -> year 2000 is represented by no digits */
        case 4: /* YDDD -> years 2000-2009 are represented by a single digit */
        case 5: /* Ambigious: (YYDDD or YMMDD).
         We will do a little heuristic to figure it out.
         If you have a five digit date or leess
         and aren't sure if it's YYMMDD or YYDDD
         then:

         If the last three digits are > 365 it is YYMMDD.
         This will cover most cases for UAM.
         It's an ozone model and is therefore unlikely to be run
         in the January - March time frame.

         If the last three digits are < 365
         and the first digit is 7 or less then
         it is YYMMDD - because the UAM was not run
         for years in the 1970s. */
            julian = *sdate%1000;
            if ( julian > 366 ) goto mmdd;
            else
                {
                if ( ndigits == 5 )
                    {
                    sprintf ( first_digit,"%c",date[0] );
                    if ( atoi ( first_digit ) <= 7 ) goto mmdd;
                    }
                }
            /* assume YYJJJ */
            tm.tm_year = *sdate/1000;
            tm.tm_mday = *sdate%1000;
            tm.tm_mon=0;
            break;
        case 6: /* calendar format YYMMDD */
mmdd:
            for ( i=0; i<6; i++ ) date6[i]='0';
            date6[6]='\0';
            for ( i=0; i<ndigits; i++ ) date6[5-i]=date[ndigits-1-i];
            sscanf ( date6,"%02d%02d%02d",
                     &tm.tm_year,&tm.tm_mon,&tm.tm_mday );
            tm.tm_mon--; /* months start from 0 */
            break;
        default:
            ERROR ( "Not a valid date format: must be YYMMDD or YYDDD" );
            return ( -1 );
        }

    tm.tm_hour = ( int ) ( hour );
    tm.tm_min  = ( int ) ( ( hour-tm.tm_hour ) *60 );
    tm.tm_sec  = nint ( ( hour-tm.tm_hour-tm.tm_min/60.0 ) *3600 );

    tm.tm_isdst = -1; /*determine day-light savings time, don't convert */
    /* In UAM the year format is YY implying that the years are counted
     * from 1900, but mktime() returns the
     * number of seconds since Jan 1 1970. Therefore, if we follow
     * the 2 digit year format, a number less than 70 is meaningless
     * and could be used for the years 2000-2069.
     * Actually, this will work only for calendar times between
     * 00:00:00 UTC, January 1, 1970 to 03:14:07 UTC, January 19, 2038.
     */

    if ( tm.tm_year<70 ) tm.tm_year += 100;
    t=mktime ( &tm );
    if ( t<0 )
        {
        ERROR ( "time must be in the range 00:00:00 1/1/1970 to 03:14:07 1/19/2038" );
        return ( -1 );
        }
    timestr = asctime ( localtime ( &t ) );

#ifdef DEBUG
    printf ( "%s", timestr );
#endif
    sscanf ( timestr+20,"%d",&year );

    *sdate = year*1000+tm.tm_yday+1;
    *stime = 10000* ( tm.tm_hour )+100* ( tm.tm_min )+ ( tm.tm_sec );

    t /= 3600; /* convert the time to hours */
    return t;
    }
/* ========================================================================== */

int rec_byte_read ( buf, n )
char *buf;
int n;
    {
    int cw, eor;
    extern FILE *fp;
    extern int convert;
    void flip();

    if ( fread ( &cw, sizeof ( cw ), 1, fp ) != 1 )
        {
        ERROR ( "Cannot read FORTRAN record control word" );
        return ( IOERROR );
        }
    if ( convert ) flip ( &cw,sizeof ( cw ) );
    if ( cw < n )
        {
        ERROR ( "Record does not contain this much data" );
        fprintf ( stderr, "cw=%d n=%d\n",cw,n );
        return ( IOERROR );
        }
    if ( fread ( buf, sizeof ( buf[0] ), n, fp ) != n )
        {
        ERROR ( "Record read failed" );
        return ( IOERROR );
        }
    if ( cw > n ) if ( fseek ( fp, ( cw-n ), SEEK_CUR ) != 0 )
            {
            ERROR ( "File seek error" );
            return ( IOERROR );
            }
    if ( fread ( &eor, sizeof ( eor ), 1, fp ) != 1 )
        {
        ERROR ( "Cannot read FORTRAN end-of-record word" );
        return ( IOERROR );
        }
    if ( convert ) flip ( &cw,sizeof ( cw ) );
    if ( cw != eor )
        {
        ERROR ( "Record headers mismatch" );
        return ( IOERROR );
        }
    return ( PAVE_SUCCESS );
    }
/* ========================================================================== */















