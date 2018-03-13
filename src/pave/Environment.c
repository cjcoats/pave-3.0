/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Environment.c 83 2018-03-12 19:24:33Z coats $
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
 ****************************************************************************
 *  CONTAINS
 *      Static state-variables for PAVE environment variables.
 *      Static routine for initializing state-variables
 *      Public routines for returning state-variables
 ****************************************************************************
 *  REVISION HISTORY
 *      
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ****************************************************************************/
 
 #include <stdlib.h>
 #include <string.h>

 static char    pave_dir_buf[ 512 ] ;
 static char    pave_map_buf[ 512 ] ;
 static char    pave_bin_buf[ 512 ] ;
 static char    pave_doc_buf[ 512 ] ;

 static int     env_init = 0 ;

 static int pave_env_init()
    {
    char    * name ;
    size_t  namlen ;
    
    if ( env_init )  return 1 ;

    name   = getenv ( "PAVE_DIR" ) ;
    namlen = strlen( name ) ;
    if ( name && ( namlen > 0 ) && ( namlen < 500 ) )
        {
        sprintf( pave_dir_buf, "%s", name ) ;
        sprintf( pave_doc_buf, "%s/%s", name, "Docs" ) ;
        }
    else{
        return 0 ;
        }
    
    name   = getenv ( "PAVE_BINDIR" ) ;
    namlen = strlen( name ) ;
    if ( name && ( namlen > 0 ) && ( namlen < 512 ) )
        {
        sprintf( pave_bin_buf, "%s", name ) ;
        }
    else{
        sprintf( pave_bin_buf, "%s/bin", pave_dir_buf
        }
    
    name   = getenv ( "PAVE_MAPDIR" ) ;
    namlen = strlen( name ) ;
    if ( name && ( namlen > 0 ) && ( namlen < 512 ) )
        {
        sprintf( pave_bin_buf, "%s", name ) ;
        }
    else{
        sprintf( pave_bin_buf, "%s/maps", pave_dir_buf
        }

    }

 char  * pave_dir()
    {
    if ( ! pave_env_init() ) return (char *) 0 ;
    return pave_dir_buf ;
    }

 char  * pave_bindir()
    {
    if ( ! pave_env_init() ) return (char *) 0 ;
    return pave_bin_buf ;
    }

 char  * pave_mapdir()
    {
    if ( ! pave_env_init() ) return (char *) 0 ;
    return pave_map_buf ;
    }
