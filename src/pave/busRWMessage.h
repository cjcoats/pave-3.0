/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busRWMessage.h 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  EDSS_SB - busRWMessage.h
 *
 *       This include file contains (data structures and) function prototypes
 *       for bus read/write messages functionality.
 *
 * VERSION "$Id: busRWMessage.h 83 2018-03-12 19:24:33Z coats $"
 ********************************************************************
 *
 * HISTORY:
 *
 * Date: 30-Dec-94
 * Version: 0.2.1
 * Change Description: ORIGINAL CODE
 * Change author: R. Balay
 *
 * Date: 17-Jan-95
 * Version: 0.3
 * Change Description: Modified definition of BusSendBusByte function
 * Change author: R. Balay
 *
 * Date: 19-Aug-95
 * Version: 0.6b
 * Change Description: Changed function definitions of BusSendBusByte etc to
 *                     use  char *
 * Change author: Balay, R.
 *
 * Date: 15-Aug-96
 * Version: 0.7
 * Change Description:  Changed definition of BusGetReponse to return int
 *                     instead of void
 * Change author: Balay, R.
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

/* so that this header isn't included twice... */

#ifndef SBUS_READ_MSG_H_INCLUDED
#define SBUS_READ_MSG_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhbusRWMsg= "$Id: busRWMessage.h 83 2018-03-12 19:24:33Z coats $" ;
#endif

#include "busMsgQue.h"
#include "busClient.h"

int getSeqNum ( struct BusData * );
int BusReadMessage ( int, struct BusMessage * );
int BusWriteMessage ( int, struct BusMessage * );
int BusSendBusByte ( struct BusData *, int, unsigned char, int,  char * );
int BusReplyBusByte ( int, int, int, int, unsigned char, int,  char * );
int BusForwdBusByte ( int, int, int, int, unsigned char, int,  char * );
int BusGetResponse ( struct BusData *, int, unsigned char, char ** );

#endif


