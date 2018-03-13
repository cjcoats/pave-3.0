#ifndef LINK_H
#define LINK_H

/////////////////////////////////////////////////////////////
//File: $Id: Link.h 85 2018-03-13 13:17:36Z coats $
//
// Project Title: Environmental Decision Support System
//         File: @(#)Link.h	2.1
//     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.Link.h
// Last updated: 12/15/97 16:26:38
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
// Link.h
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 23, 1995
//
/////////////////////////////////////////////////////////////
//
// Link Class 
//
// Portions from Roger Sessions' "Class Construction in C
// and C++", copyright 1992 by Prentice-Hall
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT	950523	Implemented
// SRT	960513	Changed link to linkp to get around unistd.h problem
//
/////////////////////////////////////////////////////////////


        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include "BaseType.h"


class linkp {
  public:
	linkp(baseType *newContents);

	/* NOTE:  the following data items could have (should have?)
	   been implemented as private, but Roger Sessions chose
	   not to, and I am just using his code basically as is - SRT*/

	/* SRT 951122 link*/ void *next;
	/* SRT 951122 link*/ void *previous;
	baseType *contents;
};


#endif  // LINK_H
