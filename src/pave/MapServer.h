#ifndef ___MAP_SERVER_H___
#define ___MAP_SERVER_H___

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)MapServer.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_drawmap/SCCS/s.MapServer.h
 * Last updated: 12/15/97 16:24:05
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

///////////////////////////////////////////////////
//
// MapServer.h
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 8, 1996
//
///////////////////////////////////////////////////
//
//    MapServer Class
//
//        1. Creates map list
//        2. Adds new maps to list when necessary
//        3. Returns pointers to map line info when requested
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  960508  Implemented
//
///////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */


#include "LinkedList.h"
#include "vis_data.h"
#include "Map.h"
#include <stdio.h>


class MapServer 
{

   public:	
		MapServer();

		~MapServer();               	// Destructor

		int map_overlay(VIS_DATA *info, // returns 1 for success,
				float **xpts,   // 	   0 for failure
				float **ypts, 
				int **n,
       		 		int *npolyline, 
				char *mapName,
				char *message);
		Map *generateMap(VIS_DATA *info, char *mapName, char *message);
   protected:

   private:
			// see above Map.h for description of params
		void environmentVariableCheck(M3IOParameters *params);

		linkedList	*mapList_;

};


extern "C" 
{
	// C "Helper" functions as per Steve Fine's suggestion.
	//
	// These enable us to replace an original C map_overlay()
	// function with this map_overlay() function, which relies
	// on C++ objects rather than C code to do the guts of the
	// work.  In this way we don't have to modify any of the
	// code that calls map_overlay(), and yet we get the 
	// benefits of incorporating OO techniques with Lockheed/Martin's
	// new DrawMap library.

int init_projmaps(char *message);   // returns 1 if error, 0 for success

int projmap_overlay(VIS_DATA *info,   // returns 0 if error, 1 for success
                float **xpts,
                float **ypts,
                int **n,
                int *npolyline,
		char *mapName,		
                char *message);

int cleanup_projmaps(char *message);   // returns 1 if error, 0 for success
}


#endif // ___MAP_SERVER_H___
