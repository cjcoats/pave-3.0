/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: busXt.c 83 2018-03-12 19:24:33Z coats $
 *  Copyright (C) 1996-2004 MCNC
 *            (C) 2004-2010 UNC Institute for the Environment
 *            (C) 2018-     Carlie J. Coats, Jr., Ph.D.
 *
 *  Licensed under the GNU General Public License Version 2.
 *  See enclosed gpl.txt for more details
 *
 *  For further information on PAVE:
 *      Usage: type -usage in PAVE's standard input
 *      User Guide: https://cjcoats.github.io/pave/PaveManual.html
 *      FAQ:        https://cjcoats.github.io/pave/Pave.FAQ.html
 *
 ****************************************************************************
 * ABOUT:  busXt.c
 *
 *      Bus routines for handling X-window and Motif events (for use with
 *      X-based and Motif-based clients).
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *
 ********************************************************************
 * REVISION HISTORY: busXt.c
 *
 * Date: 14-Dec-94
 * Version: 0.1
 * Change Description: ORIGINAL CODE
 * Change author: Leland Morrison, NCSU, CSC
 *
 * Date: 15-Dec-94
 * Version: 0.2
 * Change Description: Added some comments and main headers
 * Change author: M. Vouk, NCSU, CSC
 *
 * Date: 17-Jan-95
 * Version: 0.3
 * Change Description: Added 2 extra dummy parameters to BusXtInputDispatch
 *        to make it compatible with SGIs.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 10-Mar-95
 * Version: 0.4
 * Change Description: Equated xtid to null before close.
 * Change author: M. Vouk, NCSU, CSC
 *
 * Date: 24-July-95
 * Version: 0.6a
 * Change Description: Changed call to BusInitialize to conform with new iface.
 * Change author: R. Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 *
 ********************************************************************/

#include "busXtClient.h"
#include "busError.h"
#include "busDebug.h"

extern int masterAlive;

void BusXtInputDispatch ( XtPointer bd, int *dummy, XtInputId *id )
    {
    int err;

    err = BusDispatch ( ( struct BusData * ) bd );
    if ( err != SBUSERROR_NOT )
        {
        printf ( "CONNECTION TO BUS HAS CLOSED \n" );
        if ( err == SBUSERROR_NODATA )
            {
            masterAlive = 0;

            /* Remove the Input Callback from XtAppContext */
            BusXtClose ( ( struct BusData * ) bd );
            }

        }
    }

int BusXtInitialize ( struct BusData *bd,XtAppContext app_context )
    {
    int err;

    /* Initialize as an infinite life process */
    err = BusInitialize ( bd, -1, 0, NULL );

    if ( err == SBUSERROR_NOT )
        bd->xtd->xtid = XtAppAddInput (
                            app_context,
                            bd->fd,
                            XtInputReadMask,
                            BusXtInputDispatch,
                            ( XtPointer ) bd
                        );
    return err;
    }

void BusXtClose ( struct BusData *bd )
    {
    if ( bd->xtd != NULL && bd->xtd->xtid != 0 )
        {
        XtRemoveInput ( bd->xtd->xtid );
        bd->xtd->xtid = NULL;
        }

    BusClose ( bd );
    }

