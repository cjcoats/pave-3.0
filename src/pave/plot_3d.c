/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: plot_3d.c 83 2018-03-12 19:24:33Z coats $
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
 *      Author:  Kathy Pearson, MCNC,  January 6, 1995
 *      950703 SRT Fixed problem indexing into a subdomained grid's data
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ****************************************************************************/

#include <stdio.h>      /* for printf, fprintf */
#include <string.h>     /* for strlen, strcpy */
#include <stdlib.h>     /* for getenv */
#include <sys/types.h>  /* for fork */
#include <unistd.h>     /* for fork */
#include <signal.h>     /* for kill */
#include "vis_data.h"   /* for VIS_DATA structure */
#include "vis_proto.h"  /* for VIS prototypes */


#define PAVE_SUCCESS 1
#define FAILURE 0

int plplot3d ( int input_color, int ncol, int nrow,
               int which_graph, int ngraph, float altitude, float azimuth,
               float char_height, float *ztable, float zmin, float zmax,
               char **titles, int ntitle );

void plstart_3d ( int xlen, int ylen, int xoff, int yoff, char *driver,
                  char *psfilename );
void plstop_3d ( char *driver );


void get_peak   ( float *grid, int ncol, int nrow, int *ix, int *iy, float *fmax );
void get_valley ( float *grid, int ncol, int nrow, int *ix, int *iy, float *fmin );

void julian2text ( char *string, int date, int time );

int plot_3d ( VIS_DATA info, char *user_title1, char *user_title2, char *message )
    {
#ifndef NO_PLPLOT
    register int i, j, k;
    int   ix, iy, child_pid;
    float peak;
    float valley;

    static char title1[512];
    static char title2[512];
    static char title3[512];
    static char title4[512];
    static char title5[512];

    char *titles[] = {title1, title2, title3, title4, title5};

    int ntitle = 5;

    static char peak_str[64];
    static char valley_str[64];

#define CORAL 0
#define RED 1
#define YELLOW 2
#define GREEN 3
#define AQUAMARINE 4
#define PINK 5
#define WHEAT 6
#define GREY 7
#define BROWN 8
#define BLUE 9
#define BLUE_VIOLET 10
#define CYAN 11
#define TURQUOISE 12
#define MAGENTA 13
#define SALMON 14
#define WHITE 15
#define BLACK 16

#define NX (info.col_max - info.col_min + 1)
#define NY (info.row_max - info.row_min + 1)
#define stepsize ((NX) * (NY))

    int xlen = 475;
    int ylen = 475;
    int xoff = 0;
    int yoff = 0;

    int input_color = YELLOW;
    int which_graph = 1;
    int ngraph = 1;
    static float altitude = 30.;
    static float azimuth  = 45.;

    int which_step = 1;

    float char_height = .75;

    static char cmd[512], wname[512] ;
    static char string[512];
    static char psfilename[256];
    static char format[32] = {'%', '.', '4', 'f', '\0', };
    
    char    * aname ;
    char    * dname ;

    static int   user_scale = 1;
    static float user_min   = 0.0;
    static float user_max   = 1.0;

    float zmin;
    float zmax;
    FILE *f;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter plot_3d() with user_title1='%s' and user_title2='%s'\n",
              user_title1, user_title2 );
#endif /* DIAGNOSTICS */

    if ( user_title1 != NULL )
        titles[0] = user_title1;
    else
        titles[0] = title1;
    if ( user_title2 != NULL )
        titles[1] = user_title2;
    else
        titles[1] = title2;

    aname = getenv( "WISH" ) ;
    dname = getenv( "PAVE_DIR" ) ;

    if ( aname && *aname )
        {
        sprintf( wname, "%s", aname ) ;
        }
    else{
        aname = getenv( "PAVE_BINDIR" ) ;
        if ( aname && *aname )
            {
            sprintf( wname, "%s/wish", aname ) ;
            }
        else if ( dname && *dname )
            {
            sprintf( wname, "%s/bin/wish", dname ) ;
            }
        else{
            sprintf( wname, "wish" ) ;
            }
        }        

    sprintf ( cmd, "%s -f %s/bin/tksurfer %d %d %s %d %g %g",
              wname, dname,
              info.step_min, info.step_max, format,
              user_scale, info.grid_min, info.grid_max );


    if ( ( child_pid = fork() ) == 0 ) /* then I am the child process */
        {
        if ( ( f = popen ( cmd, "r" ) ) != NULL )
            {
            plstart_3d ( xlen, ylen, xoff, yoff, "xwin", ( char * ) NULL );

            if ( !info.filename ) /* ADDED SRT */
                sprintf ( title1, "Dummy Title" ); /* ADDED SRT */
            else /* ADDED SRT */
                if ( strrchr ( info.filename, ( int ) '/' ) != NULL )
                    sprintf ( title1, "%s",
                              strrchr ( info.filename, ( int ) '/' ) + 1 );
                else
                    sprintf ( title1, "%s", info.filename );

            sprintf ( title2, "%s(%s)",
                      info.species_short_name[info.selected_species-1],
                      info.units_name[info.selected_species-1] );

            while ( fgets ( string, 132, f ) != NULL )
                {
                if ( strncmp ( string, "quit", 4 ) )
                    {
                    if ( !strncmp ( string, "psc", 3 ) )
                        {
                        sscanf ( string+4, "%s", psfilename );
                        plstart_3d ( xlen, ylen,
                                     xoff, yoff, "psc", psfilename );
                        }
                    else if ( !strncmp ( string, "ps", 2 ) )
                        {
                        sscanf ( string+3, "%s", psfilename );
                        plstart_3d ( xlen, ylen,
                                     xoff, yoff, "ps", psfilename );
                        }
                    else
                        {
                        sscanf ( string, "%d%g%g%s%d%g%g",
                                 &which_step,
                                 &altitude, &azimuth, format,
                                 &user_scale, &user_min, &user_max );
                        }

                    get_peak ( info.grid +
                               ( which_step - info.step_min ) * stepsize,
                               NX, /* SRT 950703 info.ncol, */
                               NY, /* SRT 950703 info.nrow, */
                               &ix, &iy, &peak );

                    sprintf ( peak_str, format, peak );
                    sprintf ( title3,
                              "Peak @ (%d,%d) = %s", /* "Peak = %s", SRT 950911 */
                              ix, iy, /* SRT 950911 */
                              peak_str );

                    get_valley ( info.grid +
                                 ( which_step - info.step_min ) * stepsize,
                                 NX, /* info.ncol, SRT 950721 */
                                 NY, /* info.nrow, SRT 950721 */
                                 &ix, &iy, &valley );

                    sprintf ( valley_str, format, valley );
                    sprintf ( title4,
                              "Valley @ (%d,%d) = %s", /* "Valley = %s", SRT 950911 */
                              ix, iy, /* SRT 950911 */
                              valley_str );

                    if ( user_scale )
                        {
                        zmin = user_min;
                        zmax = user_max;
                        }
                    else
                        {
                        zmin = valley;
                        zmax = peak;
                        }

                    if ( ( info.sdate[which_step-1] == 0 ) && /* SRT 950911 */
                         ( info.stime[which_step-1] == 0 ) )
                        sprintf ( title5, "Step %d", which_step );
                    else
                        julian2text ( title5,
                                      info.sdate[which_step-1],
                                      info.stime[which_step-1] );

                    if ( !plplot3d (  input_color,
                                      NX, /* info.ncol, SRT 950703 */
                                      NY, /* info.nrow, SRT 950703 */
                                      which_graph, ngraph, altitude, azimuth, char_height,
                                      info.grid + ( which_step - info.step_min ) * stepsize,
                                      zmin, zmax, titles, ntitle ) )
                        {
                        sprintf ( message, "ERROR in plplot3d\n" );
                        return ( FAILURE );
                        }
                    }
                if ( !strncmp ( string, "psc", 3 ) )
                    plstop_3d ( "psc" );
                else if ( !strncmp ( string, "ps", 2 ) )
                    plstop_3d ( "ps" );
                }
            pclose ( f );
            plstop_3d ( "xwin" );
            }
        else
            {
            fprintf ( stderr, "ERROR opening TCL command pipe\n" );
            exit ( 1 ); /* exit the child process; its work is done */
            }
        exit ( 0 ); /* exit the child process; its work is done */
        }
    else
        {
        /* I am the parent process, return to caller */
        return ( PAVE_SUCCESS );
        }
#endif
    }

#ifndef NO_PLPLOT
void get_valley ( float *grid, int ncol, int nrow, int *ix, int *iy, float *fmin )
    {
    register int i, j, n;

    n = 0;
    *fmin = grid[0];
    *ix = 1;
    *iy = 1;
    for ( j = 1; j <= nrow; ++j )
        {
        for ( i = 1; i <= ncol; ++i, ++n )
            {
            if ( grid[n] < *fmin )
                {
                *fmin = grid[n];
                *ix = i;
                *iy = j;
                }
            }
        }
    }

void get_peak ( float *grid, int ncol, int nrow, int *ix, int *iy, float *fmax )
    {
    register int i, j, n;

    n = 0;
    *fmax = grid[0];
    *ix = 1;
    *iy = 1;
    for ( j = 1; j <= nrow; ++j )
        {
        for ( i = 1; i <= ncol; ++i, ++n )
            {
            if ( grid[n] > *fmax )
                {
                *fmax = grid[n];
                *ix = i;
                *iy = j;
                }
            }
        }
    }

#endif

void julian2text ( char *string, int date, int time )
    {
    if ( ( date == 0 ) && ( time == 0 ) )
        {
        sprintf ( string, "%s", "TIME INDEPENDENT data" ) ;
        }
    else if ( ( date == -635 ) && ( time == 0 ) )
        {
        sprintf ( string, "%s", "TIME INDEPENDENT data" ) ;
        }
    else
        {
        dt2strc ( date, time, string ) ;
        }
    }

