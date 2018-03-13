/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 2.4
 *
 *  File: $Id: busDebug.h 84 2018-03-12 21:26:53Z coats $
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
 * ABOUT:  busDebug.h
 *
 *      busDebug.h is the header file that defines debugging functions
 *      and a number of variables to selectively set debugging on or off.
 *
 * VERSION "$Id: busDebug.h 84 2018-03-12 21:26:53Z coats $"
 ********************************************************************
 *
 * HISTORY: EDSS_SB - Client.h
 *
 * Date: 17-Jan-95
 * Version: 0.3
 * Change Description: ORIGINAL VERSION
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 ********************************************************************/

/* so that this header isn't included twice... */

#ifndef SBUS_DEBUG_H_INCLUDED
#define SBUS_DEBUG_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhDebug= "$Id: busDebug.h 84 2018-03-12 21:26:53Z coats $" ;
#endif

#include <stdio.h>

#define debug0(c,x) { if (c) { printf(x); fflush(stdout);}}
#define debug1(c,x,y) { if (c) { printf(x,y); fflush(stdout);}}
#define debug2(c,x,y,z) { if (c) { printf(x,y,z); fflush(stdout);}}
#define debug3(c,x,y,z,a) { if (c) { printf(x,y,z,a); fflush(stdout);}}
#define debug4(c,x,y,z,a,b) { if (c) { printf(x,y,z,a,b); fflush(stdout);}}

#define DEBUG_CLIENT	0
#define DEBUG_MASTER	0
#define DEBUG_DISP	0
#define DEBUG_RWMSG	0
#define DEBUG_RESPONDER 0
#define DEBUG_QUEUE	0
#define DEBUG_NEW_MOD	0
#define DEBUG_SOCKET	0

void debugShowMessage(struct BusMessage *bmsg);

#endif /* SBUS_DEBUG_H_INCLUDED */

