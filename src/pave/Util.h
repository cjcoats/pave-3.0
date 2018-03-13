#ifndef UTIL_H
#define UTIL_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)Util.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.Util.h
 * Last updated: 12/15/97 16:28:37
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

/////////////////////////////////////////////////////////////
// Util.h
// K. Eng Pua
// Dec 19, 1994
/////////////////////////////////////////////////////////////
//
//   Util Class
//
//   Util                                         Concrete
//        1. Converts a file to a string
//
//
/////////////////////////////////////////////////////////////


#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif


#include <Xm/Xm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <Xm/MessageB.h>

class Util {
  public:
	Util ();

  protected:
	char *file2string(char * filename);
};


/////////////////////////////////////////////////////////////
//
//    Message Class
//
//    Message                                     Concrete
//        1. Create a popup dialog box
//        2. Display a message in the box
//        3. Destroy the dialog box
//
/////////////////////////////////////////////////////////////

class Message {
   public:
	Message(Widget parent, int dialog_type, char *msg);
};

	
#endif
