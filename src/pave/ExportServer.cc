/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: ExportServer.cc 83 2018-03-12 19:24:33Z coats $
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
/////////////////////////////////////////////////////////////
//
// ExportServer.C
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// June 15, 1995
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950615  Implemented
// SRT  950908  added int removeAllItems(Widget dialog)
// SRT  960517  Added hooks to netCDF exporting
// SRT  961011  Added release_edata(), save_cancelCB(), and save_cancel_cb()
//
/////////////////////////////////////////////////////////////
//
// ExportServer Class
//
/////////////////////////////////////////////////////////////


#include "ExportServer.h"

// Constructor
ExportServer::ExportServer ( int etype,         // type of data
                             char *name,             // name
                             Widget parent,          // parent to attach UI to
                             char *dialogtitle ) :   // title
    SelectLoadSaveServer (
        name,
        parent,
        dialogtitle,
        "dummyFileMarker" )
    {
    exportType_ = etype;
    edata_ = ( void * ) NULL;
    }


ExportServer::~ExportServer()           // destructor
    {
    release_edata();
    }



void ExportServer::ShowUI ( void *edata )      // data ptr
    {
    release_edata();
    edata_ = edata;
    if ( !edata_ )
        {
        Message error ( parent_, XmDIALOG_ERROR, "No data passed to ExportServer::ShowUI()!" );
        return;
        }
    postSaveSelectionDialog();
    }



void ExportServer::save_okCB ( Widget, XtPointer clientData, XtPointer callData ) // override SelectLoadSaveServer's
    {
    ExportServer *obj = ( ExportServer * ) clientData;
    XmFileSelectionBoxCallbackStruct *cbs = ( XmFileSelectionBoxCallbackStruct * ) callData;

    obj->save_ok_cb ( cbs->value );
    }


void ExportServer::save_ok_cb ( XmString xmstr )           // override SelectLoadSaveServer's
    {
    char *str;

    if ( XmStringGetLtoR ( xmstr, XmSTRING_DEFAULT_CHARSET, &str ) )
        saveSelectionListToFile ( str, ( Widget ) NULL );

    XtFree ( str );
    }



void ExportServer::saveSelectionListToFile              // override SelectLoadSaveServer's
( char *filename, Widget )             // saveSelectionListToFile()
    {
    char estring[256];

#ifdef DIAGNOSTICS
    fprintf ( stderr, "ExportServer::saveSelectionListToFile saving to \n" );
    fprintf ( stderr, "%s\n", filename );
#endif // DIAGNOSTICS

    switch ( exportType_ )
        {
        case PAVE_EXPORT_AVS:
            if ( dump_VIS_DATA_to_AVS_file ( ( VIS_DATA * ) edata_,
                                             filename,
                                             estring ) )
                {
                Message error ( parent_, XmDIALOG_ERROR, estring );
                }
            break;

        case PAVE_EXPORT_TABBED:
            if ( dump_VIS_DATA_to_tabbed_ascii_file ( ( VIS_DATA * ) edata_,
                    filename,
                    estring ) )
                {
                Message error ( parent_, XmDIALOG_ERROR, estring );
                }
            break;

        case PAVE_EXPORT_NETCDF:
            if ( dump_VIS_DATA_to_netCDF_file ( ( VIS_DATA * ) edata_,
                                                filename,
                                                estring ) )
                {
                Message error ( parent_, XmDIALOG_ERROR, estring );
                }

            break;

        default:
            {
            Message error ( parent_, XmDIALOG_ERROR,
                            "Unknown export data typE in ExportServer::saveSelectionListToFile" );
            }
        }
    release_edata();
    }


// override SelectionServer's
Widget ExportServer::createEditSelectionDialog ( void )
    {
    return ( Widget ) NULL;
    }


// override SelectionServer's
int ExportServer::addItem ( char *, Widget )
    {
    return 0;
    }


// override SelectionServer's
int ExportServer::removeAllItems ( Widget )
    {
    return 0;
    }


void ExportServer::release_edata ( void ) // frees up the data at *edata_;
    {
    if ( edata_ == ( void * ) NULL )
        return;

    free_vis ( ( VIS_DATA * ) edata_ );
    free ( edata_ );
    edata_ = ( void * ) NULL;
    }


void ExportServer::save_cancel_cb ( XmString /*xmstr*/ )           // override SelectLoadSaveServer's
    {
    release_edata();
    }
