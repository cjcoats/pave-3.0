/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 2.4
 *
 *  File: $Id: busMasterTime.h 84 2018-03-12 21:26:53Z coats $
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
 * ABOUT: - busMasterTime.h
 *
 *         busMasterTime.h consists of definitions for functions to measure busMaster
 *                         service time
 *
 *         INPUT FILES:       stdin stream
 *         OUTPUT FILES:      stdout stream
 *         ERROR HANDLING:    output to stderr (screen)
 *         INPUT PARAMETERS:  client name (ascii string), optional
 *         INPUT OPTIONS:     if client name is not input, default is
 *
 *         NETWORKING:        remote/network connectivity is via bus library (sockets)
 *
 *         COMPILATION:       (see makefile)
 *
 *         KNOWN BUGS:  :-(
 *
 *         OTHER NOTES: :-)
 *
 * VERSION "$Id: busMasterTime.h 84 2018-03-12 21:26:53Z coats $"
 ********************************************************************
 *
 * HISTORY: EDSS_SB - busMasterTime.h
 * 
 * Date: 13-Aug-96
 * Version: 0.7
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 * 
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 ********************************************************************/

#ifndef SBUS_MASTERTIME_H_INCLUDED
#define SBUS_MASTERTIME_H_INCLUDED
 
#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhbusMasterTime= "$Id: busMasterTime.h 84 2018-03-12 21:26:53Z coats $" ;
#endif

void t_start();
void t_stop();
double t_getrtime(void);
double t_getutime(void);
double t_getstime(void);

#endif
