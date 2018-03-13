#ifndef DRAWWINDOW_H
#define DRAWWINDOW_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)DrawWnd.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.DrawWnd.h
 * Last updated: 12/15/97 16:26:16
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
// DrawWnd.h 
// K. Eng Pua
// Copyright (C)
// Dec 12, 1994
///////////////////////////////////////////////////////////////////////////////
//
//    DrawWnd Class
//
//    DrawWnd : Shell                          Conrete
//        1. Creates a form widget as a
//           container.
//        2. Initializes graphics contexts and
//           character fonts
//        3. Supports low-level draw functions
//
//////////////////////////////////////////////////////////////////////////////
//
// 950728 SRT Added void drawTimeStamp(int currAnimate, int date, int time)

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <Xm/Xm.h>
#include "vis_proto.h"
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/Form.h>

#include "Shell.h"
#include "DrawScale.h"
#include "AppInit.h"

#define DEFAULT_FONT	"-*-helvetica-*-r-*-*-14-*-*-*-*-*-*-*"	
#define TITLE_FONT	"-*-times-*-r-*-*-24-*-*-*-*-*-*-*"	

class DrawWnd : public Shell {
   public:
        DrawWnd (AppInit *app, char *name, char *drawtype,
                Dimension width, Dimension height,
                char **colornames, int numcolornames,
		int bar_plot);

	~DrawWnd();

        void drawSetup(Widget draw_area, Drawable drw, GC gc, DrawScale *s);

	void drawTimeStamp(int t); 	// SRT 950804 	
	void drawTimeStamp(int currAnimate, int date, int time, char *tz);
	int newTitleFontSize(int size);
	int newSubTitleFontSize(int size);

   protected:
	DrawScale	*s_;
	AppInit		*app_;

	Widget		draw_;	// Form widget
	Display		*dpy_;
	Drawable	drw_;
	GC		gc_;
	char		**colornames_;
	int		numcolornames_;
	char 		*drawtype_;

	XFontStruct	*title_font_;
	XFontStruct	*subtitle_font_;
	XFontStruct	*def_font_;
	Dimension	width_;
	Dimension	height_;
	Pixmap		pix_;

	int getNamedPixel(char *colorname);

	virtual void drawFrame();

	virtual void drawXtics(int num_tics, char *formatY);
	virtual void drawYtics(int num_tics, char *formatX, int draw_on_right,int barwnd);
	virtual void drawYtics(int num_tics, char *formatX);

	virtual void drawTitle(XFontStruct *fontstruct, int offset, char *title);
	virtual void drawTitleY(XFontStruct *fontstruct, int offset, char *title);

	virtual void setForeground(char *);
	virtual void setBackground(char *);

	virtual void drawTitles(char *title1, char *title2, char *title3, char *xtitle, char *ytitle);
// SRT 950804 	virtual void drawTimeStamp(int t);
// SRT 950804   virtual void drawTimeStamp(int currAnimate, int date, int time);

	Widget		draw_area_;	// drawing area widget


   private:

	void draw_mcnc_text(void); 	// 961015 added SRT

	int bar_plot_; 			// 961017 added SRT
};

#endif
