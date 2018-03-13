#ifndef READVISDATA_H
#define READVISDATA_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)ReadVisData.h	2.2
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.ReadVisData.h
 * Last updated: 12/02/99 10:56:04
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
//	ReadVisData.h
//	K. Eng Pua
//	Copyright (C)
//	Nov 23, 1994
//	
//////////////////////////////////////////////////////////////////////
//
//      Modification history
//
//      SRT 950911 added getUnits() routine
//      SRT 960509 added code to integrate DrawMap lib (see #define USE_OLDMAP)
//
//////////////////////////////////////////////////////////////////////
//
//   ReadVisData Class
//
//   ReadVisData                                  Concrete
//        1. Reads in gridded data from file and
//           allocate data arrays
//
//        or
//           access gridded data passed in by
//           VIS_DATA structure
//        2. Read map coordinates into data
//           array
//        3. Stores other map information into
//           data structure
//
//////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include "MapServer.h" // for DrawMap library stuff

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include "vis_data.h"
#include "vis_proto.h"
#include "visDataClient.h"
}
#include "bts.h"


typedef char str20[20];

class ReadVisData {
  public:

	ReadVisData(char *drawtype, BusData *ibd, VIS_DATA *vdata);
	virtual ~ReadVisData(); // 960918 added SRT for memory management

// SRT 960509 	ReadVisData(char *tilefile, char *drawtype, 
// SRT 960509 		int selected_species);
// SRT 960509 
// SRT 960509 	ReadVisData(char *tilefile, char *drawtype);  

	int getTimeMax() const { return (step_max_-step_min_+1); };

	VIS_DATA	*info; 
	VIS_DATA	*tmp_info;
	BusData		*bd;

        int     col_min_;
        int     col_max_;
        int     row_min_;
        int     row_max_;
        int     level_min_;
        int     level_max_;
        int     step_min_;
        int     step_max_;


        char    *title1_;
        char    *title2_;
        char    *title3_;
        char    *xtitle_;
        char    *ytitle_;

        str20   plotformatX_;
        str20   plotformatY_;

#ifdef USE_OLDMAP
	/* For Lambert and UTM maps */
	enum { MAP_MAXPTS_ =  250000 };
	float 	map_x_[MAP_MAXPTS_];
	float	map_y_[MAP_MAXPTS_];
	int	map_n_[MAP_MAXPTS_];
	int	map_npolyline_;
#else
	float   *map_x_;
        float   *map_y_;
        int     *map_n_;
	int	map_npolyline_;
	char	mapName_[256];
#endif

	enum { MAXNAME = 20 }; 
	enum { MAX_PATH_LEN = 256 };

	char *getUnits(void); // added 950911 SRT

	void setTileData();
	void setMapData();

   protected:
	char	message_[256];

	int	species_;
	char	*drawtype_;

	int	offset_left_;
	int	offset_right_;
	int	offset_top_;
	int	offset_bottom_;
/*
	int	_plot_left;
	int	_plot_top;
*/

	char TILE_DATA_FILE[MAX_PATH_LEN];
	char ARCINFO_MAP_1[MAX_PATH_LEN];
	char ARCINFO_MAP_2[MAX_PATH_LEN];


	float GRID_X_MIN_;
	float GRID_X_MAX_;
	float GRID_Y_MIN_;
	float GRID_Y_MAX_;
	float CLIP_GRID_X_MIN_;
	float CLIP_GRID_X_MAX_;
	float CLIP_GRID_Y_MIN_;
	float CLIP_GRID_Y_MAX_;

  private:
	FILE *fp_;
	void initialize();

};


#endif
