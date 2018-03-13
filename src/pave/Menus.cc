/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  Version 3.0
 *
 *  File: $Id: Menus.cc 83 2018-03-12 19:24:33Z coats $
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

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Menus.C
// Kah Eng Pua
// December 7, 1994
// Note:
//  The createMenuButtons function is based on Douglas Young's original
//  menus.c program.  See pp 104 to 106 of The X Window System
//  Programming and Applications with Xt.  Prentice Hall, 1990.
//  Modification by K. Eng Pua
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include "Menus.h"

Menus::Menus()
    {
    // Empty constructor
    }


void Menus::createMenuBar ( Widget parent, XtPointer caller )
    {
    caller_ = caller;

    assert ( parent );

    menuBar_ = XmCreateMenuBar ( parent, "menubar", NULL, 0 );
    XtVaSetValues ( menuBar_,
                    XmNleftAttachment, XmATTACH_FORM,
                    XmNrightAttachment, XmATTACH_FORM,
                    NULL );
    if ( XtIsManaged ( menuBar_ ) ) XtUnmanageChild ( menuBar_ );
    XtManageChild ( menuBar_ );

    }


void Menus::createMenuButtons ( char *title, Widget menu, menu_struct *menulist, int nitems )
    {
    /*
    ** If a title is given, create Label and Separator widgets.
    */
    if ( title )
        {
        XtCreateManagedWidget ( title, xmLabelWidgetClass, menu, NULL, 0 );
        XtCreateManagedWidget ( "separator", xmSeparatorWidgetClass,
                                menu, NULL, 0 );
        }
    /*
    **  Create an entry for each item in the menu.
    */
    for ( int i=0; i<nitems; i++ )
        {
        /*
        ** A NULL name represents a separator.
        */
        if ( menulist[i].name == NULL )
            {
            XtCreateManagedWidget ( "separator",
                                    xmSeparatorWidgetClass,
                                    menu, NULL, 0 );
            }
        /*
        ** If there is a name and a callback, create a selectable
        ** menu entry and register the callback function.
        */
        else if ( menulist[i].func )
            {
            menulist[i].wid = XtCreateManagedWidget ( menulist[i].name,
                              xmPushButtonWidgetClass,
                              menu, NULL, 0 );
            XtAddCallback ( menulist[i].wid, XmNactivateCallback,
                            menulist[i].func, caller_ );
            }
        /*
        ** If there is a name, but no callback function, the entry
        ** must be a label, unless there is a submenu.
        */
        else if ( !menulist[i].sub_menu )
            menulist[i].wid = XtCreateManagedWidget ( menulist[i].name,
                              xmLabelWidgetClass,
                              menu, NULL, 0 );
        /*
        ** If we got here, the entry must be a submenu.
        ** Create a pulldown menu pane and an XmCascadeButton
        ** widget.  Attach the menu pane and make a recursive call
        ** to create the entries in the submenu.
        */
        else
            {
            Widget sub_menu = XmCreatePulldownMenu ( menu,
                              menulist[i].name,
                              NULL, 0 );
            menulist[i].wid = XtVaCreateManagedWidget ( menulist[i].name,
                              xmCascadeButtonWidgetClass, menu,
                              XmNsubMenuId, sub_menu,
                              NULL );
            createMenuButtons ( menulist[i].sub_menu_title,
                                sub_menu, menulist[i].sub_menu,
                                menulist[i].n_sub_items );
            }
        }
    }
