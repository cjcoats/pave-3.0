#ifndef CASE_SERVER_H
#define CASE_SERVER_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)CaseServer.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.CaseServer.h
 * Last updated: 12/15/97 16:25:35
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
// File:	CaseServer.h 
// Author:	K. Eng Pua
// Date:	March 23, 1995
///////////////////////////////////////////////////
//
//   CaseServer Class                             Concrete
//   CaseServer : SelectLoadSaveServer
//        1. Creates selection dialog for case
//        2. Adds case to selection list
//
///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950607  Added linked list and speciesP args/variables
// SRT  950831  Added removeAllItems()
// SRT  950901  Added verifyCurrentSelection() routine
// SRT  950905  Added setFormulaServer() routine
// SRT  960410  Added VIS_DATA *CaseServer::get_dataSets_VISDATA(char *item)
// SRT  960410  Added void updateSpecies_cb(void)
//
//////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
//#define __UNISTD_H__
#endif

#include <unistd.h>
#include <Xm/Xm.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/RowColumn.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "SelectLoadSaveServer.h"
#include "LinkedList.h"
#include "LocalFileBrowser.h"
#include "SpeciesServer.h"
#include "Formula.h"
#include "Util.h"
#include "AppInit.h"
#include "DriverWnd.h" // SRT 960410

#include "DataSet.h"
#include "bts.h"

#include "FormulaServer.h"


extern void FileBrowseButtonCallback // in Browser.cc
	(Widget w, XtPointer clientData, XtPointer callBackArg);



class CaseServer : public SelectLoadSaveServer {
   public:
	CaseServer	(
			char *name, 
			Widget parent, 
			char *dialog_tile, 
			char *file_marker,
        		linkedList *datasetList_,
        		linkedList *formulaList_,
        		linkedList *domainList_,
        		linkedList *levelList_,
			SpeciesServer *species_,
			struct BusData *bd,
			AppInit *app
			);

        int addItem(char *item, Widget dialog);
        int removeAllItems(Widget dialog);

        virtual ~CaseServer();               // Destructor
	int setFormulaServer(FormulaServer *formulaS);

        void verifyCurrentSelection(void);

	VIS_DATA *get_dataSets_VISDATA(char *item);  // added SRT 960410  

	struct BusData *bd_;

        static LocalFileBrowser *myLocalBrowser;

   private:
//	Widget	host_list_;
//	Widget	path_list_;
	Widget  parent_;

        linkedList *datasetListP_;
        linkedList *formulaListP_;
        linkedList *domainListP_;
	FormulaServer *formulaS_;
        linkedList *levelListP_;
        SpeciesServer *speciesP_;

	void notifyFormulasDataSetsWereChanged(void);
	void notifyFormulasDataSetsWereChanged(int);

	Widget createEditSelectionDialog();


	//void edit_add_cb();  // will override the one from SelectionServer.cc
	static void edit_addCB(Widget, XtPointer clientData, XtPointer callData); // will replace the one from SelectionServer.cc


	void edit_delete_cb();  // will override the one from SelectionServer.cc

        static void localFileCB(void *object, char *fname);
        void addLocalFile(char *fname);

	AppInit *app_;

	void updateSpecies_cb(void); // SRT 960410


};


void init_Browser(struct BusData *bd);

#endif
