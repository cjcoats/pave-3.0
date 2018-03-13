/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: graph2d.c 83 2018-03-12 19:24:33Z coats $
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
 *  MODIFICATION HISTORY:
 *
 *      WHO  WHEN       WHAT
 *      SRT  09/18/95   Added lineWidth param to graph2d() routine
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "nan_incl.h"

#define PAVE_SUCCESS 1
#define FAILURE 0

/* replaces any [ or ] chars with ( or ) chars, respectively */
void replaceBrackets ( char *s )
    {
    int len, i;

    if ( !s ) return;
    len = strlen ( s );
    if ( !len ) return;
    for ( i = 0; i < len; i++ )
        {
        if ( s[i] == '[' ) s[i] = '(';
        if ( s[i] == ']' ) s[i] = ')';
        }
    return;
    }


int graph2d ( float *x, float *y, int nlines, int *npoints,
              char *title, char *xaxis_label, char *yaxis_label,
              char **legend, char **symbol, char **color, char *message,
              int lineWidth )
    {
    char tmpfilename[ 512 ];
    char graph2d_cmd[ 512 ];
    FILE *tcl_input;
    int status;
    int lw;
    register int i, j, k;
    pid_t pid;
    char *graph2d_cfg;
    static int num2d = 0;
    FILE *cmdpipe;
#define BUFSIZE 256
    static char response[BUFSIZE];
    float value;
    char *hlineval;

    status = FAILURE;
    replaceBrackets ( title );
    replaceBrackets ( xaxis_label );
    replaceBrackets ( yaxis_label );

    pid = getpid();
    sprintf ( tmpfilename, "/tmp/graph2d.blt.%d.%d", pid, num2d++ );

    sprintf ( graph2d_cfg, "%s/graph2d.blt", getenv( "PAVE_BINDIR" ) ) ;
    if ( graph2d_cfg == NULL )
        {
        sprintf ( graph2d_cmd, "%s -f %s < %s&", getenv( "BLT_WISH" ), "$PAVE_BINDIR/graph2d.blt", tmpfilename );
        }
    else
        {
        sprintf ( graph2d_cmd, "%s -f %s < %s&", getenv( "BLT_WISH" ), graph2d_cfg, tmpfilename );
        }

    if ( ( tcl_input = fopen ( tmpfilename, "w" ) ) != NULL )
        {
        if ( fprintf ( tcl_input, "%s\n", tmpfilename ) < 0 )
            goto WRITE_GRAPH2D_ERROR;
        if ( xaxis_label == NULL )
            {
            if ( fprintf ( tcl_input, "option add *graph.xTitle \"%s\"\n",
                           "X Axis Label" ) < 0 )
                goto WRITE_GRAPH2D_ERROR;
            }
        else
            {
            if ( fprintf ( tcl_input, "option add *graph.xTitle \"%s\"\n",
                           xaxis_label ) < 0 )
                goto WRITE_GRAPH2D_ERROR;
            }
        if ( yaxis_label == NULL )
            {
            if ( fprintf ( tcl_input, "option add *graph.yTitle \"%s\"\n",
                           "Y Axis Label" ) < 0 )
                goto WRITE_GRAPH2D_ERROR;
            }
        else
            {
            if ( fprintf ( tcl_input, "option add *graph.yTitle \"%s\"\n",
                           yaxis_label ) < 0 )
                goto WRITE_GRAPH2D_ERROR;
            }
        if ( title == NULL )
            {
            if ( fprintf ( tcl_input, "option add *graph.title \"%s\"\n",
                           " " ) < 0 )
                goto WRITE_GRAPH2D_ERROR;
            }
        else
            {
            if ( fprintf ( tcl_input, "option add *graph.title \"%s\"\n",
                           title ) < 0 )
                goto WRITE_GRAPH2D_ERROR;
            }

        k = 0;
        for ( i = 0; i < nlines; i++ )
            {
            lw = lineWidth;
            if ( !strcmp ( legend[i],"Model" ) ) lw=2;
            if ( fprintf ( tcl_input, "set X%d { ", i+1 ) < 0 )
                goto WRITE_GRAPH2D_ERROR;
            for ( j = 0; j < npoints[i]; j++ )
                {
                if ( !isnanf ( y[k+j] ) )
                    {
                    if ( fprintf ( tcl_input, "%g ", x[k+j] ) < 0 )
                        goto WRITE_GRAPH2D_ERROR;
                    }
                }
            if ( fprintf ( tcl_input, "}\n" ) < 0 )
                goto WRITE_GRAPH2D_ERROR;

            if ( fprintf ( tcl_input, "set Y%d { ", i+1 ) < 0 )
                goto WRITE_GRAPH2D_ERROR;
            for ( j = 0; j < npoints[i]; j++ )
                {
                /* ALT NaN pseudo fix */
                if ( isnanf ( y[k+j] ) )
                    {
                    lw = 0;
                    }
                else
                    {
                    if ( fprintf ( tcl_input, "%g ", y[k+j] ) < 0 )
                        goto WRITE_GRAPH2D_ERROR;
                    }
                }
            if ( fprintf ( tcl_input, "}\n" ) < 0 )
                goto WRITE_GRAPH2D_ERROR;

            if ( nlines > 1 ) /* show the label for each line type */
                {
                if ( fprintf ( tcl_input,
                               "$graph element create \"line%d\" -xdata $X%d -ydata "
                               "$Y%d -symbol %s -bg black -foreground %s -linewidth "
                               "%d -label \"%s\"\n",
                               i+1, i+1, i+1, symbol[i], color[i], lw,
                               legend[i] ) < 0 )
                    goto WRITE_GRAPH2D_ERROR;
                }
            else
                {
                /* don't show the label for each line type, since 1 only */
                if ( fprintf ( tcl_input,
                               "$graph element create \"line%d\" -xdata $X%d -ydata "
                               "$Y%d -symbol %s -bg black -foreground %s -linewidth "
                               "%d -label \" \"\n",
                               i+1, i+1, i+1, symbol[i], color[i], lw ) < 0 )
                    goto WRITE_GRAPH2D_ERROR;
                }
            k += npoints[i];
            }

        /* new addition upon Sarav's request */
        hlineval = getenv ( "ADD2DHLINE" );
        if ( hlineval != NULL )
            {
            value = atof ( hlineval );
            fprintf ( tcl_input, "%s\n",
                      "scan [$graph xaxis limits] \"%s %s\" xmin xmax" );

            fprintf ( tcl_input, "%s\n",
                      "set XXX { $xmin $xmax }" );
            fprintf ( tcl_input, "%s %g %g %s\n",
                      "set YYY { ", value, value, " }" );
            fprintf ( tcl_input, "%s \"%s\"\n",
                      "$graph element create \"lineH\" -xdata $XXX -ydata $YYY -bg black -foreground red -linewidth 1 -label ", hlineval );
            }

        }
    else
        goto CREATE_GRAPH2D_ERROR;

    fflush ( tcl_input );
    if ( fclose ( tcl_input ) != 0 )
        goto CLOSE_GRAPH2D_ERROR;

    /*
      if (system(graph2d_cmd) == 0)
        status = PAVE_SUCCESS;
        */
    putenv ( "BLT_GRAPH_WINDOW_ID=0" ); /* reset to default non-valid window ID */
    if ( ( cmdpipe = popen ( graph2d_cmd, "r" ) ) != NULL )
        {

        int fd;
        fd = fileno ( cmdpipe );
        fcntl ( fd, F_SETFL, O_NONBLOCK );

        i = 5; /* make five attemtps to receive BLT_GRAPH_WINDOW_ID */
        while ( i>0 )
            {
            if ( fgets ( response, BUFSIZE, cmdpipe ) != NULL )
                {
                if ( !strncmp ( response,"BLT_GRAPH_WINDOW_ID=",20 ) )
                    {
                    putenv ( response );
                    }
                break;
                }
            else
                {
                i--;
                sleep ( 1 );
                }
            }
        pclose ( cmdpipe );
        status = PAVE_SUCCESS;
        }
    return ( status );

CREATE_GRAPH2D_ERROR:
    sprintf ( message, "Error creating file: %s\n", tmpfilename );
    return ( status );

WRITE_GRAPH2D_ERROR:
    sprintf ( message, "Error writing to file: %s\n", tmpfilename );
    fclose ( tcl_input );
    return ( status );

CLOSE_GRAPH2D_ERROR:
    sprintf ( message, "Error closing file: %s\n", tmpfilename );
    return ( status );
    }

