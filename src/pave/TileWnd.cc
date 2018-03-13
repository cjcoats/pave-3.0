/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: TileWnd.cc 83 2018-03-12 19:24:33Z coats $
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
// File:    TileWnd.CC
// Author:  K. Eng Pua
// Date:    Dec 3, 1994
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//  Modification history:
//
//  950616 SRT Fixed how indexing into vis_->grid is done
//  950630 SRT Added disabled map feature
//  951130 SRT Added stuff for vector drawing
//  951212 SRT Added drawGridLines(), toggleGridLines(), & toggleScaleVectors()
//  951218 SRT Added logic for slowing down animations if necessary
//  960522 SRT Began adding code to incorporate Suresh Balu's animation export
//  960529 SRT Added callbacks for map setting routines
//  960530 SRT Added logic for saving RGB, XWD, PNG, and GIF Images
// 2018058 CJC Version for PAVE-3.0.  Major grid-loop reorganization.
//////////////////////////////////////////////////////////////////////////////

/** NOTE: all drawing is done in drawDetail */

/* bald messes this up in some header file */

#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */

#include "nan_incl.h"

#include "TileWnd.h"

#include "iodecl3.h"

#define OBS_SIZE 4
#define OBS_THICK 1


TileWnd::TileWnd (   Config *cfgp, void *dwnd, AppInit *app, char *name, BusData *ibd,
                     VIS_DATA *vdata, char *drawtype,
                     char *title2,  // Species/Formula String
                     char *title3,  // Dataset (file) name(s) string
                     Dimension width, Dimension height,
                     int *frameDelayInTenthsOfSeconds,
                     int exit_button_on,
                     char **colornames, int numcolornames ) :
    DrawWnd ( app, name, drawtype, width, height, colornames, numcolornames, 0 ),
    Menus(),
    RubberBand(),
    ColorLegend(),
    SaveMPEGBrowser_ ( draw_, ( char * ) "MPEG", ( char * ) NULL,
                       ( void * ) &saveMPEGCB, NULL, ( void * ) this ),

    SaveAnimatedGIFBrowser_ ( draw_, ( char * ) "AnimGIF", ( char * ) NULL,
                              ( void * ) &saveAnimatedGIFCB, NULL, ( void * ) this ),

    SavePNGBrowser_ ( draw_, ( char * ) "PNG", ( char * ) NULL,
                      ( void * ) &savePNGCB, NULL, ( void * ) this ),

    SaveGIFBrowser_ ( draw_, ( char * ) "GIF", ( char * ) NULL,
                      ( void * ) &saveGIFCB, NULL, ( void * ) this ),

    SaveConfigBrowser_ ( draw_, ( char * ) "Config", ( char * ) NULL,
                         ( void * ) &saveConfigCB, NULL, ( void * ) this ),

    SaveRGBBrowser_ ( draw_, ( char * ) "RGB", ( char * ) NULL,
                      ( void * ) &saveRGBCB, NULL, ( void * ) this ),

    SaveXWDBrowser_ ( draw_, ( char * ) "XWD", ( char * ) NULL,
                      ( void * ) &saveXWDCB, NULL, ( void * ) this ),

    SavePSBrowser_ ( draw_, ( char * ) "PostScript", ( char * ) NULL,
                     ( void * ) &savePSCB, NULL, ( void * ) this )
    {
    int mpc;
    char *mp;
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::TileWnd() 1\n" );
    dump_VIS_DATA ( vdata, NULL, NULL );
    fprintf ( stderr, "Enter TileWnd::TileWnd() with width %d and height %d\n",
              width, height );
#endif // DIAGNOSTICS


    assert ( drawtype );
    work_proc_id_ = ( XtWorkProcId ) NULL; // 961018 added SRT
    dwnd_ = dwnd;
    vis_ = ( ReadVisData * ) NULL;   // 960918 added SRT for memory management
    uwind_ = ( VIS_DATA * ) NULL;    // 960918 added SRT for memory management
    vwind_ = ( VIS_DATA * ) NULL;    // 960918 added SRT for memory management
    canvas_ = ( Widget ) NULL;   // 960918 added SRT for memory management
    mappix1_ = ( Pixmap ) NULL;      // 960918 added SRT for memory management
    mappix2_ = ( Pixmap ) NULL;      // 960918 added SRT for memory management
    cfgp_ = cfgp;
    initColorLegendObject();
    uwind_ = vwind_ = NULL;
    exit_button_on_ = exit_button_on;
    colornames_ = colornames;
    numcolornames_ = numcolornames;
    vis_ = new ReadVisData ( drawtype, ibd, vdata );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "In TileWnd::TileWnd()\n" );
    fprintf ( stderr, "with step_min == %d\n", vis_->step_min_ );
    fprintf ( stderr, "with step_max == %d\n", vis_->step_max_ );
#endif // DIAGNOSTICS

    vis_->title1_ = strdup ( title2 ); // Species/Fomula String
    vis_->title3_ = strdup ( title3 ); // Dataset (file) name(s) string
    frameDelayInTenthsOfSecondsP_ = frameDelayInTenthsOfSeconds;
    selected_step_ = 0;
    if ( vdata && vdata->selected_step > 0 ) selected_step_=vdata->selected_step-1;
    initTileWnd();
    tiles_on_ = 1;
    vectors_on_ = 0;
    overlay_ = 0;
    obs_dialog_ = cntr_dialog_ = NULL;
    draw_dist_counties_ = 0;
    if ( getenv ( "PAVE_DISTINCT_STATE_COUNTIES" ) != NULL ) draw_dist_counties_ = 1;
    mapChoices_ = MapDefault;
    if ( ( mp=getenv ( "MAPCHOICE" ) ) != NULL )
        {
        mpc = atoi ( mp );
        if ( mpc == 1 )
            {
            OUTLCO_cb ( 0 );
            }
        else if ( mpc == 2 )
            {
            OUTLUSAM_cb();
            }
        else if ( mpc == 3 )
            {
            OUTLSTATES3000_cb ( 0 );
            }
        else if ( mpc == 4 )
            {
            OUTLRIVERS3000_cb();
            }
        else if ( mpc == 5 )
            {
            OUTLROADS3000_cb();
            }
        else if ( mpc == 6 )
            {
            OUTLHRES_cb();
            }
        else
            {
            fprintf ( stderr,
                      "PAVE Warning: env. var. MAPCHOICE has illegal value: %s\n",
                      mp );
            }
        }
    createUI ( draw_ );

    initRubberBand ( &s, canvas_ );

    initColorLegend ( canvas_, pix_, gc_, &s,
                      ( cfgp_->hasLegend_Min() ) ? cfgp_->getLegend_Min() : vis_->info->grid_min,
                      ( cfgp_->hasLegend_Max() ) ? cfgp_->getLegend_Max() : vis_->info->grid_max,
                      vis_->getUnits(), &vis_->title1_, &vis_->title2_, &vis_->title3_ );

    manage();
    manage_ = 1;
    setButtonSensitiviy();

    val_xytdelta_ = ( vis_->info->grid_max - vis_->info->grid_min ) / ( float ) numcolornames_;
    plotDataListP_ = new linkedList;
    current_pdata_obs_config_ = current_pdata_cntr_config_ = NULL;


    }


TileWnd::TileWnd (   Config *cfgp, void *dwnd, AppInit *app, char *name, ReadVisData *vis,
                     char *drawtype,
                     char *title2,  // Species/Fomula String
                     char *title3,  // Dataset (file) name(s) string
                     Dimension width, Dimension height,
                     int *frameDelayInTenthsOfSeconds,
                     int exit_button_on,
                     char **colornames, int numcolornames ) :
    DrawWnd ( app, name, drawtype, width, height, colornames, numcolornames, 0 ),
    Menus(),
    RubberBand(),
    ColorLegend(),
    SaveMPEGBrowser_ ( draw_, ( char * ) "MPEG", ( char * ) NULL,
                       ( void * ) &saveMPEGCB, NULL, ( void * ) this ),

    SaveAnimatedGIFBrowser_ ( draw_, ( char * ) "AnimGIF", ( char * ) NULL,
                              ( void * ) &saveAnimatedGIFCB, NULL, ( void * ) this ),

    SavePNGBrowser_ ( draw_, ( char * ) "PNG", ( char * ) NULL,
                      ( void * ) &savePNGCB, NULL, ( void * ) this ),

    SaveGIFBrowser_ ( draw_, ( char * ) "GIF", ( char * ) NULL,
                      ( void * ) &saveGIFCB, NULL, ( void * ) this ),

    SaveConfigBrowser_ ( draw_, ( char * ) "Config", ( char * ) NULL,
                         ( void * ) &saveConfigCB, NULL, ( void * ) this ),

    SaveRGBBrowser_ ( draw_, ( char * ) "RGB", ( char * ) NULL,
                      ( void * ) &saveRGBCB, NULL, ( void * ) this ),

    SaveXWDBrowser_ ( draw_, ( char * ) "XWD", ( char * ) NULL,
                      ( void * ) &saveXWDCB, NULL, ( void * ) this ),

    SavePSBrowser_ ( draw_, ( char * ) "PostScript", ( char * ) NULL,
                     ( void * ) &savePSCB, NULL, ( void * ) this )
    {
    assert ( drawtype );
    work_proc_id_ = ( XtWorkProcId ) NULL; // 961018 added SRT
    dwnd_ = dwnd;
    vis_ = ( ReadVisData * ) NULL;   // 960918 added SRT for memory management
    uwind_ = ( VIS_DATA * ) NULL;    // 960918 added SRT for memory management
    vwind_ = ( VIS_DATA * ) NULL;    // 960918 added SRT for memory management
    canvas_ = ( Widget ) NULL;       // 960918 added SRT for memory management
    mappix1_ = ( Pixmap ) NULL;      // 960918 added SRT for memory management
    mappix2_ = ( Pixmap ) NULL;      // 960918 added SRT for memory management
    cfgp_ = cfgp;
    initColorLegendObject();
    uwind_ = vwind_ = NULL;

    vis_ = vis;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::TileWnd() 2\n" );
    fprintf ( stderr, "with step_min == %d\n", vis_->step_min_ );
    fprintf ( stderr, "with step_max == %d\n", vis_->step_max_ );
#endif // DIAGNOSTICS

    exit_button_on_ = exit_button_on;
    colornames_ = colornames;
    numcolornames_ = numcolornames;
    vis_->title1_ = strdup ( title2 );
    vis_->title3_ = strdup ( title3 ); // Dataset (file) name(s) string
    frameDelayInTenthsOfSecondsP_ = frameDelayInTenthsOfSeconds;

    selected_step_ = 0;
    //   if (vis && vis->selected_step > 0) selected_step_=vis->selected_step-1;

    initTileWnd();
    tiles_on_ = 1;
    vectors_on_ = 0;
    overlay_ = 0;
    obs_dialog_ = cntr_dialog_ = NULL;
    draw_dist_counties_ = 0;
    if ( getenv ( "PAVE_DISTINCT_STATE_COUNTIES" ) != NULL ) draw_dist_counties_ = 1;
    mapChoices_ = MapDefault;
    if ( getenv ( "MAPCHOICE" ) != NULL )
        {
        OUTLCO_cb ( 0 );
        }
    createUI ( draw_ );
    initRubberBand ( &s, canvas_ );
    initColorLegend ( canvas_, pix_, gc_, &s,
                      ( cfgp_->hasLegend_Min() ) ? cfgp_->getLegend_Min() : vis_->info->grid_min,
                      ( cfgp_->hasLegend_Max() ) ? cfgp_->getLegend_Max() : vis_->info->grid_max,
                      vis_->getUnits(), &vis_->title1_, &vis_->title2_, &vis_->title3_  );

    manage();
    manage_ = 1;
    setButtonSensitiviy();
    val_xytdelta_ = ( vis_->info->grid_max - vis_->info->grid_min ) / ( float ) numcolornames_;
    plotDataListP_ = new linkedList;
    current_pdata_obs_config_ = current_pdata_cntr_config_ = NULL;
    }


TileWnd::TileWnd (   Config *cfgp, void *dwnd, AppInit *app, char *name, BusData *ibd,
                     VIS_DATA *vdata,
                     VIS_DATA *uwind,
                     VIS_DATA *vwind,
                     char *drawtype,
                     char *title2,  // Species/Fomula String
                     char *title3,  // Dataset (file) name(s) string
                     Dimension width, Dimension height,
                     int *frameDelayInTenthsOfSeconds,
                     int exit_button_on,
                     char **colornames, int numcolornames ) :
    DrawWnd ( app, name, drawtype, width, height, colornames, numcolornames, 0 ),
    Menus(),
    RubberBand(),
    ColorLegend(),
    SaveMPEGBrowser_ ( draw_, ( char * ) "MPEG", ( char * ) NULL,
                       ( void * ) &saveMPEGCB, NULL, ( void * ) this ),

    SaveAnimatedGIFBrowser_ ( draw_, ( char * ) "AnimGIF", ( char * ) NULL,
                              ( void * ) &saveAnimatedGIFCB, NULL, ( void * ) this ),

    SavePNGBrowser_ ( draw_, ( char * ) "PNG", ( char * ) NULL,
                      ( void * ) &savePNGCB, NULL, ( void * ) this ),

    SaveGIFBrowser_ ( draw_, ( char * ) "GIF", ( char * ) NULL,
                      ( void * ) &saveGIFCB, NULL, ( void * ) this ),

    SaveConfigBrowser_ ( draw_, ( char * ) "Config", ( char * ) NULL,
                         ( void * ) &saveConfigCB, NULL, ( void * ) this ),

    SaveRGBBrowser_ ( draw_, ( char * ) "RGB", ( char * ) NULL,
                      ( void * ) &saveRGBCB, NULL, ( void * ) this ),

    SaveXWDBrowser_ ( draw_, ( char * ) "XWD", ( char * ) NULL,
                      ( void * ) &saveXWDCB, NULL, ( void * ) this ),

    SavePSBrowser_ ( draw_, ( char * ) "PostScript", ( char * ) NULL,
                     ( void * ) &savePSCB, NULL, ( void * ) this )
    {
    assert ( drawtype );
    work_proc_id_ = ( XtWorkProcId ) NULL; // 961018 added SRT
    assert ( uwind );
    assert ( vwind );
    work_proc_id_ = ( XtWorkProcId ) NULL; // 961018 added SRT
    dwnd_ = dwnd;
    vis_ = ( ReadVisData * ) NULL;   // 960918 added SRT for memory management
    uwind_ = ( VIS_DATA * ) NULL;    // 960918 added SRT for memory management
    vwind_ = ( VIS_DATA * ) NULL;    // 960918 added SRT for memory management
    canvas_ = ( Widget ) NULL;       // 960918 added SRT for memory management
    mappix1_ = ( Pixmap ) NULL;      // 960918 added SRT for memory management
    mappix2_ = ( Pixmap ) NULL;      // 960918 added SRT for memory management
    cfgp_ = cfgp;
    initColorLegendObject();
    uwind_ = uwind;
    vwind_ = vwind;
    exit_button_on_ = exit_button_on;
    colornames_ = colornames;
    numcolornames_ = numcolornames;

    vis_ = new ReadVisData ( drawtype, ibd, vdata );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::TileWnd() 3\n" );
    fprintf ( stderr, "with step_min == %d\n", vis_->step_min_ );
    fprintf ( stderr, "with step_max == %d\n", vis_->step_max_ );
#endif // DIAGNOSTICS

    vis_->title1_ = strdup ( title2 ); // Species/Fomula String
    vis_->title3_ = strdup ( title3 ); // Dataset (file) name(s) string
    frameDelayInTenthsOfSecondsP_ = frameDelayInTenthsOfSeconds;
    selected_step_ = 0;
    if ( vdata && vdata->selected_step > 0 ) selected_step_=vdata->selected_step-1;
    initTileWnd();
    tiles_on_ = ( vdata->grid != NULL );
    vectors_on_ = 1;
    overlay_ = 0;
    obs_dialog_ = cntr_dialog_ = NULL;
    if ( vectors_on_ && !tiles_on_ )    interact_mode_ = ZOOM_MODE;
    draw_dist_counties_ = 0;
    if ( getenv ( "PAVE_DISTINCT_STATE_COUNTIES" ) != NULL ) draw_dist_counties_ = 1;
    mapChoices_ = MapDefault;
    if ( getenv ( "MAPCHOICE" ) != NULL )
        {
        OUTLCO_cb ( 0 );
        }
    createUI ( draw_ );
    initRubberBand ( &s, canvas_ );
    initColorLegend ( canvas_, pix_, gc_, &s,
                      ( cfgp_->hasLegend_Min() ) ? cfgp_->getLegend_Min() : vis_->info->grid_min,
                      ( cfgp_->hasLegend_Max() ) ? cfgp_->getLegend_Max() : vis_->info->grid_max,
                      vis_->getUnits(), &vis_->title1_, &vis_->title2_, &vis_->title3_  );
    manage();
    manage_ = 1;
    setButtonSensitiviy();
    val_xytdelta_ = ( vis_->info->grid_max - vis_->info->grid_min ) / ( float ) numcolornames_;
    plotDataListP_ = new linkedList;
    current_pdata_obs_config_ = current_pdata_cntr_config_ = NULL;

    }





TileWnd::TileWnd (   Config *cfgp, void *dwnd, AppInit *app, char *name, BusData *ibd,
                     VIS_DATA *uwind,
                     VIS_DATA *vwind,
                     VIS_DATA *uwind_obs,
                     VIS_DATA *vwind_obs,
                     char *drawtype,
                     char *title2,  // Species/Fomula String
                     char *title3,  // Dataset (file) name(s) string
                     Dimension width, Dimension height,
                     int *frameDelayInTenthsOfSeconds,
                     int exit_button_on,
                     char **colornames, int numcolornames ) :
    DrawWnd ( app, name, drawtype, width, height, colornames, numcolornames, 0 ),
    Menus(),
    RubberBand(),
    ColorLegend(),
    SaveMPEGBrowser_ ( draw_, ( char * ) "MPEG", ( char * ) NULL,
                       ( void * ) &saveMPEGCB, NULL, ( void * ) this ),

    SaveAnimatedGIFBrowser_ ( draw_, ( char * ) "AnimGIF", ( char * ) NULL,
                              ( void * ) &saveAnimatedGIFCB, NULL, ( void * ) this ),

    SavePNGBrowser_ ( draw_, ( char * ) "PNG", ( char * ) NULL,
                      ( void * ) &savePNGCB, NULL, ( void * ) this ),

    SaveGIFBrowser_ ( draw_, ( char * ) "GIF", ( char * ) NULL,
                      ( void * ) &saveGIFCB, NULL, ( void * ) this ),

    SaveConfigBrowser_ ( draw_, ( char * ) "Config", ( char * ) NULL,
                         ( void * ) &saveConfigCB, NULL, ( void * ) this ),

    SaveRGBBrowser_ ( draw_, ( char * ) "RGB", ( char * ) NULL,
                      ( void * ) &saveRGBCB, NULL, ( void * ) this ),

    SaveXWDBrowser_ ( draw_, ( char * ) "XWD", ( char * ) NULL,
                      ( void * ) &saveXWDCB, NULL, ( void * ) this ),

    SavePSBrowser_ ( draw_, ( char * ) "PostScript", ( char * ) NULL,
                     ( void * ) &savePSCB, NULL, ( void * ) this )
    {
    VIS_DATA *vdata;

    assert ( drawtype );
    work_proc_id_ = ( XtWorkProcId ) NULL; // 961018 added SRT
    assert ( uwind );
    assert ( vwind );
    assert ( uwind_obs );
    assert ( vwind_obs );
    work_proc_id_ = ( XtWorkProcId ) NULL; // 961018 added SRT
    dwnd_ = dwnd;
    vis_ = ( ReadVisData * ) NULL;   // 960918 added SRT for memory management
    uwind_ = ( VIS_DATA * ) NULL;    // 960918 added SRT for memory management
    vwind_ = ( VIS_DATA * ) NULL;    // 960918 added SRT for memory management
    canvas_ = ( Widget ) NULL;   // 960918 added SRT for memory management
    mappix1_ = ( Pixmap ) NULL;      // 960918 added SRT for memory management
    mappix2_ = ( Pixmap ) NULL;      // 960918 added SRT for memory management
    cfgp_ = cfgp;
    initColorLegendObject();
    uwind_ = uwind;
    vwind_ = vwind;
    uwind_obs_ = uwind_obs;
    vwind_obs_ = vwind_obs;
    exit_button_on_ = exit_button_on;
    colornames_ = colornames;
    numcolornames_ = numcolornames;

    vdata = uwind;

    vis_ = new ReadVisData ( drawtype, ibd, vdata );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::TileWnd() 4\n" );
    fprintf ( stderr, "with step_min == %d\n", vis_->step_min_ );
    fprintf ( stderr, "with step_max == %d\n", vis_->step_max_ );
#endif // DIAGNOSTICS

    vis_->title1_ = strdup ( title2 ); // Species/Fomula String
    vis_->title3_ = strdup ( title3 ); // Dataset (file) name(s) string
    frameDelayInTenthsOfSecondsP_ = frameDelayInTenthsOfSeconds;
    selected_step_ = 0;
    if ( vdata && vdata->selected_step > 0 ) selected_step_=vdata->selected_step-1;
    initTileWnd();
    tiles_on_ = 0;
    vectors_on_ = 1;
    overlay_ = 1;
    obs_dialog_ = cntr_dialog_ = NULL;
    if ( vectors_on_ && !tiles_on_ )    interact_mode_ = ZOOM_MODE;
    draw_dist_counties_ = 0;
    if ( getenv ( "PAVE_DISTINCT_STATE_COUNTIES" ) != NULL ) draw_dist_counties_ = 1;
    mapChoices_ = MapDefault;
    if ( getenv ( "MAPCHOICE" ) != NULL )
        {
        OUTLCO_cb ( 0 );
        }
    createUI ( draw_ );
    initRubberBand ( &s, canvas_ );
    initColorLegend ( canvas_, pix_, gc_, &s,
                      ( cfgp_->hasLegend_Min() ) ? cfgp_->getLegend_Min() : vis_->info->grid_min,
                      ( cfgp_->hasLegend_Max() ) ? cfgp_->getLegend_Max() : vis_->info->grid_max,
                      vis_->getUnits(), &vis_->title1_, &vis_->title2_, &vis_->title3_  );
    manage();
    manage_ = 1;
    setButtonSensitiviy();
    val_xytdelta_ = ( vis_->info->grid_max - vis_->info->grid_min ) / ( float ) numcolornames_;
    plotDataListP_ = new linkedList;
    current_pdata_obs_config_ = current_pdata_cntr_config_ = NULL;

    }



TileWnd::~TileWnd() // 960918 added SRT for memory management
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::~TileWnd()\n" );
#endif // #ifdef DIAGNOSTICS

    if ( vis_ )     delete ( vis_ );
    vis_ = ( ReadVisData * ) NULL;
    if ( uwind_ )   free_vis ( uwind_ );
    uwind_ = ( VIS_DATA * ) NULL;
    if ( vwind_ )   free_vis ( vwind_ );
    vwind_ = ( VIS_DATA * ) NULL;
    if ( canvas_ )
        {
        if ( mappix1_ ) XFreePixmap ( XtDisplay ( canvas_ ), mappix1_ );
        mappix1_ = ( Pixmap ) NULL;
        if ( mappix2_ ) XFreePixmap ( XtDisplay ( canvas_ ), mappix2_ );
        mappix2_ = ( Pixmap ) NULL;
        XtDestroyWidget ( canvas_ );
        canvas_ = ( Widget ) NULL;
        }
    // loop over pdata linked list; free all data ALT
    }

long TileWnd::getWidgetId() // SRT 970326
    {
    return ( long ) _w;
    }


Window TileWnd::getWindowId() // ALT 110899
    {
    return XtWindow ( _w );
    }

Widget mostRecentlyAddedWindowsWidget = NULL; // added 970325 SRT


void TileWnd::initTileWnd()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::initTileWnd()\n" );
#endif // #ifdef DIAGNOSTICS

    mostRecentlyAddedWindowsWidget = _w; // added 970325 SRT

    mappix1_ = ( Pixmap ) NULL;
    mappix2_ = ( Pixmap ) NULL;

    interact_mode_ = PROBE_MODE;
    interact_submode_ = PROBE_TILE;
    probe_dialog_ = NULL;
    zoom_dialog_ = NULL;
    animate_dialog_ = NULL;
    titlefont_dialog_ = NULL;
    curr_animate_ = selected_step_;
    //   curr_animate_ = 0;
    animate_frame_ = 0;
    tiles_on_ = 0;
    vectors_on_ = 0;
    timezone_ = 0;
    tzname_ = NULL;
    titlefont_size_ = 24;
    subtitlefont_size_ = 14;

    zoom_[0].GRID_X_MIN_ = vis_->col_min_; // SRT 950707 Eng already had this
    zoom_[0].GRID_Y_MIN_ = vis_->row_min_; // SRT 950707 Eng already had this
    zoom_[0].GRID_X_MAX_ = vis_->col_max_; // SRT 950707 Eng already had this
    zoom_[0].GRID_Y_MAX_ = vis_->row_max_; // SRT 950707 Eng already had this

#ifdef DIAGNOSTICS
    fprintf ( stderr, "In initTileWnd row_max_=%d\n", vis_->row_max_ );
#endif // DIAGNOSTICS

    zoom_[0].parent = 0;
    zoom_[0].GRID_X_MIN_ -= 1.0; // SRT 950707 added -1.0
    zoom_[0].GRID_Y_MIN_ -= 1.0; // SRT 950707 added -1.0
    zoom_[0].GRID_X_MAX_ -= 1.0; // SRT 950707 Eng already had this -1.0 here
    zoom_[0].GRID_Y_MAX_ -= 1.0; // SRT 950707 Eng already had this -1.0 here
    }


void TileWnd::refreshColor()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::refreshColor()\n" );
#endif // #ifdef DIAGNOSTICS

    resize();
    }


void TileWnd::createUI ( Widget parent )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::createUI()\n" );
#endif // #ifdef DIAGNOSTICS

    // File -----------------------------------------------
    menu_struct sub_menu_file[10];
    int index = 0;

    sub_menu_file[index].name = "Print";
    sub_menu_file[index].func = &TileWnd::printMenuCB;
    sub_menu_file[index].sub_menu = NULL;
    sub_menu_file[index].n_sub_items = 0;
    sub_menu_file[index].sub_menu_title = NULL;
    printmenuIndex_ = index;
    index++;

    sub_menu_file[index].name = "Save Configuration Settings";
    sub_menu_file[index].func = ( void ( * ) ( _WidgetRec *,void *,void * ) ) &TileWnd::saveConfigMenuCB;
    sub_menu_file[index].sub_menu = NULL;
    sub_menu_file[index].n_sub_items = 0;
    sub_menu_file[index].sub_menu_title = NULL;
    index++;

    sub_menu_file[index].name = "Save PNG Image";
    sub_menu_file[index].func = &TileWnd::savePNGMenuCB;
    sub_menu_file[index].sub_menu = NULL;
    sub_menu_file[index].n_sub_items = 0;
    sub_menu_file[index].sub_menu_title = NULL;
    PNGmenuIndex_ = index;
    index++;

    sub_menu_file[index].name = "Save GIF Image";
    sub_menu_file[index].func = &TileWnd::saveGIFMenuCB;
    sub_menu_file[index].sub_menu = NULL;
    sub_menu_file[index].n_sub_items = 0;
    sub_menu_file[index].sub_menu_title = NULL;
    GIFmenuIndex_ = index;
    index++;

    sub_menu_file[index].name = "Save PostScript Image";
    sub_menu_file[index].func = &TileWnd::savePSMenuCB;
    sub_menu_file[index].sub_menu = NULL;
    sub_menu_file[index].n_sub_items = 0;
    sub_menu_file[index].sub_menu_title = NULL;
    PSmenuIndex_ = index;
    index++;

    sub_menu_file[index].name = "Save RGB Image";
    sub_menu_file[index].func = &TileWnd::saveRGBMenuCB;
    sub_menu_file[index].sub_menu = NULL;
    sub_menu_file[index].n_sub_items = 0;
    sub_menu_file[index].sub_menu_title = NULL;
    RGBmenuIndex_ = index;
    index++;

    sub_menu_file[index].name = "Save XWD Image";
    sub_menu_file[index].func = &TileWnd::saveXWDMenuCB;
    sub_menu_file[index].sub_menu = NULL;
    sub_menu_file[index].n_sub_items = 0;
    sub_menu_file[index].sub_menu_title = NULL;
    index++;

    sub_menu_file[index].name = "Save MPEG Animation";
    sub_menu_file[index].func = &TileWnd::saveMPEGMenuCB;
    sub_menu_file[index].sub_menu = NULL;
    sub_menu_file[index].n_sub_items = 0;
    sub_menu_file[index].sub_menu_title = NULL;
    MPEGmenuIndex_ = index;
    index++;

    sub_menu_file[index].name = "Save Animated GIF";
    sub_menu_file[index].func = &TileWnd::saveAnimatedGIFMenuCB;
    sub_menu_file[index].sub_menu = NULL;
    sub_menu_file[index].n_sub_items = 0;
    sub_menu_file[index].sub_menu_title = NULL;
    index++;

    if ( exit_button_on_ == 1 )
        sub_menu_file[index].name = "Exit";
    else
        sub_menu_file[index].name = "Close";
    sub_menu_file[index].func = &TileWnd::exitCB;
    sub_menu_file[index].sub_menu = NULL;
    sub_menu_file[index].n_sub_items = 0;
    sub_menu_file[index].sub_menu_title = NULL;
    index++;
    nFileMenuItems_ = index;


    // Probe --------------------------------------------
    menu_struct sub_menu_probe[2];
    sub_menu_probe[0].name = "Tiles";
    sub_menu_probe[0].func = &TileWnd::probeCB;
    sub_menu_probe[0].sub_menu = NULL;
    sub_menu_probe[0].n_sub_items = 0;
    sub_menu_probe[0].sub_menu_title = NULL;
    sub_menu_probe[1].name = "OBS";
    sub_menu_probe[1].func = &TileWnd::probeobsCB;
    sub_menu_probe[1].sub_menu = NULL;
    sub_menu_probe[1].n_sub_items = 0;
    sub_menu_probe[1].sub_menu_title = NULL;

    // Time Series_--------------------------------------
    menu_struct sub_menu_ts[3];
    sub_menu_ts[0].name = "Tiles";
    sub_menu_ts[0].func = &TileWnd::timeSeriesCB;
    sub_menu_ts[0].sub_menu = NULL;
    sub_menu_ts[0].n_sub_items = 0;
    sub_menu_ts[0].sub_menu_title = NULL;
    sub_menu_ts[1].name = "OBS";
    sub_menu_ts[1].func = &TileWnd::timeSeriesObsCB;
    sub_menu_ts[1].sub_menu = NULL;
    sub_menu_ts[1].n_sub_items = 0;
    sub_menu_ts[1].sub_menu_title = NULL;
    sub_menu_ts[2].name = "Both";
    sub_menu_ts[2].func = &TileWnd::timeSeriesCB; /* WRONG */
    sub_menu_ts[2].sub_menu = NULL;
    sub_menu_ts[2].n_sub_items = 0;
    sub_menu_ts[2].sub_menu_title = NULL;

    // Interact --------------------------------------------
    menu_struct sub_menu_interact[3];
    sub_menu_interact[0].name = "Probe...";
    sub_menu_interact[0].func = NULL;
    sub_menu_interact[0].sub_menu = sub_menu_probe;
    sub_menu_interact[0].n_sub_items = 2;
    sub_menu_interact[0].sub_menu_title = NULL;
    sub_menu_interact[1].name = "Zoom";
    sub_menu_interact[1].func = &TileWnd::zoomCB;
    sub_menu_interact[1].sub_menu = NULL;
    sub_menu_interact[1].n_sub_items = 0;
    sub_menu_interact[1].sub_menu_title = NULL;
    sub_menu_interact[2].name = "Time Series...";
    sub_menu_interact[2].func = NULL;
    sub_menu_interact[2].sub_menu = sub_menu_ts;
    sub_menu_interact[2].n_sub_items = 2; /* Both is temporary disabled */
    sub_menu_interact[2].sub_menu_title = NULL;


    // Configure ---------------------------------------------
    menu_struct sub_menu_config[5];
    sub_menu_config[0].name = "Tile..";
    sub_menu_config[0].func = &TileWnd::configureCB;
    sub_menu_config[0].sub_menu = NULL;
    sub_menu_config[0].n_sub_items = 0;
    sub_menu_config[0].sub_menu_title = NULL;
    sub_menu_config[1].name = "OBS..";
    sub_menu_config[1].func = &TileWnd::configureobsCB;
    sub_menu_config[1].sub_menu = NULL;
    sub_menu_config[1].n_sub_items = 0;
    sub_menu_config[1].sub_menu_title = NULL;
    sub_menu_config[2].name = "Contour..";
    sub_menu_config[2].func = &TileWnd::configurecntrCB;
    sub_menu_config[2].sub_menu = NULL;
    sub_menu_config[2].n_sub_items = 0;
    sub_menu_config[2].sub_menu_title = NULL;
    sub_menu_config[3].name = "VectorObs..";
    sub_menu_config[3].func = &TileWnd::configureobsCB;
    sub_menu_config[3].sub_menu = NULL;
    sub_menu_config[3].n_sub_items = 0;
    sub_menu_config[3].sub_menu_title = NULL;
    sub_menu_config[4].name = "Title/Subtitle font...";
    sub_menu_config[4].func = &TileWnd::titlefont_dialogCB;
    sub_menu_config[4].sub_menu = NULL;
    sub_menu_config[4].n_sub_items = 0;
    sub_menu_config[4].sub_menu_title = NULL;

    // Control ---------------------------------------------
    menu_struct sub_menu_control[3];
    sub_menu_control[0].name = "Animate..";
    sub_menu_control[0].func = &TileWnd::animate_dialogCB;
    sub_menu_control[0].sub_menu = NULL;
    sub_menu_control[0].n_sub_items = 0;
    sub_menu_control[0].sub_menu_title = NULL;
    sub_menu_control[1].name = "Zoom..";
    sub_menu_control[1].func = &TileWnd::zoom_dialogCB;
    sub_menu_control[1].sub_menu = NULL;
    sub_menu_control[1].n_sub_items = 0;
    sub_menu_control[1].sub_menu_title = NULL;
    sub_menu_control[2].name = "Configure...";
    sub_menu_control[2].func = NULL;
    sub_menu_control[2].sub_menu = sub_menu_config;
    sub_menu_control[2].n_sub_items = 5;
    sub_menu_control[2].sub_menu_title = NULL;

    // Map ---------------------------------------------
    menu_struct sub_menu_map[6];

    sub_menu_map[0].name = "Counties (SLOW the first time)";
    sub_menu_map[0].func = &TileWnd::OUTLCO_CB;
    sub_menu_map[0].sub_menu = NULL;
    sub_menu_map[0].n_sub_items = 0;
    sub_menu_map[0].sub_menu_title = NULL;

    sub_menu_map[1].name = "Medium-res states";
    sub_menu_map[1].func = &TileWnd::OUTLUSAM_CB;
    sub_menu_map[1].sub_menu = NULL;
    sub_menu_map[1].n_sub_items = 0;
    sub_menu_map[1].sub_menu_title = NULL;

    sub_menu_map[2].name = "High-res states";
    sub_menu_map[2].func = &TileWnd::OUTLSTATES3000_CB;
    sub_menu_map[2].sub_menu = NULL;
    sub_menu_map[2].n_sub_items = 0;
    sub_menu_map[2].sub_menu_title = NULL;

    sub_menu_map[3].name = "Rivers (SLOW the first time)";
    sub_menu_map[3].func = &TileWnd::OUTLRIVERS3000_CB;
    sub_menu_map[3].sub_menu = NULL;
    sub_menu_map[3].n_sub_items = 0;
    sub_menu_map[3].sub_menu_title = NULL;

    sub_menu_map[4].name = "Roads/transport routes (SLOW the first time)";
    sub_menu_map[4].func = &TileWnd::OUTLROADS3000_CB;
    sub_menu_map[4].sub_menu = NULL;
    sub_menu_map[4].n_sub_items = 0;
    sub_menu_map[4].sub_menu_title = NULL;

    sub_menu_map[5].name = "World map (SLOW the first time)";
    sub_menu_map[5].func = &TileWnd::OUTLHRES_CB;
    sub_menu_map[5].sub_menu = NULL;
    sub_menu_map[5].n_sub_items = 0;
    sub_menu_map[5].sub_menu_title = NULL;


    // Plot ---------------------------------------------
    menu_struct sub_menu_plot[2];

    index = 0;
    sub_menu_plot[index].name = "Time Series At Max Point";
    sub_menu_plot[index].func = &TileWnd::maxPointTimeSeriesCB;
    sub_menu_plot[index].sub_menu = NULL;
    sub_menu_plot[index].n_sub_items = 0;
    sub_menu_plot[index].sub_menu_title = NULL;
    index++;

    sub_menu_plot[index].name = "Time Series At Min Point";
    sub_menu_plot[index].func = &TileWnd::minPointTimeSeriesCB;
    sub_menu_plot[index].sub_menu = NULL;
    sub_menu_plot[index].n_sub_items = 0;
    sub_menu_plot[index].sub_menu_title = NULL;
    index++;


    //Overlay ---------------------------------------------
    menu_struct sub_menu_overlay[3];

    index = 0;
    sub_menu_overlay[index].name = "Observations...";
    sub_menu_overlay[index].func = &TileWnd::overlay_obsCB;
    sub_menu_overlay[index].sub_menu = NULL;
    sub_menu_overlay[index].n_sub_items = 0;
    sub_menu_overlay[index].sub_menu_title = NULL;
    index++;

    sub_menu_overlay[index].name = "VectorObs...";
    sub_menu_overlay[index].func = &TileWnd::overlay_vectorobsCB;
    sub_menu_overlay[index].sub_menu = NULL;
    sub_menu_overlay[index].n_sub_items = 0;
    sub_menu_overlay[index].sub_menu_title = NULL;
    index++;

    sub_menu_overlay[index].name = "Contour...";
    sub_menu_overlay[index].func = &TileWnd::overlay_cntrCB;
    sub_menu_overlay[index].sub_menu = NULL;
    sub_menu_overlay[index].n_sub_items = 0;
    sub_menu_overlay[index].sub_menu_title = NULL;
    index++;


    // Menu bar --------------------------------------------
    menu_struct PulldownData[6];

    index = 0;
    PulldownData[index].name = "File";
    PulldownData[index].func = NULL;
    PulldownData[index].sub_menu = sub_menu_file;
    PulldownData[index].n_sub_items = nFileMenuItems_;
    PulldownData[index].sub_menu_title = NULL;
    index++;

    PulldownData[index].name = "Interact";
    PulldownData[index].func = NULL;
    PulldownData[index].sub_menu = sub_menu_interact;
    PulldownData[index].n_sub_items = 3;
    PulldownData[index].sub_menu_title = NULL;
    index++;

    PulldownData[index].name = "Control";
    PulldownData[index].func = NULL;
    PulldownData[index].sub_menu = sub_menu_control;
    PulldownData[index].n_sub_items = 3;
    PulldownData[index].sub_menu_title = NULL;
    index++;

    PulldownData[index].name = "Map";
    PulldownData[index].func = NULL;
    PulldownData[index].sub_menu = sub_menu_map;
    PulldownData[index].n_sub_items = 6;
    PulldownData[index].sub_menu_title = NULL;
    index++;

    PulldownData[index].name = "Plot";
    PulldownData[index].func = NULL;
    PulldownData[index].sub_menu = sub_menu_plot;
    PulldownData[index].n_sub_items = 2;
    PulldownData[index].sub_menu_title = NULL;
    index++;

    PulldownData[index].name = "Overlay";
    PulldownData[index].func = NULL;
    PulldownData[index].sub_menu = sub_menu_overlay;
    PulldownData[index].n_sub_items = 3;
    PulldownData[index].sub_menu_title = NULL;
    index++;

    createMenuBar ( parent, ( XtPointer ) this );
    createMenuButtons ( ( char * ) NULL, menuBar_, PulldownData, index );

    // store the ID of these widgets in order to possibly set
    // their sensitivity later
    printmenuButton_ = sub_menu_file[printmenuIndex_].wid;
    createPSMenuButton_ = sub_menu_file[PSmenuIndex_].wid;
    createPNGMenuButton_ = sub_menu_file[PNGmenuIndex_].wid;
    createGIFMenuButton_ = sub_menu_file[GIFmenuIndex_].wid;
    createRGBMenuButton_ = sub_menu_file[RGBmenuIndex_].wid;
    createMPEGAnimationMenuButton_ = sub_menu_file[MPEGmenuIndex_].wid;
    animateButton_ = sub_menu_control[0].wid;
    if ( vectors_on_ && !tiles_on_ ) // disable Probe and TimeSeries for vector plots (ALT)
        {
        XtSetSensitive ( sub_menu_interact[0].wid, False );
        XtSetSensitive ( sub_menu_interact[2].wid, False );
        }

    probeobsButton_ = sub_menu_probe[1].wid;
    XtSetSensitive ( probeobsButton_, False );
    configobsButton_ = sub_menu_config[1].wid;
    XtSetSensitive ( configobsButton_, False );
    configcntrButton_ = sub_menu_config[2].wid;
    XtSetSensitive ( configcntrButton_, False );
    configvectobsButton_ = sub_menu_config[3].wid;
    XtSetSensitive ( configvectobsButton_, False );
    tsobsButton_ = sub_menu_ts[1].wid;
    XtSetSensitive ( tsobsButton_, False );

    // store the ID of these widgets in order to redraw them
    // later (sometimes the X server seems to "forget" to redraw
    // them, even though they're still active, after an image
    // is saved from PAVE)
    fileButton_ = PulldownData[0].wid;
    interactButton_ = PulldownData[1].wid;
    controlButton_ = PulldownData[2].wid;
    mapButton_ = PulldownData[3].wid;

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
    assert ( canvas_ );

    // XSynchronize(XtDisplay(canvas_), True); // use if you want, for debugging purposes

    XtAddCallback ( canvas_,
                    XmNexposeCallback,
                    &TileWnd::redisplayCB,
                    ( XtPointer ) this );

    XtAddCallback ( canvas_,
                    XmNresizeCallback,
                    &TileWnd::resizeCB,
                    ( XtPointer ) this );
    }


/*
 * ALL TILE PLOT DRAWING DONE HERE!!
 */
void TileWnd::drawDetail()
    {
    int jdate, jtime;
    int drawLegend = 1;
    int drawTiles = 1;
    int drawGridLabels = 1;
    int doDrawTimeStamp = 1;
    int doDrawMinMax = 1;
    int onlyLegend = 0;
    
    char *dirname ;

    if ( getenv ( "DRAWLEGEND" ) !=NULL )
        {
        if ( strcasecmp ( getenv ( "DRAWLEGEND" ),"OFF" ) == 0 )
            {
            drawLegend = 0;
            }
        }
    if ( getenv ( "DRAWTILES" ) !=NULL )
        {
        if ( strcasecmp ( getenv ( "DRAWTILES" ),"OFF" ) == 0 )
            {
            drawTiles = 0;
            }
        }
    if ( getenv ( "DRAWTIMESTAMP" ) !=NULL )
        {
        if ( strcasecmp ( getenv ( "DRAWTIMESTAMP" ),"OFF" ) == 0 )
            {
            doDrawTimeStamp = 0;
            }
        }
    if ( getenv ( "DRAWMINMAX" ) !=NULL )
        {
        if ( strcasecmp ( getenv ( "DRAWMINMAX" ),"OFF" ) == 0 )
            {
            doDrawMinMax = 0;
            }
        }
    if ( getenv ( "DRAWGRIDLABELS" ) !=NULL )
        {
        if ( strcasecmp ( getenv ( "DRAWGRIDLABELS" ),"OFF" ) == 0 )
            {
            drawGridLabels = 0;
            }
        }
    if ( getenv ( "DRAWONLYLEGEND" ) !=NULL )
        {
        if ( strcasecmp ( getenv ( "DRAWONLYLEGEND" ),"ON" ) == 0 )
            {
            doDrawMinMax = 0;
            doDrawTimeStamp = 0;
            drawTiles = 0;
            onlyLegend = 1;
            }
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "In drawDetail\n" );
#endif // DIAGNOSTICS
    if ( vis_->step_min_ == vis_->step_max_ )
        {
        XtSetSensitive ( animateButton_, False );
        }
    else
        {
        XtSetSensitive ( animateButton_, True );
        }

    setForeground ( "black" );

    drawSetup ( canvas_, pix_, gc_, &s );

    if ( drawTiles ) drawTile ( curr_animate_ );
    if ( !onlyLegend ) drawOverlays ( curr_animate_ );

    if ( mapChoices_ == MapCounties && draw_dist_counties_ )
        {
        setForeground ( "gray" );
        dirname = getenv ( "PAVE_MAPDIR" ) ;
        if ( dirname && *dirname )
            {
            sprintf ( vis_->mapName_, "%s/OUTLCOUNTIES", getenv ( "PAVE_MAPDIR" ) );
            vis_->setMapData();
            drawMap();
            setForeground ( "black" );
            sprintf ( vis_->mapName_, "%s/OUTLSTATES3000", getenv ( "PAVE_MAPDIR" ) );
            vis_->setMapData();
            }
        else{
            fprintf( stderr, "WARNING:  missing environment variable PAVE_MAPDIR\n" ) ;
            sprintf ( vis_->mapName_, "OUTLCOUNTIES" );
            vis_->setMapData();
            drawMap();
            setForeground ( "black" );
            sprintf ( vis_->mapName_, "OUTLSTATES3000" );
            vis_->setMapData();
            }
        }
    if ( !onlyLegend )
        {
        drawMap();
        drawFrame();
        drawTitles ( vis_->title1_, vis_->title2_, vis_->title3_,  vis_->xtitle_, vis_->ytitle_ );
        if ( drawGridLabels )
            {
            drawYtics ( 1 /* 4 SRT 950707 */, vis_->plotformatY_,
                        ( getenv ( "TILEYLABELSONRIGHT" ) !=NULL ) && ( strlen ( getenv ( "TILEYLABELSONRIGHT" ) ) ),0 );
            drawXtics ( 1 /* 4 SRT 950707 */, vis_->plotformatX_ );
            }
        }
    initColorLegend ( canvas_, pix_, gc_, &s,
                      -1.0, -1.0 /* SRT 950721 vis_->info->grid_min, vis_->info->grid_max */,
                      vis_->getUnits(), &vis_->title1_, &vis_->title2_, &vis_->title3_  );
    if ( vectors_on_ && scale_vectors_on_ && !onlyLegend )
        {
        float        x, y, x2, y2;
        char str[256];
        sprintf ( str,"%2.1f%s",vector_scale_,vis_->getUnits() );
        x = s.scalex ( s.xmin_ );
        x2 = s.scalex ( s.xmax_ );
        x = x + 0.9* ( x2-x );
        y = y2 = s.scaley ( s.ymin_ ) + 30;

        //       setForeground("red");
        drawVect ( x, y, x2, y2 );
        XDrawString ( XtDisplay ( canvas_ ), pix_, gc_,
                      ( int ) ( 0.5* ( x+x2 ) ), ( int ) ( y+20 ), str, strlen ( str ) );
        //       setForeground("black");
        }

    if ( tiles_on_ && drawLegend ) drawColorLegend ( 5, 10 );
    jdate = vis_->info->sdate[curr_animate_];
    jtime = vis_->info->stime[curr_animate_];
    if ( timezone_ != 0 )
        {
        nextimec ( &jdate, &jtime, sec2timec ( timezone_*3600 ) );
        }
    if ( doDrawTimeStamp ) drawTimeStamp ( curr_animate_, jdate, jtime, tzname_ );
    if ( tiles_on_ && doDrawMinMax ) drawMinMax ( curr_animate_ );

    setForeground ( "black" );
    }



///////////////////////////////////////////////////////////////////////

void TileWnd::fillCanvasBackground ( char *color )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::fillCanvasBackground()\n" );
#endif // #ifdef DIAGNOSTICS

    setForeground ( color );

    XFillRectangle ( XtDisplay ( canvas_ ), XtWindow ( canvas_ ), gc_, 0, 0,
                     width_, height_ );
    }




void TileWnd::toggleMapDrawing()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::toggleMapDrawing()\n" );
#endif // #ifdef DIAGNOSTICS

    refreshColor();
    }


void TileWnd::toggleSmoothPlots()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::toggleSmoothPlots()\n" );
#endif // #ifdef DIAGNOSTICS

    refreshColor();
    }


void TileWnd::toggleGridLines()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::toggleGridLines()\n" );
#endif // #ifdef DIAGNOSTICS

    refreshColor();
    }


void TileWnd::toggleScaleVectors()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::toggleScaleVectors()\n" );
#endif // #ifdef DIAGNOSTICS

    refreshColor();
    }


int TileWnd::get_nhours ( void )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::get_nhours()\n" );
#endif // #ifdef DIAGNOSTICS

    if ( vis_!=NULL )
        return vis_->info->step_max-vis_->info->step_min+1;
    else
        return 0;
    }

int TileWnd::get_sdate ( void )
    {
    if ( vis_!=NULL )
        return vis_->info->sdate[vis_->info->step_min-1];
    else
        return 0;
    }

int TileWnd::get_stime ( void )
    {
    if ( vis_!=NULL )
        return vis_->info->stime[vis_->info->step_min-1];
    else
        return 0;
    }

int TileWnd::get_tstep ( void )
    {
    if ( vis_!=NULL )
        return vis_->info->incr_sec;
    else
        return 0;
    }

int TileWnd::get_edate ( void )
    {
    if ( vis_!=NULL )
        return vis_->info->sdate[vis_->info->step_max-1];
    else
        return 0;
    }

int TileWnd::get_etime ( void )
    {
    if ( vis_!=NULL )
        return vis_->info->stime[vis_->info->step_max-1];
    else
        return 0;
    }

void TileWnd::set_offset ( int value )
    {
    offset_ = value;
    }

int TileWnd::get_offset ( void )
    {
    return offset_;
    }

int TileWnd::get_skip ( void )
    {
    return skip_;
    }

void TileWnd::set_skip ( int value )
    {
    skip_ = value;
    }

void TileWnd::drawMap()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::drawMap()\n" );
#endif // #ifdef DIAGNOSTICS

    if ( grid_lines_on_ )
        drawGridLines();

    // added if test SRT
    if ( !legend_map_off_ )
        if ( ( vis_->info->slice == XYTSLICE ) || ( vis_->info->slice == XYSLICE ) )
            {

            XPoint points[2500];
            int i, j, k, npoints;

            npoints = 0;
            for ( i = 0; i < vis_->map_npolyline_; i++ )
                {
                k = 0;
                for ( j = 0; j < vis_->map_n_[i]; j++ )
                    {
                    points[k].x = s.scalex ( vis_->map_x_[npoints] );
                    points[k].y = s.scaley ( vis_->map_y_[npoints] );

#ifdef DIAGNOSTICS
                    fprintf ( stderr, "In TileWnd::drawMap() drawing point (%g,%g)\n",
                              vis_->map_x_[npoints], vis_->map_y_[npoints] );
#endif // DIAGNOSTICS

                    if ( k < 2499 )
                        ++k;
                    ++npoints;
                    }
                XDrawLines ( dpy_, pix_, gc_, points,
                             k, CoordModeOrigin );
                //                        vis_->map_n_[i], CoordModeOrigin);
                }

            }
    }


/////////////////////////////////////////////////////////////////////////////////


void TileWnd::redisplayCB ( Widget, XtPointer clientData, XtPointer callData )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::redisplayCB()\n" );
#endif // #ifdef DIAGNOSTICS

    TileWnd *obj = ( TileWnd * ) clientData;
    XmAnyCallbackStruct *cbs = ( XmAnyCallbackStruct * ) callData;
    XExposeEvent *event = ( XExposeEvent * ) cbs->event;

    obj->redisplay ( event );
    }


void TileWnd::redisplay ( XExposeEvent *event )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::redisplay()\n" );
#endif // #ifdef DIAGNOSTICS

    if ( pix_ != ( Pixmap ) NULL )
        {
        XCopyArea ( XtDisplay ( canvas_ ), pix_, XtWindow ( canvas_ ), gc_,
                    event->x, event->y, event->width, event->height,
                    event->x, event->y );
        }
    else
        resize();
    }


void TileWnd::resizeCB ( Widget, XtPointer clientData, XtPointer )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::resizeCB()\n" );
#endif // #ifdef DIAGNOSTICS

    TileWnd *obj = ( TileWnd * ) clientData;
    obj->resize();
    }


void TileWnd::resize()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::resize()\n" );
#endif // DIAGNOSTICS

    if ( !XtWindow ( canvas_ ) ) return; // 961001 SRT prevents XCreatePixMap below from crashing PAVE

    // Get the size of the drawing area

    XtVaGetValues ( canvas_,
                    XmNwidth, &width_,
                    XmNheight, &height_,
                    NULL );

    s.scaleInit ( floor ( zoom_[curr_zoom_].GRID_X_MIN_ ),
                  floor ( zoom_[curr_zoom_].GRID_Y_MIN_ ),
                  floor ( zoom_[curr_zoom_].GRID_X_MAX_+1 ),
                  floor ( zoom_[curr_zoom_].GRID_Y_MAX_+1 ),
                  width_, height_ );

    if ( pix_ ) XFreePixmap ( XtDisplay ( canvas_ ), pix_ );
    pix_ = ( Pixmap ) NULL;


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

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Exit TileWnd::resize()\n" );
#endif // DIAGNOSTICS
    }


void TileWnd::adjustZoomDialogPosition ( void ) // added 950913 SRT to
// override RubberBand.cc's
    {
    Position xpos, ypos;

    if ( !zoom_dialog_ ) return;

    // Position control dialog box so it doesn't obscure the plot
    XtVaGetValues ( draw_, XmNx, &xpos, XmNy, &ypos, NULL );

    XtVaSetValues ( zoom_dialog_,
                    XmNdefaultPosition,     False,   // so won't be centered - SRT
                    XtNx,                   xpos+100, // SRT 950911
                    XtNy,                   ypos-90, // SRT 950911
                    NULL );
    }



void TileWnd::createAnimateDialog ( Widget parent )
    {
    Position xpos, ypos;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "In createAnimateDialog 333\n" );
#endif // DIAGNOSTICS

    assert ( parent );

    work_proc_id_ = ( XtWorkProcId ) NULL;

    // Create control dialog box.
    animate_dialog_ = XmCreateFormDialog ( parent, "control", NULL, 0 );

    // Position control dialog box so it doesn't obscure the plot
    XtVaGetValues ( parent, XmNx, &xpos, XmNy, &ypos, NULL );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "TileWnd::createAnimateDialog's parent's xpos=%d, ypos=%d\n",
              ( int ) xpos, ( int ) ypos );
#endif // DIAGNOSTICS

    XtVaSetValues ( animate_dialog_,
                    XmNautoUnmanage,    False,
                    XmNdefaultPosition, False,   // so won't be centered - SRT
                    XtNx,           xpos+35, // SRT 950911
                    XtNy,           ypos-90, // SRT 950911
                    NULL );

    Widget form1 = XtVaCreateWidget ( "form1",
                                      xmFormWidgetClass,  animate_dialog_,
                                      XmNtopAttachment,   XmATTACH_FORM,
                                      XmNtopOffset,       10,
                                      XmNleftAttachment,  XmATTACH_FORM,
                                      XmNleftOffset,      10,
                                      XmNrightAttachment, XmATTACH_FORM,
                                      XmNrightOffset,     10,
                                      NULL );

    animate_scale_ = XtVaCreateManagedWidget ( "Hour",
                     xmScaleWidgetClass, form1,

                     XtVaTypedArg, XmNtitleString, XmRString, "Timestep", 5,

                     XmNheight,      100,
                     XmNmaximum,         vis_->step_max_
                     /* -1 950801 SRT*/ -vis_->step_min_,
                     XmNminimum,     0,
                     XmNvalue,       0,
                     XmNshowValue,       True,
                     XmNorientation,     XmHORIZONTAL,
                     XmNtopAttachment,   XmATTACH_FORM,
                     XmNleftAttachment,  XmATTACH_FORM,
                     XmNrightAttachment, XmATTACH_FORM,
                     NULL );

    XtAddCallback ( animate_scale_, XmNvalueChangedCallback,  &TileWnd::animate_scaleCB, ( XtPointer ) this );


    if ( XtIsManaged ( form1 ) ) XtUnmanageChild ( form1 );
    XtManageChild ( form1 );

    Widget separator = XtVaCreateManagedWidget ( "sep",
                       xmSeparatorWidgetClass, animate_dialog_,
                       XmNtopAttachment,       XmATTACH_WIDGET,
                       XmNtopWidget,           form1,
                       XmNtopOffset,       10,
                       XmNleftAttachment,      XmATTACH_FORM,
                       XmNrightAttachment,     XmATTACH_FORM,
                       NULL );



    // Button to start animating.
    animate_ = XtVaCreateManagedWidget ( "Animate",
                                         xmPushButtonWidgetClass, animate_dialog_,
                                         XmNtopAttachment,   XmATTACH_WIDGET,
                                         XmNtopWidget,       separator,
                                         XmNtopOffset,       10,
                                         XmNleftAttachment,  XmATTACH_FORM,
                                         XmNleftOffset,      10,
                                         XmNbottomAttachment,    XmATTACH_FORM,
                                         XmNbottomOffset,    10,
                                         XmNwidth,       100,
                                         XmNheight,      40,
                                         NULL );
    XtAddCallback ( animate_, XmNactivateCallback, &TileWnd::animateCB, ( XtPointer ) this );


    // Button to stop animating.
    stop_ = XtVaCreateManagedWidget ( "Stop",
                                      xmPushButtonWidgetClass, animate_dialog_,
                                      XmNtopAttachment,   XmATTACH_WIDGET,
                                      XmNtopWidget,       separator,
                                      XmNtopOffset,       10,
                                      XmNleftAttachment,  XmATTACH_WIDGET,
                                      XmNleftWidget,      animate_,
                                      XmNleftOffset,      10,
                                      XmNwidth,       100,
                                      XmNheight,      40,
                                      NULL );
    XtAddCallback ( stop_, XmNactivateCallback, &TileWnd::stopCB, ( XtPointer ) this );


    close_ = XtVaCreateManagedWidget ( "Close",
                                       xmPushButtonWidgetClass, animate_dialog_,
                                       XmNtopAttachment,   XmATTACH_WIDGET,
                                       XmNtopWidget,       separator,
                                       XmNtopOffset,       10,
                                       XmNleftAttachment,  XmATTACH_WIDGET,
                                       XmNleftWidget,      stop_,
                                       XmNrightAttachment, XmATTACH_FORM,
                                       XmNrightOffset,     10,
                                       XmNwidth,       100,
                                       XmNheight,      40,
                                       NULL );
    XtAddCallback ( close_, XmNactivateCallback, &TileWnd::closeCB, ( XtPointer ) this );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Leaving createAnimateDilog\n" );
#endif // DIAGNOSTICS
    }



void TileWnd::exitCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->exit_cb();          // close up the window
    }


extern void TileWndHasBeenClosed_CB ( void * );
void TileWnd::exit_cb()
    {
    stop_cb();                      // stop any animation going on
    if ( exit_button_on_ == 1 )
        exit ( 1 );
    else
        {
        unmanage();
        manage_ = 0;
        }

    if ( mostRecentlyAddedWindowsWidget == _w )
        mostRecentlyAddedWindowsWidget = NULL;

    TileWndHasBeenClosed_CB ( dwnd_ ); // let the DriverWnd know, which
    // in turn will delete this object
    }



void TileWnd::probeCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->probe_cb();
    }

void TileWnd::probe_cb()
    {
    interact_mode_ = PROBE_MODE;
    interact_submode_ = PROBE_TILE;
    }

void TileWnd::probeobsCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->probeobs_cb();
    }

void TileWnd::probeobs_cb()
    {
    interact_mode_ = PROBE_MODE;
    interact_submode_ = PROBE_OBS;
    }


void TileWnd::zoomCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->zoom_cb();
    }

void TileWnd::zoom_cb()
    {
    interact_mode_ = ZOOM_MODE;
    }


void TileWnd::animate_dialogCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->animate_dialog_cb();
    }

void TileWnd::animate_dialog_cb()
    {
    if ( animate_dialog_ == NULL )
        createAnimateDialog ( draw_ );
    if ( XtIsManaged ( animate_dialog_ ) ) XtUnmanageChild ( animate_dialog_ );
    XtManageChild ( animate_dialog_ );
    }

void TileWnd::titlefont_dialogCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->titlefont_dialog_cb();
    }

void TileWnd::titlefont_dialog_cb()
    {
    if ( titlefont_dialog_ == NULL )
        createTitleFontDialog ( draw_ );
    if ( XtIsManaged ( titlefont_dialog_ ) ) XtUnmanageChild ( titlefont_dialog_ );
    XtManageChild ( titlefont_dialog_ );
    }


void TileWnd::zoom_dialogCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->zoom_dialog_cb();
    }

void TileWnd::zoom_dialog_cb()
    {
    if ( zoom_dialog_ == NULL )
        {
        createZoomDialog();
        adjustZoomDialogPosition();
        }
    if ( XtIsManaged ( zoom_dialog_ ) ) XtUnmanageChild ( zoom_dialog_ );
    XtManageChild ( zoom_dialog_ );
    }


void TileWnd::configureCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->configure_cb();
    }

void TileWnd::configure_cb()
    {
    if ( legend_dialog_ == NULL )
        {
        Position xpos, ypos;

        // Position control dialog box so it doesn't obscure the plot
        XtVaGetValues ( draw_, XmNx, &xpos, XmNy, &ypos, NULL );

#ifdef DIAGNOSTICS
        fprintf ( stderr,
                  "TileWnd::zoom_dialog_cb's parent's xpos=%d, ypos=%d\n",
                  ( int ) xpos, ( int ) ypos );
#endif // DIAGNOSTICS

        createColorLegendDialog();
        XtVaSetValues ( legend_dialog_,
                        XmNdefaultPosition,     False,   // so won't be centered - SRT
                        XtNx,                   xpos+375,// SRT 950911
                        XtNy,                   ypos-28, // SRT 950911
                        NULL );
        }

    if ( XtIsManaged ( legend_dialog_ ) ) XtUnmanageChild ( legend_dialog_ );
    XtManageChild ( legend_dialog_ );
    }

void TileWnd::timezone_dialog_cb()
    {
    /*
     if (timezone_dialog_ == NULL)
    {
          createTimezoneDialog();
    adjustTimezoneDialogPosition();
    }
     if (XtIsManaged(timezone_dialog_)) XtUnmanageChild(timezone_dialog_);  XtManageChild(timezone_dialog_);
    */
    fprintf ( stderr,"%s\n","DEBUG, attempt to adjust timezone" );
    resize();
    }


void TileWnd::configureobsCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->configureobs_cb();
    }

void TileWnd::configureobs_cb()
    {
    if ( obs_dialog_ == NULL )
        {
        Position xpos, ypos;

        // Position control dialog box so it doesn't obscure the plot
        XtVaGetValues ( draw_, XmNx, &xpos, XmNy, &ypos, NULL );

        createObsDialog();
        XtVaSetValues ( obs_dialog_,
                        XmNdefaultPosition,     False,   // so won't be centered - SRT
                        XtNx,                   xpos+375,// SRT 950911
                        XtNy,                   ypos-28, // SRT 950911
                        NULL );
        }

    if ( XtIsManaged ( obs_dialog_ ) ) XtUnmanageChild ( obs_dialog_ );
    XtManageChild ( obs_dialog_ );
    }

void TileWnd::configurecntrCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->configurecntr_cb();
    }

// ATTENTION
void TileWnd::configurecntr_cb()
    {
    if ( cntr_dialog_ == NULL )
        {
        createCntrDialog();
        }
    if ( XtIsManaged ( cntr_dialog_ ) ) XtUnmanageChild ( cntr_dialog_ );
    XtManageChild ( cntr_dialog_ );
    }

// ATTENTION END

void TileWnd::animate_scaleCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    XmScaleCallbackStruct * cbs = ( XmScaleCallbackStruct * ) callData;
    obj->animate_scale_cb ( cbs->value );
    }

void TileWnd::animate_scale_cb ( int value )
    {
    curr_animate_ = animate_frame_ = value;
    if ( value >= vis_->getTimeMax() )   // added SRT 950803; mod ALT 980712
        {
        curr_animate_ = 0;              // added SRT 950803
        return;                         // added ALT 980712
        }
    animateTileCore ( value );
    }


void TileWnd::animateCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->animate_cb();
    }



void TileWnd::animate_cb()
    {
    if ( work_proc_id_ == ( XtWorkProcId ) NULL )
        work_proc_id_ = XtAppAddWorkProc ( app_->appContext(),
                                           &TileWnd::animateTileTrigger,
                                           ( XtPointer ) this );
    }

void TileWnd::stopCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->stop_cb();
    }


void TileWnd::stop_cb()
    {
    if ( work_proc_id_ )
        {
        XtRemoveWorkProc ( work_proc_id_ );
        work_proc_id_ = ( XtWorkProcId ) NULL;
        }
    }


void TileWnd::zoom_scaleCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->zoom_scale_cb ( cbs->value );
    }

void TileWnd::zoom_scale_cb ( int value )
    {
    if ( value <= num_zooms_ )
        {
        curr_zoom_ = value;

        resize();
        }
    }



void TileWnd::closeCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->close_cb();
    }


void TileWnd::close_cb()
    {
    XtUnmanageChild ( animate_dialog_ );
    }



Boolean TileWnd::animateTileTrigger ( XtPointer clientData )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->animateTile();
    return 0;
    }


void TileWnd::animateTile()
    {
    curr_animate_  = animate_frame_++ % vis_->getTimeMax();

    if ( *frameDelayInTenthsOfSecondsP_ )
        registerCurrentTime();

    animateTileCore ( curr_animate_ );

    if ( *frameDelayInTenthsOfSecondsP_ )
        verifyElapsedClockTime ( ( float ) ( *frameDelayInTenthsOfSecondsP_/10.0 ) );

    // Update value on scale widget.
    XtVaSetValues ( animate_scale_, XmNvalue, curr_animate_, NULL );

    }

void TileWnd::synchronizeAnimate ( int animate )
    {
    // SRT 950803 animateTileCore( (curr_animate_+animate) % vis_->getTimeMax() );
    curr_animate_ = animate;     // added SRT 950803
    if ( animate >= vis_->getTimeMax() )   // added SRT 950803
        {
        curr_animate_ = 0;              // added SRT 950803
        return;                         // added ALT 980712
        }
    animateTileCore ( curr_animate_ );   // added SRT 950803
    }

void TileWnd::animateTileCore ( int animate )
    {
    int   jdate, jtime;
    char *dirname ;
    
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::animateTileCore()\n" );
#endif // DIAGNOSTICS

    // Reset foreground color

    setForeground ( "Black" );

    drawSetup ( canvas_, pix_, gc_, &s );
    drawTile ( animate );
    drawOverlays ( animate );

    if ( mapChoices_ == MapCounties && draw_dist_counties_ )
        {
        dirname = getenv ( "PAVE_MAPDIR" ) ;
        if ( dirname && *dirname )
            {
            setForeground ( "gray" );
            sprintf ( vis_->mapName_, "%s/OUTLCOUNTIES", dirname );
            vis_->setMapData();
            drawMap();
            setForeground ( "black" );
            sprintf ( vis_->mapName_, "%s/OUTLSTATES3000", dirname );
            vis_->setMapData();
            }
        else{
            fprintf( stderr, "WARNING:  missing environment variable PAVE_MAPDIR\n" ) ;
            setForeground ( "gray" );
            sprintf ( vis_->mapName_, "OUTLCOUNTIES" );
            vis_->setMapData();
            drawMap();
            setForeground ( "black" );
            sprintf ( vis_->mapName_, "OUTLSTATES3000" );
            vis_->setMapData();
            }
        }
    drawMap();

    drawFrame();
    drawTitles ( vis_->title1_, vis_->title2_, vis_->title3_, vis_->xtitle_, vis_->ytitle_ );
    drawYtics ( 1 /* 4 SRT 950707 */, vis_->plotformatY_,
                ( getenv ( "TILEYLABELSONRIGHT" ) !=NULL ) && ( strlen ( getenv ( "TILEYLABELSONRIGHT" ) ) ),0 );
    drawXtics ( 1 /* 4 SRT 950707 */, vis_->plotformatX_ );
    initColorLegend ( canvas_, pix_, gc_, &s,
                      -1.0, -1.0 /* SRT 950721 vis_->info->grid_min, vis_->info->grid_max */,
                      vis_->getUnits(), &vis_->title1_, &vis_->title2_, &vis_->title3_  );
    if ( vectors_on_ && scale_vectors_on_ )
        {
        float        x, y, x2, y2;
        char str[256];
        sprintf ( str,"%2.1f",vector_scale_ );
        x = s.scalex ( s.xmin_ );
        x2 = s.scalex ( s.xmax_ );
        x = x + 0.9* ( x2-x );
        y = y2 = s.scaley ( s.ymin_ ) + 30;

        //       setForeground("red");
        drawVect ( x, y, x2, y2 );
        XDrawString ( XtDisplay ( canvas_ ), pix_, gc_,
                      ( int ) ( 0.5* ( x+x2 ) ), ( int ) ( y+20 ), str, strlen ( str ) );
        //       setForeground("black");
        }
    if ( tiles_on_ ) drawColorLegend ( 5, 10 );
    jdate = vis_->info->sdate[animate];
    jtime = vis_->info->stime[animate];
    if ( timezone_ != 0 )
        {
        nextimec ( &jdate, &jtime, sec2timec ( timezone_*3600 ) );
        }
    drawTimeStamp ( animate, jdate, jtime, tzname_ );
    if ( tiles_on_ ) drawMinMax ( animate );


    // Copy data.pix to drawing area widget.
    XCopyArea ( XtDisplay ( canvas_ ), pix_, XtWindow ( canvas_ ), gc_,
                0, 0, width_, height_,
                0, 0 );

    // Reset foreground color
    setForeground ( "Black" );
    }


void TileWnd::drawTile ( int t )
    {
    XGCValues        values;
    unsigned long foreground;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::drawTile()\n" );
#endif // DIAGNOSTICS

    fillTheBackgroundWithWhite();

    if ( tiles_on_ )
        if ( !smooth_plots_on_ )
            drawBlockTile ( t );
        else
            drawSmoothTile ( t );
    if ( vectors_on_ )
        {
        char *color_str;
        color_str = getenv ( "VECTOR_COLOR" );
        if ( color_str != NULL )
            {
            /* save the current foreground color */
            XGetGCValues ( dpy_, gc_, GCForeground, &values );
            foreground = values.foreground;
            setForeground ( color_str );
            }
        drawVectors ( t );
        if ( color_str != NULL )
            {
            /* restore the current foreground color */
            XSetForeground ( dpy_, gc_, foreground );
            }
        }

    if ( vectors_on_ && overlay_ )
        {
        // new experimental stuff
        int linewidth;
        int vec_skip;
        VIS_DATA *uwind_tmp, *vwind_tmp;

        uwind_tmp = uwind_;
        vwind_tmp = vwind_;
        uwind_ = uwind_obs_;
        vwind_ = vwind_obs_;
        vec_skip = legend_nskip_;
        legend_nskip_ = 1;
        XGetGCValues ( dpy_, gc_, GCLineWidth, &values );
        linewidth = values.line_width;
        values.line_width = 3;
        XChangeGC ( dpy_, gc_, GCLineWidth, &values );
        // end new experimental stuff
        char *color_str;
        color_str = getenv ( "VECTOR_OBS_COLOR" );
        if ( color_str != NULL )
            {
            /* save the current foreground color */
            XGetGCValues ( dpy_, gc_, GCForeground, &values );
            foreground = values.foreground;
            setForeground ( color_str );
            }
        drawVectors ( t );
        // new experimental stuff - part 2
        uwind_ = uwind_tmp;
        vwind_ = vwind_tmp;
        legend_nskip_ = vec_skip;
        values.line_width = linewidth;
        XChangeGC ( dpy_, gc_, GCLineWidth, &values );
        // end new experimental stuff - part 2
        if ( color_str != NULL )
            {
            /* restore the current foreground color */
            XSetForeground ( dpy_, gc_, foreground );
            }
        }

    }


void TileWnd::drawGridLines ( void )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::drawGridLines()\n" );
#endif // DIAGNOSTICS

    int i, j, i_xmin, i_xmax, i_ymin, i_ymax, top, bottom, left, right;
    float dx, dy, pos;
    char *gridLineColor;
    char white_color[]="White";
    char black_color[]="Black";

    i_xmin = ( int ) floor ( zoom_[curr_zoom_].GRID_X_MIN_ );
    i_xmax = ( int ) floor ( zoom_[curr_zoom_].GRID_X_MAX_ );
    i_ymin = ( int ) floor ( zoom_[curr_zoom_].GRID_Y_MIN_ );
    i_ymax = ( int ) floor ( zoom_[curr_zoom_].GRID_Y_MAX_ );
    dx = s.fscalex ( s.xmin_+1 )-s.fscalex ( s.xmin_ );
    dy = s.fscaley ( s.ymin_+1 )-s.fscaley ( s.ymin_ );
    top = s.scaley ( s.ymax_ );
    bottom = s.scaley ( s.ymin_ );
    left = s.scalex ( s.xmin_ );
    right = s.scalex ( s.xmax_ );

    pos = s.scaley ( i_ymin+1 );
    for ( i=vis_->row_min_; i < vis_->row_max_; i++ )
        if ( ( i>=i_ymin+1 ) && ( i<=i_ymax+1 ) )
            {
            gridLineColor = i%10?white_color:black_color;
            setForeground ( gridLineColor );
            XDrawLine ( XtDisplay ( canvas_ ),
                        pix_,
                        gc_,
                        left,
                        ( int ) pos,
                        right,
                        ( int ) pos );
            pos += dy;
            }

    pos = s.scalex ( i_xmin+1 );
    for ( j=vis_->col_min_; j < vis_->col_max_; j++ )
        if ( ( j>=i_xmin+1 ) && ( j<=i_xmax+1 ) )
            {
            gridLineColor = j%10?white_color:black_color;
            setForeground ( gridLineColor );
            XDrawLine ( XtDisplay ( canvas_ ),
                        pix_,
                        gc_,
                        ( int ) pos,
                        bottom,
                        ( int ) pos,
                        top );
            pos += dx;
            }

    setForeground ( "Black" );
    }



void TileWnd::fillTheBackgroundWithWhite ( void )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::fillTheBackgroundWithWhite()\n" );
#endif // DIAGNOSTICS

    GC           whiteGC;
    XGCValues        values;
    int              scr;
    scr = DefaultScreen ( XtDisplay ( canvas_ ) );
    values.foreground = WhitePixel ( XtDisplay ( canvas_ ), scr );
    values.background = BlackPixel ( XtDisplay ( canvas_ ), scr );
    whiteGC = XCreateGC ( XtDisplay ( canvas_ ), RootWindow ( XtDisplay ( canvas_ ),scr ),
                          ( GCForeground|GCBackground ), &values );

    XFillRectangle ( XtDisplay ( canvas_ ), pix_, whiteGC,
                     ( int ) s.scalex ( s.xmin_ ), ( int ) s.scaley ( s.ymax_ ),
                     ( int ) s.scalex ( s.xmax_ )- ( int ) s.scalex ( s.xmin_ ),
                     ( int ) s.scaley ( s.ymin_ )- ( int ) s.scaley ( s.ymax_ ) );
    XFreeGC ( XtDisplay ( canvas_ ), whiteGC );
    }


void TileWnd::drawSmoothTile ( int t )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::drawSmoothTile()\n" );
#endif // DIAGNOSTICS

    /*
    drawSmoothTile - draw a smooth contour plot (rather than a tile plot)
    using bilinear interpolation.

    Please see NUMERICAL RECIPES IN C (by William H. Press,
    Brian P. Flannery, Saul A. Teukolsky, and William T. Vetterling,
    Press Syndicate of the University of Cambridge, New York, N.Y.,
    1988, pp 104-105) for further details regarding this method.
    The interpolation logic came directly from this source.

    Its definitely inefficient right now and needs optimization - SRT
    */

    int          i, j, cindex, ti, tj;
    int          idx = 0, jdx = 0, inc = 0, range;
    int          i_xmin, i_xmax, i_ymin, i_ymax, first_time = 1;
    float    tt, uu, thisVal;
    float        slower, sleft, sheight, swidth;// for the cell we're
    // filling in, which
    // has a data value at
    // the CENTER
    float        y1, // lowerLeft_val,   for the cell with
                 y2, // lowerRight_val,  data values at all
                 y3, // upperRight_val;  four CORNERS, we'll
                 y4; // upperLeft_val,   interpolate to get
    //                  a value for the current
    //                  pixel of the cell we're
    //                  filling in


    swidth =  s.scalex ( s.xmin_+1.0 )-s.scalex ( s.xmin_ );
    sheight = s.scaley ( s.ymin_ )-s.scaley ( s.ymin_+1.0 );

    i_xmin = ( int ) floor ( zoom_[curr_zoom_].GRID_X_MIN_ );
    i_xmax = ( int ) floor ( zoom_[curr_zoom_].GRID_X_MAX_ );
    i_ymin = ( int ) floor ( zoom_[curr_zoom_].GRID_Y_MIN_ );
    i_ymax = ( int ) floor ( zoom_[curr_zoom_].GRID_Y_MAX_ );



    /*
       loop over all the cells; we actually have data from
       (col_min_,row_min_) .. (col_max_,row_max_) inclusive.
       When we're doing tile plots one cell is drawn with each point
       at the center.  But when doing smooth plots, these data points
       are used as corners.  So we range the inner loop over the range
       (col_min_,row_min_) .. (col_max_-1,row_max_-1), inclusive,
       and have each (i,j) value represent the lower left of the
       current cell being drawn
    */

    for ( j=vis_->col_min_; j <= vis_->col_max_; j++ )
        for ( i= vis_->row_min_; i <= vis_->row_max_; i++ )
            {
            if ( ( j>=i_xmin+1 ) && ( j<=i_xmax+1 ) && // SRT added +1's 950707
                    ( i>=i_ymin+1 ) && ( i<=i_ymax+1 ) ) // SRT added +1's 950707
                {
                range = ( int ) ( floor ( zoom_[curr_zoom_].GRID_X_MAX_ ) -
                                  floor ( zoom_[curr_zoom_].GRID_X_MIN_ ) )+1;

                jdx = inc++ % range;
                if ( ( jdx == 0 ) && ( inc >= range-1 ) )
                    idx++;

                if ( ( i < vis_->row_max_ ) && ( j < vis_->col_max_ ) )
                    {
                    // calculate lower left corner of this cell
                    // being drawn, remembering to shift one
                    // up and to the right since data is at
                    // the *corners* not the center

                    sleft = s.scalex ( s.xmin_ + ( float ) jdx ) + swidth/2;
                    slower = s.scaley ( s.ymin_ + ( float ) idx ) - sheight/2;


                    // lower left value

                    y1 = vis_->info->grid
                         [INDEX
                          (
                              j-vis_->col_min_,   // Eng reversed i & j SRT
                              i-vis_->row_min_,   // Eng reversed i & j SRT
                              0,
                              t,
                              vis_->col_max_-vis_->col_min_+1, // SRT
                              vis_->row_max_-vis_->row_min_+1, // SRT
                              1
                          )
                         ];

                    // lower right value
                    y2 = vis_->info->grid
                         [INDEX
                          (
                              j-vis_->col_min_+1, // Eng reversed i & j SRT
                              i-vis_->row_min_,   // Eng reversed i & j SRT
                              0,
                              t,
                              vis_->col_max_-vis_->col_min_+1, // SRT
                              vis_->row_max_-vis_->row_min_+1, // SRT
                              1
                          )
                         ];

                    // upper right value
                    y3 = vis_->info->grid
                         [INDEX
                          (
                              j-vis_->col_min_+1, // Eng reversed i & j SRT
                              i-vis_->row_min_+1, // Eng reversed i & j SRT
                              0,
                              t,
                              vis_->col_max_-vis_->col_min_+1, // SRT
                              vis_->row_max_-vis_->row_min_+1, // SRT
                              1
                          )
                         ];

                    // upper left value
                    y4 = vis_->info->grid
                         [INDEX
                          (
                              j-vis_->col_min_,   // Eng reversed i & j SRT
                              i-vis_->row_min_+1, // Eng reversed i & j SRT
                              0,
                              t,
                              vis_->col_max_-vis_->col_min_+1, // SRT
                              vis_->row_max_-vis_->row_min_+1, // SRT
                              1
                          )
                         ];


                    // loop over all the pixels in this cell
                    for ( ti = 0; ti <= ( int ) swidth; ti++ )
                        {
                        float one_minus_tt;

                        tt = ti / swidth;
                        one_minus_tt = 1.0-tt;

                        for ( tj = 0; tj <= ( int ) sheight; tj++ )
                            {
                            float one_minus_uu;

                            uu = tj / sheight;
                            one_minus_uu = 1.0 - uu;

                            thisVal = one_minus_tt*one_minus_uu*y1 +
                                      tt*one_minus_uu*y2 +
                                      tt*uu*y3 +
                                      one_minus_tt*uu*y4;
                            if ( ( cindex = colorIndex ( thisVal ) ) >= 0 )
                                XDrawPoint
                                (
                                    XtDisplay ( canvas_ ),
                                    pix_,
                                    color_gc_table_[cindex],
                                    ( int ) ( sleft+ti ),
                                    ( int ) ( slower-tj )
                                );
                            else
                                {
                                if ( first_time )
                                    {
                                    first_time = 0;
                                    fprintf ( stderr,
                                              "One or more cells probably has bogus data.\n"
                                              "For example, I just interpolated the value %g ...\n"
                                              "No color will be shown in pixels with bogus data.\n",
                                              thisVal );
                                    }
                                }
                            }
                        }

#ifdef DIAGNOSTICS

                    // draw test points to indicate where the raw
                    // data is - the *center* of a tile plot's grid cell;
                    // the *corner* of a smooth plot's grid cell
                    if ( colorIndex ( vis_->info->grid_max ) > 0 )
                        {
                        XDrawPoint
                        (
                            XtDisplay ( canvas_ ),
                            pix_,
                            color_gc_table_[colorIndex ( vis_->info->grid_max )],
                            ( int ) sleft,
                            ( int ) ( slower-sheight )
                        );
                        XDrawPoint
                        (
                            XtDisplay ( canvas_ ),
                            pix_,
                            color_gc_table_[colorIndex ( vis_->info->grid_max )],
                            ( int ) ( sleft+swidth ),
                            ( int ) ( slower-sheight )
                        );
                        XDrawPoint
                        (
                            XtDisplay ( canvas_ ),
                            pix_,
                            color_gc_table_[colorIndex ( vis_->info->grid_max )],
                            ( int ) ( sleft+swidth ),
                            ( int ) slower
                        );
                        XDrawPoint
                        (
                            XtDisplay ( canvas_ ),
                            pix_,
                            color_gc_table_[colorIndex ( vis_->info->grid_max )],
                            ( int ) sleft,
                            ( int ) slower
                        );
                        }
#endif // DIAGNOSTICS

                    } // if

                } // outermost if

            } // outer loop

    // Reset foreground color
    setForeground ( "Black" );
    }



void TileWnd::drawBlockTile ( int t )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::drawBlockTile()\n" );
#endif // DIAGNOSTICS

    Display *display = XtDisplay ( canvas_ );

    GC  gc;

    int i, j, ulx, index, cindex, first_time = 1;

    // *0* based range of cells currently being displayed
    // i.e. 0,0 corresponds to xorig, yorig
    int     i_xmin = ( int ) floor ( zoom_[curr_zoom_].GRID_X_MIN_ ),
            i_xmax = ( int ) floor ( zoom_[curr_zoom_].GRID_X_MAX_ ),
            i_ymin = ( int ) floor ( zoom_[curr_zoom_].GRID_Y_MIN_ ),
            i_ymax = ( int ) floor ( zoom_[curr_zoom_].GRID_Y_MAX_ );


    // the bounding box info for the tiles in pixels
    float   tileLLXPixel   = s.scalex ( s.xmin_ ),
            tileURXPixel   = s.scalex ( s.xmax_ ),
            tileLLYPixel   = s.scaley ( s.ymin_ ),
            tileURYPixel   = s.scaley ( s.ymax_ ),
            tileDXInPixels = ( tileURXPixel-tileLLXPixel ) / ( i_xmax-i_xmin+1.0 ),
            tileDYInPixels = ( tileURYPixel-tileLLYPixel ) / ( i_ymax-i_ymin+1.0 ),
            val;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "i_xmin = %d\n", i_xmin );
    fprintf ( stderr, "i_xmax = %d\n", i_xmax );
    fprintf ( stderr, "i_ymin = %d\n", i_ymin );
    fprintf ( stderr, "i_ymax = %d\n", i_ymax );
    fprintf ( stderr, "vis_->col_min_ = %d\n", vis_->col_min_ );
    fprintf ( stderr, "vis_->col_max_ = %d\n", vis_->col_max_ );
    fprintf ( stderr, "vis_->row_min_ = %d\n", vis_->row_min_ );
    fprintf ( stderr, "vis_->row_max_ = %d\n", vis_->row_max_ );
    fprintf ( stderr, "tileLLXPixel = %f\n", tileLLXPixel );
    fprintf ( stderr, "tileURXPixel = %f\n", tileURXPixel );
    fprintf ( stderr, "tileLLYPixel = %f\n", tileLLYPixel );
    fprintf ( stderr, "tileURYPixel = %f\n", tileURYPixel );
    fprintf ( stderr, "tileDXInPixels = %f\n", tileDXInPixels );
    fprintf ( stderr, "tileDYInPixels = %f\n", tileDYInPixels );
#endif // DIAGNOSTICS


    // Loop over the cells in the currently zoomed region,
    // filling them in with the appropriate colors

    char *color_str;
    GC gc_bckgnd = NULL;
    unsigned long rgb;
    XColor color;
    gc_bckgnd = XCreateGC ( display,
                            DefaultRootWindow ( display ),
                            ( unsigned long ) NULL, ( XGCValues * ) NULL );

    if ( ( color_str=getenv ( "MISSING_DATA_COLOR" ) ) != NULL )
        {
        rgb = atol ( color_str );
        color.red   = ( unsigned short ) ( ( rgb & 0xff0000 ) >> 8 );
        color.green = ( unsigned short ) ( ( rgb & 0x00ff00 ) );
        color.blue  = ( unsigned short ) ( ( rgb & 0x0000ff ) << 8 );

        if ( !XAllocColor ( display, DefaultColormap ( display, DefaultScreen ( display ) ),
                            &color ) )
            {
            fprintf ( stderr, "Can't allocate color for MISSING data background\n" );
            rgb = WhitePixel ( display, DefaultScreen ( display ) );
            }
        else
            {
            rgb = color.pixel;
            }
        }
    else
        {
        rgb = WhitePixel ( display, DefaultScreen ( display ) );
        }
    XSetForeground ( display,
                     gc_bckgnd,
                     rgb );

    for ( j=i_ymin; j <= i_ymax; j++ )
        {
        for ( i= i_xmin; i <= i_xmax; i++ )
            {
            ulx = ( int ) ( tileLLXPixel+ ( i-i_xmin ) *tileDXInPixels );
        
            index = INDEX ( i-vis_->col_min_+1, j-vis_->row_min_+1, 0, t,
                            vis_->col_max_-vis_->col_min_+1,
                            vis_->row_max_-vis_->row_min_+1, 1 );
            val = vis_->info->grid[index];
            if ( isnanf ( val ) )
                {
                XFillRectangle
                (
                    display, pix_, gc_bckgnd,
                    ulx,                          // ULX of tile
                    ( int ) ( tileLLYPixel+ ( j-i_ymin+1 ) *tileDYInPixels ), // ULY of tile
                    ( unsigned int ) ( tileDXInPixels+1 ),   // width
                    ( unsigned int ) ( -tileDYInPixels+1 )   // height
                );
                continue;
                }
            cindex = colorIndex ( val );

            if ( cindex >= 0 )
                {
                gc = color_gc_table_[cindex];
                XFillRectangle
                (
                    display, pix_, gc,
                    ulx,                          // ULX of tile
                    ( int ) ( tileLLYPixel+ ( j-i_ymin+1 ) *tileDYInPixels ), // ULY of tile
                    ( unsigned int ) ( tileDXInPixels+1 ),    // width
                    ( unsigned int ) ( -tileDYInPixels+1 )    // height
                );
                }
            else
                {
                if ( first_time )
                    {
                    first_time = 0;
                    fprintf ( stderr,
                              "One or more cells probably has bogus data.\n"
                              "For example, I just found %g ...\n"
                              "No color will be shown in cells with bogus data.\n",
                              val );
                    }
                }
            }
        }

    // Reset foreground color
    setForeground ( "Black" );
    if ( gc_bckgnd ) XFreeGC ( display, gc_bckgnd );
    }



/************************************************************
drawVect - draws a vector from (n1,n2) to (n3,n4)
************************************************************/
#define nint(x) int((x)+0.5)
#define DRAW_CALM_CIRCLE
#define DEFAULT_HEADSIZE 6
void TileWnd::drawVect ( float n1, float n2, float n3, float n4 )
    {
    drawVect ( n1, n2, n3, n4, DEFAULT_HEADSIZE );
    }

void TileWnd::drawVect ( float n1, float n2, float n3, float n4, int headsize )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::drawVect(%g,%g,%g,%g)\n", n1, n2, n3, n4 );
#endif // DIAGNOSTICS

    float n5, n6, n7, n8, dx, dy;
    float r;
    XPoint arrowhead[3];

    /*    C1     -  ARROW HEAD LENGTH SCALE FACTOR - EACH SIDE OF THE ARROW
                    HEAD IS THIS LONG RELATIVE TO THE LENGTH OF THE ARROW
          ST,CT  -  SIN AND COS OF THE ARROW HEAD ANGLE */
#ifdef ARROW_45
    // head angle 45 degrees
#define    ST 0.382683432365090
#define    CT 0.923879532511287
#else
    // head angle 60 degrees
#define    ST 0.5
#define    CT 0.8660254
#endif

    dx = n3 - n1;
    dy = n4 - n2;
    r = sqrt ( dx*dx+dy*dy );
    if ( r==0.0 )
        {
#ifdef DRAW_CALM_CIRCLE
#define CIRCLE_RADIUS nint(0.38490018*headsize)
#define CIRCLE_DIAMETER (2*CIRCLE_RADIUS)
        XDrawArc ( XtDisplay ( canvas_ ), pix_, gc_,
                   nint ( n1 )-CIRCLE_RADIUS, nint ( n2 )-CIRCLE_RADIUS,
                   CIRCLE_DIAMETER, CIRCLE_DIAMETER, 0, 360*64 );
        return;
#else
        dx = 0.0;
        dy = -1.0; // calm wind arrowhead points to North
#endif
        }
    else
        {
        dx /= r;
        dy /= r;
        }
    n5 = n3 - headsize * ( CT * dx - ST * dy );
    n6 = n4 - headsize * ( CT * dy + ST * dx );
    n7 = n3 - headsize * ( CT * dx + ST * dy );
    n8 = n4 - headsize * ( CT * dy - ST * dx );

    if ( fill_arrowheads_ )
        {
        arrowhead[0].x = nint ( n3 );
        arrowhead[0].y = nint ( n4 );
        arrowhead[1].x = nint ( n5 );
        arrowhead[1].y = nint ( n6 );
        arrowhead[2].x = nint ( n7 );
        arrowhead[2].y = nint ( n8 );

        n3 -= ( headsize-1 ) * CT * dx;
        n4 -= ( headsize-1 ) * CT * dy;

        XDrawLine ( XtDisplay ( canvas_ ), pix_, gc_,
                    nint ( n1 ), nint ( n2 ), nint ( n3 ), nint ( n4 ) );

        XFillPolygon ( XtDisplay ( canvas_ ), pix_, gc_,
                       arrowhead, 3, Convex, CoordModeOrigin );
        }
    else
        {
        XDrawLine ( XtDisplay ( canvas_ ), pix_, gc_,
                    nint ( n1 ), nint ( n2 ), nint ( n3 ), nint ( n4 ) );

        XDrawLine ( XtDisplay ( canvas_ ), pix_, gc_,
                    nint ( n3 ), nint ( n4 ), nint ( n5 ), nint ( n6 ) );
        XDrawLine ( XtDisplay ( canvas_ ), pix_, gc_,
                    nint ( n3 ), nint ( n4 ), nint ( n7 ), nint ( n8 ) );
        }
    }



void TileWnd::drawVectors ( int t )
    {
#ifdef DIAGNOSTICS
    int mx, my, mt;
    fprintf ( stderr, "Enter TileWnd::drawVectors()\n" );
#endif // DIAGNOSTICS

    // info from map_info for UAM and netCDF data formats
    float           llx[2], lly[2], urx[2], ury[2], grid_type[2],
                    xorig[2], yorig[2], xcell[2], ycell[2], xcent[2],
                    ycent[2], p_gam[2], p_bet[2], p_alp[2];
    int             utm_zone[2], ncol[2], nrow[2];

    float       tileLLXmapCoord,
                tileLLYmapCoord,
                tileURXmapCoord,
                tileURYmapCoord,
                tileDXmapCoord,
                tileDYmapCoord,
                vectorLLXmapCoord,
                vectorLLYmapCoord,
                vectorURXmapCoord,
                vectorURYmapCoord,
                vectorDXmapCoord,
                vectorDYmapCoord,
                tileLLXPixel,
                tileLLYPixel,
                tileURXPixel,
                tileURYPixel,
                tileDXInPixels,
                tileDYInPixels,
                vectorLLXPixel,
                vectorLLYPixel,
                vectorURXPixel,
                vectorURYPixel,
                vectorDXInPixels,
                vectorDYInPixels;

    // increment to use when looping over vectors
    // sometimes this is > 1 when they are too close together;
    // we'll keep the vectors at least 20 pixels apart in each
    // direction
    int         xinc, yinc;
    float       xdist, ydist; // distance in pixels between vectors
    float       x, y, x2, y2;

    float       maxMagnitude, // max vector length
                mag,
                dx,
                dy,
                scale = 1.75 * vector_scale_;

    // *0* based range of cells currently being displayed
    // i.e. 0,0 corresponds to xorig, yorig
    int         i_xmin = ( int ) floor ( zoom_[curr_zoom_].GRID_X_MIN_ );
    int         i_xmax = ( int ) floor ( zoom_[curr_zoom_].GRID_X_MAX_ );
    int         i_ymin = ( int ) floor ( zoom_[curr_zoom_].GRID_Y_MIN_ );
    int         i_ymax = ( int ) floor ( zoom_[curr_zoom_].GRID_Y_MAX_ );

    int         i, j, ts;
    long        index;
    VIS_DATA   *vdata[3];
    enum        dataset_type dataset[3];
    float       signx, signy;

#ifdef DIAGNOSTICS
    fflush ( stderr );
    fflush ( stdout );
    fprintf ( stderr, "Enter drawVectors(%d)\n", t );
#endif // DIAGNOSTICS

    vdata[0] = vis_->info;
    vdata[1] = uwind_;
    vdata[2] = vwind_;


    // grab the map_info contents from the tile and vector structs
    for ( i=0; i < 2; i++ )
        {
        /* parse the map_info string */
        if ( sscanf ( vdata[i]->map_info, "%g%g%g%g%g%g%g%g%g%g%d%d",
                      &grid_type[i], &xorig[i], &yorig[i], &xcell[i],
                      &ycell[i], &xcent[i], &ycent[i], &p_gam[i],
                      &p_bet[i], &p_alp[i], &ncol[i], &nrow[i] ) == 12 )
            dataset[i] = netCDF_DATA;
        else if ( sscanf ( vdata[i]->map_info, "%g%g%g%g%d%d%d",
                           &llx[i], &lly[i], &urx[i], &ury[i], &utm_zone[i],
                           &ncol[i], &nrow[i] ) == 7 )
            dataset[i] = UAM_DATA;
        else
            dataset[i] = UNDETERMINED;

        switch ( dataset[i] )
            {
            case UAM_DATA:
            case UAMV_DATA:
                //    sscanf(vdata[i]->map_info, "%g%g%g%g%d%d%d",
                //              &llx[i], &lly[i], &urx[i], &ury[i], &utm_zone[i],
                //              &ncol[i], &nrow[i]);
#ifdef DIAGNOSTICS
                fprintf ( stderr, "\nvdata[%d]->dataset == UAM_DATA or UAMV_DATA\n", i );
                fprintf ( stderr, "llx[%d]==%g lly[%d]==%g urx[%d]==%g ury[%d]==%g\n",
                          i, llx[i], i, lly[i], i, urx[i], i, ury[i] );
                fprintf ( stderr, "utm_zone[%d]==%d ncol[%d]==%d nrow[%d]==%d\n",
                          i, utm_zone[i], i, ncol[i], i, nrow[i] );
#endif // DIAGNOSTICS
                break;

            case netCDF_DATA:
                //    sscanf(vdata[i]->map_info, "%g%g%g%g%g%g%g%g%g%g%d%d",
                //              &grid_type[i], &xorig[i], &yorig[i], &xcell[i],
                //              &ycell[i], &xcent[i], &ycent[i], &p_gam[i],
                //              &p_bet[i], &p_alp[i], &ncol[i], &nrow[i]);
#ifdef DIAGNOSTICS
                fprintf ( stderr, "\nvdata[%d]->dataset == netCDF_DATA\n", i );
                fprintf ( stderr,
                          "grid_type[%d]==%g xorig[%d]==%g yorig[%d]==%g\n",
                          i, grid_type[i], i, xorig[i], i, yorig[i] );
                fprintf ( stderr,
                          "xcell[%d]==%g ycell[%d]==%g xcent[%d]==%g ycent[%d]==%g\n",
                          i, xcell[i], i, ycell[i], i, xcent[i], i, ycent[i] );
                fprintf ( stderr,
                          "p_gam[%d]==%g p_bet[%d]==%g p_alp[%d]==%g\n",
                          i, p_gam[i], i, p_bet[i], i, p_alp[i] );
                fprintf ( stderr,
                          "ncol[%d]==%d nrow[%d]==%d\n",
                          i, ncol[i], i, nrow[i] );
#endif // DIAGNOSTICS
                break;

            case UNDETERMINED:
            default:
                fprintf ( stderr, "vdata[%d]->dataset unknown dataset type %d!\n",
                          i, dataset );
                return;
            }
        }


    // find the bounding box info for the tiles in map coordinates
    switch ( dataset[0] )
        {
        case UAM_DATA:
        case UAMV_DATA:
            tileDXmapCoord  = ( urx[0]-llx[0] ) /ncol[0];
            tileDYmapCoord  = ( ury[0]-lly[0] ) /nrow[0];
            tileLLXmapCoord = llx[0]+tileDXmapCoord*i_xmin;
            tileURXmapCoord = llx[0]+tileDXmapCoord* ( i_xmax+1 );
            tileLLYmapCoord = lly[0]+tileDYmapCoord*i_ymin;
            tileURYmapCoord = lly[0]+tileDYmapCoord* ( i_ymax+1 );
            break;

        case netCDF_DATA:
            tileDXmapCoord  = xcell[0];
            tileDYmapCoord  = ycell[0];
            tileLLXmapCoord = xorig[0]+tileDXmapCoord*i_xmin;
            tileURXmapCoord = xorig[0]+tileDXmapCoord* ( i_xmax+1 );
            tileLLYmapCoord = yorig[0]+tileDYmapCoord*i_ymin;
            tileURYmapCoord = yorig[0]+tileDYmapCoord* ( i_ymax+1 );
            break;

        case UNDETERMINED:
        default:
            fprintf ( stderr,
                      "Unknown dataset type in TileWnd::drawVectors()!\n" );
            return;
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "\ntileDXmapCoord  == %f\n", tileDXmapCoord );
    fprintf ( stderr, "tileDYmapCoord  == %f\n", tileDYmapCoord );
    fprintf ( stderr, "tileLLXmapCoord == %f\n", tileLLXmapCoord );
    fprintf ( stderr, "tileURXmapCoord == %f\n", tileURXmapCoord );
    fprintf ( stderr, "tileLLYmapCoord == %f\n", tileLLYmapCoord );
    fprintf ( stderr, "tileURYmapCoord == %f\n", tileURYmapCoord );
#endif // DIAGNOSTICS



    // find the bounding box info for the vectors in map coordinates
    switch ( dataset[1] )
        {
        case UAM_DATA:
        case UAMV_DATA:
            vectorDXmapCoord  = ( urx[1]-llx[1] ) /ncol[1];
            vectorDYmapCoord  = ( ury[1]-lly[1] ) /nrow[1];
            vectorLLXmapCoord = llx[1]+vectorDXmapCoord* ( vdata[1]->col_min-1 );
            vectorURXmapCoord = llx[1]+vectorDXmapCoord* ( vdata[1]->col_max );
            vectorLLYmapCoord = lly[1]+vectorDYmapCoord* ( vdata[1]->row_min-1 );
            vectorURYmapCoord = lly[1]+vectorDYmapCoord* ( vdata[1]->row_max );
            break;

        case netCDF_DATA:
            vectorDXmapCoord  = xcell[1];
            vectorDYmapCoord  = ycell[1];
            vectorLLXmapCoord = xorig[1]+vectorDXmapCoord* ( vdata[1]->col_min-1 );
            vectorURXmapCoord = xorig[1]+vectorDXmapCoord* ( vdata[1]->col_max );
            vectorLLYmapCoord = yorig[1]+vectorDYmapCoord* ( vdata[1]->row_min-1 );
            vectorURYmapCoord = yorig[1]+vectorDYmapCoord* ( vdata[1]->row_max );
            break;

        case UNDETERMINED:
        default:
            fprintf ( stderr,
                      "Unknown dataset type in TileWnd::drawVectors()!\n" );
            return;
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "\nvectorDXmapCoord  == %f\n", vectorDXmapCoord );
    fprintf ( stderr, "vectorDYmapCoord  == %f\n", vectorDYmapCoord );
    fprintf ( stderr, "vectorLLXmapCoord == %f\n", vectorLLXmapCoord );
    fprintf ( stderr, "vectorURXmapCoord == %f\n", vectorURXmapCoord );
    fprintf ( stderr, "vectorLLYmapCoord == %f\n", vectorLLYmapCoord );
    fprintf ( stderr, "vectorURYmapCoord == %f\n", vectorURYmapCoord );
#endif // DIAGNOSTICS


    // find the bounding box info for the tiles in pixels
    tileLLXPixel   = s.scalex ( s.xmin_ );
    tileURXPixel   = s.scalex ( s.xmax_ );
    tileLLYPixel   = s.scaley ( s.ymin_ );
    tileURYPixel   = s.scaley ( s.ymax_ );
    tileDXInPixels = ( tileURXPixel-tileLLXPixel ) / ( i_xmax-i_xmin+1.0 );
    tileDYInPixels = ( tileURYPixel-tileLLYPixel ) / ( i_ymax-i_ymin+1.0 );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "\ntileLLXPixel  == %f\n", tileLLXPixel );
    fprintf ( stderr, "tileURXPixel  == %f\n", tileURXPixel );
    fprintf ( stderr, "tileLLYPixel == %f\n", tileLLYPixel );
    fprintf ( stderr, "tileURYPixel == %f\n", tileURYPixel );
    fprintf ( stderr, "tileDXInPixels == %f\n", tileDXInPixels );
    fprintf ( stderr, "tileDYInPixels == %f\n", tileDYInPixels );
#endif // DIAGNOSTICS


    // find the bounding box info for the vectors in pixels
    vectorLLXPixel   = tileLLXPixel+tileDXInPixels*
                       ( ( vectorLLXmapCoord-tileLLXmapCoord ) /tileDXmapCoord );
    vectorURXPixel   = tileLLXPixel+tileDXInPixels*
                       ( ( vectorURXmapCoord-tileLLXmapCoord ) /tileDXmapCoord );
    vectorLLYPixel   = tileLLYPixel+tileDYInPixels*
                       ( ( vectorLLYmapCoord-tileLLYmapCoord ) /tileDYmapCoord );
    vectorURYPixel   = tileLLYPixel+tileDYInPixels*
                       ( ( vectorURYmapCoord-tileLLYmapCoord ) /tileDYmapCoord );
    vectorDXInPixels = ( vectorURXPixel-vectorLLXPixel ) /
                       ( vdata[1]->col_max-vdata[1]->col_min+1 );
    vectorDYInPixels = ( vectorURYPixel-vectorLLYPixel ) /
                       ( vdata[1]->row_max-vdata[1]->row_min+1 );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "\nvectorLLXPixel  == %f\n", vectorLLXPixel );
    fprintf ( stderr, "vectorURXPixel  == %f\n", vectorURXPixel );
    fprintf ( stderr, "vectorLLYPixel == %f\n", vectorLLYPixel );
    fprintf ( stderr, "vectorURYPixel == %f\n", vectorURYPixel );
    fprintf ( stderr, "vectorDXInPixels == %f\n", vectorDXInPixels );
    fprintf ( stderr, "vectorDYInPixels == %f\n", vectorDYInPixels );
#endif // DIAGNOSTICS


    // based on the size in pixels of the the vector area to be drawn on
    // and the number of vectors to be drawn in each direction, do we
    // have to take every 2nd, 3rd, etc in either the X or Y direction?
    xinc = ( ( vectorDXInPixels>  20 ) ? 1 : ( int ) ceil ( ( double ) 20.0/vectorDXInPixels ) );
    yinc = ( ( vectorDYInPixels< -20 ) ? 1 : ( int ) ceil ( ( double )-20.0/vectorDYInPixels ) );
    xinc = yinc = legend_nskip_;
    xdist = xinc * vectorDXInPixels;
    ydist = yinc * vectorDYInPixels;


    vectorLLXPixel   = tileLLXPixel+tileDXInPixels*
                       ( ( vectorLLXmapCoord-tileLLXmapCoord ) /tileDXmapCoord );
    vectorURXPixel   = tileLLXPixel+tileDXInPixels*
                       ( ( vectorURXmapCoord-tileLLXmapCoord ) /tileDXmapCoord );
    vectorLLYPixel   = tileLLYPixel+tileDYInPixels*
                       ( ( vectorLLYmapCoord-tileLLYmapCoord ) /tileDYmapCoord );
    vectorURYPixel   = tileLLYPixel+tileDYInPixels*
                       ( ( vectorURYmapCoord-tileLLYmapCoord ) /tileDYmapCoord );
    vectorDXInPixels = ( vectorURXPixel-vectorLLXPixel ) /
                       ( vdata[1]->col_max-vdata[1]->col_min+1 );
    vectorDYInPixels = ( vectorURYPixel-vectorLLYPixel ) /
                       ( vdata[1]->row_max-vdata[1]->row_min+1 );
    tileLLXPixel   = s.scalex ( s.xmin_ );
    tileURXPixel   = s.scalex ( s.xmax_ );
    tileLLYPixel   = s.scaley ( s.ymin_ );
    tileURYPixel   = s.scaley ( s.ymax_ );


    // determine maximum vector length
    maxMagnitude = -1.0;
    for ( i = vdata[1]->col_min; i <= vdata[1]->col_max; i+=xinc )
        {
        x =    vectorLLXPixel +
               vectorDXInPixels/2.0 +   // since vectors are *center* of cell
               vectorDXInPixels* ( i-1 ); // i-1 since VIS_DATA structs are 1 based

        if ( ( x>=tileLLXPixel ) && ( x<=tileURXPixel ) )

            for ( j = vdata[1]->row_min; j <= vdata[1]->row_max; j+=yinc )
                {
                y =  vectorLLYPixel +
                     vectorDYInPixels/2.0 +   // since vectors are *center* of cell
                     vectorDYInPixels* ( j-1 ); // j-1 since VIS_DATA structs are 1 based

                if ( ( y<=tileLLYPixel ) && ( y>=tileURYPixel ) )
                    for ( ts = vdata[1]->step_min; ts <= vdata[1]->step_max; ts++ )
                        {
                        index = INDEX (  i-vdata[1]->col_min,
                                         j-vdata[1]->row_min,
                                         0,
                                         ts-vdata[1]->step_min,
                                         vdata[1]->col_max-vdata[1]->col_min+1,
                                         vdata[1]->row_max-vdata[1]->row_min+1,
                                         1 );

                        mag = sqrt ( ( double ) ( vdata[1]->grid[index]*vdata[1]->grid[index]+
                                                  vdata[2]->grid[index]*vdata[2]->grid[index] ) );
                        if ( mag > maxMagnitude )
                            {
                            maxMagnitude = mag;
#ifdef DIAGNOSTICS
                            mx = i;
                            my = j;
                            mt = ts;
#endif // DIAGNOSTICS
                            }
                        }
                }
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "maxMagnitude==%f at i==%d j==%d t==%d\n",
              maxMagnitude, mx, my, mt );
#endif // DIAGNOSTICS


    // now loop over the vectors to be drawn, and draw each one of them,
    // finding the intersection of the vectors with the bounding box of the tiles;
    // those are the vectors to draw
    signx = ( vectorDXInPixels >=0.0 ) ?1.0:-1.0;
    signy = ( vectorDYInPixels >=0.0 ) ?1.0:-1.0;

    for ( i = vdata[1]->col_min; i <= vdata[1]->col_max; i+=xinc )
        {
        x =    vectorLLXPixel +
               vectorDXInPixels/2.0 +   // since vectors are *center* of cell
               vectorDXInPixels* ( i-vdata[1]->col_min );

        // CALT if ((x>=tileLLXPixel) && (x<=tileURXPixel))

        for ( j = vdata[1]->row_min; j <= vdata[1]->row_max; j+=yinc )
            {
            y =  vectorLLYPixel +
                 vectorDYInPixels/2.0 +   // since vectors are *center* of cell
                 vectorDYInPixels* ( j-vdata[1]->row_min );

            // CALT   if ((y<=tileLLYPixel) && (y>=tileURYPixel))
                {
                index = INDEX (  i-vdata[1]->col_min,
                                 j-vdata[1]->row_min,
                                 0,
                                 t,
                                 vdata[1]->col_max-vdata[1]->col_min+1,
                                 vdata[1]->row_max-vdata[1]->row_min+1,
                                 1 );

                if ( isnanf ( vdata[1]->grid[index] ) ||
                        isnanf ( vdata[2]->grid[index] ) )
                    {
                    continue;
                    }

                if ( scale_vectors_on_ )
                    {
                    //        mag = sqrt((double) (vdata[1]->grid[index]*vdata[1]->grid[index] +
                    //              vdata[2]->grid[index]*vdata[2]->grid[index])) *
                    //              scale / maxMagnitude;
                    //    dx = vdata[1]->grid[index] * vectorDXInPixels / 2.0 // * xinc / 2.0
                    //                           * mag / maxMagnitude;
                    //    dy = vdata[2]->grid[index] * vectorDYInPixels / 2.0 // * yinc / 2.0
                    //                   * mag / maxMagnitude;
                    mag = 0.1 * ( tileURXPixel-tileLLXPixel ) /vector_scale_;
                    dx = vdata[1]->grid[index] * signx * mag;
                    dy = vdata[2]->grid[index] * signy * mag;
                    }
                else
                    {
                    float len = sqrt ( ( double ) ( vdata[1]->grid[index]*vdata[1]->grid[index] +
                                                    vdata[2]->grid[index]*vdata[2]->grid[index] ) );
                    if ( len > 0 )
                        {
                        dx = vdata[1]->grid[index] / len
                             * vectorDXInPixels * 0.5; /* xinc / 2.0;*/
                        dy = vdata[2]->grid[index] / len
                             * vectorDYInPixels * 0.5; /* yinc / 2.0;*/
                        }
                    else
                        dx = dy = 0.0;
                    }

                x2 = x + dx;
                y2 = y + dy;

                drawVect ( x, y, x2, y2 );
                }
            }
        }

    // Added by ALT 10/01/1999
#ifdef DIAGNOSTICS
    fflush ( stderr );
    fflush ( stdout );
    for ( i=0; i<3; i++ )
        {
        if ( i==0 ) fprintf ( stderr, "\nTile Species == '%s'\n                         ",
                                  vdata[i]->species_short_name[vdata[i]->selected_species-1] );
        if ( i==1 ) fprintf ( stderr, "   U Species == '%s' ",
                                  vdata[i]->species_short_name[vdata[i]->selected_species-1] );
        if ( i==2 ) fprintf ( stderr, "   V Species == '%s' ",
                                  vdata[i]->species_short_name[vdata[i]->selected_species-1] );
        fprintf ( stderr, "grid_min == %f grid_max == %f\n",
                  vdata[i]->grid_min, vdata[i]->grid_max );
        }
    fprintf ( stderr, "maxMagnitude==%f at i==%d j==%d t==%d\n",
              maxMagnitude, mx, my, mt );
    fflush ( stderr );
#endif // DIAGNOSTICS
    }


void TileWnd::writeProbeFile ( float x1, float x2, float y1, float y2 )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::writeProbeFile()\n" );
#endif // DIAGNOSTICS

    FILE         *fp;
    int          i, j, yes;
    int      int_x1, int_x2, int_y1, int_y2;
    pid_t    pid;
    long     index;
    char         format[80];
    int w;
    char *tmp, pb_fmt[10];

    sprintf ( format, "%s ", legend_format_ ? legend_format_ : "%10.4f" );

    pid = getpid();
    probefilename_[0] = '\0';
    sprintf ( probefilename_, "/tmp/prob.%d", pid );
    if ( ( fp = fopen ( probefilename_, "w" ) ) == NULL )
        {
        fprintf ( stderr, "Can't write data to file.\n" );
        fclose ( fp );
        return;
        }

    int_x1 = ( int ) x1;
    int_x2 = ( int ) x2;
    int_y1 = ( int ) y1;
    int_y2 = ( int ) y2;

    // SRT 950802 fprintf(fp, "Grid x1=%g y1=%g x2=%g y2=%g\n", x1, y1, x2, y2);

    // added SRT 950802
    /*   if (fprintf(fp, "       ") <= 0)
    {
    fprintf(stderr, "Can't write data to file.\n");
    fclose(fp);
         return;
    }
    */
    // added ALT 980929
    tmp = strchr ( format,'%' );
    if ( tmp != NULL )
        {
        j=sscanf ( tmp+1,"%d",&w );
        if ( j == 0 )
            {
            w=4;
            fprintf ( stderr,"%s %d\n",
                      "ERROR: width field is missing, using default value",w );
            }
        }
    else
        {
        w = 4;
        fprintf ( stderr,"%s\n","ERROR: invalid C-format specs" );
        }
    sprintf ( pb_fmt,"%s%d%s ","%",w,"d" );

    if ( fprintf ( fp, "Col->  " ) <= 0 )
        {
        fprintf ( stderr, "Can't write data to file.\n" );
        fclose ( fp );
        return;
        }

    // added SRT 950802
    for ( j = 0; j < vis_->info->col_max; j++ )
        if ( ( j >= int_x1 ) && ( j <= int_x2 ) )
            // added ALT 980929       if (fprintf(fp, "    Col%3d ", j+1) <= 0)
            if ( fprintf ( fp, pb_fmt, j+1 ) <= 0 )
                {
                fprintf ( stderr, "Can't write data to file.\n" );
                fclose ( fp );
                return;
                }

    // added SRT 950802
    if ( fprintf ( fp, "\n" ) <= 0 )
        {
        fprintf ( stderr, "Can't write data to file.\n" );
        fclose ( fp );
        return;
        }

    for ( i=vis_->info->row_max; i >=0; i-- )
        {

        // added SRT 950802
        if ( ( i <= int_y1 ) && ( i >= int_y2 ) )
            if ( fprintf ( fp, "Row%3d ", i+1 ) <= 0 )
                {
                fprintf ( stderr, "Can't write data to file.\n" );
                fclose ( fp );
                return;
                }

        yes = 0;
        for ( j = 0; j <= vis_->info->col_max; j++ )
            {
            // SRT 950802 index = curr_animate_*vis_->row_max_*vis_->col_max_ + i*vis_->col_max_ + j;

            if ( ( j >= int_x1 ) && ( j <= int_x2 ) &&
                    ( i <= int_y1 ) && ( i >= int_y2 ) )
                {

                // added SRT 950802
                index = INDEX (  j- ( vis_->info->col_min-1 ),
                                 i- ( vis_->info->row_min-1 ),
                                 0,
                                 curr_animate_,
                                 vis_->info->col_max
                                 -vis_->info->col_min+1,
                                 vis_->info->row_max
                                 -vis_->info->row_min+1,
                                 1 );

                fprintf ( fp, format, vis_->info->grid[index] );
                yes = 1;
                }
            }
        if ( yes ) fprintf ( fp, "\n" );
        }

    fclose ( fp );
    }


void TileWnd::writeProbeObsFile ( int x1, int x2, int y1, int y2 )
    {

    FILE         *fp;
    int          i, j, n, k, ts;
    pid_t    pid;
    char         format[80];
    float        x, y, val;
    float        xx1, xx2, yy1, yy2;

    PLOT_DATA *pdata;
    char str6[7];


    sprintf ( format, "%s \n", legend_format_ ? legend_format_ : "%10.4f" );

    pid = getpid();
    probefilename_[0] = '\0';
    sprintf ( probefilename_, "/tmp/prob.%d", pid );
    if ( ( fp = fopen ( probefilename_, "w" ) ) == NULL )
        {
        fprintf ( stderr, "Can't write data to file.\n" );
        fclose ( fp );
        return;
        }

    if ( x1 == x2 && y1 == y2 )
        {
        xx1 = s.gridx ( x1-obs_size_ );
        xx2 = s.gridx ( x2+obs_size_ );
        yy1 = s.gridy ( y1-obs_size_ );
        yy2 = s.gridy ( y2+obs_size_ );
        }
    else
        {
        xx1 = s.gridx ( x1 );
        xx2 = s.gridx ( x2 );
        yy1 = s.gridy ( y1 );
        yy2 = s.gridy ( y2 );
        }

    pdata = ( PLOT_DATA * ) plotDataListP_->head();
    while ( pdata )
        {
        if ( pdata->plot_type == OBS_PLOT )
            {
            ts = pdata->jstep[curr_animate_];
            if ( ts>=0 )
                {
                int n=pdata->vdata->ncol;

                for ( j=0; j<n; j++ )
                    {
                    k = n*ts+j;
                    x = pdata->coord_x[k];
                    y = pdata->coord_y[k];
                    if ( ( x >= xx1 ) && ( x <= xx2 ) && ( y >= yy2 ) && ( y <= yy1 ) )
                        {
                        val = pdata->vdata->grid[k];
                        if ( !isnanf ( val ) )
                            {
                            if ( pdata->stnid )
                                {
                                fprintf ( fp,"%s\t",pdata->stnid[k] );
                                }
                            fprintf ( fp, format, val );
                            }
                        }
                    }
                }
            }
        pdata = ( PLOT_DATA * ) plotDataListP_->next();
        }

    fclose ( fp );
    }



void TileWnd::saveMPEGMenuCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->saveMPEGMenu_cb();
    }


void TileWnd::saveMPEGMenu_cb()
    {
    SaveMPEGBrowser_.postSaveSelectionDialog();
    }


void TileWnd::saveMPEGCB ( void *object, char *fname )

    {
    TileWnd *obj = ( TileWnd * ) object;
    obj->saveAnimation_cb ( fname,"mpeg" );
    }


void TileWnd::saveAnimatedGIFMenuCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->saveAnimatedGIFMenu_cb();
    }


void TileWnd::saveAnimatedGIFMenu_cb()
    {
    SaveAnimatedGIFBrowser_.postSaveSelectionDialog();
    }


void TileWnd::saveAnimatedGIFCB ( void *object, char *fname )

    {
    TileWnd *obj = ( TileWnd * ) object;
    obj->saveAnimation_cb ( fname,"gif" );
    }


void TileWnd::saveAnimation_cb ( char *fname, char *ftype )
    {
    int i, j;
    char XWDname[256];
    char errorString[256];
    char command[256];
    char hname[256];
    char pname[256];
    char shortFname[256];
    char fileOnly[256];
    char *convert_ptr;
    char convert_cmd[256];
    char *lastSlash;
    char tmplt[256], dirname[256];

    fprintf ( stderr,"Create animation %s\n",fname );

    // Figure out what directory all this stuff goes into
    if ( parseLongDataSetName ( fname,    /* full name to be parsed */
                                hname,       /* extracted host name goes here */
                                pname,       /* extracted path name goes here */
                                shortFname ) /* extracted fname name goes here */
       )
        {
        fprintf ( stderr, "\007saveAnimation_cb() can't find path name!\n" );
        return;
        }
    if ( strlen ( shortFname ) == 0 )
        {
        strcpy ( shortFname,"anim" );
        sprintf ( fname,"%s/%s",fname,shortFname );
        }

    strcpy ( dirname, fname );
    lastSlash = strrchr ( dirname,'/' );
    // get the file part of the file name
    if ( lastSlash != NULL )
        {
        strcpy ( fileOnly,lastSlash+1 );
        }
    else
        {
        strcpy ( fileOnly,fname );
        }
    *lastSlash='\0';
    if ( strlen ( fileOnly ) ==0 )
        {
        fprintf ( stderr,"Empty file name - assigning default name 'anim'\n" );
        sprintf ( fileOnly,"anim" );
        }
    if ( strlen ( dirname ) == 0 )
        {
        strcpy ( dirname,"." );
        }
    sprintf ( tmplt,"%s/animXXXXXX",dirname );
    strcpy ( dirname, mktemp ( tmplt ) );
    if ( dirname[0] == '\0' )
        {
        fprintf ( stderr,"ERROR: cannot generate temporary directory name\n" );
        return;
        }
    if ( mkdir ( dirname, S_IRWXU ) != 0 )
        {
        fprintf ( stderr,"ERROR: cannot generate temporary directory %s\n", dirname );
        return;
        }

    // Save all the images in XWD format
    for ( i = 0; i < vis_->getTimeMax(); i++ )
        {
        resize();
        animate_scale_cb ( i );
        sprintf ( XWDname, "%s/%s.%04d.xwd", dirname, shortFname, i );
        if ( dumpImage ( "XWD", XWDname, errorString ) )
            {
            fprintf ( stderr, "\007%s\n", errorString );
            for ( j = 0; j <= i; j++ )
                {
                sprintf ( XWDname, "%s/%s.%04d.xwd", dirname, shortFname, j );
                unlink ( XWDname );
                }
            return;
            }
        }

    convert_ptr = getenv ( "CONVERT" );
    // AME: added to prevent crash on Windows
    if ( convert_ptr != NULL )
        strcpy ( convert_cmd,getenv ( "CONVERT" ) );
    sprintf ( command,"cd '%s'; '%s' %s %s.*.xwd ../%s.%s", dirname,
              ( convert_ptr != NULL ? convert_cmd: "convert" ),
              ( getenv ( "IMAGE_MAGICK_ARGS" ) != NULL ? getenv ( "IMAGE_MAGICK_ARGS" ) : "" ),
              shortFname, shortFname, ftype );
    fprintf ( stderr,"Creating animation with command:\n  %s\n",command );
    i = system ( command );
    if ( i != 0 )
        {
        fprintf ( stderr,
                  "ANIMATION CREATION FAILED for file %s\nImage snapshots are in\n%s",
                  fname, dirname );
        // don't delete the images - the user may want them
        }
    else
        {
        fprintf ( stderr,"Created animation %s!\n",fname );
        // remove xwd files
        for ( i = 0; i < vis_->getTimeMax(); i++ )
            {
            sprintf ( XWDname, "%s/%s.%04d.xwd", dirname, shortFname, i );
            unlink ( XWDname );
            }
        rmdir ( dirname );
        }
    }

////////////////////////////////////////////////////////
//
// dumpImage()
//
// returns 1 if error
//
////////////////////////////////////////////////////////

extern "C" {
    int write_frame ( Display *, Window, char * );
    }
int TileWnd::dumpImage ( char *imagetype, // for now only these are supported:
                         // "PNM", "XWD", "GIF", "RGB", "MPEG",
                         // "PS, "PNG"
                         char *fname,    // image file name
                         char *estring ) // error messages will go here
    {
    char  xwdname[256],
          command[512],
          dirname[512],
          tstring[256];
    char *lastSlash;
    Display *display = XtDisplay ( canvas_ );
    Window  window = XtWindow ( canvas_ );
    int   i;
    char *convert_ptr;
    char convert_cmd[256];

    // check the arguments
    if ( ( !imagetype ) || ( !fname ) || ( !estring ) )
        {
        fprintf ( stderr, "\007Bad args to TileWnd::dumpImage()!" );
        return 1;
        }
    if ( strcmp ( imagetype, "PNM" ) && strcmp ( imagetype, "XWD" ) &&
            strcmp ( imagetype, "PNG" ) &&
            strcmp ( imagetype, "RGB" ) && strcmp ( imagetype, "GIF" ) &&
            strcmp ( imagetype, "MPEG" ) && strcmp ( imagetype, "PS" ) )
        {
        sprintf ( estring,
                  "\007Unsupported image type '%s' arg to TileWnd::dumpImage()!",
                  imagetype );
        return 1;
        }

    // if its an MPEG, save it as such
    if ( !strcmp ( imagetype, "MPEG" ) )
        {
        saveAnimation_cb ( fname, "mpeg" );
        return 0;
        }


    // if '%' is in the fname, then dump all time steps
    // as individual images, using the % format characters
    // with sprintf to get the file names.  For example,
    // if the fname is "/tmp/test%d" and there are 12 time
    // steps, the images will be called /tmp/test0 .. /tmp/test11
    if ( strchr ( fname, ( int ) '%' ) )
        {
        for ( i = 0; i < vis_->getTimeMax(); i++ )
            {
            resize();
            animate_scale_cb ( i );
            sprintf ( tstring, fname, i );
            if ( dumpImage ( imagetype, tstring, estring ) )
                {
                fprintf ( stderr, "\007%s\n", estring );
                return 1;
                }
            }
        return 0;
        }

    // move the image completely on to the monitor if necessary
    //fprintf(stderr, "display->default_screen == %d\n", display->default_screen);
    //fprintf(stderr, "display->screens[default_screen].width == %d\n",
    //         display->screens[default_screen].width);
    //fprintf(stderr, "display->screens[default_screen].height == %d\n",
    //         display->screens[default_screen].height);

    // redraw the images since it may need to be after dialog box
    // also remap the menu widgets, since for some reason
    // their display sometimes disappears (while still functioning)
    manage();
    resize();
    XSync ( display, False );
    sleep ( 1 ); // added 960913 SRT

    strcpy ( dirname, fname );
    lastSlash = strrchr ( dirname,'/' );
    // get the file part of the file name
    if ( lastSlash != NULL )
        {
        strcpy ( tstring,lastSlash+1 );
        }
    else
        {
        strcpy ( tstring,fname );
        }
    *lastSlash='\0';
    if ( strlen ( tstring ) ==0 )
        {
        fprintf ( stderr,"Empty file name - assigning default name 'image'\n" );
        sprintf ( tstring,"image" );
        }
    if ( strlen ( dirname ) == 0 )
        {
        strcpy ( dirname,"." );
        }

    // dump the image as an XWD
    if ( strcmp ( imagetype, "XWD" ) )
        sprintf ( xwdname, "%s/%s.xwd", dirname,tstring );
    else
        sprintf ( xwdname, "%s/%s", dirname, tstring );


    //fprintf(stderr, "\nWriting XWD image '%s'...\n", xwdname);
    if ( write_frame ( display, window, xwdname ) )
        {
        sprintf ( estring,
                  "\007TileWnd::dumpImage() couldn't write '%s'!", xwdname );
        return 1;
        }
    i = 0;

    // convert the to the image desired format (if not XWD) and name
    if ( strcmp ( imagetype, "XWD" ) )
        {
        convert_ptr = getenv ( "CONVERT" );

        // AME: added to prevent crash on Windows
        if ( convert_ptr != NULL )
            strcpy ( convert_cmd,getenv ( "CONVERT" ) );

        // AME: changed message to show full conversion command below
        //fprintf(stderr, "Converting '%s' to %s image '%s'...\n",
        //xwdname, imagetype, fname);

        // old version
        //sprintf(command, "'%s' XWD:%s %s:%s",
        //(convert_ptr != NULL ? convert_cmd: "convert"),
        //xwdname, imagetype, fname);
        sprintf ( command, "cd '%s'; '%s' %s XWD:%s.xwd %s:%s", dirname,
                  ( convert_ptr != NULL ? convert_cmd: "convert" ),
                  ( getenv ( "IMAGE_MAGICK_ARGS" ) != NULL ? getenv ( "IMAGE_MAGICK_ARGS" ) : "" ),
                  tstring, imagetype, tstring );
        printf ( "Converting to %s using command:\n   %s\n",imagetype, command );
        i=system ( command );
        unlink ( xwdname );
        if ( i != 0 )
            fprintf ( stderr,"IMAGE CONVERSION FAILED!  Make sure Image Magick's convert is in your path -\n  or specify its location with setenv CONVERT pathtoconvert.\n" );
        }

    if ( i == 0 )
        fprintf ( stderr, "Created '%s'\n", fname );
    else
        fprintf ( stderr, "ERROR CREATING '%s'\n", fname );
    return 0;
    }



void TileWnd::OUTLCO_CB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->OUTLCO_cb ( 1 );
    }

void TileWnd::OUTLCO_cb ( int rz )
    {
    char *dirname ;
    mapChoices_ = MapCounties;
    dirname = getenv ( "PAVE_MAPDIR" ) ;
    if ( dirname && *dirname )
        sprintf ( vis_->mapName_, "%s/OUTLCOUNTIES", dirname );
    else
        sprintf ( vis_->mapName_, "OUTLCOUNTIES" );
    vis_->setMapData();
    if ( rz ) resize();
    }


void TileWnd::OUTLRIVERS3000_CB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->OUTLRIVERS3000_cb();
    }

void TileWnd::OUTLRIVERS3000_cb()
    {
    char *dirname ;
    mapChoices_ = MapRivers;
    dirname = getenv ( "PAVE_MAPDIR" ) ;
    if ( dirname && *dirname )
        sprintf ( vis_->mapName_, "%s/OUTLRIVERS3000", dirname );
    else
        sprintf ( vis_->mapName_, "OUTLRIVERS3000" );
    vis_->setMapData();
    resize();
    }


void TileWnd::OUTLROADS3000_CB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->OUTLROADS3000_cb();
    }

void TileWnd::OUTLROADS3000_cb()
    {
    char *dirname ;
    mapChoices_ = MapRoads;
    dirname = getenv ( "PAVE_MAPDIR" ) ;
    if ( dirname && *dirname )
        sprintf ( vis_->mapName_, "%s/OUTLROADS3000", dirname );
    else
        sprintf ( vis_->mapName_, "OUTLROADS3000" );
    vis_->setMapData();
    resize();
    }


void TileWnd::OUTLSTATES3000_CB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->OUTLSTATES3000_cb ( 1 );
    }

void TileWnd::OUTLSTATES3000_cb ( int rz )
    {
    char *dirname ;
    mapChoices_ =  MapHighResStates;
    dirname = getenv ( "PAVE_MAPDIR" ) ;
    if ( dirname && *dirname )
        sprintf ( vis_->mapName_, "%s/OUTLSTATES3000", dirname );
    else
        sprintf ( vis_->mapName_, "OUTLSTATES3000" );
    vis_->setMapData();
    if ( rz ) resize();
    }


void TileWnd::OUTLUSAM_CB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->OUTLUSAM_cb();
    }

void TileWnd::OUTLUSAM_cb()
    {
    char *dirname ;
    mapChoices_ =  MapDefault;
    dirname = getenv ( "PAVE_MAPDIR" ) ;
    if ( dirname && *dirname )
        sprintf ( vis_->mapName_, "%s/OUTLUSAM", dirname );
    else
        sprintf ( vis_->mapName_, "OUTLUSAM" );
    vis_->setMapData();
    resize();
    }



void TileWnd::OUTLHRES_CB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->OUTLHRES_cb();
    }

void TileWnd::OUTLHRES_cb()
    {
    char *dirname ;
    mapChoices_ =  MapMediumResStates;
    dirname = getenv ( "PAVE_MAPDIR" ) ;
    if ( dirname && *dirname )
        sprintf ( vis_->mapName_, "%s/OUTLHRES", dirname );
    else
        sprintf ( vis_->mapName_, "OUTLHRES" );
    vis_->setMapData();
    resize();
    }



void TileWnd::saveRGBMenuCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->saveRGBMenu_cb();
    }


void TileWnd::saveRGBMenu_cb()
    {
    SaveRGBBrowser_.postSaveSelectionDialog();
    }


void TileWnd::saveRGBCB ( void *object, char *fname )

    {
    TileWnd *obj = ( TileWnd * ) object;
    obj->saveRGB_cb ( fname );
    }


void TileWnd::saveRGB_cb ( char *fname )
    {
    char errorString[256];
    if ( dumpImage ( "RGB", fname, errorString ) )
        {
        fprintf ( stderr, "\007" );
        fprintf ( stderr, "%s\n", errorString );
        }
    }


void TileWnd::saveConfigMenuCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->saveConfigMenu_cb();
    }


void TileWnd::saveConfigMenu_cb()
    {
    SaveConfigBrowser_.postSaveSelectionDialog();
    }


void TileWnd::saveConfigCB ( void *object, char *fname )

    {
    TileWnd *obj = ( TileWnd * ) object;
    obj->saveConfig_cb ( fname );
    }


void TileWnd::saveConfig_cb ( char *fname )
    {
    FILE *fp;
    char tstring[80];

    fp = fopen ( fname, "w" );
    if ( !fp )
        {
        fprintf ( stderr, "\007Couldn't open file '%s' for writing!!\n", fname );
        return;
        }

    if ( legend_cmap_ == NEWTON_COLORMAP ) sprintf ( tstring, "NEWTON_COLORMAP" );
    else if ( legend_cmap_ == JET_COLORMAP )    sprintf ( tstring, "JET_COLORMAP" );
    else if ( legend_cmap_ == GREY_COLORMAP )   sprintf ( tstring, "GREY_COLORMAP" );
    else
        tstring[0] = '\0';
    if ( tstring[0] )
        fprintf ( fp, "ColorMapType    %s\n", tstring );

    sprintf ( tstring, "Legend_Max      %s\n", legend_format_ ? legend_format_ : "%f" );
    fprintf ( fp, tstring, val_max_ );

    sprintf ( tstring, "Legend_Min      %s\n", legend_format_ ? legend_format_ : "%f" );
    fprintf ( fp, tstring, val_min_ );

    if ( legend_format_ )
        fprintf ( fp, "Legend_Format   %s\n", legend_format_ );

    fprintf ( fp, "Number_Labels   %d\n", legend_nlabel_ );

    fprintf ( fp, "Invert_Colormap %d\n", scale_vectors_on_ );

    fprintf ( fp, "Number_Tiles    %d\n", legend_ntile_ );

    int         i,
                scr = DefaultScreen ( XtDisplay ( canvas_ ) );;
    XGCValues   values;
    XColor      color;
    Colormap    cmap = DefaultColormap ( cl_dpy_, scr );
    for ( i = legend_ntile_-1; i >= 0; i-- )
        {
        XGetGCValues ( cl_dpy_, color_gc_table_[i], GCForeground, &values );
        color.pixel = values.foreground;
        XQueryColor ( cl_dpy_, cmap, &color );
        fprintf ( fp, "%d\t%d\t%d\tColorNumber%d\n",
                  color.red/256, color.green/256, color.blue/256, i+1 );
        }

    fprintf ( fp, "Save_MPEG_Files %d\n", saveMPEGControls_ );

    fprintf ( fp, "Disable_Map     %d\n", legend_map_off_ );

    fprintf ( fp, "Smooth_Plot     %d\n", smooth_plots_on_ );

    fprintf ( fp, "Draw_Grid_Lines %d\n", grid_lines_on_ );

    fprintf ( fp, "Scale_Vectors   %d\n", scale_vectors_on_ );

    fclose ( fp );
    }


void TileWnd::savePNGMenuCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->savePNGMenu_cb();
    }

void TileWnd::saveGIFMenuCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->saveGIFMenu_cb();
    }


void TileWnd::savePNGMenu_cb()
    {
    SavePNGBrowser_.postSaveSelectionDialog();
    }


void TileWnd::saveGIFMenu_cb()
    {
    SaveGIFBrowser_.postSaveSelectionDialog();
    }


void TileWnd::savePNGCB ( void *object, char *fname )

    {
    TileWnd *obj = ( TileWnd * ) object;
    obj->savePNG_cb ( fname );
    }

void TileWnd::saveGIFCB ( void *object, char *fname )

    {
    TileWnd *obj = ( TileWnd * ) object;
    obj->saveGIF_cb ( fname );
    }


void TileWnd::savePNG_cb ( char *fname )
    {
    char errorString[256];
    if ( dumpImage ( "PNG", fname, errorString ) )
        {
        fprintf ( stderr, "\007" );
        fprintf ( stderr, "%s\n", errorString );
        }
    }

void TileWnd::saveGIF_cb ( char *fname )
    {
    char errorString[256];
    if ( dumpImage ( "GIF", fname, errorString ) )
        {
        fprintf ( stderr, "\007" );
        fprintf ( stderr, "%s\n", errorString );
        }
    }


void TileWnd::saveXWDMenuCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->saveXWDMenu_cb();
    }


void TileWnd::saveXWDMenu_cb()
    {
    SaveXWDBrowser_.postSaveSelectionDialog();
    }


void TileWnd::saveXWDCB ( void *object, char *fname )

    {
    TileWnd *obj = ( TileWnd * ) object;
    obj->saveXWD_cb ( fname );
    }


void TileWnd::saveXWD_cb ( char *fname )
    {
    char errorString[256];
    if ( dumpImage ( "XWD", fname, errorString ) )
        {
        fprintf ( stderr, "\007" );
        fprintf ( stderr, "%s\n", errorString );
        }
    }



void TileWnd::drawMinMax ( int t )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::drawMinMax()\n" );
#endif // DIAGNOSTICS

    char     buf[128], format[128];
    float    min,
             max,
             val;
    int      i, j, mincol, minrow, maxcol, maxrow, set,
             i_xmin = ( int ) floor ( zoom_[curr_zoom_].GRID_X_MIN_ ),
             i_xmax = ( int ) floor ( zoom_[curr_zoom_].GRID_X_MAX_ ),
             i_ymin = ( int ) floor ( zoom_[curr_zoom_].GRID_Y_MIN_ ),
             i_ymax = ( int ) floor ( zoom_[curr_zoom_].GRID_Y_MAX_ );

    set = 0 ;
    for ( j=vis_->col_min_; j <= vis_->col_max_; j++ )
        for ( i= vis_->row_min_; i <= vis_->row_max_; i++ )
            if ( ( j>=i_xmin+1 ) && ( j<=i_xmax+1 ) &&
                 ( i>=i_ymin+1 ) && ( i<=i_ymax+1 ) )
                {
                val = vis_->info->grid
                      [INDEX (
                           j-vis_->col_min_,   // Eng reversed i & j SRT
                           i-vis_->row_min_,   // Eng reversed i & j SRT
                           0,
                           t,
                           vis_->col_max_-vis_->col_min_+1,
                           vis_->row_max_-vis_->row_min_+1,
                           1 )];
                if ( isnanf ( val ) ) continue;
                if ( !set )
                    {
                    min    = max = val;
                    mincol = j;
                    minrow = i;
                    maxcol = j;
                    maxrow = i;
                    set    = 1;
                    }
                else if ( val < min )
                    {
                    min = val;
                    mincol = j;
                    minrow = i;
                    }
                else if ( val > max )
                    {
                    max = val;
                    maxcol = j;
                    maxrow = i;
                    }
                }

    if ( ! ( ( getenv ( "DISABLE_MINMAX" ) != NULL ) &&
             ( !strcmp ( getenv ( "DISABLE_MINMAX" ), "1" ) ) ) )
        {
        sprintf ( format, "Min=%s at (%d,%d), Max=%s at (%d,%d)",
                  legend_format_, mincol, minrow,
                  legend_format_, maxcol, maxrow );
        sprintf ( buf, format, min, max );
        drawTitle ( def_font_, height_-5, buf );
        }
    }



void TileWnd::savePSMenuCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->savePSMenu_cb();
    }


void TileWnd::savePSMenu_cb()
    {
    SavePSBrowser_.postSaveSelectionDialog();
    }


void TileWnd::savePSCB ( void *object, char *fname )

    {
    TileWnd *obj = ( TileWnd * ) object;
    obj->savePS_cb ( fname );
    }


void TileWnd::savePS_cb ( char *fname )
    {
    char errorString[256];
    if ( dumpImage ( "PS", fname, errorString ) )
        {
        fprintf ( stderr, "\007" );
        fprintf ( stderr, "%s\n", errorString );
        }
    }



void TileWnd::printMenuCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->printMenu_cb();
    }


void TileWnd::printMenu_cb()
    {
    char    printer[64],
            XWDname[64],
            command[512];
    pid_t   pid = getpid();


    // do we have all the utils we need?
    if ( ( getenv ( "XWDTOPNM" ) == NULL ) || ( !strlen ( getenv ( "XWDTOPNM" ) ) ) ||
         ( getenv ( "PNMDEPTH" ) == NULL ) || ( !strlen ( getenv ( "PNMDEPTH" ) ) ) ||
         ( getenv ( "PNMTOPS"  ) == NULL ) || ( !strlen ( getenv ( "PNMTOPS"  ) ) ) )
        {
        fprintf ( stderr,
                  "\007TileWnd::printMenu_cb() can't find all the utilities it needs!\n" );
        return;
        }


    // is there a special printer for us to use?
    if ( ( getenv ( "PRINTER" ) != NULL ) || ( strlen ( getenv ( "PRINTER" ) ) ) )
        sprintf ( printer, "-P%s", getenv ( "PRINTER" ) );
    else
        printer[0] = '\0';


    // save the current image as an XWD
    sprintf ( XWDname, "/tmp/%d.xwd", ( int ) pid );
    if ( dumpImage ( "XWD", XWDname, errorString ) )
        {
        fprintf ( stderr, "\007%s\n", errorString );
        unlink ( XWDname );
        return;
        }


    // convert XWD->XWD with 8 bits of depth->PostScript->print
    sprintf ( command, "cat %s | %s | %s 255 | %s | lpr %s",
              XWDname, getenv ( "XWDTOPNM" ), getenv ( "PNMDEPTH" ),
              getenv ( "PNMTOPS" ), printer );
    fprintf ( stderr, "%s\n", command );
    system ( command );


    // delete the XWD file
    unlink ( XWDname );


    // confirmation
    fprintf ( stderr, "Image has been printed to %s.\n",
              ( ( getenv ( "PRINTER" ) != NULL ) && ( strlen ( getenv ( "PRINTER" ) ) ) ) ?
              getenv ( "PRINTER" ) : "the default printer lp" );
    }


void TileWnd::setButtonSensitiviy()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter TileWnd::setButtonSensitiviy()\n" );
#endif // DIAGNOSTICS

    if ( ( getenv ( "MPEG_ENCODE" ) == NULL ) || ( !strlen ( getenv ( "MPEG_ENCODE" ) ) ) )
        {
        XtSetSensitive ( createMPEGAnimationMenuButton_, False );
        }
    }



void TileWnd::timeSeriesCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->interact_mode_ = TIME_SERIES_MODE;
    obj->interact_submode_ = TIME_SERIES_MODE;
    }

void TileWnd::timeSeriesObsCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->interact_mode_ = TIME_SERIES_MODE;
    obj->interact_submode_ = TIME_SERIES_OBS_MODE;
    }



// args are *0* based
void TileWnd::timeSeriesProbe (  int x1, int x2, int y1, int y2  )
    {
    int     i, j, t, ti,
            ni = vis_->col_max_-vis_->col_min_+1,
            nj = vis_->row_max_-vis_->row_min_+1,
            nperstep,
            nsteps = vis_->step_max_-vis_->step_min_+1;
    float   *tsdata;
    char    *legend[] = { "      " };
    char    *symbol[] = { "circle" };
    char    *color[] = { "red" };
    int     npoints[1];
    char    message[512];
    float   *x;
    char    unitLabel[128];
    char    title[256];
    char    *units;

    if ( vectors_on_ && !tiles_on_ )
        {
        fprintf ( stderr,"%s\n","ERROR: timeSeries not allowed for vectorTile plots" );
        return;
        }
    /* change the args to 1-based */
    x1++;
    x2++;
    y1++;
    y2++;
    if ( y1 > y2 )
        {
        ti = y1;
        y1 = y2;
        y2 = ti;
        }
    nperstep = ( x2-x1+1 ) * ( y2-y1+1 );

    if (    ( x1 < vis_->col_min_ ) || ( x2 > vis_->col_max_ ) ||
            ( y1 < vis_->row_min_ ) || ( y2 > vis_->row_max_ ) )
        {
        fprintf ( stderr, "\007Invalid args to TileWnd::timeSeriesProbe():\n" );
        fprintf ( stderr, "  x1 == %d\n", x1 );
        fprintf ( stderr, "  x2 == %d\n", x2 );
        fprintf ( stderr, "  y1 == %d\n", y1 );
        fprintf ( stderr, "  y2 == %d\n", y2 );
        fprintf ( stderr, "  vis_->col_min_ == %d\n", vis_->col_min_ );
        fprintf ( stderr, "  vis_->col_max_ == %d\n", vis_->col_max_ );
        fprintf ( stderr, "  vis_->row_min_ == %d\n", vis_->row_min_ );
        fprintf ( stderr, "  vis_->row_max_ == %d\n", vis_->row_max_ );
        return;
        }

    tsdata = ( float * ) malloc ( nsteps*sizeof ( float ) );
    if ( !tsdata )
        {
        fprintf ( stderr,
                  "\007Memory allocation failure in TileWnd::timeSeriesProbe()!\n" );
        return;
        }

    for ( t = 0; t < nsteps; t++ )
        {
        tsdata[t] = 0.0;
        nperstep = 0;
        float val;
        for ( j = y1; j <= y2; j++ )
            {
            for ( i = x1; i <= x2; i++ )
                {
                val = vis_->info->grid[INDEX ( i-vis_->col_min_,
                                               j-vis_->row_min_,
                                               0, t, ni, nj, 1 )];

                if ( isnanf ( val ) ) continue;
                tsdata[t] += val;
                nperstep++;
                }
            }
        tsdata[t] /= nperstep;
        }


    npoints[0] = nsteps;
    x = ( float * ) malloc ( nsteps*sizeof ( float ) );
    for ( t = vis_->step_min_; t <= vis_->step_max_; t++ )
        x[t-vis_->step_min_] = ( float ) ( t-1 );

    units = vis_->getUnits();
    if ( ( x1 == x2 ) && ( y1 == y2 ) )
        {
        if ( units != NULL )
            {
            sprintf ( unitLabel, "Average value (%s) at cell (%d, %d)",
                      units, x1, y1 );
            }
        else
            {
            sprintf ( unitLabel, "Average value at cell (%d, %d)",
                      x1, y1 );
            }
        }
    else
        {
        if ( units != NULL )
            {
            sprintf ( unitLabel, "Average value (%s) of cells (%d,%d)->(%d,%d)",
                      units, x1, y1, x2, y2 );
            }
        else
            {
            sprintf ( unitLabel, "Average value of cells (%d,%d)->(%d,%d)",
                      x1, y1, x2, y2 );
            }
        }

    sprintf ( title, "%s  %s", vis_->title1_, vis_->title3_ );

    char timeStepLabel[128];
    sprintf ( timeStepLabel, "Time Step (%s to %s)",
              get_vdata_TimeMinString ( vis_->info ),
              get_vdata_TimeMaxString ( vis_->info ) );

    if ( !graph2d (   x, tsdata, 1, npoints, title, timeStepLabel,
                      unitLabel, legend, symbol, color, message, 1 ) )
        fprintf ( stderr, "\007%s\n", message );

    if ( tsdata )
        {
        free ( tsdata );
        tsdata = NULL;
        }
    if ( x )
        {
        free ( x );
        x = NULL;
        }
    }



void TileWnd::minPointTimeSeriesCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->minOrMaxPointTimeSeriesCB ( 0 );
    }


void TileWnd::maxPointTimeSeriesCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->minOrMaxPointTimeSeriesCB ( 1 );
    }


void TileWnd::minOrMaxPointTimeSeriesCB ( int minOrMax )
    {
    int     ni = vis_->col_max_-vis_->col_min_+1,
            nj = vis_->row_max_-vis_->row_min_+1,
            nt = vis_->step_max_-vis_->step_min_+1,
            i, j, t, iloc, jloc ;
    float   val = minOrMax ? vis_->info->grid_max : vis_->info->grid_min;

    iloc = -1,
    jloc = -1;
    for ( t = 0; t < nt; t++ )    
        for ( j = 0; j < nj; j++ )
            for ( i = 0; i < ni; i++ )
                if ( vis_->info->grid[INDEX ( i, j, 0, t, ni, nj, 1 )] == val )
                    {
                    iloc = i;
                    jloc = j;
                    }

    if ( ( iloc == -1 ) || ( jloc == -1 ) )
        {
        fprintf ( stderr,
                  "\007Couldn't find val in TileWnd::minOrMaxPointTimeSeriesCB()!\n" );
        return;
        }

    timeSeriesProbe ( iloc, iloc, jloc, jloc );
    return;
    }


// ============================================================

void TileWnd::overlay_obsCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;

    obj->overlay_cb ( OBS_PLOT );
    }

// ============================================================

void TileWnd::overlay_vectorobsCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;

    obj->overlay_cb ( OBSVECTOR_PLOT );
    }

// ============================================================

void TileWnd::overlay_cntrCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;

    obj->overlay_cb ( CONTOUR_PLOT );
    }

// ============================================================
extern Widget driverWindowSelectionDialog_CB ( void *, TileWnd * );
extern void   driverWindowGetFormula ( void *, Widget, int );
extern PLOT_DATA *driverWindowGetPlotData ( void *, char *, int );
// ============================================================
void TileWnd::set_overlay_mode ( int mode )
    {
    overlay_mode_ = mode;

    if ( mode == OBS_PLOT )
        {
        XtSetSensitive ( probeobsButton_, True );
        XtSetSensitive ( configobsButton_, True );
        XtSetSensitive ( tsobsButton_, True );
        }
    if ( mode == CONTOUR_PLOT )
        {
        XtSetSensitive ( configcntrButton_, True );
        }

    }

void TileWnd::overlay_cb ( int mode )
    {
    Widget  edit_selection_dialog;
    void *dwnd;

    extern void driverWindowMultiSelectDialog ( void *dwnd, int mode, void *twnd );

    set_overlay_mode ( mode );
    dwnd = getDriverWnd();

    if ( mode == OBSVECTOR_PLOT )
        {
        driverWindowMultiSelectDialog ( dwnd, mode, this );
        return;
        }

    edit_selection_dialog = driverWindowSelectionDialog_CB ( dwnd, this );
    driverWindowGetFormula ( dwnd, edit_selection_dialog, mode );

    if ( XtIsManaged ( edit_selection_dialog ) )
        {
        XtUnmanageChild ( edit_selection_dialog );
        }
    XtManageChild ( edit_selection_dialog );
    }

// ============================================================
void TileWnd::overlay_selectedCB ( Widget dialog, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->overlay_selected_cb ( dialog );
    }


// ============================================================

Cntr_line *ctr_line=NULL;
extern "C" {
    int compute_contour   ( float *, float *, float *, int, int, int, int, float * );
    void auto_cntr_levels ( float, float, int, float *, int * );
    }

void TileWnd::overlay_selected_cb ( Widget dialog )
    {
    char *newtext  = XmTextGetString ( XmSelectionBoxGetChild ( dialog,
                                       XmDIALOG_TEXT ) );
    overlay_create ( newtext );
    }

void TileWnd::overlay_create ( char *newtext )
    {
    PLOT_DATA *pdata;
    VIS_DATA *vdt;
    int i, i2, j, n, n1, tstep;
    void *dwnd;
    Map *map;
    char message[256];
    float cntr_lvls[32];
    extern Map *projmap_generate ( VIS_DATA *, char *, char * );
    M3IOParameters params;

    if ( !newtext || !*newtext )
        {
        // null string entered
        goto cleanup;
        }

    dwnd = getDriverWnd();
    pdata = driverWindowGetPlotData ( dwnd, newtext, overlay_mode_ );
    if ( pdata == NULL ) goto cleanup;
    n = vis_->step_max_ - vis_->step_min_ + 1;
    if ( overlay_mode_ == OBSVECTOR_PLOT )
        {
        vdt = pdata->vect2d->vdata_x;
        XtSetSensitive ( configvectobsButton_, True );
        }
    else   // OBS or CONTOUR
        {
        vdt = pdata->vdata;
        }
    i2 = vdt->step_min - 1;
    n1 = vdt->step_max - i2;

    pdata->jstep = ( int * ) malloc ( n*sizeof ( int ) ); //new int(n);
    if ( pdata->jstep == NULL )
        {
        fprintf ( stderr,"JSTEP allocation error!!!\n" );
        }
    tstep = sec2timec ( vdt->incr_sec );
    for ( i=0; i<n; i++ )
        {
        j = JSTEP3 ( vis_->info->sdate+i, vis_->info->stime+i,
                     vdt->sdate, vdt->stime,&tstep )-1;
        if ( j >= n1 ) j = -3;
        pdata->jstep[i] = j;
        }

    plotDataListP_->addTail ( pdata );
    switch ( overlay_mode_ )
        {
        case OBS_PLOT:
            obs_size_ = OBS_SIZE;
            obs_thick_ = OBS_THICK;
        case OBSVECTOR_PLOT:
            obs_size_ = 9;
            obs_thick_ = pdata->vect2d->vect_thickness;
            map = projmap_generate ( vis_->info, vis_->mapName_, message );
            map->copyParameters ( &params );

            // rotate vectors
            if ( overlay_mode_ == OBSVECTOR_PLOT && params.gdtyp != LATGRD3 )
                {
                float alp;
                float cos_a, sin_a;
                float u, v;
                float k;
                float lon;
                float lon0;
                float deg2rad = M_PI/180.;
                switch ( params.gdtyp )
                    {
                    case  LAMGRD3:
                        float xn;
                        float sign;
                        float phi1, phi2;

                        if ( params.p_gam < 0.0 )
                            {
                            sign = -1.0;            // Southern hemisphere
                            }
                        else
                            {
                            sign = 1.0;             // Northern hemisphere
                            }

                        phi1 = sign * params.p_alp;
                        phi2 = sign * params.p_bet;

                        xn = ( log ( cos ( phi1*deg2rad ) ) -
                               log ( cos ( phi2*deg2rad ) ) ) /
                             ( log ( tan ( ( 45.0 - 0.5*sign*phi1 ) *deg2rad ) ) -
                               log ( tan ( ( 45.0 - 0.5*sign*phi2 ) *deg2rad ) ) );

                        k = xn;
                        lon0 = params.xcent;
                        break;
                    case POLGRD3:
                        k = 1.0;
                        lon0 = params.p_gam;
                        break;
                    default:
                        fprintf ( stderr,"Do not know how to rotate vectors for GRIDTYPE=%d\n",
                                  params.gdtyp );
                        k = 0.0; // this effectivelly disables rotation
                        lon0 = 0.0;
                    }
                // now make the rotation
                n = vdt->ncol * n1;
                for ( j=0; j<n; j++ )
                    {
                    lon = pdata->coord_x[j];
                    alp = k * ( lon - lon0 ) * deg2rad;
                    cos_a = cos ( alp );
                    sin_a = sin ( alp );
                    u = pdata->vect2d->vdata_x->grid[j];
                    v = pdata->vect2d->vdata_y->grid[j];
                    if ( !isnanf ( u ) && !isnanf ( v ) )
                        {
                        pdata->vect2d->vdata_x->grid[j] = u*cos_a-v*sin_a;
                        pdata->vect2d->vdata_y->grid[j] = u*sin_a+v*cos_a;
                        }
                    }
                }
            MapProjection mapProjection;
            createMapProjection ( &params, &mapProjection );
            if ( setMapProjection ( &mapProjection ) )
                {
                double lat, lon;
                double x, y;
                double projectedXOrigin;
                double projectedYOrigin;
                float  origx, origy, dx, dy;

                computeProjectedGridOrigin ( &params,
                                             &projectedXOrigin,
                                             &projectedYOrigin );

                dx = params.xcell;
                dy = params.ycell;
                origx = projectedXOrigin;
                origy = projectedYOrigin;
                n = vdt->ncol * n1;
                for ( j=0; j<n; j++ )
                    {
                    lon = pdata->coord_x[j];
                    lat = pdata->coord_y[j];

                    projectLatLon ( lat, lon, &x, &y );

                    // now scale the monitor coordinates
                    pdata->coord_x[j] = ( x-origx ) /dx;
                    pdata->coord_y[j] = ( y-origy ) /dy;

                    }
                }

#ifdef DEBUG
            fprintf ( stderr, "params->gdtyp == %d\n", params.gdtyp );
            fprintf ( stderr, "params->p_alp == %f\n", params.p_alp );
            fprintf ( stderr, "params->p_bet == %f\n", params.p_bet );
            fprintf ( stderr, "params->p_gam == %f\n", params.p_gam );
            fprintf ( stderr, "params->xcent == %f\n", params.xcent );
            fprintf ( stderr, "params->ycent == %f\n", params.ycent );
            fprintf ( stderr, "params->nrows == %d\n", params.nrows );
            fprintf ( stderr, "params->ncols == %d\n", params.ncols );
            fprintf ( stderr, "params->xorig == %f\n", params.xorig );
            fprintf ( stderr, "params->yorig == %f\n", params.yorig );
            fprintf ( stderr, "params->xcell == %f\n", params.xcell );
            fprintf ( stderr, "params->ycell == %f\n", params.ycell );


            int j;
            for ( j=0; j<vdt->ncol; j++ )
                fprintf ( stderr,"DEBUG: monitor at: (%f, %f)\n",
                          pdata->coord_x[j],
                          pdata->coord_y[j] );
#endif

            break;
        case CONTOUR_PLOT:
            if ( !compute_this_contour ( pdata, n1, 1, cntr_lvls ) ) goto cleanup;

            if ( cntr_dialog_ )
                {
                if ( XtIsManaged ( cntr_dialog_ ) )
                    {
                    XtUnmanageChild ( cntr_dialog_ );
                    cntr_dialog_=NULL;
                    configurecntr_cb();
                    }
                }
            break;
        }

    resize();

cleanup:
    //  XtFree(newtext);
    return;
    }
// ============================================================

#define max(a,b) ((a) > (b) ? (a) : (b))

void TileWnd::overlay_ts()
    {
    if ( vis_ )
        {
        int x1, x2, y1, y2;

        x1 = s.scalex ( ( float ) ( vis_->col_min_ )-1.0 );
        x2 = s.scalex ( ( float ) ( vis_->col_max_ ) );
        y1 = s.scaley ( ( float ) ( vis_->row_max_ ) );
        y2 = s.scaley ( ( float ) ( vis_->row_min_ )-1.0 );
        overlay_ts ( x1, x2, y1, y2 );
        }
    }

void TileWnd::overlay_ts ( int x1, int x2, int y1, int y2 )
    {
    float *x;
    float *tsdata;
    static char *plotSymbol[] = { "Circle","Diamond",
                                  "Plus","Cross"
                                };
    static char *plotColor[] = { "red", "yellow","magenta", "cyan", "purple", "khaki", "orange" };

    char **color, **symbol;
    char **legend = NULL;
    int  *npoints;
    char message[256];
    int nsteps;
    int n, ts, t;
    int i, j, k;
    int ni, nj;
    int imax, nperstep;
    int nlines;
    float xx1, xx2, yy1, yy2;
    float val;
    float px, py;
    PLOT_DATA *pdata;
    int found;
    char *title;

    nsteps = vis_->step_max_-vis_->step_min_+1;

    /* not needed anymore
    x1 = vis_->col_min_;
    x2 = vis_->col_max_;
    y1 = vis_->row_min_;
    y2 = vis_->row_max_;
    */

    ni = vis_->col_max_ - vis_->col_min_ + 1;
    nj = vis_->row_max_ - vis_->row_min_ + 1;

    if ( x1 == x2 && y1 == y2 )
        {
        xx1 = s.gridx ( x1-obs_size_ );
        xx2 = s.gridx ( x2+obs_size_ );
        yy1 = s.gridy ( y2+obs_size_ );
        yy2 = s.gridy ( y1-obs_size_ );
        }
    else
        {
        xx1 = s.gridx ( x1 );
        xx2 = s.gridx ( x2 );
        yy1 = s.gridy ( y2 );
        yy2 = s.gridy ( y1 );
        }

    //  units = vis_->getUnits();



    imax=0;
    for ( t = 0; t < nsteps; t++ )
        {
        pdata = ( PLOT_DATA * ) plotDataListP_->head();
        while ( pdata )
            {
            if ( pdata->plot_type == OBS_PLOT )
                {
                ts = pdata->jstep[t];
                if ( ts>=0 )
                    {
                    n=pdata->vdata->ncol;
                    if ( legend == NULL )
                        {
                        legend = ( char ** ) malloc ( n*sizeof ( char * ) );

                        if ( !legend )
                            {
                            fprintf ( stderr,
                                      "Memory allocation failure in TileWnd::overlay_ts()!\n" );
                            return;
                            }
                        }

                    for ( j=0; j<n; j++ )
                        {
                        k = n*ts+j;
                        px = pdata->coord_x[k];
                        py = pdata->coord_y[k];
                        if ( ( px >= xx1 ) && ( px <= xx2 ) && ( py >= yy1 ) && ( py <= yy2 ) )
                            {
                            found = 0;
                            for ( i=0; i<imax; i++ )
                                {
                                if ( !strcmp ( pdata->stnid[k], legend[i] ) )
                                    {
                                    found = 1;
                                    break;
                                    }
                                }
                            if ( !found )
                                {
                                legend[imax] = strdup ( pdata->stnid[k] );
                                imax++;
                                }
                            }
                        }
                    }
                }
            pdata = ( PLOT_DATA * ) plotDataListP_->next();
            }
        }
    if ( imax == 0 )
        {
        fprintf ( stderr,"Error in overlay_ts: IMAX=0\n" );
        fprintf ( stderr,"In overlay_ts with (%d,%d) - (%d,%d)\n", x1,y1,x2,y2 );
        fprintf ( stderr,"In overlay_ts with (%f,%f) - (%f,%f)\n", xx1,yy1,xx2,yy2 );
        return;
        }


    nlines = imax+1;
    legend[imax] = "Avg Obs";
#ifdef MODEL_LINES
    nlines = imax+2;
    if ( ! ( legend[imax+1] = getenv ( "PAVE_MODELNAME" ) ) )
        {
        legend[imax+1] = "Model";
        }
#endif

    //  tsdata = new float(nsteps*nlines);
    tsdata =  ( float * ) malloc ( nsteps*nlines*sizeof ( tsdata[0] ) );
    if ( !tsdata )
        {
        fprintf ( stderr,
                  "Memory allocation failure in TileWnd::overlay_ts()!\n" );
        return;
        }

    //  x = new float(nsteps*nlines);
    x = ( float * ) malloc ( nsteps*nlines*sizeof ( x[0] ) );
    npoints = ( int * ) malloc ( nlines*sizeof ( npoints[0] ) );
    color = ( char ** ) malloc ( nlines*sizeof ( char * ) );
    symbol = ( char ** ) malloc ( nlines*sizeof ( char * ) );
    if ( !x || !npoints || !color || !symbol )
        {
        fprintf ( stderr,
                  "Memory allocation failure in TileWnd::overlay_ts()!\n" );
        return;
        }

    for ( i=0; i<nlines; i++ )
        {
        npoints[i] = nsteps;
        for ( t = vis_->step_min_; t <= vis_->step_max_; t++ )
            {
            x[nsteps*i+t-vis_->step_min_] = ( float ) ( t-1 );
            }
        }

    for ( i = 0; i < nlines; i++ )
        {
        if ( i== ( nlines-1 ) )
            {
            symbol[i] = "line";
            color[i] = "blue";
            }
        else if ( i==imax )
            {
            symbol[i] = "square";
            color[i] = "green";
            }
        else
            {
            j = ( i % ( sizeof ( plotSymbol ) /sizeof ( plotSymbol[0] ) ) );
            symbol[i] = plotSymbol[j];
            j = ( i % ( sizeof ( plotColor ) /sizeof ( plotColor[0] ) ) );
            color[i] = plotColor[j];
            }
        }


    for ( t = 0; t < nsteps; t++ )
        {
        /* initialize ts plot to missing */
        val = setNaNf();
        for ( i=0; i<nlines; i++ )
            {
            tsdata[nsteps*i + t] = val;
            }


#ifdef MODEL_LINES
        tsdata[nsteps* ( imax+1 )+t] = 0.0;
        nperstep = 0;
        for ( j = y1; j <= y2; j++ )
            {
            for ( i = x1; i <= x2; i++ )
                {
                val = vis_->info->grid[INDEX ( i-x1,
                                               j-y1,
                                               0, t, ni, nj, 1 )];

                if ( !isnanf ( val ) )
                    {
                    tsdata[nsteps* ( imax+1 )+t] += val;
                    nperstep++;
                    }
                }
            }
        tsdata[nsteps* ( imax+1 )+t] /= nperstep; // model values
#endif

        pdata = ( PLOT_DATA * ) plotDataListP_->head();
        nperstep = 0;
        while ( pdata )
            {
            if ( pdata->plot_type == OBS_PLOT )
                {
                ts = pdata->jstep[t];
                if ( ts>=0 )
                    {
                    n=pdata->vdata->ncol;
                    tsdata[nsteps*imax+t] = 0.0;
                    for ( j=0; j<n; j++ )
                        {
                        k = n*ts+j;
                        px = pdata->coord_x[k];
                        py = pdata->coord_y[k];
                        if ( ( px >= xx1 ) && ( px <= xx2 ) && ( py >= yy1 ) && ( py <= yy2 ) )
                            {
                            val = pdata->vdata->grid[k];
                            if ( !isnanf ( val ) )
                                {
                                tsdata[nsteps*imax+t] += val; // cumulative average obs
                                nperstep++;
                                }

                            for ( i=0; i<imax; i++ )
                                {
                                if ( !strcmp ( pdata->stnid[k], legend[i] ) ) break;
                                }
                            if ( i>=imax )
                                {
                                fprintf ( stderr,"%s\n","Something is wrong ... No 2d plot" );
                                return;
                                }
                            tsdata[nsteps*i + t] = val; // obs values
                            }
                        }
                    }
                }
            pdata = ( PLOT_DATA * ) plotDataListP_->next();
            }
        tsdata[nsteps*imax+t] /= nperstep; // average obs

        }

    title = strdup ( vis_->title1_ );
    //fprintf(stderr,"DEBUG:: title=%s\n",vis_->title1_);
    if ( !graph2d ( x, tsdata, nlines, npoints, title, "time step",
                    "y-Axis", legend, symbol, color, message, 1 ) )
        fprintf ( stderr, "\007%s\n", message );


    /***********************************
    if (tsdata) delete [] tsdata; tsdata = NULL;
    if (x) delete [] x; x = NULL;
    if (npoints) delete [] npoints; npoints = NULL;

    if (color) free(color); color = NULL;
    if (symbol) free(symbol); symbol = NULL;
    if (legend) free(legend); legend = NULL;
    *************************************/
    }

// ============================================================
int TileWnd::compute_this_contour ( PLOT_DATA *pdata, int n1, int auto_lvl,
                                    float *cntr_lvls )
    {
    float *x;
    float *y;
    int i, k;
    int nx;
    int ny;
    int ncol;
    int nc;
    int lc;
    float *z;
    float zmin, zmax;

    nx = pdata->vdata->ncol;
    ny = pdata->vdata->nrow;
    if ( ( nx != ( vis_->col_max_-vis_->col_min_+1 ) ) ||
            ( ny != ( vis_->row_max_-vis_->row_min_+1 ) ) )
        {
        fprintf ( stderr,"ERROR: domain mismatch" );
        // free(pdata);
        return 0;
        }

    ncol = pdata->cntr_data->n_levels;

    x = new float[nx];
    y = new float[ny];

    for ( i=0; i<nx; i++ )
        {
        x[i]=i+0.5;
        }

    for ( i=0; i<ny; i++ )
        {
        y[i]=i+0.5;
        }

    lc = 1;

#if 0
    if ( pdata->cntr_data->cntr_list )
        {
        for ( i=0; i<n1; i++ )
            {
            free ( pdata->cntr_data->cntr_list[i] );
            }
        free ( pdata->cntr_data->cntr_list );
        }
#endif

    pdata->cntr_data->cntr_list = ( Cntr_line ** ) malloc ( n1*sizeof ( Cntr_line * ) );
    //    pdata->cntr_list = new (Cntr_line **)[n1];

    zmin = zmax = ( pdata->vdata->grid ) [0];
    for ( i=0; i<n1; i++ )
        {
        z = ( pdata->vdata->grid ) + INDEX ( 0,0,0,i,nx,ny,1 );
        for ( k=0; k<nx*ny; k++ )
            {
            zmin = MIN ( zmin, z[k] );
            zmax = MAX ( zmax, z[k] );
            }
        }

    if ( auto_lvl )
        {
        auto_cntr_levels ( zmin, zmax, ncol, cntr_lvls, &nc );
        ncol = nc;

        pdata->cntr_data->n_levels = nc;
        pdata->cntr_data->cntr_lvls = new float[nc];
        }

    for ( i=0; i<ncol; i++ )
        {
        pdata->cntr_data->cntr_lvls[i] = cntr_lvls[i];
        }

    for ( i=0; i<n1; i++ )
        {
        ctr_line = NULL;
        z = ( pdata->vdata->grid ) + INDEX ( 0,0,0,i,nx,ny,1 );
        compute_contour ( z, x, y, nx, ny, ncol, lc, cntr_lvls );
        pdata->cntr_data->cntr_list[i] = ctr_line;
        }

    delete [] x;
    delete [] y;
    return 1;
    }

// ============================================================

void TileWnd::drawOverlays ( int t )
    {

    // loop over the overlays
    int k, mlx, mly;
    int llx, lly, urx, ury;
    float x, y;
    float x2, y2;
    float val;
    float val_x, val_y;
    float dx, dy;
    float mag;
    PLOT_DATA *pdata;
    XPoint points[4096];

    pdata = ( PLOT_DATA * ) plotDataListP_->head();
    Display *display = XtDisplay ( canvas_ );

    int                   ts;
    int               scr;
    int                   cindex;
    GC            blackGC, gc;
    XGCValues     values;
    scr = DefaultScreen ( display );

    values.foreground = BlackPixel ( display, scr );
    values.background = WhitePixel ( display, scr );
    blackGC = XCreateGC ( display, RootWindow ( display,scr ),
                          ( GCForeground|GCBackground ), &values );


    while ( pdata )
        {
        ts = pdata->jstep[t];
        if ( ts>=0 )
            {
            if ( pdata->plot_type == OBS_PLOT )
                {
                int n=pdata->vdata->ncol;
                int j;

                values.line_width = obs_thick_;
                XChangeGC ( display, blackGC, GCLineWidth, &values );
                for ( j=0; j<n; j++ )
                    {
                    k = n*ts+j;
                    x = pdata->coord_x[k];
                    y = pdata->coord_y[k];

                    val = pdata->vdata->grid[k];
                    if ( !isnanf ( val ) )
                        {

                        cindex = colorIndex ( val );

                        if ( cindex < 0 ) cindex = 0;
                        gc = color_gc_table_[cindex];
                        mlx = s.scalex ( x );
                        mly = s.scaley ( y );

                        llx = mlx - obs_size_;
                        lly = mly - obs_size_;
                        urx = mlx + obs_size_;
                        ury = mly + obs_size_;
                        if ( x >= s.xmin_ && y >= s.ymin_ &&
                                x <  s.xmax_ && y <  s.ymax_ )
                            {
                            points[0].x = mlx;
                            points[0].y = lly;
                            points[1].x = urx;
                            points[1].y = mly;
                            points[2].x = mlx;
                            points[2].y = ury;
                            points[3].x = llx;
                            points[3].y = mly;
                            points[4].x = points[0].x;
                            points[4].y = points[0].y;

                            XFillPolygon ( display, pix_, gc, points, 4,
                                           Convex, CoordModeOrigin );
                            XDrawLines ( display, pix_, blackGC, points, 5,
                                         CoordModeOrigin );
                            }
                        }
                    }
                } // OBS_PLOT
            if ( pdata->plot_type == OBSVECTOR_PLOT
                    && scale_vectors_on_ )
                {
                int n=pdata->vect2d->vdata_x->ncol;
                int j;
                char *colorname;
                XColor color, exactColor;
                int scr = DefaultScreen ( display );
                Colormap cmap = DefaultColormap ( display, scr );


                colorname=getenv ( "PAVE_VECTOBS_COLOR" );
                if ( colorname != NULL )
                    {
                    if ( !XAllocNamedColor ( display, cmap, colorname,
                                             &color, &exactColor ) )
                        {
                        fprintf ( stderr, "Can't allocate color %s\n", colorname );
                        }

                    XSetForeground ( display,
                                     gc_,
                                     color.pixel );
                    }
                //      values.line_width = pdata->vect2d->vect_thickness;
                values.line_width = obs_thick_;
                XChangeGC ( display, gc_, GCLineWidth, &values );

                mag = 0.1 * ( s.scalex ( s.xmax_ )-s.scalex ( s.xmin_ ) ) /vector_scale_;

                for ( j=0; j<n; j++ )
                    {
                    k = n*ts+j;
                    x = pdata->coord_x[k];
                    y = pdata->coord_y[k];

                    val_x = pdata->vect2d->vdata_x->grid[k];
                    val_y = pdata->vect2d->vdata_y->grid[k];
                    if ( !isnanf ( val_x ) && !isnanf ( val_y ) )
                        {

                        if ( x >= s.xmin_ && y >= s.ymin_ &&
                                x <  s.xmax_ && y <  s.ymax_ )
                            {

                            mlx = s.scalex ( x );
                            mly = s.scaley ( y );

                            dx = val_x * mag;
                            dy = val_y * mag;
                            x = mlx;
                            y = mly;
                            x2 = x + dx;
                            y2 = y - dy;
                            drawVect ( x, y, x2, y2,obs_size_ );
                            }
                        }
                    }
                values.line_width = 1;
                XChangeGC ( display, gc_, GCLineWidth, &values );
                setForeground ( "black" );


                } // OBSVECTOR_PLOT
            else if ( pdata->plot_type == CONTOUR_PLOT )
                {
                int i, k;
                Cntr_line *cline;


                int n_cntr_labels;


                values.line_width = pdata->cntr_data->cntr_thickness;
                XChangeGC ( display, pdata->cntr_data->gc, GCLineWidth, &values );

                for ( cline=pdata->cntr_data->cntr_list[ts]; cline != NULL; cline=cline->next )
                    {

                    k = cline->npoints;
                    for ( i=0; i<k; i++ )
                        {
                        points[i].x = s.scalex ( cline->x[i] );
                        points[i].y = s.scaley ( cline->y[i] );
                        }
                    gc = pdata->cntr_data->gc;
                    XDrawLines ( display, pix_, gc, points, k, CoordModeOrigin );
                    // annotation
                    if ( pdata->cntr_data->labels_on )
                        {
                        n_cntr_labels = 0;
                        XSetFont ( display, gc, def_font_->fid );
                        if ( k > 14 && n_cntr_labels < 15 )
                            {
                            int xt, yt;
                            int j;
                            char text[128];
                            char *lvl_fmt="%g";
                            if ( n_cntr_labels%2 ==1 )
                                {
                                j = k<7 ? k-3 : 4; /* default 4 */
                                j = k<2 ? 2 : k ; /*minimum of k: 2*/
                                }
                            else
                                {
                                j = k<7 ? k-1 : k-3;  /*default n-3 */
                                }
                            xt= points[j].x;
                            yt= points[j].y;
                            sprintf ( text, lvl_fmt,
                                      pdata->cntr_data->cntr_lvls[cline->color-1] );
                            XDrawString ( display, pix_, gc, xt, yt, text, strlen ( text ) );
                            n_cntr_labels++;
                            }
                        } // end of annotation
                    }

                } // CONTOUR_PLOT
            } // valid time step
        pdata = ( PLOT_DATA * ) plotDataListP_->next();
        }

    XFreeGC ( display, blackGC );

    }

void TileWnd::createObsDialog()
    {

    Widget close;
    Widget obs_size_wgt, obs_thick_wgt;

    // Create control dialog box.

    obs_dialog_ = XmCreateFormDialog ( canvas_, "OBS Control", NULL, 0 );
    XtVaSetValues ( obs_dialog_,
                    XmNwidth,               550,
                    XmNheight,              160,
                    XmNautoUnmanage,        False,
                    NULL );

    // Create tiles widget.

    obs_size_wgt = XtVaCreateManagedWidget ( "OBS size",
                   xmScaleWidgetClass, obs_dialog_,
                   XtVaTypedArg, XmNtitleString, XmRString, "Symbol size", OBS_SIZE,
                   XmNmaximum,             64,
                   XmNminimum,             1,
                   XmNvalue,               obs_size_,
                   XmNorientation,         XmHORIZONTAL,
                   XmNtopAttachment,       XmATTACH_FORM,
                   XmNleftAttachment,      XmATTACH_FORM,
                   XmNleftOffset,          30,
                   XmNrightAttachment,     XmATTACH_FORM,
                   XmNrightOffset,         30,
                   XmNshowValue,           True,
                   NULL );

    XtAddCallback ( obs_size_wgt, XmNvalueChangedCallback, &TileWnd::obs_sizeCB, ( XtPointer ) this );

    obs_thick_wgt = XtVaCreateManagedWidget ( "OBS thick",
                    xmScaleWidgetClass, obs_dialog_,
                    XtVaTypedArg, XmNtitleString, XmRString, "Symbol thickness", OBS_THICK,
                    XmNmaximum,             10,
                    XmNminimum,             1,
                    XmNvalue,               obs_thick_,
                    XmNorientation,         XmHORIZONTAL,
                    XmNtopAttachment,       XmATTACH_WIDGET,
                    XmNtopWidget,           obs_size_wgt,
                    XmNleftAttachment,      XmATTACH_FORM,
                    XmNleftOffset,          30,
                    XmNrightAttachment,     XmATTACH_FORM,
                    XmNrightOffset,         30,
                    XmNshowValue,           True,
                    NULL );

    XtAddCallback ( obs_thick_wgt, XmNvalueChangedCallback, &TileWnd::obs_thickCB, ( XtPointer ) this );

    close = XtVaCreateManagedWidget ( "Close",
                                      xmPushButtonWidgetClass, obs_dialog_,
                                      XmNtopAttachment,       XmATTACH_WIDGET,
                                      XmNtopWidget,           obs_thick_wgt,
                                      XmNtopOffset,           0,
                                      XmNrightAttachment,     XmATTACH_FORM,
                                      XmNrightOffset,         30,
                                      XmNbottomOffset,        0,
                                      XmNwidth,               100,
                                      XmNheight,              35,
                                      NULL );
    XtAddCallback ( close, XmNactivateCallback, &TileWnd::closeObsDialogCB, ( XtPointer ) this );

    }

void TileWnd::closeObsDialogCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->close_obs_dialog_cb();
    }


void TileWnd::close_obs_dialog_cb()
    {
    XtUnmanageChild ( obs_dialog_ );
    }

void TileWnd::closeCntrDialogCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->close_cntr_dialog_cb();
    }


void TileWnd::close_cntr_dialog_cb()
    {
    XtUnmanageChild ( cntr_dialog_ );
    }

void TileWnd::obs_sizeCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->obs_size_cb ( cbs->value );
    }

void TileWnd::obs_size_cb ( int valu )
    {

    if ( valu != obs_size_ )
        {
        obs_size_ = valu;
        resize();
        }
    }

void TileWnd::obs_thickCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->obs_thick_cb ( cbs->value );
    }

void TileWnd::obs_thick_cb ( int valu )
    {

    if ( valu != obs_thick_ )
        {
        obs_thick_ = valu;
        resize();
        }
    }

void TileWnd::ncontoursCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->ncontours_cb ( cbs->value );
    }

void TileWnd::ncontours_cb ( int valu )
    {

    PLOT_DATA *pdata = current_pdata_cntr_config_;

    if ( valu != pdata->cntr_data->n_levels )
        {
        pdata->cntr_data->n_levels = valu;
        int n1 = pdata->vdata->step_max - pdata->vdata->step_min + 1;

        if ( XtIsManaged ( cntr_dialog_ ) ) XtUnmanageChild ( cntr_dialog_ );
        cntr_dialog_=NULL;
        configurecntr_cb();

        compute_this_contour ( pdata, n1, 0, pdata->cntr_data->cntr_lvls );
        resize();

        }
    }

void TileWnd::cntr_thickCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->cntr_thick_cb ( cbs->value );
    }

void TileWnd::cntr_thick_cb ( int valu )
    {
    if ( valu != current_pdata_cntr_config_->cntr_data->cntr_thickness )
        {
        current_pdata_cntr_config_->cntr_data->cntr_thickness = valu;
        resize();
        }
    }

void TileWnd::cnfg_cntrCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->cnfg_cntr_cb ( cbs->value );
    }

void TileWnd::cnfg_cntr_cb ( int valu )
    {
    int n_cntr;
    PLOT_DATA *pdata;

    n_cntr = 0;
    pdata = ( PLOT_DATA * ) plotDataListP_->head();
    while ( pdata )
        {
        if ( pdata->plot_type == CONTOUR_PLOT )
            {
            n_cntr++;
            if ( n_cntr == valu )
                {
                current_pdata_cntr_config_ = pdata;
                if ( XtIsManaged ( cntr_dialog_ ) ) XtUnmanageChild ( cntr_dialog_ );
                cntr_dialog_=NULL;
                configurecntr_cb();
                return;
                }
            }
        pdata = ( PLOT_DATA * ) plotDataListP_->next();
        }
    }


void TileWnd::createCntrDialog()
    {

    Widget config_n;
    Widget ncontours_wgt;
    Widget thick_wgt;
    Widget close;
    Widget text_w,  rc;
    int i;
    int n_cntr;
    int n;
    int curr_cntr;
    char buf[80];
    PLOT_DATA *pdata;

    n_cntr = 0;
    curr_cntr = 1;
    pdata = ( PLOT_DATA * ) plotDataListP_->head();
    while ( pdata )
        {
        if ( pdata->plot_type == CONTOUR_PLOT )
            {
            n_cntr++;
            if ( current_pdata_cntr_config_ == NULL )
                {
                current_pdata_cntr_config_ = pdata;
                }
            else if ( current_pdata_cntr_config_ == pdata )
                {
                curr_cntr = n_cntr;
                }

            }
        pdata = ( PLOT_DATA * ) plotDataListP_->next();
        }

    // Create control dialog box.

    cntr_dialog_ = XmCreateFormDialog ( canvas_, "CNTR Control", NULL, 0 );
    XtVaSetValues ( cntr_dialog_,
                    XmNwidth,               550,
                    XmNheight,              600,
                    XmNautoUnmanage,        False,
                    NULL );

    if ( n_cntr > 1 )
        {
        // Create Config_n widget.
        config_n = XtVaCreateManagedWidget ( "Config_n",
                                             xmScaleWidgetClass, cntr_dialog_,
                                             XtVaTypedArg, XmNtitleString, XmRString, "Configure Contour #", 1,
                                             XmNmaximum,             n_cntr,
                                             XmNminimum,             1,
                                             XmNvalue,               curr_cntr,
                                             XmNorientation,         XmHORIZONTAL,
                                             XmNtopAttachment,       XmATTACH_FORM,
                                             XmNleftAttachment,      XmATTACH_FORM,
                                             XmNleftOffset,          30,
                                             XmNrightAttachment,     XmATTACH_FORM,
                                             XmNrightOffset,         30,
                                             XmNshowValue,           True,
                                             NULL );
        XtAddCallback ( config_n, XmNvalueChangedCallback, &TileWnd::cnfg_cntrCB, ( XtPointer ) this );
        ncontours_wgt = XtVaCreateManagedWidget ( "NContours",
                        xmScaleWidgetClass, cntr_dialog_,
                        XtVaTypedArg, XmNtitleString, XmRString, "# Contours", 8,
                        XmNmaximum,             32,
                        XmNminimum,             1,
                        XmNvalue,               current_pdata_cntr_config_->cntr_data->n_levels,
                        XmNorientation,         XmHORIZONTAL,
                        XmNtopAttachment,       XmATTACH_WIDGET,
                        XmNtopWidget,           config_n,
                        XmNleftAttachment,      XmATTACH_FORM,
                        XmNleftOffset,          30,
                        XmNrightAttachment,     XmATTACH_FORM,
                        XmNrightOffset,         30,
                        XmNshowValue,           True,
                        NULL );
        }
    else
        {
        ncontours_wgt = XtVaCreateManagedWidget ( "NContours",
                        xmScaleWidgetClass, cntr_dialog_,
                        XtVaTypedArg, XmNtitleString, XmRString, "# Contours", 8,
                        XmNmaximum,             32,
                        XmNminimum,             1,
                        XmNvalue,               current_pdata_cntr_config_->cntr_data->n_levels,
                        XmNorientation,         XmHORIZONTAL,
                        XmNtopAttachment,       XmATTACH_FORM,
                        XmNleftAttachment,      XmATTACH_FORM,
                        XmNleftOffset,          30,
                        XmNrightAttachment,     XmATTACH_FORM,
                        XmNrightOffset,         30,
                        XmNshowValue,           True,
                        NULL );
        }
    XtAddCallback ( ncontours_wgt, XmNvalueChangedCallback, &TileWnd::ncontoursCB, ( XtPointer ) this );

    thick_wgt = XtVaCreateManagedWidget ( "ThickCntr",
                                          xmScaleWidgetClass, cntr_dialog_,
                                          XtVaTypedArg, XmNtitleString, XmRString, "Thickness", 3,
                                          XmNmaximum,             8,
                                          XmNminimum,             1,
                                          XmNvalue,               current_pdata_cntr_config_->cntr_data->cntr_thickness,
                                          XmNorientation,         XmHORIZONTAL,
                                          XmNtopAttachment,       XmATTACH_WIDGET,
                                          XmNtopWidget,           ncontours_wgt,
                                          XmNleftAttachment,      XmATTACH_FORM,
                                          XmNleftOffset,          30,
                                          XmNrightAttachment,     XmATTACH_FORM,
                                          XmNrightOffset,         30,
                                          XmNshowValue,           True,
                                          NULL );
    XtAddCallback ( thick_wgt, XmNvalueChangedCallback, &TileWnd::cntr_thickCB, ( XtPointer ) this );

    n = current_pdata_cntr_config_->cntr_data->n_levels;
    rc = XtVaCreateManagedWidget ( "rc", xmRowColumnWidgetClass,
                                   cntr_dialog_,
                                   XmNpacking,        XmPACK_COLUMN,
                                   XmNnumColumns,     n,
                                   XmNorientation,    XmHORIZONTAL,
                                   XmNisAligned,      True,
                                   XmNentryAlignment, XmALIGNMENT_END,
                                   XmNtopAttachment,  XmATTACH_WIDGET,
                                   XmNtopWidget,      thick_wgt,
                                   NULL );


    for ( i=0; i<n; i++ )
        {
        TransferObject *obj;
        obj = new TransferObject();
        sprintf ( buf,"level %d",i+1 );
        XtVaCreateManagedWidget ( buf, xmLabelWidgetClass, rc, NULL );

        text_w = XtVaCreateManagedWidget ( "text_w",
                                           xmTextWidgetClass,   rc, NULL );
        sprintf ( buf, "%g",
                  current_pdata_cntr_config_->cntr_data->cntr_lvls[i] );
        XmTextSetString ( text_w, buf );

        obj->obj=this;
        obj->lvl=i;
        XtAddCallback ( text_w, XmNactivateCallback, &TileWnd::cntr_levelCB, ( XtPointer ) obj );
        }
    XtManageChild ( rc );


    close = XtVaCreateManagedWidget ( "Close",
                                      xmPushButtonWidgetClass, cntr_dialog_,
                                      XmNtopAttachment,       XmATTACH_WIDGET,
                                      XmNtopWidget,           rc,
                                      XmNtopOffset,           0,
                                      XmNrightAttachment,     XmATTACH_FORM,
                                      XmNrightOffset,         30,
                                      XmNbottomOffset,        0,
                                      XmNwidth,               100,
                                      XmNheight,              35,
                                      NULL );
    XtAddCallback ( close, XmNactivateCallback, &TileWnd::closeCntrDialogCB, ( XtPointer ) this );

    }

void TileWnd::cntr_levelCB ( Widget wgt, XtPointer client_data, XtPointer call_data )
    {
    int lvl;
    char *value = XmTextGetString ( wgt );
    TransferObject *obj = ( TransferObject * ) client_data;
    TileWnd *twnd = ( TileWnd * ) obj->obj;
    float val = atof ( value );
    lvl = obj->lvl;

    twnd->cntr_level_cb ( lvl, val );
    }

void TileWnd::cntr_level_cb ( int lvl, float val )
    {
    PLOT_DATA *pdata = current_pdata_cntr_config_;
    int n1 = pdata->vdata->step_max - pdata->vdata->step_min + 1;
    pdata->cntr_data->cntr_lvls[lvl] = val;

    compute_this_contour ( pdata, n1, 0, pdata->cntr_data->cntr_lvls );
    if ( XtIsManaged ( cntr_dialog_ ) ) XtUnmanageChild ( cntr_dialog_ );
    cntr_dialog_=NULL;
    configurecntr_cb();
    resize();
    }

TransferObject::TransferObject ( void )
    {
    obj = NULL;
    lvl = 0;
    }

void long2str ( int num, char *str )
    {

    unsigned long l;

    int m;
    int i, c;

    l = num;

    for ( i=0; i<6; i++ )
        {
        m = l % 37;
        if ( m == 0 )
            {
            c = ' ';
            }
        else if ( m < 11 )
            {
            c = m + '0' - 1;
            }
        else
            {
            c = m + 'A' - 11;
            }
        str[i]=c;
        l -= m;
        l /= 37;
        }
    str[i] = '\0';

    }


void TileWnd::setTZ ( int tzoff )
    {
    timezone_ = tzoff;
    resize();
    }

void TileWnd::setTZname ( char *tzin, char *tzout )
    {
    int t1, t2;

    t1 = zone2GMT ( tzin );
    t2 = zone2GMT ( tzout );

    if ( ( t1 != -999 ) && ( t2 != -999 ) )
        {
        tzname_ = tzout;
        setTZ ( t2-t1 );
        }
    }

void TileWnd::resetTZname()
    {
    tzname_ = NULL;
    }

typedef   struct _tz
    {
    char *name;
    int off;
    } TimeZone;

int TileWnd::zone2GMT ( char *zone )
    {
    TimeZone tz[] =
        {
        "GMT",   0,
        "CET",   1, // Central European
        "EET",   2, // Eastern European
        "AST",  -4, // Atlantic Standard
        "EST",  -5, // Eastern Standard
        "CST",  -6,
        "MST",  -7,
        "PST",  -8,
        "YST",  -9,
        "HST", -10,
        };
    int i;
    int n = sizeof ( tz ) /sizeof ( tz[0] );
    int found = 0;


    for ( i=0; i<n; i++ )
        {
        if ( !strcmp ( tz[i].name, zone ) )
            {
            found = 1;
            break;
            }
        }

    if ( !found ) return -999;
    else return tz[i].off;
    }

void TileWnd::setTitleFontSize ( int size )
    {
    if ( newTitleFontSize ( size ) ) resize();
    }

void TileWnd::setSubTitleFontSize ( int size )
    {
    if ( newSubTitleFontSize ( size ) ) resize();
    }




void TileWnd::createTitleFontDialog ( Widget parent )
    {
    Position xpos, ypos;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "In createTitlefontDialog 333\n" );
#endif // DIAGNOSTICS

    assert ( parent );


    // Create control dialog box.
    titlefont_dialog_ = XmCreateFormDialog ( parent, "control", NULL, 0 );

    // Position control dialog box so it doesn't obscure the plot
    XtVaGetValues ( parent, XmNx, &xpos, XmNy, &ypos, NULL );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "TileWnd::createTitlefontDialog's parent's xpos=%d, ypos=%d\n",
              ( int ) xpos, ( int ) ypos );
#endif // DIAGNOSTICS

    XtVaSetValues ( titlefont_dialog_,
                    XmNautoUnmanage,   False,
                    XmNdefaultPosition,    False,   // so won't be centered - SRT
                    XtNx,          xpos+35, // SRT 950911
                    XtNy,          ypos-90, // SRT 950911
                    NULL );

    Widget form1 = XtVaCreateWidget ( "form1",
                                      xmFormWidgetClass,  titlefont_dialog_,
                                      XmNtopAttachment,   XmATTACH_FORM,
                                      XmNtopOffset,       10,
                                      XmNleftAttachment,  XmATTACH_FORM,
                                      XmNleftOffset,      10,
                                      XmNrightAttachment, XmATTACH_FORM,
                                      XmNrightOffset,     10,
                                      NULL );

    titlefont_scale_ = XtVaCreateManagedWidget ( "Title font size",
                       xmScaleWidgetClass, form1,
                       XtVaTypedArg, XmNtitleString, XmRString, "Size", 5,
                       XmNheight,      100,
                       XmNmaximum,         99,
                       XmNminimum,     0,
                       XmNvalue,       titlefont_size_,
                       XmNshowValue,       True,
                       XmNorientation,     XmHORIZONTAL,
                       XmNtopAttachment,   XmATTACH_FORM,
                       XmNleftAttachment,  XmATTACH_FORM,
                       XmNrightAttachment, XmATTACH_FORM,
                       NULL );

    XtAddCallback ( titlefont_scale_, XmNvalueChangedCallback,  &TileWnd::fontSliderMovedCB, ( XtPointer ) this );


    subtitlefont_scale_ = XtVaCreateManagedWidget ( "Subtitle font size",
                          xmScaleWidgetClass, form1,
                          XtVaTypedArg, XmNtitleString, XmRString, "Size", 5,
                          XmNheight,      100,
                          XmNmaximum,         99,
                          XmNminimum,     0,
                          XmNvalue,       subtitlefont_size_,
                          XmNshowValue,       True,
                          XmNorientation,     XmHORIZONTAL,
                          XmNtopAttachment,   XmATTACH_WIDGET,
                          XmNtopWidget,           titlefont_scale_,
                          XmNleftAttachment,  XmATTACH_FORM,
                          XmNrightAttachment, XmATTACH_FORM,
                          NULL );

    XtAddCallback ( subtitlefont_scale_, XmNvalueChangedCallback,  &TileWnd::fontSliderMovedCB, ( XtPointer ) this );

    Widget sep = XtVaCreateManagedWidget ( "sep",
                                           xmSeparatorWidgetClass,     form1,
                                           XmNleftAttachment,      XmATTACH_FORM,
                                           XmNrightAttachment,     XmATTACH_FORM,
                                           XmNtopAttachment,       XmATTACH_WIDGET,
                                           XmNtopWidget,           subtitlefont_scale_,
                                           XmNtopOffset,           10,
                                           NULL );

    Widget ok = XtVaCreateManagedWidget ( "OK",
                                          xmPushButtonWidgetClass, form1,
                                          XmNtopAttachment,       XmATTACH_WIDGET,
                                          XmNtopWidget,           sep,
                                          XmNtopOffset,           10,
                                          XmNleftAttachment,      XmATTACH_FORM,
                                          XmNleftOffset,          10,
                                          XmNbottomAttachment,    XmATTACH_FORM,
                                          XmNbottomOffset,        10,
                                          XmNwidth,               100,
                                          XmNheight,              40,
                                          NULL );
    XtAddCallback ( ok, XmNactivateCallback, &titlefont_okCB, ( XtPointer ) this );

    Widget cancel = XtVaCreateManagedWidget ( "Cancel",
                    xmPushButtonWidgetClass, form1,
                    XmNtopAttachment,       XmATTACH_WIDGET,
                    XmNtopWidget,           sep,
                    XmNtopOffset,           10,
                    XmNleftAttachment,      XmATTACH_WIDGET,
                    XmNleftWidget,          ok,
                    XmNleftOffset,          10,
                    XmNrightAttachment,     XmATTACH_FORM,
                    XmNrightOffset,         10,
                    XmNwidth,               100,
                    XmNheight,              40,
                    NULL );
    XtAddCallback ( cancel, XmNactivateCallback, &titlefont_cancelCB, ( XtPointer ) this );


    if ( XtIsManaged ( form1 ) ) XtUnmanageChild ( form1 );
    XtManageChild ( form1 );

    }

void TileWnd::fontSliderMovedCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->fontSliderMoved_cb();
    }


void TileWnd::fontSliderMoved_cb()
    {
    int tSliderVal, sSliderVal;

    XtVaGetValues ( titlefont_scale_, XmNvalue, &tSliderVal, NULL );
    XtVaGetValues ( subtitlefont_scale_, XmNvalue, &sSliderVal, NULL );
    tSliderVal_ = tSliderVal;
    sSliderVal_ = sSliderVal;
    }

void TileWnd::titlefont_okCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->titlefont_ok_cb();
    }

void TileWnd::titlefont_ok_cb()
    {
    XtUnmanageChild ( titlefont_dialog_ );
    titlefont_size_ = tSliderVal_;
    subtitlefont_size_ = sSliderVal_;
    setTitleFontSize ( titlefont_size_ );
    setSubTitleFontSize ( subtitlefont_size_ );
    }

void TileWnd::titlefont_cancelCB ( Widget, XtPointer clientData, XtPointer )
    {
    TileWnd *obj = ( TileWnd * ) clientData;
    obj->titlefont_cancel_cb();
    }

void TileWnd::titlefont_cancel_cb()
    {
    XtUnmanageChild ( titlefont_dialog_ );
    }




#ifdef TZ_DIALOG

void TileWnd::createTimezoneDialog ( Widget parent )
    {

    TimeZone tz[] =
        {
        "GMT",   0,
        "CET",   1, // Central European
        "EET",   2, // Eastern European
        "AST",  -4, // Atlantic Standard
        "EST",  -5, // Eastern Standard
        "CST",  -6,
        "MST",  -7,
        "PST",  -8,
        "YST",  -9,
        "HST", -10,
        };
    int i;
    char t[256];

    assert ( parent );
    // Create control dialog box.
    Widget tz_dialog_ = XmCreateFormDialog ( parent, "TZ control", NULL, 0 );


    XtVaSetValues ( tz_dialog_,
                    XmNautoUnmanage,    False,
                    XmNdefaultPosition, False,   // so won't be centered - SRT
                    NULL );

    Widget form1 = XtVaCreateWidget ( "form1",
                                      xmFormWidgetClass,  tz_dialog_,
                                      XmNtopAttachment,   XmATTACH_FORM,
                                      XmNtopOffset,       10,
                                      XmNleftAttachment,  XmATTACH_FORM,
                                      XmNleftOffset,      10,
                                      XmNrightAttachment, XmATTACH_FORM,
                                      XmNrightOffset,     10,
                                      NULL );

    Widget radio_box = XmCreateRadioBox ( form1, "radio_box",
                                          NULL, 0 );

    for ( i = 0; i < sizeof ( tz ) /sizeof ( tz[0] ); i++ )
        {
        sprintf ( t,"%s (%d)", tz[i].name, tz[i].off );
        Widget one = XtVaCreateManagedWidget ( t,
                                               xmToggleButtonWidgetClass, radio_box, NULL );

        if ( i==0 ) XmToggleButtonSetState ( one, True, True );
        //  XtAddCallback(one, XmNvalueChangedCallback, &m->multiselect1CB, (XtPointer) m);

        }
    //  if (XtIsManaged(form1)) XtUnmanageChild(form1);XtManageChild(form1);
    if ( XtIsManaged ( tz_dialog_ ) ) XtUnmanageChild ( tz_dialog_ );
    XtManageChild ( tz_dialog_ );
    }
#endif
