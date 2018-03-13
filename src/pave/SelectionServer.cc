/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: SelectionServer.cc 83 2018-03-12 19:24:33Z coats $
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
// File:    SelectionServer.C
// Author:  K. Eng Pua
// Date:    Jan 12, 1995
///////////////////////////////////////////////////////////////////////////////
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950523  Modified showSelectionLabel() to show only filename
//      of a dataset, but not the hostname and pathname
// SRT  950816  Added void freeSelectionElements(void)
// SRT  960410  Added void setDriverWnd(void *);
//
//////////////////////////////////////////////////////////////////////////////


#include "SelectionServer.h"

Widget SelectionServer::createEditSelectionDialog()
    {
    return ( Widget ) NULL;    // SRT 950929
    }
int SelectionServer::addItem ( char *, Widget )
    {
    return 0;   // SRT 950929
    }
int SelectionServer::removeAllItems ( Widget )
    {
    return 0;    // SRT 950929
    }

SelectionServer::SelectionServer ( char *name, Widget parent, char *dialogtitle_,
                                   char *file_marker ) : UIComponent ( name )
    {
    assert ( parent );
    parent_ = parent;
    title_ = strdup ( dialogtitle_ );
    file_marker_ = strdup ( file_marker );

    selection_label_ = NULL;
    curr_selection_ = NULL;

    edit_selection_dialog_ = NULL;

    addingFirstTime_ = 1;

    driverWnd_ = NULL;
    }


void SelectionServer::setDoneFirstTime ( void )
    {
    addingFirstTime_ = 0;
    }


void SelectionServer::updateSpecies_cb() // SRT 960410 will be overridden
// by CaseServer.cc's version
    {
    }


void SelectionServer::showSelectionLabel ( char *label )
    {
    char strbuf[512];

    if ( selection_label_ == NULL )
        {
        sprintf ( strbuf, "%s:       ", title_ );
        selection_label_ = XtVaCreateManagedWidget ( strbuf,
                           xmLabelWidgetClass, parent_,
                           NULL );
        }
    else
        {
        // SRT handle special case of "Dataset" differently -
        // show only the filename, not the pathname or hostname
        if ( !strcmp ( title_, "Dataset" ) )
            {
            char    hname[256], // host name
                    pname[256], // path name
                    fname[256]; // file name

            // find the 3 components
            parseLongDataSetName ( label, hname, pname, fname );

            // show only the file name
            sprintf ( strbuf, "%s: %s", title_, fname );
            }
        else
            sprintf ( strbuf, "%s: %s", title_, label );

        XmString xmstr = XmStringCreateLocalized ( strbuf );
        XtVaSetValues ( selection_label_, XmNlabelString, xmstr, NULL );
        XmStringFree ( xmstr );

        updateSpecies_cb(); // will call CaseServer.cc's version SRT 960410
        }
    }


//
// This function is taken from Dan Heller's Motif Programming Manual. p.430
//
void SelectionServer::addItemToOrderedList ( char *newtext, Widget dialog )
    {
    XmString *strlist;
    int u_bound, l_bound = 0;

    assert ( dialog );

    removeWhiteSpace ( newtext ); // added SRT 950908

    // Get the current entries (and number of entries) from the list
    XtVaGetValues ( XmSelectionBoxGetChild ( dialog, XmDIALOG_LIST ),
                    XmNitemCount,   &u_bound,
                    XmNitems,       &strlist,
                    NULL );
    u_bound--;
    char *text;
    // Perform binary search
    while ( u_bound >= l_bound )
        {
        int i = l_bound + ( u_bound - l_bound ) /2;

        // convert the compound string into a regular C string
        if ( !XmStringGetLtoR ( strlist[i], XmSTRING_DEFAULT_CHARSET, &text ) )
            break;
        if ( strcmp ( text, newtext ) > 0 )
            u_bound = i-1;  // newtext comes before item
        else
            l_bound = i+1;  // newtext comes after item
        XtFree ( text );
        }

    XmString xmstr = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( newtext );

    // Position indices starting at 1, so increment accordingly
    XmListAddItemUnselected ( XmSelectionBoxGetChild ( dialog, XmDIALOG_LIST ),
                              xmstr, l_bound+1 );
    XmStringFree ( xmstr );
    XmTextSetString ( XmSelectionBoxGetChild ( dialog, XmDIALOG_TEXT ), "" );
    }


void SelectionServer::addItemToList ( char *item, Widget dialog )
    {
    if ( dialog == NULL ) fprintf ( stderr,"Null dialog in addItemToList\n" );
    assert ( dialog );

    removeWhiteSpace ( item ); // added SRT 950908

    XmString xmstr = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( item );

    int count = getItemCount();

    //fprintf(stderr,"child, count = %d\n", count);
    Widget child = XmSelectionBoxGetChild ( dialog, XmDIALOG_LIST );
    //fprintf(stderr,"add unselected\n");
    XmListAddItemUnselected ( child, xmstr, count+1 );
    //XmListAddItemUnselected(XmSelectionBoxGetChild(dialog, XmDIALOG_LIST),
    //             xmstr, count+1);
    //fprintf(stderr,"free xmstr\n");
    XmStringFree ( xmstr );

    //fprintf(stderr,"text set string, box get child\n");
    XmTextSetString ( XmSelectionBoxGetChild ( dialog, XmDIALOG_TEXT ), "" );
    //fprintf(stderr,"done additemtolist\n");
    }


// find "item" on the list and return its item number
// returns 0 if item not found on list
int SelectionServer::getItemNumber ( char *item )
    {
    int n;
    if ( ! item ) return 0;

    XmString str = XmStringCreateLocalized ( item );

    n= XmListItemPos (
           XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_LIST ), str );
    XmStringFree ( str );
    return n;
    }


int SelectionServer::getItemCount()
    {
    int count;

    XtVaGetValues ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_LIST ),
                    XmNitemCount,   &count,
                    NULL );
    return count;
    }

///////////////////////////////////////
//  Add/Delete/Select SelectionServer
///////////////////////////////////////

Widget SelectionServer::postEditSelectionDialog()
    {
    if ( edit_selection_dialog_ == NULL )
        createEditSelectionDialog();
    if ( XtIsManaged ( edit_selection_dialog_ ) ) XtUnmanageChild ( edit_selection_dialog_ );
    XtManageChild ( edit_selection_dialog_ );
    return ( edit_selection_dialog_ );
    }

void SelectionServer::edit_addCB ( Widget, XtPointer clientData, XtPointer )
    {
    SelectionServer *obj = ( SelectionServer * ) clientData;
    obj->edit_add_cb();
    }


void SelectionServer::edit_add_cb()
    {
    char *newtext  = XmTextGetString (
                         XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ) );


    if ( !newtext || !*newtext )
        {
        // null string entered
        XtFree ( newtext );
        return;
        }

    if ( !addItem ( newtext, edit_selection_dialog_ ) )
        {
        // Newly added item is selected automatically
        if ( curr_selection_ != NULL ) free ( curr_selection_ );
        curr_selection_ = strdup ( newtext );
        showSelectionLabel ( newtext );
        }
    XtFree ( newtext );
    }



void SelectionServer::edit_deleteCB ( Widget, XtPointer clientData, XtPointer )
    {
    SelectionServer *obj = ( SelectionServer * ) clientData;

    obj->edit_delete_cb();
    }

void SelectionServer::edit_delete_cb()
    {
    char *newtext  = XmTextGetString (
                         XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ) );


    if ( !newtext || !*newtext )
        {
        // null string entered
        XtFree ( newtext );
        return;
        }

    XmString xmstr = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( newtext );
    XmListDeleteItem ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_LIST ), xmstr );

    XmStringFree ( xmstr );
    XtFree ( newtext );

    XmTextSetString ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ), "" );
    }



void SelectionServer::edit_selectCB ( Widget, XtPointer clientData, XtPointer )
    {
    SelectionServer *obj = ( SelectionServer * ) clientData;

    obj->edit_select_cb();
    }

void SelectionServer::edit_select_cb()
    {
    char *newtext  = XmTextGetString (
                         XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ) );


    if ( !newtext || !*newtext )
        {
        // null string entered
        XtFree ( newtext );
        return;
        }
    if ( curr_selection_ != NULL ) free ( curr_selection_ );
    curr_selection_ = strdup ( newtext );
    showSelectionLabel ( newtext );

    XtFree ( newtext );

    //   XmTextSetString(XmSelectionBoxGetChild(edit_selection_dialog_, XmDIALOG_TEXT), "");
    }



void SelectionServer::setCurrentSelection ( char *selection )
    {
    removeWhiteSpace ( selection ); // added SRT 950908

    if ( curr_selection_ != NULL ) free ( curr_selection_ );
    curr_selection_ = NULL;

    if ( !selection || !*selection ) // null string entered
        // SRT 950816 return;
        curr_selection_ = strdup ( "\0" );
    else
        curr_selection_ = strdup ( selection );

    showSelectionLabel ( selection );
    }



void SelectionServer::freeSelectionElements ( void ) // added SRT 950816
    {
    XmListDeleteAllItems ( XmSelectionBoxGetChild
                           ( edit_selection_dialog_, XmDIALOG_LIST ) );
    setCurrentSelection ( "" );
    }


void SelectionServer::setDriverWnd ( void *driverWnd ) // 960410 SRT
    {
    driverWnd_ = driverWnd;
    }


// added SRT 960411
void SelectionServer::setScreenPosition ( Position xpos, Position ypos )
    {
    if ( edit_selection_dialog_ ) XtVaSetValues (
            edit_selection_dialog_,
            XmNdefaultPosition,     False,    // so won't be centered
            XtNx,                   xpos,
            XtNy,                   ypos,
            NULL );
    }
