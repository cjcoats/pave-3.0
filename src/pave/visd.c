/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: visd.c 83 2018-03-12 19:24:33Z coats $
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
 *  REVISION HISTORY
 *      Author:      Rajini Balay, NCSU
 *      Date:        February 25, 1995
 *****************************************************************************/

/* bald messes this up in some header file */
#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */

#include "visDataClient.h"

static const char SVN_ID[] = "$Id: visd.c 83 2018-03-12 19:24:33Z coats $";

int main( int argc, char *argv[] )
    {
    struct BusData bd;

#ifdef DIAGNOSTICS
    char local_hname[256];
    gethostname ( local_hname, 256 );
    fprintf ( stderr, "Enter visd on %s with SBUSHOST '%s' and SBUSPORT '%s'\n",
              local_hname, getenv ( "SBUSHOST" ), getenv ( "SBUSPORT" ) ); /*SRT*/
#endif /* DIAGNOSTICS */

    initVisDataClient ( &bd,"visd" );

    BusEventLoop ( &bd );

    return 0; /* added SRT */
    }
