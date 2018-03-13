/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: SpeciesServer.cc 83 2018-03-12 19:24:33Z coats $
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
// File:    SpeciesServer.C
// Author:  K. Eng Pua
// Date:    April 18, 1995
///////////////////////////////////////////////////////////////////////////////
// Modification History
// SRT 950908 added int SpeciesServer::removeAllItems(Widget dialog)
//////////////////////////////////////////////////////////////////////////////

#include "SpeciesServer.h"


SpeciesServer::SpeciesServer ( char *name, Widget parent, char *dialog_title ) :
    SelectionServer ( name, parent, dialog_title, "" )
    {
    createEditSelectionDialog();
    formulaServer_ = ( void * ) NULL;
    }

int SpeciesServer::addItem ( char *item, Widget dialog )
    {
    removeWhiteSpace ( item ); // added SRT 950908
    addItemToList ( item, dialog );
    return 0;
    }



///////////////////////////////////////
//  SpeciesServer Dialog
///////////////////////////////////////
Widget SpeciesServer::createEditSelectionDialog()
    {
    char strbuf[256];

    memset ( strbuf, ( int ) NULL, sizeof ( strbuf ) );

    // SRT 950705 sprintf(strbuf, " ");
    // SRT 950705 XmString label = XmStringCreateSimple(strbuf);

    sprintf ( strbuf, "%s List", title_ );

    XmString label = XmStringCreateLocalized ( strbuf ); // added SRT 950705

#ifdef DIAGNOSTICS
    fprintf ( stderr, "In SpeciesServer::createEditSelectionDialog() len==%d strbuf=='%s'\n", strlen ( strbuf ), strbuf );
#endif // DIAGNOSTICS

    edit_selection_dialog_ = XmCreateSelectionDialog ( parent_, strbuf, NULL, 0 );
    XtVaSetValues ( edit_selection_dialog_,
                    XmNautoUnmanage,        False,
                    XmNselectionLabelString,    label,
                    XmNheight,                      320, // SRT 960411
                    NULL );
    XmStringFree ( label );

    // Change ok button to Close
    label = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( "Close" );
    XtVaSetValues ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_OK_BUTTON ),
                    XmNlabelString, label,
                    NULL );
    XmStringFree ( label );

    // Unmange dialog text widget
    XtUnmanageChild ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ) );

    // Unmange cancel button
    XtUnmanageChild ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_CANCEL_BUTTON ) );

    // Unmange apply button
    XtUnmanageChild ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_APPLY_BUTTON ) );

    // Unmanage help button
    XtUnmanageChild ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_HELP_BUTTON ) );


    // SRT 960409 when a species is single clicked, call singleClickCB()
    XtAddCallback ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_LIST ),
                    XmNbrowseSelectionCallback,
                    &SpeciesServer::singleClickCB,
                    ( XtPointer ) this );

    return ( edit_selection_dialog_ );
    }


void SpeciesServer::singleClickCB ( Widget, XtPointer clientData, XtPointer )
    {
    SpeciesServer *obj = ( SpeciesServer * ) clientData;
    obj->single_click_cb();
    }

void SpeciesServer::single_click_cb()
    {
    char *newtext  = XmTextGetString (
                         XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ) );

    if ( !newtext || !*newtext )
        {
        // null string entered
        XtFree ( newtext );
        return;
        }

    if ( formulaServer_ )
        ( ( FormulaServer * ) formulaServer_ )->selectThisItemAddingIfNecessary ( newtext );

    XtFree ( newtext );
    }


void SpeciesServer::postSpeciesDialog (  char **speclist,
        int nspecies,
        int dataSetNum ) // 1 based
    {
    char tstring[255];

    if ( dataSetNum < 0 )
        {
        fprintf ( stderr, "Non positive dataSetNum supplied to "
                  "SpeciesServer::postSpeciesDialog()!\n" );
        return;
        }

    for ( int i=0; i < nspecies; i++ )
        {
        sprintf ( tstring, "%s%c", speclist[i], ( int ) ( 'a'+dataSetNum-1 ) );
        addItemToList ( tstring, edit_selection_dialog_ );
        }

    if ( XtIsManaged ( edit_selection_dialog_ ) ) XtUnmanageChild ( edit_selection_dialog_ );
    XtManageChild ( edit_selection_dialog_ );
    }


void SpeciesServer::clearList()
    {
    int count = getItemCount();
    if ( count > 0 )
        {
        XmListDeleteItemsPos (
            XmSelectionBoxGetChild ( edit_selection_dialog_,
                                     XmDIALOG_LIST ),
            count, 1 );
        }
    }


int SpeciesServer::removeAllItems ( Widget dialog )
    {
    XmListDeleteAllItems ( XmSelectionBoxGetChild ( dialog, XmDIALOG_LIST ) );
    setCurrentSelection ( "" );
    return 0;
    }


int SpeciesServer::setFormulaServer ( void *formulaServer )
    {
    formulaServer_ = ( void * ) formulaServer;
    return 0;
    }
