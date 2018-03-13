#ifndef SPECIESSERVER_H
#define SPECIESSERVER_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)SpeciesServer.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.SpeciesServer.h
 * Last updated: 12/15/97 16:28:18
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
// File:	SpeciesServer.h 
// Author:	K. Eng Pua
// Date:	April 18, 1995
///////////////////////////////////////////////////
//
//    SpeciesServer Class
//
//    SpeciesServer: SelectionServer              Concrete
//        1. Creates and posts selection dialog
//           for species
//        2. Adds species to selection list
//        3. Clears all items in selection list
//
///////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Modification History
// SRT 950908 added int SpeciesServer::removeAllItems(Widget dialog)
// SRT 960410 added singleClickCB(), single_click_cb(), setFormulaServer()
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

#include "SelectionServer.h"
#include "FormulaServer.h"
#include "Util.h"


class SpeciesServer:public SelectionServer {
   public:
	SpeciesServer(char *name, Widget parent, char *dialog_tile);
	void postSpeciesDialog(char **speclist, int nspecies, int dataSetNum);
	void clearList();
	int removeAllItems(Widget dialog);
	int setFormulaServer(void *formulaServer);

   private:
	int addItem(char *item, Widget dialog); 
        Widget createEditSelectionDialog();

        static void singleClickCB(Widget, XtPointer clientData, XtPointer callData);
        virtual void single_click_cb();

	void *formulaServer_;
};


#endif
