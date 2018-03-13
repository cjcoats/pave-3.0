#ifndef RUBBERBAND_H
#define RUBBERBAND_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)RubberBand.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.RubberBand.h
 * Last updated: 12/15/97 16:28:05
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
// File:	RubberBand.h 
// Author:	K. Eng Pua
// Date:	Feb 1, 1995
///////////////////////////////////////////////////////////////////////////////
//
//   RubberBand Class
//
//   RubberBand                                  Abstract
//        1. Enables zooming and probing of
//           selected plot
//        2. Provides visual effects of rubber-
//           banding using Xor GC
//        3. Writes probe data to file and
//           displays them on dialog box
//        4. Supports zoom data structure
//        5. Creates zoom dialog box
//        6. Creates probe dialog box
//
///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "DrawScale.h"
#include "Util.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

class RubberBand : public Util {
   public:
	RubberBand() { };  

	void initRubberBand(DrawScale *s, Widget parent);  

	virtual void timeSeriesProbe(int x1, int x2, int y1, int y2); 

   private:

	typedef struct {
		int             parent;		// parent zoom frame 

		float           GRID_X_MIN_;
		float           GRID_X_MAX_;
		float           GRID_Y_MIN_;
		float           GRID_Y_MAX_;
		float           CLIP_GRID_X_MIN_;
		float           CLIP_GRID_X_MAX_;
		float           CLIP_GRID_Y_MIN_;
		float           CLIP_GRID_Y_MAX_;

	} zoom_data,    *zoom_data_ptr;

	DrawScale	*s_;

        static void zoom_scaleCB(Widget, XtPointer, XtPointer);
        void zoom_scale_cb(int);

        static void zoom_closeCB(Widget, XtPointer, XtPointer);
        void zoom_close_cb();

        static void probe_closeCB(Widget, XtPointer, XtPointer);
        void probe_close_cb();

	GC createXorGC(Widget w); 

	void drawMyRubberBand(Widget w);

   protected:
        enum InteractMode { PROBE_MODE, ZOOM_MODE, MARKUP_MODE, TIME_SERIES_MODE, PROBE_OBS_MODE, TIME_SERIES_OBS_MODE, TIME_SERIES_BOTH_MODE }; // SRT 950915, ALT 091400, ALT 110901

        enum ProbeInteractMode { PROBE_TILE, PROBE_OBS };

        virtual void writeProbeFile(float x1, float x2, float y1, float y2) = 0; 
        virtual void writeProbeObsFile(int x1, int x2, int y1, int y2) = 0; 
        virtual void overlay_ts(int x1, int x2, int y1, int y2) = 0; 
	virtual void resize() = 0;

	virtual void adjustZoomDialogPosition(void); // added 950913 SRT

	enum		{ MAX_ZOOM_WINDOWS = 80 };
	zoom_data	zoom_[MAX_ZOOM_WINDOWS];

	int		interact_mode_;
	int		interact_submode_;

	Widget		parent_widget_;
	Widget		status_;
	GC		xorgc_;

	int		startx_, lastx_, starty_, lasty_;

	Widget		probe_dialog_;
	Widget		probe_text_;

	Widget		zoom_dialog_;
	Widget		zoom_scale_;
	Widget		close_;

	int		num_zooms_;
	int		curr_zoom_;
	char		probefilename_[100];

	int		iCellDown_, jCellDown_; // SRT 950404

	void createProbeDialog();
	void createZoomDialog();

	void start_rb(Widget w, XEvent *event);
	void track_rb(Widget w, XEvent *event);
	void end_rb(Widget w, XEvent *event);

	static void start_rb_EV(Widget		w,
				XtPointer	clientData,
				XEvent		*event,
				Boolean		*dispatch);

	static void track_rb_EV(Widget		w,
				XtPointer	clientData,
				XEvent		*event,
				Boolean		*dispatch);

	static void end_rb_EV(Widget		w,
				XtPointer	clientData,
				XEvent		*event,
				Boolean		*dispatch);

	void zoomin();
};

#endif
