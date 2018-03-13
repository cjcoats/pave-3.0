/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: ComboWnd.cc 83 2018-03-12 19:24:33Z coats $
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
// File:    ComboWnd.C
// Author:  K. Eng Pua
// Date:    Jan 28, 1994
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include "ComboWnd.h"


void ComboWnd::drawDetail() { } // SRT 950929


ComboWnd::ComboWnd ( AppInit *app, char *name, char *drawtype, ComboData *combo,
                     Dimension width, Dimension height,
                     int exit_button_on,
                     char **colornames, int numcolornames,
                     char **in_patterns, int numpatterns ) :
    DrawWnd ( app, name, drawtype, width, height, colornames, numcolornames, 1 ),
    Menus(),
    RubberBand()
    {
    combo_ = combo;

    exit_button_on_ = exit_button_on;
    colornames_ = colornames;
    numcolornames_ = numcolornames;
    patterns_ = in_patterns;
    numpatterns_ = numpatterns;

    initComboWnd();
    createUI ( draw_ );
    initRubberBand ( &s, canvas_ );

    manage();
    }



void ComboWnd::initComboWnd()
    {
    interact_mode_ = PROBE_MODE;
    probe_dialog_ = ( Widget ) NULL;
    zoom_dialog_ = ( Widget ) NULL;

    zoom_[0].GRID_X_MIN_ = combo_->val_xmin();
    zoom_[0].GRID_Y_MIN_ = combo_->val_ymin();
    zoom_[0].GRID_X_MAX_ = combo_->val_xmax();
    zoom_[0].GRID_Y_MAX_ = combo_->val_ymax();

    zoom_[0].parent = 0;
    }


void ComboWnd::createUI ( Widget parent )
    {
    // File ----------------------------------------------
    menu_struct sub_menu_file[1];
    if ( exit_button_on_ == 1 )
        sub_menu_file[0].name = "Exit";
    else
        sub_menu_file[0].name = "Close";
    sub_menu_file[0].func = &ComboWnd::exitCB;
    sub_menu_file[0].sub_menu = ( struct menu_struct * ) NULL;
    sub_menu_file[0].n_sub_items = 0;
    sub_menu_file[0].sub_menu_title = ( char * ) NULL;

    // Option-Interact ------------------------------------
    menu_struct sub_menu_interact[2];
    sub_menu_interact[0].name = "Probe";
    sub_menu_interact[0].func = &ComboWnd::probeCB;
    sub_menu_interact[0].sub_menu = ( struct menu_struct * ) NULL;
    sub_menu_interact[0].n_sub_items = 0;
    sub_menu_interact[0].sub_menu_title = ( char * ) NULL;
    sub_menu_interact[1].name = "Zoom";
    sub_menu_interact[1].func = &ComboWnd::zoomCB;
    sub_menu_interact[1].sub_menu = ( struct menu_struct * ) NULL;
    sub_menu_interact[1].n_sub_items = 0;
    sub_menu_interact[1].sub_menu_title = ( char * ) NULL;

    // Option ---------------------------------------------
    menu_struct sub_menu_option[1];
    sub_menu_option[0].name = "Zoom..";
    sub_menu_option[0].func = &ComboWnd::controlCB;
    sub_menu_option[0].sub_menu = ( struct menu_struct * ) NULL;
    sub_menu_option[0].n_sub_items = 0;
    sub_menu_option[0].sub_menu_title = ( char * ) NULL;

    // Menu bar --------------------------------------------
    menu_struct PulldownData[3];
    PulldownData[0].name = "File";
    PulldownData[0].func = ( void ( * ) ( Widget, XtPointer, XtPointer ) ) NULL;
    PulldownData[0].sub_menu = sub_menu_file;
    PulldownData[0].n_sub_items = 1;
    PulldownData[0].sub_menu_title = ( char * ) NULL;
    PulldownData[1].name = "Interact";
    PulldownData[1].func = ( void ( * ) ( Widget, XtPointer, XtPointer ) ) NULL;
    PulldownData[1].sub_menu = sub_menu_interact;
    PulldownData[1].n_sub_items = 2;
    PulldownData[1].sub_menu_title = ( char * ) NULL;
    PulldownData[2].name = "Control";
    PulldownData[2].func = ( void ( * ) ( Widget, XtPointer, XtPointer ) ) NULL;
    PulldownData[2].sub_menu = sub_menu_option;
    PulldownData[2].n_sub_items = 1;
    PulldownData[2].sub_menu_title = ( char * ) NULL;

    createMenuBar ( parent, ( XtPointer ) this );
    createMenuButtons ( ( char * ) NULL, menuBar_, PulldownData, 3 );

    // Create Label widget
    status_ = XtVaCreateManagedWidget ( "X:  Y:",
                                        xmLabelWidgetClass, parent,
                                        XmNtopAttachment,   XmATTACH_WIDGET,
                                        XmNtopWidget,       menuBar_,
                                        XmNleftAttachment,  XmATTACH_FORM,
                                        XmNrightAttachment, XmATTACH_FORM,
                                        NULL );

    // Create drawing area widget
    canvas_ = XtVaCreateManagedWidget ( "canvas",
                                        xmDrawingAreaWidgetClass, parent,
                                        XtNwidth,       width_,
                                        XtNheight,      height_,
                                        XmNtopAttachment,   XmATTACH_WIDGET,
                                        XmNtopWidget,       status_,
                                        XmNleftAttachment,  XmATTACH_FORM,
                                        XmNrightAttachment, XmATTACH_FORM,
                                        XmNbottomAttachment,    XmATTACH_FORM,
                                        NULL );

    XtAddCallback ( canvas_,
                    XmNexposeCallback,
                    &ComboWnd::redisplayCB,
                    ( XtPointer ) this );

    XtAddCallback ( canvas_,
                    XmNresizeCallback,
                    &ComboWnd::resizeCB,
                    ( XtPointer ) this );
    }



void ComboWnd::drawLegend()
    {
    int          i, startx, starty, colr;
    int          leg_width, win_width, leg_per_line, row, col;
    int          junk, ascent, descent, leg_height;
    char         buf[20];
    int          tmp_len, leg_len;
    XCharStruct  overall_return;

    if ( combo_->exists_y_label_list() == 0 ) return;

    startx = s.scalex ( s.xmin_ ) - 50;
    starty = height_ - 45;

    leg_len = 0;

    for ( i=0; i< combo_->y_data_num(); i++ )
        {
        tmp_len = strlen ( combo_->y_label_list ( i ) );
        if ( tmp_len > leg_len )
            {
            leg_len = tmp_len;
            strcpy ( buf, combo_->y_label_list ( i ) );
            }
        }

    // Width of legend text + legend symbol + allowance.
    leg_width = leg_len * 8 + 20 +20;
    if ( def_font_ )
        leg_width = XTextWidth ( def_font_, buf, leg_len ) + 20 + 20;

    // Total window width reserved for legend
    win_width = width_ - startx*2;

    leg_per_line = win_width / leg_width;

    // Get the height of character 'H'
    XTextExtents ( def_font_, "H", 0, &junk, &ascent, &descent, &overall_return );
    leg_height = ascent+descent;

    row = 0;
    for ( i=0; i < combo_->y_data_num(); i++ )
        {
        colr = i % numcolornames_;
        XSetForeground ( dpy_, gc_, getNamedPixel ( colornames_[colr] ) );

        if ( ( col = i % leg_per_line ) == 0 ) row++;

        legendFunc ( i, startx+col*leg_width, starty+row*leg_height );

        // Reset foreground color
        XSetForeground ( dpy_, gc_,
                         BlackPixel ( dpy_,
                                      DefaultScreen ( dpy_ ) ) );

        XDrawString ( dpy_, drw_, gc_,
                      startx+col*leg_width+15,
                      starty+row*leg_height+6,
                      combo_->y_label_list ( i ),
                      strlen ( combo_->y_label_list ( i ) ) );
        }
    }


void ComboWnd::legendFunc ( int choice, int x, int y )
    {
    Pixel        background;
    Pixmap       tile;

    XtVaGetValues ( canvas_,
                    XtNbackground, &background,
                    NULL );

    tile = XmGetPixmap ( XtScreen ( canvas_ ),
                         patterns_[choice % XtNumber ( patterns_ )],
                         getNamedPixel ( colornames_[choice % numcolornames_] ),
                         background );

    XSetTile ( dpy_, gc_, tile );

    XSetFillStyle ( dpy_, gc_, FillTiled );

    XFillRectangle ( dpy_, drw_, gc_,
                     x-6, y-6, 12, 12 );

    // Reset foreground color
    XSetForeground ( dpy_, gc_,
                     BlackPixel ( dpy_, DefaultScreen ( dpy_ ) ) );
    XSetFillStyle ( dpy_, gc_, FillSolid );

    // Draw a surrounding frame.
    XDrawRectangle ( dpy_, drw_, gc_,
                     x-6, y-6, 12, 12 );
    }





void ComboWnd::drawBarTics()
    {
    int i, j, idy, inc = 1, cx;

    for ( i=0; i < combo_->obs_num(); i++ )
        {
        for ( j = 0; j < combo_->y_data_num(); j++ )
            {

            idy = i*combo_->y_data_num() + j;
            cx = s.scalex ( ( float ) ( idy+inc ) );

            if ( j == 0 )
                {
                XDrawString ( dpy_, drw_, gc_,
                              cx, s.scaley ( s.ymin_ )+14,
                              combo_->val_xs ( i ),
                              strlen ( combo_->val_xs ( i ) ) );
                }
            }
        inc++;
        }
    }


/////////////////////////////////////////////////////////////////////////////////

void ComboWnd::redisplayCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    ComboWnd *obj = ( ComboWnd * ) clientData;
    XmAnyCallbackStruct *cbs = ( XmAnyCallbackStruct * ) callData;
    XExposeEvent *event = ( XExposeEvent * ) cbs->event;

    obj->redisplay ( event );
    }


void ComboWnd::redisplay ( XExposeEvent *event )
    {
    if ( pix_ != ( Pixmap ) NULL )
        {
        XCopyArea ( XtDisplay ( canvas_ ), pix_, XtWindow ( canvas_ ), gc_,
                    event->x, event->y, event->width, event->height,
                    event->x, event->y );
        }
    else
        resize();
    }


void ComboWnd::resizeCB ( Widget, XtPointer clientData, XtPointer )
    {
    ComboWnd *obj = ( ComboWnd * ) clientData;
    obj->resize();
    }


void ComboWnd::resize()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "In resize\n" );
#endif

    // Get the size of the drawing area

    XtVaGetValues ( canvas_,
                    XmNwidth, &width_,
                    XmNheight, &height_,
                    NULL );

    s.scaleInit ( zoom_[curr_zoom_].GRID_X_MIN_, zoom_[curr_zoom_].GRID_Y_MIN_,
                  zoom_[curr_zoom_].GRID_X_MAX_, zoom_[curr_zoom_].GRID_Y_MAX_,
                  width_, height_ );

    if ( pix_ )
        XFreePixmap ( XtDisplay ( canvas_ ), pix_ );

    pix_ = XCreatePixmap ( XtDisplay ( canvas_ ),
                           XtWindow ( canvas_ ),
                           width_, height_,
                           DefaultDepthOfScreen ( XtScreen ( canvas_ ) ) );

    // Set pixmap color to white.
    setForeground ( "white" );
    setBackground ( "white" );
    XFillRectangle ( XtDisplay ( canvas_ ), pix_, gc_, 0, 0,
                     width_, height_ );

    drawDetail();

    /* Copy data.pix to drawing area widget. */
    XCopyArea ( XtDisplay ( canvas_ ), pix_, XtWindow ( canvas_ ), gc_,
                0, 0, width_, height_,
                0, 0 );
    }



void ComboWnd::exitCB ( Widget, XtPointer clientData, XtPointer )
    {
    ComboWnd *obj = ( ComboWnd * ) clientData;
    obj->exit_cb();
    }

void ComboWnd::exit_cb()
    {
    if ( exit_button_on_ == 1 )
        exit ( 1 );
    else
        unmanage();
    }


void ComboWnd::probeCB ( Widget, XtPointer clientData, XtPointer )
    {
    ComboWnd *obj = ( ComboWnd * ) clientData;
    obj->probe_cb();
    }

void ComboWnd::probe_cb()
    {
    interact_mode_ = PROBE_MODE;
    }


void ComboWnd::zoomCB ( Widget, XtPointer clientData, XtPointer )
    {
    ComboWnd *obj = ( ComboWnd * ) clientData;
    obj->zoom_cb();
    }

void ComboWnd::zoom_cb()
    {
    interact_mode_ = ZOOM_MODE;
    }

void ComboWnd::controlCB ( Widget, XtPointer clientData, XtPointer )
    {
    ComboWnd *obj = ( ComboWnd * ) clientData;
    obj->control_cb();
    }

void ComboWnd::control_cb()
    {
    if ( zoom_dialog_ == NULL )
        createZoomDialog();
    if ( XtIsManaged ( zoom_dialog_ ) ) XtUnmanageChild ( zoom_dialog_ );
    XtManageChild ( zoom_dialog_ );
    }



