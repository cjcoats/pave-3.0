/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busMasterTime.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  - busMasterTime.c
 *
 *     busMasterTime.c consists of functions for measuring busMaster service
 *                      time using various system functions :
 *                  gethrtime - high resolution real time
 *                  getrusage - for user and system time used
 *                  times     -     -ditto-
 *
 *     INPUT FILES:       stdin stream
 *     OUTPUT FILES:      stdout stream
 *     ERROR HANDLING:    output to stderr (screen)
 *     INPUT PARAMETERS:  client name (ascii string), optional
 *     INPUT OPTIONS:     if client name is not input, default is
 *
 *     NETWORKING:        remote/network connectivity is via bus library (sockets)
 *
 *     COMPILATION:       (see makefile)
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *
 ********************************************************************
 * REVISION HISTORY - busMasterTime.c
 *
 * Date: 15-Aug-96
 * Version: 0.7
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/
 
#if defined(TIME_BM_RU) || defined(TIME_BM_HR) || defined(TIME_BM_TIMES)

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>

#include <sys/times.h>
#include <sys/param.h>


#ifdef TIME_BM_RU
static  struct timeval          time_start, time_stop; /* for real time */
static  struct rusage           ru_start, ru_stop, ru_overhead;  /* for user & sys time */
static  double                  start, stop, useconds, overhead;
#endif

#ifdef TIME_BM_HR
static hrtime_t start_time, stop_time, read_time, end_time, service_time;
#endif

#ifdef TIME_BM_TIMES
struct tms ru_start, ru_end, ru_read, ru_serv;
long read_time, serv_time;
#endif

/***** Functions for using getrusage **********/
#ifdef TIME_BM_RU
void t_start()
    {
    if ( getrusage ( RUSAGE_SELF, &ru_start ) < 0 )
        fprintf ( stderr, "t_start: getrusage() error" );
    }


void t_stop()
    {
    if ( getrusage ( RUSAGE_SELF, &ru_stop ) < 0 )
        fprintf ( stderr, "t_stop: getrusage() error" );
    if ( getrusage ( RUSAGE_SELF, &ru_overhead ) < 0 )
        fprintf ( stderr, "t_stop: getrusage() error" );

    }

double t_getrtime()
    {
    }

double t_getutime()
    {

    start = ( ( double ) ru_start.ru_utime.tv_sec ) * 1000000.0
            + ru_start.ru_utime.tv_usec;
    stop = ( ( double ) ru_stop.ru_utime.tv_sec ) * 1000000.0
           + ru_stop.ru_utime.tv_usec;
    overhead = ( ( double ) ru_overhead.ru_utime.tv_sec ) * 1000000.0
               + ru_overhead.ru_utime.tv_usec;
    useconds = ( stop-start );
    return useconds;
    }

double t_getstime()
    {
    }
#endif

/***** Functions for using gethrtime **********/
#ifdef TIME_BM_HR
void t_start()
    {
    start_time = gethrtime();
    }

void t_stop()
    {
    end_time = gethrtime();
    }

double t_getrtime()
    {
    }

double t_getutime()
    {
    double utime;

    utime = ( double ) ( end_time - start_time );
    return utime;
    }

double t_getstime()
    {
    }
#endif

/***** Functions for using times **********/
#ifdef TIME_BM_TIMES
void t_start()
    {
    times ( &ru_start );
    }

void t_stop()
    {
    times ( &ru_end );
    }

double t_getrtime()
    {
    }

double t_getutime()
    {

    double utime;
    utime = ( double ) ( ( ( ( ru_end.tms_utime - ru_start.tms_utime )+
                             ( ru_end.tms_stime - ru_start.tms_stime ) ) * 1000000 ) /2147 );
    return utime;
    }

double t_getstime()
    {
    }
#endif

#endif
