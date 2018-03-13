/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 2.4
 *
 *  File: $Id: busRepReq.h 84 2018-03-12 21:26:53Z coats $
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
 * ABOUT: busRepReq.h
 *
 *      Bus message request/reply code definitions.
 *
 * VERSION "$Id: busRepReq.h 84 2018-03-12 21:26:53Z coats $"
 ********************************************************************
 *
 * HISTORY: EDSS_SB - busRepReq.h
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
 * Date: 7-Jan-95 
 * Version: 0.2.1
 * Change Description: Added definitions of FIND_NAME_ERR and FIND_ID_ERR 
 * Change author: Rajini Balay, NCSU, CSC 
 *
 * Date: 18-March-95
 * Version: 0.4
 * Change Description: Definitions for BUSBYTE_FIND_MODULE_BY_NAME and
 *                     BUSBYTE_FIND_MODULE_BY_ID 
 * Change author: Balay, R.
 *
 * Date: 12-Apr-95
 * Version: 0.5a
 * Change Description: Added definitions for BUSBYTE_KILL_CLIENT
 * Change author: Balay, R.
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 ********************************************************************/
 
 /* so that this header isn't included twice... */ 
 
#ifndef SBUS_RESPONSE_REQUEST_H_INCLUDED 
#define SBUS_RESPONSE_REQUEST_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhbusRepReq= "$Id: busRepReq.h 84 2018-03-12 21:26:53Z coats $" ;
#endif

#define BUSBYTE_RESET_CONNECTION     0x00
#define BUSBYTE_MODULE_LEAVING       0x05
#define BUSBYTE_SHUTDOWN             0x0a

/* #define BUSBYTE_MESSAGE_WAITING      0x11 */
#define BUSBYTE_MESSAGE_HERE 	     0x11 
#define BUSBYTE_PLEASE_ACKNOWLEDGE   0x22

#define BUSBYTE_ACKNOWLEDGEMENT      0x24
#define BUSBYTE_GET_FIRST_MESSAGE    0x14
#define BUSBYTE_DEQUE_MESSAGE  	     0x18

#define BUSBYTE_REGISTER_TYPE        0x33

#define BUSBYTE_FIND_MODULE_BY_NAME  0x41
#define BUSBYTE_FIND_MODULE_BY_ID    0x42
#define BUSBYTE_FIND_TYPE_BY_NAME    0x44
#define BUSBYTE_FIND_TYPE_BY_ID      0x48

#define BUSBYTE_WHOS_CONNECTED       0x4a
#define BUSBYTE_MODULE_INFO_BY_NAME  0x4b
#define BUSBYTE_MODULE_INFO_BY_ID    0x4c

#define BUSBYTE_REQ_DIRECT_CONNECT   0x50
#define BUSBYTE_DIRECT_CONNECT_OK    0x51
#define BUSBYTE_DIRECT_CONNECT_FAIL  0x52
#define BUSBYTE_DIRECT_CONNECT_DELAY 0x53
#define BUSBYTE_REPLY_DIRECT_CONNECT 0x54

#define BUSBYTE_KILL_CLIENT          0x55

#define BUSBYTE_I_DONT_UNDERSTAND    0xff

#endif
