/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: StringPair.cc 83 2018-03-12 19:24:33Z coats $
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



#include "StringPair.h"


StringPair::StringPair ( char *name, char *value )
    {

    name_ = strdup ( name );
    value_ = strdup ( value );
    }

StringPair::~StringPair()
    {

    if ( name_ ) free ( name_ );
    if ( value_ ) free ( value_ );

    }


int StringPair::match ( void *target )
    {
    char *targetName = ( char * ) target;

    return ( !strcmp ( targetName, name_ ) );

    }

char *StringPair::getClassName ( void )
    {
    static char *name="StringPair";
    return name;
    }

void StringPair::print ( FILE *output )
    {

    fprintf ( output,"%s <%s>\n",name_, value_ );

    }



