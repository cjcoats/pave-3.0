/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: MultiSel.cc 83 2018-03-12 19:24:33Z coats $
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


#include "DriverWnd.h"

MultiSelect::MultiSelect ( Widget dialog, int n, int type, void *dwnd,
                           void *twnd )
    {
    int i;

    msdialog_ = dialog;
    dwnd_ = dwnd;
    twnd_ = twnd;
    type_ = type;
    nSelection_ = n;
    currentSelection_ = 0;

    name_ = new char*[n];
    for ( i=0; i<n; i++ )
        {
        name_[i] = new char[256];
        strcpy ( name_[i], "" );
        }
    }

void MultiSelect::multiselect1CB ( Widget, XtPointer clientData, XtPointer )
    {
    MultiSelect *obj = ( MultiSelect * ) clientData;
    obj->multiselect ( 0 );
    }

void MultiSelect::multiselect2CB ( Widget, XtPointer clientData, XtPointer )
    {
    MultiSelect *obj = ( MultiSelect * ) clientData;
    obj->multiselect ( 1 );
    }

void MultiSelect::multiselect3CB ( Widget, XtPointer clientData, XtPointer )
    {
    MultiSelect *obj = ( MultiSelect * ) clientData;
    obj->multiselect ( 2 );
    }

void MultiSelect::multiselect ( int n )
    {
    currentSelection_ = n;
    showMultiSelection();
    }

void MultiSelect::multiselectOkCB ( Widget, XtPointer clientData, XtPointer )
    {
    MultiSelect *obj = ( MultiSelect * ) clientData;
    obj->multiselectOk();
    }

void MultiSelect::multiselectOk()
    {
    if ( ( ( DriverWnd * ) dwnd_ )->processMultiSelect ( type_, nSelection_,
            name_, twnd_ ) )
        {
        XtUnmanageChild ( msdialog_ );
        }
    else
        {
        fprintf ( stderr,"WARNING: not all required components are specified. Operation cancelled\n" );
        }

    }

void MultiSelect::multiselectApplyCB ( Widget, XtPointer clientData, XtPointer )
    {
    MultiSelect *obj = ( MultiSelect * ) clientData;
    obj->multiselectApply();
    }

void MultiSelect::multiselectApply()
    {

    char *newtext  = XmTextGetString (
                         XmSelectionBoxGetChild ( msdialog_, XmDIALOG_TEXT ) );
    strcpy ( name_[currentSelection_], newtext );
    }

void MultiSelect::showMultiSelection()
    {
    XmString string = XmStringCreateLocalized ( name_[currentSelection_] );
    XtVaSetValues ( msdialog_,
                    XmNtextString, string, NULL );
    XmStringFree ( string );
    }

//  if (nSelection_ <= currentSelection_) {
//    nSelection_ = currentSelection_ + 1;
//  }
