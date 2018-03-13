
/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: newMaster.c 83 2018-03-12 19:24:33Z coats $
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
 *  ABOUT:  newMaster.c
 *
 *   The main bus master program. Starts the bus and quits the bus.
 *   Provides diagnostic output. However, user interaction is provided
 *   through a menu-driven client interface.
 *
 * INPUT:  The BusMaster will take stdinput command Quit to stop execution
 *         The bus can also be killed by typing kill -SIGUSR1 xxxxx
 *         where xxxx is the BusMaster pid.
 *
 * KNOWN BUGS:  :-(
 *
 * OTHER NOTES: :-)
 *
 ****************************************************************************
 * REVISION HISTORY: newMaster.c
 *
 * Date: 14-Dec-94
 * Version: 0.1
 * Change Description: ORIGINAL CODE
 * Change author: Leland Morrison, NCSU, CSC
 *
 * Date: 15-Dec-94
 * Version: 0.2
 * Change Description: Added some comments and main headers
 * Change author: M. Vouk, NCSU, CSC
 *
 * Date: 17-Dec-94
 * Version: 0.3
 * Change Description: Removed fd flow control (blokc/non-block) by
 *                     removing stdin interaction (lm).
 *                     Added a newer style header (mav)
 * Change authors: L. Morrison, M.A. Vouk, NCSU, CSC
 *
 * Date: 6-Apr-95
 * Version: 0.5
 * Change Description: BusMaster responds to QUIT and INT signals and
 *                     cleans ups the files created in /tmp.
 * Change author: Balay, R.
 *
 * Date: 9-July-95
 * Version: 0.5c
 * Change Description: Replaced function "cuserid" with "BusGetMyUserid()"
 * Change author: R. Balay
 *
 * Date: 14-Aug-96
 * Version: 0.7
 * Change Description: Added signal handler for SIGPIPE so that busMaster does
 *                     not die when a socket closes while reading/writing into it.
 *                     Also added timestamping code for measuring service times.
 * Change author: R. Balay
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 *
 ********************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "busSocket.h" /* prototypes/#defines for select */
#include "busMaster.h"
#include "busError.h"
#include "busMsgQue.h"
#include "busVersion.h"
#include "busRW.h"
#include "busDebug.h"


#define debugf(x) { printf(x); fflush(stdout); }
#define debugf2(x,y) { printf(x,y); fflush(stdout); }
/*
     The (socket) file descriptor sets are stored as bit fields in  arrays
     of  integers.  The following macros are provided for manipu-
     lating such file descriptor sets:  FD_ZERO()  initializes  a
     file  descriptor  set  fdset  to  the  null  set.   FD_SET()
     includes a particular file descriptor fd in fdset.  FD_CLR()
     removes  fd  from  fdset.   FD_ISSET() is nonzero if fd is a
     member of fdset, zero otherwise.  The behavior of these mac-
     ros  is  undefined  if  a file descriptor value is less than
     zero or greater than or equal to FD_SETSIZE.  FD_SETSIZE  is
     a constant defined in <sys/select.h>.
*/
void bus_action_fd_set ( int fd, char *data )
    {
    debug1 ( DEBUG_MASTER,"FD_SET-ing #%d, ",fd );
    FD_SET ( fd, ( fd_set * ) data );
    }

int readyFd = 0;
void bus_action_ready_fd ( int fd, char *data )
    {
    if ( FD_ISSET ( fd, ( fd_set * ) data ) )
        readyFd = fd;
    }

int notDone;

void exit_handler ( int signo )
    {
    notDone = 0;
    }

void donot_exit ( int signo )
    {
    printf ( "Received SIGPIPE \n" );
    notDone = 1;
    }

/* Main bus routine */
int main ( int argc, char **argv )
    {
    struct BusMasterData bmd;          /* busmaster data structure */
    fd_set rfds;                       /* address of file descriptors for input */
    int nfds,                          /* number of file descriptors in the set */
        err,                           /* read return status code */
        notDone;                       /* main loop control flag */
    struct sigaction act;
    char buffer[80], *sbusprocf, pid_fname[256], sbusfile[128], *filename;
    pid_t bus_pid;
    FILE *procfd;
    char s[256];

    /* Initialized the bus */
    debugf2 ( "SOFTWARE BUS: %s \n", BusVersion() );
    debugf ( "Initializing the bus master\n" );
    err=BusMasterInitialize ( &bmd );
    if ( err != SBUSERROR_NOT )
        {
        perror ( "Bus Server Init failed" );
        return;
        }

    nfds = FD_SETSIZE;

    notDone = 42;

    /* close standard input socket */
    close ( 0 );

    /* make sure that pid is in SBUSPROCF file */
    sbusprocf = getenv ( "SBUSPROCF" );
    bus_pid = getpid();
    if ( sbusprocf == NULL )
        {
        sprintf ( pid_fname, "/tmp/sbus_proc_%s", BusGetMyUserid() );
        sbusprocf = pid_fname;
        }
    if ( ( procfd = fopen ( sbusprocf, "w+" ) ) )
        {
        fprintf ( procfd,"%d",bus_pid );
        fclose ( procfd );
        }
    else
        {
        sprintf ( s,"Could not open %s",sbusprocf );
        perror ( s );
        }


    act.sa_handler = exit_handler;
    memset ( & ( act.sa_mask ), 0, sizeof ( sigset_t ) );

    /* allow bus to be killed by signal action */
    act.sa_flags = 0;
    sigaction ( SIGKILL, &act, NULL );
    sigaction ( SIGINT, &act, NULL );
    sigaction ( SIGQUIT, &act, NULL );

    /* Dont die when a SIGPIPE is recvd */
    act.sa_handler = donot_exit;
    memset ( & ( act.sa_mask ), 0, sizeof ( sigset_t ) );
    act.sa_flags = 0;
    sigaction ( SIGPIPE, &act, NULL );

    do
        {
        debug0 ( DEBUG_MASTER,"Setting up rfds table - " );
        FD_ZERO ( &rfds );
        debug0 ( DEBUG_MASTER,"fd_set table zeroed, " );

        /* perform FD_SET on all of the file descriptors in use by the bus */
        BusMasterActOnfds ( &bmd, bus_action_fd_set, ( char * ) &rfds );
        debug0 ( DEBUG_MASTER,"\n" );

        debug0 ( DEBUG_MASTER,"calling select... " );
        /* standard select function */
        err = select ( nfds, ( SELECTFDSETPTR ) &rfds, NULL, NULL, NULL );
        if ( err<=0 )
            {
            perror ( "select failed" );
            break;
            }
        else
            debug1 ( DEBUG_MASTER,"%d fd's ready... ", err );

        readyFd = -1;

        /* perform FD_ISSET on all of the file descriptors in use by the bus */
        BusMasterActOnfds ( &bmd, bus_action_ready_fd, ( char * ) &rfds );
        debug1 ( DEBUG_MASTER,"readyFd = %d\n",readyFd );

        if ( readyFd>=0 )
            BusMasterDispatcher ( &bmd, readyFd );
        else
            debugf ( "Error: Unknown file descriptor has data???\n" );
        }
    while ( notDone );

    printf ( "Bus Master going down...\n" );
    BusMasterShutdown ( &bmd );
    if ( remove ( sbusprocf ) )
        printf ( "Could not remove %s\n",sbusprocf );
    else
        printf ( "Removed %s\n",sbusprocf );

    filename = getenv ( SBUSLOCSAVE );
    if ( filename == NULL )
        {
        sprintf ( sbusfile, "/tmp/sbus_port_%s", BusGetMyUserid() );
        filename = sbusfile;
        }
    if ( remove ( filename ) )
        printf ( "Could not remove %s \n", filename );
    else
        printf ( "Removed %s \n", filename );

#if defined(TIME_ST_BUS) || defined(TIME_IAT)
    BusTSCloseFile();
#endif

    printf ( "bye-bye\n" );
    }
