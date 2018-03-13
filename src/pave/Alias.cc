/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Alias.cc 83 2018-03-12 19:24:33Z coats $
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

#include "Alias.h"

static const char SVN_ID[] = "$Id: Alias.cc 83 2018-03-12 19:24:33Z coats $";

Alias::Alias ( char *name, int ntoken, char **token, int *tflag )
    {
    int i;

    name_ = strdup ( name );
    ntoken_ = ntoken;
    token_ = NULL;
    tflag_ = NULL;
    if ( ntoken > 0 )
        {
        token_ = ( char ** ) malloc ( ntoken*sizeof ( char * ) );
        tflag_ = ( int * ) malloc ( ntoken*sizeof ( int ) );
        if ( token_ == NULL || tflag_ == NULL )
            {
            fprintf ( stderr,"Allocation error in Alias\n" );
            return;
            }
        for ( i=0; i<ntoken; i++ )
            {
            tflag_[i] = tflag[i];
            token_[i] = strdup ( token[i] );
            }
        }

    }

Alias::~Alias()
    {
    int i;

    if ( ntoken_ > 0 )
        {
        for ( i=0; i<ntoken_; i++ )
            {
            if ( token_[i] ) free ( token_[i] );
            }
        free ( token_ );
        free ( tflag_ );
        free ( name_ );
        }
    }


int Alias::match ( void *target )
    {
    char *targetName = ( char * ) target;

    return ( !strcmp ( targetName, name_ ) );

    }

char *Alias::getClassName ( void )
    {
    static char *name="Alias";
    return name;
    }

void Alias::print ( FILE *output )
    {
    int i;
    fprintf ( output,"%s=",name_ );

    for ( i=0; i<ntoken_; i++ )
        {

        //    if(token_[i]) fprintf(output,"%s",token_[i]);
        //    if(tflag_[i]) fprintf(output,"<case>");
        fprintf ( output,"%s",token_[i] );
        }

    fprintf ( output,"\n" );

    }



