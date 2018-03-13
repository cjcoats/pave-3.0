#ifndef SELECTLOADSAVE_SERVER_H
#define SELECTLOADSAVE_SERVER_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)SelectLoadSaveServer.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.SelectLoadSaveServer.h
 * Last updated: 12/15/97 16:28:08
 *
 * Made available by MCNC and the Carolina Environmental Program of UNC Chapel
 * Hill under terms of the GNU Public License.  See gpl.txt for more details.
 *
 * See file COPYRIGHT for license information on this and supporting software.
 *
 * Carolina Environmental Program
 * University of North Carolina at Chapel Hill
 * 137 E. Franklin St.
 * Chapel Hill, NC 27599-6116
 *
 * See http://www.cep.unc.edu, http://www.cmascenter.org, http://bugz.unc.edu
 *
 ****************************************************************************/

///////////////////////////////////////////////////
///////////////////////////////////////////////////
// File:	SelectLoadSaveServer.h 
// Author:	K. Eng Pua
// Date:	April 18, 1995
///////////////////////////////////////////////////
//
//   SelectLoadSaveServer Class
//
//   SelectLoadSaveServer : SelectionServer       Concrete
//        1. Reads items from file and loads
//           them to selection list
//        2. Saves items in the selection list
//           to file
//        3. Creates/Posts dialog box for loading
//           items
//        4. Creates/Posts dialog box for saving
//           items
//
///////////////////////////////////////////////////
//
// Modification History
// When   Who What
// 961011 SRT Added load_cancelCB(), load_cancel_cb(), save_cancelCB(), save_cancel_cb()
//

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <Xm/Xm.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/SelectioB.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/Label.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "SelectionServer.h"
#include "Util.h"

class SelectLoadSaveServer : public SelectionServer {
   public:
	SelectLoadSaveServer(char *name, Widget parent, char *dialog_tile, char *file_marker);

	virtual ~SelectLoadSaveServer() { };

	void postLoadSelectionDialog();
	void postSaveSelectionDialog();

	void loadSelectionFile(char *filename);

	virtual /* added SRT */
	void saveSelectionListToFile(char *filename, Widget dialog);

   private:
	Widget		load_selection_dialog_;
	Widget		save_selection_dialog_;

        void createLoadSelectionDialog();
        void createSaveSelectionDialog();

	static void load_okCB(Widget, XtPointer clientData, XtPointer callData);
	void load_ok_cb(XmString);

	static void save_okCB(Widget, XtPointer clientData, XtPointer callData);
	/*virtual*/ void save_ok_cb(XmString);

	static void load_cancelCB(Widget, XtPointer clientData, XtPointer callData);
	void load_cancel_cb(XmString);

	static void save_cancelCB(Widget, XtPointer clientData, XtPointer callData);
	virtual void save_cancel_cb(XmString);

};

#endif
