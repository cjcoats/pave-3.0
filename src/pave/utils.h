#ifndef ___UTILS___
#define ___UTILS___

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 * Carolina Environmental Program
 * University of North Carolina at Chapel Hill
 * 137 E. Franklin St.
 * Chapel Hill, NC 27599-6116
 *
 * See http://www.cep.unc.edu, http://www.cmascenter.org, http://bugz.unc.edu
 *
 * Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.utils.h
 * Last updated: 07/02/98 10:32:07
 *
 ****************************************************************************
 *
 * Made available by MCNC and the Carolina Environmental Program of UNC Chapel
 * Hill under terms of the GNU Public License.  See gpl.txt for more details.
 *
 * See file COPYRIGHT for license information on this and supporting software.
 *
 ****************************************************************************
 *                                               
 *                           C R E D I T S       
 *                                               
 *   Development of this Software was supported in part through the 
 *   MCNC-EPA cooperative agreement number CR822066-01.
 *                                                                  
 *   Portions of this software were developed by the State University
 *   of New York at Albany, and funded by the United States Environmental
 *   Protection Agency under Contract 68D80016 with the Research Foundation
 *   of the State University of New York.
 *
 ****************************************************************************/

/************************************************************

FILE:           utils.h
 
AUTHOR:         Steve Thorpe
                MCNC Environmental Programs
                thorpe@ncsc.org
 
DATE:           12/12/94
 
PURPOSE:        Header file for utils.c, miscellaneous 
		routines which will be used by EDSS 
		Analysis and Visualization code.

************************************************************/

/***********************************************************
Modification History
950908 SRT added removeWhiteSpace() routine
951218 SRT added registerCurrentTime() and verifyElapsedClockTime() routines
960515 SRT added M3IO_parameter_fix()
960517 SRT added dump_VIS_DATA_to_netCDF_file()
961021 SRT added makeSureIts_netCDF()
961021 SRT added map_infos_areReasonablyEquivalent()

************************************************************/


/*
static char utilshSid[] = "$Id: utils.h 83 2018-03-12 19:24:33Z coats $";
*/

        /* unistd.h messes up compilation sometimes because of "link" */
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include "vis_data.h"
#include "readuam.h"

		/* functions declared in utils.c */

extern int 	cancelKeys(void);

extern void 	itoa(int, char *);

extern void 	ftoa(double, char *);

extern int 	errmsg(char *);

extern int     	getNthItem(int, char *, char *);

extern char 	*getLocalHostName(void);

extern void	diagmsg(char *);

extern int 	range_get(  
		        char *percents, /* array to denote which cells in domain */
                int  IMAX,    
                int  JMAX,   
                int *imin,   /* minimum column w/ percents on */
                int *imax,   /* maximum column w/ percents on */
                int *jmin,   /* minimum row w/ percents on */
                int *jmax);   /* maximum row w/ percents on */

extern
int calc_stats  (
                        /* INPUTS to calc_stats */

                VIS_DATA *vdata,

                char percents[],    /* This is an array indicating which
                                       cells are in the domain and which
                                       are turned off.  Where 
                                       IMAX=vdata->col_max-vdata->col_min+1,
                                       JMAX=vdata->row_max-vdata->row_min+1,
                                       KMAX=vdata->level_max-vdata->level_min+1,
                                       percents is indexed
                                       [i+j*IMAX] (where i ranges 0..IMAX-1
                                       and j ranges 0..JMAX-1).  For a
                                       grid cell completely in the domain,
                                       the corresponding percent's value
                                       should be 100, for a cell completely
                                       NOT in the domain, it should be 0.
                                       For a cell which should only count
                                       partially, use a value between 0
                                       and 100 */
 
                int layers[],       /* vdata->levelMax-vdata->levelMin+1
                                       items telling whether to use each
                                       layer.  0 means don't use it,
                                       non-zero means use it */
 

                int step,             /* Note:  step is *0* based

                                         if (step >= 0) then only that
                                         time step is computed for;

                                         if (step < 0) then stats
                                         are calculated for all time steps
                                         that are there */


                int TMAX,             /* the number of time steps for which
                                         data exists within vdata */

                        /* OUTPUT by calc_stats */

                int *maxi, /* 1 based */
                int *maxj, /* 1 based */
                int *maxk, /* 1 based */
                int *maxt, /* 1 based */
                int *mini, /* 1 based */
                int *minj, /* 1 based */
                int *mink, /* 1 based */
                int *mint, /* 1 based */
                float *min,
                float *max,
                float *mean,
                float *var,
                float *std_dev,
                float *sum
                );


extern int dump_VIS_DATA

		 (VIS_DATA *vdata,
 
                  char *percents,   /* This is an array indicating which
                                       cells are in the domain and which
                                       are turned off.  Where IMAX==
                                       vdata->ncol and JMAX==vdata->nrow,
                                       percents is indexed
                                       [i+j*IMAX] (where i ranges 0..IMAX-1
                                       and j ranges 0..JMAX-1).  For a
                                       grid cell completely in the domain,
                                       the corresponding percent's value
                                       should be 100, for a cell completely
                                       NOT in the domain, it should be 0.
                                       For a cell which should only count
                                       partially, use a value between 0
                                       and 100 */
 
                  int *layers);     /* vdata->levelMax-vdata->levelMin+1
                                       items telling whether to use each
                                       layer.  0 means don't use it,
                                       non-zero means use it
                                       NOTE - THIS IS 0 BASED */


extern void     myFreeVis (VIS_DATA *info);

extern void     M3IO_parameter_fix (VIS_DATA *info);

extern VIS_DATA *VIS_DATA_dup(VIS_DATA *info, char *estring);

extern int strip_VIS_DATA_operator_chars(VIS_DATA *info, char *estring);

extern int dump_VIS_DATA_to_AVS_file(VIS_DATA *vdata, char *fname, char *estring);

extern int dump_VIS_DATA_to_tabbed_ascii_file(VIS_DATA *vdata, char *fname, char *estring);

extern int dump_VIS_DATA_to_netCDF_file(VIS_DATA *vdata, char *fname, char *estring);

extern int 	integral(double d);

extern int 	removeWhiteSpace(char *s);

extern int 	parseLongDataSetName
			(char *lname,   /* full name to be parsed */
                         char *hname,   /* extracted host name goes here */
                         char *pname,   /* extracted path name goes here */
                         char *fname);  /* extracted fname name goes here */

extern char 	*getPointerToBaseName(char *fullname); /* full name */

extern float 	ieee_nan(void);

extern float    setNaNf();

extern void 	registerCurrentTime(void);

extern void 	verifyElapsedClockTime(float secsSinceRegistered);

extern char 	*get_vdata_SelectedCellRange(VIS_DATA *vdata); 	/* SRT 961014 */

extern char 	*get_vdata_TimeMinString(VIS_DATA *vdata);	/* SRT 961014 */

extern char 	*get_vdata_TimeMaxString(VIS_DATA *vdata);	/* SRT 961014 */

extern void julian2shorttext(char *string, int date, int time); /* 101496 added SRT */

extern int 	makeSureIts_netCDF(VIS_DATA *vdata, char *estring); /* 102196 added SRT */

extern int map_infos_areReasonablyEquivalent(char *p, char *q, char *estring); /* 102296 added SRT */


		/* global variables declared in utils.c */

extern char     *errorString;

#ifdef MDIAGS

extern void *thorpe_malloc (size_t size);
extern void thorpe_free (void *ptr);

#define malloc(x) thorpe_malloc(x)
#define free(x)   thorpe_free(x)

#endif /* #ifdef MDIAGS */


#endif
