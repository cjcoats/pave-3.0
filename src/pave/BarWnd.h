#ifndef BARWND_H
#define BARWND_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)BarWnd.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.BarWnd.h
 * Last updated: 12/15/97 16:25:14
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

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// File:	BarWnd.h 
// Author:	K. Eng Pua
// Date:	Feb 2, 1995
///////////////////////////////////////////////////////////////////////////////
//
//   BarWnd Class
//
//   BarWnd : ComboWnd                            Concrete
//        1. Supports a specific function for
//           displaying bar plot
//
//////////////////////////////////////////////////////////////////////////////


#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "ComboWnd.h"

static char *bar_default_colornames[] = {
   "Red",
   "Blue",
   "Orange",
   "Yellow",
   "GreenYellow",
   "Cyan",
   "Green",
   "LimeGreen"
};



class BarWnd : public ComboWnd {
   public:
	BarWnd ( AppInit *app, char *name, char *drawtype, ComboData *combo,
		Dimension width, Dimension height,
		int exit_button_on = 1,  // 1 means Exit button sensitive. Otherwise insensitive. 
		char **colornames = bar_default_colornames, int numcolornames = 8);

   private:
	void drawBar();
	void drawDetail();
        void writeProbeFile(float x1, float x2, float, float);
        void writeProbeObsFile(int, int, int, int);
        void overlay_ts(int, int, int, int);

};

#endif

