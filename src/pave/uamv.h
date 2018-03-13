/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)uamv.h	1.5
 *     Pathname: /tmp_mnt/pub/storage/edss/framework/src/pave/pave_include/SCCS/s.uamv.h
 * Last updated: 09 May 1996 12:24:55
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

#ifndef READUAMV_H
#define READUAMV_H

        /* unistd.h messes up compilation sometimes because of "link" */
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

/* 
#define UNICOS 1
*/
/* ... definitions ... */

enum uamv_types {
	UNKNOWN=0, 	UAMV_WIND, UAMV_TEMP, UAMV_CLOUD, UAMV_H2O, UAMV_RAIN, 
	UAMV_VDIF,  UAMV_HEIGHT, UAMV_FAVER, UAMV_FINST
	};

#define PAVE_SUCCESS 1  	/* successful function return */
#define FAILURE 0 	/* unsuccessful function return */
#define IOERROR -1 	/* unsuccessful i/o function return */

#define WORD_SIZE (sizeof(int))		/* word size - integer or float */
#ifdef UNICOS
#define REC_CW	(1)
#define CW_BCW	(0)
#define CW_EOR	(010)
#define CW_EOF	(016)
#define CW_EOD	(017)
#define TRUE	(1)
#define FALSE	(0)
#else
#define REC_CW	(2)
#endif

#define NO_OFFSET 	            (0)
#define UAMV_REC_OFFSET		    (2)
#define UAMV_TIME_STEP_HEADER	(2)

#define FAVG_MSG			(80)
#define FAVG_NFIN_NSPC		(2)
#define FAVG_NFIN			(0)
#define FAVG_NSPC			(1)
#define FAVG_GRID_INFO		(11)
#define FAVG_IXFB			(0)

#define SPC_SPEC 			(10)

/* ... typedef definitions ... */

typedef struct hdrStruct
	{
	int 	icol;
	int 	irow;
	int 	ilevel;
	int 	ispec;
	int 	utm_zone;
	int 	hour1;
	int 	hour2;
	int 	begin_date;
	int 	end_date;
	int     nstep;
	int     *sdate;
	int     *stime;
	int 	uamv_type;
	int		datapos;
	int		steplen;
	int		speclen;
	int		levellen;	
	int		domainlen;	
	int		numfin;
	int		nfx;
	int		nfy;
	int		ixfb;
	int		iyfb;
	int		ixfe;
	int		iyfe;
	int		nhf;
	int		nvf;
	int		fine_grid;
	int		gridlen;
	int		grid_offset;
	float 	sw_utmx;
	float 	sw_utmy;
	float 	ne_utmx;
	float 	ne_utmy;
	char *	spec_list;
	char * 	file_id;
	} UAMV_INFO;

#endif  /* READUAMV_H */
