/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: plplot3d_sub.c 83 2018-03-12 19:24:33Z coats $
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
 *      Author?  1995?
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ****************************************************************************/

static const char SVN_ID[] = "$Id: plplot3d_sub.c 83 2018-03-12 19:24:33Z coats $";

#include "plplot.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <sys/types.h>      /* for getpid() */
#include <unistd.h>     /* for getpid() */

int plplot3d ( int input_color, int ncol, int nrow,
               int which_graph, int ngraph, float altitude, float azimuth,
               float char_height, float *ztable, float zmin, float zmax,
               char **titles, int ntitle )

    {
#ifndef NO_PLPLOT
    int i, j, k;

    PLFLT *x, *y;
    PLFLT **z;

    PLFLT az;
    PLFLT alt;

    PLFLT ypos;

    int npts;

#ifdef DIAGNOSTICS
    for ( i = 0; i < ntitle; i++ )
        if ( titles[i] != NULL )
            fprintf ( stderr, "plplot3d titles[%d] = '%s' \n", i, titles[i] );
#endif /* DIAGNOSTICS */

    if ( ( x = ( PLFLT * ) malloc ( ncol * sizeof ( PLFLT ) ) ) == NULL )
        return ( 0 );
    if ( ( y = ( PLFLT * ) malloc ( nrow * sizeof ( PLFLT ) ) ) == NULL )
        {
        free ( x );
        return ( 0 );
        }
    if ( ( z = ( PLFLT ** ) malloc ( ncol * sizeof ( PLFLT * ) ) ) == NULL )
        {
        free ( x );
        free ( y );
        return ( 0 );
        }

    for ( i = 0; i < ncol; i++ )
        {
        z[i] = ( PLFLT * ) malloc ( nrow * sizeof ( PLFLT ) );
        x[i] = ( PLFLT ) i;
        }

    for ( i = 0; i < nrow; i++ )
        y[i] = ( PLFLT ) i;

    for ( i = 0; i < ncol; i++ )
        {
        for ( j = 0; j < nrow; j++ )
            {
            z[i][j] = ztable[i + j * ncol];
            }
        }
    az  = ( PLFLT ) ( azimuth );
    alt = ( PLFLT ) ( altitude );


    pladv ( 0 );

    plschr ( 0, char_height );

    plvasp ( 1 ); /* set x-y aspect ratio to 1 */
    plvpor ( ( PLFLT ) 0.05, ( PLFLT ) .95, ( PLFLT ) 0.05, ( PLFLT ) .95 );
    plwind ( ( PLFLT ) -1.0, ( PLFLT ) 1.0, ( PLFLT ) -0.5, ( PLFLT ) 1.5 );
    plcol ( input_color );
    plw3d ( ( PLFLT ) 1.0, ( PLFLT ) 1.0, ( PLFLT ) 1.0, ( PLFLT ) 0.0, ( PLFLT ) ncol,
            ( PLFLT ) 0.0, ( PLFLT ) nrow, ( PLFLT ) zmin, ( PLFLT ) zmax, alt, az );

    plbox3 ( "bnstu", "x axis", ( PLFLT ) 0.0, 0, "bnstu", "y axis", ( PLFLT ) 0.0, 0,
             "bcdmnst", "z axis", ( PLFLT ) 0.0, 0 );
    plwid ( 5 ); /* SRT set the line width to 2*/

    ypos = 1.5;
    if ( ( which_graph == 1 ) && ( ntitle >= 1 ) && ( titles[0] != NULL ) )
        plmtex ( "t", ( PLFLT ) ypos, ( PLFLT ) 0.5, ( PLFLT ) 0.5, titles[0] );
    for ( i = 1; i < ntitle; i++ )
        {
        if ( titles[i] != NULL )
            {
            ypos -= 1.75;
            plmtex ( "t", ( PLFLT ) ypos, ( PLFLT ) 0.5, ( PLFLT ) 0.5, titles[i] );
            }
        }

    /* must use mesh to draw inverted cone correctly */
    plmesh ( x, y, z, ncol, nrow, 3 );

    /* end-of-page to force pixmap double buffer to show up */

    pleop();

    free ( x );
    free ( y );
    free ( *z );

    return ( 1 );
    }

void plstop_3d ( char *driver )
    {
    if ( !strcmp ( driver, "xwin" ) )
        plend();            /* stop master stream */
    else
        {
        plend1();
        plsstrm ( 0 );
        }
    }

void plstart_3d ( int xlen, int ylen, int xoff, int yoff, char *driver, char *outputfile )
    {
    FILE *f;
    char *newfilename = NULL;
    int ps_okay;

    if ( !strcmp ( driver, "xwin" ) )
        plSetInternalOpt ( "db", "" ); /* set xwin double buffering */
    else
        {
        plsstrm ( 1 );      /* switch streams */

        /* Make sure PostScript output file will be writeable by user; if not,
           write warning to stderr and try to create a file that WILL work!
        */

        ps_okay = 0;

        if ( ( f = fopen ( outputfile, "rw" ) ) != NULL )
            {
            fprintf ( stderr, "overwriting file %s\n", outputfile );
            fclose ( f );
            plSetInternalOpt ( "o", outputfile ); /* set PS output file name */
            ps_okay = 1;
            }
        else if ( ( f = fopen ( outputfile, "w" ) ) != NULL )
            {
            fclose ( f );
            plSetInternalOpt ( "o", outputfile ); /* set PS output file name */
            ps_okay = 1;
            }
        else
            {
            if ( ( newfilename = ( ( char * ) malloc ( sizeof ( char ) * 512 ) ) )
                    != NULL )
                {
                fprintf ( stderr, "Specified file name unwriteable!\n" );
                sprintf ( newfilename, "/tmp/mesh_%d.%s",
                          getpid(), driver );
                fprintf ( stderr, "Trying to create file %s!\n", newfilename );
                if ( ( f = fopen ( newfilename, "rw" ) ) != NULL )
                    {
                    fprintf ( stderr,
                              "overwriting file %s\n", newfilename );
                    fclose ( f );
                    plSetInternalOpt ( "o", newfilename );
                    ps_okay = 1;
                    }
                else if ( ( f = fopen ( newfilename, "w" ) ) != NULL )
                    {
                    fclose ( f );
                    plSetInternalOpt ( "o", newfilename );
                    ps_okay = 1;
                    }
                free ( newfilename );
                }
            }
        if ( !ps_okay )
            {

            /* If all this fails, PlPlot will prompt for stdin entry of filename, and
               if it can't create the specified file, it will barf!
            */
            fprintf ( stderr,
                      "Type carefully!\nEntering an erroneous filename here is perilous!\n" );
            }
        }
    plSetInternalOpt ( "bg", "000064" ); /* set background color */
    plSetInternalOpt ( "np", "" );  /* set no pause */
    plspage ( 0, 0, xlen, ylen, xoff, yoff ); /* set window size & position */
    plstart ( driver, 1, 1 );       /* init */
#endif
    }
