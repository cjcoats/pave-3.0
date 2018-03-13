/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: migrate.c 83 2018-03-12 19:24:33Z coats $
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
 * Author:  Kathy Pearson, MCNC, kathyp@mcnc.org, circa 1993
/*****************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

/*
#define UNICOS 1
*/

int get_migrate_state ( char *filename, int *online, int *offline )
    {
    struct stat statbuf;
    int istat;

#define IOERROR -1

    if ( ( istat = stat ( filename, &statbuf ) ) != IOERROR )
        {
#ifdef UNICOS
        if ( statbuf.st_dm_state == S_DMS_REGULAR )
            {
            *online = 1;
            *offline = 0;
            }
        else if ( statbuf.st_dm_state == S_DMS_OFFLINE )
            {
            *online = 0;
            *offline = 1;
            }
        else if ( statbuf.st_dm_state == S_DMS_DUALSTATE )
            {
            *online = 1;
            *offline = 1;
            }
#else
        *online = 1;
        *offline = 0;
#endif
        return ( 1 );
        }
    return ( 0 );
    }

