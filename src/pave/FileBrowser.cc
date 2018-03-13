/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: FileBrowser.cc 83 2018-03-12 19:24:33Z coats $
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
//
// FileBrowser.cc
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// August 16, 1995
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT  950816  Implemented based on Rajini Balay, Dan Hils,
//      Steve Fine, and Ted Smith's code
//
/////////////////////////////////////////////////////////////

/* bald messes this up in some header file */
#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */

#include "Browser.h"

static const char SVN_ID[] = "$Id: FileBrowser.cc 83 2018-03-12 19:24:33Z coats $";

static char spathname[256] = "\0";
static char *pathname = spathname;
static struct BusData *bd = NULL;


// FileBrowseButtonCallback
//
// Activate callback for Browse button in PAVE dataset browser.
// Call the browser with where the file should be stored.
//
// Input Arguments:
// w = this widget
// clientData = the client data which should be the widget of
//      the text field for the file path
// callBackArg
//
// History of Major Changes:
// Created by Steve Fine 3/29/95
// Copied/modified by Steve Thorpe 8/16/95

void FileBrowseButtonCallback ( Widget w,
                                XtPointer clientData,
                                XtPointer callBackArg )
    {
    // SRT 950817 CallTheBrowser(pathname);
    }


/** initBrowser  *************************************************************
 *
 * Ensure that, when a message of type "BROWSER_REPLY" is received
 * from the remote file browser via the Bus,
 * the Bus callback "BrowserOKCallback" will be called by the Bus.
 *
 * Returns: Nothing.
 *
 * Input Arguments:  bdp---bus data.  Already initialized
 *
 * Input/Output Arguments:
 *
 * Output Arguments:
 *
 * Maintenance Notes:
 *
 * History of Major Changes:
 *    Based on code by Ted Smith and Dan Hils - SRT 950816

void initBrowser (struct BusData *bdp, char *name, XtAppContext context)
{
bd = bdp;
BusAddTypeCallback  (
            bd,
            BusFindTypeByName(bd, "BROWSER_REPLY"),
            BrowserOKCallback
            );
}



** CallTheBrowser *************************************************************
 *
 * First start up the local portion of the remote file browser
 *   by calling Rajini's new function,  BusVerifyClient("Browser").
 *   This will cause the remote file browser (a.k.a. "browser") to appear
 *   in a window on-screen.
 *
 * Then send a BROWSER_SEIZE message to the remote file browser, to seize
 *   control of the browser from any other bus client--i.e., EDSS module--
 *   that currently has control of the browser.
 *
 * Input Arguments: bd---"bus data" data structure.  When passed to this
 *    function, bd has already been initially populated
 *    by the call to BusXtInitialize in Ted's initSpClient(), which is found
 *    in SP_BusConnect.cc (like this function).
 *
 * Input/Output Arguments:pathname--the path name and file name selected
 *    by user from the on-screen window displayed by remote file browser.
 *    Of the form "host:path".  If host is omitted, assumes the local machine.
 *    If path is omitted, assumes the user's home directory.
 *   input--initially set to tell browser what host & directory to open on.
 *   output--Returned in this output parameter when user clicks "OK" button
 *     on browser.
 *
 * Called by: FileBrowseButtonCallback().
 *
 * History of Major Changes:
 *    Created by Dan Hils 6/12/95--really, copied from Rajini Balay's code.
 *                             */
static void CallTheBrowser ( char *pname )
    {
    int browserId;
    struct BusMessage seizeMsg;
    char errorStr[256] = "ERROR from CallTheBrowser() : could not start local portion of remote file browser in three seconds.";

    while ( BusVerifyClient ( bd, NULL, "Browser", 1, 10, NULL, errorStr ) )
        {
        sleep ( 6 ); // Wait another 6-sec. increment for browser to start,
        //  before sending it the BROWSER_SEIZE message.
        // BusVerifyClient returns 0 if the local portion is already
        //  running, and nonzero if it is not yet running.
        // Browser (local portion) *should* be running already,
        //  but I'm just being cautious.
        BusVerifyClient ( bd, NULL, "Browser", 1, 10, NULL, errorStr );
        }

    // Now that local portion of remote browser has been started,
    //    send a BROWSER_SEIZE message to seize control of it.
    browserId = BusFindModuleByName ( bd, "Browser" );

    if ( browserId != FIND_ID_ERR )
        {
        seizeMsg.toModule = browserId;
        seizeMsg.fromModule = bd->moduleId;
        seizeMsg.messageType = BusFindTypeByName ( bd, "BROWSER_SEIZE" );

        seizeMsg.messageLength = strlen ( pname )+1;
        seizeMsg.message = pname;
        BusSendMessage ( bd, &seizeMsg );
        }
    else
        {
        fprintf ( stderr, "Browser not running !! \n" );
        }

    }


/** BrowserOKCallback *********************************************************
 *
 * Passes "host:pathname" string, selected by user from remote file browser,
 *    to other routines.
 *
 * Called by: Bus, when BROWSER_REPLY message is received.
 *
 * Maintenance Notes:
 *
 * History of Major Changes:
 *    Created by Dan Hils 6/13/95, from Rajini Balay's code.
 *    Modified by DH 8/8/95 to ensure consistency of host:path between
 *       Find button-presses.
 *                             */
static void BrowserOKCallback ( struct BusMessage *bmsg )
    {

    struct BusMessage relMsg;
    int totalLength; // Total length of whole "bmsg->message" string
    // (e.g., 35, in example below).
    int filenameLength; // Length of file-name portion of "bmsg->message"
    //  string, below, including '/'.  (e.g., 7, below).
    char* p;
    char temp[200];

    relMsg.toModule = bmsg->fromModule;
    relMsg.fromModule = bd->moduleId;
    relMsg.messageType = BusFindTypeByName ( bd, "BROWSER_RELEASE" );
    relMsg.messageLength = 0;
    BusSendMessage ( bd, &relMsg );

    fprintf ( stderr, "File Selected: %s.\n", bmsg->message );
    //AssignText(bmsg->message);
    // Assign the "host:pathname" string, selected by user from remote
    //  file browser & returned to this function by the BROWSER_RELEASE msg,
    //  to the placeTextIntoMe swidget.   See object.cc's AssignText
    //  for details.  (Note: placeTextIntoMe is a global variable, defined
    //     at start of this file.)
    // AssignText is defined in object.cc & object.h, not in this file,
    //  to save this file from having to call UxPutText, which would
    //  require doing <#include UxLib.h>, and would entangle this bus
    //  code with UIMX user-interface code.

    totalLength = strlen ( bmsg->message );

    filenameLength = 0;
    p = bmsg->message;
    while ( *p++ != '\0' ) ; // Move p to end of "bmsg->message" string
    p--;                   // Move p back one so it doesn't point to '\0'.
    while ( *p-- != '/' )
        filenameLength++;  // Find last '/' in bmsg->message.
    strcpy ( pathname, "\n" ); // clear out any old contents of pathname.
    strncpy ( pathname, bmsg->message, totalLength - filenameLength );

    // Above code ensures that the next press of any Find button will use
    //  the same "host:path" just selected via the last-pressed Find button,
    //  even if that last "host:path" differs from the node's "Host
    //  For Execution".
    // Also ensure that path does NOT include a file name,
    //  or the browser will not work right next time
    //  it is called with pathname.  (E.g. pathname should be "neptune:
    //  /home/hils/sp/spui", NOT "neptune:/home/hils/spui/spui/Update".
    }


