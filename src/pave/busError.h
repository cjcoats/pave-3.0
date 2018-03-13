/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busError.h 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:   Error.h
 *
 *     Client.h is the include file that defines bus error codes.
 *
 * VERSION "$Id: busError.h 83 2018-03-12 19:24:33Z coats $"
 ********************************************************************
 *
 * HISTORY: Error.h
 *
 * Date: 14-Dec-94
 * Version: 0.1
 * Change Description: ORIGINAL VERSION
 * Change author: Leland Morrison, NCSU, CSC
 *
 * Date: 15-Dec-94
 * Version: 0.2
 * Change Description: Added main headers
 * Change author: Vouk
 *
 * Date: 18-March-95
 * Version: 0.4
 * Change Description: Added a new definition for SBUSERROR_MASTER_ABSENT
 * Change author: Balay, R.
 *
 * Date: 27-Apr-95
 * Version: 0.5b
 * Change Description: Added a new definition for SBUSERROR_DUP_CLIENT
 * Change author: Balay, R.
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 *
 ********************************************************************/

/* so that this header isn't included twice... */

#ifndef SBUS_ERROR_H_INCLUDED
#define SBUS_ERROR_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhError= "$Id: busError.h 83 2018-03-12 19:24:33Z coats $" ;
#endif

#define SBUSERROR_NOT                    0
#define SBUSERROR_DUP_CLIENT            -1

#define SBUSERROR_INITIALIZATION        -2
#define SBUSERROR_SBUSPORT_NOT_SET      -15

#define SBUSERROR_READ                  -3
#define SBUSERROR_WRITE                 -4
#define SBUSERROR_NODATA                -5

#define SBUSERROR_NOMEMORY              -6

#define SBUSERROR_BADPARAMETER          -7
#define SBUSERROR_CBNOTFOUND            -8

#define SBUSERROR_MSG_NOT_UNDERSTOOD    -9

#define SBUSERROR_SOCKET_COULDNT_CREATE -10
#define SBUSERROR_COULDNT_getsockname   -11
#define SBUSERROR_ACCEPT_FAILED         -12
#define SBUSERROR_INVALIDHOST           -13
#define SBUSERROR_CONNECT_FAILED        -14

#define SBUSERROR_MASTER_ABSENT         -16
#define SBUSERROR_GENERAL_FAILURE       -17

/* Definitions added by Rajini */
#define FIND_ID_ERR     -1
#define FIND_NAME_ERR   " "

#endif
