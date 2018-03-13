/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 2.4
 *
 *  File: $Id: busXtClient.h 84 2018-03-12 21:26:53Z coats $
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
 * ABOUT: busXtClient.h
 *
 *    Callback and prototype definitions for use with X-clients.
 *
 * VERSION "$Id: busXtClient.h 84 2018-03-12 21:26:53Z coats $"
 ********************************************************************
 *
 * HISTORY:
 * Date: 14-Dec-94
 * Version: 0.1
 * Change Description: ORIGINAL VERSION 
 * Change author: Leland Morrison, NCSU, CSC 
 *
 * Date: 15-Dec-94
 * Version: 0.2
 * Change Description: Added some comments and main headers
 * Change author: Vouk
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 ********************************************************************/


/* so that this header isn't included twice... */
#ifndef SBUS_XT_CLIENT_H_INCLUDED
#define SBUS_XT_CLIENT_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhbusXtClient= "$Id: busXtClient.h 84 2018-03-12 21:26:53Z coats $" ;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>  
#include "busClient.h"

struct BusXtData {
  XtInputId xtid;
};

/***********************************************************
  For use of the bus by X toolkit (Motif/Athena/Openlook)
  based programs
  **********************************************************/

int BusXtInitialize(struct BusData *bd,
		     XtAppContext app_context);
/* connects this client to the bus, and setups an
   XtCallback to handle bus reads */

void BusXtClose( struct BusData *bd );
/* should be used to close a connection created by
   BusXtInitialize */

#endif
