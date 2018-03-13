#ifndef SBUS_VERSION_H_INCLUDED
#define SBUS_VERSION_H_INCLUDED

/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 2.4
 *
 *  File: $Id: busVersion.h 84 2018-03-12 21:26:53Z coats $
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
 * ABOUT:   busVersion.h
 *
 *     busVersion.h contains the current version number of the Software Bus.
 *
 * VERSION "$Id: busVersion.h 84 2018-03-12 21:26:53Z coats $"
 *
 * ********************************************************************
 * HISTORY: EDSS_SB - busVersion.h
 *
 * Date: 6-Apr-95
 * Version: 0.5
 * Change Description: Original code
 * Change author: Rajini Balay
 *
 * Date 12-Apr-95
 * Version: 0.5a
 * Change Description: Ability for BusMaster to send DIE signals to Clients
 * Change author: Rajini Balay
 *
 * Date 27-Apr-95
 * Version: 0.5b
 * Change Description: BusInitialize returns SBUSERROR_DUP_CLIENT if client
 *            with same name already exists
 * Change author: Rajini Balay
 *
 * Date: July-9-95
 * Version: 0.5c
 * Change Description: Added new function definition for "getMyuserid()"
 *                     as replacement for "cuserid"
 * Change author: R. Balay
 *
 * Date: 17-July-95
 * Version: 0.6
 * Change Description: Added new utility functions in busUtil.c
 * Change author: R. Balay
 *
 * Date: 24-July-95
 * Version: 0.6a
 * Change Description: Modified interface to BusInitialize to add an exitCallback
 * Change author: R. Balay
 *
 * Date: 7-Sept-95
 * Version: 0.6c
 * Change Description: Fixed bug with client not receving messages because
 *    the messages were accumulated in the RecvdMessages Queue and were
 *    not processed unless a message was received from the Bus - which
 *    did not send any because no ack was sent by the client
 * Change author: R. Balay
 *
 * Date: 18-Sept-95
 * Version: 0.6d
 * Change Description: Changed 'rsh' in BusStartModule to start the process in
 *    the background instead of redirecting it to /dev/null
 * Change author: R. Balay
 *
 * Date: 27-Oct-95
 * Version: 0.6d
 * Change Description: Fixed bug in getRemoteUserID in busUtil.h
 * Change author: R. Balay
 * 
 * Version 02/2018 by Carlie J. Coats, Jr., for PAVE-2.4
 *
 ********************************************************************/

#define SBUSVERSION "Version 6.6e (14-Feb-2018)"
#define BusVersion()     SBUSVERSION

#endif /* SBUS_VERSION_H_INCLUDED */
