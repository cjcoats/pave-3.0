/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: FormulaServer.cc 83 2018-03-12 19:24:33Z coats $
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
// File:    FormulaServer.C
// Author:  K. Eng Pua
// Date:    Jan 12, 1995
///////////////////////////////////////////////////////////////////////////////
//
// --------------------
// Modification History
// --------------------
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950831  Added removeAllItems() routine
// SRT  950831  add "this" parameter to "new Formula" call
// SRT  950831  Added removeThisItem() routine
// SRT  950901  Added verifyCurrentSelection() routine
// SRT  960410  Added int itemIsAlreadyAdded(char *item)
// SRT  960410  Added selectThisItemAddingIfNecessary(char *item)
//
//////////////////////////////////////////////////////////////////////////////

#include "FormulaServer.h"
#include "DriverWnd.h"

#if defined(linux)
#include <Xm/DragDrop.h>
#endif

FormulaServer::FormulaServer ( char *name, Widget parent, char *dialog_title,
                               char *file_marker, Widget caseListDialog, BusData *bd,
                               linkedList *datasetList_,
                               linkedList *formulaList_,
                               linkedList *domainList_,
                               linkedList *levelList_
                             ) :
    SelectLoadSaveServer ( name, parent, dialog_title, file_marker ),
    BtsData()
    {
    assert ( caseListDialog );
    case_list_dialog_ = caseListDialog;

    datasetListP_ =datasetList_;
    formulaListP_ = formulaList_;
    domainListP_ = domainList_;
    levelListP_ = levelList_;

    aliasListP_ = new linkedList;

    fparent_ = parent;

    assert ( bd );
    for_bd_ = bd;

    setCurrentSelection ( "" );
    createEditSelectionDialog();
    }


FormulaServer::~FormulaServer()
    {
    if ( edit_selection_dialog_ )
        {
        XtDestroyWidget ( edit_selection_dialog_ );
        edit_selection_dialog_ = NULL;
        }
    }


int FormulaServer::addItem ( char *item, Widget dialog )
    {
    Formula *formula;
    char tstring[256];

    removeWhiteSpace ( item ); // added SRT 950908

    // IS THIS FORMULA ALREADY ON THE LIST ??
    if ( formula = ( Formula * ) formulaListP_->find ( item ) )
        {
        sprintf ( tstring, "\nFormula '%s' already added!", item );
        if ( !addingFirstTime_ )
            {
            Message error ( parent_, XmDIALOG_ERROR, tstring );
            }
        else
            fprintf ( stderr, "%s\n", tstring );
        return 1;
        }

    // ITS NOT ON LIST SO LET'S CREATE AN ITEM AND ADD TO LIST
    tstring[0] = '\0';
    formula = new Formula ( item,
                            for_bd_,
                            datasetListP_,
                            domainListP_,
                            levelListP_,
                            fparent_,
                            ( void * ) this, // added 950831 SRT
                            tstring );
    if (
        ( tstring[0] != '\0' )
        ||
        ( !formula->isFormulaValid ( tstring ) )
    )
        {
        char tstring2[256];
        sprintf ( tstring2, "\nError adding formula '%s':\n%s", item, tstring );
        if ( !addingFirstTime_ )
            {
            Message error ( parent_, XmDIALOG_ERROR, tstring2 );
            }
        else
            fprintf ( stderr, "%s\n", tstring2 );
        delete formula;
        return 1;
        }
#ifdef DIAGNOSTICS
    else
        fprintf ( stderr, "FormulaServer::addItem() Successfully added %s\n", item );
#endif // DIAGNOSTICS

    formulaListP_->addTail ( formula );
    formula = NULL;
    addItemToOrderedList ( item, dialog );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Exiting FormulaServer::addItem() with formulaList:\n" );
    formulaListP_->print ( stderr );
#endif // DIAGNOSTICS

    if ( driverWnd_ ) // SRT 960425 will write out ~/.pave* files
        ( ( DriverWnd * ) driverWnd_ )->writeHistoryFile();

    return 0;
    }



int FormulaServer::removeAllItems ( Widget dialog )
    {
    XmListDeleteAllItems ( XmSelectionBoxGetChild ( dialog, XmDIALOG_LIST ) );
    formulaListP_->freeContents();
    setCurrentSelection ( "" );
    return 0;
    }


// removes this string from the selection dialog box if it is there
// called by a formula upon invalidating itself
int FormulaServer::removeThisItem ( char *item )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter FormulaServer::removeThisItem('%s')\n", item );
#endif // DIAGNOSTICS

    if (!item || !*item)
       return 0;
    else if (formulaListP_->find(item) == 0)
       return 0;

    XmString xmstr = XmStringCreateLocalized ( item );
    XmListDeleteItem ( XmSelectionBoxGetChild ( edit_selection_dialog_,
                       XmDIALOG_LIST ), xmstr );
    XmStringFree ( xmstr );
    XmTextSetString ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ),
                      "" );

    return 0;
    }


// returns nonzero if item is already added, otherwise 0
int FormulaServer::itemIsAlreadyAdded ( char *item )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter FormulaServer::itemIsAlreadyAdded('%s')\n", item );
#endif // DIAGNOSTICS

    if ( !item || !*item ) return 0;
    // return ((int)(formulaListP_->find(item)));
    return ( formulaListP_->find ( item ) ? 1 : 0 );
    }


int FormulaServer::selectThisItemAddingIfNecessary ( char *item ) // 960410 SRT
    {
    if ( !itemIsAlreadyAdded ( item ) )
        {
        if ( !addItem ( item, edit_selection_dialog_ ) )
            setCurrentSelection ( item );
        else
            return 1;
        }
    else
        setCurrentSelection ( item );
    return 0;
    }


void FormulaServer::edit_delete_cb()
    {
    Formula *formula;
    char *newtext  = XmTextGetString (
                         XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ) );

    if ( !newtext || !*newtext )
        {
        // null string entered
        XtFree ( newtext );
        return;
        }

    XmString xmstr = XmStringCreateLocalized ( newtext );
    XmListDeleteItem ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_LIST
                                              ), xmstr );

    XmStringFree ( xmstr );

    // FIND AND DELETE THIS FORMULA FROM THE LIST - SRT 950901
    if ( formula = ( Formula * ) formulaListP_->findAndRemoveLink ( newtext ) )
        {
#ifdef DIAGNOSTICS
        fprintf ( stderr, "\nFound and removed this (now deleting):\n" );
        formula->print ( stderr );
#endif // DIAGNOSTICS

        delete formula;
        formula = NULL;
        }

    XtFree ( newtext );

    XmTextSetString ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT )
                      , "" );

    verifyCurrentSelection();

    if ( driverWnd_ ) // SRT 960425 will write out ~/.pave* files
        ( ( DriverWnd * ) driverWnd_ )->writeHistoryFile();
    }


int FormulaServer::parseSuccess()
    {
    if ( parseFormula ( formulaStr_, caseList_,
                        hostList_, for_bd_,
                        errString_, postFixQueue_,
                        caseUsed_, whichUnit_,
                        &dim_, &dateDay_, &dateMonth_, &dateYear_,
                        &hourStart_, &mixCase_, &imax_, &jmax_, &kmax_ ) )
        {

        Message error ( parent_, XmDIALOG_ERROR, "Formula invalid.\nCheck the case and species." );
        return ( 0 );
        }
    return ( 1 );
    }


void FormulaServer::getCaseList()
    {
    int i;
    XmString *strlist;
    // Get the current entries (and number of entries) from the list
    XtVaGetValues ( XmSelectionBoxGetChild ( case_list_dialog_, XmDIALOG_LIST ),
                    XmNitemCount,   &caseCount_,
                    XmNitems,       &strlist,
                    NULL );

    if ( caseCount_ > 0 )
        {
        char *text;
        int len = 0;

        // Count the total length of string needed
        for ( i = 0; i < caseCount_; i++ )
            {
            XmStringGetLtoR ( strlist[i], XmSTRING_DEFAULT_CHARSET, &text );
            len += strlen ( text ) + 1;
            XtFree ( text );
            }
        // Allocate memory for string. WILL PROVIDE EXCEPTION HANDLING LATER...
        caseList_ = new char[len];

        // Assume each host name is no longer than 100 characters
        // WILL PROVIDE EXCEPTION HANDLING LATER...
        hostList_ = new char[caseCount_*100];

        // Writing to string
        caseList_[0] = '\0';
        hostList_[0] = '\0';
        char *p;
        for ( i = 0; i < caseCount_; i++ )
            {
            XmStringGetLtoR ( strlist[i], XmSTRING_DEFAULT_CHARSET, &text );

            p = strtok ( text, ":" );
            strcat ( hostList_, p );
            strcat ( hostList_, "," );

            p = strtok ( NULL, " " );
            strcat ( caseList_, p );
            strcat ( caseList_, "," );

            XtFree ( text );
            }

        // Get rid of last "," at the end of string
        len = strlen ( caseList_ );
        caseList_[len-1] = '\0';

        len = strlen ( hostList_ );
        hostList_[len-1] = '\0';

        }
    else
        {
        Message message ( parent_, XmDIALOG_WARNING, "No item in case list." );
        }
    }


Widget FormulaServer::createEditSelectionDialog()
    {
    char strbuf[100];

    sprintf ( strbuf, "Enter New %s:", title_ );

    XmString label = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( strbuf );

    sprintf ( strbuf, "Add/Delete/Select %s", title_ );
    edit_selection_dialog_ = XmCreateSelectionDialog ( parent_, strbuf, NULL, 0 );
    XtVaSetValues ( edit_selection_dialog_,
                    XmNautoUnmanage,                False,
                    XmNselectionLabelString,        label,
                    XmNheight,              320, // SRT 960411
                    NULL );
    XtAddCallback ( edit_selection_dialog_, XmNapplyCallback, &SelectionServer::edit_addCB, ( XtPointer ) this );
    XtAddCallback ( edit_selection_dialog_, XmNhelpCallback, &SelectionServer::edit_deleteCB, ( XtPointer ) this );
    XtAddCallback ( edit_selection_dialog_, XmNcancelCallback, &SelectionServer::edit_selectCB, ( XtPointer ) this );

    XmStringFree ( label );

#if defined(linux)
    Arg args[1];
    int n=0;
    Widget text_dialog = XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT );
    XtSetArg ( args[n], XmNdropSiteActivity, XmDROP_SITE_INACTIVE );
    n++;

    XmDropSiteUpdate ( text_dialog, args, n );

#endif
    // Change OK button to Close
    label = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( "Close" );
    XtVaSetValues ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_OK_BUTTON ),
                    XmNlabelString, label,
                    NULL );
    XmStringFree ( label );

    // Change Apply button to Add
    label = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( "Add" );
    XtVaSetValues ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_APPLY_BUTTON ),
                    XmNlabelString, label,
                    NULL );
    XmStringFree ( label );

    // Change Cancel button to Close
    label = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( "Close" );
    XtVaSetValues ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_CANCEL_BUTTON ),
                    XmNlabelString, label,
                    NULL );
    XmStringFree ( label );

    // Change Help button to Delete
    label = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( "Delete" );
    XtVaSetValues ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_HELP_BUTTON ),
                    XmNlabelString, label,
                    NULL );
    XmStringFree ( label );


    // SRT 960327 will cause a return to do same as "Add" button (only if the entry has changed)
    XtAddCallback ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ),
                    XmNactivateCallback, &SelectionServer::edit_addCB, ( XtPointer ) this );

    // SRT 960327 when a formula is highlighted, be select it as the current formula
    XtAddCallback ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_LIST ),
                    XmNbrowseSelectionCallback,
                    &SelectionServer::edit_selectCB,
                    ( XtPointer ) this );

    // to make carriage return do the same as add, NOT the same as close
    XtVaSetValues ( edit_selection_dialog_,
                    XmNdefaultButtonType, XmDIALOG_APPLY_BUTTON, NULL );


    // SRT 960327 the "OK" button is no longer needed, since highlighting it will now do the trick
    XtUnmanageChild ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_OK_BUTTON ) );


    return ( edit_selection_dialog_ );
    }



void FormulaServer::verifyCurrentSelection ( void )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter FormulaServer::verifyCurrentSelection\n" );
#endif // DIAGNOSTICS

    if ( getCurrSelection() )
        if ( strlen ( getCurrSelection() ) )
            if ( !formulaListP_->find ( getCurrSelection() ) )
                {
                fprintf ( stderr,
                          "Can't use formula '%s' as default formula - its not on formula list !\n",
                          getCurrSelection() );
                setCurrentSelection ( "" );
                }

#ifdef DIAGNOSTICS
            else
                fprintf ( stderr, "'%s' is a valid selection as its on the formula list\n",
                          getCurrSelection() );
#endif // DIAGNOSTICS
    }



Formula *getSelectedFormula ( FormulaServer *fsp )
    {
    Formula *f = ( Formula * ) NULL;

    if ( fsp->getCurrSelection() )
        if ( strlen ( fsp->getCurrSelection() ) )
            f = ( Formula * ) fsp->formulaListP_->find ( fsp->getCurrSelection() );

    return f;
    }

int FormulaServer::isAliasAdded ( char *item )
    {
    if ( aliasListP_->find ( item ) )
        {
        return 0;
        }
    return 1;
    }




void FormulaServer::addNewAlias ( Alias *alias )
    {
    aliasListP_->addTail ( alias );
    }

void FormulaServer::printAlias ( FILE *output )
    {
    fprintf ( output,"\nCurrent alias list:\n" );
    aliasListP_->print ( output );
    fprintf ( output,"\n" );
    }

void FormulaServer::printAlias ( void )
    {
    printAlias ( stderr );
    }


int FormulaServer::expandAlias ( char *fml, char *item, char c )
    {
    Alias *alias;
    int i;
    int ntoken;
    int *tflag;
    char **token;
    int len;

    if ( ( alias = ( Alias * ) aliasListP_->find ( item ) ) == NULL ) return 0;

    ntoken = alias->getNtoken();
    token  = alias->getToken();
    tflag  = alias->getTflag();

    for ( i=0; i<ntoken; i++ )
        {
        if ( tflag[i] == 0 )
            {
            len = strlen ( fml );
            sprintf ( fml+len,"%s",token[i] );
            }
        else
            {
            if ( !expandAlias ( fml, token[i], c ) )
                {
                len = strlen ( fml );
                sprintf ( fml+len,"%s%c",token[i],c );
                }
            }
        }
    return 1;
    }

int FormulaServer::findAndRemoveAlias ( char *item )
    {
    Alias *alias;

    if ( ( alias= ( Alias * ) aliasListP_->findAndRemoveLink ( item ) ) == NULL ) return 0;
    delete alias;
    return 1;
    }


void FormulaServer::printAliasList ( FILE *fp )
    {
    aliasListP_->print ( fp );
    }


int FormulaServer::checkAlias ( linkedList *currAlist, char *item )
    {
    Alias *alias;
    int i;
    int ntoken;
    int *tflag;
    char **token;
    int len;
    int ret, k;

    if ( ( alias = ( Alias * ) aliasListP_->find ( item ) ) == NULL ) return 0;

    if ( ( ( Alias * ) currAlist->find ( item ) ) != NULL )
        {

        fprintf ( stderr,"ERROR: Recursive reference to alias '%s'\n", item );
        currAlist->print ( stderr );
        return -1;
        }
    currAlist->addTail ( alias );

    ntoken = alias->getNtoken();
    token  = alias->getToken();
    tflag  = alias->getTflag();

    ret = 0;
    for ( i=0; i<ntoken; i++ )
        {
        len = strlen ( token[i] );
        if ( tflag[i] == 0 )
            {
            ret += len;
            }
        else
            {
            if ( ( k=checkAlias ( currAlist, token[i] ) ) >=0 )
                {
                ret += k;
                }
            else return -1;
            }
        }
    currAlist->findAndRemoveLink ( alias );
    return ret;
    }

