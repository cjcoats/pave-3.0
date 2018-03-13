/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: dates.c 83 2018-03-12 19:24:33Z coats $
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
 *      Author:  Kathy Pearson, MCNC, kathyp@mcnc.org
 *      Modified:  06/25/98 by  A. Trayanov
 ****************************************************************************/

#include <stdio.h>
#include <time.h>

#define nint(x) ((x)>0)?(int)((x)+0.5):(int)((x)-0.5)

/* ========================================================================== */


void net2julian ( int sdate, int stime, int tstep_sec, int record,
                  int *jdate, int *jtime )
    {

#ifdef USE_UNIX_MKTIME
    struct tm tm;
    time_t t;
    char date[10];
    char *timestr;
    int year;


    tm.tm_year = sdate/1000 - 1900;
    tm.tm_mday = sdate%1000;
    tm.tm_mon  = 0;
    tm.tm_hour = stime/10000;
    tm.tm_min  = ( stime%10000 ) /100;
    tm.tm_sec  = ( stime%100 ) + ( record-1 ) *tstep_sec;

    tm.tm_isdst = -1; /*determine day-light savings time, don't convert */
    /* In UAM the year format is YY implying that the years are counted
     * from 1900, but mktime() returns the
     * number of seconds since Jan 1 1970. Therefore, if we follow
     * the 2 digit year format, a number less than 70 is meaningless
     * and could be used for the years 2000-2069.
     * Actually, this will work only for calendar times between
     * 00:00:00 UTC, January 1, 1970 to 03:14:07 UTC, January 19, 2038.
     */

    t=mktime ( &tm );
    if ( t<0 )
        {
        fprintf ( stderr, "%s\n",
                  "time must be in the range 00:00:00 1/1/1970 to 03:14:07 1/19/2038"
                );
        return;
        }
    timestr = asctime ( localtime ( &t ) );

#ifdef DEBUG
    printf ( "%s", timestr );
#endif
    sscanf ( timestr+20,"%d",&year );

    *jdate = year*1000+tm.tm_yday+1;
    *jtime = 10000* ( tm.tm_hour )+100* ( tm.tm_min )+ ( tm.tm_sec );

    return;
#else
    void nextimec();
    int sec2timec();

    *jdate = sdate;
    *jtime = stime;

    nextimec ( jdate, jtime, sec2timec ( ( record-1 ) *tstep_sec ) );
#endif
    }




