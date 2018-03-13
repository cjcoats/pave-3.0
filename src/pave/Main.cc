/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Main.cc 83 2018-03-12 19:24:33Z coats $
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
 ****************************************************************************/

#define PAVE_SHORT_VERSION "PAVE v3.0"
#define PAVE_LONG_VERSION  PAVE_SHORT_VERSION" $Id: Main.cc 83 2018-03-12 19:24:33Z coats $"
#define DATE_OF_COMPILATION "$Date: 2018-03-12 15:24:33 -0400 (Mon, 12 Mar 2018) $"


///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// File:    Main.cc
// Author:  K. Eng Pua
// Date:    Dec 19, 1994
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

// unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <stdio.h>
#include "AppInit.h"
#include "DriverWnd.h"
#include <pwd.h>
#include <sys/utsname.h>
// SRT #include <unistd.h>
extern "C" {
    uid_t getuid ( void );    // SRT
    }
#include <stdlib.h>
#include <X11/Intrinsic.h>      // added 950913 SRT
#include <X11/StringDefs.h>     // added 950913 SRT
#include <Xm/Xm.h>              // added 950913 SRT
#include <Xm/Text.h>            // added 950913 SRT
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef linux
#include <netdb.h>
#endif

typedef struct XINPUT_DATA
    {
    FILE *stream;   /* identifies the stream */
    } XINPUT_DATA;



static DriverWnd *window;
static char logFile[256];
static time_t startSeconds;


static char *get_time_string ( void )
    {
    static char time_string[128];
    struct timeval tp;
    struct timezone tzp;
    struct tm *local;

    gettimeofday ( &tp, &tzp );
    local = localtime (
#ifdef __osf__
                ( const int * )
#endif /* __osf__ */
                ( const time_t * ) &tp.tv_sec );
    strcpy ( time_string, asctime ( local ) );
    if ( time_string[strlen ( time_string )-1] == '\n' )
        time_string[strlen ( time_string )-1] = '\0';
    return time_string;
    }


static char *get_user_at_host ( void )
    {
    static char     user_string[128];
    static int  first_time = 1;
    struct passwd   *pwd;
    struct utsname  uts;

    if ( first_time )
        {
        pwd = getpwuid ( getuid() );
        uname ( &uts );
        sprintf ( user_string, "%s@%s", pwd->pw_name, uts.nodename );
        first_time = 0;
        }

    return user_string;
    }


char *get_user_name ( void )
    {
    static struct passwd *pwd = getpwuid ( getuid() );
    return pwd->pw_name;
    }


char *get_host ( void )
    {
    static char     user_string[128];
    static int  first_time = 1;

    if ( first_time )
        {
        struct utsname  uts;
        uname ( &uts );
        sprintf ( user_string, "%s", uts.nodename );
        first_time = 0;
        }

    return user_string;
    }


void pave_log_start ( char *exe_name )
    {
    FILE *fp;
    char dummy_host_name[256], dummy_path_name[256], dummy_file_name[256];
    struct timeval tp;
    struct timezone tzp;

    parseLongDataSetName ( exe_name,dummy_host_name,dummy_path_name,dummy_file_name );
    logFile[0] = '\0';
    if ( dummy_path_name[0] ) sprintf ( logFile, "%s/", dummy_path_name );
    strcat ( logFile, "." );
    strcat ( logFile, dummy_file_name );
    strcat ( logFile, ".pave_usage.log" );
    fp = fopen ( logFile, "a+" );
    if ( fp == NULL )
        {
        fprintf ( stderr,"Error writing to file %s\n",logFile );
        exit ( -1 );
        }
    fprintf ( fp,
              "Enter %s %s %s\n",
              PAVE_SHORT_VERSION,
              get_time_string(),
              get_user_at_host()
            );
    gettimeofday ( &tp, &tzp );
    startSeconds = tp.tv_sec;
    fclose ( fp );
    chmod ( logFile, ( mode_t ) S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH );
    }


void pave_log_stop ( void )
    {
    struct timeval tp;
    struct timezone tzp;
    FILE   *fp = fopen ( logFile, "a+" );
    long   esec, hr, min, sec;

    gettimeofday ( &tp, &tzp );
    esec = tp.tv_sec - startSeconds;
    sec = esec%60;
    min = ( ( esec-sec ) /60 ) %60;
    hr = ( esec-min*60-sec ) /3600;
    fprintf ( fp,
              "Exit  %s %s %s %02ld:%02ld:%02ld clock time\n",
              PAVE_SHORT_VERSION,
              get_time_string(),
              get_user_at_host(),
              hr,
              min,
              sec
            );
    fclose ( fp );
    }


void pave_version ( void )
    {
    printf ( "%s\n", PAVE_LONG_VERSION );
    fflush ( stdout );
    }




char *get_local_host_name ( void )
    {
    static char     hostname[128];
    struct utsname  uts;

    uname ( &uts );
    sprintf ( hostname, "%s", uts.nodename );
    return hostname;
    }

#ifndef __osf__
char *get_local_ip_number ( void )
    {
    struct      hostent *h_info;
    struct      in_addr *hptr;
    static char     local_ip_number[64];

    h_info = gethostbyname ( get_local_host_name() );
    hptr = ( struct in_addr * ) *h_info->h_addr_list;
    sprintf ( local_ip_number, "%s", inet_ntoa ( *hptr ) );
    return local_ip_number;
    }
#endif // #ifndef __osf__


void init ( void )
    {
#ifndef __osf__
    char    *mcnc_subnets[] = { "198", "128" },
                              *local_ip_number = get_local_ip_number(),
                               versions[160];
    int     i, len, at_mcnc = 0;

    strcpy ( versions, PAVE_SHORT_VERSION );
    strcat ( versions, PAVE_LONG_VERSION );
    len = strlen ( versions );
    for ( i = 0; i < len; i++ ) versions[i] = toupper ( versions[i] );
#endif // #ifndef __osf__
    }


void pave_startup_msg()
    {
    printf ( "\n" );
    printf ( "    Package for Analysis and Visualization of Environmental data        \n" );
    printf ( "               %s\n",                                    PAVE_SHORT_VERSION );
    printf ( "    Version=$Id: Main.cc 83 2018-03-12 19:24:33Z coats $\n" );
    printf ( "                                                                        \n" );
    printf ( "    Copyright (C) 1996-2004 MCNC,                                       \n" );
    printf ( "              (C) 2004-2010 UNC Institute for the Environment           \n" );
    printf ( "              (C) 2018-     Carlie J. Coats, Jr., Ph.D.                 \n" );
    printf ( "                                                                        \n" );
    printf ( "    Licensed under the GNU General Public License Version 2.            \n" );
    printf ( "    See gpl.txt for more details.                                       \n" );
    printf ( "                                                                        \n" );
    printf ( "    For further information on PAVE:                                    \n" );
    printf ( "         Usage: type -usage in PAVE's standard input                    \n" );
    printf ( "    User Guide: https://cjcoats.github.io/pave/PaveManual.html          \n" );
    printf ( "           FAQ: https://cjcoats.github.io/pave/Pave.FAQ.html          \n\n" );
    fflush ( stdout );
    }


void pave_copyright ( void )
    {
    printf ( "\n" );
    printf ( "    Package for Analysis and Visualization of Environmental data        \n" );
    printf ( "    %s\n",                                     PAVE_SHORT_VERSION           );
    printf ( "    Version=$Id: Main.cc 83 2018-03-12 19:24:33Z coats $\n" );
    printf ( "                                                                        \n" );
    printf ( "    Copyright (C) 1996-2004 MCNC,                                       \n" );
    printf ( "              (C) 2004-2010 UNC Institute for the Environment           \n" );
    printf ( "              (C) 2018-     Carlie J. Coats, Jr., Ph.D.                 \n" );
    printf ( "                                                                        \n" );
    printf ( "    Licensed under the GNU General Public License Version 2.            \n" );
    printf ( "    See $PAVE_DIR/GPL.txt for more details.                             \n" );
    printf ( "                                                                        \n" );
    printf ( "    For copyright info on the libraries used, see README.txt            \n" );
    printf ( "    in the external_libs subdirectory.                                  \n" );
    fflush ( stdout );
    }

// SRT added 950913 to catch any input from stdin
void read_input ( caddr_t client_data, int *, XtInputId * )
    {
    static char line[2048];
    XINPUT_DATA *p_xp = ( XINPUT_DATA * ) client_data;
    int len;

    // read the input line
    if ( fgets ( line, sizeof ( line ), p_xp->stream ) == NULL )
        {
        // perror ("fgets");
        return; // exit(1);
        }

    // strip off the '\n' at the end
    len = strlen ( line );
    if ( line[len-1] == '\n' ) line[len-1]='\0';

    window->parseMessage ( line );
    }


#if (XlibSpecificationRelease>=5)
int main ( int argc, char **argv )
#else
int main ( unsigned int argc, char *argv[] )
#endif
    {

    XtAppContext this_app; // SRT added 950913
    XEvent theEvent;       // SRT added 950913
    int i;

    XINPUT_DATA xpinfo;    // SRT added 950913

    char errorMsg[512];

    errorMsg[0] = '\0';

    //verify_copyright_file();
    init();
    pave_startup_msg();

    for ( i = 0; i < argc; i++ )
        if  ( ( !strcasecmp ( argv[i], "-?"         ) ) ||
              ( !strcasecmp ( argv[i], "--?"        ) ) ||
              ( !strcasecmp ( argv[i], "--help"     ) ) ||
              ( !strcasecmp ( argv[i], "--fullhelp" ) ) ||
              ( !strcasecmp ( argv[i], "--usage"    ) ) ||
              ( !strcasecmp ( argv[i], "-help"      ) ) ||
              ( !strcasecmp ( argv[i], "-fullhelp"  ) ) ||
              ( !strcasecmp ( argv[i], "-usage"     ) ) ||
              ( !strcasecmp ( argv[i], "help"       ) ) ||
              ( !strcasecmp ( argv[i], "fullhelp"   ) ) ||
              ( !strcasecmp ( argv[i], "usage"      ) ) )
            {
            pave_usage();
            exit ( 1 );
            }

    //   pave_log_start(argv[0]); // disabled by A.Traynov 04/10/2002

#ifdef DIAGNOSTICS
    printf ( "Enter '" );
    for ( i = 0; i < argc; i++ )
        {
        if ( i ) printf ( " " );
        printf ( "%s", argv[i] );
        }
    printf ( "'\n" );
    fflush ( stdout );
#endif // DIAGNOSTICS

    AppInit *uam = new AppInit ( "PAVE" );
    uam->initialize ( &argc, argv );

    char title[100];
    sprintf ( title, "%s (%s)", PAVE_SHORT_VERSION, get_user_at_host() );

    char historyfile[255];
    sprintf ( historyfile, "%s/.pave_history_rc", getenv ( "HOME" ) );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "using historyfile '%s'\n", historyfile );
#endif

    window  = new DriverWnd ( uam, title, historyfile, argc, argv, errorMsg );
    if ( errorMsg[0] )
        {
        fprintf ( stderr, "%s\n", errorMsg );
        delete uam;
        delete window;
        return 1;
        }

    // to catch standard input add this callback
    xpinfo.stream = stdin;
    XtAppAddInput (
        ( XtAppContext ) uam->appContext(), // app_context
        ( int ) fileno ( stdin ),       // source
        ( XtPointer ) XtInputReadMask,      // condition
        ( XtInputCallbackProc ) read_input, // proc
        ( XtPointer ) &xpinfo       // closure
    );

    uam->realizeWidget();

    // uam->handleEvents(); SRT 950913 removed and replaced with code below
    this_app = uam->appContext();        // added SRT 950913
    while ( 1 )                  // added SRT 950913
        {
        // added SRT 950913
        XtAppNextEvent ( this_app, &theEvent ); // added SRT 950913
        XtDispatchEvent ( &theEvent );      // added SRT 950913
        }                   // added SRT 950913
    }

