#ifndef BTS_DATA_H
#define BTS_DATA_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)BtsData.h	2.2
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.BtsData.h
 * Last updated: 12/02/99 14:18:25
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
// File:	BtsData.h
// Author:	K. Eng Pua and Steve Thorpe
// Date:	Jan 11, 1995 	
//////////////////////////////////////////////////////////////////////
//
//   BtsData Class
//
//   BtsData                                      Concrete
//        1. Contains the data items for
//           formula parser and data retriever
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950516  added setStepMinMaxIncr() routine
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
#include <assert.h>

#include "Util.h"

#include "bts.h"
extern "C" {
#include "vis_data.h"
}

class BtsData {
  public:
	BtsData();
	void initParserData();
	int initRetrieveData();
	int setPercentArray(int *domain_mask, int col_max, int row_max);
	int setStepMinMaxIncr(int scase, int smin, int smax, int sincr, char *eString);

	//--- INPUTS to parseFormula ---
	char	*formulaStr_;
	int	caseCount_;

	char	*caseList_; 		// a list of cases (ie data file
                                        // names) in order (a..?),
                                        // separated by commas 

        char	*hostList_;		//a list of hosts (ie
                                        // "todd.hpcc.epa.gov,ozone,
                                        // flyer.ncsc.org") separated by
                                        // commas, one for each case in
                                        // caseList 
        struct BusData *bd_;  		// needed to communicate with
                                        // the SW Bus; this should already
                                        // have been initialized with
                                        // initVisDataClient() 

	//--- MODIFIED by ParseFormula ---
	char	errString_[512];         // errormsg if any 
        char	postFixQueue_[512];      // postfix formula result 
        char	caseUsed_[512];          // "010" if ncases = 3 and only
               				 //	 case b in formula 
        char	whichUnit_[256];         // the units of the formula's output 

	int     dim_;                    // ndim of formula result's data 
        int	dateDay_;                // the day of formula result 
        int	dateMonth_;              // the month of formula result 
        int	dateYear_;               // the year of formula result 
        int	hourStart_;              // starting hour in GMT 
        int	mixCase_;                // 1 if can't put a time on the
					 //	starting hour, otherwise 0 
        int	imax_;
        int	jmax_;
        int	kmax_;


	// Type of integration
/*
	enum { NO_INT = 0 };
	enum { JK_INT = 1 };
	enum { IK_INT = 2 };
	enum { IJ_INT = 3 };
	enum { ALL_INT = 4 };
	enum { TIME_INT = 5 };
	enum { ZvsT_INT = 10 };
*/

	//--- INPUT to retrieveData ---

	int	selected_step_;		// what time step we are getting data
					//	for? NOTE: This is 1 based.
					// 	This is used as as offset to
					//	the step_min for each case.
					//	This matters only for slice_type's
					//	XYSLICE, YZSLICE, XZSLICE, and XYZSLICE.
	int	integration_;		// should take value in the enum above;	
	float	*thickValues_;		// array of kmax_ floats indicating the
					//	weights to each layer

	int	*whichLevel_;		// array of kmax_ integers, each should be
					//	non-zero if that level is in the
					//	domain, otherwise 0
	int	use_floor_;
	float	floorCut_;
	char	*percents_;		// array indicating which cells are in the
					//	domain and which are turned off.
					//	size = i * jmax_ + j.
					//	For a cell completely in the domain
					//	the percent's value should be 100
					//	For a cell completely NOT in the
					//	domain, it should be 0.

	int	selected_col_;		// which column selected, 1-based
	int	selected_row_;		// which row selected, 1-based
	int	selected_level_;	// which level selected, 1-based


	//--- INPUTS to retrieveData, but possibly modified
	//	by retrieveData

	int	*step_min_;		// one step_min for each dataset listed in
					//	caseList_ argument.  This will 
					//	probably be set in the UI to clamp
					//	down to time periods of interest.
	int	*step_max_;		// one step_max for each dataset listed in
					//	caseList_ argument
	int	*step_incr_;		// one step_incr for each dataset listed in
					//	caseList_argument
	int 	slice_type_;		// should be one of:
					// XYSLICE - selected_level_ at step hourf
					// YZSLICE - selected_col_ at step hourf
					// XZSLICE - selected_row_ at step hourf
					// XYZSLICE - all levels at step hourf
					// XYTSLICE - selected levels for steps
					//	hrMin_ through hrMax_
					// YZTSLICE - selected_col_ for steps
					//	hrMin_ through hrMax_
					// XZTSLICE - selected_row_ for steps
					//	hrMin_ through hrMax_
					// XYZTSLICE - all levels for steps
					//	hrMin_ through hrMax_

	int	hrMin_;			// first timestep (if time series data)
					//	This is used as an offset to the
					//	step_min for each case. So in effect
					//	step_min[] and step_max[] clamp a 
					//	dataset down once, then hrMin_ and 
					//	hrMax_ clamp it down again, within
					//	the step_min_ and step_max_.
	int	hrMax_;			// last timstrp (if time series data); this
					//	may be modified to the highest
					//	value if data files don't go up
					//	this high

	//--- MODIFIED by retrieveData

	float	*tsdata_;		// if integration == TIME_INT or ZvsT_INT,
					//	then the resulting data will be
					//	stuffed here
					// 	Space required:
					//	   hrMin_ - hrMin_ + 1
					//	    (for time series)
					//	or
					//	   kmax_ * (hrMax_ - hrMin_ + 1)
					//	    (for ZvsT) 

	float	tot_value_;		// if integration == ALL_INT then this
					//	will receive the result.

	VIS_DATA	vdata_;
};


#endif
