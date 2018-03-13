/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: AppInit.cc 83 2018-03-12 19:24:33Z coats $
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

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// AppInit.C:
////////////////////////////////////////////////////////////
#include "AppInit.h"


AppInit::AppInit ( char *appClassName ) :
    UIComponent ( appClassName )
    {

    // Initialize data members

    display_    = ( Display * ) NULL;
    appContext_ = ( XtAppContext ) NULL;
    applicationClass_ = strdup ( appClassName );
    }

#if (XlibSpecificationRelease>=5)
void AppInit::initialize ( int *argcp, char **argv )
#else
void AppInit::initialize ( unsigned int *argcp, char **argv )
#endif
    {
    _w = XtAppInitialize ( &appContext_,
                           applicationClass_, NULL, 0,
                           argcp, argv,
                           NULL, NULL, 0 );

    // Extract and save a pointer to the X display structure

    display_ = XtDisplay ( _w );

    // The AppInit class is less likely to need to handle
    // "surprise" widget destruction than other classes, but
    // we might as well install a callback to be safe and consistent

    installDestroyHandler();

    // Center the shell, and make sure it isn't visible

    XtVaSetValues ( _w,
                    XmNmappedWhenManaged, FALSE,
                    XmNx, DisplayWidth ( display_, 0 ) / 2,
                    XmNy, DisplayHeight ( display_, 0 ) / 2,
                    XmNwidth,  1,
                    XmNheight, 1,
                    NULL );

    // The instance name of this object was set in the UIComponent
    // constructor, before the name of the program was available
    // Free the old name and reset it to argv[0]

    if ( _name ) /* SRT delete _name */  free ( _name );
    _name = ( char * ) NULL;
    _name = strdup ( argv[0] );

    // Force the shell window to exist so dialogs popped up from
    // this shell behave correctly
    XtRealizeWidget ( _w );

    }

AppInit::~AppInit()
    {
    delete applicationClass_;
    }

void AppInit::handleEvents()
    {
    // Just loop forever

    XtAppMainLoop ( appContext_ );
    }

