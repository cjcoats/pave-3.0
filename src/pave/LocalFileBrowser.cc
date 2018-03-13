/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: LocalFileBrowser.cc 83 2018-03-12 19:24:33Z coats $
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
// File:    LocalFileBrowser.cc
// Author:  Steve Thorpe
// Date:    May 22, 1996
///////////////////////////////////////////////////////////////////////////////
//
// Modification History
// When   Who What
// 960522 SRT Implemented
//
//////////////////////////////////////////////////////////////////////////////

#include "LocalFileBrowser.h"


////////////////////////////////////////////////
//
//  LocalFileBrowser()
//
////////////////////////////////////////////////

LocalFileBrowser::LocalFileBrowser
(
    Widget parent,
    char *fileTypeName,
    char *defaultDirectory,
    void *gotSaveFileName, /* void (*gotSaveFileName)(void *, char *), */
    void *gotLoadFileName, /* void (*gotLoadFileName)(void *, char *), */
    void *object
)
    {
    defaultDirectory_ = NULL;
    fileTypeName_ = NULL;
    load_selection_dialog_ = NULL;
    save_selection_dialog_ = NULL;
    parent_ = parent;
    if ( fileTypeName ) fileTypeName_ = strdup ( fileTypeName );
    if ( defaultDirectory ) defaultDirectory_ = strdup ( defaultDirectory );
    gotSaveFileName_ = ( void ( * ) ( void *, char * ) ) gotSaveFileName;
    gotLoadFileName_ = ( void ( * ) ( void *, char * ) ) gotLoadFileName;
    object_ = object;
    }



////////////////////////////////////////////////
//
//  ~LocalFileBrowser()
//
////////////////////////////////////////////////

LocalFileBrowser::~LocalFileBrowser()
    {
    if ( fileTypeName_ ) free ( fileTypeName_ );
    fileTypeName_ = NULL;
    if ( defaultDirectory_ ) free ( defaultDirectory_ );
    defaultDirectory_ = NULL;
    }



////////////////////////////////////////////////
//
//  postLoadSelectionDialog()
//
////////////////////////////////////////////////

void LocalFileBrowser::postLoadSelectionDialog()
    {
    if ( load_selection_dialog_ == NULL )
        createLoadSelectionDialog();
    if ( XtIsManaged ( load_selection_dialog_ ) ) XtUnmanageChild ( load_selection_dialog_ );
    XtManageChild ( load_selection_dialog_ );
    }



////////////////////////////////////////////////
//
//  postSaveSelectionDialog()
//
////////////////////////////////////////////////

void LocalFileBrowser::postSaveSelectionDialog()
    {
    if ( save_selection_dialog_ == NULL )
        createSaveSelectionDialog();
    if ( XtIsManaged ( save_selection_dialog_ ) ) XtUnmanageChild ( save_selection_dialog_ );
    XtManageChild ( save_selection_dialog_ );
    }




////////////////////////////////////////////////
//
//  createSaveSelectionDialog()
//
////////////////////////////////////////////////

void LocalFileBrowser::createSaveSelectionDialog()
    {

    char strbuf[100];
    XmString dir = defaultDirectory_ ?
                   XmStringCreateLocalized ( defaultDirectory_ ) :
                   XmStringCreateLocalized ( getenv ( "HOME" ) );

    sprintf ( strbuf, "Save %s as:", fileTypeName_ ? fileTypeName_ : "" );
    save_selection_dialog_ =
        XmCreateFileSelectionDialog ( parent_, strbuf, NULL, 0 );
    XtVaSetValues ( save_selection_dialog_,
                    XmNautoUnmanage,        True,
                    XmNdirectory,           dir, // SRT 960425
                    NULL );

    XmStringFree ( dir );
    dir = NULL; // SRT 960425

    XtAddCallback ( save_selection_dialog_, XmNokCallback, &LocalFileBrowser::save_okCB, ( XtPointer ) this );
    }



////////////////////////////////////////////////
//
//  save_okCB()
//
////////////////////////////////////////////////

void LocalFileBrowser::save_okCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    LocalFileBrowser *obj = ( LocalFileBrowser * ) clientData;
    XmFileSelectionBoxCallbackStruct *cbs = ( XmFileSelectionBoxCallbackStruct * ) callData;

    obj->save_ok_cb ( cbs->value );
    }



////////////////////////////////////////////////
//
//  save_ok_cb()
//
////////////////////////////////////////////////

void LocalFileBrowser::save_ok_cb ( XmString xmstr )
    {
    char *str = NULL;

    if ( XmStringGetLtoR ( xmstr, XmSTRING_DEFAULT_CHARSET, &str ) )
        {
        XtUnmanageChild ( save_selection_dialog_ );
        ( *gotSaveFileName_ ) ( object_, str );
        }

    XtFree ( str );
    }



////////////////////////////////////////////////
//
//  createLoadSelectionDialog()
//
////////////////////////////////////////////////

void LocalFileBrowser::createLoadSelectionDialog()
    {
    char strbuf[100];
    XmString dir = defaultDirectory_ ?
                   XmStringCreateLocalized ( defaultDirectory_ ) :
                   XmStringCreateLocalized ( getenv ( "HOME" ) );

    sprintf ( strbuf, "Load %s", fileTypeName_ ? fileTypeName_ : "" );

    load_selection_dialog_ = XmCreateFileSelectionDialog ( parent_, strbuf, NULL, 0 );
    XtVaSetValues ( load_selection_dialog_,
                    XmNautoUnmanage,    True,
                    XmNdirectory,           dir, // SRT 960425
                    NULL );

    XmStringFree ( dir );
    dir = NULL; // SRT 960425

    XtAddCallback ( load_selection_dialog_, XmNokCallback, &LocalFileBrowser::load_okCB, ( XtPointer ) this );
    }



////////////////////////////////////////////////
//
//  load_okCB()
//
////////////////////////////////////////////////

void LocalFileBrowser::load_okCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    LocalFileBrowser *obj = ( LocalFileBrowser * ) clientData;
    XmFileSelectionBoxCallbackStruct *cbs = ( XmFileSelectionBoxCallbackStruct * ) callData;

    obj->load_ok_cb ( cbs->value );
    }


////////////////////////////////////////////////
//
//  load_ok_cb()
//
////////////////////////////////////////////////

void LocalFileBrowser::load_ok_cb ( XmString xmstr )
    {
    char *str;

    if ( XmStringGetLtoR ( xmstr, XmSTRING_DEFAULT_CHARSET, &str ) )
        {
        XtUnmanageChild ( load_selection_dialog_ );
        ( *gotLoadFileName_ ) ( object_, str );
        }

    XtFree ( str );
    }



