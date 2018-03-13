#ifndef SHELL_H
#define SHELL_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)Shell.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.Shell.h
 * Last updated: 12/15/97 16:28:15
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
// Shell.h
// K. Eng Pua
// Dec 14, 1994
///////////////////////////////////////////////////////////////////////////////
//
//   Shell Class
//
//   Shell : UIComponent                          Concrete
//        1. Create a popup shell
//        2. Manages, unmanages, and iconifies
//           the shell window
//
//////////////////////////////////////////////////////////////////////////////

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif


#include <Xm/Xm.h>
#include <assert.h>
#include <Xm/MwmUtil.h> /* For bit definitions for MwmHints.functions */

#include "AppInit.h"
#include "UIComponent.h"

class Shell : public UIComponent {

  protected:

    Widget	shell_;		// The XmMainWindow widget

  public :

    Shell ( AppInit *app, char * );

    virtual void manage();	// Pop up the window
    virtual void unmanage();	// Pop down the window
    virtual void iconify();
};
#endif
