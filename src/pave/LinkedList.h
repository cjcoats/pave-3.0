#ifndef LINKEDLIST_H
#define LINKEDLIST_H

/////////////////////////////////////////////////////////////
//File: $Id: LinkedList.h 85 2018-03-13 13:17:36Z coats $
//
// Project Title: Environmental Decision Support System
//         File: @(#)LinkedList.h	2.1
//     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.LinkedList.h
// Last updated: 12/15/97 16:26:41
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
// LinkedList.h
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 23, 1995
//
/////////////////////////////////////////////////////////////
//
// LinkedList Class 
//
// Portions from Roger Sessions' "Class Construction in C
// and C++", copyright 1992 by Prentice-Hall
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT	950523	Implemented
// SRT  950607  Added findAndRemoveLink(void *) routine
// SRT  950908  Changed MAX_INT from 26 to 1028
//
/////////////////////////////////////////////////////////////


        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include "Link.h"


#define MAX_INT 1028	// Default maximum list size


class linkedList:public baseType {

  public:

	linkedList();			// Constructor

	virtual ~linkedList();		// Destructor

	virtual void freeContents();	// Works like constructor,
					// but also deletes elements
					// stored in list

	virtual baseType *head();	// head becomes current

	virtual baseType *tail();	// tail becomes current

	virtual int length();		// Returns length of list

	virtual void setMax(int newMax);// Sets max length of list

	virtual int left();		// max - length
	
	virtual baseType *next();	// next becomes current

	virtual baseType *previous();	// previous becomes current

	virtual baseType *retrieve();	// retrieve current contents

	virtual baseType *replace(baseType *newElement);
					// replace current contents

	virtual baseType *promoteTail();// move tail to head

	virtual baseType *addHead(baseType *newElement);	
					// add a new element as head

	virtual baseType *addTail(baseType *newElement);	
					// add new element as tail

	virtual baseType *removeHead();	// delete head

	virtual baseType *findAndRemoveLink(void *); // SRT DOES THIS WORK?

	virtual int isTail();		// is current the tail?

	virtual void print(FILE *output); // print list

	virtual baseType *find(void *findme);// is this item on list already?

	char *getClassName(void);	// override baseType's 
					// virtual getClassName()

  private:

	int max;

	int nlinks;

	/* SRT 951122 link*/ void *currentLink;

	/* SRT 951122 link*/ void *headLink;

	/* SRT 951122 link*/ void *tailLink;
};


#endif  // LINKEDLIST_H

