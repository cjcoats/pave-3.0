/* so that this header isn't included twice... */
#ifndef SBUS_MASTER_RTFUNCS_H_INCLUDED
#define SBUS_MASTER_RTFUNCS_H_INCLUDED

/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: masterRTFuncs.h 83 2018-03-12 19:24:33Z coats $
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
 *  REVISION HISTORY: masterRTFuncs.h
 *
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
 * Date: 27-Jan-95
 * Version: 0.3a
 * Change Description: Modified definition of BusMasterBroadcast to
 *                     take 'struct BusMessage *' as a parameter instead
 *             of 'struct BusMessage'.
 * Change author: Rajini Balay, CSC, NCSU.
 *
 * Date: 18-March-95
 * Version: 0.4
 * Change Description: Added definitions for BusMasterRTGetModuleInfoByName
 *                     and BusMasterRTGetModuleInfoById
 * Change author: Balay, R.
 *
 * Date: 12-Apr-95
 * Version: 0.5a
 * Change Description: Definition for the function BusMasterRTKillClient
 * Change author: Balay, R.
 *
 *  Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 *
 ********************************************************************/

#include "busMaster.h"

int BusMasterRTResetConnection  ( struct BusMasterData *,
                                  struct BusModuleNode *, struct BusMessage * );
int BusMasterRTModuleLeaving    ( struct BusMasterData *,
                                  struct BusModuleNode * );
int BusMasterRTProcessMessage   ( struct BusMasterData *,
                                  struct BusModuleNode *, struct BusMessage * );
int BusMasterRTPassMessage      ( struct BusMasterData *,
                                  struct BusModuleNode * );
int BusMasterRTFindModuleByName ( struct BusMasterData *,
                                  struct BusModuleNode *, struct BusMessage * );
int BusMasterRTFindModuleById   ( struct BusMasterData *,
                                  struct BusModuleNode *, struct BusMessage * );
int BusMasterRTGetModuleInfoByName ( struct BusMasterData *,
                                     struct BusModuleNode *, struct BusMessage * );
int BusMasterRTGetModuleInfoById   ( struct BusMasterData *,
                                     struct BusModuleNode *, struct BusMessage * );

int BusMasterRTFindTypeByName   ( struct BusMasterData *,
                                  struct BusModuleNode *, struct BusMessage * );
int BusMasterRTFindTypeById     ( struct BusMasterData *,
                                  struct BusModuleNode *, struct BusMessage * );
int BusMasterRTTellWhosConnected ( struct BusMasterData *,
                                   struct BusModuleNode *, struct BusMessage * );
int BusMasterRTReqDirect    ( struct BusMasterData *,
                              struct BusModuleNode *, struct BusMessage * );
int BusMasterRTReplyDirect  ( struct BusMasterData *,
                              struct BusModuleNode *, struct BusMessage * );
int BusMasterRTKillClient   ( struct BusMasterData *,
                              struct BusModuleNode *, struct BusMessage * );

void BusMasterBroadcast ( struct BusMasterData *bmd, struct BusMessage *bmsg );

#endif
