/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Util.cc 83 2018-03-12 19:24:33Z coats $
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
///////////////////////////////////////////////////////////
// Util.cc
//////////////////////////////////////////////////////////

#include "Util.h"

Util::Util ()
    {
    // Empty constructor
    }


char *Util::file2string ( char * filename )
    {
    FILE *fp;
    static char *content;
    int size;
    struct stat statbuf;

    if ( content ) free ( content );

    if ( ( fp = fopen ( filename, "r" ) ) != NULL )
        {
        /* Determine the size of file */
        fstat ( fileno ( fp ), &statbuf );

        /* Allocate memory for buffer */
        content = ( char* ) malloc ( sizeof ( char ) * ( int ) ( statbuf.st_size )+2 );

        fseek ( fp, 0, SEEK_SET );

        /* Read file to buffer */
        for ( size=0; !feof ( fp ); size++ ) content[size]=fgetc ( fp );
        content[size-1] = '\0';
        fclose ( fp );
        return ( content );
        }
    else
        {
        fprintf ( stderr, "Input File not found. Can't convert it into string.\n" );

        return ( char * ) NULL;
        }
    }


//********************************************************************************************

Message::Message ( Widget parent, int dialog_type, char *msg )
    {
    assert ( parent );
    assert ( msg );

    if ( !msg[0] )
        {
        fprintf ( stderr, "Empty msg supplied to Message!\n" );
        return;
        }
    Widget dialog = XmCreateMessageDialog ( parent, "Message", NULL, 0 );
    XmString xmstr = XmStringCreateLtoR ( msg, XmSTRING_DEFAULT_CHARSET );
    XtVaSetValues ( dialog,
                    XmNdialogType,  dialog_type,
                    XmNmessageString,       xmstr,
                    NULL );
    XmStringFree ( xmstr );
    XtUnmanageChild ( XmMessageBoxGetChild ( dialog, XmDIALOG_CANCEL_BUTTON ) );
    XtUnmanageChild ( XmMessageBoxGetChild ( dialog, XmDIALOG_HELP_BUTTON ) );
    //   XtAddCallback(dialog, XmNokCallback, XtDestroyWidget, NULL);
    if ( XtIsManaged ( dialog ) ) XtUnmanageChild ( dialog );
    XtManageChild ( dialog );
    }

