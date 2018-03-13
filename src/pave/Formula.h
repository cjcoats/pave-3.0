#ifndef FORMULA_H
#define FORMULA_H

/////////////////////////////////////////////////////////////
//
// Project Title: Environmental Decision Support System
//         File: @(#)Formula.h	2.2
//     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.Formula.h
// Last updated: 03/21/00 17:18:05
//
// Made available by MCNC and the Carolina Environmental Program of UNC Chapel
// Hill under terms of the GNU Public License.  See gpl.txt for more details.
//
// See file COPYRIGHT for license information on this and supporting software.
//
// Carolina Environmental Program
// University of North Carolina at Chapel Hill
// 137 E. Franklin St.
// Chapel Hill, NC 27599-6116
//
// See http://www.cep.unc.edu, http://www.cmascenter.org, http://bugz.unc.edu
//
/////////////////////////////////////////////////////////////
//
// Formula.h
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 26, 1995
//
/////////////////////////////////////////////////////////////
//
//   Every Formula Object:
//
// o Must supply its (in)validity when reqested 
//
// o Must retrieve and supply its data when requested	
//
// o Needs to request info from the list of available DataSet,
//   Level, and Domain objects in order to determine (in)validity
//   of itself and its data
//
// o Needs to access its FormulaServer object if it becomes invalid,
//   so the FormulaServer can delete it from its dialog box
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT	950526	Implemented
// SRT	950831	Added association with FormulaServer objects
// SRT  960416  Added updateCaseNamesAndTimes()
//
/////////////////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */


        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include "LinkedList.h"
#include "DataSet.h"
#include "bts.h"
#include "StepUI.h"
#include "Level.h"
#include "FormulaServer.h"


class Formula:public linkedList {

  public:
				      	// Constructor 

        Formula(char *formulaP,       	// infix formula typed in by user 

                struct BusData *bdP, 	// needed to communicate with
                                        // the SW Bus; this should already
                                        // have been initialized with
                                        // initVisDataClient() 
		
		linkedList *dataList,	// the list of DataSet objects to
					// refer to when parsing a formula

		linkedList *domList,    // the list of Domain objects to
					// determine which cells are on
					// for a formula

		linkedList *levList,    // the list of Levels objects to
					// determine which layers are on
					// for a formula

		Widget parent,		// parent of this puppy

		void *formulaS, 	// FormulaServer using this puppy

		char *estring		// for error msgs
 	       );

        virtual ~Formula();           	// Destructor

	int match(void *target); 	// override baseType's 
                                        // virtual match() 

	char *getClassName(void);       // override baseType's 
                                        // virtual getClassName()

        void print(FILE *output); 	// override linkedList's print()

	int isFormulaValid 		// is this a valid formula?
	     (char *estring);	// error msgs will be written here

        int dataSetListWasChanged       // to notify of changed dataSet list
		(int *, char *estring);	// error msgs will be written here

        int dataSetStepsWereChanged     // to notify of changed step-min &
	  	(char *fullname, 	// step-max settings in a dataset
		 int newmin,
		 int newmax);

        int levelsWereChanged           // to notify of a changed Level object
	  	(int nlayers);

        int domainWasChanged            // to notify of a changed Domain object
	  	(int ni, 		
		 int nj,
		 char *mapinfo);

        VIS_DATA *get_VIS_DATA_struct(char *estring, int slice_type); // grab an actual copy of the data

        float *get_time_series_data(char *estring); // grab a copy of time series data for this formula

        int editFormulaSteps(void);     // edit the step_min and step_max
                                        // for this formula

	void invalidateThisData(void);	// sets formulaDataValidity
					// to unsure_or_no and clears
					// out any info struct

	char *getFormulaName(void)	{ return infixFormula_; }

        int get_ncol(void)              { return IMAX_; }

        int get_nrow(void)              { return JMAX_; }

	int getFormulaNlevel(void)	{ return KMAX_; }

	int get_selected_level(void);

	int get_selected_row(void);

	int get_selected_column(void);

	int get_cellrange(int *xmin, int *xmax, int *ymin, int *ymax);

	int get_levelrange(int *zmin, int *zmax);

	int get_nsteps(void)		{ return hrMax_-hrMin_+1; }

	int get_hrMin(void)		{ return hrMin_; }

	int get_hrMax(void)		{ return hrMax_; }

	int get_selected_step(void)	{ return selectedStep_; }

	int set_selected_step(int);	// 1 based added 950909 SRT

	int set_selected_level(int);	// 1 based added 950909 SRT

	int set_levelRange(int, int);	// 1 based added 951230 SRT

	int set_hr_min(int);	        // 0 based added 950909 SRT

	int set_hr_max(int);	        // 0 based added 950909 SRT

	int getMapInfo(char *s, char *estring);	// fills s with map_info string 
						// for this formula

	char *getUnits(void)		{ return whichUnit_; }

	char *getSelectedCellRange(void);

	char *getTimeMinString(void);

	char *getTimeMaxString(void);

	void setUnits(char *u);

	char *getCasesUsedString(void); // this allocates space using malloc

	int isObs(void);

	VIS_DATA info_;			// to hold MetaData about 
					// as well as copies of the
					// the actual data 
  private:

	int updateCaseNamesAndTimes(char *estring); // SRT 960416

	void invalidateThisFormula(void);// sets formulaValidity
					// to unsure_or_no, frees up
					// out char *caseList_, 
					// and invalidates this data;

	int isFormulaDataValid(void);   // is there valid data for this
					// formula already here ?

	int getAndSetMaxHourRange	// sets range of hrMin_, hrMax_,
		(char *estring);	// and maxNumHours_ to be biggest 
					// possible for this formula

	char postFixQueue_[512];  	// postfix formula result 

	char caseUsed_[MAX_INT];	// We can't have more 
					// dataSets than the max
					// number of lists available

	int caseNhours_[MAX_INT];	// For each case used,
					// what is that case's
					// step_max - step_min +1 ?

	int caseStepMin_[MAX_INT];	// For each case used,
					// what is that case's step_min ?

	int caseStepMax_[MAX_INT];	// For each case used,
					// what is that case's step_max ?

	int caseStepIncr_[MAX_INT];	// For each case used,
					// what is that case's step_incr ?

	char whichUnit_[100];		// what units is this case?

	char *userUnit_;		// for storing user defined unit string

	int dim_;			// ndim of formula result's data

        int dateDay_;       		// the day of formula result 

        int dateMonth_;     		// the month of formula result

        int dateYear_;      		// the year of formula result 

        int hourStart_;     		// starting hour 

        int mixCase_;       		// 1 if can't put a time on the
					// starting hour, otherwise 0 

        int IMAX_;           		// IMAX for formula

        int JMAX_;           		// JMAX for formula

        int KMAX_;           		// KMAX for formula

	enum Validity { unsure_or_no = 0, yes } 

			validFormula_,	

			validFormulaData_;

//	VIS_DATA info_;			// to hold MetaData about 
					// as well as copies of the
					// the actual data 

	int hrMin_;			// 0 based

	int hrMax_;			// 0 based

	int maxNumHours_;		// for this formula, what is
					// the max number of hours for us?

	int selectedStep_;		// 1 based

	StepUI *stepDialogBox_;		// to track the min/max steps for
					// this formula

	int ncases_;			// how many DataSets are available?

	char *infixFormula_;		// the formula typed in by the user

	char *caseList_;		// a list of full dataset names

	char *caseOnlyList_;		// a list of dataset names - filenames only

	char *hostOnlyList_;		// a list of full dataset names - hostnames only

        struct BusData  *bd_; 		// needed to communicate with Bus

	int *whichLevel_;		// denotes which levels are on and which are off

	linkedList      *datasetList_;	// point to list of its parent

	linkedList      *domainList_;	// point to list of its parent

	linkedList      *levelList_;	// point to list of its parent

        void   		*formulaS_;	// ptr to FormulaServer using this pup

        Widget 		parent_;

	int		use_floor_;

	float		floorCut_;

	int 		slice_type_;
};



		// C "Helper" function as per Steve Fine's suggestion
		// to get around these annoying ;) flaws of the C++ language;
		// this should be declared "static" within Formula.h, and
		// then a pointer to it can be passed to other routines.

static void modifiedTheseStepsCB(void *); // run this if user subselects
                                          // on the formula's range



#endif  // FORMULA_H
