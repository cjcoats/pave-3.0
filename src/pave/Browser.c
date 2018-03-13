/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Browser.c 83 2018-03-12 19:24:33Z coats $
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
 ********************************************************************
 *                                                                  *
 *                        EDSS Software Bus                         *
 *                                                                  *
 *                     Copyright (c) 1994 by                        *
 *                                                                  *
 *                  North Carolina State University                 *
 *              Department of Computer Science, Box 8206            *
 *                      Raleigh, NC 27695-8206                      *
 *                               and                                *
 *           MCNC, North Carolina Supercomputing Center             *
 *              Environmental Programs, P.O. Box 12889              *
 *                 Research Triangle Park, 27709                    *
 *                                                                  *
 *      Made available under terms of the GNU Public License        *
 *                                                                  *
 ********************************************************************
 *
 * ABOUT:  - Browser.c
 *
 *      Browser.c is a bus client that provides GUI interface for remote
 *      file browsing. It has a text-client counterpart (edss ftp) that needs to
 *      run on the remote machine to allow this.
 *
 *     INPUT FILES:       stdin stream
 *     OUTPUT FILES:      stdout stream
 *     ERROR HANDLING:    output to stderr (screen)
 *     INPUT PARAMETERS:  client name (ascii string), optional
 *     INPUT OPTIONS:     if client name is not input, default is
 *
 *     NETWORKING:        remote/network connectivity is via bus library (sockets)
 *
 *     COMPILATION:       (see makefile)
 *
 *     USE/EXECUTION:
 *
 *     KNOWN BUGS:  :-(
 *
 *     OTHER NOTES: :-)
 *
 ********************************************************************
 * REVISION HISTORY - Browser.c
 *
 * Date: 15-Mar-95
 * Version: 0.1
 * Change Description: ORIGINAL CODE
 * Author:             M. Vouk, NCSU, CSC
 *
 * Date: 25-Mar-95
 * Version: 0.2
 * Change Description: Remote browsing updates to directory and file
 *                     search procedures. Incorporation of merror,
 *                     mwarning and mmessage routines into bus Xt library.
 *
 * Author:             M. Vouk, R. Balay NCSU, CSC
 *
 * Date: 22-June-95
 * Version: 0.4
 * Change Description: The Browser requires 2 environment variables :
 *            BUS_BROWSE_RC and BUS_APPLN_NAME
 *            to obtain the ".rc" file and the name of the application
 *            All references to EDSS/edss have been removed and
 *            the Browser depends on the above env variables for them.
 *            Also, the browser checks for one ".rc" to construct
 *            the HostList and some features of host selection have
 *            been smoothened.
 * Author:             R. Balay, NCSU, CSC
 *
 * Date: 23-June-95
 * Version: 0.5
 * Change Description: The Browser starts daemons dynamically as needed.
 *    The host selection box uses BusFindWhosConnected function to list
 *    existing daemons.
 * Author:             R. Balay, NCSU, CSC
 *
 * Date: 16-Aug-95
 * Version: 0.5.1
 * Change Description: unmapFileList for remote dirs.
 *    Added Exit timeout. 'Cancel' does not exit the Browser.
 * Author:             R. Balay, NCSU, CSC
 *
 * Date: 19-Aug-95
 * Version: 0.5.2
 * Change Description: Allows users to type in the filename of selection
 * Author:             R. Balay, NCSU, CSC
 *
 * Date: 23-Aug-95
 * Version: 0.5.3
 * Change Description: Replaced instances of cuserid with BusGetMyUserid
 *    and increased secsToWait for BusVerifyClient to 20sec.
 * Author:             R. Balay, NCSU, CSC
 *
 * Date: 1-Sept-95
 * Version: 0.5.4
 * Change Description:
 *    1. Browser sends a path '/~' to the remote deamon
 *       to display the contents of the home dir on the remote machine.
 *    2. Also modified to  display the file list
 *       in a fixed sized selection box.
 *    3. The Browser is 'unmapped' when started and is mapped when a
 *       'Seize' message received.
 * Author:             R. Balay, NCSU, CSC
 *
 * Date: 26-Sept-95
 * Version: 0.5.5
 * Change Description: The Browser expand the dir "/~/" to the home dir or
 *                     dirList[0] before sending the selected file  name
 *                     to the Owner module.
 * Author:             R. Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0:
 *   BUS_APPLN_NAME = "PAVE"
 ********************************************************************/

#include "Browser.h"

/* Global Variables */

XtAppContext context;
XmStringCharSet char_set=XmSTRING_DEFAULT_CHARSET;

Widget toplevel, form, shell, button, fsb_dialog, hst_dialog, host, user_dialog;
Widget path_dialog;
Widget error_dialog, message_dialog, warning_dialog;
char hostname[80],username[80], tmp_username[80], tmp_pathname[256], ipaddress[80];
int userModified = 0;
int pathModified = 0;
struct BusData bd; /* use BusData structure */
/* Used by the callback to refresh remote files */
Widget callbackWidget, callbackWidget;
int callbackTypeId;
int OwnerModuleId = -1;
char busApplName[] = "PAVE" ;
struct host_item *hostConfigList;
int numHostsConfigured = 0;
int remoteFetch = 0;

/* ==================== Utility routines ===================== */
void merror ( char *message )
    {
    int ac;
    Arg al[10];

    ac = 0;
    XtSetArg ( al[ac], XmNmessageString,
               XmStringCreateLtoR ( message, char_set ) );
    ac++;
    XtSetValues ( error_dialog,al,ac );
    XtManageChild ( error_dialog );
    }
void mmessage ( char *message )
    {
    int ac;
    Arg al[10];

    ac = 0;
    XtSetArg ( al[ac], XmNmessageString,
               XmStringCreateLtoR ( message, char_set ) );
    ac++;
    XtSetValues ( message_dialog,al,ac );
    XtManageChild ( message_dialog );
    }
void mwessage ( char *message )
    {
    int ac;
    Arg al[10];

    ac = 0;
    XtSetArg ( al[ac], XmNmessageString,
               XmStringCreateLtoR ( message, char_set ) );
    ac++;
    XtSetValues ( warning_dialog,al,ac );
    XtManageChild ( warning_dialog );
    }

void user_dialogCB ( w,client_data,call_data )
Widget w;
int client_data;
XmSelectionBoxCallbackStruct *call_data;
/* callback function for the dialog box */
    {
    char *s;

    switch ( client_data )
        {

        case USER_OK:
            XmStringGetLtoR ( call_data->value,char_set,&s );
            /* printf("Username = %s \n", s); */
            userModified = 1;
            if ( strlen ( s ) > 0 )
                strcpy ( tmp_username, s );
            break;

        case USER_CANCEL:
            break;
        }
    XtUnmanageChild ( w );
    }


void path_dialogCB ( w,client_data,call_data )
Widget w;
int client_data;
XmSelectionBoxCallbackStruct *call_data;
/* callback function for the dialog box */
    {
    char *s;

    switch ( client_data )
        {

        case PATH_OK:
            XmStringGetLtoR ( call_data->value,char_set,&s );
            /* printf("Username = %s \n", s); */
            pathModified = 1;
            if ( strlen ( s ) > 0 )
                strcpy ( tmp_pathname, s );
            break;

        case PATH_CANCEL:
            break;
        }
    XtUnmanageChild ( w );
    }

void hst_dialogCB ( Widget w,
                    int client_data,
                    XmSelectionBoxCallbackStruct *call_data     /* callback function for the dialog box */
                  )
    {
    char *s, s2[280];
    int res;

    switch ( client_data )
        {
            char pathname[256];
            Widget host_list;
            XmString item;
            int pos;

        case HST_OK:
            /* get the string from the event structure. */
            XmStringGetLtoR ( call_data->value,char_set,&s );
            /* printf("string='%s'\n",s); */
            if ( strlen ( s ) <=0 )
                {
                /* (void) gethostname(s,256); */
                break;
                }
            res = is_localhost ( s, ipaddress );
            if ( res == -1 )
                {
                merror ( "Illegal hostname" );
                if ( pathModified == 1 )
                    pathModified = 0;
                if ( userModified == 1 )
                    userModified = 0;
                break;
                }
            /* not local host */
            if ( res == 0 )
                {
                char s[ERR_BUFSIZE];

                if ( BusVerifyClient ( &bd, ipaddress, "busd", 0, 15, NULL, s ) )
                    {
                    merror ( s );
                    if ( userModified == 1 )
                        userModified = 0;
                    if ( pathModified == 1 )
                        pathModified = 0;
                    break;
                    }
                }

            /* Update the pathname for current host */
            host_list=XmSelectionBoxGetChild ( hst_dialog, XmDIALOG_LIST );
            item = XmStringCreateLtoR ( hostname, char_set );
            pos = XmListItemPos ( host_list, item );
            if ( pos != 0 )
                {
                XmString dir;
                char *cur_dir;

                XtVaGetValues ( fsb_dialog,
                                XmNtextString, &dir,
                                NULL );
                if ( XmStringGetLtoR ( dir, char_set, &cur_dir ) )
                    {
                    strcpy ( hostConfigList[pos-1].pathname, cur_dir );
                    XtFree ( cur_dir );
                    }
                XmStringFree ( dir );
                }
            XmStringFree ( item );

            strcpy ( hostname,s );
            if ( userModified == 1 )
                {
                strcpy ( username, tmp_username );
                userModified = 0;
                }
            /*sprintf(pathname, "~%s", username); */
            sprintf ( pathname, "/~" );

            if ( pathModified == 1 )
                {
                strcpy ( pathname, tmp_pathname );
                pathModified = 0;
                }
            else
                {
                /* Find the pathname */
                host_list=XmSelectionBoxGetChild ( hst_dialog, XmDIALOG_LIST );
                item = XmStringCreateLtoR ( s, char_set );
                pos = XmListItemPos ( host_list, item );
                if ( pos != 0 )
                    strcpy ( pathname, hostConfigList[pos-1].pathname );
                XmStringFree ( item );
                }

            /* Update the file selection box widget */
            XtVaSetValues ( fsb_dialog,
                            XmNdirectory, XmStringCreateLtoR ( pathname,char_set ),
                            NULL );
            unmapFileList();

            /*
                       XmNdirectoryValid,        True,
                       XmNlistUpdated,        True,
             */
            strcpy ( s2,"Host: " );
            strcat ( s2,s );
            strcat ( s2," (" );
            strcat ( s2,ipaddress );
            strcat ( s2,")    User: " );
            strcat ( s2, username );
            if ( OwnerModuleId != FIND_ID_ERR )
                {
                strcat ( s2, "\n Owner Module: " );
                strcat ( s2, BusFindModuleById ( &bd, OwnerModuleId ) );
                }
            else
                strcat ( s2, "\n Owner Module: NONE specified" );
            /* printf("string='%s'\n",s2); */
            XtVaSetValues ( host, XmNlabelString,
                            XmStringCreateLtoR ( s2,char_set ),NULL );

            XtFree ( s );
            break;
        case HST_USER:
            /* printf("HST USER selected \n"); */
            /* make the dialog box visible */
            XtManageChild ( user_dialog );
            break;

        case HST_PATH:
            /* make the dialog box visible */
            XtManageChild ( path_dialog );
            break;

        case HST_CANCEL:
            /* printf("HOST CANCEL selected\n"); */
            break;
        }
    /* make the dialog box invisible */
    if ( ( client_data != HST_USER ) && ( client_data != HST_PATH ) )
        XtUnmanageChild ( w );
    }

void unmapFileList()
    {
    if ( remoteFetch )
        {
        XtSetMappedWhenManaged (
            XmSelectionBoxGetChild ( fsb_dialog, XmDIALOG_LIST ), FALSE ) ;
        }
    }

void fsb_dialogCB ( Widget w,
                    int client_data,
                    XmSelectionBoxCallbackStruct *call_data     /* callback function for the dialog box */
                  )
    {
    XmFileSelectionBoxCallbackStruct *cbs =
        ( XmFileSelectionBoxCallbackStruct * ) call_data;
    char *s;
    struct BusMessage new_bmsg;


    switch ( client_data )
        {

        case FSB_OK:
            /* a new file was selected -- check to see if it is a directory.
               If a directory, scan it just as tho the user had typed it in
               the mask Text field and selected "Search".  */
            /* get the string typed in the text field in char * format */
            if ( !XmStringGetLtoR ( call_data->value,char_set,&s ) )
                return;  /* hosed */
            /* XtVaGetValues(fsb_dialog, XmNtextString, &s, NULL); */
            /*
            printf("string='%s'\n",s);
            printf("Host='%s'\n",hostname);
            printf("User='%s'\n",username);
            */

            if ( s[strlen ( s )-1] != '/' )
                {
                /* if it's not a directory, determine the full pathname
                 * of the selection by concatenating it to the "dir" part
                 */
                char *dir, *newfile;
                if ( *s != '/' )
                    {
                    if ( XmStringGetLtoR ( cbs->dir, char_set, &dir ) )
                        {
                        /* printf("Dir = %s \n", dir); */
                        /* if the dir is "/~/" expand it to the home dir */
                        if ( ! strcmp ( dir, "/~/" ) )
                            {
                            XmString *dirNames;
                            char *homeDir;
                            int numdirNames = 0;

                            XtVaGetValues ( w,
                                            XmNdirListItems, &dirNames,
                                            XmNdirListItemCount, &numdirNames,
                                            NULL );
                            if ( numdirNames > 0 )
                                XmStringGetLtoR ( dirNames[0], char_set, &homeDir );
                            else
                                homeDir = "/~/";
                            newfile = XtMalloc ( strlen ( homeDir ) + 1 + strlen ( s ) + 1 );
                            sprintf ( newfile, "%s/%s", homeDir, s );
                            }
                        else
                            {
                            newfile = XtMalloc ( strlen ( dir ) + 1 + strlen ( s ) + 1 );
                            sprintf ( newfile, "%s%s", dir, s );
                            }
                        XtFree ( s );
                        XtFree ( dir );
                        s = newfile;
                        }
                    }
                else
                    {
                    newfile = XtMalloc ( strlen ( s ) + 1 );
                    sprintf ( newfile, "%s",  s );
                    XtFree ( s );
                    s = newfile;
                    }
                }
            else
                {
                /* Its a directory */
                XmString str = XmStringCreateSimple ( s );
                XmFileSelectionDoSearch ( w, str );
                unmapFileList();    /* If remote dir - unmap the file list */
                XmStringFree ( str );
                XtFree ( s );
                return;
                }
#if 0
            switch ( is_accessible ( s ) )
                {
                case 1 :
                    puts ( s ); /* or do anything you want */
                    break;
                case 0 :
                    {
                    /* a directory was selected, scan it */
                    XmString str = XmStringCreateSimple ( s );
                    XmFileSelectionDoSearch ( w, str );
                    unmapFileList();    /* If remote dir - unmap the file list */
                    XmStringFree ( str );
                    break;
                    }
                case -1 :
                    /* a system error on this file */
                    perror ( s );
                }
#endif
            /* Send a message to the owner client module if there is an owner */
            if ( OwnerModuleId != FIND_ID_ERR )
                {
                char *buf;

                buf = malloc ( strlen ( s )+strlen ( hostname )+4 );
                sprintf ( buf, "%s:%s", hostname, s );
                new_bmsg.fromModule = bd.moduleId;
                new_bmsg.toModule = OwnerModuleId;
                new_bmsg.messageType = BusFindTypeByName ( &bd, "BROWSER_REPLY" );
                new_bmsg.messageLength = strlen ( buf )+1;
                new_bmsg.message = buf;
                BusSendMessage ( &bd, &new_bmsg );
                /* OwnerModuleId = FIND_ID_ERR; */
                free ( buf );
                }
            else
                {
                merror ( "Cannot send FileSelection.. Owner not specified" );
                }

            XtFree ( s );
            break;

        case FSB_FILTER:
            {
            unmapFileList();
            break ;

            }
        case FSB_CANCEL:
            {
            struct BusMessage new_bmsg;

            /* printf("FSB CANCEL selected\n"); */
            /*
                        XtDestroyWidget(toplevel);
            */
            /* Send a message to the owner module that the Browser is exiting */
            if ( OwnerModuleId != FIND_ID_ERR )
                {
                char hname[256], ipaddress[32], s2[256];
                char msg[80] = "File Selection Cancelled ";

                new_bmsg.toModule = OwnerModuleId;
                new_bmsg.fromModule = bd.moduleId;
                new_bmsg.messageType = BusFindTypeByName ( &bd, "BROWSER_CANCEL" );
                new_bmsg.messageLength = strlen ( msg )+1;
                new_bmsg.message = msg;
                BusSendMessage ( &bd, &new_bmsg );

                /* Hide the Browser window */
                XUnmapWindow ( XtDisplay ( toplevel ), XtWindow ( toplevel ) );
                OwnerModuleId = FIND_ID_ERR;

                /* Reset the host dialog and file dialog box to default values */
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
            else
                {
                /*
                sleep(5);
                BusClose(&bd);
                exit(0);
                */
                /* Hide the Browser window */
                XUnmapWindow ( XtDisplay ( toplevel ), XtWindow ( toplevel ) );
                }
            break;
            }
        case FSB_HOST:
            /* printf("FSB HOST selected\n"); */
            /* make the dialog box visible */
            /*
            if (numHostsConfigured > 0) {
               XtManageChild(hst_dialog);
            }
            else {
            merror("No deamons present. \n Cannot choose any remote host");
            }
            */
            UpdateHostDialog();
            break;
        }
    /*    XtUnmanageChild(w); */
    }

void UpdateHostDialog()
    {
    int numModules, list_cnt;
    struct BusModuleData *list;
    Widget host_list;
    XmString xs1;
    struct in_addr host_addr;
    char tmp_addr[256], localhname[256];
    struct host_item *tmpConfigList;
    XmString *strlist;
    int oldNumHostsConfigured;
    int i,j;

    host_list = XmSelectionBoxGetChild ( hst_dialog, XmDIALOG_LIST );
    XmListDeleteAllItems ( host_list );

    list = BusFindWhosConnected ( &bd, &numModules );
    oldNumHostsConfigured = numHostsConfigured;
    numHostsConfigured = 0;
    for ( list_cnt=0; list_cnt<numModules; list_cnt++, list++ )
        {
        if ( strstr ( list->name, "busd_" ) != NULL )
            {
            int b0, b1, b2, b3;
            struct hostent *hp;

            strcpy ( tmp_addr, list->name+5 );
            if ( sscanf ( tmp_addr, "%d.%d.%d.%d",&b0,&b1,&b2,&b3  ) ==4 )
                {
                host_addr.s_addr = htonl ( b0 << 24 | b1 << 16 | b2 << 8 | b3 );
                hp = gethostbyaddr ( ( char * ) &host_addr, sizeof ( struct in_addr ),
                                     AF_INET );
                if ( hp )
                    {
                    xs1 = XmStringCreateLtoR ( hp->h_name, char_set );
                    XmListAddItem ( host_list, xs1, 0 );
                    XmStringFree ( xs1 );
                    numHostsConfigured ++;
                    }
                }

            }
        }

    /* Add an entry for the local host */
    gethostname ( localhname, 256 );
    xs1 = XmStringCreateLtoR ( localhname, char_set );
    XmListAddItem ( host_list, xs1, 0 );
    XmStringFree ( xs1 );
    numHostsConfigured ++;

    /* Update the pathnames table */
    tmpConfigList = hostConfigList;

    XtVaGetValues ( host_list,
                    XmNitems, &strlist,
                    NULL );

    hostConfigList = ( struct host_item * )
                     malloc ( numHostsConfigured * sizeof ( struct host_item ) );
    for ( i=0; i<numHostsConfigured; i++ )
        {
        char *hname;

        XmStringGetLtoR ( strlist[i], char_set, &hname );
        for ( j=0; j<oldNumHostsConfigured; j++ )
            {
            if ( !strcmp ( tmpConfigList[j].hname, hname ) )
                {
                break;
                }
            }
        if ( j != oldNumHostsConfigured )
            {
            memcpy ( &hostConfigList[i], &tmpConfigList[j], sizeof ( struct host_item ) );
            }
        else
            {
            strcpy ( hostConfigList[i].hname, hname );
            sprintf ( hostConfigList[i].pathname, "/~" );
            }
        }
    if ( oldNumHostsConfigured != 0 )
        free ( ( char * ) tmpConfigList );

    /* Show the host dialog box */
    XtManageChild ( hst_dialog );

    }
/*\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
/* FUNCTION void unmanageCB                                             */
/*                                                                      */
/* Parameters: standard call back parameters                            */
/*                                                                      */
/* Description:                                                         */
/*      This is a standard motif function that handles the unmanagement */
/* of any widgets that need to simply disapear.                         */
/*                                                                      */
/*\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void unmanageCB ( Widget w,
                  int client_data,
                  XmSelectionBoxCallbackStruct *call_data
                )
/* handles the unmanagement of any dialog needing to simply dissappear. */
    {
    XtUnmanageChild ( w );
    }

/*\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
/* FUNCTION create_dialog_boxes                                         */
/*                                                                      */
/* Parameters:  none                                                    */
/*                                                                      */
/* Description:                                                         */
/*                                                                      */
/* This function that creates most of the different types     */
/* of standard dialog boxes which will be used in the application.      */
/* Boxes like the file selection box, and the jump dialog are created.  */
/*                                                                      */
/*\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void create_dialog_boxes()
    {
    Arg al[10];
    int ac;
    char app_str[256];
    /*    int x; */


    /* create the error dialog. */
    ac = 0;
    /*
        XtSetArg(al[ac], XmNdialogStyle,XmDIALOG_SYSTEM_MODAL); ac++;
    */
    XtSetArg ( al[ac], XmNmessageString, XmStringCreateLtoR
               ( "The file could not be written.", char_set ) );
    ac++;

    sprintf ( app_str, "%s Error", busApplName );
    XtSetArg ( al[ac], XmNdialogTitle,
               XmStringCreateLtoR ( app_str,char_set ) );
    ac++;
    error_dialog = XmCreateErrorDialog ( toplevel, "error_dialog", al, ac );
    XtUnmanageChild ( XmMessageBoxGetChild ( error_dialog,
                      XmDIALOG_HELP_BUTTON ) );
    XtUnmanageChild ( XmMessageBoxGetChild ( error_dialog,
                      XmDIALOG_CANCEL_BUTTON ) );
    XtAddCallback ( error_dialog, XmNokCallback, unmanageCB, ( XtPointer ) OK );

    /*  create the message dialog. */
    ac=0;
    sprintf ( app_str, "%s Message", busApplName );
    XtSetArg ( al[ac],XmNdialogTitle,
               XmStringCreateLtoR ( app_str,char_set ) );
    ac++;
    message_dialog = XmCreateMessageDialog ( toplevel, "message_dialog", al, ac );
    XtAddCallback ( message_dialog, XmNokCallback, unmanageCB, ( XtPointer ) OK );
    XtUnmanageChild ( XmMessageBoxGetChild ( message_dialog,
                      XmDIALOG_CANCEL_BUTTON ) );
    XtUnmanageChild ( XmMessageBoxGetChild ( message_dialog,
                      XmDIALOG_HELP_BUTTON ) );

    /* create the warning dialog. */
    ac=0;
    sprintf ( app_str, "%s Warning", busApplName );
    XtSetArg ( al[ac],XmNdialogTitle,
               XmStringCreateLtoR ( app_str,char_set ) );
    ac++;
    warning_dialog = XmCreateWarningDialog ( toplevel, "warning_dialog", al, ac );
    XtAddCallback ( warning_dialog, XmNokCallback, unmanageCB, ( XtPointer ) OK );
    XtUnmanageChild ( XmMessageBoxGetChild ( warning_dialog,
                      XmDIALOG_CANCEL_BUTTON ) );
    XtUnmanageChild ( XmMessageBoxGetChild ( warning_dialog,
                      XmDIALOG_HELP_BUTTON ) );

    /* create the host selection dialog. */
    ac=0;
    XtSetArg ( al[ac],XmNautoUnmanage,False );
    ac++;
    /* XtSetArg(al[ac],XmNmustMatch,True); ac++; */

    sprintf ( app_str, "%s Browser: Host Selector", busApplName );
    XtSetArg ( al[ac],XmNdialogTitle,XmStringCreateLtoR (
                   app_str,char_set ) );
    ac++;
    XtSetArg ( al[ac],XmNlistLabelString,
               XmStringCreateLtoR ( "Hosts",char_set ) );
    ac++;
    XtSetArg ( al[ac],XmNselectionLabelString,
               XmStringCreateLtoR ( "Select a host:",char_set ) );
    ac++;
    XtSetArg ( al[ac],XmNokLabelString,XmStringCreateLtoR (
                   "Select",char_set ) );
    ac++;
    XtSetArg ( al[ac],XmNapplyLabelString,XmStringCreateLtoR (
                   "Path",char_set ) );
    ac++;
    XtSetArg ( al[ac],XmNcancelLabelString,XmStringCreateLtoR (
                   "Cancel",char_set ) );
    ac++;
    XtSetArg ( al[ac],XmNhelpLabelString,XmStringCreateLtoR (
                   "User",char_set ) );
    ac++;

    hst_dialog = XmCreateSelectionDialog ( host,"hst_dialog",al,ac );
    XtAddCallback ( hst_dialog,XmNokCallback,hst_dialogCB, ( XtPointer ) HST_OK );
    XtAddCallback ( hst_dialog,XmNcancelCallback,hst_dialogCB,
                    ( XtPointer ) HST_CANCEL );
    XtAddCallback ( hst_dialog,XmNhelpCallback,hst_dialogCB, ( XtPointer ) HST_USER );
    XtAddCallback ( hst_dialog,XmNapplyCallback,hst_dialogCB, ( XtPointer ) HST_PATH );
    XtUnmanageChild ( XmSelectionBoxGetChild ( hst_dialog,
                      XmDIALOG_HELP_BUTTON ) );

    /* create the user dialog box */
    ac=0;
    XtSetArg ( al[ac], XmNselectionLabelString,
               XmStringCreateLtoR ( "Type in username",char_set ) );
    ac++;
    user_dialog = XmCreatePromptDialog ( host,"user_dialog",al,ac );
    XtAddCallback ( user_dialog,XmNokCallback,user_dialogCB, ( XtPointer ) USER_OK );
    XtAddCallback ( user_dialog,XmNcancelCallback,user_dialogCB,
                    ( XtPointer ) USER_CANCEL );
    XtUnmanageChild ( XmSelectionBoxGetChild ( user_dialog,
                      XmDIALOG_HELP_BUTTON ) );

    /* create the path dialog box */
    ac=0;
    XtSetArg ( al[ac], XmNselectionLabelString,
               XmStringCreateLtoR ( "Type in pathname",char_set ) );
    ac++;
    path_dialog = XmCreatePromptDialog ( host,"path_dialog",al,ac );
    XtAddCallback ( path_dialog,XmNokCallback,path_dialogCB, ( XtPointer ) PATH_OK );
    XtAddCallback ( path_dialog,XmNcancelCallback,path_dialogCB,
                    ( XtPointer ) PATH_CANCEL );
    XtUnmanageChild ( XmSelectionBoxGetChild ( path_dialog,
                      XmDIALOG_HELP_BUTTON ) );
    }

void exitBrowser ( XtPointer client_data )
    {
    BusClose ( &bd );
    exit ( 0 );
    }

/* =================    Main ====================================== */
int main ( int argc, char *argv[] )
    {
    Arg al[10];
    int ac;
    char s[256], name[80], *cname="Browser", *bushost, *busport;
    int namelen=256;
    int err, typeId;
    char hname[256] ;
    char titlename[256];
    char app_str[256];
    Widget file_list, dir_list, fsb_filter, fsb_text;
    /* Timer for Dying after 60 min */
    XtIntervalId timer;

    /* create the toplevel shell */

    sprintf ( app_str, "%s_Browser", busApplName );
    toplevel = XtAppInitialize ( &context,app_str,NULL,0,&argc,argv,NULL,NULL,0 );

    gethostname ( hname, 256 );
    sprintf ( titlename, "%s File Browser (%s@%s)", busApplName,
              BusGetMyUserid(), hname );
    ac = 0;
    XtSetArg ( al[ac], XmNtitle, titlename );
    ac++;
    XtSetArg ( al[ac], XmNmappedWhenManaged, False );
    ac++;
    XtSetValues ( toplevel,al,ac );

    /* create the fsb_dialog box */
    ac = 0;
    /* XtSetArg(al[ac],XmNwidth,500); ac++; */
    /* XtSetValues(toplevel,al,ac); */
    /* XtSetArg (al[ac], XmNlistSizePolicy, XmCONSTANT); ac++; */
    XtSetArg ( al[ac], XmNdirSearchProc, dir_search );
    ac++;
    XtSetArg ( al[ac], XmNfileSearchProc, file_search );
    ac++;

#ifndef linux
    XtSetArg ( al[ac], XmNheight, 420 );
    ac++;
    XtSetArg ( al[ac], XmNwidth, 500 );
    ac++;
    XtSetArg ( al[ac], XmNlistSizePolicy, XmVARIABLE );
    ac++;
#else
    XtSetArg ( al[ac], XmNlistSizePolicy, XmCONSTANT );
    ac++;
#endif

    XtSetArg ( al[ac], XmNcursorPositionVisible, True );
    ac++;
    XtSetArg ( al[ac], XmNresizePolicy, XmRESIZE_ANY );
    ac++;

    ( void ) gethostname ( name,namelen );
    ( void ) is_localhost ( name,ipaddress );
    strcpy ( hostname,name );
    strcpy ( username,BusGetMyUserid() );

    fsb_dialog=XmCreateFileSelectionBox ( toplevel,"fsb_dialog",al,ac );
    XtAddCallback ( fsb_dialog,XmNokCallback,fsb_dialogCB, ( XtPointer ) FSB_OK );
    XtAddCallback ( fsb_dialog,XmNapplyCallback,fsb_dialogCB, ( XtPointer ) FSB_FILTER );
    XtAddCallback ( fsb_dialog,XmNcancelCallback,fsb_dialogCB,
                    ( XtPointer ) FSB_CANCEL );
    XtUnmanageChild ( XmSelectionBoxGetChild ( fsb_dialog,
                      XmDIALOG_HELP_BUTTON ) );

    XtManageChild ( fsb_dialog );

    /*
    dir_list = XmFileSelectionBoxGetChild(fsb_dialog, XmDIALOG_DIR_LIST);
    file_list = XmFileSelectionBoxGetChild(fsb_dialog, XmDIALOG_LIST);

    XtVaSetValues(file_list,
          XmNleftAttachment,     XmATTACH_WIDGET,
          XmNleftWidget,         dir_list,
          NULL);
    XtVaSetValues(file_list, XmNwidth, 300, NULL);
    */


    /* Make cursor visible in dir-mask and selection widgets */
    /*
        fsb_filter = XmFileSelectionBoxGetChild(fsb_dialog, XmDIALOG_FILTER_TEXT);
        XtVaSetValues(fsb_filter, XmNcursorPositionVisible, True,
                NULL);
        fsb_text = XmFileSelectionBoxGetChild(fsb_dialog, XmDIALOG_TEXT);
        XtVaSetValues(fsb_text, XmNcursorPositionVisible, True,
                NULL);
    */


    /* create and manage the host pushbutton */
    ac=0;
    strcpy ( s,"Host: " );
    strcat ( s,name );
    strcat ( s," (" );
    strcat ( s,ipaddress );
    strcat ( s,")    User: " );
    strcat ( s,BusGetMyUserid() );
    strcat ( s, "\n Owner Module: NONE specified" );
    XtSetArg ( al[ac],XmNlabelString,
               XmStringCreateLtoR ( s,char_set ) );
    ac++;
    host=XmCreatePushButton ( fsb_dialog,"Host: ",al,ac );
    XtAddCallback ( host,XmNactivateCallback,fsb_dialogCB, ( XtPointer ) FSB_HOST );
    XtManageChild ( host );

    create_dialog_boxes();

    XtRealizeWidget ( toplevel );

    /* Iconify the window */
    /*
        XIconifyWindow(XtDisplay(toplevel), XtWindow(toplevel),
        XScreenNumberOfScreen(XtScreen(toplevel)));
        XWithdrawWindow(XtDisplay(toplevel), XtWindow(toplevel),
        XScreenNumberOfScreen(XtScreen(toplevel)));
    */


    /* do the bus stuff */
    /* set client name */
    bd.name = malloc ( strlen ( cname )+1 );
    strcpy ( bd.name, cname );
    bushost = getenv ( "SBUSHOST" );
    if ( bushost == NULL ) bushost = "local";
    busport = getenv ( "SBUSPORT" );

    /* attempt to attach to the bus */
    /* printf("Attaching to bus \n"); */
    err = BusXtInitialize ( &bd, context );
    if ( err != SBUSERROR_NOT )
        {
        merror ( "Attempt to connect to the software bus failed" );
        /* exit(-1); */
        }
    else
        {
        /* Get the handle for standard bus-managed ASCII data type */
        /* mmessage("Find id for type ASCIIZ"); */
        /* printf("Attached to the bus .... adding callbacks \n"); */
        typeId = BusFindTypeByName ( &bd, "ASCIIZ" );

        /*  mmessage("Setting callback for type ASCIIZ"); */
        BusAddTypeCallback ( &bd, typeId, ASCIIZ_callback );

        typeId = BusFindTypeByName ( &bd, "FTP_DIR_HERE" );

        /* Store in global variable to be used in get_dir_remote */
        callbackTypeId = typeId;

        /*  mmessage("Setting callback for type ASCIIZ"); */
        BusAddTypeCallback ( &bd, typeId, recv_dir_callback );

        typeId = BusFindTypeByName ( &bd, "BROWSER_SEIZE" );
        BusAddTypeCallback ( &bd, typeId, Seize_callback );

        typeId = BusFindTypeByName ( &bd, "BROWSER_RELEASE" );
        BusAddTypeCallback ( &bd, typeId, Release_callback );

        sprintf ( s, "Connected to the bus on host: %s, at port %s \n\
as module \"%s\" with id %i\n\
that understands data type FTP_DIR_HERE = %d",
                  bushost, busport, bd.name, bd.moduleId, typeId );
        /* mmessage(s); */
        }

    /* Set timer to kill the Browser */
    timer = XtAppAddTimeOut ( context, 6000000,
                              ( XtTimerCallbackProc ) exitBrowser, NULL );

    /* XUnmapWindow(XtDisplay(toplevel), XtWindow(toplevel)); */
    XtAppMainLoop ( context );
    }
