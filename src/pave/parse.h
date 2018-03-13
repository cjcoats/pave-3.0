#ifndef ___PARSE___
#define ___PARSE___
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
 * Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.parse.h
 * Last updated: 05/28/98 11:04:56
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

FILE:           parse.h
 
AUTHOR:         Steve Thorpe
                MCNC Environmental Programs
                thorpe@ncsc.org
 
DATE:           12/9/94
 
PURPOSE:        Header file for parse.c code, which translates
		an Environmental Decision Support System (EDSS) 
		formula string from infix to postfix notation.

************************************************************/

/*
static char parsehSid[] = "@(#)parse.h	2.2 /env/proj/archive/edss/src/pave/pave_include/SCCS/s.parse.h 05/28/98 11:04:56";
*/

/* unistd.h messes up compilation sometimes because of "link" */
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif


extern int parseFormula

                (

                /* INPUTS TO parseFormula */

                char *formulaP,       /* infix formula typed in by user;
                                         NOTE:  this is possibly
                                         "prettied up" to look better
                                         on graphs and in dialog boxes */

                char *caseListP,      /* a list of cases (ie data file
                                         names) in order (a..?),
                                         separated by commas */

                char *hostListP,      /* a list of hosts (ie
                                         "todd.hpcc.epa.gov,ozone,
                                         flyer.ncsc.org") separated by
                                         commas, one for each case in
                                         caseList */

                struct BusData *bdP,  /* needed to communicate with
                                         the SW Bus; this should already
                                         have been initialized with
                                         initVisDataClient() */


                /* MODIFIED BY parseFormula */

                char *errString,      /* errormsg if any */
                char *postFixQueueP,  /* postfix formula result */
                char *caseUsedP,      /* "010" if ncases = 3 and only
                                         case b in formula */
                char *whichUnitP,     /* the units of the formula's output */

                int  *dimP,           /* ndim of formula result's data */
                int  *dateDayP,       /* the day of formula result */
                int  *dateMonthP,     /* the month of formula result */
                int  *dateYearP,      /* the year of formula result */
                int  *hourStartP,     /* starting hour */
                int  *mixCaseP,       /* 1 if can't put a time on the
                                         starting hour, otherwise 0 */
                int  *IMAX,           /* IMAX for formula */
                int  *JMAX,           /* JMAX for formula */
                int  *KMAX            /* KMAX for formula */

                );

#endif
