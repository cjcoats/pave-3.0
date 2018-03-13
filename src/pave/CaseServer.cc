/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: CaseServer.cc 83 2018-03-12 19:24:33Z coats $
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
// File:    CaseServer.C
// Author:  K. Eng Pua
// Date:    March 23, 1995
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950607  Added linked list and speciesP args/variables and
//      modified add_item()
// SRT  950831  Added removeAllItems() routine
// SRT  950901  Added verifyCurrentSelection() routine
// SRT  950905  Added setFormulaServer() routine
// SRT  960410  Added VIS_DATA *CaseServer::get_dataSets_VISDATA(char *item)
// SRT  960410  Added void updateSpecies_cb(void)
//      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
//
//////////////////////////////////////////////////////////////////////////////

extern "C" {
    int renameFormula ( char *, int, char * );
    }

#include "CaseServer.h"

static void call_Browser ( struct BusData *bd, char *pathname, void *cs );
static void BrowserOK_callback ( struct BusData *bd, struct BusMessage *bmsg );
static void BrowserCancel_callback ( struct BusData *bd, struct BusMessage *bmsg );

CaseServer::CaseServer ( char *name,
                         Widget parent,
                         char *dialog_title,
                         char *file_marker,
                         linkedList *datasetList_,
                         linkedList *formulaList_,
                         linkedList *domainList_,
                         linkedList *levelList_,
                         SpeciesServer *species_,
                         struct BusData *bd,
                         AppInit *app
                       ) :
    SelectLoadSaveServer ( name, parent, dialog_title, file_marker )
    {
    datasetListP_ = datasetList_;
    formulaListP_ = formulaList_;
    domainListP_ = domainList_;
    levelListP_ = levelList_;
    speciesP_ = species_;
    formulaS_ = NULL;
    parent_ = parent;
    app_ = app;
    bd_ = bd;
    createEditSelectionDialog();
    setCurrentSelection ( "" );
    }

static LocalFileBrowser *localBrowser = NULL;


CaseServer::~CaseServer()
    {
    if ( edit_selection_dialog_ )
        {
        XtDestroyWidget ( edit_selection_dialog_ );
        edit_selection_dialog_ = NULL;
        }
    }


int CaseServer::setFormulaServer ( FormulaServer *formulaS )
    {
    formulaS_ = formulaS;
    return 0;
    }


VIS_DATA *CaseServer::get_dataSets_VISDATA ( char *item )
    {
    char tstring[512];
    dataSet *dataset;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter CaseServer::get_dataSets_VISDATA() with '%s'\n", item );
#endif // DIAGNOSTICS

    removeWhiteSpace ( item ); // added SRT 950908

    if ( !strlen ( item ) )
        return NULL;

    // find the dataset
    if ( ! ( dataset = ( dataSet * ) datasetListP_->find ( item ) ) )
        {
        sprintf ( tstring, "\nDataset '%s' not on the dataset list!", item );
        Message error ( parent_, XmDIALOG_ERROR, tstring );
        return ( VIS_DATA * ) NULL;
        }

    // is the dataset valid?
    if ( !dataset->isValid ( bd_, tstring ) )
        {
        Message error ( parent_, XmDIALOG_ERROR, tstring );
        return ( VIS_DATA * ) NULL;
        }

    // return the pointer
    return dataset->get_vdata_ptr();
    }


int CaseServer::addItem ( char *item, Widget dialog )
    {
    char tstring[512];
    dataSet *dataset;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter CaseServer::addItem() with '%s'\n", item );
#endif // DIAGNOSTICS

    removeWhiteSpace ( item ); // added SRT 950908

    // IS THIS DATASET ALREADY ON THE LIST ??
    if ( dataset = ( dataSet * ) datasetListP_->find ( item ) )
        {
        sprintf ( tstring, "\nDataset '%s' already added!", item );
        if ( !addingFirstTime_ )
            {
            Message error ( parent_, XmDIALOG_ERROR, tstring );
            }
        else
            fprintf ( stderr, "%s\n", tstring );
        return 1;
        }

    if ( getItemCount() == 26 )
        {
        Message error (  parent_, XmDIALOG_ERROR,
                         "Can't add more than 26 datasets!" );
        return 1;
        }

    // ITS NOT ON LIST SO LET'S CREATE AN ITEM AND ADD TO LIST
    dataset = new dataSet ( item,
                            formulaListP_,
                            domainListP_,
                            levelListP_,
                            datasetListP_,
                            speciesP_,
                            app_,
                            parent_ );
    assert ( dataset );
    if ( !dataset->isValid ( bd_, tstring ) )
        {
        fprintf ( stderr,"invalid data set\n" );
        char tstring2[256];
        sprintf ( tstring2, "\nError adding '%s':\n%s", item, tstring );
        if ( !addingFirstTime_ )
            {
            Message error ( parent_, XmDIALOG_ERROR, tstring2 );
            }
        else
            fprintf ( stderr, "%s\n", tstring2 );
        delete dataset;
        return 1;
        }
    datasetListP_->addTail ( dataset );
    dataset = ( dataSet * ) NULL;
    notifyFormulasDataSetsWereChanged();
    //fprintf(stderr,"addItemToList\n");
    char copyOfItem[256];
    strcpy ( copyOfItem,item );
    addItemToList ( copyOfItem, dialog );

    //fprintf(stderr,"writehistoryfile\n");
    if ( driverWnd_ ) // SRT 960425 will write out ~/.pave* files
        ( ( DriverWnd * ) driverWnd_ )->writeHistoryFile();
    //fprintf(stderr,"done addItem\n");

    return 0;
    }


int CaseServer::removeAllItems ( Widget dialog )
    {
    XmListDeleteAllItems ( XmSelectionBoxGetChild ( dialog, XmDIALOG_LIST ) );
    datasetListP_->freeContents();
    notifyFormulasDataSetsWereChanged();
    setCurrentSelection ( "" );
    return 0;
    }


void CaseServer::edit_addCB ( Widget parent, XtPointer clientData, XtPointer ) // will replace the one from SelectionServer.cc
    {
    static char home_dir[256];
    static int first_time = 1;

    CaseServer *obj = ( CaseServer * ) clientData;

    if ( first_time )
        {
        sprintf ( home_dir, "%s", getenv ( "HOME" ) );
        first_time = 0;

        localBrowser = new LocalFileBrowser
        ( parent, ( char * ) "Local Files", ( char * ) NULL, ( void * ) NULL,
          ( void * ) & ( CaseServer::localFileCB ), ( void * ) obj );
        }

    if ( strlen ( getenv ( "BUS_MASTER_EXE" ) ) == 0 )
        {
        localBrowser->postLoadSelectionDialog();
        }
    else
        {
        call_Browser ( obj->bd_, home_dir, ( void * ) obj );
        }
    // obj->edit_add_cb();
    }


Widget CaseServer::createEditSelectionDialog()
    {
    char strbuf[100];

    sprintf ( strbuf, "Enter New %s:", title_ );
    XmString label = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( strbuf );

    //XmString *xmstr = new XmString[_selection_list->Count()];
    //for (int i = 0; i < _selection_list->Count(); i++)
    //   xmstr[i] = XmStringCreateSimple(_selection_list->Get(i));

    sprintf ( strbuf, "Add/Delete/Select %s", title_ );
    edit_selection_dialog_ = XmCreateSelectionDialog ( parent_, strbuf, NULL, 0 );

    XtVaSetValues ( edit_selection_dialog_,
                    XmNautoUnmanage,                False,
                    XmNselectionLabelString,        label,
                    XmNheight,                      320, // SRT 960411
                    NULL );

    XtAddCallback ( edit_selection_dialog_, XmNapplyCallback,
                    // &SelectionServer::edit_addCB, // SRT 950921
                    &CaseServer::edit_addCB, // SRT 950921
                    ( XtPointer ) this );
    XtAddCallback ( edit_selection_dialog_, XmNhelpCallback, &SelectionServer::edit_deleteCB, ( XtPointer ) this );
    XtAddCallback ( edit_selection_dialog_, XmNcancelCallback, &SelectionServer::edit_selectCB, ( XtPointer ) this );


    XmStringFree ( label );


    // Change OK button to Close
    label = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( "Close" );
    XtVaSetValues ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_OK_BUTTON ),
                    XmNlabelString, label,
                    NULL );
    XmStringFree ( label );


    // Change apply button to Add
    label = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( "Add" );
    XtVaSetValues ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_APPLY_BUTTON ),
                    XmNlabelString, label,
                    NULL );
    XmStringFree ( label );

    // Change Help button to Delete
    label = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( "Delete" );
    XtVaSetValues ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_HELP_BUTTON ),
                    XmNlabelString, label,
                    NULL );
    XmStringFree ( label );

    // Change Cancel button to Select
    label = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( "Select" );
    XtVaSetValues ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_CANCEL_BUTTON ),
                    XmNlabelString, label,
                    NULL );
    XmStringFree ( label );

    XtVaSetValues ( XtParent ( edit_selection_dialog_ ),
                    XmNautoUnmanage,    False,
                    NULL );

    // SRT 960327 will cause a return to do same as "Add" button (only if the entry has changed)
    XtAddCallback ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ),
                    XmNactivateCallback, &SelectionServer::edit_addCB, ( XtPointer ) this );

    // SRT 960327 when a formula is highlighted, be select it as the current formula
    XtAddCallback ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_LIST ),
                    XmNbrowseSelectionCallback,
                    &SelectionServer::edit_selectCB,
                    ( XtPointer ) this );

    // SRT 960327 the "Select" button is no longer needed, since highlighting it will now do the trick
    XtUnmanageChild ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_CANCEL_BUTTON ) );

    // SRT 960327, since user's typing in a Dataset name is no longer needed
    XtUnmanageChild ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_SELECTION_LABEL ) );
    XtUnmanageChild ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ) );

    return ( edit_selection_dialog_ );
    }


void CaseServer::notifyFormulasDataSetsWereChanged()
    {
    notifyFormulasDataSetsWereChanged ( -1 );
    }

void CaseServer::notifyFormulasDataSetsWereChanged ( int dsId )
    {
    Formula *formula;
    char tstring[255];
    int rmstat;
    char *fmlname;
    int i, nrepl;
    char newname[512];
    char *newf[1028];

#ifdef DIAGNOSTICS
    fprintf ( stderr, "CaseServer::notifyFormulasDataSetsWereChanged() formulaListP_ BEFORE nofitication:\n" );
    formulaListP_->print ( stderr );
#endif // DIAGNOSTICS

    // NOTIFY EACH FORMULA THAT WE'VE CHANGED THE DATASETLIST
    formula = ( Formula * ) formulaListP_->head();

    nrepl = 0;
    while ( formula )
        {

        fmlname = formula->getFormulaName();

#ifdef DIAGNOSTICS
        fprintf ( stderr, "Notifying ..." );
        if ( formula->getFormulaName() )
            fprintf ( stderr, "'%s'...\n", formula->getFormulaName() );
        else
            fprintf ( stderr, "[NULL FORMULA]...\n" );
#endif  // DIAGNOSTICS


        if ( dsId >= 0 )
            {
            if ( formula->dataSetListWasChanged ( &rmstat, tstring ) )
                {
                if ( !addingFirstTime_ )
                    {
                    Message error ( parent_, XmDIALOG_ERROR, tstring );
                    }
                else
                    fprintf ( stderr, "%s\n", tstring );
                }
            if ( rmstat )
                {
                if ( renameFormula ( fmlname, dsId, newname ) )
                    {
                    newf[nrepl++] = strdup ( newname );
                    }
                }
            }
        formula = ( Formula * ) formulaListP_->next();
        }

    verifyCurrentSelection();
    formulaS_->verifyCurrentSelection();

    for ( i = 0; i < nrepl; i++ )
        {
        if ( formulaS_->addItem ( newf[i], formulaS_->getSelectionDialog() ) )
            {
            sprintf ( tstring, "Failure adding formula '%s'!!", newf[i] );
            }
        formulaS_->setCurrentSelection ( newf[i] );
        formulaS_->verifyCurrentSelection();
        free ( newf[i] );
        }



#ifdef DIAGNOSTICS
    formula = ( Formula * ) formulaListP_->head();
    while ( formula )
        {
        fprintf ( stderr, "DEBUG: FORMULA '%s' still here\n", formula->getFormulaName() );
        formula = ( Formula * ) formulaListP_->next();
        }
    fprintf ( stderr, "CaseServer::notifyFormulasDataSetsWereChanged() formulaListP_ AFTER nofitication:\n" );
    formulaListP_->print ( stderr );
#endif // DIAGNOSTICS

    }

void CaseServer::edit_delete_cb()
    {

    int i, found;
    dataSet *dataset;
    char *newtext  = XmTextGetString (
                         XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT ) );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter CaseServer::edit_delete_cb() with '%s'\n", newtext );
#endif // DIAGNOSTICS

    if ( !newtext || !*newtext )
        {
        // null string entered
        XtFree ( newtext );
        return;
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "CaseServer::edit_delete_cb() datasetListP_ BEFORE REMOVAL:\n" );
    datasetListP_->print ( stderr );
#endif // DIAGNOSTICS

    dataset = ( dataSet * ) datasetListP_->head();
    i = found = 0;
    while ( dataset )
        {
        if ( !strcmp ( newtext, dataset->getFullName() ) )
            {
            found = 1;
            break;
            }
        i++;
        dataset = ( dataSet * ) datasetListP_->next();
        }
    // FIND AND DELETE THIS DATASET FROM THE LIST
    if ( dataset = ( dataSet * ) datasetListP_->findAndRemoveLink ( newtext ) )
        {
#ifdef DIAGNOSTICS
        fprintf ( stderr, "\nCaseServer::edit_delete_cb() Found and removed this (now deleting):\n" );
        dataset->print ( stderr );
#endif // DIAGNOSTICS

        delete dataset;
        dataset = ( dataSet * ) NULL;
        }
#ifdef DIAGNOSTICS
    fprintf ( stderr, "CaseServer::edit_delete_cb() datasetListP_ AFTER REMOVAL:\n" );
    datasetListP_->print ( stderr );
#endif // DIAGNOSTICS

    XmString xmstr = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( newtext );
    XmListDeleteItem ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_LIST
                                              ), xmstr );

    XmStringFree ( xmstr );
    XtFree ( newtext );

    XmTextSetString ( XmSelectionBoxGetChild ( edit_selection_dialog_, XmDIALOG_TEXT )
                      , "" );

    if ( found ) notifyFormulasDataSetsWereChanged ( i );

    if ( driverWnd_ ) // SRT 960425 will write out ~/.pave* files
        ( ( DriverWnd * ) driverWnd_ )->writeHistoryFile();
    }


void CaseServer::verifyCurrentSelection ( void )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter CaseServer::verifyCurrentSelection\n" );
#endif // DIAGNOSTICS

    if ( getCurrSelection() )
        if ( strlen ( getCurrSelection() ) )
            if ( !datasetListP_->find ( getCurrSelection() ) )
                {
                fprintf ( stderr,
                          "Can't use dataset '%s' as default dataset - its not on dataset list !\n",
                          getCurrSelection() );
                setCurrentSelection ( "" );
                }

#ifdef DIAGNOSTICS
            else
                fprintf ( stderr, "'%s' is a valid selection as its on the list\n",
                          getCurrSelection() );
#endif // DIAGNOSTICS
    }



static CaseServer *cs_ = ( CaseServer * ) NULL;
static char *pname = NULL;


/*
 * This is called when the Add button on the datasets window is clicked
 * Function that sends a BROWSER_SEIZE message to the Browser with the
 * 'pathname' - the directory you want the Browser to display initially.
 * 'pathname' takes the form hostname:path - If the hostname is absent,
 * then the local host is assumed
 */
static void call_Browser ( struct BusData *bd, char *pathname, void *cs )
    {
    int browserId;
    struct BusMessage seizeMsg;
    static char errorStr[256] =
        "ERROR from CallTheBrowser() : could not start local "
        "portion of remote file browser in six seconds.";

    pname = pathname;
    cs_ = ( CaseServer * ) cs;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter call_Browser()\n" );
#endif /*DIAGNOSTIC$*/

    /* if no BUS_MASTER_EXE was specified, use a local file browser */
    while ( BusVerifyClient ( bd, NULL, "Browser", 1, 10, NULL, errorStr ) )
        {
        sleep ( 6 );  /* Wait another 6-sec. increment for browser to start,
                      before sending it the BROWSER_SEIZE message.
                      BusVerifyClient returns 0 if the local portion is already
                      running, and nonzero if it is not yet running.
                      Browser (local portion) *should* be running already,
                      but I'm just being cautious. */
        BusVerifyClient ( bd, NULL, "Browser", 1, 10, NULL, errorStr );
        }


    /* Send a BROWSER_SEIZE message */
    browserId = BusFindModuleByName ( bd, "Browser" );

    if ( browserId != FIND_ID_ERR )
        {
        seizeMsg.toModule = browserId;
        seizeMsg.fromModule = bd->moduleId;
        seizeMsg.messageType = BusFindTypeByName ( bd, "BROWSER_SEIZE" );

        /* printf("Path = %s \n", pathname); */
        seizeMsg.messageLength = strlen ( pathname )+1;
        seizeMsg.message = pathname;
        BusSendMessage ( bd, &seizeMsg );
        }
    else
        {
        fprintf ( stderr, "Browser not running !! \n" );
        }
    }


void init_Browser ( struct BusData *bd )
    {
    FILE *fp;
    int typeId;
    struct stat statbuf;
    char tmp_name[256];
    static char errorStr[256] =
        "ERROR from CallTheBrowser() : could not start local "
        "portion of remote file browser in six seconds.";

    /* Add the following lines in your initialization routine or the place
       where your progam makes a call to BusXtInitialize/BusInitialize */

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter init_Browser()\n" );
#endif /*DIAGNOSTIC$*/
    if ( strlen ( getenv ( "BUS_MASTER_EXE" ) ) == 0 )
        {
        return;
        }
    if ( !getenv ( "SBUS_EXEC_RC" ) )
        {
        static char rc[256];

        /*
        rc is declared static as per the man pages
        for putenv's mention of a potential error:

        int putenv(char *string)

        A potential error is to  call  putenv()  with  an  automatic
        variable  as  the  argument,  then exit the calling function
        while string is still part of the environment.
        */

        sprintf ( rc, "SBUS_EXEC_RC=%s/.pave_exec_rc", getenv ( "HOME" ) );

#ifdef DIAGNOSTICS
        fprintf ( stderr, "setting %s\n", rc );
#endif /* DIAGNOSTICS */

        putenv ( rc );
        }

    sprintf ( tmp_name, "%s", getenv ( "SBUS_EXEC_RC" ) );
    if ( ( stat ( tmp_name, &statbuf ) ) ||
            ( ! ( fp=fopen ( tmp_name, "r" ) ) ) )
        {
        fprintf ( stderr,
                  "\n\nWARNING - couldn't find and/or open an SBUS_EXEC_RC file!"
                  "\nIf PAVE doesn't work as expected, exit and then set one up."
                  "\nThis can be done either in the file ~/.pave_exec_rc, or"
                  "\nanywhere else.  If it is not done in the file ~/.pave_exec_rc,"
                  "\nyou'll need to do a setenv SBUS_EXEC_RC <that file> before"
                  "\nbooting PAVE."
                  "\nFYI, here are the contents of an example ~/.pave_exec_rc file:"
                  "\n########################################################################"
                  "\n# $PAVE_DIR/.pave_exec_rc"
                  "\n# The file corresponding to env variable SBUS_EXEC_RC"
                  "\n# This file contains the executable name for a corresponding module name"
                  "\n# If you want to customize the file, copy the file into your work area"
                  "\n#    setenv SBUS_EXEC_RC \"pathname of file in your work area\""
                  "\n########################################################################"
                  "\nHelp $PAVE_DIR/scripts/mosaic.wrapper"
                  "\n# Uncomment this next line if you want to run the Browser without the Console"
                  "\n#Browser $PAVE_DIR/scripts/browser.wrapper"
                );
        }
    else
        fclose ( fp );

    if ( !getenv ( "SBUS_RLOGIN_RC" ) )
        {
        static char rc[256];

        /*
        rc is declared static as per the man pages
        for putenv's mention of a potential error:

        int putenv(char *string)

        A potential error is to  call  putenv()  with  an  automatic
        variable  as  the  argument,  then exit the calling function
        while string is still part of the environment.
        */

        sprintf ( rc, "SBUS_RLOGIN_RC=.pave_rlogin_rc" );

#ifdef DIAGNOSTICS
        fprintf ( stderr, "setting %s\n", rc );
#endif /* DIAGNOSTICS */

        putenv ( rc );
        }

    sprintf ( tmp_name, "%s/%s", getenv ( "HOME" ), getenv ( "SBUS_RLOGIN_RC" ) );
    if ( ( stat ( tmp_name, &statbuf ) ) ||
            ( ! ( fp=fopen ( tmp_name, "r" ) ) ) )
        {
#ifdef DIAGNOSTICS
        fprintf ( stderr,
                  "\n \n WARNING - couldn't find and/or open an SBUS_RLOGIN_RC file!"
                  "\n If PAVE doesn't work as expected when using remote files, "
                  "\n exit and then set one up.  This can be done either in the file "
                  "\n ~/.pave_rlogin_rc, or anywhere else in your $HOME directory "
                  "\n tree. If it is not done in the file ~/.pave_rlogin_rc, you'll need"
                  "\n to do a setenv SBUS_RLOGIN_RC <that file relative to $HOME>"
                  "\n before booting PAVE. MAKE SURE THE FOLLOWING EXECUTABLES ARE"
                  "\n IN YOUR PATH ON THE REMOTE MACHINE for PAVE to read remote data:"
                  "\n visd and busd.  You can verify this by using the following"
                  "\n command on the local machine (where PAVE itself runs)"
                  "\n   rsh <REMOTE MACHINE NAME> -l <REMOTE USER ID> which visd busd"
                  "\n If you get \"permission denied\", fix your remote .rhosts file"
                  "\n If visd and/or busd aren't found, fix your path as set up in"
                  "\n the beginning of your remote machine's .cshrc file so it finds them.\n "
                  "\n FYI, here are the contents of an example ~/.pave_rlogin_rc file:"
                  "\n ####################################################################"
                  "\n # $HOME/.pave_rlogin_rc file contains the userids for remote machine"
                  "\n # names.  If remote userid is same as local, you don't need to list it."
                  "\n # Lines preceded by a '#' are ignored."
                  "\n # The format of the contents in the file are:"
                  "\n # <machine-name><space><userid>"
                  "\n # The machine name can be the entire name or without the domain name"
                  "\n # (eg. nox, rain)"
                  "\n ####################################################################"
                  "\n #mary.jane.doe doe"
                  "\n sequoia.nesc.epa.gov <YOUR USER ID ON THE REMOTE MACHINE>\n \n "
                );
#endif // DIAGNOSTICS
        }
    else
        fclose ( fp );

    typeId = BusFindTypeByName ( bd, "BROWSER_REPLY" );
    BusAddTypeCallback ( bd, typeId, BrowserOK_callback );

    typeId = BusFindTypeByName ( bd, "BROWSER_CANCEL" );
    BusAddTypeCallback ( bd, typeId, BrowserCancel_callback );

    while ( BusVerifyClient ( bd, NULL, "Browser", 1, 10, NULL, errorStr ) )
        {
        sleep ( 6 );  /* Wait another 6-sec. increment for browser to start,
                      before sending it the BROWSER_SEIZE message.
                      BusVerifyClient returns 0 if the local portion is already
                      running, and nonzero if it is not yet running.
                      Browser (local portion) *should* be running already,
                      but I'm just being cautious. */
        BusVerifyClient ( bd, NULL, "Browser", 1, 10, NULL, errorStr );
        }
    }


/* When the user clicks OK on the Browser, the browser sends a message of
 * type BROWSER_REPLY, the message field consisting of the hostname:path
 * of the filename selected by the user. This is the callback routine for
 * that message which sends a message to release control of the Browser.
 * Add code to process the filename
 */
static void BrowserOK_callback ( struct BusData *bd, struct BusMessage *bmsg )
    {
    extern char *get_host ( void );

    char host[256], path[256], file[256], fullname[256];
    struct BusMessage relMsg;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter BrowserOK_callback()\n" );
    fprintf ( stderr, "File Selected: %s \n", bmsg->message );
#endif // DIAGNOSTICS

    relMsg.toModule = bmsg->fromModule;
    relMsg.fromModule = bd->moduleId;
    relMsg.messageType = BusFindTypeByName ( bd, "BROWSER_RELEASE" );
    relMsg.messageLength = 0;
    BusSendMessage ( bd, &relMsg );

    /* process the file selected */
    parseLongDataSetName ( bmsg->message, /* full name to be parsed */
                           host,           /* extracted host name goes here */
                           path,           /* extracted path name goes here */
                           file );         /* extracted fname name goes here */

    fullname[0] = '\0';
    if ( host[0] && ( strncmp ( host, get_host(), ( size_t ) strlen ( get_host() ) ) ) )
        sprintf ( fullname, "%s:", host );
    if ( path[0] )
        strcat ( fullname, path );
    strcpy ( pname, fullname ); /* reset pathname for future browser calls */
    if ( path[0] )
        strcat ( fullname, "/" );
    strcat ( fullname, file );

    XmTextSetString ( XmSelectionBoxGetChild
                      ( cs_->getSelectionDialog(), XmDIALOG_TEXT ),
                      fullname );
    cs_->edit_add_cb();
    }


/* This function is called when the user clicks on CANCEL */
static void BrowserCancel_callback ( struct BusData *, struct BusMessage * )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter BrowserCancel_callback()\n" );
#endif // DIAGNOSTICS

    /* No need to send BROWSER_RELEASE message */

    /* Take appropriate action when file selection has been cancelled */
    }



void CaseServer::updateSpecies_cb ( void )
    {
    if ( driverWnd_ ) // SRT 960410 will cause newly selected
        // dataset's variables to show up to user
        ( ( DriverWnd * ) driverWnd_ )->access_species_cb();
    }

void CaseServer::localFileCB ( void *object, char *fname )

    {
    fprintf ( stderr,"in localFileCB\n" );
    CaseServer *obj = ( CaseServer * ) object;
    obj->addLocalFile ( fname );
    }


void CaseServer::addLocalFile ( char *fname )
    {
    char    estring[256];

    fprintf ( stderr,"in addLocalFile fname = %s\n", fname );
    if ( parent_ == NULL )
        fprintf ( stderr,"parent is NULL\n" );
    /*if (addItem(fname, parent_))
        {
        Message error(parent_, XmDIALOG_ERROR, estring);
        }*/
    }

