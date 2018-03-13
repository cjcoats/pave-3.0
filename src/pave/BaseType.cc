/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: BaseType.cc 83 2018-03-12 19:24:33Z coats $
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
// BaseType.C
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 23, 1995
//
/////////////////////////////////////////////////////////////
//
// BaseType Class
//
// Portions from Roger Sessions' "Class Construction in C
// and C++", copyright 1992 by Prentice-Hall
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT  950523  Implemented
//
/////////////////////////////////////////////////////////////


#include "BaseType.h"


static void terminate ( char * );



baseType::~baseType()
    {
    }


int baseType::match ( void *target )
    {
    fprintf ( stderr, "match(%ld)\n", ( long ) target );
    terminate ( "match" );
    return 0;
    }


void baseType::print ( FILE *output )
    {
    fprintf ( stderr, "print(%ld)\n", ( long ) output );
    terminate ( "print" );
    }


void terminate ( char *methodName )
    {
    printf ( "Method %s\ninvoked, but not implemented for this type\n",
             methodName );
    exit ( 0 );
    }


// the assumption here is that every derived class
// will override this method, using a similar method
// but replacing the string baseType with the name
// of its own class
char *baseType::getClassName()
    {
    static char *myName = "baseType";
    return myName;
    }
