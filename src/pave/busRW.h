/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busRW.h 83 2018-03-12 19:24:33Z coats $
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
 * ABOUT:  busRW.h
 *
 *   This include file contains (data structures and) function prototypes
 *   for bus read/write functionality.
 *
 * VERSION "$Id: busRW.h 83 2018-03-12 19:24:33Z coats $"
 ********************************************************************
 *
 * HISTORY: EDSS_SB - busRW.h
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
 * Date: July-9-95
 * Version: 0.5c
 * Change Description: Added new function definition for "BusGetMyUserid()"
 * Change author: R. Balay
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ********************************************************************/

/* so that this header isn't included twice... */

#ifndef SBUS_READ_H_INCLUDED
#define SBUS_READ_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhbusRW= "$Id: busRW.h 83 2018-03-12 19:24:33Z coats $" ;
#endif

#define BUFSIZE 4000  /* size of buffer used in the XDR routines */

/* id - input - an "id" number for the communication channel
   to use - (the socket, pipe, port, or whatever else)
   */

int BusReadCharacter ( int id, unsigned char  *c );
int BusReadInteger   ( int id, int   *i );
int BusReadFloat     ( int id, float *f );

int BusReadASCIIZ    ( int id, char **buffer );
int BusReadnString   ( int id, char **buffer, int *buflen );

int BusWriteCharacter ( int id, unsigned char   c );
int BusWriteInteger  ( int id, int    i );
int BusWriteFloat    ( int id, float  f );

int BusWriteASCIIZ   ( int id, char  *string );
int BusWritenString  ( int id, char  *buffer, int  length );

char *BusGetMyUserid();
#endif
