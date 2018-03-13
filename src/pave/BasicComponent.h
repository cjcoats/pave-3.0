#ifndef BASICCOMPONENT_H
#define BASICCOMPONENT_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)BasicComponent.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.BasicComponent.h
 * Last updated: 12/15/97 16:25:19
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
//   BasicComponent Class
//
//   BasicComponent                               Concrete
//        1. Stores component name
//        2. Stores widget ID
//        3. Manages and unmanage widget
//
//   Note: This class is taken from Doug Young's ComponentLib.
//
//////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////
// BasicComponent.h: First version of a class to define 
//                    a protocol for all components
///////////////////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <stdlib.h>
#include <Xm/Xm.h>
#include <malloc.h>
#include <assert.h>
#include <stdio.h>


class BasicComponent {
    
  protected:
    
    char    *_name;
    Widget   _w;    
    
    // Protected constructor to prevent instantiation
    
    BasicComponent ( const char * );   
    
  public:
    
    virtual ~BasicComponent();
    virtual void manage();   // Manage and unmanage widget tree
    virtual void unmanage();
    const Widget baseWidget() { return _w; }
};
#endif

