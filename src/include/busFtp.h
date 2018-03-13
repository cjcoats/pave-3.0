/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 2.4
 *
 *  File: $Id: busFtp.h 84 2018-03-12 21:26:53Z coats $
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
 * ABOUT:   busFtp.h
 *
 *     busFtp.h is a header file for bus utility functions for transferring files
 *     between machines.
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
 * VERSION "$Id: busFtp.h 84 2018-03-12 21:26:53Z coats $"
 ********************************************************************
 *
 * HISTORY:EDSS_SB - busFtp.h
 *
 * Date: 1-Feb-95
 * Version: 0.3
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 ********************************************************************/

#ifndef SBUS_FTP_H_INCLUDED
#define SBUS_FTP_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhbusFtp= "$Id: busFtp.h 84 2018-03-12 21:26:53Z coats $" ;
#endif

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
/* #include <sys/dir.h> */
#include <dirent.h> 
#include <unistd.h>
#include <sys/file.h>
#include <netdb.h>
#include <netinet/in.h>

#include "bus.h"

#define MAXLINE		1024
#define MAXDATA 4000
#define MAXNUMFILES 512
#define FTP_ERR_FOPEN -1
#define FTP_ERR_NONE   1
#define FTP_ERR_HOST  -2
#define FTP_ERR_LOCALHOST -3
#define FTP_ERR_CONNECT -4

#define FTP_GET 1
#define FTP_PUT 2

#define DIR_OPEN_ERR  -1
#define DIR_HOST_UNKNOWN -2
#define DIR_REMOTE_FETCH -3

#define IS_DIR 0
#define IS_FILE 1


#define DEBUG_FTP 0


int FTP_xferBINARY(struct BusData *bd, int mode, char *hostName, 
		  char *localFile, char *remoteFile);

/* Functions for Directory search */
int FTP_getdir(struct BusData *bd, char *hname, char *directory, int code, 
		int returnType, char **fileList);
int FTP_dirRemote(struct BusData *bd, char *ipaddr, char *directory, 
		int code, int returnType);
int FTP_dirLocal(char *directory, int code, char **fList);

int is_localhost(char *hname, char *ipaddr);
int is_accessible(char *file);

#endif  /* SBUS_FTP_H_INCLUDED */
