#ifndef DRAWSCALE_H
#define DRAWSCALE_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)DrawScale.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.DrawScale.h
 * Last updated: 12/15/97 16:26:13
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
//	DrawScale.h
//	K. Eng Pua
//	May 12, 1993
//	
//////////////////////////////////////////////////////////////////////
//
//   DrawScale Class
//
//   DrawScale                                    Concrete
//        1. Computes the scale factor based on
//           the min and max of data values and
//           the width and height of the drawing
//           window
//        2. Translates values from world-
//           coordinates to window-coordinates
//        3. Translates values from window-
//           coordinates to world-coordinates
//
//////////////////////////////////////////////////////////////////////

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <stdio.h>

class DrawScale {
  protected:
	float	sx_;
	float	sy_;
	int	offset_left_;
	int	offset_right_;
	int	offset_top_;
	int	offset_bottom_;

  public:
	float gridx(int x);
	float gridy(int y);

	void scaleInit(float x1, float y1, float x2, float y2, 
			int wmaxx, int wmaxy,
			int offset_left = 100, int offset_right = 60,
			int offset_top = 100,  int offset_bottom = 60);
	float xmin_;
	float xmax_;
	float ymin_;
	float ymax_;

	DrawScale() {};		// empty constructor
	int scalex(float x);	
	int scaley(float y);
	float fscalex(float x);	
	float fscaley(float y);
};

#endif

