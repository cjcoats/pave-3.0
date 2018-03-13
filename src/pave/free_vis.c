/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: free_vis.c 83 2018-03-12 19:24:33Z coats $
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
 *  Author:  Kathy Pearson, MCNC, kathyp@mcnc.org
 *  Date:    December 12, 1994
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#include "netcdf.h"
#include "vis_data.h"

/* This function frees the malloced pointers in the VIS_DATA structure */
void free_vis ( VIS_DATA *info )
    {
    register int i;

    if ( !info ) return; /* Added 960419 SRT */

    /* free double pointer sources */

    for ( i = 0; i < ( *info ).nspecies; i++ )
        {
        if ( ( *info ).species_short_name != NULL )
            if ( ( *info ).species_short_name[i] != NULL )
                free ( ( *info ).species_short_name[i] );
        if ( ( *info ).species_long_name != NULL )
            if ( ( *info ).species_long_name[i] != NULL )
                free ( ( *info ).species_long_name[i] );
        if ( ( *info ).units_name != NULL )
            if ( ( *info ).units_name[i] != NULL )
                free ( ( *info ).units_name[i] );
        }

    /* free pointer allocated by user */

    if ( ( *info ).filename != NULL )
        free ( ( *info ).filename );

    /* free other pointers */

    if ( ( *info ).species_short_name != NULL )
        free ( ( *info ).species_short_name );
    if ( ( *info ).species_long_name != NULL )
        free ( ( *info ).species_long_name );
    if ( ( *info ).units_name != NULL )
        free ( ( *info ).units_name );
    if ( ( *info ).map_info != NULL )
        free ( ( *info ).map_info );
    if ( ( *info ).data_label != NULL )
        free ( ( *info ).data_label );
    if ( ( *info ).grid != NULL )
        free ( ( *info ).grid );
    if ( ( *info ).sdate != NULL )
        free ( ( *info ).sdate );
    if ( ( *info ).stime != NULL )
        free ( ( *info ).stime );

    /* free host pointers */
    if ( ( *info ).filehost.ip != NULL )
        free ( ( *info ).filehost.ip );
    if ( ( *info ).filehost.name != NULL )
        free ( ( *info ).filehost.name );

    memset ( ( void * ) info, 0, ( size_t ) sizeof ( VIS_DATA ) ); /* SRT 061395 */
    }

