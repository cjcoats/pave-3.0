/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: buslib.h 83 2018-03-12 19:24:33Z coats $
 *  Copyright (C) 2018-     Carlie J. Coats, Jr., Ph.D.
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
 * ABOUT:   buslib.h
 *
 *      Supercedes bus.h, busClient.h, busDebug.h, busError.h, busMsgQue.h,
 *      busRpc.h, busRW.h, busUtil.h, busVersion.h, busXtClient.h,
 *      busRWMessage.h, busFtp.h
 *
 * VERSION "$Id: buslib.h 83 2018-03-12 19:24:33Z coats $"
 ********************************************************************
 *
 * HISTORY:
 *      Initial version 2/2018 by Carlie J. Coats, Jr.
 *********************************************************************/


#ifndef SBUS_H_INCLUDED
#define SBUS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <dirent.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Intrinsic.h>


/**************************  From busVersion.h  **************************/

#define SBUSVERSION "Version 6.6e (14-Feb-2018)"
#define BusVersion()     SBUSVERSION

#define SBUS_ID     "$Id: buslib.h 83 2018-03-12 19:24:33Z coats $"


/**************************  From busDebug.h  **************************/

#define debug0(c,x)         { if (c) { printf(x); fflush(stdout);}}
#define debug1(c,x,y)       { if (c) { printf(x,y); fflush(stdout);}}
#define debug2(c,x,y,z)     { if (c) { printf(x,y,z); fflush(stdout);}}
#define debug3(c,x,y,z,a)   { if (c) { printf(x,y,z,a); fflush(stdout);}}
#define debug4(c,x,y,z,a,b) { if (c) { printf(x,y,z,a,b); fflush(stdout);}}

#define DEBUG_CLIENT    0
#define DEBUG_MASTER    0
#define DEBUG_DISP      0
#define DEBUG_RWMSG     0
#define DEBUG_RESPONDER 0
#define DEBUG_QUEUE     0
#define DEBUG_NEW_MOD   0
#define DEBUG_SOCKET    0

void debugShowMessage ( struct BusMessage *bmsg );


/**************************  From busError.h  **************************/

#define SBUSERROR_NOT                    0
#define SBUSERROR_DUP_CLIENT            -1

#define SBUSERROR_INITIALIZATION        -2
#define SBUSERROR_SBUSPORT_NOT_SET      -15

#define SBUSERROR_READ                  -3
#define SBUSERROR_WRITE                 -4
#define SBUSERROR_NODATA                -5

#define SBUSERROR_NOMEMORY              -6

#define SBUSERROR_BADPARAMETER          -7
#define SBUSERROR_CBNOTFOUND            -8

#define SBUSERROR_MSG_NOT_UNDERSTOOD    -9

#define SBUSERROR_SOCKET_COULDNT_CREATE -10
#define SBUSERROR_COULDNT_getsockname   -11
#define SBUSERROR_ACCEPT_FAILED         -12
#define SBUSERROR_INVALIDHOST           -13
#define SBUSERROR_CONNECT_FAILED        -14

#define SBUSERROR_MASTER_ABSENT         -16
#define SBUSERROR_GENERAL_FAILURE       -17

/* Definitions added by Rajini */
#define FIND_ID_ERR                     -1
#define FIND_NAME_ERR                   " "


/**************************  From busMsgQue.h  **************************/

struct BusMessage
    {
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

    /* int messageClass; */   /* Changed to messageOption by Rajini */
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
struct BusMessageQueue
    {
    struct BusMessageQueue *next;

    struct BusMessage       data;
    };

struct BusMessage *
BusGetFirstMessage ( struct BusMessageQueue *last );
/* returns a pointer to the first BusMessage in the queue,
   this pointer should not be freed - the memory it
   points to is allocated and deallocated by
   Bus_EnqueMessage and Bus_DequeMessage
   last - in - pointer to the last node in a circular list of
   BusMessageQueue Nodes
   returns a pointer to a BusMessage structure that is part
   of a BusMessageQueue Node */

int Bus_MessagesLeft ( struct BusMessageQueue *last );
/* returns true(1) if their is a message in the queue,
   0 otherwise */

struct BusMessageQueue *
Bus_DequeMessage ( struct BusMessageQueue *last );
/* removes the first item from the queue,
   last - in - pointer to the last node in a circular linked
   list of BusMessageQueue Nodes
   returns a pointer to the last node in the "new" list
   (which may be NULL for an empty list)
   */

struct BusMessageQueue *
Bus_EnqueMessage ( struct BusMessageQueue *last,
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
BusGetMessageByModuleOption ( struct BusMessageQueue *last, int moduleId,
                              unsigned char option );

struct BusMessageQueue *
Bus_DequeMessageByModuleOption ( struct BusMessageQueue *last, int moduleId,
                                 unsigned char option );

struct BusMessage *
BusGetMessageBySeq ( struct BusMessageQueue *last, int seq );

struct BusMessageQueue *
Bus_DequeMessageBySeq ( struct BusMessageQueue *last, int seq );

struct BusMessageQueue *
Bus_DequeMessageByModuleSeq ( struct BusMessageQueue *last, int moduleId, int seq );



/**************************  From busClient.h  **************************/
/**************************  Depends on  busMsgQue.h  *******************/

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


struct BusData
    {
    /* name client wants to be known by to the bus */
    char      *name;

    /* bus id number */
    int        moduleId;

    /* Added by Rajini */
    int        nextSeqNum;

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

struct BusModuleData
    {
    int moduleId;
    char *name;
    char *ip_addr;
    } ;

/*** Functions ***/

/* int BusInitialize( struct BusData *bd ); */
int BusInitialize ( struct BusData *bd, int setTimeout, int minsToLive,
                    void ( *exitCallback ) ( struct BusData *bd ) );
/* Connects a client to the software bus, and
   completes initialization of the BusData structure
   bd - in - a valid pointer to a bus data structure
   bd->name point to an ASCIIZ string which is the
   name of the client (that the client will be known
   by to the bus) */

void BusClose ( struct BusData *bd );
/* close a connection to the bus
   bd - in - pointer to BusData structure for
   connection to be closed */

/*************************************************************
  BusAddInputCallback, BusRemoveInputCallback
  are intended for use with the function
  BusEventLoop
  ************************************************************/

typedef void ( *BusInputCBfunc ) ( int, struct BusData * );

int BusAddInputCallback ( struct BusData *bd,
                          int fd,
                          BusInputCBfunc   read_callback,
                          BusInputCBfunc  write_callback,
                          BusInputCBfunc except_callback );
/* For use with BusEventLoop */
/* Adds a set of callbacks for a given file descriptor
   for BusEventLoop to check and call when the file
   is in the specified state */
/* bd - in - a BusData struct initialized by BusInitialize   */
/* fd - in - the file descriptor to add the callback for     */
/* read_callback - in - a callback for when data can be read */
int BusRemoveInputCallback ( struct BusData *,int fd );

void BusEventLoop ( struct BusData *bd );
/* The function can be called by text base client programs   *
 * which can operate as event driven loops                   */

int BusAddTypeCallback ( struct BusData *bd, int type,
                         void ( *callback ) ( struct BusData *,
                                 struct BusMessage * ) );
/* add a function to the set of functions to be called when
   a message of the specified type is received */

int BusRemoveTypeCallback ( struct BusData *bd, int type,
                            void ( *callback ) ( struct BusData *,
                                    struct BusMessage * ) );
/* removes a function from the set of functions to be called
   when a message of the specified type is received */

int BusDispatch ( struct BusData *bd );
/* (internal function) Handles the reading of a signal from the
   bus master, and takes an appropriate action (such a reading
   a message, and then calling the callback for that message */

void BusProcessRecvdMessages ( struct BusData *bd );

/* Added by Rajini */
void BusProcessOption ( struct BusData *bd, struct BusMessage *bmsg );

int BusFindModuleByName ( struct BusData *bd, const char *name );
/* returns a Module's id # given the modules name (-1 if not found) */

char *BusFindModuleById ( struct BusData *bd, int typeId );
/* returns a Module's name given its id # (NULL if not found)
   - free the return pointer when finished with it */

int BusFindTypeByName ( struct BusData *bd, const char *name );
/* returns a type's id # given the name,
   an id number is assigned if the type is not found in the
   pre-existing list */

char *BusFindTypeById ( struct BusData *bd, int id );
/* returns a type's name given its id #, (NULL if not found/error)
   free the (char *) when finished with it */

int BusRegisterType ( struct BusData *bd, int typeId );
/* registers a type as being "understood" by the client,
   primarily for purposes of getting message sent to BusByType */

struct BusModuleData *BusFindWhosConnected ( struct BusData *bd, int *numModules );
/* Find the modules that are connected to the Bus */

int BusGetModuleInfoById ( struct BusData *bd, int moduleId, struct BusModuleData *moduleInfo );
int BusGetModuleInfoByName ( struct BusData *bd, const char *name, struct BusModuleData *moduleInfo );
/* Find information about a particular module */

int BusSendMessage ( struct BusData *bd, struct BusMessage *bmsg );
/* queues a messages for distribution to the rest of the bus */

int BusBroadcastMessage ( struct BusData *bd, int typeId,
                          char *message, int length );
/* enques a message for broadcasting to all other modules attached to the bus */

int BusSendByType ( struct BusData *bd, int typeId,
                    char *message, int length );
/* enques a message for broadcasting a message according to type */

int BusBounceMessage ( struct BusData *bd, int typeId,
                       char *message, int length );
/* enques a message to be bounced off the busMaster */

int BusSendASCIIZ ( struct BusData *bd, int toModule, char *message );
/* enques a ASCIIZ message */

int BusBroadcastASCIIZ ( struct BusData *bd, char *message );
/* enques an ASCIIZ message for broadcasting */

int BusAddDirectCallback ( struct BusData *bd,
                           int typeId,
                           void ( *callback ) ( int, char* ),
                           char *data );

/* specifies the function to be called when 'select' in BusEventLoop
   times out. */
int BusAddTimeoutCallback ( struct BusData *bd, struct timeval *timeout,
                            void ( *callback ) ( struct BusData * ) );
int BusRemoveTimeoutCallback ( struct BusData *bd );

/* specifies a function to be called when a point-to-point (direct) connection
   is request for the given type */

int BusRemoveDirectCallback ( struct BusData *bd,
                              int typeId,
                              void ( *callback ) ( int, char* ) );
/* removes a function (from the list of functions) specified to handle a
   point-to-point connection */

int BusSendDirect ( struct BusData *bd, int toModule, int typeId,
                    void ( *send_call ) ( int, char * ), char *data );
/* establishes a point-to-point connection, calls a user-function
   transmit the data, and closes the connection upon return of the
   function */

int BusKillClient ( struct BusData *bd, const char *name );
/* Function available to the Console client to send a Kill message to
   any other client on the Bus */


/**************************  From busXtClient.h  **************************/
/**************************  Depends upon  busClient   ********************/

struct BusXtData
    {
    XtInputId xtid;
    };

/***********************************************************
  For use of the bus by X toolkit (Motif/Athena/Openlook)
  based programs
  **********************************************************/

int BusXtInitialize ( struct BusData *bd,
                      XtAppContext app_context );
/* connects this client to the bus, and setups an
   XtCallback to handle bus reads */

void BusXtClose ( struct BusData *bd );
/* should be used to close a connection created by
   BusXtInitialize */


/**************************  From busUtil.h  **************************/

#define ERR_BUFSIZE  256

int BusVerifyClient ( struct BusData *bd, char *ipAddress, char *moduleName,
                      int isUnique, int secsToWait, char *args, char *errorstr );
void BusGetMyIPaddress ( char *ipaddr );
void BusExitCallback ( struct BusData *bd );


/**************************  From busSocket.h  **************************/

/* First, prototypes for socket related function for systems
   lacking them */
/* sys/types.h, sys/socket.h, sys/time.h
   should be included by this file */

/* Include files added by Rajini */
#include <sys/types.h>
#include <sys/time.h>

#ifndef NOFDSETPTR
#define SELECTFDSETPTR fd_set *
#else
#define SELECTFDSETPTR int *
#endif /* NOFDSETPTR */

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK (0x7f000001)
#endif

#ifdef INCLUDE_SOCKET_PROTOTYPES

/* Standard TCP/IP socket connection routines */
int accept ( int s, struct sockaddr *addr, int *addrlen );
int bind ( int s, const struct sockaddr *name, int namelen );
int close ( int );
int connect ( int s, struct sockaddr *name, int namelen );
struct hostent *gethostbyname ( char *host_name );
char *gethostname ( char *name, int namelen ); /* namelen should be 32 */
int getsockname ( int s, struct sockaddr *name, int *namelen );
int listen ( int s, int backlog );
int shutdown ( int s, int how );
int socket ( int domain, int type, int protocol );

#ifndef FD_SETSIZE
int getdtablesize ( void );
#define FD_SETSIZE (getdtablesize())
#endif

#ifndef NOFDSETPTR
int select ( int nfds, fd_set *readfds, fd_set *writefds,
             fd_set *exceptfds, struct timeval *timeout );
#else
int select ( int nfds, int *readfds, int *writefds,
             int *exceptfds, struct timeval *timeout );
#endif /* NOFDSETPTR */

int sendto ( int s, const char *msg, int len, int flags,
             const struct sockaddr *to, int tolen );

int recvfrom ( int s, char *buf, int len, int flags,
               struct sockaddr *from, int *fromlen );

#else  /* INCLUDE_SOCKET_PROTOTYPES */
#endif /* INCLUDE_SOCKET_PROTOTYPES */

/* Now for the functions defined in busSocket.c */

int busSocket_createAcceptorSocket ( int *portNumber );
int busSocket_makeBoundConnectionless ( int *portNumber );
int busSocket_acceptConnection ( int portSocket,
                                 struct sockaddr_in *sad );
int busSocket_acceptFromConnectionless ( int portSocket,
        struct sockaddr_in *sin );
int busSocket_makeConnection ( char *host_addr, int port );


/**************************  From busRWMessage.h  **************************/
/**************************  Depends upon busMsgQue, busClient  ***********/

int getSeqNum ( struct BusData * );
int BusReadMessage ( int, struct BusMessage * );
int BusWriteMessage ( int, struct BusMessage * );
int BusSendBusByte ( struct BusData *, int, unsigned char, int,  char * );
int BusReplyBusByte ( int, int, int, int, unsigned char, int,  char * );
int BusForwdBusByte ( int, int, int, int, unsigned char, int,  char * );
int BusGetResponse ( struct BusData *, int, unsigned char, char ** );


/**************************  From busRpc.h  **************************/

int BusCallRemote ( struct BusData *bd, int toModule, int typeId,
                    void ( *stub ) ( int, char *, char * ), char *args, char *res );


/**************************  From busRw.h  **************************/

#define BUFSIZE 4000  /* size of buffer used in the XDR routines */

/* id - input - an "id" number for the communication channel
   to use - (the socket, pipe, port, or whatever else)
   */

int BusReadCharacter ( int id, unsigned char  *c );
int BusReadInteger   ( int id, int   *i );
int BusReadFloat     ( int id, float *f );

int BusReadASCIIZ    ( int id, char **buffer );
int BusReadnString   ( int id, char **buffer, int *buflen );

int BusWriteCharacter ( int id, unsigned char   c );
int BusWriteInteger  ( int id, int    i );
int BusWriteFloat    ( int id, float  f );

int BusWriteASCIIZ   ( int id, char  *string );
int BusWritenString  ( int id, char  *buffer, int  length );

char *BusGetMyUserid();


/**************************  From busFtp.h  **************************/
/**************************  Depends upon all of the above   *********/

#define MAXLINE         1024
#define MAXDATA         4000
#define MAXNUMFILES     512
#define FTP_ERR_FOPEN   -1
#define FTP_ERR_NONE    1
#define FTP_ERR_HOST    -2
#define FTP_ERR_LOCALHOST -3
#define FTP_ERR_CONNECT -4

#define FTP_GET 1
#define FTP_PUT 2

#define DIR_OPEN_ERR     -1
#define DIR_HOST_UNKNOWN -2
#define DIR_REMOTE_FETCH -3

#define IS_DIR  0
#define IS_FILE 1


#define DEBUG_FTP 0


int FTP_xferBINARY ( struct BusData *bd, int mode, char *hostName,
                     char *localFile, char *remoteFile );

/* Functions for Directory search */
int FTP_getdir ( struct BusData *bd, char *hname, char *directory, int code,
                 int returnType, char **fileList );
int FTP_dirRemote ( struct BusData *bd, char *ipaddr, char *directory,
                    int code, int returnType );
int FTP_dirLocal ( char *directory, int code, char **fList );

int is_localhost ( char *hname, char *ipaddr );
int is_accessible( char *file );


/**************************  From busDebug.h  **************************/


/**************************  From busDebug.h  **************************/











#ifdef __cplusplus
    }
#endif

#endif /* SBUS_H_INCLUDED */

