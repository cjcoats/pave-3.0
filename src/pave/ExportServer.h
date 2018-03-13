#ifndef EXPORT_SERVER_H__
#define EXPORT_SERVER_H__

/////////////////////////////////////////////////////////////
//
// Project Title: Environmental Decision Support System
//         File: @(#)ExportServer.h	2.1
//     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.ExportServer.h
// Last updated: 12/15/97 16:26:23
//
// Made available by MCNC and the Carolina Environmental Program of UNC Chapel
// Hill under terms of the GNU Public License.  See gpl.txt for more details.
//
// See file COPYRIGHT for license information on this and supporting software.
//
// Carolina Environmental Program
// University of North Carolina at Chapel Hill
// 137 E. Franklin St.
// Chapel Hill, NC 27599-6116
//
// See http://www.cep.unc.edu, http://www.cmascenter.org, http://bugz.unc.edu
//
/////////////////////////////////////////////////////////////
//
// ExportServer.h
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// June 16, 1995
//
/////////////////////////////////////////////////////////////
//
// ExportServer Class 
//
// Will be used to show a UI to select a file name then
// export data to that file.
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT	950615	Implemented
// SRT  950908  added int removeAllItems(Widget dialog)
// SRT  960517  Added hooks to netCDF exporting
// SRT	961011  Added release_edata() and save_cancel_cb()
//
/////////////////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */


#include "SelectLoadSaveServer.h"
#include "vis_proto.h"


#define PAVE_EXPORT_AVS		1
#define PAVE_EXPORT_TABBED	2
#define PAVE_EXPORT_NETCDF	3


class ExportServer:public SelectLoadSaveServer {

  public:

						// Constructor 
	ExportServer(	int etype, 		// type of data
			char *name,		// name
			Widget parent,		// parent to attach UI to
			char *dialogtitle );	// title
			

	virtual ~ExportServer();		// Destructor

	void saveSelectionListToFile		// override SelectLoadSaveServer's
	 (char *filename, Widget dialog);	// saveSelectionListToFile()

	void ShowUI 	(void *edata);		// data ptr

        static void save_okCB(Widget, 		// override SelectLoadSaveServer's
			XtPointer clientData, 
			XtPointer callData); 
        void save_ok_cb(XmString); 		// override SelectLoadSaveServer's

        void save_cancel_cb(XmString); 		// override SelectLoadSaveServer's

	Widget createEditSelectionDialog(void); // override SelectionServer's
	int addItem(char *item, Widget dialog); // override SelectionServer's
	int removeAllItems(Widget dialog);      // override SelectionServer's



  private:

	int exportType_;			// set to PAVE_EXPORT_AVS, 
						// PAVE_EXPORT_TABBED, or
						// PAVE_EXPORT_NETCDF

	void *edata_;				// points to ExportServer's copy of the
						// data to be exported 
						// freed upon exporting the data

	void release_edata(void);		// frees up the data at *edata_;
};


#endif  // EXPORT_SERVER_H__

