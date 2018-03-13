#ifndef LEVEL_H
#define LEVEL_H

/////////////////////////////////////////////////////////////
//
// Project Title: Environmental Decision Support System
//         File: @(#)Level.h	2.1
//     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.Level.h
// Last updated: 12/15/97 16:26:35
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
// Level.h
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 26, 1995
//
/////////////////////////////////////////////////////////////
//
//   Every Level Object:
//
// o When requested, must provide a UI which manages a 
//   dynamic "mask" of its currently selected levels
//
// o Must notify each of the Formulas when its currently
//   selected levels change
//
// o Must retrieve and supply its data when requested
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT	950526	Implemented
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
#include "bts.h"
#include "StepUI.h"
#include "Formula.h"


class Level:public linkedList {

  public:
				      	// Constructor 

        Level(linkedList *formulaList,  // points to the list of
					// Formula objects this
					// Object may need to 
					// interact with

	      int nlevels,     		// the number of layers this
					// object needs to manage

	      Widget parent,		// to base position of UI on

	      char *estring);		// for error msgs

        virtual ~Level();           	// Destructor


	int match(void *target); 	// override baseType's 
                                        // virtual match() 


	char *getClassName(void);       // override baseType's 
                                        // virtual getClassName()


        void print(FILE *output); 	// override linkedList's print()

	int showUI(void);		// brings up a UI which allows
					// the user to modify the layers
					// selected

	int *getCopyOfLevels(void);	// to supply a copy of its data

	int get_nLevels(void)		{ return KMAX_; }

	int get_min_level(void)		{ return minLevel_; }

	int get_max_level(void)		{ return maxLevel_; }

	int setMinAndMaxLevelTo(int);	// added 950909 SRT

	int setLevelRange(int, int);	// added 951230 SRT

	linkedList *get_formula_list(void) { return formulaList_; }

  private:

	linkedList *formulaList_;	// point to list of its parent

	int KMAX_;			// how many levels are here?

	int minLevel_;			// lowest layer turned on, 1 based

	int maxLevel_;			// highest layer turned on, 1 based

	StepUI *levelDialogBox_;	// a UI to edit the layers with

	Widget parent_;			// to base position of UI on

};


                // C "Helper" function as per Steve Fine's suggestion
                // to get around these annoying ;) flaws of the C++ language;
                // this should be declared "static" within Level.h, and
                // then a pointer to it can be passed to other routines.

static void modifiedTheseLevelsCB(void *); // run this if user subselects
                                           // on that Level'ss range



#endif  // LEVEL_H
