/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 2.4
 *
 *  File: $Id: busMaster.h 84 2018-03-12 21:26:53Z coats $
 *  Copyright (C) 1996-2004 MCNC
 *            (C) 2004-2010 UNC Institute for the Environment
 *            (C) 2018-     Carlie J. Coats, Jr., Ph.D.
 *
 *  Licensed under the GNU General Public License Version 2.
 *  See enclosed gpl.txt for more details
 *
 *  For further information on PAVE:
 *      Usage: type -usage in PAVE's standard input
 *      User Guide: https://www.cmascenter.org/pave/documentation/2.3/
 *      FAQ: https://www.cmascenter.org/pave/documentation/2.3/Pave.FAQ.html
 *
 ****************************************************************************
 * ABOUT:   busMaster.h
 *
 *     This include header contains data structures and bus master library
 *     protytpes and descriptions.
 *
 * VERSION "$Id: busMaster.h 84 2018-03-12 21:26:53Z coats $"
 ********************************************************************
 *
 * * HISTORY: EDSS_SB - busMaster.h
 *
 * Date: 14-Dec-94
 * Version: 0.1
 * Change Description: ORIGINAL VERSION 
 * Change author: Leland Morrison, NCSU, CSC 
 *
 * Date: 15-Dec-94
 * Version: 0.2
 * Change Description: Added some comments and main headers
 * Change author: Vouk
 *
 * Date: 17-Jan-95
 * Version: 0.3
 * Change Description: Added a definition for maximum size of a module name
 *	which is used in BusMasterFindWhosConnected. 
 * Change author: Rajini Balay, NCSU, CSC 
 *
 * Date: 27-Jan-95
 * Version: 0.3a
 * Change Description: Added a definition for maximum size of integer that 
 *        can fit into a string which is used in BusMasterFindWhosConnected. 
 * Change author: Rajini Balay, NCSU, CSC 
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 ********************************************************************/

/* So that this file isn't included twice */

#ifndef _busMaster_H_INCLUDED
#define _busMaster_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhbusMaster= "$Id: busMaster.h 84 2018-03-12 21:26:53Z coats $" ;
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include "busClient.h"
#include "busMsgQue.h" 

#define MAX_MODULENAME  64	/* Added by Rajini */	
#define MAX_INTSTR_SIZE 10      /* Added by Rajini after v6.3 */

struct BusTypeNode {
  /* a node in the list of type names the bus knows about */
  struct BusTypeNode *next;    /* pointer to the next node */
  
  char               *typeName;/* the name of the type */
  int                 typeId;  /* the enumeration of the type */
};


struct BusRegisteredTypeNode {
  /* a node in the list of types a module "understands" */
  struct BusRegisteredTypeNode *next;
  
  /* the enumeration of the understood type */
  int    typeId;
};


struct BusModuleNode {
  /* a node which contains all the information known about one
   module that has connected to the bus */
  struct BusModuleNode         *next;

  /* name of the module this node holds the information for */
  char                         *moduleName;
  
  /* moduleId is the id number of the module
   used to identify it on this bus */
  int                           moduleId;
  
  /* file descriptor for the communication with the module,
     also used as the id for the module */
  int                           fd;
  
  /* a list of types "understood" by the module, used by
     message destination BusByType to get a message to all
     the module that "understand" a given type */
  struct BusRegisteredTypeNode *Types;
  
  /* where the client is */
  struct sockaddr_in            sin;
  
  struct BusMessageQueue       *Messages;
};


struct BusMasterData {
  /* this structure should hold all the information used by the
   bus master functions */
  
  /* portNumber is the port which the portSocket has been
     bound to */
  int     portNumber;
  /* portSocket is the socket (file descriptor) which has been
     bound to a port to enable it to receive connection from the
     rest of the bus */
  int     portSocket;
  
  /* unixPath is the path that unixSocket is bound to */
  char   *unixPath;
  /* unixSocket is the socket (file descriptor) which has been
     bound to a path in the file system to receive connections to
     the bus */
  int     unixSocket;
  
  /* #ifdef SUN */
  int  sunPortNumber;
  /* port number of a connectionless to received messages
     requesting connections */
  int  sunPort;
  /* file descriptor for connectionless socket associated w/ above */
  /* #endif */
  
  /* firstType points to the first node of a list of the Types
     that have been registered with the bus, this list contains
     the ASCII names of the types */
  struct  BusTypeNode       *Types;
  /* this is the type id number to use for the next type to be
     added to the list of type register with the bus */
  int nextTypeId;

  /* firstModule is a pointer to the list of modules which are
     connected to the bus, this list contains the ASCIIZ names
     of the modules */
  struct  BusModuleNode     *Modules;
  /* this is the module id number to use for the next module to
     connect to the bus */
  int nextModuleId;

  int nextMessageSerialNumber;
  
  /* if non-NULL, a pointer to a function to be called whenever
     a file descriptor (socket) is opened, the function should
     be called with the file descriptor (of the socket) as the
     first argument, and the character pointer open_data as the
     second argument.  Both the function pointer and the open_data
     pointer must be specified in the same function call,
     BusMasterSetOpenAction */
  void  (* open_action)(int opened_fd, char *data_open_arg);
  char   * open_data;
  
  /* close_action is the equivalent of open_action except that
     the function should be called whenever a file descriptor(socket)
     is closed */
  void  (* close_action)(int closing_fd, char *data_close_arg);
  char   * close_data;

  struct BusMessageQueue *WaitingReq;
};

/* Bus Master functions ------------------------------------------------ */

int     BusMasterInitialize(struct BusMasterData *);
/* sets up this process as the bus master */
/* initialize the BusMasterData data structure */

void BusMasterShutdown( struct BusMasterData *bmd );
/* called to shutdown the entire bus.  use when the BusMaster process
   is exiting */


void BusMasterSetOpenAction (struct BusMasterData *bmd,
			     void (*action)(int fd, char *data),
			     char *data);
void BusMasterSetCloseAction(struct BusMasterData *bmd,
			     void (*action)(int fd, char *data),
			     char *data);
/* assigns an action to be performed after opening(/closing) a new file
   descriptor */
/* [action] will be called whenever another file descriptor is opened
   after this call is made, action is not called for the file descriptors
   currently in use, (use BusMasterActOnfds for that) */


void BusMasterActOnfds(struct BusMasterData *bmd,
		       void (* action)(int fd, char *data),
		       char *data);
/* calls the function pointed to by action multiple times using
   each of the file descriptor in use by the bus master */


void BusMasterDispatcher( struct BusMasterData *bmd, int fd );
/* this function should be called whenever data is available from the
   files (sockets) in use by the bus, it reads the data, and performs
   all the necessary actions */

#endif
