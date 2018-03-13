/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Browser.h 83 2018-03-12 19:24:33Z coats $
 *  Copyright (C) 1996-2004 MCNC
 *            (C) 2004-2010 UNC Institute for the Environment
 *            (C) 2018-     Carlie J. Coats, Jr., Ph.D.
 *
 *  Licensed under the GNU General Public License Version 2.
 *  See enclosed gpl.txt for more details
 *
 *  For further information on PAVE:
 *      Usage: type -usage in PAVE's standard input
 *      User Guide: https://www.cmascenter.org/pave/documentation/2.3/
 *      FAQ: https://www.cmascenter.org/pave/documentation/2.3/Pave.FAQ.html
 *
 ****************************************************************************
 * ABOUT:  Browser.h
 *
 *     Browser.h contains the definitions necessary for the motif-busclient,
 *     Browser.
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
 * VERSION "$Id: Browser.h 83 2018-03-12 19:24:33Z coats $"
 ********************************************************************
 *
 * HISTORY: EDSS_SB - Browser.c
 *
 * Date: 11-Apr-95
 * Version: 0.3
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 23-July-95
 * Version: 0.5
 * Change Description: Added include files for gethostbyaddr.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 16-Aug-95
 * Version: 0.5
 * Change Description: Added defn for FSB_FILTER 
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0:  clean-up of
 * unused prototypes.
 ********************************************************************/

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhBrowser = "$Id: Browser.h 83 2018-03-12 19:24:33Z coats $" ;
#endif

#include <stdio.h>
#include <string.h>
#include <X11/Xos.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>		/* for AF_INET */
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/List.h>
#include <Xm/FileSB.h> 
#include <Xm/Form.h> 
#include <Xm/SelectioB.h>
#include <Xm/MessageB.h>

#include "bus.h"
#include "busFtp.h"

/* DEFINES */
#define FSB_OK        1
#define FSB_CANCEL    2
#define FSB_HOST      3
#define FSB_FILTER    4

#define HST_OK        10
#define HST_CANCEL    11
#define HST_USER      12
#define HST_PATH      13

#define USER_OK       20
#define USER_CANCEL   21

#define PATH_OK       30
#define PATH_CANCEL   31

#define OK            100
#define CANCEL        101

#define MAX_FILES     512

struct host_item {
    char hname[80];
    char pathname[256];
};

/* Function Definitions */
void mmessage(char *message);
void mwessage(char *message);
void merror(char *message);
void unmapFileList();

/* int gethostname(char *name, int namelen); */
char *cuserid(char *s);
struct dirent *readdir(DIR *dirp);

void UpdateHostDialog();

/* Callback functions */
void   ASCIIZ_callback( struct BusData *bd, struct BusMessage *bmsg );
void recv_dir_callback( struct BusData *bd, struct BusMessage *bmsg );
void    Seize_callback( struct BusData *bd, struct BusMessage *bmsg );
void  Release_callback( struct BusData *bd, struct BusMessage *bmsg );

void  dir_search(Widget widget, XtPointer search_data);
void file_search(Widget widget, XtPointer search_data);
