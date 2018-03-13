#ifndef FORMULA_SERVER_H
#define FORMULA_SERVER_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)FormulaServer.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.FormulaServer.h
 * Last updated: 12/15/97 16:26:29
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
// File:	FormulaServer.h 
// Author:	K. Eng Pua
// Date:	Jan 12, 1995
///////////////////////////////////////////////////
//
//    FormulaServer Class
//
//    FormulaServer : SelectLoadSaveServer        Concrete
//        1. Creates selection dialog for formula
//        2. Adds formula to selection list
//        3. Parses the formula                   ::parseFormula()
//        4. Retrieves relevant data specified    ::retrieveData()
//           in the formula
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950516  Added getFormulaParent()
// SRT  950831  Added removeAllItems()
// SRT  950831  Added removeThisItem() routine
// SRT  950901  Added verifyCurrentSelection() routine
// SRT  960410  Added int itemIsAlreadyAdded(char *item);
// SRT  960410  Added selectThisItemAddingIfNecessary(char *item)
//
///////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */


        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <Xm/Xm.h>

#include "Formula.h"
#include "Alias.h"
#include "SelectLoadSaveServer.h"
#include "BtsData.h"
#include "LinkedList.h"
#include "Util.h"

extern "C" {
#include "bus.h"
}

class FormulaServer : public SelectLoadSaveServer, public BtsData {
   public:
	FormulaServer(char *name, Widget parent, char *dialog_tile, 
			char *file_marker, Widget caseListDialog, BusData *bd,
                        linkedList *datasetList_,
                        linkedList *formulaList_,
                        linkedList *domainList_,
                        linkedList *levelList_
			);

	virtual ~FormulaServer();               // Destructor

        void getCaseList();
	int parseSuccess();
	int retrieveSuccess();
        Widget getFormulaParent() const { return fparent_; }   

	int addItem(char *item, Widget dialog); // SRT 950804 
	int itemIsAlreadyAdded(char *item);
        int removeAllItems(Widget dialog);
        int removeThisItem(char *item); // SRT 950831
	void verifyCurrentSelection(void);
	int selectThisItemAddingIfNecessary(char *item);
	void addNewAlias(Alias *);
	int isAliasAdded(char *);
	int expandAlias(char *, char *, char);
	int checkAlias(linkedList *, char *);
	void printAlias(FILE *);
	void printAlias(void);
	int findAndRemoveAlias(char *);
	void printAliasList(FILE *fp);

        linkedList *formulaListP_;

   private:
	BusData		*for_bd_;

        linkedList *datasetListP_;
        linkedList *domainListP_;
        linkedList *levelListP_;

        linkedList *aliasListP_;

	// SRT void edit_add_cb();

        Widget case_list_dialog_;

	Widget createEditSelectionDialog(); 

	Widget fparent_;

	// SRT 950804 int addItem(char *item, Widget dialog);

        void edit_delete_cb();  // will override the one from SelectionServer.cc

};


#endif
