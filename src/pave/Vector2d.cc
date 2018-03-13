/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Vector2d.cc 83 2018-03-12 19:24:33Z coats $
 *  Copyright (C) 1996-2004 MCNC
 *            (C) 2004-2010 UNC Institute for the Environment
 *            (C) 2018-     Carlie J. Coats, Jr., Ph.D.
 *
 *  Licensed under the GNU General Public License Version 2.
 *  See enclosed gpl.txt for more details
 *
 *  For further information on PAVE:
 *      Usage: type -usage in PAVE's standard input
 *      User Guide: https://cjcoats.github.io/pave/PaveManual.html
 *      FAQ:        https://cjcoats.github.io/pave/Pave.FAQ.html
 *
 ****************************************************************************/

#include <stdlib.h>

#include "vis_data.h"
#include "Vector2d.h"

static const char SVN_ID[] = "$Id: Vector2d.cc 83 2018-03-12 19:24:33Z coats $";

VECTOR2D_DATA::VECTOR2D_DATA ( void )
    {
    vect_thickness=2;
    vdata_x = vdata_y = NULL;
    }
