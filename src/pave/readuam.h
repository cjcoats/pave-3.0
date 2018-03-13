#ifndef READUAM_H
#define READUAM_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)readuam.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.readuam.h
 * Last updated: 12/15/97 16:28:58
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

/*
MODIFICATION HISTORY:

WHO  WHEN       WHAT
---  ----       ----
SRT  04/06/95   Added #ifdef __cplusplus lines
*/

        /* in order to get the linker to resolve Kathy's
           subroutines when using CC to compile */
#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

	/* unistd.h messes up compilation sometimes because of "link" */
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif


/* 
#define UNICOS 1
*/
/* ... definitions ... */

enum uam_types {
	UNKNOWN=0, UAM_AVER, UAM_INST, UAM_EMIS, UAM_WIND, UAM_AIRQ,
	UAM_DIFF,  UAM_REGT, UAM_TEMP, UAM_TOPC, UAM_PTSR, UAM_VERT, UAM_BNDRY
	};

#define PAVE_SUCCESS    (1)
#define FAILURE         (0)
#define IOERROR         (-1)

#define WORD_SIZE (sizeof(int))		/* word size - integer or float */
#ifdef UNICOS
#define REC_CW	1
#define CW_BCW	0
#define CW_EOR	010
#define CW_EOF	016
#define CW_EOD	017
#define TRUE	1
#define FALSE	0
#else
#define REC_CW	2
#endif

#define FILE_DESC_HEADER	(76)
#define REGN_DESC_HEADER	(15)
#define WIND_SCAL_HEADER	(8)
#define SEGM_DESC_HEADER	(4)
#define TIME_STEP_HEADER	(4)
#define TIME_INV_CNTR_REC	(2)
#define POINT_SOURCE_DEF	(6)
#define TIME_VAR_CNTR_REC	(2)
#define POINT_SOURCE_LOC	(5)
#define NO_OFFSET		    (0)
#define UAM_REC_OFFSET		(11)

/*
 * File Description Header Record
 * word 1-10:   character       file name (10 characters, 1 character/word)
 * word 11-70:  character       file id (60 characters, 1 character/word)
 * word 71:     integer         number of segments (must be 1)
 * word 72:     integer         number of chemical species
 * word 73:     integer         Julian beginning date of the file
 * word 74:     float           beginning time of the file
 * word 75:     integer         Julian ending date of the file
 * word 76:     float           ending time of the file
 */

enum file_desc {
	FDH_FILEID=10,
	FDH_SEGS=70, FDH_CHEM, FDH_BEGD, FHD_BEGH, FDH_ENDD, FDH_ENDH };

/*
 * Region Description Header
 * word 1:      float           x-coordinate (UTM units)
 * word 2:      float           y-coordinate (UTM units)
 * word 3:      integer         UTM zone
 * word 4:      float           x-location (meters)
 * word 5:      float           y-location (meters)
 * word 6:      float           grid cell size in the x-direction (meters)
 * word 7:      float           grid cell size in the y-direction (meters)
 * word 8:      integer         number of grid cells in the x-direction
 * word 9:      integer         number of grid cells in the y-direction
 * word 10:     integer         number of grid cells in the z-direction
 * word 11:     integer         number of cells between surface layer and
 *                                      diffusion break
 * word 12:     integer         number of cells between diffusion break and
 *                                      top of region
 * word 13:     float           height of surface layer (meters)
 * word 14:     float           minimum height of cells between surface layer
 *                                      and diffusion break (meters)
 * word 15:     float           minimum height of cells between diffusion
 *                                      break and top of region (meters)
 */

enum regn_desc {
	RGN_XCRD=0, RGN_YCRD, RGN_UTMZ, RGN_XLOC, RGN_YLOC,
	RGN_GCSX,   RGN_GCSY, RGN_NGCX, RGN_NGCY, RGN_NGCZ,
	RGN_NC_SL_DB, 
	RGN_NC_DB_TR,
	RGN_HEIGHT_SL,
	RGN_MINH_SL_DB,
	RGN_MINH_DB_TR
	};

/* Segment Description Header Record
 * word 1:      integer         x-location of the segment origin with
 *                                      respect to origin of the region
 * word 2:      integer         y-location of the segment origin with
 *                                      respect to origin of the region
 * word 3:      integer         number of grid cells in the segment in the
 *                                      x-direction
 * word 4:      integer         number of grid cells in the segment in the
 *                                      y-direction
 */

enum segm_desc {
	SEG_XLOC=0, SEG_YLOC, SEG_NGCX, SEG_NGCY
	};

/*
 * Species Description Header Record
 * word 1-10:	integer		species 1 (10 characters, 1 character/word) 
 * .........
 * .........
 * .........
 * word ..-10N:	integer		species N
 */

enum spec_desc {
	SPC_SPEC=10
	};
#define MAXHEADER 	(FDH_SEGS-FDH_FILEID)

/* for PTSOURCE files only */
/*
 * word 1:	integer		segment number (musr be 1)
 * word 2:	integer		number of point sources in segment
 */
enum time_inv_counter_record {
	TIC_SEGN=0, TIC_NPTS
	};
/* ... typedef definitions ... */

typedef struct hdrStruct
	{
	int 	icol;
	int 	irow;
	int 	ilevel;
	int 	ispec;
	char *	spec_list;
	float 	sw_utmx;
	float 	sw_utmy;
	float 	ne_utmx;
	float 	ne_utmy;
	int 	utm_zone;
	int 	hour1;
	int 	hour2;
	int 	begin_date;
	int 	end_date;
	int     nstep;
	int     *sdate;
	int     *stime;
	int 	uam_type;
	char * 	file_id;
	int	datapos;
	int	steplen;
	int	speclen;
	int	levellen;	
	} UAM_INFO;

        /* in order to get the linker to resolve Kathy's
           subroutines when using CC to compile */
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif 	/* READUAM_H */
