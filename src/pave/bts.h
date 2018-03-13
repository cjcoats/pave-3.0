#ifndef ___BTS___
#define ___BTS___
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
 * Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.bts.h
 * Last updated: 12/02/99 14:17:55
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

FILE:           bts.h

AUTHOR:         Steve Thorpe
                MCNC Environmental Programs
                thorpe@ncsc.org

DATE:           12/9/94

PURPOSE:        Header file for the miscellaneous Environmental
        Decision Support System (EDSS) Analysis and
        Visualization "Behind The Scenes" code.

************************************************************/
#include <math.h>

/* in order to get the linker to resolve Kathy's
   subroutines when using CC to compile */
#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */


/* for SCCS usage
static char btshSid[] = "@(#)bts.h  2.3 /env/proj/archive/edss/src/pave/pave_include/SCCS/s.bts.h 12/02/99 14:17:55";
*/


#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


/* unistd.h messes up compilation sometimes because of "link" */
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif


/* bald messes this up in some header file */
#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */

/* generic UNIX includes */

#include <stdio.h>
#include <string.h>

#if !(defined(__sparc) || defined(__hpux))
#include <strings.h>
#endif /* __sparc */

#include <ctype.h>
#include <stdlib.h>
#include <memory.h>
/* SRT removed 950915 #include <ieeefp.h> */
#include <assert.h>
#include <unistd.h> /* extern unsigned sleep(unsigned); */
#include <time.h>
#include <sys/time.h>



/* balay includes */

#include "vis_data.h"
#include "vis_proto.h"
#include "visDataClient.h"
#include "netcdf.h"
#include "readuam.h"


/* pearson includes */
#include "bus.h"
#include "busClient.h"
/*#include "busSocket.h"*/
#include "busRW.h"
/*#include "busRWMessage.h"*/
#include "busMsgQue.h"
#include "busRepReq.h"
#include "busError.h"
#include "busDebug.h"
#include "busXtClient.h"
#include "busVersion.h"


/* thorpe includes */

#include "parse.h"
#include "utils.h"
#include "retrieveData.h"


/* nextimec() is in Carlie Coats' Models-3 IO/API library */
extern void nextimec ( int *jdate , int *jtime , int dtime );



/* indexing macro */
/* revised 2/2018 by CJC for "medium" memory model */

#define INDEX(col, row, level, step, NCOL, NROW, NLEVEL) \
    ( ( (long)(col) +                 \
        ((long)(row) * (NCOL)) +           \
        ((long)(level) * (NCOL) * (NROW)) +        \
        ((long)(step) * (NCOL) * (NROW) * (NLEVEL))))


/* in order to get the linker to resolve Kathy's
   subroutines when using CC to compile */
#ifdef __cplusplus

    /* blt-1.7 includes */
    /*
    #include "applications/extloader/extConfig.h"
    #include "src/blt.h"
    #include "src/bltGrElem.h"
    #include "src/bltGrPS.h"
    #include "src/bltGrTag.h"
    #include "src/bltGraph.h"
    #include "src/bltList.h"
    */

    /* netcdf-2.3.2pl2 includes */
    /*
    #include "libsrc/alloc.h"
    #include "libsrc/error.h"
    #include "libsrc/local_nc.h"
    #include "libsrc/netcdf.h"
    #include "ncdump/dumplib.h"
    #include "ncdump/ncdump.h"
    #include "ncdump/vardata.h"
    #include "ncgen/generic.h"
    #include "ncgen/genlib.h"
    #include "ncgen/msofttab.h"
    #include "ncgen/ncgen.h"
    #include "ncgen/vmstab.h"
    #include "nctest/add.h"
    #include "nctest/emalloc.h"
    #include "nctest/error.h"
    #include "nctest/testcdf.h"
    #include "nctest/tests.h"
    #include "nctest/val.h"
    #include "xdr/NOTICE.h"
    #include "xdr/types.h"
    #include "xdr/xdr.h"
    */

    /* plplot4p99i includes */
    /*
    #include "drivers/tk/plserver.h"
    #include "include/drivers.h"
    #include "include/metadefs.h"
    #include "include/pdf.h"
    #include "include/plcore.h"
    #include "include/plevent.h"
    #include "include/plplot.h"
    #include "include/plplotP.h"
    #include "include/plplotTK.h"
    #include "include/plplotX.h"
    #include "include/plstream.h"
    #include "include/plstubs.h"
    #include "include/pltcl.h"
    #include "include/pmdefs.h"
    #include "include/tclMatrix.h"
    #include "sys/amiga/cf/plConfig.h"
    #include "sys/amiga/cf/plDevs.h"
    #include "sys/amiga/old/plamiga.h"
    #include "sys/amiga/src/pla_menu.h"
    #include "sys/amiga/src/plamiga.h"
    #include "sys/dos/djgpp/cf/plconfig.h"
    #include "sys/dos/djgpp/cf/pldevs.h"
    #include "sys/os2/pmserv/pmdefs.h"
    */

    /* tcl7.3 includes */
    /*
    #include "tcl.h"
    #include "tclRegexp.h"
    #include "tclUnix.h"
    #include "tclInt.h"
    #include "compat/dirent.h"
    #include "compat/dirent2.h"
    #include "compat/float.h"
    #include "compat/limits.h"
    #include "compat/stdlib.h"
    #include "compat/string.h"
    #include "compat/unistd.h"
    #include "patchlevel.h"
    */

    /* tk3.6 includes */
    /*
    #include "default.h"
    #include "tk.h"
    #include "tkInt.h"
    #include "patchlevel.h"
    #include "tkText.h"
    #include "tkConfig.h"
    #include "tkCanvas.h"
    #include "ks_names.h"
    */

    }

#endif /* #ifdef __cplusplus */

#endif  /* #ifndef ___BTS___ */
