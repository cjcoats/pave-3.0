/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Shell.cc 83 2018-03-12 19:24:33Z coats $
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
// Shell.C
// K. Eng Pua
// Dec 14, 1994
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include "Shell.h"

Shell::Shell ( AppInit *app, char *name ) : UIComponent ( name )
    {
    _w = shell_ = XtCreatePopupShell ( _name,
                                       applicationShellWidgetClass,
                                       app->baseWidget(),
                                       NULL, 0 );

    installDestroyHandler();

    // added SRT 961014
    XtVaSetValues (     shell_,

                        XmNdeleteResponse, XmDO_NOTHING,   // do nothing if the
                        // user double-clicks
                        // the upper left of the
                        // motif window, rather
                        // than delete it

                        XmNmwmFunctions, MWM_FUNC_RESIZE |  // don't show the
                        MWM_FUNC_MOVE |    // window manager's
                        MWM_FUNC_MINIMIZE |    // close and delete
                        MWM_FUNC_MAXIMIZE, // menu items in
                        // this window
                        NULL );
    }


void Shell::manage()
    {
    assert ( shell_ );
    XtPopup ( shell_, XtGrabNone );

    // Map the window, in case the window is iconified

    if ( XtIsRealized ( shell_ ) )
        XMapRaised ( XtDisplay ( shell_ ), XtWindow ( shell_ ) );
    }


void Shell::unmanage()
    {
    assert ( shell_ );
    XtPopdown ( shell_ );
    }


void Shell::iconify()
    {
    assert ( shell_ );

    // Set the widget to have an initial iconic state
    // in case the base widget has not yet been realized

    XtVaSetValues ( shell_, XmNiconic, TRUE, NULL );

    // If the widget has already been realized,
    // iconify the window

    if ( XtIsRealized ( shell_ ) )
        XIconifyWindow ( XtDisplay ( shell_ ), XtWindow ( shell_ ), 0 );
    }


