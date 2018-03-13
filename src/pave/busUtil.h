/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busUtil.h 83 2018-03-12 19:24:33Z coats $
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
 ****************************************************************************                                   *
 * ABOUT:   busUtil.h
 *
 *     busUtil.h contains the definitions for higher level utility functions.
 *
 * VERSION "$Id: busUtil.h 83 2018-03-12 19:24:33Z coats $"
 ****************************************************************************
 *  REVISION HISTORY
 * HISTORY:
 *
 * Date: 17-July-95
 * Version: 0.6
 * Change Description: Original code
 * Change author: Rajini Balay
 *
 * Date: 24-July-95
 * Version: 0.6a
 * Change Description: added definition for BusExitCallback and ERR_BUFSIZE
 * Change author: Rajini Balay
 *
 * Version 02/2018 by Carlie J. Coats, Jr., for PAVE-2.4
 ****************************************************************************/

#ifndef SBUS_UTIL_H_INCLUDED
#define SBUS_UTIL_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhUtil = "$Id: busUtil.h 83 2018-03-12 19:24:33Z coats $" ;
#endif

#define ERR_BUFSIZE  256

int BusVerifyClient ( struct BusData *bd, char *ipAddress, char *moduleName,
                      int isUnique, int secsToWait, char *args, char *errorstr );
void BusGetMyIPaddress ( char *ipaddr );
void BusExitCallback ( struct BusData *bd );

#endif /* SBUS_UTIL_H_INCLUDED */
