///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//         This example code is from the book:
//
//           Object-Oriented Programming with C++ and OSF/Motif
//         by
//           Douglas Young
//           Prentice Hall, 1992
//           ISBN 0-13-630252-1
//
//         Copyright 1991 by Prentice Hall
//         Hill under terms of the GNU Public License.  See gpl.txt for more details.
//
//  Permission to use, copy, modify, and distribute this software for
//  any purpose except publication and without fee is hereby granted, provided
//  that the above copyright notice appear in all copies of the software.
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// RGBView.C: Display the contents of a ColorModel as
//            RGB color components
/////////////////////////////////////////////////////////////
#include "RGBView.h"
#include "ColorModel.h"
#include <Xm/Xm.h>
#include <Xm/TextF.h>
#include <stdio.h>

static const char SVN_ID[] = "$Id: RGBView.cc 83 2018-03-12 19:24:33Z coats $";

RGBView::RGBView ( Widget parent, char *name ) :
    TextView ( parent, name )
    {
    // Empty
    }

void RGBView::update ( ColorModel *model )
    {
    char buf[100];

    sprintf ( buf, "%3.3d", model->red() );   // Red
    XmTextFieldSetString ( _field1, buf );
    sprintf ( buf, "%3.3d", model->green() ); // Green
    XmTextFieldSetString ( _field2, buf ) ;
    sprintf ( buf, "%3.3d", model->blue() );  // Blue
    XmTextFieldSetString ( _field3, buf );
    }
