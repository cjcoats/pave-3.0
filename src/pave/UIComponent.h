#ifndef UICOMPONENT_H
#define UICOMPONENT_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)UIComponent.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.UIComponent.h
 * Last updated: 12/15/97 16:28:33
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
//         This example code is from the book:
//
//           Object-Oriented Programming with C++ and OSF/Motif
//         by
//           Douglas Young
//           Prentice Hall, 1992
//           ISBN 0-13-630252-1	
//
//         Copyright 1991 by Prentice Hall
//         Hill under terms of the GNU Public License.  See gpl.txt for more details.
//
//  Permission to use, copy, modify, and distribute this software for 
//  any purpose except publication and without fee is hereby granted, provided 
//  that the above copyright notice appear in all copies of the software.
///////////////////////////////////////////////////////////////////////////////
//
//   UIComponent Class
//
//   UIComponent : BasicComponent                 Abstract
//
//        1. Encapsulates a widget subtree
//        2. Manages/unmanages a widget tree
//        3. Initializes derived classes from
//           resource database
//        4. Handles widget destruction
//        5. Loads default resources into
//           resource database
//
//////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////
// UIComponent.h: Base class for all C++/Motif UI components
///////////////////////////////////////////////////////////////
#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif


#include <Xm/Xm.h>
#include "BasicComponent.h"
#include <assert.h>
#include <stdio.h>


class UIComponent : public BasicComponent {
    
  private:
    
    // Interface between XmNdestroyCallback and this class
    
    static void widgetDestroyedCallback ( Widget, 
					 XtPointer, 
					 XtPointer );
    
  protected:
    
    // Protect constructor to prevent direct instantiation
    
    UIComponent ( const char * );
    
    void installDestroyHandler(); // Easy hook for derived classes
    
    // Called by widgetDestroyedCallback() if base widget is destroyed
    
    virtual void widgetDestroyed(); 
    
    // Loads component's default resources into database
    
    void setDefaultResources ( const Widget , const String *);
    
    // Retrieve resources for this clsss from the resource manager
    
    void getResources ( const XtResourceList, const int );
    
  public:
    
    virtual ~UIComponent();  // Destructor
    
    // Manage the entire widget subtree represented
    // by this component. Overrides BasicComponent method
    
    virtual void manage();
    
    // Public access functions
    
    virtual const char *const className() { return "UIComponent"; }
};

#endif
