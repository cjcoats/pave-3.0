#ifndef READCOMBODATA_H
#define READCOMBODATA_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)ReadComboData.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.ReadComboData.h
 * Last updated: 12/15/97 16:27:59
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
//	ReadComboData.h
//	K. Eng Pua
//	Copyright (C)
//	Jan 28, 1995
//////////////////////////////////////////////////////////////////////
//
//   ReadComboData Class
//
//   ReadComboData : ComboData                    Concrete
//        1. Reads in header segment of 2D
//           data file
//        2. Reads in control segment
//        3. Reads in option segment
//        4. Counts the number of data sets
//
//////////////////////////////////////////////////////////////////////

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ComboData.h"

class ReadComboData : public ComboData {
  public:
	ReadComboData(char *inpfile, char *drawtype);
	virtual ~ReadComboData();

  private:
	FILE *fp_;

  protected:
	enum { MAXNAME = 20 };
	enum { MAX_BUFF_LEN = 256 };
	enum PlotStyle { POINT_ONLY, LINE_ONLY, POINT_LINE };


	virtual void readData(char *infile);
        void readHeaders();
        void readControls();
        void readOptions();

	void countDataSets();
	void readBar();
};

#endif
