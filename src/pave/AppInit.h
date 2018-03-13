#ifndef APPLICATION_H
#define APPLICATION_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)AppInit.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.AppInit.h
 * Last updated: 12/15/97 16:25:10
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

////////////////////////////////////////////////////////////
// AppInit.h: 
////////////////////////////////////////////////////////////
//
//   AppInit Class
//
//   AppInit : UIComponent                        Concrete
//        1. Handles X/Xt Initialization
//        2. Encapsulates Xt event loop
//        3. Granting friendship to main program
//        4. Maintains global data structures,
//           including the X display, app context
//
////////////////////////////////////////////////////////////

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

#include <X11/Xlib.h>
#include <assert.h>
#include <stdlib.h>

#include "UIComponent.h"

class AppInit : public UIComponent {
    
    // Allow main and MainWindow to access protected member functions
#if (XlibSpecificationRelease>=5)
    friend int main ( int, char ** );
#else
    friend int main ( unsigned int, char ** );
#endif

    
  protected:
    
    Display     *display_;
    XtAppContext appContext_;

#if (XlibSpecificationRelease>=5)
    virtual void initialize ( int *, char ** );
#else
    virtual void initialize ( unsigned int *, char ** );
#endif
    virtual void handleEvents();
    
    char   *applicationClass_;    // Class name of this application
    
  public:

    AppInit ( char * );
    virtual ~AppInit();     
        
    // Convenient access functions
    
    Display      *display()     { return display_; }
    XtAppContext  appContext()  { return appContext_; }
    const char   *applicationClass()  { return applicationClass_; }
    virtual const char *const className() { return "AppInit"; }
    void	realizeWidget() { XtRealizeWidget(_w); }
};




#endif
