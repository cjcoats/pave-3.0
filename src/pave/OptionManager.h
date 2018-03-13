#ifndef OPTIONMANAGER_H
#define OPTIONMANAGER_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)OptionManager.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.OptionManager.h
 * Last updated: 12/15/97 16:27:51
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
// File:	OptionManager.h
// Author:	K. Eng Pua
// Date:	April 25, 1995 	
//////////////////////////////////////////////////////////////////////
//
//    OptionManager Class
//
//    OptionManager                               Concrete
//        1. Creates/Posts an option dialog
//        2. Sets the range for a given scale
//           widget
//        3. Obtains the current value of a
//           selected scale widget
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950516  verified getHourMin and getHourMax 
// 
//////////////////////////////////////////////////////////////////////


#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <Xm/Xm.h> 
#include "assert.h"
#include <X11/Intrinsic.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/Scale.h>
#include <stdio.h>


class OptionManager {
  public:
	OptionManager(Widget parent);

	void postOptionDialog();
	int getLevel() const { return selected_level_; }
	int getRow() const { return selected_row_; }
	int getColumn() const { return selected_col_; }
	int getHourMin() const { return hourMin_; }  
	int getHourMax() const { return hourMax_; } 

	void setLevel_hi(int valu);
	void setRow_hi(int valu);
	void setColumn_hi(int valu);

  private:
	Widget		option_dialog_;

	Widget		parent_;
	Widget		selected_level_scale_;
	Widget		selected_row_scale_;
	Widget		selected_col_scale_;
	Widget		hourMin_scale_;
	Widget		hourMax_scale_;
	Widget		apply_;

	int		selected_level_;
	int		selected_row_;
	int		selected_col_;
	int		hourMin_;
	int		hourMax_;


	void createOptionDialog();

	static void selected_level_scaleCB(Widget, XtPointer, XtPointer);
	void selected_level_scale_cb();

	static void applyCB(Widget, XtPointer, XtPointer);
	void apply_cb();

	static void closeCB(Widget, XtPointer, XtPointer);
	void close_cb();
};


#endif
