#ifndef STEP_UI_H
#define STEP_UI_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)StepUI.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.StepUI.h
 * Last updated: 12/15/97 16:28:21
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

//////////////////////////////////////////////////////////////////////
// File:	StepUI.h
// Author:	Steve Thorpe
// Date:	June 5, 1995 	
//////////////////////////////////////////////////////////////////////
//
//    StepUI Class
//
//    StepUI                               Concrete
//        1. Creates/Posts an step edit dialog
//        2. Sets the range for a given step min/max
//        3. Obtains the current value of a selected step widgets
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950605  Implemented
// SRT  960412  Added Julian start date/time
// SRT  960416  Added getFirst_sdate_InRange(), getFirst_stime_InRange(), 
//		      getNStepsInRange(), getCurrentJulianTimeCutoffs(),
//		      tryToSetJulianTimeCutoffs()
// SRT  960419  Added apply button
// SRT  960424  Added updateValues()
// SRT  960502  Added getFirst_Offset_InRange()
// 
//////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif


#include <stdio.h> 
#include <stdlib.h> 
#include <Xm/Xm.h> 
#include <X11/Intrinsic.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/Scale.h>
#include <assert.h>

#include "vis_proto.h"


class StepUI {

  public:
						// Constructor

	StepUI	(
		char *title,			// title of dialog box
		Widget parent,			// to base position on
		char *minLabel,			// "Step Min", "Layer Min", etc
		char *maxLabel,			// "Step Max", "Layer Max", etc
		int rangeMin,			// 0, 1, etc
		int rangeMax,			// 24, 72, etc
        	int *sdate,    /* Julian start date for time steps 960412 SRT */
        	int *stime,    /* Julian start time for time steps 960412 SRT */
		int *currentMinP,		// current Min
		int *currentMaxP,		// current Max
		void (*valsModifiedParentCB)(void *), // call when vals change
		void *obj,			// to pass this in
		char *estring			// to hold error msgs
		);

	virtual ~StepUI(void);          // Destructor

	int updateValues	(
		char *minLabel,			// "Step Min", "Layer Min", etc
		char *maxLabel,			// "Step Max", "Layer Max", etc
		int rangeMin,			// 0, 1, etc
		int rangeMax,			// 24, 72, etc
        	int *sdate,    /* Julian start date for time steps 960412 SRT */
        	int *stime,    /* Julian start time for time steps 960412 SRT */
		int *currentMinP,		// current Min
		int *currentMaxP,		// current Max
		char *estring			// to hold error msgs
		);

        void postOptionDialog();

        void setMinMax( int currentMin, // 0, 1, etc
			int currentMax);// 24, 72, etc

	int	getFirst_sdate_InRange(void); 	// returns Julian start date 
					        // for 1st time step in range

	int	getFirst_stime_InRange(void); 	// returns Julian start time 
					        // for 1st time step in range

	int	getFirst_Offset_InRange(void); 	// returns offset to 
						// 1st time step in range
						// (*0* based)

	int	getNStepsInRange(void); 	// returns Num steps in range

	void getCurrentJulianTimeCutoffs	
                (int *dateMin, int *timeMin, int *dateMax, int *timeMax);

	void tryToSetJulianTimeCutoffs
                (int dateMin, int timeMin, int dateMax, int timeMax);

  private:

	char 		title_[512];
	Widget		parent_;
	char		minLabel_[255];
	char		maxLabel_[255];
	int		rangeMin_;
	int		rangeMax_;
	int		*currentMinP_;
	int		*currentMaxP_;
	void 		(*parent_OK_CB_)(void *);
        int		currentMin_;
        int		currentMax_;

	Widget		step_dialog_;
	Widget		currentMin_scale_;
	Widget		currentMax_scale_;
	Widget		ok_;
	Widget		cancel_;
	Widget		apply_;

	void		*obj_;

	int		*sdate_, 
			*stime_;

	void createOptionDialog();

	static void okCB(Widget, XtPointer, XtPointer);
	void ok_cb();

	static void minOrMaxSliderMovedCB(Widget, XtPointer, XtPointer);
	void minOrMaxSliderMoved_cb();

	static void cancelCB(Widget, XtPointer, XtPointer);
	void cancel_cb();

	static void applyCB(Widget, XtPointer, XtPointer);
	void apply_cb();

	void updateStepWidgets(void);

	void setMinMaxLabel(int min_or_max);
};


#endif // STEP_UI_H
