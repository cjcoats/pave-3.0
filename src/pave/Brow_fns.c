/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Brow_fns.c 83 2018-03-12 19:24:33Z coats $
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
 ****************************************************************************
 *
 *      Brow_fns.c contains the non-motif functions and callbacks for the
 *      Browser client.
 *
 *     INPUT FILES:       stdin stream
 *     OUTPUT FILES:      stdout stream
 *     ERROR HANDLING:    output to stderr (screen)
 *     INPUT PARAMETERS:  client name (ascii string), optional
 *     INPUT OPTIONS:     if client name is not input, default is
 *
 *     NETWORKING:        remote/network connectivity is via bus library (sockets)
 *
 *     COMPILATION:       (see Makefile)
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *
 ********************************************************************
 * REVISION HISTORY - Brow_fns.c
 *
 * Date: 7-Apr-95
 * Version: 0.3a
 * Change Description: ORIGINAL CODE
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 22-June-95
 * Version: 0.4
 * Change Description: Fixed bug for browsing files after checking remote files
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 22-June-95
 * Version: 0.5
 * Change Description: Seize Callback uses BusVerifyClient to start deamons.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 16-Aug-95
 * Version: 0.5.1
 * Change Description: unmapFileList to unmap the list of the browser when
 *    fetching from remote directories
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 19-Aug-95
 * Version: 0.5.2
 * Change Description: When cannot open remote directory, the unmapped filelist
 *         has to be mapped back
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 23-Aug-95
 * Version: 0.5.3
 * Change Description: Replaced instanced of cuserid with BusGetMyUserid
 *    and increased secsToWait in BusVerifyClient to 20sec.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 11-Sept-95
 * Version: 0.5.4
 * Change Description: Set pathname to "/~" by default
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0:  clean-up.
 ********************************************************************/

#include "Browser.h"

char *vcBrow_fns= "$Id: Brow_fns.c 83 2018-03-12 19:24:33Z coats $" ;

extern struct BusData bd;
extern Widget callbackWidget, host, fsb_dialog, toplevel;
extern char   hostname[80], username[];
extern XmStringCharSet char_set;
extern int    callbackTypeId, OwnerModuleId;
extern int    remoteFetch;

static int   num_rem_dir, num_rem_files;
static char *remfileList[MAX_FILES], *remdirList[MAX_FILES];
static char *remDir;


static void UpdateRemoteFileWidget ( Widget widget )
    {
    XmString names[MAX_FILES];   /* maximum of MAX_FILES files in dir */
    XmString dir_names[MAX_FILES];   /* maximum of MAX_FILES files in dir */
    int i;
    XmString xdir;
    char *dir;

    for ( i=0; i<num_rem_dir; i++ )
        dir_names[i] = XmStringCreateSimple ( remdirList[i] );

    if ( num_rem_files )
        {
        for ( i=0; i<num_rem_files; i++ )
            names[i] = XmStringCreateSimple ( remfileList[i] );
        XtVaSetValues ( widget,
                        XmNdirListItems,      dir_names,
                        XmNdirListItemCount,  num_rem_dir,
                        XmNlistUpdated,        True,
                        XmNdirectoryValid,        True,
                        XmNdirSpec,            dir_names[0],
                        XmNfileListItems,      names,
                        XmNfileListItemCount,  i,
                        NULL );
        while ( i >0 )
            {
            XmStringFree ( names[--i] );
            free ( remfileList[i] );
            }
        }
    else
        {

        XtVaSetValues ( widget,
                        XmNdirListItems,      dir_names,
                        XmNdirListItemCount,  num_rem_dir,
                        XmNdirectoryValid,        True,
                        XmNdirSpec,            dir_names[0],
                        XmNfileListItems,      NULL,
                        XmNfileListItemCount,  0,
                        XmNlistUpdated,        True,
                        NULL );
        }

#ifdef DUMMY
    /* Check if the directory is displayed as "/~/" and set the
        filter to the right directory */

    XtVaGetValues ( widget,
                    XmNtextString, &xdir,
                    NULL );
    if ( XmStringGetLtoR ( xdir, char_set, &dir ) )
        {
        printf ( "Remote dir = %s \n", dir );
        if ( ( strlen ( dir ) > 2 ) && ( dir[0] == '/' ) && ( dir[1] == '~' ) )
            {
            char *dirname;

            if ( num_rem_dir > 0 )
                {
                printf ( "Setting the value of rem dir \n" );
                XtVaSetValues ( widget,
                                XmNtextString, dir_names[0],
                                NULL );
                }
            }
        }
#endif

    for ( i=0; i<num_rem_dir; i++ )
        {
        XmStringFree ( dir_names[i] );
        free ( remdirList[i] );
        }
    }

/**************************************************************************/

static void UpdateFileWidget ( Widget widget, int nitems, char **fileList )
    {
    XmString names[MAX_FILES];   /* maximum of MAX_FILES files in dir */
    int i;

    if ( nitems )
        {
        for ( i=0; i<nitems; i++ )
            names[i] = XmStringCreateSimple ( fileList[i] );
        XtVaSetValues ( widget,
                        XmNfileListItems,      names,
                        XmNfileListItemCount,  i,
                        XmNlistUpdated,        True,
                        NULL );
        while ( i >0 )
            {
            XmStringFree ( names[--i] );
            free ( fileList[i] );
            }
        }
    else
        {

        XtVaSetValues ( widget,
                        XmNfileListItems,      NULL,
                        XmNfileListItemCount,  0,
                        XmNlistUpdated,        True,
                        NULL );
        }
    }

/**************************************************************************/

static void UpdateDirWidget ( Widget widget, int nitems, char **fileList )
    {
    XmString names[MAX_FILES];   /* maximum of MAX_FILES files in dir */
    int i;

    if ( nitems )
        {
        for ( i=0; i<nitems; i++ )
            names[i] = XmStringCreateSimple ( fileList[i] );
        XtVaSetValues ( widget,
                        XmNdirListItems,      names,
                        XmNdirListItemCount,  i,
                        XmNlistUpdated,        True,
                        XmNdirSpec,            names[0],
                        XmNdirectoryValid,        True,
                        NULL );
        while ( i >0 )
            {
            XmStringFree ( names[--i] );
            free ( fileList[i] );
            }
        }
    else
        {
        XtVaSetValues ( widget,
                        XmNdirListItems,      NULL,
                        XmNdirListItemCount,  0,
                        XmNlistUpdated,        True,
                        XmNdirectoryValid,        True,
                        NULL );

        }
    }

/**************************************************************************/

void dir_search ( Widget widget,  /* file selection box widget */
                  XtPointer search_data )
    {
    char           *directory, buf[BUFSIZ];
    char           *fileList[MAX_FILES];
    int            i = 0;
    char       s[1024];

    XmFileSelectionBoxCallbackStruct *cbs =
        ( XmFileSelectionBoxCallbackStruct * ) search_data;

    if ( !XmStringGetLtoR ( cbs->dir, XmSTRING_DEFAULT_CHARSET, &directory ) )
        return; /* can't do anything */

    sprintf ( buf, "%s", directory );
    XtFree ( directory );

    i = FTP_getdir ( &bd, hostname, buf, IS_DIR, callbackTypeId, fileList );

    if ( ( i == DIR_OPEN_ERR ) || ( i == DIR_HOST_UNKNOWN ) )
        {
        XtVaSetValues ( widget,
                        XmNlistUpdated,        False,
                        XmNdirectoryValid,     False,
                        NULL );
        sprintf ( s, "*Cannot open %s err=%d", buf,i );
        merror ( s );
        }
    else if ( i == DIR_REMOTE_FETCH )
        {
        remoteFetch = 1;
        callbackWidget = widget;
        remDir = malloc ( strlen ( buf )+1 );
        strcpy ( remDir, buf );
        }
    else
        {
        remoteFetch = 0;
        UpdateDirWidget ( widget, i, fileList );
        }
    }

/**************************************************************************/

static void remote_dir_search ( Widget widget, XtPointer search_data )
    {
    UpdateRemoteFileWidget ( widget );
    }

/**************************************************************************/

static void remote_file_search ( Widget widget, XtPointer search_data )
    {
    }

/**************************************************************************/

void file_search ( Widget widget, XtPointer search_data )
    {
    char           *directory, buf[BUFSIZ];
    char           *fileList[MAX_FILES];
    int            i = 0;
    char       s[BUFSIZ];

    XmFileSelectionBoxCallbackStruct *cbs =
        ( XmFileSelectionBoxCallbackStruct * ) search_data;

    if ( !XmStringGetLtoR ( cbs->dir, XmSTRING_DEFAULT_CHARSET, &directory ) )
        return; /* can't do anything */

    sprintf ( buf, "%s", directory );
    XtFree ( directory );

    /* printf("fil_search for dir : %s \n", buf); */
    i = FTP_getdir ( &bd, hostname, buf, IS_FILE, callbackTypeId, fileList );

    if ( ( i == DIR_OPEN_ERR ) || ( i == DIR_HOST_UNKNOWN ) )
        {
        XtVaSetValues ( widget,
                        XmNlistUpdated,        False,
                        XmNdirectoryValid,     False,
                        NULL );
        sprintf ( s, "**Cannot open %s", buf );
        merror ( s );
        }
    else if ( i == DIR_REMOTE_FETCH )
        {
        remoteFetch = 1;
        callbackWidget = widget;
        }
    else
        {
        remoteFetch = 0;
        UpdateFileWidget ( widget, i, fileList );
        }
    }

/**************************************************************************/

void recv_dir_callback ( struct BusData *bdd, struct BusMessage *bmsg )
    {
    char *dup_msg;
    char *name, *buf;
    int i, j, nfiles, code;

    dup_msg = malloc ( strlen ( bmsg->message )+1 );
    strcpy ( dup_msg, bmsg->message );

    j=0;
    buf = strtok ( dup_msg, " " );
    if ( buf != NULL )
        code = atoi ( buf );
    else
        code = -1;

    buf = strtok ( NULL, " " );
    if ( buf != NULL )
        nfiles = atoi ( buf );
    else
        nfiles = 0;

    /* printf("receive_dir_callback : Number of files recvd = %d Code = %d\n",
            nfiles, code);  */

    /* Check if its a DIR_OPEN error */
    if ( nfiles < 0 )
        {
        char s[BUFSIZ];

        XtSetMappedWhenManaged (
            XmSelectionBoxGetChild ( callbackWidget, XmDIALOG_LIST ), TRUE ) ;
        sprintf ( s, "***Cannot open %s", remDir );
        merror ( s );
        return;
        }

    if ( nfiles > 0 )
        {
        for ( j=0, i=0; i<nfiles; i++ )
            {
            name = strtok ( NULL, " " );
            if ( name != NULL )
                {
                if ( code == IS_DIR )
                    {
                    remdirList[j] = malloc ( strlen ( name )+1 );
                    strcpy ( remdirList[j], name );
                    remdirList[j][strlen ( name )] = '\0';
                    }
                else
                    {
                    remfileList[j] = malloc ( strlen ( name )+1 );
                    strcpy ( remfileList[j], name );
                    remfileList[j][strlen ( name )] = '\0';
                    }
                j++;
                }
            }
        /* Something wrong in the communication line? */
        if ( j != nfiles )
            {
            printf ( "ERROR !! Number of files recvd(%d) does not match nfiles (%d)\n",
                     j, nfiles );
            }
        }

    if ( code == IS_FILE )
        {
        if ( nfiles >= 0 )
            {
            XmString dir = XmStringCreateSimple ( remDir );

            num_rem_files = nfiles;
            XtVaSetValues ( callbackWidget,
                            XmNdirSearchProc, remote_dir_search,
                            XmNfileSearchProc, remote_file_search,
                            NULL );
            /* XmNdirectory, XmStringCreateLtoR(remDir,char_set), */
            XmFileSelectionDoSearch ( callbackWidget, dir );

            XtVaSetValues ( callbackWidget,
                            XmNdirSearchProc, dir_search,
                            XmNfileSearchProc, file_search,
                            NULL );
            }
        XtSetMappedWhenManaged (
            XmSelectionBoxGetChild ( callbackWidget, XmDIALOG_LIST ), TRUE ) ;
        }
    else if ( code == IS_DIR )
        {
        /*
        XtSetMappedWhenManaged(
        XmSelectionBoxGetChild(callbackWidget, XmDIALOG_LIST), FALSE) ;
        */
        num_rem_dir = nfiles;
        if ( num_rem_dir == DIR_OPEN_ERR )
            {
            XtVaSetValues ( callbackWidget,
                            XmNlistUpdated,        False,
                            XmNdirectoryValid,     False,
                            NULL );
            merror ( "Cannot open given directory!" );
            }
        else
            {
            int i;
            XmString dir = XmStringCreateSimple ( remDir );

            i = FTP_getdir ( &bd, hostname, remDir, IS_FILE,
                             callbackTypeId, remfileList );
            /*
            XtVaSetValues( callbackWidget,
            XmNdirSearchProc, file_search,
            NULL);
            printf("Calling XmFileSelectionDoSearch \n");
            XmFileSelectionDoSearch(callbackWidget, dir);
            printf("Resetting the dir_search routine \n");
            XtVaSetValues( callbackWidget,
            XmNdirSearchProc, dir_search,
            NULL);
            */
            }
        /* free(remDir); */
        }
    if ( dup_msg != NULL ) free ( dup_msg );
    }


/**************************************************************************
 ASCIIZ Callback routine that is called by the Bus when a message
 of type "ASCIIZ" is received.
 **************************************************************************/
void ASCIIZ_callback ( struct BusData *bdd, struct BusMessage *bmsg )
    {
    char *from, *to;

    from = BusFindModuleById ( bdd, bmsg->fromModule );
    if ( from == NULL )
        from = "Unidentified source (source no longer connected to bus?)";

    if ( bmsg->toModule == BusBroadcast )
        to   = "Everybody";
    else if ( bmsg->toModule == BusByType )
        to   = "modules understanding type";
    else if ( bmsg->toModule == BusBounce )
        to   = "myself (Bounce)";
    else if ( bmsg->toModule == bdd->moduleId )
        to   = "myself (my module id)";
    else if ( bmsg->toModule != bdd->moduleId )
        to   = BusFindModuleById ( bdd, bmsg->  toModule );
    else
        to = "Unknown (not good...)";

    printf ( "ASCIIZ message received:\n" );
    printf ( "\tFrom: %s\n", from );

    if ( to != NULL )
        printf ( "\tTo  : %s\n", to );

    printf ( "Message = %s \n", bmsg->message );
    fflush ( stdout );
    }


void Seize_callback ( struct BusData *bd, struct BusMessage *bmsg )
    {

    char *tmp_msg, *hname, *pathname, tmp_hname[256], ipaddress[80];
    char s2[256];
    int res;

    /* Check if a module already owns the Browser and if so send a
    message asking the module to try again
     */
    if ( OwnerModuleId != FIND_ID_ERR )
        {
        char buf[32] = "Browser busy: Try Again";

        /*
        new_bmsg.toModule = bmsg->fromModule;
        new_bmsg.fromModule = bd->moduleId;
        new_bmsg.messageType = BROWSER_REPLY;
        new_bmsg.message = buf;
        BusSendMessage(bd, &new_bmsg);
        */
        merror ( buf );
        return;
        }
    gethostname ( tmp_hname, 256 );

#ifdef STRDUP_UNAVAILABLE
    tmp_msg = malloc ( strlen ( bmsg->message )+1 );
    strcpy ( tmp_msg, bmsg->message );
#else
    tmp_msg = strdup ( bmsg->message );
#endif

    /* Check if the char ':' is present in the string */
    if ( strchr ( tmp_msg, ':' ) != NULL ) /* its present */
        {
        hname = strtok ( tmp_msg, ":" );
        if ( hname != NULL )
            {
            pathname = strtok ( NULL, " " );
            if ( pathname == NULL )
                {
                /* Set pathname to the HOME directory */
                pathname = "/~";
                }

            /* Check if its a valid host and the ftp deamon exists on that machine */
            res = is_localhost ( hname, ipaddress );
            if ( res == -1 )  /* Illegal hostname */
                {
                char s1[80];

                sprintf ( s1, "%s: Illegal hostname", hname );
                merror ( s1 );
                hname = tmp_hname;
                }
            else if ( res == 0 )
                {
                char s[256];

                if ( BusVerifyClient ( bd, ipaddress, "busd", 0, 18, NULL, s ) )
                    {
                    merror ( s );
                    hname = tmp_hname;
                    pathname = "~";
                    is_localhost ( hname, ipaddress );
                    }
                }
            }
        else
            {
            pathname = "~";
            hname = tmp_hname;
            gethostname ( hname, 256 );
            is_localhost ( hname, ipaddress );
            }
        }
    else         /* ':' not present in the message string */
        {
        pathname = tmp_msg;
        hname = tmp_hname;
        gethostname ( hname, 256 );
        is_localhost ( hname, ipaddress );
        }

    /* Set the owner Id to the moduleId of the client that sent the request */
    OwnerModuleId = bmsg->fromModule;
    /*
    printf("Browser_callback : Host = %s Path = %s OwnerId = %d \n",
     hname, pathname, OwnerModuleId);
    */

    strcpy ( hostname,hname );
    strcpy ( username,BusGetMyUserid() );

    strcpy ( s2,"Host: " );
    strcat ( s2, hname );
    strcat ( s2," (" );
    strcat ( s2,ipaddress );
    strcat ( s2,")    User: " );
    strcat ( s2,BusGetMyUserid() );
    strcat ( s2,"\nOwner Module: " );
    strcat ( s2, BusFindModuleById ( bd, bmsg->fromModule ) );
    /* printf("string='%s'\n",s2); */
    XtVaSetValues ( host, XmNlabelString,
                    XmStringCreateLtoR ( s2, char_set ),NULL );
    XtVaSetValues ( fsb_dialog, XmNdirectory,
                    XmStringCreateLtoR ( pathname, char_set ),NULL );
    unmapFileList();  /* If remote dir - unmap the file list */
    XMapWindow ( XtDisplay ( toplevel ), XtWindow ( toplevel ) );
    }

/**************************************************************************/

void Release_callback ( struct BusData *bd, struct BusMessage *bmsg )
    {
    char hname[256], ipaddress[256], s2[256];

    /* printf("BROWSER_RELEASE message from : %d \n", bmsg->fromModule); */

    if ( bmsg->fromModule != OwnerModuleId )
        {
        merror ( "Module requesting RELEASE is not owner!!" );
        return;
        }
    /* mmessage("Releasing control of Browser \n"); */
    XUnmapWindow ( XtDisplay ( toplevel ), XtWindow ( toplevel ) );
    OwnerModuleId = FIND_ID_ERR;

    /* Reset the host dialog box and file dialog box to default values */
    gethostname ( hname, 256 );
    is_localhost ( hname, ipaddress );
    strcpy ( hostname,hname );
    strcpy ( username,BusGetMyUserid() );

    strcpy ( s2,"Host: " );
    strcat ( s2, hname );
    strcat ( s2," (" );
    strcat ( s2,ipaddress );
    strcat ( s2,")    User: " );
    strcat ( s2,BusGetMyUserid() );
    strcat ( s2,"\nOwner Module: " );
    strcat ( s2, "NONE specified" );
    /* printf("string='%s'\n",s2); */
    XtVaSetValues ( host, XmNlabelString,
                    XmStringCreateLtoR ( s2, char_set ),NULL );
    XtVaSetValues ( fsb_dialog, XmNdirectory,
                    XmStringCreateLtoR ( "~", char_set ),NULL );
    }
