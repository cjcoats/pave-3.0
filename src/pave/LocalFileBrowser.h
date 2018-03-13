#ifndef ___LOCALFILEBROWSER_H___
#define ___LOCALFILEBROWSER_H___
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)LocalFileBrowser.h	1.7
 *     Pathname: /tmp_mnt/pub/storage/edss/framework/src/pave/pave_include/SCCS/s.LocalFileBrowser.h
 * Last updated: 18 Sep 1996 16:04:26
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
// File:	LocalFileBrowser.h 
// Author:	Steve Thorpe, based on Eng Pua's SelectLoadSaveServer.h
// Date:	May 22, 1996
///////////////////////////////////////////////////
//
//   LocalFileBrowser Class
//
//   LocalFileBrowser
//        1. Creates/Posts dialog box for choosing files to read from
//        2. Creates/Posts dialog box for choosing files to write to
//
///////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

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

#include "Util.h"

class LocalFileBrowser {

   public:
	LocalFileBrowser
        (        Widget parent,
                 char *fileTypeName,      // can be NULL
                 char *defaultDirectory,  // can be NULL

                 // The next two args are routines to be
                 // called when the user selects a file
                 // to save or a file to load.
                 // The calling class should have these
                 // routines declared in their class
                 // definition like this:
                 //
                 //   static void f(void *object, char *fname))
                 //
                 // These args can be passed in as NULL.
                 // (Although at least one should be non-NULL
                 // if you want this object to do anything)

                 void *, /* void (*gotSaveFileName)(void *, char *), */
                 void *, /* void (*gotLoadFileName)(void *, char *), */

                 // this is a pointer to the object with the
                 // above routines in it
                 void *object
        );

	~LocalFileBrowser();

	void postLoadSelectionDialog();
	void postSaveSelectionDialog();

   private:

        void createSaveSelectionDialog();
	static void save_okCB(Widget, XtPointer clientData, XtPointer callData);
	void save_ok_cb(XmString);

        void createLoadSelectionDialog();
	static void load_okCB(Widget, XtPointer clientData, XtPointer callData);
	void load_ok_cb(XmString);

	Widget parent_;

	Widget load_selection_dialog_;
	Widget save_selection_dialog_;

	char *defaultDirectory_;
	char *fileTypeName_;

	void (*gotSaveFileName_)(void *, char *);
	void (*gotLoadFileName_)(void *, char *);
	void *object_;
};

#endif // ___LOCALFILEBROWSER_H___
