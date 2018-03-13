/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: BusConnect.cc 83 2018-03-12 19:24:33Z coats $
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
 ****************************************************************************/
////////////////////////////////////////////////////////////
// BusConnect.C:
//
////////////////////////////////////////////////////////////
//
// Revision History
// SRT  950523  Added check of REMOTEACCESS environment variable
//      so bus warning msgs are only printed out if the
//      PAVE wrapper script thought we were supposed to
//      have remote access
//
/////////////////////////////////////////////////////////////

#include "BusConnect.h"

static BusConnect *theBus;
static struct BusData bd;

extern void init_Browser ( struct BusData *bd ); // added 960411 SRT

void BusConnect::busCallback ( char * ) { } // SRT 950929


/***** Bus Callbacks *****/
void pave_callback ( struct BusData *bd, struct BusMessage *bmsg )
/* Call-back function for a ASCII data type called pave
   (that is, a simple ascii string, in plain English)
   This function a) checks the caller-id,
                 b) establishes the requested distribution mode,
                 c) informs user that message was received,
                 d) prints the message and flushes output buffer. */
    {
    char *from, *to;

    // Identify message sender
    from = BusFindModuleById ( bd, bmsg->fromModule );
    if ( from == NULL )
        from = "Unidentified source (source no longer connected to bus?)";

    // Identify message distribution mode
    if ( bmsg->toModule == BusBroadcast )
        to   = "Everybody";
    else if ( bmsg->toModule == BusByType )
        to   = "modules understanding type";
    else if ( bmsg->toModule == BusBounce )
        to   = "myself (Bounce)";
    else if ( bmsg->toModule == bd->moduleId )
        to   = "myself (my module id)";
    else if ( bmsg->toModule != bd->moduleId )
        to   = BusFindModuleById ( bd, bmsg->  toModule );
    else
        to = "Unknown (not good...)";

#ifdef DIAGNOSTICS
    // Inform user that msg was received, from whom it was received, and
    //   to whom it was directed (distribution mode)
    printf ( "PAVE message received:\n" );
    printf ( "\tFrom: %s\n", from );

    if ( to != NULL )
        printf ( "\tTo  : %s\n", to );

    // Print message
    printf ( "\"%s\"\n", bmsg->message );

    // Flush print buffer to ensure that msg is printed promptly
    fflush ( stdout );
#endif

    theBus->busCallback ( bmsg->message );

    }


void testcallback()
    {
    theBus->busCallback ( "TestCallback" );
    }



BusConnect::BusConnect ( AppInit *app )
    {
    bd.name = strdup ( "pave" );

    // attempt to attach to the bus
    int err = BusXtInitialize ( &bd, app->appContext() );
    if ( err != SBUSERROR_NOT )
        {
        fprintf ( stderr, "Did not connect to the software bus.\n" );
        isConnected_ = 0;
        }
    else
        {

        isConnected_ = 1;

#ifdef DIAGNOSTICS
        printf ( "Bus Connection established.\n" );

        // Get the handle for standard bus-managed pave data type
        printf ( "Find id for type pave\n" );
#endif

        int typeId = BusFindTypeByName ( &bd, "PAVE_TYPE" );

#ifdef DIAGNOSTICS
        printf ( "Setting callback for type pave\n" );
#endif

        BusAddTypeCallback ( &bd, typeId, pave_callback );

#ifdef DIAGNOSTICS
        printf ( "Callback for pave established.\n" );
#endif


        init_Browser ( &bd ); // added 950921 SRT
        }

    theBus = this;
    bd_ = &bd;
    }


