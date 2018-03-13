/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 2.4
 *
 *  File: $Id: busClient.h 84 2018-03-12 21:26:53Z coats $
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
 * ABOUT:  busClient.h
 *
 *     busClient.h is the include file that describes the data structures
 *     and functions that a bus client needs to communicate with the
 *     bus and other clients. Client needs to register with the bus,
 *     and then it can talk to other clients eithr via bus-managed
 *     channels, or it can open a direct link to the other client. 
 *
 * VERSION "$Id: busClient.h 84 2018-03-12 21:26:53Z coats $"
 ********************************************************************
 *
 * HISTORY:
 *
 * Date: 14-Dec-94
 * Version: 0.1
 * Change Description: ORIGINAL VERSION 
 * Change author: Leland Morrison, NCSU, CSC 
 *
 * Date: 15-Dec-94
 * Version: 0.2
 * Change Description: Added main headers
 * Change author: Vouk
 *
 * Date: 7-Jan-95
 * Version: 0.2.1
 * Change Description: Added "RecvdMessages" field to struct BusData. 
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 17-Jan-95
 * Version: 0.3
 * Change Description: Added "nextSeqNum" field to struct BusData for generating 
 *	sequence numbers for messages and modified definitions of few 
 *	functions .
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 27-Jan-95
 * Version: 0.3a
 * Change Description: Modified definition of BusSendMessage to take
 *                     'struct BusMessage *' as a parameter instead of
 *		     'struct BusMessage'.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 6-April-95
 * Version: 0.5
 * Change Description: Added an extra field to the BusData structure: 
 *			BusTimeoutCallbackNode to allow 'select' in
 *			BusEventLoop to timeout. 
 *			Added function definitions "BusModuleInfoByName"
 *			and "BusModuleInfoById"
 *		        Added a definition for the Version Number.
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 24-July-95
 * Version: 0.6a
 * Change Description: Changed interface to BusInitialize 
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 19-Aug-95
 * Version: 0.6b
 * Change Description: Changed some function definitions to const char * 
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Date: 7-Sept-95
 * Version: 0.6b
 * Change Description: Added definition for BusProcessRecvdMessages
 * Change author: Rajini Balay, NCSU, CSC
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 ********************************************************************/

/* so that this header isn't included twice... */

#ifndef SBUS_CLIENT_H_INCLUDED
#define SBUS_CLIENT_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhClient= "$Id: busClient.h 84 2018-03-12 21:26:53Z coats $" ;
#endif

/*** Includes and global defines ***/

#include <time.h>
#include "busMsgQue.h"

/* names for environment variable used by bus clients */
#define SBUSPORT  "SBUSPORT"
#define SBUSHOST  "SBUSHOST"

#define SBUSLOCSAVE "SBUSFILE"

/* In case unix sockets are (re-) incorporated */
#define SBUSPATH  "SBUSPATH"
#define SBUSDEFAULTPATH "/tmp/software_bus"


/*** Data structures ***/

/* MasterId --- Defined by Rajini */
#define MASTERID 0


struct BusData {
  /* name client wants to be known by to the bus */
  char      *name;
  
  /* bus id number */
  int        moduleId;
  
  /* Added by Rajini */
  int 	     nextSeqNum;

  /* can(or should) the client program be restarted by the
     bus in the event of a crash ? */
  int        restartable;
  /* the command to be executed on the system the client is
     running on to restart this client module */
  char      *restartCommand;
  
  /* socket used to communicate w/ the bus master */
  int        fd;
  
  struct BusMessageQueue *Messages;

  /* Added by Rajini : RecvdMessages holds messages got while waiting for 
     a response to queries like FindIdByName etc.. 
   */
  struct BusMessageQueue *RecvdMessages;
  
  struct BusXtData *xtd;

  struct BusTypeCallbackNode   *  TypeCallbacks;
  struct BusDirectCallbackNode *DirectCallbacks;
  
  /* Added by Rajini: to allow a timeout in select of BusEventLoop */
  struct BusTimeoutCallbackNode *TimeoutCallback;
  
  /* if BusEventLoop is being used,
     a list of file descriptors in use, and
     callback associated with them */
  struct BusInputCallback *InputCallbacks;
};

struct BusModuleData {
   int moduleId;
   char *name;
   char *ip_addr;   
} ;

/*** Functions ***/

/* int BusInitialize( struct BusData *bd ); */
int BusInitialize( struct BusData *bd, int setTimeout, int minsToLive,
                      void (*exitCallback)(struct BusData *bd));
/* Connects a client to the software bus, and
   completes initialization of the BusData structure
   bd - in - a valid pointer to a bus data structure
   bd->name point to an ASCIIZ string which is the
   name of the client (that the client will be known
   by to the bus) */

void BusClose(struct BusData *bd);
/* close a connection to the bus
   bd - in - pointer to BusData structure for
   connection to be closed */

/*************************************************************
  BusAddInputCallback, BusRemoveInputCallback
  are intended for use with the function
  BusEventLoop
  ************************************************************/

typedef void (*BusInputCBfunc)(int, struct BusData *);

int BusAddInputCallback(struct BusData *bd,
		       int fd,
		       BusInputCBfunc   read_callback,
		       BusInputCBfunc  write_callback,
		       BusInputCBfunc except_callback);
/* For use with BusEventLoop */
/* Adds a set of callbacks for a given file descriptor
   for BusEventLoop to check and call when the file
   is in the specified state */
/* bd - in - a BusData struct initialized by BusInitialize   */
/* fd - in - the file descriptor to add the callback for     */
/* read_callback - in - a callback for when data can be read */
int BusRemoveInputCallback(struct BusData *,int fd);

void BusEventLoop(struct BusData *bd);
/* The function can be called by text base client programs   *
 * which can operate as event driven loops                   */

/*************************************************************
  ************************************************************/

int BusAddTypeCallback(struct BusData *bd, int type,
		       void (*callback)(struct BusData *,
					struct BusMessage *));
/* add a function to the set of functions to be called when
   a message of the specified type is received */

int BusRemoveTypeCallback(struct BusData *bd, int type,
			  void (*callback)(struct BusData *,
					   struct BusMessage *));
/* removes a function from the set of functions to be called
   when a message of the specified type is received */

int BusDispatch( struct BusData *bd );
/* (internal function) Handles the reading of a signal from the
   bus master, and takes an appropriate action (such a reading
   a message, and then calling the callback for that message */

void BusProcessRecvdMessages(struct BusData *bd);

/* Added by Rajini */
void BusProcessOption( struct BusData *bd, struct BusMessage *bmsg );

int BusFindModuleByName( struct BusData *bd, const char *name );
/* returns a Module's id # given the modules name (-1 if not found) */

char *BusFindModuleById( struct BusData *bd, int typeId );
/* returns a Module's name given its id # (NULL if not found)
   - free the return pointer when finished with it */

int BusFindTypeByName( struct BusData *bd, const char *name );
/* returns a type's id # given the name,
   an id number is assigned if the type is not found in the
   pre-existing list */

char *BusFindTypeById( struct BusData *bd, int id );
/* returns a type's name given its id #, (NULL if not found/error)
   free the (char *) when finished with it */

int BusRegisterType( struct BusData *bd, int typeId );
/* registers a type as being "understood" by the client,
   primarily for purposes of getting message sent to BusByType */

struct BusModuleData *BusFindWhosConnected( struct BusData *bd, int *numModules);
/* Find the modules that are connected to the Bus */ 

int BusGetModuleInfoById( struct BusData *bd, int moduleId, struct BusModuleData *moduleInfo);
int BusGetModuleInfoByName( struct BusData *bd, const char *name, struct BusModuleData *moduleInfo);
/* Find information about a particular module */

int BusSendMessage( struct BusData *bd, struct BusMessage *bmsg );
/* queues a messages for distribution to the rest of the bus */

int BusBroadcastMessage(struct BusData *bd, int typeId,
			char *message, int length);
/* enques a message for broadcasting to all other modules attached to the bus */

int BusSendByType(struct BusData *bd, int typeId,
		  char *message, int length);
/* enques a message for broadcasting a message according to type */

int BusBounceMessage(struct BusData *bd, int typeId,
		     char *message, int length);
/* enques a message to be bounced off the busMaster */

int BusSendASCIIZ( struct BusData *bd, int toModule, char *message );
/* enques a ASCIIZ message */

int BusBroadcastASCIIZ( struct BusData *bd, char *message );
/* enques an ASCIIZ message for broadcasting */

int BusAddDirectCallback( struct BusData *bd, 
                          int typeId,
			  void (*callback)(int, char*),
			  char *data);

/* specifies the function to be called when 'select' in BusEventLoop
   times out. */
int BusAddTimeoutCallback(struct BusData *bd, struct timeval *timeout,
			  void (*callback)(struct BusData *));
int BusRemoveTimeoutCallback(struct BusData *bd);

/* specifies a function to be called when a point-to-point (direct) connection
   is request for the given type */

int BusRemoveDirectCallback( struct BusData *bd,
			     int typeId,
			     void (*callback)(int, char*) );
/* removes a function (from the list of functions) specified to handle a
   point-to-point connection */

int BusSendDirect(struct BusData *bd, int toModule, int typeId,
		  void (*send_call)(int, char *), char *data );
/* establishes a point-to-point connection, calls a user-function
   transmit the data, and closes the connection upon return of the
   function */

int BusKillClient(struct BusData *bd, const char *name);
/* Function available to the Console client to send a Kill message to
   any other client on the Bus */

#endif /* SBUS_CLIENT_H_INCLUDED */

