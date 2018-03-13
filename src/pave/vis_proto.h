#ifndef ___VISPROTO_H___
#define ___VISPROTO_H___

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)vis_proto.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.vis_proto.h
 * Last updated: 12/15/97 16:29:19
 *
 * Made available by MCNC and the Carolina Environmental Program of UNC Chapel
 * Hill under terms of the GNU Public License.  See gpl.txt for more details.
 *
 * See file COPYRIGHT for license information on this and supporting software.
 *
 * Carolina Environmental Program
 * University of North Carolina at Chapel Hill
 * 137 E. Franklin St.
 * Chapel Hill, NC 27599-6116
 *
 * See http://www.cep.unc.edu, http://www.cmascenter.org, http://bugz.unc.edu
 *
 ****************************************************************************/


/*****************************************************************************/
/* Author:	Kathy Pearson, MCNC, kathyp@mcnc.org, (919) 248-9240         */
/* Date:	December 20, 1994                                            */
/*****************************************************************************/
/* Prototypes for VIS data reading interface routines                        */
/*****************************************************************************/
/*
MODIFICATION HISTORY:

WHO  WHEN       WHAT
---  ----       ----
SRT  03/20/95   Added plot_3d() prototype
SRT  04/06/95   Added #ifdef __cplusplus lines, added a few prototypes
SRT  09/18/95   Added lineWidth param to graph2d() prototype
*/

        /* unistd.h messes up compilation sometimes because of "link" */
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif


        /* in order to get the linker to resolve Kathy's 
           subroutines when using CC to compile */
#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */


#include "vis_data.h"


extern void free_vis(VIS_DATA *info);

extern void init_vis(VIS_DATA *info);

extern void set_file_ip(VIS_DATA *info);

extern void print_vis_grid(VIS_DATA *info, int n );

extern void print_vis_data(VIS_DATA *info);

extern int map_overlay(VIS_DATA info, float *xpts, float *ypts, int *n, 
	int *npolyline, int maxpoints, int map_options, char *message);

extern int graph2d(float *x, float *y, int nlines, int *npoints,
	char *title, char *xaxis_label, char *yaxis_label, 
	char **legend, char **symbol, char **color, char *message,
	int lineWidth /* added 950918 SRT */);

extern int plot_3d(VIS_DATA info, char *user_title1, char *user_title2, char *message);

extern void julian2text(char *string, int date, int time);

extern int map(float *x, float *y, int nlines, int *npoints,
char *title, char *xaxis_label, char *yaxis_label, char *message);

extern int clip( float *fx1, float *fy1, float *fx2, float *fy2,
                 float xmin, float ymin, float xmax, float ymax);

        /* in order to get the linker to resolve Kathy's
           subroutines when using CC to compile */
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif	/* ___VISPROTO_H___ */
