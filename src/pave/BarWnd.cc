/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: BarWnd.cc 83 2018-03-12 19:24:33Z coats $
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
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// File:    BarWnd.C
// Author:  K. Eng Pua
// Date:    Jan 28, 1994
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include "BarWnd.h"

BarWnd::BarWnd ( AppInit *app, char *name, char *drawtype, ComboData *combo,
                 Dimension width, Dimension height,
                 int exit_button_on,
                 char **colornames, int numcolornames ) :
    ComboWnd ( app, name, drawtype, combo,
               width, height,
               exit_button_on,
               colornames, numcolornames )
    {

    }



///////////////////////////////////////////////////////////////////////

void BarWnd::drawBar()
    {
    int          i,j, colr, idy;
    int          cx, cy, height, width, inc = 1;
    float        valu;
    Pixel        background;
    Pixmap       tile;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "In drawBar combo_->obs_num()=%d combo_->y_data_num()=%d\n",
              combo_->obs_num(), combo_->y_data_num() );
#endif // DIAGNOSTICS

    for ( i=0; i < combo_->obs_num(); i++ )
        {
        for ( j = 0; j < combo_->y_data_num(); j++ )
            {

            colr = j % numcolornames_;
            XSetForeground ( dpy_, gc_,
                             getNamedPixel ( colornames_[colr] ) );

            idy = i*combo_->y_data_num() + j;

            valu = combo_->val_y ( idy );
            if ( valu > 0.0 )
                {
                cy = s.scaley ( valu );
                height = abs ( cy - s.scaley ( combo_->val_ymin() >0.0 ? combo_->val_ymin() : 0.0 ) );
                }
            else
                {
                cy = s.scaley ( 0.0 );
                height = abs ( cy - s.scaley ( valu ) );
                }

            cx = s.scalex ( ( float ) ( idy+inc ) );
            width = abs ( cx - s.scalex ( ( float ) ( idy+inc-1 ) ) );

            XtVaGetValues ( canvas_,
                            XtNbackground, &background,
                            NULL );

            tile = XmGetPixmap ( XtScreen ( canvas_ ),
                                 patterns_[j % XtNumber ( patterns_ )],
                                 getNamedPixel ( colornames_[j % numcolornames_] ),
                                 background );

            XSetTile ( dpy_, gc_, tile );

            XSetFillStyle ( dpy_, gc_, FillTiled );

            // Draw the bar with filled pattern.
            XFillRectangle ( dpy_, drw_, gc_, cx, cy, width, height );

            // Reset foreground color
            XSetForeground ( dpy_, gc_,
                             BlackPixel ( dpy_, DefaultScreen ( dpy_ ) ) );
            XSetFillStyle ( dpy_, gc_, FillSolid );

            // Draw a surrounding frame.
            XDrawRectangle ( dpy_, drw_, gc_, cx, cy, width, height );
            }
        inc++;

        }

    // Draw the base line at y = 0.
    if ( combo_->val_ymin() < 0.0 )
        XDrawLine ( dpy_, drw_, gc_,
                    s.scalex ( combo_->val_xmin() ), s.scaley ( 0.0 ),
                    s.scalex ( combo_->val_xmax() ), s.scaley ( 0.0 ) );
    }


void BarWnd::drawDetail()
    {
    setForeground ( "black" );

    drawSetup ( canvas_, pix_, gc_, &s );
    drawBar();
    drawFrame();
    drawTitles ( combo_->title1(), combo_->title2(), combo_->title3(), combo_->xtitle(), combo_->ytitle() );
    drawYtics ( 4, combo_->plotformatY(), 0, 1 );
    drawLegend();
    drawBarTics();

    setForeground ( "black" );
    }


#include <sys/types.h>
#include <unistd.h>


void BarWnd::writeProbeFile ( float x1, float x2, float, float )
    {
    int          i, j, idy, match;
    int          inc = 1;
    float        cx;
    FILE         *fp;
    pid_t    pid;

    pid = getpid();
    probefilename_[0] = '\0';
    sprintf ( probefilename_, "/tmp/prob.%d", pid );
    if ( ( fp = fopen ( probefilename_, "w" ) ) == NULL )
        {
        fprintf ( stderr, "Can't write data to file.\n" );
        fclose ( fp );
        return;
        }

    for ( i=0; i < combo_->obs_num(); i++ )
        {
        match = 0;
        for ( j = 0; j < combo_->y_data_num(); j++ )
            {

            idy = i*combo_->y_data_num() + j;
            cx = ( float ) ( idy+inc );

            if ( ( cx   >= x1 ) && ( cx <= x2 ) )
                {
                match = 1;
                if ( combo_->exists_y_label_list() )
                    fprintf ( fp, "%s %s:%f\n", combo_->val_xs ( i ),
                              combo_->y_label_list ( j ),
                              combo_->val_y ( idy ) );
                else
                    fprintf ( fp, "%s:%f\n", combo_->val_xs ( i ),
                              combo_->val_y ( idy ) );
                }
            }
        if ( match ) fprintf ( fp, "\n" );
        inc++;
        }
    fclose ( fp );
    }

void BarWnd::writeProbeObsFile ( int x1, int x2, int y1, int y2 )
    {
    }

void BarWnd::overlay_ts ( int x1, int x2, int y1, int y2 )
    {
    }

