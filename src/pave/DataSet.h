#ifndef DATASET_H
#define DATASET_H

/////////////////////////////////////////////////////////////
//
// Project Title: Environmental Decision Support System
//         File: @(#)DataSet.h	2.1
//     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.DataSet.h
// Last updated: 12/15/97 16:26:03
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
// DataSet.h
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 23, 1995
//
/////////////////////////////////////////////////////////////
//
// DataSet Class 
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT	950523	Implemented
// SRT  960416  Added getIncrSdateStimeNsteps()
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
#include "StepUI.h"
#include "bts.h"
#include "Domain.h"
#include "SpeciesServer.h"
#include "Level.h"
#include "Formula.h"
#include "AppInit.h"


class dataSet:public linkedList {

  public:

        dataSet(char *hname, char *pname, char *fname,
					// Constructor with hostname,
					// pathname, and filename specified
					// separately
	       linkedList *formulaList, // parent's formulaList
	       linkedList *domainList,  // parent's domainList
               linkedList *levelList,	// parent's levelList
               linkedList *dataSetList,	// parent's datasetList
               void *speciesS, 		// parent's speciesServer
	       AppInit *app,
	       Widget parent);		// to base position of UI on

        dataSet(char *longname, 	// alternate Constructor with
					// dataset specified in one string 
					// [<hostname>:][<pathname>/]<filename>
	       linkedList *formulaList, // parent's formulaList
	       linkedList *domainList,  // parent's domainList
               linkedList *levelList,   // parent's levelList
               linkedList *dataSetList,	// parent's datasetList
               void *speciesS, 		// parent's speciesServer
	       AppInit *app,
	       Widget parent);		// to base position of UI on

        virtual ~dataSet();      	// Destructor

	int  match(void *target); 	// override baseType's 
                                        // virtual match() 

	char *getClassName(void);       // override baseType's 
                                        // virtual getClassName()

        void print(FILE *output); 	// override linkedList's print()

	char *getFullName(void) 	{ return fullname_; }

	char *getHostName(void) 	{ return hostname_; }

	char *getFileName(void) 	{ return filename_; }

	char *getPathName(void) 	{ return pathname_; }

	char *getMapInfo(void) 		{ return info_.map_info; }

	int isValid(struct BusData *bdp, char *estring); 
					// is this a valid dataSet?

	int editDataSetSteps(void);	// edit the step_min, step_max,
					// and step for this dataSet

	int getNStepsInRange(void)	{ return step_max_-step_min_+1; }

	linkedList *getFormulaList(void) { return formList_; }

	int get_step_min(void)		{ return step_min_; }

	int get_step_max(void)		{ return step_max_; }

	int get_step_incr(void)		{ return 1; } // this is bogus for now SRT

	int get_nlevel(void)		{ return info_.nlevel; }

	int get_nrow(void)		{ return info_.nrow; }

	int get_ncol(void)		{ return info_.ncol; }

	int isObs(void)		        { return isObs_; }

	int getIncrSdateStimeNsteps	(int *incr, int *sdate, 
					int *stime, int *nsteps,
					int **sdates, int **stimes);

	VIS_DATA *get_vdata_ptr(void);

  private:

        void init(char *hname, char *pname, char *fname, 
					// hostname, pathname, filename
	     linkedList *formulaList,   // formulaList of its parent
	     linkedList *domainList,    // formulaList of its parent
             linkedList *levelList,     // parent's levelList
             linkedList *dataSetList,	// parent's datasetList
             void *speciesS,   // parent's speciesServer
	     AppInit *app,
	     Widget parent);		// to base position of UI on

	int  setHostName(char *hname); 	// modifies existing memory
					// returns 0 if success

	int  setFileName(char *fname); 	// modifies existing memory
					// returns 0 if success

	int  setPathName(char *pname); 	// modifies existing memory
					// returns 0 if success

	int  setFullName(char *pname); 	// modifies existing memory
					// returns 0 if success

	int  updateMetaData(struct BusData *bdp, char *estring); 
					// modifies existing memory
                                        // returns 0 if success

	int updateFullName(void);	// returns 0 if success;

	int step_min_;			// 1 based
	int step_max_;			// 1 based
	int step_incr_;			// 1 based

	int isObs_;                     // flag for monitor data (0=no, 1=yes)

	char fullname_[768];		
	char hostname_[256];		// can be empty - if so then local
	char pathname_[256];		// if non-null terminate with '/'
	char filename_[256];		

	VIS_DATA info_;			// to hold MetaData about the file

	enum dataSetValidity { unsure_or_no = 0, yes } validDataSet_;

	linkedList *formList_;   
	linkedList *domainList_;
        linkedList *levelList_;
        linkedList *datasetList_;
        void *speciesServer_;

	StepUI *stepDialogBox_;

        AppInit         *app_;

	Widget parent_;
};


                // C "Helper" function as per Steve Fine's suggestion
                // to get around these annoying ;) flaws of the C++ language;
                // this should be declared "static" within DataSet.h, and
                // then a pointer to it can be passed to other routines.

static	void modifiedTheseStepsCB(void *); // run this if user subselects
					   // on the dataset


#endif  // DATASET_H
