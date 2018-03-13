/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: DrawWnd.cc 83 2018-03-12 19:24:33Z coats $
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
//////////////////////////////////////////////////////////////////////////////
// File:    DrawWnd.C
// Author:  K. Eng Pua
// Date:    Dec 12, 1994
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include "DrawWnd.h"


DrawWnd::DrawWnd ( AppInit *app, char *name, char *drawtype,
                   Dimension width, Dimension height,
                   char **colornames, int numcolornames, int bar_plot ) :
    Shell ( app, name )
    {
    app_ = app;
    width_ = width;
    height_ = height;
    drawtype_ = strdup ( drawtype );
    colornames_ = colornames;
    numcolornames_ = numcolornames;
    pix_ = ( Pixmap ) NULL; // 961001 SRT
    bar_plot_ = bar_plot;

    draw_ = XtVaCreateManagedWidget ( "draw_window",
                                      xmFormWidgetClass,  shell_,
                                      XmNminWidth,        ( int ) ( width_/2 ), // added 951019 SRT
                                      XmNminHeight,       ( int ) ( height_/2 ), // added 951019 SRT
                                      XmNminAspectX,      ( int ) width_, // added 951019 SRT
                                      XmNmaxAspectX,      ( int ) width_, // added 951019 SRT
                                      XmNminAspectY,      ( int ) height_, // added 951019 SRT
                                      XmNmaxAspectY,      ( int ) height_, // added 951019 SRT
                                      NULL );

    //   installDestroyHandler();

    dpy_ = XtDisplay ( draw_ );

    gc_  = XCreateGC ( XtDisplay ( draw_ ),
                       DefaultRootWindow ( XtDisplay ( draw_ ) ),
                       ( unsigned long ) NULL, ( XGCValues * ) NULL );

    def_font_ = XLoadQueryFont ( XtDisplay ( draw_ ), DEFAULT_FONT );
    title_font_ = XLoadQueryFont ( XtDisplay ( draw_ ), TITLE_FONT );
    subtitle_font_ = XLoadQueryFont ( XtDisplay ( draw_ ), DEFAULT_FONT );
    }


DrawWnd::~DrawWnd() // added 960426 SRT
    {
    if ( drawtype_ ) free ( drawtype_ );
    drawtype_ = NULL;
    }


void DrawWnd::drawSetup ( Widget draw_area, Drawable drw, GC gc, DrawScale *s )
    {
    draw_area_ = draw_area;
    dpy_ = XtDisplay ( draw_area_ );
    drw_ = drw;
    gc_  = gc;
    s_   = s;

    // Set default font
    if ( def_font_ )
        XSetFont ( dpy_, gc_, def_font_->fid );
    }



int DrawWnd::getNamedPixel ( char *colorname )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter DrawWnd::getNamedPixel()\n" );
#endif // DIAGNOSTICS

    int       scr  = DefaultScreen ( dpy_ );
    Colormap  cmap = DefaultColormap ( dpy_, scr );
    XColor    color, ignore;

    if ( XAllocNamedColor ( dpy_, cmap, colorname, &color, &ignore ) )
        {

        return ( color.pixel );
        }
    else
        {
        return ( BlackPixel ( dpy_, scr ) );
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Exit DrawWnd::getNamedPixel()\n" );
#endif // DIAGNOSTICS
    }


void DrawWnd::drawTitles ( char *title1, char *title2, char *title3, char *xtitle, char *ytitle )
    {
    // Show title1
    if ( title1 )
        drawTitle ( title_font_, 40, title1 );

    // Show title2
    if ( title2 )
        drawTitle ( def_font_, 70, title2 );

    // Show title3
    if ( title3 )
        drawTitle ( subtitle_font_, 85, title3 ); // added 950911 SRT

    // Show X-title
    if ( xtitle )
        drawTitle ( def_font_, s_->scaley ( s_->ymin_ )+50, xtitle );

    // Show Y-title
    if ( ytitle )
        drawTitleY ( def_font_, 10, ytitle );
    }


void DrawWnd::drawFrame()
    {
    // char strg[80];

    XRectangle rects[4];

    // Clear the margins

    rects[0].x = rects[0].y = 0;
    rects[0].width  = s_->scalex ( s_->xmin_ );
    rects[0].height = height_;

    rects[1].x = rects[1].y = 0;
    rects[1].width  = width_;
    rects[1].height = s_->scaley ( s_->ymax_ );

    rects[2].x = s_->scalex ( s_->xmax_ );
    rects[2].y =  0;
    rects[2].width  = width_-s_->scalex ( s_->xmax_ );
    rects[2].height = height_;

    rects[3].x = 0;
    rects[3].y = s_->scaley ( s_->ymin_ );
    rects[3].width  = width_;
    rects[3].height = height_-s_->scaley ( s_->ymin_ );

    XSetForeground ( dpy_, gc_, getNamedPixel ( "white" ) );
    XFillRectangles ( dpy_, drw_, gc_, rects, 4 );

    // Reset foreground color
    XSetForeground ( dpy_, gc_,
                     BlackPixel ( dpy_,
                                  DefaultScreen ( dpy_ ) ) );

    XDrawRectangle ( dpy_,
                     drw_,
                     gc_,
                     s_->scalex ( s_->xmin_ ),s_->scaley ( s_->ymax_ ),
                     s_->scalex ( s_->xmax_ )-s_->scalex ( s_->xmin_ ),
                     s_->scaley ( s_->ymin_ )-s_->scaley ( s_->ymax_ ) );

    //if (getenv("NO_AUTHOR_STRING")==NULL) draw_mcnc_text();  // 961014 added SRT
    }


void DrawWnd::drawYtics ( int num_tics, char *formatY )
    {
    drawYtics ( num_tics, formatY, 0, 0 );
    }


#ifdef _AIX // this is a TOTAL HACK because IBMs 
// can't deal with sprintf format characters properly) - SRT

void DrawWnd::drawYtics ( int num_tics, char *formatY, int draw_on_right, int barwnd )
    {
    float    val, step;
    char     strg[80];
    int ival, first_time = 1;

    // Show Y coordinates

    step = ( s_->ymax_-s_->ymin_ ) / ( float ) num_tics;

    val = s_->ymin_;
    do
        {
        if ( val >= s_->ymin_ && val <= s_->ymax_ )
            {
            // if (val == s_->ymin_)        // SRT 950707 HACK as Eng printed 1 too many
            // sprintf(strg, "%d" /*SRT 960718 formatY (float) */ , (int)s_->ymin_+1); // SRT 950707 HACK
            // ival = s_->ymin_+1;
            if ( ( first_time ) && ( !barwnd ) )
                {
                first_time=0;
                sprintf ( strg, "%.0f", val+1.0 );
                }
            else                // SRT 950707 HACK
                // sprintf(strg, "%d" /*SRT 960718 formatY */, (int)val);
                // ival = val;
                sprintf ( strg, "%.0f", val );

            // SRT 950707   XDrawLine( dpy_, drw_, gc_,
            // SRT 950707              s_->scalex(s_->xmin_),s_->scaley(val),
            // SRT 950707              s_->scalex(s_->xmin_)-6,s_->scaley(val) );

            // Want to change the following function so that x is right-justified
            //  i.e. as close to the left edge of plot area as possible
            int textwidth_ = strlen ( strg ) *6;
            if ( def_font_ )
                textwidth_ = XTextWidth ( def_font_, strg, strlen ( strg ) );

            XDrawString ( dpy_, drw_, gc_,
                          s_->scalex ( s_->xmin_ )-textwidth_-8, s_->scaley ( val )+6,
                          strg,
                          strlen ( strg ) );
            //  fprintf(stderr, "XXX DrawWnd::drawYtics '%s' with val == %f and ival == %d\n", strg, val, ival);
            }
        val+=step;
        }
    while ( val <= s_->ymax_ );

    }

void DrawWnd::drawXtics ( int num_tics, char *formatX )
    {
    int ival, first_time = 1;
    float    val, step;
    char     strg[80];


    // Show X coordinates
    step = ( s_->xmax_-s_->xmin_ ) / ( float ) num_tics;

    val = s_->xmin_;
    do
        {
        if ( val >= s_->xmin_ && val <= s_->xmax_ )
            {

            // if (val == s_->xmin_) // SRT 950707 HACK as Eng printed 1 too many
            // sprintf(strg, "%d" /*SRT 960718 formatX (float) */ , (int)s_->xmin_+1); // SRT 950707 HACK
            // ival = s_->xmin_+1;
            if ( first_time )
                {
                sprintf ( strg, "%.0f", val+1.0 );
                first_time=0;
                }
            else                            // SRT 950707 HACK
                // sprintf(strg, "%d" /*SRT 960718 formatX */, (int) val);
                // ival = val;
                sprintf ( strg, "%.0f", val );

            // SRT 950707   XDrawLine( dpy_, drw_, gc_,
            // SRT 950707              s_->scalex(val),s_->scaley(s_->ymin_),
            // SRT 950707              s_->scalex(val),s_->scaley(s_->ymin_)+6 );

            XDrawString ( dpy_, drw_, gc_,
                          s_->scalex ( val ),s_->scaley ( s_->ymin_ )+20,
                          strg,
                          strlen ( strg ) );
            // fprintf(stderr, "XXX DrawWnd::drawXtics '%s' with val == %f and ival == %d\n", strg, val, ival);
            strg[0]='\0';
            }
        val+=step;
        }
    while ( val <= s_->xmax_ );
    }

#else // #ifdef _AIX

void DrawWnd::drawYtics ( int num_tics, char *formatY, int draw_on_right, int barwnd )
    {
    float    val, step;
    char     strg[80];

    // Show Y coordinates

    step = ( s_->ymax_-s_->ymin_ ) / ( float ) num_tics;

    val = s_->ymin_;
    do
        {
        if ( val >= s_->ymin_ && val <= s_->ymax_ )
            {
            if ( ( val == s_->ymin_ ) && ( !barwnd ) ) // SRT 950707 HACK as Eng printed 1 too many
                sprintf ( strg, formatY, ( float ) s_->ymin_+1 ); // SRT 950707 HACK
            else                // SRT 950707 HACK
                sprintf ( strg, formatY, val );

            // SRT 950707   XDrawLine( dpy_, drw_, gc_,
            // SRT 950707              s_->scalex(s_->xmin_),s_->scaley(val),
            // SRT 950707              s_->scalex(s_->xmin_)-6,s_->scaley(val) );

            // Want to change the following function so that x is right-justified
            //  i.e. as close to the left edge of plot area as possible
            int textwidth_ = strlen ( strg ) *6;
            if ( def_font_ )
                textwidth_ = XTextWidth ( def_font_, strg, strlen ( strg ) );

            XDrawString ( dpy_, drw_, gc_,
                          draw_on_right ? s_->scalex ( s_->xmax_ )+8 : s_->scalex ( s_->xmin_ )-textwidth_-8,
                          s_->scaley ( val )+6,
                          strg,
                          strlen ( strg ) );
            }
        val+=step;
        }
    while ( val <= s_->ymax_ );

    }

void DrawWnd::drawXtics ( int num_tics, char *formatX )
    {
    float    val, step;
    char     strg[80];


    // Show X coordinates
    step = ( s_->xmax_-s_->xmin_ ) / ( float ) num_tics;

    val = s_->xmin_;
    do
        {
        if ( val >= s_->xmin_ && val <= s_->xmax_ )
            {

            if ( val == s_->xmin_ )         // SRT 950707 HACK as Eng printed 1 too many
                sprintf ( strg, formatX, ( float ) s_->xmin_+1 ); // SRT 950707 HACK
            else                            // SRT 950707 HACK
                sprintf ( strg, formatX, val );

            // SRT 950707   XDrawLine( dpy_, drw_, gc_,
            // SRT 950707              s_->scalex(val),s_->scaley(s_->ymin_),
            // SRT 950707              s_->scalex(val),s_->scaley(s_->ymin_)+6 );

            XDrawString ( dpy_, drw_, gc_,
                          s_->scalex ( val ),s_->scaley ( s_->ymin_ )+20,
                          strg,
                          strlen ( strg ) );
            strg[0]='\0';
            }
        val+=step;
        }
    while ( val <= s_->xmax_ );
    }


#endif // #ifdef _AIX

void DrawWnd::drawTimeStamp ( int t )
    {
    char buf[30];

    XSetBackground ( dpy_, gc_,
                     WhitePixelOfScreen ( XtScreen ( draw_area_ ) ) );
    sprintf ( buf, "Hour: %02d", t );
    //   XDrawImageString(dpy_, drw_, gc_, width_/2, height_-20, buf, strlen(buf));
    drawTitle ( def_font_, height_-20, buf );
    }


void DrawWnd::drawTimeStamp ( int currAnimate, int sdate, int stime, char *tz )
    {
    int len;

    if ( sdate != 0 )
        {
        char buf[128];
        int stringWidth;

        XSetBackground ( dpy_, gc_,
                         WhitePixelOfScreen ( XtScreen ( draw_area_ ) ) );
        julian2text ( buf, sdate, stime );

        if ( tz )
            {
            len = strlen ( buf );
            sprintf ( buf+len," (%s)",tz );
            }
        // Want to change the following function so that string is centered
        if ( def_font_ )
            stringWidth = XTextWidth ( def_font_, buf, strlen ( buf ) );
        else
            stringWidth = strlen ( buf ) *6;

        // fprintf(stderr, "DrawWnd.cc Just before XDrawImageString\n"); fflush(stderr);

        // width_ - 100 left - 60 right = tiles width
        //XDrawImageString(dpy_, drw_, gc_,
        //        100+(width_-160)/2-stringWidth/2,
        //    height_-20, buf, strlen(buf));
        drawTitle ( def_font_, height_-20, buf );

        // fprintf(stderr, "DrawWnd.cc Just after XDrawImageString\n"); fflush(stderr);
        }
    else
        drawTimeStamp ( currAnimate );
    }


void DrawWnd::drawTitle ( XFontStruct *fontstruct, int offset, char *title )
    {
    Dimension width;

    XtVaGetValues ( draw_area_,
                    XmNwidth, &width,
                    NULL );

    int textwidth_ = strlen ( title ) *4;
    if ( fontstruct )
        {
        XSetFont ( dpy_, gc_, fontstruct->fid );
        textwidth_ = XTextWidth ( fontstruct, title, strlen ( title ) ) / 2;
        }

    XDrawString ( dpy_,
                  drw_,
                  gc_,
                  ( int ) ( ( s_->scalex ( s_->xmin_ )+s_->scalex ( s_->xmax_ ) ) /2-textwidth_ ),
                  // SRT 961018 replaced by above line width/2 - textwidth_,
                  offset,
                  title,
                  strlen ( title ) );

    // Reset font
    if ( def_font_ )
        XSetFont ( dpy_, gc_, def_font_->fid );

    }



void DrawWnd::drawTitleY ( XFontStruct *fontstruct, int offset, char *title )
    {
    int junk1, ascent, descent;
    XCharStruct overall;
    Dimension height;
    int title_len = strlen ( title );

    XtVaGetValues ( draw_area_,
                    XmNheight, &height,
                    NULL );


    int Hheight_ = 12;
    // Get the height of character 'H'
    if ( fontstruct )
        {
        XTextExtents ( fontstruct, "H", 0, &junk1, &ascent, &descent, &overall );
        Hheight_ = ascent+descent;
        }

    int textheight_ = Hheight_*title_len/2;

    for ( int i=0; i<title_len; i++ )
        {
        XDrawString ( dpy_,
                      drw_,
                      gc_,
                      offset,
                      height/2 - textheight_ + Hheight_*i,
                      title+i,
                      1 );
        }
    }


void DrawWnd::setForeground ( char *color )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter DrawWnd::setForeground()\n" );
#endif // DIAGNOSTICS

    XSetForeground ( dpy_,
                     gc_,
                     getNamedPixel ( color ) );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Exit DrawWnd::setForeground()\n" );
#endif // DIAGNOSTICS
    }

void DrawWnd::setBackground ( char *color )
    {
    XSetBackground ( dpy_,
                     gc_,
                     getNamedPixel ( color ) );

    }


void DrawWnd::draw_mcnc_text()   // 961015 added SRT
    {
    static char *label = "PAVE" ;
    XFontStruct *mcnc_font;
    int mcnc_font_size, i, textwidth;
    Dimension height;

    // try helvetica medium weight italicized at 8 point
    if ( mcnc_font = XLoadQueryFont ( dpy_, "*-helvetica-medium-i-*-80-*" ) )
        mcnc_font_size = 8;
    else            // try helvetica medium weight roman at 8 point
        if ( mcnc_font = XLoadQueryFont ( dpy_, "*-helvetica-medium-r-*-80-*" ) )
            mcnc_font_size = 8;
        else            // try helvetica medium weight italicized at 10 point
            if ( mcnc_font = XLoadQueryFont ( dpy_, "*-helvetica-medium-i-*-100-*" ) )
                mcnc_font_size = 10;
            else            // try helvetica medium weight roman at 10 point
                if ( mcnc_font = XLoadQueryFont ( dpy_, "*-helvetica-medium-r-*-100-*" ) )
                    mcnc_font_size = 10;
                else            // use the def_font_
                    {
                    mcnc_font = def_font_;
                    mcnc_font_size = 12;
                    }

    XtVaGetValues ( draw_area_, XmNheight, &height, NULL );
    XSetFont ( dpy_, gc_, mcnc_font->fid );
    textwidth = XTextWidth ( mcnc_font, label, strlen ( label ) ) / 2;
    XDrawString ( dpy_, drw_, gc_,
                  ( int ) ( 25-textwidth ),
                  ( int ) ( height-mcnc_font_size/2- ( mcnc_font_size+3 ) * ( 2-i ) ),
                  label, strlen ( label ) );
    if ( def_font_ ) XSetFont ( dpy_, gc_, def_font_->fid );
    }

int DrawWnd::newTitleFontSize ( int size )
    {
    XFontStruct *title_font;
    char *font_name = strdup ( TITLE_FONT );
    if ( ! font_name )
        {
        fprintf ( stderr,"ERROR: Cannot allocate font name for size %d\n", size );
        return 0;
        }

    if ( size > 99 )
        {
        fprintf ( stderr,"ERROR: Font size too large(%d)\n", size );
        return 0;
        }
    sprintf ( font_name+17,"%d%s",size,"-*-*-*-*-*-*-*" );
    title_font = XLoadQueryFont ( XtDisplay ( draw_ ), font_name );
    free ( font_name );
    if ( title_font == NULL )
        {
        fprintf ( stderr,"ERROR: Cannot find font \"%s\"\n", font_name );
        return 0;
        }

    XFreeFont ( XtDisplay ( draw_ ), title_font_ );
    title_font_ = title_font;
    return 1;
    }

int DrawWnd::newSubTitleFontSize ( int size )
    {
    XFontStruct *subtitle_font;
    char *font_name = strdup ( DEFAULT_FONT );
    if ( ! font_name )
        {
        fprintf ( stderr,"ERROR: Cannot allocate font name for size %d\n", size );
        return 0;
        }

    if ( size > 99 )
        {
        fprintf ( stderr,"ERROR: Font size too large(%d)\n", size );
        return 0;
        }
    sprintf ( font_name+21,"%d%s",size,"-*-*-*-*-*-*-*" );
    subtitle_font = XLoadQueryFont ( XtDisplay ( draw_ ), font_name );
    free ( font_name );
    if ( subtitle_font == NULL )
        {
        fprintf ( stderr,"ERROR: Cannot find font \"%s\"\n", font_name );
        return 0;
        }

    XFreeFont ( XtDisplay ( draw_ ), subtitle_font_ );
    subtitle_font_ = subtitle_font;
    return 1;
    }
