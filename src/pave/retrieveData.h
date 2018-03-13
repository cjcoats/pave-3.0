#ifndef ___RETRIEVEDATA___
#define ___RETRIEVEDATA___
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
 * Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.retrieveData.h
 * Last updated: 05/28/98 11:04:58
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

FILE:           retrieveData.h
 
AUTHOR:         Steve Thorpe
                MCNC Environmental Programs
                thorpe@ncsc.org
 
DATE:           12/22/94
 
PURPOSE:        Header file for retrieveData.c code, which translates
		an Environmental Decision Support System (EDSS) 
		postfix formula string into actual data.

************************************************************/


/* 
	for SCCS purposes
	
static char retrieveDatahSid[] = "@(#)retrieveData.h	2.3 /env/proj/archive/edss/src/pave/pave_include/SCCS/s.retrieveData.h 05/28/98 11:04:58";

*/

/**
#define USE_OLDMAP "uncomment this if you want to use the old mapping scheme" 
**/

	/* unistd.h messes up compilation sometimes because of "link" */
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

	/*	prototypes for routines in retrieveData.c 
		that are accessible to the outside world 	*/


                /* 
                   NOTES ON TIME STEP ARGUMENTS TO retrieveData():

                         step_min[], step_max[], & step_incr[] are 
                         *1* based, and are specific to each 
                         *dataset* == *case* (NOT a formula).
                         Typically these would be set to the maximum
                         range for each case, although for some
                         situations the user may wish to clamp the
                         range down somewhat by adjusting these.

                         selectedStepP is *1* based, and is specific
                         to this *formula* (which may have multiple 
                         cases within it). It is used as an offset to 
                         the step_min for each case in a formula, and 
                         matters only for slice_type's XYSLICE, YZSLICE, XZSLICE, 
                         and XYZSLICE with integration NOT TIME_INT or ZvsT_INT.

                         hrMin & hrMax are *0* based, and
                         are specific to this *formula* (which may
                         have multiple cases within it).  hrMin == 0
                         refers to time_step step_min[<that case>],
                         hrMin == 1 to time_step step_min[<that case>]+1,
                         and so on.

                         for each species within a formula,
                         hrMin & hrMax are used as clamps within
                         that case's step_min & step_max 

                         hrMin & hrMax are used for slice_types
                         XYTSLICE, YZTSLICE, XZTSLICE, and XYZTSLICE and for integration
                         values of TIME_INT or ZvsT_INT.

                         Typically hrMin & hrMax could be set up this way
                         (although you may want to allow the user to
                          "clamp down" the range calculated by this algorithm):

                         Assuming you have n cases used by a formula 
                         [the cases used by a formula can be determined 
                         by the caseUsed argument which is filled up by 
                         parseFormula()].  A good choice for hrMin and hrMax 
                         would be to allow the maximum time step range 
                         possible.  For each case(i)  used in a formula
                         check out the step_min & step_max for that case, and
                         determine nsteps(i)= step_max-step_min+1 for that case.
                         Now determine nsteps(small) = the smallest of 
                         the nsteps(i).  Set hrMin = 0, and
                         hrMax = nsteps(small)-1
                */

extern int	retrieveData	(

                	/* INPUTS to retrieveData */

                int  I_MAX,           /* imax for formula returned
                                         by parseFormula() */

                int  J_MAX,           /* jmax for formula returned
                                         by parseFormula() */

                int  K_MAX,           /* kmax for formula returned
                                         by parseFormula() */

                char *infixFormulaP,  /* infix formula - will be
                                         stuffed into the VIS_DATA
                                         struct returned by 
                                         retrieveData() */

                char *postFixQueueP,  /* postfix version of the formula -
                                         required to retrieve the data;
                                         should be in the form returned
                                         by parseFormula() */

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

		int   selectedStepP,  /* what timestep we are getting
					 data for?

					 NOTES:  this is 1 based !! 

					 This is used as 
					 an offset to the step_min 
					 for each case.

					 This matters only for slice_type's
					 XYSLICE, YZSLICE, XZSLICE, and XYZSLICE */

		int   integration,    /* should be one of the following:
                                         NO_INT, ALL_INT, TIME_INT, or 
					 ZvsT_INT (all are defined
                                         in retrieveData.h) */

                float thickValues[],  /* NOTE:  retrieveData MODIFIES THE
					 CONTENTS OF THICKVALUES in place !

					 this is an array of K_MAX floats
					 indicating the relative weights
                                         to give each of the 0..K_MAX-1
                                         layers when doing an integration
                                         (such as time series).  For
                                         example, if you have a 15 layer
                                         RADM run, you might stuff the
                                         following numbers into thickValues:
                                         (.01,.01,.02,.03,.04,.05,.06,.08,
                                          .1,.1,.1,.1,.1,.1,.1)  

			For formulas with "sigma" in it, thickValues will 
			be used to calculate sigma using the scheme:

        		E = the summation of thickList[i], i = 0 to K_MAX-1 

			sigma[K_MAX-1] = 0.5 * thickValues[K_MAX-1]/E

        	        sigma[X] = sigma[X+1] + 0.5 * 
				   (thickValues[X+1] + thickValues[X]) / E */


		int   whichLevelP[],  /* this is an array of K_MAX integers,
                                         each element should be non-zero 
                                         if that level is in the domain 
                                         of interest, otherwise 0 

					 NOTE: RETRIEVE DATA MODIFIES
					 THE WHICHLEVELP IN PLACE !!!! */

		int   use_floor,      /* if use_floor is non-zero,
					 then divisions will be checked
                                         to avoid divide by zero conditions.
                                         if (use_floor) then for each
					 division, if the denonimator is
					 less than or equal to floorCut
					 then the result of the divide is
					 set to 0 */

		float floorCut,	      /* see use_floor description above */

		char  percentsP[],    /* this is an array indicating which
					 cells are in the domain and which
					 are turned off.  It is indexed
					 [i+j*I_MAX] (where i ranges 0..I_MAX-1,
					 and j ranges 0..J_MAX-1).  For a
					 grid cell completely in the domain,
					 the corresponding percent's value
					 should be 100, for a cell completely
					 NOT in the domain, it should be 0.
					 For a cell which should only count
					 partially, use a value between 0
					 and 100 */

		int   selected_col,   /* which column selected, 1-based */

		int   selected_row,   /* which row selected, 1-based */

		int   selected_level, /* which level selected, 1-based */

		int   step_min[],     /* one step_min for each dataset 
					 listed in caseListP argument, 
					 1-based (these will probably be 
					 passed as 1's most of the time);

					 NOTES: (selectedStep == 1) corresponds
					 to a given dataset's step_min;
					 (hrMin == 0) also corresponds
					 to a given dataset's step_min 

					 This will probably be set in
					 the UI to "clamp" datasets down
					 to time periods of interest */

		int   step_max[],     /* one step_max for each dataset 
					 listed in caseListP argument, 
					 1-based (most of the time these 
					 will probably be passed as the 
					 max number of steps in each 
					 dataset) 

					 NOTE: This will probably be set in
                                         the UI to "clamp" datasets down
                                         to time periods of interest */

		int   step_incr[],    /* one step_incr for each dataset 
					 listed in caseListP argument, 
					 1-based (most of the time these 
					 will probably be passed as 1's) 

                                         NOTE: This will probably be set in
                                         the UI to "clamp" datasets down
                                         to time increments of interest,
					 for example, to synchronize two
					 datasets in a formula whose
					 step increments differ */

		int  slice_type,      /* should be one of:

        				 XYSLICE - selected_level at step hourf

        				 YZSLICE - selected_column at step hourf

        				 XZSLICE - selected_row at step hourf

        				 XYZSLICE - all levels at step hourf

        				 XYTSLICE - selected_level for steps 
					       *hrMinP thru *hrMaxP

        				 YZTSLICE - selected_col for steps 
					       *hrMinP thru *hrMaxP

        				 XZTSLICE - selected_row for steps 
					       *hrMinP thru *hrMaxP

        				 XYZTSLICE - all levels for steps 
					       *hrMinP thru *hrMaxP

					 NOTE: these are defined in vis_data.h
				      */



			/* INPUTS to retrieveData, but possibly 
			   modified by retrieveData */

                int   *hrMinP,        /* first timestep (if time series data);
					 this is used as an offset to the
					 step_min for each case.  So in 
					 effect, step_min[] and step_max[]
					 clamp a dataset down once, then
					 *hrMinP and *hrMaxP clamp it down
					 again, *within* the step_min-
					 step_max clamp.

                                         NOTES : this is *0* based and
					 and only matters for slice_type's
                                         XYTSLICE, YZTSLICE, XZTSLICE, and XYZTSLICE */ 

                int   *hrMaxP,        /* last timestep (if time series data);
                                         this may be modified to the
                                         highest available hour if data
                                         files don't go up this high. So in 
                                         effect, step_min[] and step_max[]
                                         clamp a dataset down once, then
                                         *hrMinP and *hrMaxP clamp it down
                                         again, *within* the step_min-
                                         step_max clamp.

                                         NOTES : this is *0* based and
                                         and only matters for slice_type's
                                         XYTSLICE, YZTSLICE, XZTSLICE, and XYZTSLICE */

                	/* MODIFIED BY retrieveData, in addition to
			   whichLevelP (see above) */

		float  *tsdata,       /* if doing time series or ZvsT
					 integration (integration == 
					 TIME_INT or ZvsT_INT) then the 
					 resulting data will be stuffed here.  
					 There should be enough space already
					 allocated to hold this many floats:

					   *hrMaxP - *hrMinP + 1 
						(for time series)

					 or

					    K_MAX * (*hrMaxP - *hrMinP + 1)
						(for ZvsT)

					 if doing scatter integration
					 (integration == SCATTER_INT) then
					 the data will be stuffed here,
					 and tsdata[] should have at least
					 1+I_MAX*J_MAX*K_MAX*(*hrMax - *hrMin+1)
					 points in it.  The format of the 
					 data returned will be tsdata[0] == 
					 npoints, and tsdata[1]...
					 tsdata[npoints] holds the scatter 
					 data.

					 if NOT doing timeSeries, ZvsT, or
					 scatter integration, then this 
					 argument is ignored */

		VIS_DATA *vdata,      /* resulting data will be placed
					 here; this code will allocate
					 the necessary space for the
					 dynamically allocated values
					 pointed to by elements of
					 VIS_DATA structs - ITS UP
					 TO THE CALLING ROUTINE TO FREE 
					 THESE !!!!! Note, if doing timeSeries, 
					 ZvsT, or total integration 
					 if (integration == ALL_INT), then 
					 this argument is ignored  */

		float *tot_value,     /* if integration == ALL_INT then
					 this will receive the result.
					 If integration != ALL_INT then
					 this argument is ignored */ 

                char *errString       /* space for errormsg, if any */

			);


int my_get_info( struct BusData *bd, VIS_DATA *info, char *message);


	/* 	constants used in retrieveData.c that
		may be useful to the outside world		*/

#define MAXPAVESPECS 2500  	/* changed from 250 980527 SRT
				   if the number of species in an
				   PAVE data file is greater than
				   this, then something HAS to
				   be wrong, right ? */

/* the 6 kinds of integration performed (plus NO_INT) */
#define NO_INT		0	/* no integration performed */
#define IJ_INT          3
#define JK_INT          1
#define IK_INT          2
#define ALL_INT         4	/* computes domain's mean for formula */
#define TIME_INT        5	/* time series */
#define ZvsT_INT        10	/* vertical profile over time */
#define SCATTER_INT     11	/* scatter data, ie 1D array of data
				   with each element stored in the field */


#endif /* ifndef ___RETRIEVEDATA___ */
