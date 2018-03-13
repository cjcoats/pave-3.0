/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 2.4
 *
 *  File: $Id: busSocket.h 84 2018-03-12 21:26:53Z coats $
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
 *                                    *
 * ABOUT:  busSocket.h
 *
 *      Definitions, structures and prototypes for the bus socket
 *      manipulation routines.
 *
 ****************************************************************************
 *  REVISION HISTORY
 *      
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 ****************************************************************************/

#ifndef BUS_SOCKET_INCLUDED
#define BUS_SOCKET_INCLUDED

#ifdef SBUS_INCLUDE_HDR_DEFN
static char *vhbusSocket = "$Id: busSocket.h 84 2018-03-12 21:26:53Z coats $" ;
#endif

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
int accept(int s, struct sockaddr *addr, int *addrlen);
int bind(int s, const struct sockaddr *name, int namelen);
int close(int);
int connect(int s, struct sockaddr *name, int namelen);
struct hostent *gethostbyname( char *host_name );
char *gethostname( char *name, int namelen ); /* namelen should be 32 */
int getsockname(int s, struct sockaddr *name, int *namelen);
int listen(int s, int backlog);
int shutdown(int s, int how);
int socket(int domain, int type, int protocol);

#ifndef FD_SETSIZE
int getdtablesize( void );
#define FD_SETSIZE (getdtablesize())
#endif

#ifndef NOFDSETPTR
int select(int nfds, fd_set *readfds, fd_set *writefds,
	   fd_set *exceptfds, struct timeval *timeout);
#else
int select(int nfds, int *readfds, int *writefds,
	   int *exceptfds, struct timeval *timeout);
#endif /* NOFDSETPTR */

int sendto(int s, const char *msg, int len, int flags,
	   const struct sockaddr *to, int tolen);

int recvfrom(int s, char *buf, int len, int flags,
	     struct sockaddr *from, int *fromlen);

#else  /* INCLUDE_SOCKET_PROTOTYPES */
#endif /* INCLUDE_SOCKET_PROTOTYPES */

/* Now for the functions defined in busSocket.c */

int busSocket_createAcceptorSocket(int *portNumber);
int busSocket_makeBoundConnectionless( int *portNumber );
int busSocket_acceptConnection(int portSocket,
			       struct sockaddr_in *sad );
int busSocket_acceptFromConnectionless(int portSocket,
				       struct sockaddr_in *sin);
int busSocket_makeConnection(char *host_addr, int port);

#endif
