#ifndef DOMAINWINDOW_H
#define DOMAINWINDOW_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)DomainWnd.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.DomainWnd.h
 * Last updated: 12/15/97 16:26:09
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

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// File:	DomainWnd.h 
// Author:	K. Eng Pua
// Date:	Feb 22, 1995
///////////////////////////////////////////////////////////////////////////////
//
//   DomainWnd Class
//
//   DomainWnd : DrawWnd, RubberBand,       	Concrete
//                  Menus
//
//        1. Creates a top-level window		  AppInit
//        2. Supports a user interface that       DrawWnd
//           includes an application-specific
//           drawing area widget
//        3. Creates a pulldown menu at the top   Menus
//           of the window
//        4. Reads in information associated      ReadVisData
//           with a specific geographical area
//        5. Initializes the draw scale for       DrawScale
//           the plot
//        6. Draws the map and superimpose the
//           modeling grids on top
//        7. Supports selection of grid cells     RubberBand
//
//
// Revision History
// SRT  951107  Added setRange()
//		added int invalidateFormulasDependingOnMe(void);
//		added hideWindow()
// SRT  960520  Removed Eng Pua's DomainWnd constructor
// SRT  960525  Added saveDomain() and loadDomain()
//
//////////////////////////////////////////////////////////////////////////////

#define DOMAIN_H // SRT this is a hack to get around annoying compilation
		 // error when DomainWnd.h indirectly includes Domain.h,
		 // this file is unnecessary anyway for this DomainWnd.cc,
		 // so we'll just #define DOMAIN_H here so Domain.h won't bein
		 // included

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

#include "LinkedList.h"
#include "DrawScale.h"
#include "DrawWnd.h"
#include "ReadVisData.h"
#include "Menus.h"
#include "AppInit.h"
#include "RubberBand.h"
#include "bts.h"
#include "Formula.h"
#include "MapServer.h"
#include "LocalFileBrowser.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/ScrolledW.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

class DomainWnd : public DrawWnd, public Menus, public RubberBand {
   public:

		// SRT's version here:
        DomainWnd(AppInit *app, char *name, ReadVisData *vis,
                char *drawtype,
                Dimension width, Dimension height,
		char *percentsP,
		char *mapinfo,
		linkedList *formulaList,   // formulaList of its parent
                int exit_button_on = 1);  // 1 means Exit button sensitive. Otherwise insensitive

	virtual ~DomainWnd(); // added SRT

	virtual void drawDomain();
	virtual void drawMap();

	int *domainMask() const { return domain_mask_; }

	int rowMax() const { if (vis_) return vis_->row_max_; else return 0; }
	int colMax() const { if (vis_) return vis_->col_max_; else return 0; }

	void showWindow(void);
	void hideWindow(void);

        int setRange(int xmin, int ymin, int xmax, int ymax);

       int saveDomain(char *fname,     // file to save domain to
                        char *estring); // for error msgs
                                        // returns 1 if error, otherwise 0

        int loadDomain(char *fname,     // file to read domain from
                        char *estring); // for error msgs
                                        // returns 1 if error, otherwise 0

        virtual void drawDetail();

   protected:
	ReadVisData	*vis_;
        DrawScale 	s;

        Widget          canvas_;
	int		*domain_mask_;

        virtual void createUI ( Widget );

	void initDomainWnd(); 

   private:

	int	exit_button_on_;

	char	*percents_;

	char	*mapinfo_;

	linkedList *formulaList_;   // formulaList of its parent

	static void resizeCB(Widget, XtPointer, XtPointer);
	void resize();

	static void redisplayCB(Widget, XtPointer, XtPointer);
	void redisplay(XExposeEvent *event);

	static void animate_dialogCB(Widget, XtPointer, XtPointer);
	void animate_dialog_cb();

	static void exitCB(Widget, XtPointer, XtPointer);
	void exit_cb();

	static void loadCB(Widget, XtPointer clientData, XtPointer callData);
        static void loadDOMAINCB(void *object, char *fname);
        void loadDOMAIN_cb(char *fname);

	static void saveCB(Widget, XtPointer clientData, XtPointer callData);
        static void saveDOMAINCB(void *object, char *fname);
        void saveDOMAIN_cb(char *fname);

	static void setAllCB(Widget, XtPointer, XtPointer);
	void setAll_cb();

	static void clearAllCB(Widget, XtPointer, XtPointer);
	void clearAll_cb();

	void writeProbeFile(float x1, float x2, float y1, float y2); 
	void writeProbeObsFile(int x1, int x2, int y1, int y2); 
	void overlay_ts(int x1, int x2, int y1, int y2); 

	int invalidateFormulasDependingOnMe(void);

	LocalFileBrowser DomainBrowser_;
};

#endif

