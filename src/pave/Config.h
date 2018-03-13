#ifndef ___CONFIG_H___
#define ___CONFIG_H___
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)Config.h	1.4
 *     Pathname: /tmp_mnt/pub/storage/edss/framework/src/pave/pave_include/SCCS/s.Config.h
 * Last updated: 14 Oct 1996 13:33:29
 *
 * COPYRIGHT (C) 1996, University of North Carolina at Chapel Hill
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

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// File:	Config.h 
// Author:	Steve Thorpe
// Date:	September 5, 1996
///////////////////////////////////////////////////////////////////////////////
//
//   Config Class
//
//////////////////////////////////////////////////////////////////////////////
//
//  Modification history:
//
//  960905 SRT Implemented
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

class Config 
	{
	private:

		char	fname_[256];
		char	ColorMapType_;
		float 	Legend_Max_;
		float 	Legend_Min_;
		char 	Legend_Format_[32];
		int 	Number_Labels_;
		int 	Invert_Colormap_;
		int 	Number_Tiles_;
		int 	red_[64];
		int 	green_[64];
		int 	blue_[64];
		int 	Save_MPEG_Files_;
		int 	Disable_Map_;
		int 	Smooth_Plot_;
		int 	Draw_Grid_Lines_;
		int 	Scale_Vectors_;

		unsigned int hasColorMapType_;
		unsigned int hasLegend_Max_;
		unsigned int hasLegend_Min_;
		unsigned int hasLegend_Format_;
		unsigned int hasNumber_Labels_;
		unsigned int hasInvert_Colormap_;
		unsigned int hasNumber_Tiles_;
		unsigned int hasColors_;
		unsigned int hasSave_MPEG_Files_;
		unsigned int hasDisable_Map_;
		unsigned int hasSmooth_Plot_;
		unsigned int hasDraw_Grid_Lines_;
		unsigned int hasScale_Vectors_;

		void 	initConfig(void);
		
	public:
		Config();
		virtual ~Config();
		int setFile(char *fname, char *estring);
		void print(FILE *fp);

		int	     hasFile()			{ return (fname_[0] != '\0'); } 
		unsigned int hasColorMapType() 		{ return hasColorMapType_; }
		unsigned int hasLegend_Max() 		{ return hasLegend_Max_; }
		unsigned int hasLegend_Min() 		{ return hasLegend_Min_; }
		unsigned int hasLegend_Format()		{ return hasLegend_Format_; }
		unsigned int hasNumber_Labels()		{ return hasNumber_Labels_; }
		unsigned int hasInvert_Colormap()	{ return hasInvert_Colormap_; }
		unsigned int hasNumber_Tiles()		{ return hasNumber_Tiles_; }
		unsigned int hasColors()		{ return hasColors_; }
		unsigned int hasSave_MPEG_Files()	{ return hasSave_MPEG_Files_; }
		unsigned int hasDisable_Map()		{ return hasDisable_Map_; }
		unsigned int hasSmooth_Plot()		{ return hasSmooth_Plot_; }
		unsigned int hasDraw_Grid_Lines()	{ return hasDraw_Grid_Lines_; }
		unsigned int hasScale_Vectors()		{ return hasScale_Vectors_; }

		char 	     getColorMapType()		{ return ColorMapType_; }
		float 	     getLegend_Max()		{ return Legend_Max_; }
		float 	     getLegend_Min()		{ return Legend_Min_; }
		char * 	     getLegend_Format()		{ return Legend_Format_; }
		int 	     getNumber_Labels()		{ return Number_Labels_; }
		int 	     getInvert_Colormap()	{ return Invert_Colormap_; }
		int 	     getNumber_Tiles()		{ return Number_Tiles_; }
		int *	     getred()			{ return red_; }
		int *	     getgreen()			{ return green_; }
		int *	     getblue()			{ return blue_; }
		int 	     getSave_MPEG_Files()	{ return Save_MPEG_Files_; }
		int 	     getDisable_Map()		{ return Disable_Map_; }
		int 	     getSmooth_Plot()		{ return Smooth_Plot_; }
		int 	     getDraw_Grid_Lines()	{ return Draw_Grid_Lines_; }
		int 	     getScale_Vectors()		{ return Scale_Vectors_; }
	};

#endif /* ifndef  ___CONFIG_H___ */
