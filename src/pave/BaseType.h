#ifndef BASETYPE_H
#define BASETYPE_H

/////////////////////////////////////////////////////////////
//
// Project Title: Environmental Decision Support System
//         File: @(#)BaseType.h	2.1
//     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.BaseType.h
// Last updated: 12/15/97 16:25:17
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
// BaseType.h
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 23, 1995
//
/////////////////////////////////////////////////////////////
//
// BaseType Class 
//
// Portions from Roger Sessions' "Class Construction in C
// and C++", copyright 1992 by Prentice-Hall
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT	950523	Implemented
//
/////////////////////////////////////////////////////////////


        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>


class baseType {

  public:

		/* NOTE:  putting "= 0" after each of these
		   virtual method prototypes would make them
		   PURE virtual methods - ie this class won't
		   define them, but any and all derived classes
		   would be REQUIRED to explicitly define them */

	virtual ~baseType();
	virtual int match(void *target);
	virtual void print(FILE *output);
	virtual char *getClassName();
};


#endif  // BASETYPE_H
