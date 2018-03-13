/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: SelectLoadSaveServer.cc 83 2018-03-12 19:24:33Z coats $
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
// File:    SelectLoadSaveServer.C
// Author:  K. Eng Pua
// Date:    April 18, 1995
///////////////////////////////////////////////////////////////////////////////
//
// Modification History
// When   Who What
// 950831 SRT Added removeAllItems() call to loadSelectionFile() routine
// 961011 SRT Added load_cancelCB(), load_cancel_cb(), save_cancelCB(), save_cancel_cb()
//
//////////////////////////////////////////////////////////////////////////////

#include "SelectLoadSaveServer.h"


SelectLoadSaveServer::SelectLoadSaveServer ( char *name, Widget parent, char *dialogtitle,
        char *file_marker ) :
    SelectionServer ( name, parent, dialogtitle, file_marker )
    {
    load_selection_dialog_ = NULL;
    save_selection_dialog_ = NULL;
    }



////////////////////////////////////////
//  Load SelectLoadSaveServer
////////////////////////////////////////

void SelectLoadSaveServer::postLoadSelectionDialog()
    {
    if ( load_selection_dialog_ == NULL )
        createLoadSelectionDialog();
    if ( XtIsManaged ( load_selection_dialog_ ) ) XtUnmanageChild ( load_selection_dialog_ );
    XtManageChild ( load_selection_dialog_ );
    }

void SelectLoadSaveServer::createLoadSelectionDialog()
    {
    char strbuf[100];
    XmString dir = XmStringCreateLocalized ( getenv ( "HOME" ) ); // SRT 960425

    sprintf ( strbuf, "Load %ss", title_ );
    load_selection_dialog_ = XmCreateFileSelectionDialog ( parent_, strbuf, NULL, 0 );
    XtVaSetValues ( load_selection_dialog_,
                    XmNautoUnmanage,    True,
                    XmNdirectory,           dir, // SRT 960425
                    NULL );

    XmStringFree ( dir );
    dir = NULL; // SRT 960425

    XtAddCallback ( load_selection_dialog_, XmNokCallback, &SelectLoadSaveServer::load_okCB, ( XtPointer ) this );
    XtAddCallback ( load_selection_dialog_, XmNcancelCallback, &SelectLoadSaveServer::load_cancelCB, ( XtPointer ) this );
    }



void SelectLoadSaveServer::load_okCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    SelectLoadSaveServer *obj = ( SelectLoadSaveServer * ) clientData;
    XmFileSelectionBoxCallbackStruct *cbs = ( XmFileSelectionBoxCallbackStruct * ) callData;

    obj->load_ok_cb ( cbs->value );
    }


void SelectLoadSaveServer::load_ok_cb ( XmString xmstr )
    {
    char *str;

    if ( XmStringGetLtoR ( xmstr, XmSTRING_DEFAULT_CHARSET, &str ) )
        loadSelectionFile ( str );

    XtFree ( str );
    }


void SelectLoadSaveServer::loadSelectionFile ( char *filename )
    {
    char strbuf[1024];
    FILE *fp;
    int first_time = 1;

    if ( ( fp = fopen ( filename, "r" ) ) != NULL )
        {
        fgets ( strbuf, 1024, fp );

        if ( !strncmp ( strbuf, file_marker_, strlen ( file_marker_ ) ) )
            {
            if ( edit_selection_dialog_ == NULL )
                {
                createEditSelectionDialog();
                }
            while ( !feof ( fp ) )
                {
                fgets ( strbuf, 1024, fp );
                if ( !feof ( fp ) )
                    {
                    int len = strlen ( strbuf );
                    if ( len > 1 )
                        {
#ifdef DIAGNOSTICS
                        fprintf ( stderr,
                                  "SelectLoadSaveServer::loadSelectionFile() about to add item '%s'\n",
                                  strbuf );
#endif // DIAGNOSTICS
                        if ( first_time )
                            {
                            first_time = 0;
                            removeAllItems ( edit_selection_dialog_ );
                            }
                        strbuf[len-1] = '\0';
                        addItem ( strbuf, edit_selection_dialog_ );
                        }
                    }
                strbuf[0] = '\0';
                }
            }
        else
            {
            strbuf[0] = '\0';
            sprintf ( strbuf, "%s file marker not found in\n  %s.\n", title_, filename );
            Message error ( parent_, XmDIALOG_ERROR, strbuf );
            fclose ( fp );
            return;
            }
        }
    if ( fp != NULL ) fclose ( fp );
    }


//////////////////////////////////////
//  Save SelectLoadSaveServers
//////////////////////////////////////

void SelectLoadSaveServer::postSaveSelectionDialog()
    {
    if ( save_selection_dialog_ == NULL )
        createSaveSelectionDialog();
    if ( XtIsManaged ( save_selection_dialog_ ) ) XtUnmanageChild ( save_selection_dialog_ );
    XtManageChild ( save_selection_dialog_ );
    }


void SelectLoadSaveServer::createSaveSelectionDialog()
    {

    char strbuf[100];
    XmString dir = XmStringCreateLocalized ( getenv ( "HOME" ) ); // SRT 960425

    sprintf ( strbuf, "Save %ss", title_ );
    save_selection_dialog_ =
        XmCreateFileSelectionDialog ( parent_, strbuf, NULL, 0 );
    XtVaSetValues ( save_selection_dialog_,
                    XmNautoUnmanage,        True,
                    XmNdirectory,           dir, // SRT 960425
                    NULL );

    XmStringFree ( dir );
    dir = NULL; // SRT 960425

    XtAddCallback ( save_selection_dialog_, XmNokCallback, &SelectLoadSaveServer::save_okCB, ( XtPointer ) this );
    XtAddCallback ( save_selection_dialog_, XmNcancelCallback, &SelectLoadSaveServer::save_cancelCB, ( XtPointer ) this );
    }



void SelectLoadSaveServer::save_okCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    SelectLoadSaveServer *obj = ( SelectLoadSaveServer * ) clientData;
    XmFileSelectionBoxCallbackStruct *cbs = ( XmFileSelectionBoxCallbackStruct * ) callData;

    obj->save_ok_cb ( cbs->value );
    }



void SelectLoadSaveServer::save_ok_cb ( XmString xmstr )
    {
    char *str;

    if ( XmStringGetLtoR ( xmstr, XmSTRING_DEFAULT_CHARSET, &str ) )
        {
        if ( edit_selection_dialog_ == NULL )
            createEditSelectionDialog();

        saveSelectionListToFile ( str, edit_selection_dialog_ );
        }

    XtFree ( str );
    }



void SelectLoadSaveServer::saveSelectionListToFile ( char *filename, Widget dialog )
    {
    FILE *fp;
    char strbuf[256];

    assert ( dialog );

    if ( ( fp = fopen ( filename, "w" ) ) != NULL )
        {

        fprintf ( fp, "%s\n", file_marker_ );
        XmString *strlist;
        int count = 0;
        // Get the current entries (and number of entries) from the list
        XtVaGetValues ( XmSelectionBoxGetChild ( dialog, XmDIALOG_LIST ),
                        XmNitemCount,   &count,
                        XmNitems,       &strlist,
                        NULL );

        char *text;
        for ( int i = 0; i < count; i++ )
            {
            XmStringGetLtoR ( strlist[i], XmSTRING_DEFAULT_CHARSET, &text );
            fprintf ( fp, "%s\n", text );
            XtFree ( text );
            }
        if ( count <=0 )
            {
            sprintf ( strbuf, "No %s to save.", title_ );
            Message error ( parent_, XmDIALOG_ERROR, strbuf );
            sprintf ( strbuf, "rm %s\n", filename );
            system ( strbuf );
            }
        }
    else
        {
        sprintf ( strbuf, "Can't save to file %s\n", filename );
        Message error ( parent_, XmDIALOG_ERROR, strbuf );
        }
    fclose ( fp );

    }



void SelectLoadSaveServer::load_cancelCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    SelectLoadSaveServer *obj = ( SelectLoadSaveServer * ) clientData;
    XmFileSelectionBoxCallbackStruct *cbs = ( XmFileSelectionBoxCallbackStruct * ) callData;

    obj->load_cancel_cb ( cbs->value );
    }


void SelectLoadSaveServer::load_cancel_cb ( XmString /*xmstr*/ )
    {
    }


void SelectLoadSaveServer::save_cancelCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    SelectLoadSaveServer *obj = ( SelectLoadSaveServer * ) clientData;
    XmFileSelectionBoxCallbackStruct *cbs = ( XmFileSelectionBoxCallbackStruct * ) callData;

    obj->save_cancel_cb ( cbs->value );
    }



void SelectLoadSaveServer::save_cancel_cb ( XmString /*xmstr*/ )
    {
    }
