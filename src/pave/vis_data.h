#ifndef ___VISDATA_H___
#define ___VISDATA_H___

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)vis_data.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.vis_data.h
 * Last updated: 12/15/97 16:29:16
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
/* Date:	December 1, 1994                                             */
/*****************************************************************************/

/*
MODIFICATION HISTORY:

WHO  WHEN       WHAT
---  ----       ----
SRT  04/06/95   Added #ifdef __cplusplus lines
SRT  05/09/96   Added "SLICE" to end of each slice type, and replaced
		the enum data_slice by int.  This overcomes conflicts
		with Todd Plessel's DrawMap library code.
*/


/*****************************************************************************/
/* N O T E S

The floating point array, *grid, that contains the data points will be
filled for a single species for the requested data slice.  Depending upon
the type of data slice, the following variables will be ignored when the
slice indicates ALL of the given dimension instead of ONE: 

	selected_column		(used for YZSLICE, YZTSLICE; else ignored)
	selected_row		(used for XZSLICE, XZTSLICE; else ignored)
	selected_level		(used for XYSLICE, YZSLICE, XZSLICE; else ignored)
	selected_step 		(used for XYSLICE, YZSLICE, XZSLICE, XYZSLICE; else ignored)

When header information is read, default range clamps will be set for
extents of column, row, level and step ranges.  Before the grid data 
data buffer is stuffed, the caller may reset the clamps to reduced ranges.
For example, instead of getting XYZSLICE for ALL time steps, say 0-24, clamps for
the steps 10 to 20 could be set.                                             

The *grid data array will be stuffed with these indices increasing, in
order, the fastest: col (x), row (y), level (z), step (t).  To access
a given element represented by [col,row,level,step], the index for 
grid[index] will be calculated, using 0-based values for the 4 indices, as: 


#define NCOL (col_max - col_min + 1)
#define NROW (row_max - row_min + 1)
#define NLEVEL (level_max - level_min + 1)

		index = col + 
			row * NCOL +
			level * NCOL * NROW +
			step * NCOL * NROW * NLEVEL;

Caller responsibilities when using this data structure include:
(1) malloc STRUCT.filename and fill with the file name of interest
(2) set double character pointers to NULL with	
	STRUCT.species_short_name = NULL;
	STRUCT.species_long_name = NULL;
	STRUCT.units_name = NULL;
(3) set the grid points array to NULL with
	STRUCT.grid = NULL;
(4) set the sdate and stime integer arrays to NULL with
	STRUCT.sdate = NULL;
	STRUCT.stime = NULL;
(5) set the map info and data label strings to NULL with
	STRUCT.map_info = NULL;
	STRUCT.data_label = NULL;
(6) set the host info to NULL with
       STRUCT.filehost.ip = NULL;
       STRUCT.filehost.name = NULL;
(7) calling free_vis to release memory for each VISDATA struct malloced

An initial call to get_info will set all of these pointers.  Repeated
calls to get_info with the pointers set to non-NULL values will result
in only resetting the clamps and the selection criteria to extrema.
The non-NULL pointers will not be set again!
                                                                             */
/*****************************************************************************/

        /* unistd.h messes up compilation sometimes because of "link" */
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

        /* in order to get the linker to resolve Kathy's 
           subroutines when using CC to compile */
#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */


enum dataset_type {
	UNDETERMINED = 0,		/* undetermined data format type */
	netCDF_DATA  = 1,		/* netCDF EDSS gridded file */
	UAM_DATA,			    /* UAM gridded conc or emis file */
	UAMV_DATA,			    /* UAM-V formatted file */
	netCDF_OBS
	};

	/* dataset slice types */
#define	NONESLICE (0)			/* No data, just header info */
#define	XYSLICE   (1)			/* XY slice at 1 level & 1 step */
#define	YZSLICE	  (2)			/* YZ slice at 1 column & 1 step */
#define	XZSLICE	  (3)			/* XZ slice at 1 row & 1 step */
#define	XYZSLICE  (4)			/* XY slice at all levels & 1 step */
#define	XYTSLICE  (5)			/* XY slice at 1 level & all steps */
#define	YZTSLICE  (6)			/* YZ slice at 1 column & all steps */
#define	XZTSLICE  (7)			/* XZ slice at 1 row & all steps */
#define	XYZTSLICE (8)			/* XY slice at all levels & all steps */	

typedef struct 
	{
	char *ip;			/* host IP number */
	char *name;			/* host name */
	int   port;			/* host port number */
	} HOST_INFO;

typedef struct
	{
					/*************************************/
					/******** General Header Info ********/
					/*   filled in by calls to get_info  */
					/*************************************/
	char *filename;			    /* data file name                    */	
	HOST_INFO filehost;		    /* host where the data file resides  */ 
	enum dataset_type dataset;  /* netCDF_DATA, UAM_DATA, or UAMV_DATA */
	int nspecies;			    /* number of species in file         */
	char **species_short_name;	/* short names of species in file    */
	char **species_long_name;	/* long names of species in file     */
	char **units_name;		    /* units for species in file         */

	char *map_info;			/* map information                   

					   if dataset is UAM_DATA or UAMV_DATA:
						float llx lly urx ury 
						int  utm_zone ncol nrow 

					   if dataset is netCDF_DATA:
						int grid_type
						float xorig yorig 
						      xcell ycell 
						      xcent ycent 
						      p_gam p_bet p_alp
						int ncol nrow                */

	char *data_label;		/* data information label            */
	int first_date;			/* Julian date for first step        */
	int first_time;			/* Julian time for first step        */
	int last_date;			/* Julian date for last step         */
	int last_time;			/* Julian time for last step         */
	int incr_sec;			/* Julian time increment in seconds  */	
	int ncol;			/* number of columns in grid         */
	int nrow;			/* number of rows in grid            */
	int nlevel;			/* number of levels in grid          */
	int nstep;			/* number of time steps in file      */
					/*************************************/
					/* Initial clamps set by get_info    */
					/* but modifiable by user before     */
					/* calling get_data                  */
					/*************************************/
	int col_min;			/* column min clamp                  */
	int col_max;			/* column max clamp                  */
	int row_min;			/* row min clamp                     */
	int row_max;			/* row max clamp                     */
	int level_min;			/* level min clamp                   */
	int level_max;			/* level max clamp                   */
	int step_min;			/* step min clamp                    */
	int step_max;			/* step max clamp                    */
	int step_incr;			/* step increment                    */
					/*************************************/
					/* Default selections set by         */
					/* get_info but modifiable by user   */
					/* before calling get_data           */
					/*************************************/
	int slice;		        /* type of data slice grid contains  */
	int selected_species;		/* which species selected, 1-based   */
	int selected_col;		/* which column selected, 1-based    */
	int selected_row;		/* which row selected, 1-based       */
	int selected_level;		/* which level selected, 1-based     */
	int selected_step;		/* which step selected, 1-based      */
					/*************************************/
					/* Variables set by get_data         */
					/*************************************/
	float *grid;			/* grid of data points               */
	int *sdate;			/* Julian start date for time steps  */
	int *stime;			/* Julian start time for time steps  */
	float grid_min;			/* data grid min of data slice       */
	float grid_max;			/* data grid max of data slice       */
					/*************************************/
	} VIS_DATA;


/* mapping options */

/* if mapping options flag is set to zero,

	Lambert maps wil be drawn WITH lat-lon lines
	UTM maps will be drawn with COUNTY boundaries
*/
	
#define MAP_NO_LATLON  (1)	/* Lambert maps will be drawn without LATLON lines */
#define MAP_UTM_STATES (2)	/* UTM maps will be state maps */


        /* in order to get the linker to resolve Kathy's
           subroutines when using CC to compile */
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */


#endif 	/* ___VISDATA_H___ */

