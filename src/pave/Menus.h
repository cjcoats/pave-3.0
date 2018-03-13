#ifndef MENUS_H
#define MENUS_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)Menus.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.Menus.h
 * Last updated: 12/15/97 16:27:48
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

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Menus.h 
// Kah Eng Pua
// Copyright (C)
// December 7, 1994
//////////////////////////////////////////////////////////////////////////////
//
//    Menus Class
//
//    Menus                                     Concrete
//        1. Creates menu bar
//        2. Creates pulldown menus
//
//////////////////////////////////////////////////////////////////////////////


#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <Xm/Xm.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <assert.h>



class Menus {
  protected:

	struct menu_struct {
		char	*name;			// name of the button 
		void	(*func)(Widget, XtPointer, XtPointer); // Callback to be invoked 
		struct	menu_struct *sub_menu;	// Data for submenu 
		int	n_sub_items;		// Items in sub_menu
		char	*sub_menu_title;	// Title of submenu
		Widget	wid;			// button widget
	};

	Widget		menuBar_;
	XtPointer	caller_;

	void createMenuBar(Widget parent, XtPointer caller);

  public:
	void createMenuButtons(char *title, Widget menu, 
			menu_struct *menulist, int items);
	Menus();

};


#endif
