/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busTimeStamp.c 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  - busTimeStamp.c
 *
 *     busTimeStamp.c consists of timestamping functions that allow calculation
 *                    of Client service times (TIME_ST_BUS) and interarrival
 *                    times for socket and external arrivals (TIME_IAT)
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
 * REVISION HISTORY - busTimeStamp.c
 *
 * Date: 17-July-95
 * Version: 0.6
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/
#if defined(TIME_ST_BUS) || defined(TIME_IAT)

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include        <sys/resource.h>

FILE *time_st_fd;
struct rusage       ru_start;
double u_usec_start, s_usec_start;

void psdump()
    {

    }

#ifdef TIME_ST_BUS
void tsdump ( char *str )
    {
    struct rusage  ru_now, ru_timestamp;
    static double usec_timestamp=0;

    if ( time_st_fd )
        {
        if ( getrusage ( RUSAGE_SELF, &ru_now ) < 0 )
            {
            fprintf ( stderr, "t_start: getrusage() error" );
            }
        else
            {
            double u_usec,s_usec,u_usec_now,s_usec_now;

            u_usec_now = ( ( double ) ru_now.ru_utime.tv_sec ) * 1000000.0
                         + ru_now.ru_utime.tv_usec;
            u_usec = u_usec_now - u_usec_start - usec_timestamp;

            /*
                     s_usec_now = ((double) ru_now.ru_stime.tv_sec) * 1000000.0
                                            + ru_now.ru_stime.tv_usec;
                     s_usec = s_usec_now - s_usec_start;

                     if (fprintf(time_st_fd,"%e:%e:%d:%d:",u_usec, s_usec,ru_now.ru_inblock, ru_now.ru_oublock) < 0){
            */

            if ( fprintf ( time_st_fd,"%e:%ld:%ld:",u_usec, ru_now.ru_inblock, ru_now.ru_oublock ) < 0 )
                {
                printf ( "Error writing to Time Stamp file \n" );
                }
            else
                {
                double u_usec_ts;

                fflush ( time_st_fd );
                fputs ( str,time_st_fd );
                fflush ( time_st_fd );

                /* Find time taken to timestamp and deduct it next time */
                getrusage ( RUSAGE_SELF, &ru_timestamp );
                u_usec_ts = ( ( double ) ru_timestamp.ru_utime.tv_sec ) * 1000000.0
                            + ru_timestamp.ru_utime.tv_usec;
                usec_timestamp = u_usec_ts - u_usec_now;
                /*
                fprintf(time_st_fd,"Usec_timestamp = %e \n", usec_timestamp);
                fflush(time_st_fd);
                */
                }
            }
        }
    }

#else

void tsdump ( char *str )
    {
    struct timeval time_now;
    struct rusage  ru_now, ru_timestamp;
    static double usec_timestamp=0;
    hrtime_t start_time;
    char *s = NULL;

    if ( time_st_fd )
        {
        /*
              gettimeofday(&time_now, &s);
              if (fprintf(time_st_fd,"%ld:%ld:",time_now.tv_sec,time_now.tv_usec) < 0){
                 printf("Error writing to Time Stamp file \n");
              }
        */
        start_time = gethrtime();
        if ( fprintf ( time_st_fd,"%lld:",start_time ) < 0 )
            {
            printf ( "Error writing to Time Stamp file \n" );
            }
        else
            {
            fflush ( time_st_fd );
            fputs ( str,time_st_fd );
            fflush ( time_st_fd );
            }
        }
    }

#endif

void BusTSInitFile ( char *moduleName, int moduleId )
    {
    char *ts_path, ts_fname[1024];


    ts_path = getenv ( "SBUS_TS_PATH" );
    if ( ts_path != NULL )
        sprintf ( ts_fname,"%s/%s_%d",ts_path,moduleName,moduleId );
    else
        sprintf ( ts_fname,"/tmp/%s_%d",moduleName,moduleId );

    printf ( "Filename for time stamping = %s \n", ts_fname );
    fflush ( stdout );
    if ( ! ( time_st_fd = fopen ( ts_fname, "w+" ) ) )
        {
        char s[2048];
        sprintf ( s,"Could not open %s", ts_fname );
        perror ( s );
        }
    if ( getrusage ( RUSAGE_SELF, &ru_start ) < 0 )
        {
        char s[2048];
        sprintf ( s,"Error in getrusage \n" );
        perror ( s );
        }
    u_usec_start = ( ( double ) ru_start.ru_utime.tv_sec ) * 1000000.0
                   + ru_start.ru_utime.tv_usec;
    s_usec_start = ( ( double ) ru_start.ru_stime.tv_sec ) * 1000000.0
                   + ru_start.ru_stime.tv_usec;
    }

void BusTSCloseFile()
    {
    if ( time_st_fd != NULL )
        fclose ( time_st_fd );
    }

#endif
