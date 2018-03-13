/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 2.4
 *
 *  File: $Id: busMsgQue.h 84 2018-03-12 21:26:53Z coats $
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
 * ABOUT:   busMsgQue.h
 *
 *         busMsgQue.h is the include file that describes the data structures
 *         and functions that a bus message queu handlers need.
 *
 * VERSION "$Id: busMsgQue.h 84 2018-03-12 21:26:53Z coats $"
 ********************************************************************
 *
 * HISTORY: EDSS_SB - busMsgQue.h
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
 * Date: 7-Jan-95 
 * Version: 0.2.1
 * Change Description: Modified the field messageClass of BusMessage 
 *		     to messageOption 
 * Change author: Rajini Balay, NCSU, CSC 
 *
 * Date: 17-Jan-95
 * Version: 0.3
 * Change Description: Added definitions for functions BusGetMessageBySeq etc. 
 * Change author: Rajini Balay, NCSU, CSC 
 *
 * Date: 27-Jan-95
 * Version: 0.3a
 * Change Description: Modified definition of Bus_EnqueMessage to take
 *                     'struct BusMessage *' as a parameter instead of
 *		     'struct BusMessage'.
 * Change author: Rajini Balay, NCSU, CSC 
 *
 * Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 *
 ********************************************************************/

/* so that this header isn't included twice... */

#ifndef SBUS_MESSAGE_QUEUE_H_INCLUDED
#define SBUS_MESSAGE_QUEUE_H_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhbusMsgQue= "$Id: busMsgQue.h 84 2018-03-12 21:26:53Z coats $" ;
#endif

struct BusMessage {
  int toModule;
  /* if sending (vs. receiving), the name of the module being
     sent to, or one of the "special" destinations
     if receiving, the destination the module the message was
     sent to - possibilities include the receiving module's id,
     BusBroadcast, BusByType */
  
  int fromModule;
  /* if sending, doesn't matter, this is overwritten by the bus
     if receiving, the id of the module that sent the message,
     (the id to be used to reply to the message) */
  
  int serial;
  /* a message id number set by the bus to uniquely
     identify messages sent thru the bus */
  
  /* int messageClass; */ 	/* Changed to messageOption by Rajini */
  /* The class of data contained in the message,
     kind of a broad type */

  unsigned char messageOption;
  /* The type of the request/response contained in the message */
  
  int messageType;
  /* The type of data pointed to by the data member, may be a
     standard bus type, or a type registered with the bus */
  
  int messageLength;
  /* the length of the data pointed to by the data member */
  
  char *message;
  /* the data member - the "body" of the message
     may contain any type of data, it is not type checked in any way
     BusRead will allocate this memory block,
     and it is the user priority to then free that block */
};


#define BusBroadcast     -3
/* BusBroadcast is used to send a message to everyone on the bus */
#define BusByType        -4
/* BusByType is used to send a message to all module on the bus who
   understand a message of the type specified (in the BusMessage object) */
#define BusBounce        -5
/* BusBounce is used to talk to yourself - message are send back to the
   sender (could be used to find the current module bus id) */


/* BusMessageQueue structure is intended to be a node in a
   circular singly linked list (used as a queue) */
struct BusMessageQueue {
  struct BusMessageQueue *next;
  
  struct BusMessage       data;
};

struct BusMessage *
BusGetFirstMessage( struct BusMessageQueue *last );
/* returns a pointer to the first BusMessage in the queue,
   this pointer should not be freed - the memory it
   points to is allocated and deallocated by
   Bus_EnqueMessage and Bus_DequeMessage
   last - in - pointer to the last node in a circular list of
   BusMessageQueue Nodes
   returns a pointer to a BusMessage structure that is part
   of a BusMessageQueue Node */

int Bus_MessagesLeft( struct BusMessageQueue *last );
/* returns true(1) if their is a message in the queue,
   0 otherwise */

struct BusMessageQueue *
Bus_DequeMessage( struct BusMessageQueue *last );
/* removes the first item from the queue,
   last - in - pointer to the last node in a circular linked
   list of BusMessageQueue Nodes
   returns a pointer to the last node in the "new" list
   (which may be NULL for an empty list)
   */

struct BusMessageQueue *
Bus_EnqueMessage(struct BusMessageQueue *last,
		 struct BusMessage *bmsg );
/* adds a message to the queue,
   last - in - pointer to the last node in a circular linked
   list of BusMessageQueue Nodes
   bm   - in - a BusMessage structure containing the message
   to be added to the queue
   returns a pointer to the last node in a circular linked
   list of BusMessageQueue Nodes
   or NULL if an error occurs
   */

struct BusMessage *
BusGetMessageByModuleOption( struct BusMessageQueue *last, int moduleId,
				unsigned char option );

struct BusMessageQueue *
Bus_DequeMessageByModuleOption( struct BusMessageQueue *last, int moduleId,
				unsigned char option );

struct BusMessage *
BusGetMessageBySeq( struct BusMessageQueue *last, int seq);

struct BusMessageQueue *
Bus_DequeMessageBySeq( struct BusMessageQueue *last, int seq);

struct BusMessageQueue *
Bus_DequeMessageByModuleSeq( struct BusMessageQueue *last, int moduleId, int seq);

#endif
