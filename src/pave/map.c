/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: map.c 83 2018-03-12 19:24:33Z coats $
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
 *      Created 1994?? by ??
 *      SRT  04/06/95   Added #include "vis_proto.h"
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "vis_proto.h"

#define MAP_CMD "$BLT_WISH -f $PAVE_BINDIR/map.blt"

#define PAVE_SUCCESS 1
#define FAILURE 0

int map ( float *x, float *y, int nlines, int *npoints,
          char *title, char *xaxis_label, char *yaxis_label, char *message )
    {
    char tmpfilename[ 512 ];
    char     map_cmd[ 512 ];
    FILE *tcl_input;
    int status;
    register int i, j, k;
    pid_t pid;

    status = FAILURE;

    pid = getpid();
    sprintf ( tmpfilename, "/tmp/map.blt.%d", pid );

    sprintf ( map_cmd, "%s < %s&", MAP_CMD, tmpfilename );

    if ( ( tcl_input = fopen ( tmpfilename, "w" ) ) != NULL )
        {
        if ( fprintf ( tcl_input, "%s\n", tmpfilename ) < 0 )
            goto WRITE_MAP_ERROR;
        if ( xaxis_label == NULL )
            {
            if ( fprintf ( tcl_input, "option add *graph.xTitle \"%s\"\n",
                           "X Axis Label" ) < 0 )
                goto WRITE_MAP_ERROR;
            }
        else
            {
            if ( fprintf ( tcl_input, "option add *graph.xTitle \"%s\"\n",
                           xaxis_label ) < 0 )
                goto WRITE_MAP_ERROR;
            }
        if ( yaxis_label == NULL )
            {
            if ( fprintf ( tcl_input, "option add *graph.yTitle \"%s\"\n",
                           "Y Axis Label" ) < 0 )
                goto WRITE_MAP_ERROR;
            }
        else
            {
            if ( fprintf ( tcl_input, "option add *graph.yTitle \"%s\"\n",
                           yaxis_label ) < 0 )
                goto WRITE_MAP_ERROR;
            }
        if ( title == NULL )
            {
            if ( fprintf ( tcl_input, "option add *graph.title \"%s\"\n",
                           " " ) < 0 )
                goto WRITE_MAP_ERROR;
            }
        else
            {
            if ( fprintf ( tcl_input, "option add *graph.title \"%s\"\n",
                           title ) < 0 )
                goto WRITE_MAP_ERROR;
            }

        k = 0;
        for ( i = 0; i < nlines; i++ )
            {
            if ( fprintf ( tcl_input, "set X%d { ", i+1 ) < 0 )
                goto WRITE_MAP_ERROR;
            for ( j = 0; j < npoints[i]; j++ )
                {
                if ( fprintf ( tcl_input, "%g ", x[k+j] ) < 0 )
                    goto WRITE_MAP_ERROR;
                }
            if ( fprintf ( tcl_input, "}\n" ) < 0 )
                goto WRITE_MAP_ERROR;

            if ( fprintf ( tcl_input, "set Y%d { ", i+1 ) < 0 )
                goto WRITE_MAP_ERROR;
            for ( j = 0; j < npoints[i]; j++ )
                {
                if ( fprintf ( tcl_input, "%g ", y[k+j] ) < 0 )
                    goto WRITE_MAP_ERROR;
                }
            if ( fprintf ( tcl_input, "}\n" ) < 0 )
                goto WRITE_MAP_ERROR;

            if ( fprintf ( tcl_input,
                           "$graph element create \"line%d\" -xdata $X%d -ydata $Y%d -symbol line -bg blue -foreground blue -linewidth 1\n",
                           i+1, i+1, i+1 ) < 0 )
                goto WRITE_MAP_ERROR;
            k += npoints[i];
            }
        }
    else
        goto CREATE_MAP_ERROR;

    fflush ( tcl_input );
    if ( fclose ( tcl_input ) != 0 )
        goto CLOSE_MAP_ERROR;

    if ( system ( map_cmd ) == 0 )
        status = PAVE_SUCCESS;
    return ( status );

CREATE_MAP_ERROR:
    sprintf ( message, "Error creating file: %s\n", tmpfilename );
    return ( status );

WRITE_MAP_ERROR:
    sprintf ( message, "Error writing to file: %s\n", tmpfilename );
    fclose ( tcl_input );
    return ( status );

CLOSE_MAP_ERROR:
    sprintf ( message, "Error closing file: %s\n", tmpfilename );
    return ( status );
    }

