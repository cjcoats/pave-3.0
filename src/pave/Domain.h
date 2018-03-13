#ifndef DOMAIN_H
#define DOMAIN_H

/////////////////////////////////////////////////////////////
//
// Project Title: Environmental Decision Support System
//         File: @(#)Domain.h	2.1
//     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.Domain.h
// Last updated: 12/15/97 16:26:06
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
// Domain.h
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 26, 1995
//
/////////////////////////////////////////////////////////////
//
//   Every Domain Object:
//
// o When requested, must provide a UI which manages a 
//   dynamic "mask" of its currently selected cells
//
// o Must notify each of the Formulas when its currently
//   selected cells change
//
// o Must retrieve and supply its data when requested
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT	950526	Implemented
// SRT  951107  Added setRange()
// SRT  960525  Added saveDomain() and loadDomain()
//
/////////////////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */


#include "LinkedList.h"
#include "DomainWnd.h"
#include "bts.h"
#include "ReadVisData.h"
#include "DataSet.h"
#include "AppInit.h"


class Domain:public linkedList {

  public:
				      	// Constructor 

        Domain(linkedList *formulaList, // points to the list of
					// Formula objects this
					// Object may need to 
					// interact with

              linkedList *dataSetList,  // points to the list of
					// DataSet objects this
					// Object may need to 
					// interact with

	      int ni,			// Number columns of full domain

	      int nj,			// Number rows of full domain

	      char *mapInfo,		// map location info string

	      AppInit *app,		//

              char *estring);           // for error msgs



        virtual ~Domain();           	// Destructor

	int match(void *target); 	// override baseType's 
                                        // virtual match() 

	char *getClassName(void);       // override baseType's 
                                        // virtual getClassName()

        void print(FILE *output); 	// override linkedList's print()

	int showUI(char *estring);	// brings up a UI which allows
					// the user to modify the layers
					// selected

	char *getCopyOfPercents(char *estring);	 // to supply a copy of its data

	int setRange(int xmin, int ymin, int xmax, int ymax);

        int getRange(int *xmin, int *ymin, int *xmax, int *ymax);

	int saveDomain(char *fname, 	// file to save domain to
			char *estring); // for error msgs
					// returns 1 if error, otherwise 0

	int loadDomain(char *fname, 	// file to read domain from
			char *estring); // for error msgs
					// returns 1 if error, otherwise 0

  private:

        int verify_rvd(char *tstring);

	linkedList *formulaList_;	// point to list of its parent

	linkedList *datasetList_;	// point to list of its parent

	int IMAX_;			// how many columns are here?

	int JMAX_;			// how many rows are here?

	char *mapinfo_;			// map location info string

	char *whichCells_;		// are they on or off?

	ReadVisData *rvd_;		// to hold info about
					// datasets using this domain

	DomainWnd *dw_; 		// to hold domain UI info

	AppInit *app_;			//

};


#endif  // DOMAIN_H
