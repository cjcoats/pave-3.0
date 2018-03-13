/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: DomainWnd.cc 83 2018-03-12 19:24:33Z coats $
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
//
// File:    DomainWnd.C
// Author:  K. Eng Pua
// Date:    Feb 22, 1995
//
//////////////////////////////////////////////////////////////////////////////
//
// Revision History
// SRT  951107  Added setRange()
//      Added int invalidateFormulasDependingOnMe(void);
// SRT  960525  Added saveDomain() and loadDomain()
//
//////////////////////////////////////////////////////////////////////////////


static const char SVN_ID[] = "$Id: DomainWnd.cc 83 2018-03-12 19:24:33Z coats $";


/* bald messes this up in some header file */
#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */

#include "DomainWnd.h"


// this constructor created from Eng Pua's original - SRT June 1995
DomainWnd::DomainWnd ( AppInit *app, char *name, ReadVisData *vis,
                       char *drawtype,
                       Dimension width, Dimension height,
                       char *percentsP,
                       char *mapinfo,
                       linkedList *formulaList,   // formulaList of its parent
                       int exit_button_on ) :
    DrawWnd ( app, name, drawtype, width, height, NULL, 0, 0 ),
    Menus(),
    RubberBand(),
    DomainBrowser_ ( draw_, ( char * ) "Domain", NULL,
                     ( void * ) &saveDOMAINCB, ( void * ) &loadDOMAINCB, ( void * ) this )
    {
    assert ( drawtype && percentsP && formulaList && mapinfo );

    vis_ = vis;
    exit_button_on_ = exit_button_on;
    percents_ = percentsP;
    formulaList_ = formulaList;
    mapinfo_ = strdup ( mapinfo );;
    if ( !mapinfo_ )
        fprintf ( stderr, "Can't allocate memory for mapinfo_ in Domain()\n" );

    initDomainWnd();

    createUI ( draw_ );
    initRubberBand ( &s, canvas_ );

    manage();

    if ( ( domain_mask_ = ( int * )
                          malloc ( vis_->col_max_ * vis_->row_max_ * sizeof ( int ) ) ) == NULL )
        {
        fprintf ( stderr, "Can't allocate memory for y_label_list_.\n" );

        }
    else
        {
        int i, j;

        for ( j=0; j < vis_->row_max_ ; j++ )
            for ( i=0; i < vis_->col_max_; i++ )
                domain_mask_[INDEX ( i,j,0,0,vis_->col_max_,vis_->row_max_,1 )] =
#ifdef DOMFIX
                    ( ( i>0 && j>0 ) ? 100 : 0 ); // SRT DOMFIX
#else // SRT DOMFIX
                    100; // SRT DOMFIX
#endif // DOMFIX

        }
    }


DomainWnd::~DomainWnd()
    {
    if ( mapinfo_ )
        {
        free ( mapinfo_ );
        mapinfo_ = NULL;
        }
    }


void DomainWnd::initDomainWnd()
    {
    interact_mode_ = MARKUP_MODE;
    probe_dialog_ = NULL;
    zoom_dialog_ = NULL;

    zoom_[0].GRID_X_MIN_ = vis_->col_min_-1; // SRT HACK 950707 added -1 to show all
    zoom_[0].GRID_Y_MIN_ = vis_->row_min_-1; // SRT HACK 950707 added -1 to show all
    zoom_[0].GRID_X_MAX_ = vis_->col_max_;
    zoom_[0].GRID_Y_MAX_ = vis_->row_max_;

    zoom_[0].parent = 0;
    }


void DomainWnd::showWindow ( void )
    {
    manage();
    drawDetail(); // 960913 Added SRT
    }


void DomainWnd::hideWindow ( void )
    {
    unmanage();
    }


int DomainWnd::setRange ( int xmin, int ymin, int xmax, int ymax ) // 1 based
    {
    int t, i, j;
    long dindex, pindex;

    // convert to 0 based
    xmin--;
    ymin--;
    xmax--;
    ymax--;

    // make sure its in range
    if ( xmin > xmax )
        {
        t    = xmin;
        xmin = xmax;
        xmax = t;
        }
    if ( ymin > ymax )
        {
        t    = ymin;
        ymin = ymax;
        ymax = t;
        }
    if ( xmin < 0 ) xmin = 0;
    if ( xmax >= vis_->col_max_ ) xmax = vis_->col_max_-1;
    if ( ymin < 0 ) ymin = 0;
    if ( ymax >= vis_->row_max_ ) ymax = vis_->row_max_-1;

    // set the values
    for ( j=0; j < vis_->row_max_; j++ )
        for ( i=0; i < vis_->col_max_; i++ )
            {
            dindex = j*vis_->col_max_ + i;
            domain_mask_[dindex] =
                ( i<xmin || i>xmax || j<ymin || j>ymax ) ? 0 : 100;
            if ( percents_ )
                {
                pindex = INDEX ( i,j,0,0,vis_->col_max_,vis_->row_max_,1 );
                percents_[pindex] = domain_mask_[dindex];
                }
            }

    invalidateFormulasDependingOnMe();
    return 0;
    }

void DomainWnd::createUI ( Widget parent )
    {

    // File -----------------------------------------------
    menu_struct sub_menu_file[3];
    if ( exit_button_on_ == 1 )
        sub_menu_file[0].name = "Exit";
    else
        sub_menu_file[0].name = "Load Domain From File";
    sub_menu_file[0].func = &DomainWnd::loadCB;
    sub_menu_file[0].sub_menu = NULL;
    sub_menu_file[0].n_sub_items = 0;
    sub_menu_file[0].sub_menu_title = NULL;

    sub_menu_file[1].name = "Save Domain To File";
    sub_menu_file[1].func = &DomainWnd::saveCB;
    sub_menu_file[1].sub_menu = NULL;
    sub_menu_file[1].n_sub_items = 0;
    sub_menu_file[1].sub_menu_title = NULL;

    sub_menu_file[2].name = "Close";
    sub_menu_file[2].func = &DomainWnd::exitCB;
    sub_menu_file[2].sub_menu = NULL;
    sub_menu_file[2].n_sub_items = 0;
    sub_menu_file[2].sub_menu_title = NULL;

    // Edit -----------------------------------------------
    menu_struct sub_menu_edit[2];

    sub_menu_edit[0].name = "Select All";
    sub_menu_edit[0].func = &DomainWnd::setAllCB;
    sub_menu_edit[0].sub_menu = NULL;
    sub_menu_edit[0].n_sub_items = 0;
    sub_menu_edit[0].sub_menu_title = NULL;

    sub_menu_edit[1].name = "Select None";
    sub_menu_edit[1].func = &DomainWnd::clearAllCB;
    sub_menu_edit[1].sub_menu = NULL;
    sub_menu_edit[1].n_sub_items = 0;
    sub_menu_edit[1].sub_menu_title = NULL;

    // Menu bar --------------------------------------------
    menu_struct PulldownData[2];

    PulldownData[0].name = "File";
    PulldownData[0].func = NULL;
    PulldownData[0].sub_menu = sub_menu_file;
    PulldownData[0].n_sub_items = 3;
    PulldownData[0].sub_menu_title = NULL;

    PulldownData[1].name = "Edit";
    PulldownData[1].func = NULL;
    PulldownData[1].sub_menu = sub_menu_edit;
    PulldownData[1].n_sub_items = 2;
    PulldownData[1].sub_menu_title = NULL;

    createMenuBar ( parent, ( XtPointer ) this );
    createMenuButtons ( NULL, menuBar_, PulldownData, 2 );


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

    XtAddCallback ( canvas_,
                    XmNexposeCallback,
                    &DomainWnd::redisplayCB,
                    ( XtPointer ) this );

    XtAddCallback ( canvas_,
                    XmNresizeCallback,
                    &DomainWnd::resizeCB,
                    ( XtPointer ) this );
    }



void DomainWnd::drawDetail()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "In drawDetail\n" );
#endif

    setForeground ( "black" );

    drawSetup ( canvas_, pix_, gc_, &s );
    drawDomain();
    drawMap();
    drawFrame();
    drawTitles (  vis_->title1_, vis_->title2_, vis_->title3_, vis_->xtitle_, vis_->ytitle_ );
    // SRT 950707 drawYtics(1 /* 3 SRT 950707 */, "%2.0f");
    // SRT 950707 drawXtics(1 /* 3 SRT 950707 */, "%2.0f");

    setForeground ( "black" );
    }



///////////////////////////////////////////////////////////////////////

void DomainWnd::drawDomain()
    {
    int  i, j, index;

    int width  = ( int ) s.scalex ( 2 ) - ( int ) s.scalex ( 1 ) + 1;
    int height = ( int ) s.scaley ( 1 ) - ( int ) s.scaley ( 2 ) + 1;

    setForeground ( "cyan" );

    for ( j=0; j < vis_->row_max_; j++ )
        for ( i=0; i < vis_->col_max_; i++ )
            {

            index = j*vis_->col_max_ + i;

            if ( domain_mask_[index] )
                {
                XFillRectangle ( XtDisplay ( canvas_ ), pix_, gc_,
                                 s.scalex ( i ),
                                 s.scaley ( j+1 ),
                                 width, height );
                }
            }

    setForeground ( "white" );
    for ( j=0; j < vis_->row_max_; j++ )
        for ( i=0; i < vis_->col_max_; i++ )
            {

            index = j*vis_->col_max_ + i;

            if ( domain_mask_[index] == 0 )
                {
                XFillRectangle ( XtDisplay ( canvas_ ), pix_, gc_,
                                 s.scalex ( i ),
                                 s.scaley ( j+1 ),
                                 width, height );
                }
            }

    setForeground ( "black" );

    for ( i=0; i < vis_->row_max_; i++ )
        XDrawLine ( XtDisplay ( canvas_ ),
                    pix_,
                    gc_,
                    s.scalex ( s.xmin_ ),
                    s.scaley ( i ),
                    s.scalex ( s.xmax_ ),
                    s.scaley ( i ) );

    for ( j=0; j < vis_->col_max_; j++ )
        {
        XDrawLine ( XtDisplay ( canvas_ ),
                    pix_,
                    gc_,
                    s.scalex ( j ),
                    s.scaley ( s.ymin_ ),
                    s.scalex ( j ),
                    s.scaley ( s.ymax_ ) );
        }
    }



void DomainWnd::drawMap()
    {
    XPoint points[2500];
    int i, j, k, npoints;

    setForeground ( "black" );
    XSetLineAttributes ( dpy_, gc_,
                         2,
                         LineSolid, ( int ) NULL, ( int ) NULL );

    npoints = 0;

    if ( vis_->map_npolyline_ )
        for ( i = 0; i < vis_->map_npolyline_; i++ )
            {
            k = 0;
            for ( j = 0; j < vis_->map_n_[i]; j++ )
                {
                points[k].x = s.scalex ( vis_->map_x_[npoints] );
                points[k].y = s.scaley ( vis_->map_y_[npoints] );
                if ( k < 2499 )
                    ++k;
                ++npoints;
                }
            XDrawLines ( dpy_, pix_, gc_, points,
                         vis_->map_n_[i], CoordModeOrigin );
            }

    XSetLineAttributes ( dpy_, gc_,
                         0,
                         LineSolid, ( int ) NULL, ( int ) NULL );
    }


/////////////////////////////////////////////////////////////////////////////////

void DomainWnd::redisplayCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    DomainWnd *obj = ( DomainWnd * ) clientData;
    XmAnyCallbackStruct *cbs = ( XmAnyCallbackStruct * ) callData;
    XExposeEvent *event = ( XExposeEvent * ) cbs->event;

    obj->redisplay ( event );
    }


void DomainWnd::redisplay ( XExposeEvent *event )
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


void DomainWnd::resizeCB ( Widget, XtPointer clientData, XtPointer )
    {
    DomainWnd *obj = ( DomainWnd * ) clientData;
    obj->resize();
    }


void DomainWnd::resize()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "In resize\n" );
#endif

    if ( !XtWindow ( canvas_ ) ) return; // 961001 SRT prevents XCreatePixMap below from crashing PAVE


    // Get the size of the drawing area
    XtVaGetValues ( canvas_,
                    XmNwidth, &width_,
                    XmNheight, &height_,
                    NULL );


#ifdef DIAGNOSTICS
    fprintf ( stderr, "In resize before scaleInit xmin=%g ymin=%g xmax=%g ymax=%g\n",
              floor ( zoom_[curr_zoom_].GRID_X_MIN_ ), floor ( zoom_[curr_zoom_].GRID_Y_MIN_ ),
              floor ( zoom_[curr_zoom_].GRID_X_MAX_ ), floor ( zoom_[curr_zoom_].GRID_Y_MAX_ )
            );
#endif

    s.scaleInit ( floor ( zoom_[curr_zoom_].GRID_X_MIN_ ), floor ( zoom_[curr_zoom_].GRID_Y_MIN_ ),
                  floor ( zoom_[curr_zoom_].GRID_X_MAX_ ), floor ( zoom_[curr_zoom_].GRID_Y_MAX_ ),
                  width_, height_ );


    if ( pix_ ) XFreePixmap ( XtDisplay ( canvas_ ), pix_ );

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



void DomainWnd::exitCB ( Widget, XtPointer clientData, XtPointer )
    {
    DomainWnd *obj = ( DomainWnd * ) clientData;
    obj->exit_cb();
    }

void DomainWnd::exit_cb()
    {
    if ( exit_button_on_ == 1 )
        exit ( 1 );
    else
        unmanage();
    }


void DomainWnd::setAllCB ( Widget, XtPointer clientData, XtPointer )
    {
    DomainWnd *obj = ( DomainWnd * ) clientData;
    obj->setAll_cb();
    }

void DomainWnd::setAll_cb()
    {
    int i, j;

    for ( j=0; j < vis_->row_max_; j++ )
        for ( i=0; i < vis_->col_max_; i++ )
            {
            domain_mask_[j*vis_->col_max_+i] = 100;
            if ( percents_ )
                percents_[INDEX ( i,j,0,0,vis_->col_max_,vis_->row_max_,1 )] = 100;
            }
    invalidateFormulasDependingOnMe();
    resize();
    }


void DomainWnd::clearAllCB ( Widget, XtPointer clientData, XtPointer )
    {
    DomainWnd *obj = ( DomainWnd * ) clientData;
    obj->clearAll_cb();
    }

void DomainWnd::clearAll_cb()
    {
    int i, j;

    for ( j=0; j < vis_->row_max_; j++ )
        for ( i=0; i < vis_->col_max_; i++ )
            {
            domain_mask_[j*vis_->col_max_+i] = 0;
            if ( percents_ )
                percents_[INDEX ( i,j,0,0,vis_->col_max_,vis_->row_max_,1 )] = 0;
            }
    invalidateFormulasDependingOnMe();
    resize();
    }



void DomainWnd::writeProbeFile ( float x1, float x2, float y1, float y2 )
    {
    int int_x1, int_x2, int_y1, int_y2;
    int valToSet;

    int invalidateFormulasUsingThisDomain = 0;

    int_x1 = ( int ) x1;
    int_x2 = ( int ) x2;
    int_y1 = ( int ) y1;
    int_y2 = ( int ) y2;

    int width  = ( int ) s.scalex ( 2 ) - ( int ) s.scalex ( 1 ) + 1;
    int height = ( int ) s.scaley ( 1 ) - ( int ) s.scaley ( 2 ) + 1;

    int i, j;
    long index;

    if ( ( int_x1 <= iCellDown_ ) &&
         ( iCellDown_ <= int_x2 ) &&
         ( int_y2 <= jCellDown_ ) &&
         ( jCellDown_ <= int_y1 ) )
        valToSet = ( domain_mask_[INDEX ( iCellDown_, jCellDown_,
                                          0,0,vis_->col_max_,vis_->row_max_,1 )]>0 ) ? 0 : 100;
    else
        valToSet = ( domain_mask_[INDEX ( int_x1, int_y1,
                                          0,0,vis_->col_max_,vis_->row_max_,1 )]>0 ) ? 0 : 100;

    for ( j=0; j < vis_->row_max_; j++ )
        for ( i=0; i < vis_->col_max_; i++ )
            {
            index = INDEX ( i,j,0,0,vis_->col_max_,vis_->row_max_,1 ); // SRT j*vis_->col_max_ + i;
            if ( ( int_x1 <= i ) && ( i <= int_x2 ) && ( int_y2 <= j ) && ( j <= int_y1 ) )
                {
                domain_mask_[index] = valToSet;
                if ( percents_ )
                    {
                    invalidateFormulasUsingThisDomain = 1;
                    percents_[index] = valToSet;
                    }
                }
            }

    if ( invalidateFormulasUsingThisDomain )
        invalidateFormulasDependingOnMe();

    resize();
    }


void DomainWnd::writeProbeObsFile ( int x1, int x2, int y1, int y2 )
    {
    }

void DomainWnd::overlay_ts ( int x1, int x2, int y1, int y2 )
    {
    }

int DomainWnd::invalidateFormulasDependingOnMe ( void )
    {
    if ( percents_ )
        if ( formulaList_ )
            {
            Formula *formula = ( Formula * ) formulaList_->head();
            while ( formula )
                {
                if ( formula->domainWasChanged ( vis_->col_max_, vis_->row_max_, mapinfo_ ) )
                    fprintf ( stderr, "formula->domainWasChanged() error in DomainWnd::writeProbeFile()\n" );

                formula = ( Formula * ) formulaList_->next();
                }
            }

    return 0;
    }



void DomainWnd::loadCB ( Widget, XtPointer clientData, XtPointer )
    {
    DomainWnd *obj = ( DomainWnd * ) clientData;
    obj->DomainBrowser_.postLoadSelectionDialog();
    }


void DomainWnd::saveCB ( Widget, XtPointer clientData, XtPointer )
    {
    DomainWnd *obj = ( DomainWnd * ) clientData;
    obj->DomainBrowser_.postSaveSelectionDialog();
    }


void DomainWnd::saveDOMAINCB ( void *object, char *fname )
    {
    char estring[256];
    DomainWnd *obj = ( DomainWnd * ) object;
    if ( obj->saveDomain ( fname, estring ) )
        fprintf ( stderr, "\007%s\n", estring );
    }


void DomainWnd::loadDOMAINCB ( void *object, char *fname )
    {
    char estring[256];
    DomainWnd *obj = ( DomainWnd * ) object;
    if ( obj->loadDomain ( fname, estring ) )
        fprintf ( stderr, "\007%s\n", estring );
    }


int DomainWnd::saveDomain ( char *fname, char *estring )
    {
    int IMAX = vis_->col_max_,
        JMAX = vis_->row_max_;
    FILE    *fp;
    int     len = strlen ( mapinfo_ )+1,
            arrsize = IMAX*JMAX;

    if ( !estring )
        {
        fprintf ( stderr, "No estring supplied to DomainWnd::saveDomain()!\n" );
        return 1;
        }

    if ( !fname )
        {
        sprintf ( estring, "No fname supplied to DomainWnd::saveDomain()!" );
        return 1;
        }

    if ( !percents_ )
        {
        sprintf ( estring, "No percents_[] in DomainWnd::saveDomain()!" );
        return 1;
        }

    if ( ! ( fp=fopen ( fname, "w" ) ) )
        {
        sprintf ( estring, "Couldn't open '%s' in DomainWnd::saveDomain()!", fname );
        return 1;
        }

    if ( ( fwrite ( "PAVEDOMAIN", strlen ( "PAVEDOMAIN" ), 1, fp ) != 1 ) ||
            ( fwrite ( &IMAX, sizeof ( int ), 1, fp ) != 1 ) ||
            ( fwrite ( &JMAX, sizeof ( int ), 1, fp ) != 1 ) ||
            ( fwrite ( &len, sizeof ( int ), 1, fp ) != 1 ) ||
            ( fwrite ( mapinfo_, len, 1, fp ) != 1 ) ||
            ( fwrite ( percents_, arrsize, 1, fp ) != 1 ) )
        {
        fclose ( fp );
        sprintf ( estring, "Couldn't write data in DomainWnd::saveDomain()!" );
        return 1;
        }

    fclose ( fp );
    return 0;
    }



int DomainWnd::loadDomain ( char *fname, char *estring )
    {
    FILE    *fp;
    int IMAX = vis_->col_max_,
        JMAX = vis_->row_max_,
        len, arrsize = IMAX*JMAX,
             i, j, dindex, pindex;
    char    tstring[] = "PAVEDOMAIN", *tmapinfo, testring[128];


    if ( !estring )
        {
        fprintf ( stderr, "No estring supplied to DomainWnd::loadDomain()!\n" );
        return 1;
        }

    if ( !fname )
        {
        sprintf ( estring, "No fname supplied to DomainWnd::loadDomain()!" );
        return 1;
        }

    if ( !percents_ )
        {
        sprintf ( estring, "No percents_[] in DomainWnd::loadDomain()!" );
        return 1;
        }

    if ( ! ( fp=fopen ( fname, "r" ) ) )
        {
        sprintf ( estring, "Couldn't open '%s' in DomainWnd::loadDomain()!", fname );
        return 1;
        }

    if ( fread ( tstring, strlen ( "PAVEDOMAIN" ), 1, fp ) != 1 )
        {
        sprintf ( estring, "Couldn't read magic data in DomainWnd::loadDomain()!" );
        fclose ( fp );
        return 1;
        }

    if ( strcmp ( tstring, "PAVEDOMAIN" ) )
        {
        sprintf ( estring,
                  "Magic chars don't match - '%s' is not a valid PAVE domain file!",
                  fname );
        fclose ( fp );
        return 1;
        }

    if ( ( fread ( &IMAX, sizeof ( int ), 1, fp ) != 1 ) ||
            ( fread ( &JMAX, sizeof ( int ), 1, fp ) != 1 ) ||
            ( fread ( &len, sizeof ( int ), 1, fp ) != 1 ) )
        {
        sprintf ( estring, "Couldn't read size info from '%s'!", fname );
        fclose ( fp );
        return 1;
        }

    if ( ( vis_->col_max_ != IMAX ) || ( vis_->row_max_ != JMAX ) )
        {
        sprintf ( estring,
                  "(IMAX==%d,JMAX==%d) doesn't match this domain's (%d,%d)!!\n",
                  IMAX, JMAX, vis_->col_max_, vis_->row_max_ );
        fclose ( fp );
        return 1;
        }

    if ( ( tmapinfo = ( char * ) malloc ( len ) ) == NULL )
        {
        sprintf ( estring, "Couldn't allocate space in DomainWnd::loadDomain()!" );
        fclose ( fp );
        return 1;
        }

    if ( fread ( tmapinfo, len, 1, fp ) != 1 )
        {
        sprintf ( estring, "Couldn't read mapinfo in DomainWnd::loadDomain()!" );
        free ( tmapinfo );
        tmapinfo = NULL;
        fclose ( fp );
        return 1;
        }

    if ( !map_infos_areReasonablyEquivalent ( tmapinfo, mapinfo_, testring ) )
        {
        sprintf ( estring, "%s's mapInfo == '%s' doesn't match this domain!!\n%s",
                  fname, mapinfo_, testring );
        free ( tmapinfo );
        tmapinfo = NULL;
        fclose ( fp );
        return 1;
        }

    free ( tmapinfo );
    tmapinfo = NULL;


    if ( fread ( percents_, arrsize, 1, fp ) != 1 )
        {
        sprintf ( estring, "Couldn't read array in DomainWnd::loadDomain()!" );
        fclose ( fp );
        return 1;
        }

    // set the values of domain_mask_[]
    for ( j=0; j < vis_->row_max_; j++ )
        for ( i=0; i < vis_->col_max_; i++ )
            {
            dindex = j*vis_->col_max_ + i;
            pindex = ( int ) INDEX ( i,j,0,0,vis_->col_max_,vis_->row_max_,1 );
            domain_mask_[dindex] = percents_[pindex];
            }

    resize();
    invalidateFormulasDependingOnMe();
    fclose ( fp );
    return 0;
    }
