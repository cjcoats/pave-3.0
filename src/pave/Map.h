#ifndef ___MAP_H___
#define ___MAP_H___

/////////////////////////////////////////////////////////////
//
// Project Title: Environmental Decision Support System
//
//         File: @(#)Map.h	2.1
//     Pathname: /env/proj/archive/edss/src/pave/pave_drawmap/SCCS/s.Map.h
// Last updated: 12/15/97 16:23:48
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
// Map.h
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 8, 1996
//
/////////////////////////////////////////////////////////////
//
//   Every Map Object:
//
// o Must supply its (in)validity when reqested 
//
// o Must retrieve and supply a copy of its data when requested	
//
// o Needs to request info from Lockheed/Martin's DrawMap library
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT	960508	Implemented
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
#include "MapUtilities.h" 
#include <stdio.h>

/*
NOTE:	18 doubles are expected as parameters within the
	M3IOParameters argument.  The first 12 are defined 
     	as per Carlie Coats' Models-3 IO/API documentation - see

       	  file:/pub/storage/xcc/work/m3io/H.GRIDS.html#horiz
GDTYPE
P_ALP
P_BET
P_GAM
XCENT
YCENT
NROWS
NCOLS
XORIG
YORIG
XCELL
YCELL

	The next four are used to "pre-clip" the map file down to
	a more manageable size.  Typical values to use for this
	in order to clip down to the US domain are indicated.
	
MINLAT	  20.0
MINLON	-125.0
MAXLAT	  50.0
MAXLON	 -60.0

	The next two are used to specify the ellipse used for
	the globe.  Typical values to use for these are (for
	data based on MM5, such as MCIP, RADM, etc):
ELLIPSE	         0 (sphere)
RADIUS     6370997 (meters)

	or for other files, the following is presumed to be
	the most accurate representation:
ELLIPSE	MERIT_1983 (see MapProjections.h for a list of the ellipses) 
RADIUS           0 
*/

class Map:public linkedList 
{

   public:
				      	// Constructor 
        Map	(
		const char *mapFileName,
		const M3IOParameters* parameters
 	        );

        ~Map();           		// Destructor

	int match(void *target); 	// override baseType's 
                                        // virtual match() 

	char *getClassName(void);       // override baseType's 
                                        // virtual getClassName()

        void print(FILE *output); 	// override linkedList's print()

	int isMapValid 			// is this a valid map?
		       (char *estring);	// error msgs will be written here

	float *get_xpts(void)         	{ return xpts_; }

	float *get_ypts(void)         	{ return ypts_; }

	int   *get_npts(void)         	{ return npts_; }

	int    get_npolyline(void)      { return npolyline_; }

        void    copyParameters(M3IOParameters *);

   protected:

   private:

	M3IOParameters	parameters_;

	char	*mapFileName_;

	float	*xpts_;

	float	*ypts_;

	int	*npts_;

	int	npolyline_;

	enum Validity { unsure_or_no = 0, yes } validMap_;

};


#endif  // ___MAP_H___
