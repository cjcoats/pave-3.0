/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)nan_incl.h	1.5
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.nan_incl.h
 * Last updated: 03/21/00 17:15:50
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

#if defined(__alpha) || defined(_AIX) || defined(__hpux) || defined(linux) || defined(__OPENNT)
#include <math.h>

#if defined (_AIX)
#ifndef isnanf
#define isnanf(x)       (((*(long *)&(x) & 0x7f800000L) == 0x7f800000L) && \
                            ((*(long *)&(x) & 0x007fffffL) != 0x00000000L))
#endif /* #ifndef isnanf */
#endif /* _AIX */

#if defined (CRAY)
#endif

#if defined(linux) || defined(__OPENNT)
#define isnanf(x) isnan((double)x)
#endif

#ifdef __hpux
#define isnanf(x) isnan(x)
#endif


#else
#include <ieeefp.h>
#endif




