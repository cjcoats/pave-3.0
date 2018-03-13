/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: RubberBand.cc 83 2018-03-12 19:24:33Z coats $
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
///////////////////////////////////////////////
//   ComboRB.C
//   K. Eng Pua
//   Feb 1, 1995
//
//   This is part of RubberBand.C program
///////////////////////////////////////////////

#include "RubberBand.h"

static int        badDrag  = 0;

void RubberBand::writeProbeFile ( float,float,float,float ) { } // SRT 950929
void RubberBand::resize() { } // SRT 950929

void RubberBand::initRubberBand ( DrawScale *s, Widget parent )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter RubberBand::initRubberBand()\n" );
#endif // #ifdef DIAGNOSTICS

    assert ( s );
    assert ( parent );

    s_ = s;
    parent_widget_ = parent;   // Parent widget is a drawing area widget

    curr_zoom_ = 0;
    num_zooms_ = 0;

    xorgc_ = createXorGC ( parent_widget_ );

    XtAddEventHandler ( parent_widget_, ButtonPressMask,   FALSE,
                        &RubberBand::start_rb_EV, ( XtPointer ) this );
    XtAddEventHandler ( parent_widget_, ButtonMotionMask, FALSE,
                        &RubberBand::track_rb_EV, ( XtPointer ) this );
    XtAddEventHandler ( parent_widget_, ButtonReleaseMask, FALSE,
                        &RubberBand::end_rb_EV, ( XtPointer ) this );
    }



void RubberBand::zoomin()
    {
    static char str[100];
    XmString string;
    float x1,y1,x2,y2;

    x1 = s_->gridx ( startx_ );
    y1 = s_->gridy ( starty_ );
    x2 = s_->gridx ( lastx_ );
    y2 = s_->gridy ( lasty_ );

    x1 = floor ( x1 );
    y1 = floor ( y1 );
    x2 = ceil ( x2 );
    y2 = ceil ( y2 );

    //   fprintf(stderr, "In zoomin. x1=%g y1=%g x2=%f y2=%g\n", x1, y1, x2, y2);

    if (  ( x2 > x1 ) && ( y2 < y1 ) )
        {

        if ( num_zooms_ == MAX_ZOOM_WINDOWS )
            {
            XBell ( XtDisplay ( parent_widget_ ), 2 );
            fprintf ( stderr, "Maximum number of zoom window reached!" );
            string = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( str );
            XtVaSetValues ( status_, XmNlabelString, string, NULL );
            XmStringFree ( string );
            return;
            }

        num_zooms_++;
        zoom_[num_zooms_].parent = curr_zoom_;

        zoom_[num_zooms_].GRID_X_MIN_  = x1;
        zoom_[num_zooms_].GRID_X_MAX_  = x2;
        zoom_[num_zooms_].GRID_Y_MIN_  = y2;
        zoom_[num_zooms_].GRID_Y_MAX_  = y1;
        zoom_[num_zooms_].CLIP_GRID_X_MIN_ = x1;
        zoom_[num_zooms_].CLIP_GRID_X_MAX_ = x2;
        zoom_[num_zooms_].CLIP_GRID_Y_MIN_ = y2;
        zoom_[num_zooms_].CLIP_GRID_Y_MAX_ = y1;

        curr_zoom_ = num_zooms_;
        resize();

        // Update value on scale widget.
        XtVaSetValues ( zoom_scale_,
                        XmNmaximum, curr_zoom_,
                        XmNvalue, curr_zoom_,
                        NULL );
        }
    else
        {
        XBell ( XtDisplay ( parent_widget_ ), 2 );
        sprintf ( str, "Error: A minimum of 1x1 grid cell is required to zoom in." );
        string = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( str );
        XtVaSetValues ( status_, XmNlabelString, string, NULL );
        XmStringFree ( string );
        }
    }



/*******************************************
    RUBBER BAND FUNCTIONS
*******************************************/
void RubberBand::start_rb_EV ( Widget w,
                               XtPointer   clientData,
                               XEvent      *event,
                               Boolean     * )
    {
    RubberBand *obj = ( RubberBand * ) clientData;
    obj->start_rb ( w, event );
    }


void RubberBand::start_rb ( Widget w, XEvent *event )
    {
    char str[100];
    XmString string;
    float x, y;

    x = s_->gridx ( event->xbutton.x );
    y = s_->gridy ( event->xbutton.y );

    str[0] = '\0';

    if ( ( x > zoom_[curr_zoom_].GRID_X_MIN_ ) && ( x < zoom_[curr_zoom_].GRID_X_MAX_+1 ) && // SRT 950803
            ( y > zoom_[curr_zoom_].GRID_Y_MIN_ ) && ( y < zoom_[curr_zoom_].GRID_Y_MAX_+1 ) ) // SRT 950803
        {
        sprintf ( str, "Cell (%d,%d)",
                  ( int ) ceil ( ( double ) x ),
                  ( int ) ceil ( ( double ) y ) );
        iCellDown_ = ( int ) ceil ( ( double ) x )-1; // put it into 0 based
        jCellDown_ = ( int ) ceil ( ( double ) y )-1; // put it into 0 based

        lastx_ = startx_ = event->xbutton.x;
        lasty_ = starty_ = event->xbutton.y;

        assert ( xorgc_ );
        drawMyRubberBand ( w ); // start_rb
        }
    else
        {
#ifdef DIAGNOSTICS
        if ( x <= zoom_[curr_zoom_].GRID_X_MIN_ )
            fprintf ( stderr, "1 (x <= zoom_[curr_zoom_].GRID_X_MIN_) %f<=%f\n",
                      x, ( float ) zoom_[curr_zoom_].GRID_X_MIN_ );

        if ( x >= zoom_[curr_zoom_].GRID_X_MAX_+1 )
            fprintf ( stderr, "2 (x >= zoom_[curr_zoom_].GRID_X_MAX_+1) %f>=%f\n",
                      x, ( float ) zoom_[curr_zoom_].GRID_X_MAX_+1 );

        if ( y <= zoom_[curr_zoom_].GRID_Y_MIN_ )
            fprintf ( stderr, "3 (y <= zoom_[curr_zoom_].GRID_Y_MIN_) %f<=%f\n",
                      y, ( float ) zoom_[curr_zoom_].GRID_Y_MIN_ );

        if ( y >= zoom_[curr_zoom_].GRID_Y_MAX_+1 )
            fprintf ( stderr, "4 (y >= zoom_[curr_zoom_].GRID_Y_MAX_+1) %f>=%f\n",
                      y, ( float ) zoom_[curr_zoom_].GRID_Y_MAX_+1 );
#endif // DIAGNOSTICS

        XBell ( XtDisplay ( w ), 2 );
        sprintf ( str, "Cursor out of bounds" );
        if ( interact_mode_ == PROBE_MODE )
            {
            probe_close_cb();
            badDrag = 1;
            }
        }

    // Display message on status widget
    string = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( str );
    XtVaSetValues ( status_, XmNlabelString, string, NULL );
    XmStringFree ( string );
    }


void RubberBand::track_rb_EV ( Widget w,
                               XtPointer   clientData,
                               XEvent      *event,
                               Boolean     * )
    {
    RubberBand *obj = ( RubberBand * ) clientData;
    obj->track_rb ( w, event );
    }

void RubberBand::drawMyRubberBand ( Widget w )
    {
    int  minx = MIN ( startx_, lastx_ ),
         maxx = MAX ( startx_, lastx_ ),
         miny = MIN ( starty_, lasty_ ),
         maxy = MAX ( starty_, lasty_ );

    XDrawRectangle ( XtDisplay ( w ), XtWindow ( w ), xorgc_,
                     minx, miny, maxx-minx, maxy-miny );
    }

void RubberBand::track_rb ( Widget w, XEvent *event )
    {
    char str[100];
    XmString string;
    float x, y;

    x = s_->gridx ( event->xbutton.x );
    y = s_->gridy ( event->xbutton.y );

    // Erase the previous figure
    drawMyRubberBand ( w ); // track_rb erase previous

    str[0] = '\0';
    if ( ( x >= zoom_[curr_zoom_].GRID_X_MIN_ ) && ( x <= zoom_[curr_zoom_].GRID_X_MAX_+1 ) &&
            ( y >= zoom_[curr_zoom_].GRID_Y_MIN_ ) && ( y <= zoom_[curr_zoom_].GRID_Y_MAX_+1 ) )
        {

        lastx_ = event->xbutton.x;
        lasty_ = event->xbutton.y;

        sprintf ( str, "Cell Range (%d,%d)->(%d,%d)",
                  ( int ) ceil ( ( double ) MIN ( ( s_->gridx ( startx_ ) ), ( s_->gridx ( lastx_ ) ) ) ),
                  ( int ) ceil ( ( double ) MIN ( ( s_->gridy ( starty_ ) ), ( s_->gridy ( lasty_ ) ) ) ),
                  ( int ) ceil ( ( double ) MAX ( ( s_->gridx ( startx_ ) ), ( s_->gridx ( lastx_ ) ) ) ),
                  ( int ) ceil ( ( double ) MAX ( ( s_->gridy ( starty_ ) ), ( s_->gridy ( lasty_ ) ) ) ) );

        drawMyRubberBand ( w ); // track_rb draw next
        }
    else
        {
        XBell ( XtDisplay ( w ), 2 );
        sprintf ( str, "Cursor out of bounds" );
        if ( interact_mode_ == PROBE_MODE )
            {
            probe_close_cb();
            badDrag = 1;
            }
        }

    // Display message on status widget
    string = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( str );
    XtVaSetValues ( status_, XmNlabelString, string, NULL );
    XmStringFree ( string );
    }



void RubberBand::end_rb_EV ( Widget   w,
                             XtPointer   clientData,
                             XEvent      *event,
                             Boolean     * )
    {
    RubberBand *obj = ( RubberBand * ) clientData;
    obj->end_rb ( w, event );
    }


void RubberBand::adjustZoomDialogPosition ( void ) // added 950913 SRT
    {
    }


void RubberBand::end_rb ( Widget w, XEvent * )
    {
    int t;

    // Erase the previous figure
    drawMyRubberBand ( w ); // end_rb erase previous added 951019 SRT

    if ( lastx_ < startx_ ) // added 951018 SRT
        {
        t = lastx_;
        lastx_ = startx_;
        startx_ = t;
        }
    if ( lasty_ < starty_ ) // added 951018 SRT
        {
        t = lasty_;
        lasty_ = starty_;
        starty_ = t;
        }


    if ( interact_mode_ == ZOOM_MODE )
        {
        if ( zoom_dialog_ == NULL )
            {
            createZoomDialog();
            adjustZoomDialogPosition();
            }
        if ( XtIsManaged ( zoom_dialog_ ) ) XtUnmanageChild ( zoom_dialog_ );
        XtManageChild ( zoom_dialog_ );

        curr_zoom_ = num_zooms_;
        zoomin();
        }
    else if ( interact_mode_ == PROBE_MODE )
        {
        if ( interact_submode_ == PROBE_TILE )
            {
            writeProbeFile ( s_->gridx ( startx_ ), s_->gridx ( lastx_ ),
                             s_->gridy ( starty_ ),  s_->gridy ( lasty_ ) );

            }
        else
            {
            writeProbeObsFile ( startx_, lastx_, starty_,  lasty_ );
            }
        if ( probe_dialog_ == NULL )
            createProbeDialog();
        XtVaSetValues ( probe_text_,
                        XmNvalue, file2string ( probefilename_ ),
                        NULL );
        if ( !badDrag )
            {
            if ( XtIsManaged ( probe_dialog_ ) ) XtUnmanageChild ( probe_dialog_ );
            XtManageChild ( probe_dialog_ );
            }
        badDrag = 0;

        char command[100];
        sprintf ( command, "rm -f %s\n", probefilename_ );
        system ( command );
        }
    else if ( interact_mode_ == TIME_SERIES_MODE )
        {
        if ( interact_submode_ == TIME_SERIES_OBS_MODE )
            {
            overlay_ts ( startx_, lastx_, starty_,  lasty_ );
            }
        else
            {
            timeSeriesProbe ( ( int ) s_->gridx ( startx_ ), ( int ) s_->gridx ( lastx_ ),
                              ( int ) s_->gridy ( starty_ ), ( int ) s_->gridy ( lasty_ ) );
            }


        }
    else   // MARKUP_MODE
        {
        writeProbeFile ( s_->gridx ( startx_ ), s_->gridx ( lastx_ ),
                         s_->gridy ( starty_ ),  s_->gridy ( lasty_ ) );
        }
    }


void RubberBand::timeSeriesProbe ( int x1, int x2, int y1, int y2 )
    {
    fprintf ( stderr, "Enter RubberBand::timeSeriesProbe(%d, %d, %d, %d)\n",
              x1, x2, y1, y2 );
    }


void RubberBand::createProbeDialog()
    {
    Widget probe_text_parent;
    Arg args[10];
    int n;

    assert ( parent_widget_ );
    probe_dialog_ = XmCreateFormDialog ( parent_widget_,
                                         "Data Viewer",
                                         NULL, 0 );

    n=0;
    XtSetArg ( args[n], XmNrows,24 );
    n++;
    XtSetArg ( args[n], XmNcolumns,80 );
    n++;
    XtSetArg ( args[n], XmNeditMode, XmMULTI_LINE_EDIT );
    n++;
    XtSetArg ( args[n], XmNeditable, False );
    n++;
    XtSetArg ( args[n], XmNwordWrap, True );
    n++;

    // probe_text_ is the text, & is a child of an XmScrolledWindow widget
    probe_text_ = XmCreateScrolledText ( probe_dialog_, "text", args, n );

    // the XmScrolledWindow Widget is the parent of the probe_text_ widget
    probe_text_parent = XtParent ( probe_text_ );

    XtVaSetValues ( probe_text_parent,
                    XmNtopAttachment,       XmATTACH_FORM,
                    XmNbottomAttachment,    XmATTACH_FORM,
                    XmNleftAttachment,      XmATTACH_FORM,
                    XmNrightAttachment,     XmATTACH_FORM,
                    XmNtopOffset,           10,
                    XmNbottomOffset,    10,
                    XmNleftOffset,          10,
                    XmNrightOffset,         10,
                    NULL );
    if ( XtIsManaged ( probe_text_ ) ) XtUnmanageChild ( probe_text_ );
    XtManageChild ( probe_text_ );

    }


void RubberBand::createZoomDialog()
    {
    assert ( parent_widget_ );

    // Create control dialog box.
    zoom_dialog_ = XmCreateFormDialog ( parent_widget_, "control", NULL, 0 );
    XtVaSetValues ( zoom_dialog_,
                    XmNautoUnmanage,        False,
                    NULL );

    Widget form1 = XtVaCreateWidget ( "form1",
                                      xmFormWidgetClass,      zoom_dialog_,
                                      XmNtopAttachment,       XmATTACH_FORM,
                                      XmNtopOffset,           10,
                                      XmNleftAttachment,      XmATTACH_FORM,
                                      XmNleftOffset,          10,
                                      XmNrightAttachment,     XmATTACH_FORM,
                                      XmNrightOffset,         10,
                                      NULL );

    zoom_scale_ = XtVaCreateManagedWidget ( "Zoom",
                                            xmScaleWidgetClass, form1,
                                            XtVaTypedArg, XmNtitleString, XmRString, "Current Zoom", 13,
                                            XmNvalue,               0,
                                            XmNwidth,       200,
                                            XmNorientation,         XmHORIZONTAL,
                                            XmNtopAttachment,       XmATTACH_FORM,
                                            XmNtopOffset,           10,
                                            XmNleftAttachment,      XmATTACH_FORM,
                                            XmNrightAttachment,     XmATTACH_FORM,
                                            XmNshowValue,           True,
                                            NULL );

    XtAddCallback ( zoom_scale_, XmNvalueChangedCallback,  &RubberBand::zoom_scaleCB, ( XtPointer ) this );

    if ( XtIsManaged ( form1 ) ) XtUnmanageChild ( form1 );
    XtManageChild ( form1 );

    Widget separator = XtVaCreateManagedWidget ( "sep",
                       xmSeparatorWidgetClass, zoom_dialog_,
                       XmNtopAttachment,       XmATTACH_WIDGET,
                       XmNtopWidget,           form1,
                       XmNtopOffset,           10,
                       XmNleftAttachment,      XmATTACH_FORM,
                       XmNrightAttachment,     XmATTACH_FORM,
                       NULL );

    close_ = XtVaCreateManagedWidget ( "Close",
                                       xmPushButtonWidgetClass, zoom_dialog_,
                                       XmNtopAttachment,       XmATTACH_WIDGET,
                                       XmNtopWidget,           separator,
                                       XmNtopOffset,           10,
                                       XmNrightAttachment,     XmATTACH_FORM,
                                       XmNrightOffset,         10,
                                       XmNwidth,               100,
                                       XmNheight,              40,
                                       XmNbottomAttachment,    XmATTACH_FORM,
                                       XmNbottomOffset,    10,
                                       NULL );
    XtAddCallback ( close_, XmNactivateCallback, &RubberBand::zoom_closeCB, ( XtPointer ) this );
    }



void RubberBand::zoom_scaleCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    RubberBand *obj = ( RubberBand * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->zoom_scale_cb ( cbs->value );
    }

void RubberBand::zoom_scale_cb ( int value )
    {
    if ( value <= num_zooms_ )
        {
        curr_zoom_ = value;

        resize();
        }
    }


void RubberBand::zoom_closeCB ( Widget, XtPointer clientData, XtPointer )
    {
    RubberBand *obj = ( RubberBand * ) clientData;
    obj->zoom_close_cb();
    }


void RubberBand::zoom_close_cb()
    {
    XtUnmanageChild ( zoom_dialog_ );
    }


void RubberBand::probe_closeCB ( Widget, XtPointer clientData, XtPointer )
    {
    RubberBand *obj = ( RubberBand * ) clientData;
    obj->probe_close_cb();
    }


void RubberBand::probe_close_cb()
    {
    if ( probe_dialog_ != NULL ) XtUnmanageChild ( probe_dialog_ );
    }



GC RubberBand::createXorGC ( Widget w )
    {
    XGCValues values;
    GC        gc;
    Arg       wargs[10];

    /// Get the colors used by the widget.
    XtSetArg ( wargs[0], XtNforeground, &values.foreground );
    XtSetArg ( wargs[1], XtNbackground, &values.background );
    XtGetValues ( w, wargs, 2 );

    //
    // Set the fg to the XOR of the fg and bg, so if it is XOR'ed with
    // bg, the result will be fg and vice-versa.  This effectively
    // achieves inverse video for the line.
    //
    values.foreground = values.foreground ^ values.background;

    //
    // Set the rubber band gc to use XOR mode and draw a dash line.
    //
    values.line_style = LineOnOffDash;
    values.function = GXxor;
    gc = XtGetGC ( w, GCForeground | GCBackground |
                   GCFunction   | GCLineStyle, &values );
    return gc;
    }



