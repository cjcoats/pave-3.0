/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busRpc.h 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:
 *
 *       This include file contains  function prototypes
 *       for remote procedure call functionality.
 *
 * VERSION "$Id: busRpc.h 83 2018-03-12 19:24:33Z coats $"
 ********************************************************************
 *
 * HISTORY:  EDSS_SB - busRpc.h
 *
 * Date: 25-Feb-95
 * Version: 0.3b
 * Change Description: ORIGINAL VERSION
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

#ifndef SBUS_RPC_H_INCLUDED
#define SBUS_RPC_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhbusRpc= "$Id: busRpc.h 83 2018-03-12 19:24:33Z coats $" ;
#endif


int BusCallRemote ( struct BusData *bd, int toModule, int typeId,
                    void ( *stub ) ( int, char *, char * ), char *args, char *res );

#endif
