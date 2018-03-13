#ifndef COMBOWND_H
#define COMBOWND_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)ComboWnd.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.ComboWnd.h
 * Last updated: 12/15/97 16:25:57
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
// File:	ComboWnd.h 
// Author:	K. Eng Pua
// Date:	Jan 28, 1995
///////////////////////////////////////////////////////////////////////////////
//
//   ComboWnd Class
//
//   ComboWnd : DrawWnd, Menus, RubberBand        Abstract
//        1. Creates a top-level window for 2D    Shell
//           plots
//        2. Supports a user interface that       DrawWnd
//           includes an application-specific
//           drawing area widget
//        3. Creates a pulldown menu at the top   Menus
//           of the window
//        4. Supports zooming and data probing    RubberBand
//
//////////////////////////////////////////////////////////////////////////////

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

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

#include "DrawWnd.h"
#include "Menus.h"
#include "RubberBand.h"
#include "ComboData.h"
#include "AppInit.h"
#include "DrawScale.h"

static char *patterns[] = {
   "slant_left",
   "slant_right",
   "vertical",
   "horizontal",
   "25_foreground",
   "75_foreground",
   "50_foreground"
};

class DrawScale;
class AppInit;


class ComboWnd : public DrawWnd, public Menus, public RubberBand {
   public:
	ComboWnd ( AppInit *app, char *name, char *drawtype, ComboData *combo,
		Dimension width, Dimension height,
		int exit_button_on,  // 1 means Exit button sensitive. Otherwise insensitive. 
		char **colornames, int numcolornames,
		char **in_patterns = patterns, int numpattern = 7 );

   protected:
	DrawScale	s;
	ComboData	*combo_;

	Widget		canvas_;
	Widget		animate_scale_;

	char		**patterns_;
	int		numpatterns_;

	void drawBarTics();

	virtual void createUI ( Widget );
	virtual void drawDetail() = 0; 
        virtual void drawLegend();
	virtual void legendFunc(int choice, int x, int y);

   private:

	enum { MAX_PATH_LEN = 256 };

	int	exit_button_on_;

	void	initComboWnd();

	static void resizeCB(Widget, XtPointer, XtPointer);
	void resize();

	static void redisplayCB(Widget, XtPointer, XtPointer);
	void redisplay(XExposeEvent *event);

	static void probeCB(Widget, XtPointer, XtPointer);
	void probe_cb();

	static void zoomCB(Widget, XtPointer, XtPointer);
	void zoom_cb();

	static void controlCB(Widget, XtPointer, XtPointer);
	void control_cb();

	static void exitCB(Widget, XtPointer, XtPointer);
	void exit_cb();

	void fillCanvasBackground(char *);
	int  colorIndex(float);

	static Boolean animateTileTrigger(XtPointer clientData);

};

#endif

