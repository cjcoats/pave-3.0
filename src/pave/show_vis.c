/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: show_vis.c 83 2018-03-12 19:24:33Z coats $
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
 *      Author:      Rajini Balay, NCSU, February 25, 1995
 *      Version 2/2018 by Carlie J. Coats, Jr. for PAVE-3.0
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>

#include "netcdf.h"
#include "vis_data.h"
#include "utils.h"


/* This function initializes the VIS_DATA structure fields to zeros and NULLs */
void init_vis ( VIS_DATA *info )
    {
    memset ( ( void * ) info, 0, sizeof ( VIS_DATA ) ); /* SRT 960716
   info->nspecies = 0;
   info->species_long_name = NULL;
   info->species_short_name = NULL;
   info->units_name = NULL;
   info->grid = NULL;
   info->sdate = NULL;
   info->stime = NULL;
   info->map_info = NULL;
   info->data_label = NULL;
   info->slice = NONESLICE;

   info->dataset = UAM_DATA;
   info->first_date = 0;
   info->first_time = 0;
   info->last_date = 0;
   info->last_time = 0;
   info->incr_sec = 0;
   info->ncol = 0;
   info->nrow = 0;
   info->nlevel = 0;
   info->nstep = 0;

   info->col_min = 0;
   info->col_max = 0;
   info->row_min = 0;
   info->row_max = 0;
   info->level_min = info->level_max = 0;
   info->step_min = info->step_max = 0;
   info->step_incr =0;

   info->selected_species = info->selected_col = info->selected_row = 0;
   info->selected_level = info->selected_step = 0;

   info->grid_min = info->grid_max = 0.0;
*/
    }

/* This function prints the Grid information of the VIS_DATA structure */
void print_vis_grid ( VIS_DATA *info, int n )
    {
    printf ( "***************************************************\n" );
    printf ( "               Grid information \n" );
    printf ( "***************************************************\n" );
    printf ( "n = %d\n", n );
    printf ( "data[%d] = %g\n", 0, info->grid[0] );
    printf ( "data[%d] = %g\n", n-1, info->grid[n-1] );
    printf ( "range = %g:%g\n", info->grid_min, info->grid_max );
    printf ( "Julian Date:     %d\n", info->sdate[0] );
    printf ( "Julian Time:     %d\n", info->stime[0] );
    printf ( "***************************************************\n" );
    }

/* This function prints the remaining fields of the VIS_DATA structure */
void print_vis_data ( VIS_DATA *info )
    {
    dump_VIS_DATA ( info, NULL, NULL ); /* SRT 960716
    int i;

    printf("-----------------------------------------------------\n");
    printf("                   VIS_DATA \n");
    printf(" Filename = %s \n", info->filename);

    printf(" dataset = %d \n", info->dataset);
    printf(" npecies = %d \n", info->nspecies);
    for (i=0; i<info->nspecies; i++) {
        printf("    %d : Short name = %s Long name = %s Units name = %s \n", i,info->species_short_name[i], info->species_long_name[i], info->units_name[i]);
    }
    if (info->map_info != NULL)
        printf(" Map infor = %s \n", info->map_info);
    if (info->data_label != NULL)
        printf(" Data label = %s \n", info->data_label);
    printf(" First date = %d  First time = %d \n", info->first_date, info->first_time);
    printf(" Last date = %d Last time = %d \n", info->last_date, info->last_time);
    printf(" Incr sec = %d \n", info->incr_sec);
    printf(" ncol = %d nrow = %d nlevel = %d nstep = %d \n", info->ncol, info->nrow, info->nlevel, info->nstep);
    printf(" col min = %d col max = %d \n", info->col_min, info->col_max);
    printf(" row min = %d row max = %d \n", info->row_min, info->row_max);
    printf(" level min = %d level max = %d \n", info->level_min, info->level_max);
    printf(" step min = %d step max = %d step incr = %d \n", info->step_min, info->step_max, info->step_incr);
    printf(" Slice = %d \n", info->slice);
    printf(" selected species = %d \n", info->selected_species);
    printf(" selected col = %d selected row = %d \n", info->selected_col, info->selected_row);
    printf(" selected level = %d selected step = %d \n", info->selected_level, info->selected_step);
    printf(" Grid max = %f grid min = %f \n", info->grid_max, info->grid_min);

    if (info->sdate != NULL) {
       int numdates;

       numdates = ((info->step_max - info->step_min)/info->step_incr)+1;
       for (i=0; i<numdates; i++)
          printf("%d ", info->sdate[i]);
       printf("\n");
    }
    printf("-----------------------------------------------------\n");
*/
    }

void set_file_ip ( VIS_DATA *info )
    {
    struct hostent *h_info;
    struct in_addr *hptr;

    h_info = ( struct hostent * ) gethostbyname ( info->filehost.name );

    if ( h_info == NULL ) return; /* Illegal hostname */

    hptr = ( struct in_addr * ) *h_info->h_addr_list;
    if ( ( info->filehost.ip = ( ( char * ) malloc ( sizeof ( char ) * 256 ) ) ) == NULL )
        {
        fprintf ( stderr, "Could not allocate memory for file IP address in data structure\n" );
        exit ( EXIT_FAILURE );
        }

    sprintf ( info->filehost.ip,"%s", inet_ntoa ( *hptr ) );

    }
