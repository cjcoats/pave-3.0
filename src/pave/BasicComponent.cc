/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: BasicComponent.cc 83 2018-03-12 19:24:33Z coats $
 *  Copyright (C) 1996-2004 MCNC
 *            (C) 2004-2010 UNC Institute for the Environment
 *            (C) 2018-     Carlie J. Coats, Jr., Ph.D.
 *
 *  Licensed under the GNU General Public License Version 2.
 *  See enclosed gpl.txt for more details
 *
 *  For further information on PAVE:
 *      Usage: type -usage in PAVE's standard input
 *      User Guide: https://cjcoats.github.io/pave/PaveManual.html
 *      FAQ:        https://cjcoats.github.io/pave/Pave.FAQ.html
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
//////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
// BasicComponent.C: Initial version of a class to define
//                    a protocol for all components
///////////////////////////////////////////////////////////
#include "BasicComponent.h"

BasicComponent::BasicComponent ( const char *name )
    {
    _w = ( Widget ) NULL;
    assert ( name != NULL );  // Make sure programmers provide name
    _name = strdup ( name );
    }

BasicComponent::~BasicComponent()
    {
    if ( _w )
        XtDestroyWidget ( _w );
    if ( _name ) /* SRT delete _name */  free ( _name );
    _name = ( char * ) NULL;

    }

void BasicComponent::manage()
    {
    assert ( _w != NULL );
    if ( XtIsManaged ( _w ) ) XtUnmanageChild ( _w );
    XtManageChild ( _w );
    }

void BasicComponent::unmanage()
    {
    assert ( _w != NULL );
    XtUnmanageChild ( _w );
    }
