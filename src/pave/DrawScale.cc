/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: DrawScale.cc 83 2018-03-12 19:24:33Z coats $
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
//////////////////////////////////////////////////////////////////////
//  Drawscale.C
//  K. Eng Pua
//  May 12, 1993
//
//////////////////////////////////////////////////////////////////////
#include "DrawScale.h"

void DrawScale::scaleInit ( float x1, float y1, float x2, float y2,
                            int wmaxx, int wmaxy,
                            int offset_left, int offset_right,
                            int offset_top,  int offset_bottom )
    {
    xmin_ = x1;
    xmax_ = x2;
    ymin_ = y1;
    ymax_ = y2;
    offset_left_   = offset_left;
    offset_right_  = offset_right;
    offset_top_    = offset_top;
    offset_bottom_ = offset_bottom;

    sx_   = ( ( float ) ( wmaxx-offset_left_-offset_right_ ) ) /
            ( float ) ( xmax_-xmin_ );
    sy_   = ( ( float ) ( wmaxy-offset_top_-offset_bottom_ ) ) /
            ( float ) ( ymax_-ymin_ );


#ifdef DIAGNOSTICS
    fprintf ( stderr, "xmin=%f\n",xmin_ );
    fprintf ( stderr, "xmax=%f\n",xmax_ );
    fprintf ( stderr, "ymin=%f\n",ymin_ );
    fprintf ( stderr, "ymax=%f\n",ymax_ );
    fprintf ( stderr, "wmaxx=%d\n",wmaxx );
    fprintf ( stderr, "wmaxy=%d\n",wmaxy );
    fprintf ( stderr, "sx=%f\n",sx_ );
    fprintf ( stderr, "sy=%f\n",sy_ );
#endif
    }


int DrawScale::scalex ( float x )
    {
    return ( int ) ( ( x-xmin_ ) *sx_ ) + offset_left_;
    }


int DrawScale::scaley ( float y )
    {
    return ( int ) ( ( ymax_-y ) *sy_ ) + offset_top_;
    }



float DrawScale::fscalex ( float x )
    {
    return ( float ) ( ( x-xmin_ ) *sx_ ) + offset_left_;
    }


float DrawScale::fscaley ( float y )
    {
    return ( float ) ( ( ymax_-y ) *sy_ ) + offset_top_;
    }


float DrawScale::gridx ( int x )
    {
    return xmin_+ ( float ) ( x - offset_left_ ) /sx_;
    }


float DrawScale::gridy ( int y )
    {
    return ymax_ - ( float ) ( y - offset_top_ ) /sy_;
    }



