#ifndef SELECTION_SERVER_H
#define SELECTION_SERVER_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)SelectionServer.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.SelectionServer.h
 * Last updated: 12/15/97 16:28:12
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
// File:	SelectionServer.h 
// Author:	K. Eng Pua
// Date:	Jan 12, 1995
///////////////////////////////////////////////////
//
//   SelectionServer Class
//
//   SelectionServer : UIComponent                Abstract
//        1. Posts selection dialog
//        2. Adds/Deletes/Selects items in
//           selection list
//        3. Sets/Gets current selection
//        4. Gets item count in selection list
//
///////////////////////////////////////////////////////////////////////////////
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950523  Include "bts.h"
// SRT  950816  Added void freeSelectionElements(void)
// SRT  950831  Added removeAllItems()
// SRT  960410  Added void setDriverWnd(void *);
// SRT  960411  Added setScreenPosition(Position xpos, Position ypos);
//
//////////////////////////////////////////////////////////////////////////////


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

#include "UIComponent.h"
#include "bts.h"
#include "Util.h"

class SelectionServer : public UIComponent {
   public:
	SelectionServer(char *name, Widget parent, char *dialog_tile, char *file_marker);

	Widget postEditSelectionDialog();

	void setCurrentSelection(char *selection);

	void setScreenPosition(Position xpos, Position ypos); // SRT 960411

	char *getCurrSelection() const { return curr_selection_; };
	Widget getSelectionDialog() const { return edit_selection_dialog_; };

	int getItemCount();

	void setDoneFirstTime(void);

	void freeSelectionElements(void); // added 950816 SRT

	virtual void edit_add_cb();

	int getItemNumber(char *item);

        void setDriverWnd(void *); // 960410 SRT

	virtual void updateSpecies_cb(); // 960410 SRT

   protected:
        virtual Widget createEditSelectionDialog() = 0;
	virtual int addItem(char *item, Widget dialog) = 0;

	virtual int removeAllItems(Widget dialog) = 0;

	char		*title_;
	Widget		edit_selection_dialog_;
	Widget		parent_;
	char 		*file_marker_;
	char		*curr_selection_;


	static void edit_addCB(Widget, XtPointer clientData, XtPointer callData);

	static void edit_deleteCB(Widget, XtPointer clientData, XtPointer callData);
	virtual void edit_delete_cb();

	static void edit_selectCB(Widget, XtPointer clientData, XtPointer callData);
	virtual void edit_select_cb();

	void addItemToOrderedList(char *item, Widget dialog);
	void addItemToList(char *item, Widget dialog);

        int addingFirstTime_;

        void *driverWnd_;


   private:
	void showSelectionLabel(char *); // moved from public 960411 SRT

	Widget		selection_label_;

};


#endif
