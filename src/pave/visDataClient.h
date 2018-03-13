#ifndef VISDATACLIENT_H
#define VISDATACLIENT_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)visDataClient.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.visDataClient.h
 * Last updated: 12/15/97 16:29:13
 *
 * Made available by MCNC and the Carolina Environmental Program of UNC Chapel
 * Hill under terms of the GNU Public License.  See gpl.txt for more details.
 *
 * See file COPYRIGHT for license information on this and supporting software.
 *
 * Carolina Environmental Program
 * University of North Carolina at Chapel Hill
 * 137 E. Franklin St.
 * Chapel Hill, NC 27599-6116
 *
 * See http://www.cep.unc.edu, http://www.cmascenter.org, http://bugz.unc.edu
 *
 ****************************************************************************/

/*****************************************************************************/
/* Author:      Rajini Balay, NCSU, rajini@aristotle.csc.ncsu.edu            */
/* Date:        February 25, 1995                                            */
/*****************************************************************************/
/*
MODIFICATION HISTORY:

WHO  WHEN       WHAT
---  ----       ----
SRT  04/06/95   Added #ifdef __cplusplus lines
*/


        /* in order to get the linker to resolve Kathy's 
           subroutines when using CC to compile */
#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

        /* unistd.h messes up compilation sometimes because of "link" */
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

#include "busClient.h"
#include "busMsgQue.h"
#include "busError.h"
#include "busDebug.h"
#include "busXtClient.h"
#include "busRW.h"
#include "busVersion.h"
#include "busRpc.h"
#include "busUtil.h"

#include "vis_data.h"

#include "readuam.h"  	/*  Definitions of PAVE_SUCCESS and FAILURE */

#define XFER_ERR	    (0)
#define XFER_SUCCESS	(1)

#define GET_INFO	    (1)
#define GET_DATA	    (2)

#define DEBUG_EVAP	    (0)

int check_local_file(VIS_DATA *info);

int get_info  ( struct BusData *bd, VIS_DATA *info, char *message);
int get_data  ( struct BusData *bd, VIS_DATA *info, char *message);
int get_remote( struct BusData *bd, int code, VIS_DATA *info, char *message);
void EVAPLocalStub(int fd, char *data, char *results);
int initVisDataClient(struct BusData *bd, char *modName);
void EVAP_GetInfo(int fd, char *data);
void EVAP_GetData(int fd, char *data);

int sendVisData(int fd, VIS_DATA *info);
int getVisData (int fd, VIS_DATA *info);
int sendInteger(int fd, int n, int *nums);
int sendFloat  (int fd, int n, float *nums);
int getInteger (int fd, int *i);
int getIntegers(int fd, int *nums, int n);
int getFloat   (int fd, float *fl);
int getFloats  (int fd, float *nums, int n);
int getString  (int fd, char **buf);
int sendString (int fd, char *buf, int len);
int readSleepLoop (int fd, char *buf, int len);
int writeSleepLoop(int fd, char *buf, int len);

        /* in order to get the linker to resolve Kathy's
           subroutines when using CC to compile */
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif
