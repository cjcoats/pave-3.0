#ifndef BUS_CONNECT_H
#define BUS_CONNECT_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)BusConnect.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.BusConnect.h
 * Last updated: 12/15/97 16:25:29
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

////////////////////////////////////////////////////////////
// BusConnect.h: 
// K. Eng Pua
// Jan 19, 1995
////////////////////////////////////////////////////////////
//
//   BusConnect Class
//   BusConnect                                   Abstract
//        1. Provides a bridge between software
//           bus C functions and DriverWnd class
//
////////////////////////////////////////////////////////////

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "AppInit.h"
#include "bts.h"

extern "C" {
#include "bus.h"
}

class BusConnect {
	friend void pave_callback( struct BusData *bd, struct BusMessage *bmsg ); 
 
   public:
	BusConnect ( AppInit *app );
        
	virtual const char *const className() { return "BusConnect"; }

	virtual void busCallback(char *msg) = 0 ;

	BusData	*getBusData() const { return bd_; };

	int	isConnectedToBus(void) const { return isConnected_; }

   protected:
	BusData		*bd_;

   private:
	int		isConnected_;
};



#endif
