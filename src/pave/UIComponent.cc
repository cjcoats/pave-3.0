/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: UIComponent.cc 83 2018-03-12 19:24:33Z coats $
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


///////////////////////////////////////////////////////////////
// UIComponent.C: Base class for all C++/Motif UI components
///////////////////////////////////////////////////////////////

#include "UIComponent.h"

UIComponent::UIComponent ( const char *name ) : BasicComponent ( name )
    {
    // Empty
    }

void UIComponent::widgetDestroyedCallback ( Widget,
        XtPointer clientData,
        XtPointer )
    {
    UIComponent * obj = ( UIComponent * ) clientData;

    obj->widgetDestroyed();
    }

void UIComponent::widgetDestroyed()
    {
    _w = NULL;
    }

void UIComponent::installDestroyHandler()
    {
    assert ( _w != NULL );
    XtAddCallback ( _w,
                    XmNdestroyCallback,
                    &UIComponent::widgetDestroyedCallback,
                    ( XtPointer ) this );
    }

void UIComponent::manage()
    {
    assert ( _w != NULL );
#if 0
    assert ( XtHasCallbacks ( _w, XmNdestroyCallback ) ==
             XtCallbackHasSome );
#endif
    if ( XtIsManaged ( _w ) ) XtUnmanageChild ( _w );
    XtManageChild ( _w );
    }

UIComponent::~UIComponent()
    {
    // Make sure the widget hasn't already been destroyed

    if ( _w )
        {
        // Remove destroy callback so Xt can't call the callback
        // with a pointer to an object that has already been freed

        XtRemoveCallback ( _w,
                           XmNdestroyCallback,
                           &UIComponent::widgetDestroyedCallback,
                           ( XtPointer ) this );
        }
    }

void UIComponent::getResources ( const XtResourceList resources,
                                 const int numResources )
    {
    // Check for errors

    assert ( _w != NULL );
    assert ( resources != NULL );

    // Retrieve the requested resources relative to the
    // parent of this object's base widget

    XtGetSubresources ( XtParent ( _w ),
                        ( XtPointer ) this,
                        _name,
                        className(),
                        resources,
                        numResources,
                        NULL,
                        0 );
    }

void UIComponent::setDefaultResources ( const Widget w,
                                        const String *resourceSpec )
    {
    int         i;
    Display    *dpy = XtDisplay ( w );     // Retrieve the display pointer
    XrmDatabase rdb = NULL;             // A resource data base

    // Create an empty resource database

    rdb = XrmGetStringDatabase ( "" );

    // Add the Component resources, prepending the name of the component

    i = 0;
    while ( resourceSpec[i] != NULL )
        {
        char buf[1000];

        sprintf ( buf, "*%s%s", _name, resourceSpec[i++] );
        XrmPutLineResource ( &rdb, buf );
        }

    // Merge them into the Xt database, with lowest precendence

    if ( rdb )
        {
#if (XlibSpecificationRelease>=5)
        XrmDatabase db = XtDatabase ( dpy );
        XrmCombineDatabase ( rdb, &db, FALSE );
#else
        XrmMergeDatabases ( dpy->db, &rdb );
        dpy->db = rdb;
#endif
        }
    }

