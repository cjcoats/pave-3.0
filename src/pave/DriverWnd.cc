/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: DriverWnd.cc 83 2018-03-12 19:24:33Z coats $
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
//
// File:    DriverWnd.cc
// Author:  K. Eng Pua
// Date:    Dec 14, 1994
//
//////////////////////////////////////////////////////////////////////////////
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950516  Added code to process HourMin and HourMax bars from option box
// SRT  950525  Changed around menu titles & other little UI things
// SRT  950529  Added DriverWnd::processArgs() method; added int argc,
//              char *argv[], and char *errString args to DriverWnd
// SRT  950530  added graphics_plot_type_menu_button_ and
//              domain_edit_button_ // SRT
// SRT  950604  added Export menu and associated callback routines
// SRT  950612  Added editDataSetLayerRangeCB(), edit_data_set_layer_range_cb(),
//              editDataSetROICB(), edit_data_set_roi_cb(), editFormulaLayerRangeCB(),
//              edit_formula_layer_range_cb(), editFormulaROICB(), and editFormulaROICB()
//      calls, and changed around the menus a bit
// SRT  951218  Added logic for minimum time between animation frames
// SRT  951227  Added logic to handle tile/mesh plots of YZTSLICE and XZTSLICE planes
// SRT  960411  Added code to set Dialog Box Positions
// SRT  960517  Added hooks to netCDF exporting
// SRT  960826  Added documentation menu
// CJC  2018058 Version for PAVE-3.0
///////////////////////////////////////////////////////////////////////////////

/* check if character is white space */
#define whitespace(c)    (c == ' ' || c == '\t')

/* check if character is string quoting characters */
#define isquote(c)       (c == '"' || c == '\'')

/* get last character of string */
#define lastch(str)      str[strlen(str)-1]


#include <signal.h>
#include <unistd.h>
#include "nan_incl.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/signal.h>

#ifndef linux
#include <libgen.h>
#endif

#include "DriverWnd.h"
#include "iodecl3.h"

extern void pave_version  ( void );
extern void pave_log_stop ( void );
extern Formula *getSelectedFormula ( FormulaServer *fsp );

extern "C" {
    Window getMostRecentGraphWindow ( void );
    }
extern "C" {
    int evalTokens ( char *, char **, int * );
    }

static void killmykids ( void )
    {
    pid_t pgrp = getpgrp();
    killpg ( ( int ) pgrp, SIGKILL );
    }

DriverWnd::DriverWnd
( AppInit *app, char *name, char *historyfile,
  int argc, char *argv[], char *errorMsg ) :
    Shell ( app, name ),
    Menus(),
    BusConnect ( app )
    {
    Position xpos, ypos;
    char estring[256];
    int ans;

    num_to_loop_ = 0;
    startingUpThisPuppy_ = 1;
    work_proc_id_ = ( XtWorkProcId ) NULL;
    app_ = app;
    synchronize_dialog_ = NULL;
    mostRecentTile_ = NULL;
    minFrameTime_dialog_ = NULL;
    curr_animate_ = 0;
    animate_frame_ = 0;
    width_ = 0;
    height_ = 0;
    history_file_ = strdup ( historyfile );
    assert ( history_file_ );
    name_ = strdup ( name );
    assert ( name_ );
    num_tilewnd_ = 0;
    tilewnd_list_ = NULL;
    max_num_hours_ = 0;
    tileSliceType_ = XYTSLICE;
    obsIdListP_ = NULL;
    int i, nodisplay = 0;

#ifndef USE_OLDMAP
    if ( init_projmaps ( estring ) )
        fprintf ( stderr, "%s\n", estring );
#endif // USE_OLDMAP

    for ( i = 0; i < argc; i++ )
        if ( !strcasecmp ( argv[i], "-nodisplay" ) )
            {
            nodisplay = 1;
#ifdef DIAGNOSTICS
            fprintf ( stderr,
                      "-nodisplay argument!\n" );
#endif // DIAGNOSTICS
            }

    frameDelayInTenthsOfSeconds_ =
        ( getenv ( "TENTHS_SECS_BETWEEN_FRAMES" ) != NULL ) ?
        ( atoi ( getenv ( "TENTHS_SECS_BETWEEN_FRAMES" ) ) ) :0;
    if ( frameDelayInTenthsOfSeconds_ < 0 )
        {
        fprintf ( stderr, "Using TENTHS_SECS_BETWEEN_FRAMES of 0 rather than\n"
                  "%d since the range needs to be from 0..50\n",
                  frameDelayInTenthsOfSeconds_ );
        frameDelayInTenthsOfSeconds_ = 0;
        }
    if ( frameDelayInTenthsOfSeconds_ > 50 )
        {
        fprintf ( stderr, "Using TENTHS_SECS_BETWEEN_FRAMES of 50 rather than\n"
                  "%d since the range needs to be from 0..50\n",
                  frameDelayInTenthsOfSeconds_ );
        frameDelayInTenthsOfSeconds_ = 50;
        }

    if ( !isConnectedToBus() )
        {
        if ( getenv ( "BUS_MASTER_EXE" ) != NULL )
            {
            if ( strlen ( getenv ( "BUS_MASTER_EXE" ) ) > 0 )
                {
                fprintf ( stderr,
                          "Did not connect to bus - only local files are available.  If remote\n"
                          "file access is desired, try the following command and then restart PAVE:\n"
                          "\trm /tmp/sbus*%s\n\n", get_user_name() );
                //exit(1);
                }
            }
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "HISTORYFILE=%s\n", history_file_ );
#endif

    _w = XtVaCreateManagedWidget ( "draw_window",
                                   xmFormWidgetClass,      shell_,
                                   XmNmappedWhenManaged,       nodisplay ? False : True, // SRT 960212
                                   XmNscrollBarDisplayPolicy,  XmAS_NEEDED,
                                   XmNscrollingPolicy,     XmAUTOMATIC,
                                   XmNheight,          320,
                                   XmNwidth,           600,
                                   NULL );

    installDestroyHandler();

    createUI ( _w );
    option_  = new OptionManager ( _w );

    manage();

    // get position and dims of DriverWnd
    XtVaGetValues ( _w,
                    XmNx, &xpos,
                    XmNy, &ypos,
                    NULL ); // SRT 960411

    species_ = new SpeciesServer ( name_, info_window_, "Species" );

    case_    = ( void * ) new CaseServer
               (
                   name_,
                   info_window_,
                   "Dataset",
                   "<CASES>",
                   &datasetList_,
                   &formulaList_,
                   &domainList_,
                   &levelList_,
                   species_,
                   bd_,
                   app_
               );

    formula_ = new FormulaServer ( name_, info_window_, "Formula", "<FORMULAS>",
                                   ( ( CaseServer * ) case_ )->getSelectionDialog(),
                                   BusConnect::getBusData(),
                                   &datasetList_,
                                   &formulaList_,
                                   &domainList_,
                                   &levelList_ );

    ( ( FormulaServer * ) formula_ )->setDriverWnd ( ( void * ) this ); // SRT 96042510v:wq

    ( ( CaseServer * ) case_ )->setFormulaServer ( formula_ ); // SRT 950905
    ( ( CaseServer * ) case_ )->setDriverWnd ( ( void * ) this ); // SRT 960410
    species_->setFormulaServer ( formula_ ); // SRT 960410

    work_proc_id_ = ( XtWorkProcId ) NULL;

    // Initialize re-usable info structure
    memset ( ( void * ) &info, 0, ( size_t ) sizeof ( VIS_DATA ) );

    readHistoryFile();

    dom_obj_ = NULL;

    // Might want to move to new class later
    // SRT 950804 bus_hostname_ = NULL;
    // SRT 950804 bus_filename_ = NULL;
    // SRT 950804 bus_formula_ = NULL;
    // SRT 950804 bus_graphics_type_ = NULL;

    ans = processArgs ( argc, argv, errorMsg ); // added SRT
    if ( ( getenv ( "KEDAMODE" ) != NULL ) && ( !strcmp ( getenv ( "KEDAMODE" ), "1" ) ) )
        {
        if ( ans )
            fprintf ( stderr, "KEDA_FAILURE: %s\n", errorMsg );
        else
            {
            fprintf ( stdout, "KEDA_SUCCESS!\n" );
            fflush ( stdout );
            }
        }

    exportAVS_UI_ = new ExportServer ( PAVE_EXPORT_AVS, "AVS", info_window_, "AVS" );
    exportTabbed_UI_ = new ExportServer ( PAVE_EXPORT_TABBED, "ASCII", info_window_, "ASCII" );

    exportnetCDF_UI_ = new ExportServer ( PAVE_EXPORT_NETCDF, "netCDF", info_window_, "netCDF" );

    assert ( exportAVS_UI_ && exportTabbed_UI_ && exportnetCDF_UI_ );

    species_->setScreenPosition ( xpos, ypos+360 ); // SRT 960411
    ( ( CaseServer * ) case_ )->setScreenPosition ( xpos+185, ypos+360 ); // SRT 960411
    formula_->setScreenPosition ( xpos+615, ypos ); // SRT 960411
    modify_formula_cb();         // show Edit/Select Formula box SRT 960411
    modify_case_cb();        // show Edit/Select Dataset box SRT 960411
    updateStatus ( "" );     // blank out message window SRT 960411

    startingUpThisPuppy_ = 0;
    }


DriverWnd::~DriverWnd()
    {
    char estring[256];

    if ( exportAVS_UI_ )
        {
        delete exportAVS_UI_;
        exportAVS_UI_ = NULL;
        }
    if ( exportTabbed_UI_ )
        {
        delete exportTabbed_UI_;
        exportTabbed_UI_ = NULL;
        }
    if ( history_file_ )
        {
        delete history_file_;
        history_file_ = NULL;
        }
    if ( name_ )
        {
        delete name_;
        name_ = NULL;
        }

#ifndef USE_OLDMAP
    if ( cleanup_projmaps ( estring ) )
        fprintf ( stderr, "%s\n", estring );
#endif // USE_OLDMAP
    }



int DriverWnd::createTileVectorPlot (    char *formula1, // for tiles
        // if formula1 == NULL then vectors only will be drawn
        char *formula2, // u component
        char *formula3 ) // v component
    {
    Formula     *f[3];
    VIS_DATA    *vdata[3];
    int         i, j;
    char        statusMsg[512], *fn[3];

    // info from map_info for UAM and netCDF data formats
    float       llx[3], lly[3], urx[3], ury[3], grid_type[3],
                xorig[3], yorig[3], xcell[3], ycell[3], xcent[3],
                ycent[3], p_gam[3], p_bet[3], p_alp[3];
    int     utm_zone[3], ncol[3], nrow[3];


#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter createTileVectorPlot('%s','%s','%s')\n",
              formula1, formula2, formula3 );
#endif // DIAGNOSTICS

    assert ( formula2 &&
             formula3 &&
             formula2[0] &&
             formula3[0] );
    f[0] = f[1] = f[2] = ( Formula * ) NULL;
    vdata[0] = vdata[1] = vdata[2] = ( VIS_DATA * ) NULL;
    fn[0] = formula1;
    fn[1] = formula2;
    fn[2] = formula3;
    statusMsg[0] = '\0';


    // grab copies of the data
    for ( i = 0; i < 3; i++ )
        if ( fn[i] )
            {
            removeWhiteSpace ( fn[i] );

            // Find the currently formula's Formula object
            if ( ! ( f[i] = ( Formula * ) ( formulaList_.find ( fn[i] ) ) ) )
                {
                sprintf ( statusMsg, "Didn't find '%s'\non the formulaList!\n", fn[i] );
                if ( vdata[0] ) free_vis ( vdata[0] );
                vdata[0] = ( VIS_DATA * ) NULL;
                if ( vdata[1] ) free_vis ( vdata[1] );
                vdata[1] = ( VIS_DATA * ) NULL;
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                return 1;
                }

            // Retrieve a copy of that Formula object's data
            if ( ! ( vdata[i] = get_VIS_DATA_struct ( f[i],statusMsg,tileSliceType_ ) ) )
                {
                if ( vdata[0] ) free_vis ( vdata[0] );
                vdata[0] = ( VIS_DATA * ) NULL;
                if ( vdata[1] ) free_vis ( vdata[1] );
                vdata[1] = ( VIS_DATA * ) NULL;
                if ( strstr ( statusMsg, "==" ) )
                    displaySingleNumberFormula ( statusMsg, f[i] );
                else
                    {
                    Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                    return 1;
                    }
                return 0;
                }

            if ( tileSliceType_ != XYTSLICE )
                if ( permute_vdata_to_XYT ( vdata[i], statusMsg ) )
                    {
                    if ( vdata[0] ) free_vis ( vdata[0] );
                    vdata[0] = ( VIS_DATA * ) NULL;
                    if ( vdata[1] ) free_vis ( vdata[1] );
                    vdata[1] = ( VIS_DATA * ) NULL;
                    Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                    return 1;
                    }

            if ( ( !fn[0] ) && ( i == 1 ) )
                {
                if ( ! ( vdata[0] = VIS_DATA_dup ( vdata[1], statusMsg ) ) )
                    {
                    if ( vdata[0] ) free_vis ( vdata[0] );
                    vdata[0] = ( VIS_DATA * ) NULL;
                    if ( vdata[1] ) free_vis ( vdata[1] );
                    vdata[1] = ( VIS_DATA * ) NULL;
                    Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                    return 1;
                    }
                else
                    {
                    // keep all but the actual grid itself,
                    // as this will be needed for labeling purposes
                    // but the data won't be
                    free ( vdata[0]->grid );
                    vdata[0]->grid = NULL;
                    }
                }
            }


    // verify dataset types's match
    if     ( ( vdata[0]->dataset != vdata[1]->dataset ) ||
             ( vdata[0]->dataset != vdata[2]->dataset ) )
        {
        for ( i = 0; i < 3; i++ )
            {
            free_vis ( vdata[i] );
            vdata[i] = ( VIS_DATA * ) NULL;
            }
        Message error ( info_window_, XmDIALOG_ERROR,
                        "Dataset types don't match!" );
        return 1;
        }


    // grab the map_info contents
    for ( i=0; i < 3; i++ )
        {
        switch ( vdata[i]->dataset )
            {

            case UAM_DATA:
            case UAMV_DATA:
                sscanf ( vdata[i]->map_info, "%g%g%g%g%d%d%d",
                         &llx[i], &lly[i], &urx[i], &ury[i], &utm_zone[i],
                         &ncol[i], &nrow[i] );
                break;

            case netCDF_DATA:
                sscanf ( vdata[i]->map_info, "%g%g%g%g%g%g%g%g%g%g%d%d",
                         &grid_type[i], &xorig[i], &yorig[i], &xcell[i],
                         &ycell[i], &xcent[i], &ycent[i], &p_gam[i],
                         &p_bet[i], &p_alp[i], &ncol[i], &nrow[i] );
                break;

            default:
                sprintf ( statusMsg, "Unknown dataset type %d!\n",
                          vdata[i]->dataset );
                    {
                    Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                    }
                for ( j = 0; j < 3; j++ )
                    {
                    free_vis ( vdata[i] );
                    vdata[i] = ( VIS_DATA * ) NULL;
                    }
                return 1;
            }
        }


    // verify that appropriate map_info fields match
    if (
        (
            ( ( vdata[0]->dataset == UAM_DATA ) || ( vdata[0]->dataset == UAMV_DATA ) )
            &&
            (
                ( utm_zone[0] != utm_zone[1] )
                ||
                ( utm_zone[1] != utm_zone[2] )
            )
        )
        ||
        (
            ( vdata[0]->dataset == netCDF_DATA )
            &&
            (
                ( p_alp[0] != p_alp[1] )
                ||
                ( p_bet[0] != p_bet[1] )
                ||
                ( p_gam[0] != p_gam[1] )
                ||
                ( xcent[0] != xcent[1] )
                ||
                ( ycent[0] != ycent[1] )
                ||
                ( p_alp[1] != p_alp[2] )
                ||
                ( p_bet[1] != p_bet[2] )
                ||
                ( p_gam[1] != p_gam[2] )
                ||
                ( xcent[1] != xcent[2] )
                ||
                ( ycent[1] != ycent[2] )
            )
        )
    )

        // both vector components' map_info strings must match
        // exactly in their entirety
        if ( strcasecmp ( vdata[1]->map_info, vdata[2]->map_info ) )
            {
            for ( j = 0; j < 3; j++ )
                {
                free_vis ( vdata[i] );
                vdata[i] = ( VIS_DATA * ) NULL;
                }
            Message error ( info_window_, XmDIALOG_ERROR,
                            "Vector component variables have different map_info strings!" );
            return 1;
            }


    // verify grids match appropriately
    if (
        ( vdata[0]->level_max-vdata[0]->level_min !=
          vdata[1]->level_max-vdata[1]->level_min ) ||
        ( vdata[1]->level_max != vdata[2]->level_max ) ||
        ( vdata[1]->level_min != vdata[2]->level_min ) ||
        ( vdata[1]->ncol    != vdata[2]->ncol ) ||
        ( vdata[1]->nrow    != vdata[2]->nrow ) ||
        ( vdata[1]->col_min != vdata[2]->col_min ) ||
        ( vdata[1]->col_max != vdata[2]->col_max ) ||
        ( vdata[1]->row_min != vdata[2]->row_min ) ||
        ( vdata[0]->step_max-vdata[0]->step_min !=
          vdata[1]->step_max-vdata[1]->step_min ) ||
        ( vdata[1]->step_max-vdata[1]->step_min !=
          vdata[2]->step_max-vdata[2]->step_min )
    )
        {
        Message error ( info_window_, XmDIALOG_ERROR, "Grids don't match!" );
        }
    else
        {
        int ndts = vdata[2]->step_max-vdata[2]->step_min+1;
        if ( memcmp ( vdata[0]->sdate, vdata[1]->sdate,
                      ( size_t ) ndts*sizeof ( int ) )
                ||
                memcmp ( vdata[0]->stime, vdata[1]->stime,
                         ( size_t ) ndts*sizeof ( int ) )
                ||
                memcmp ( vdata[1]->sdate, vdata[2]->sdate,
                         ( size_t ) ndts*sizeof ( int ) )
                ||
                memcmp ( vdata[1]->stime, vdata[2]->stime,
                         ( size_t ) ndts*sizeof ( int ) ) )
            {
            fprintf ( stderr, "Time steps don't match;"
                      " setting them to all 0's\n" );
            memset ( ( void * ) vdata[0]->stime, ( int ) NULL,
                     ( size_t ) ndts*sizeof ( int ) );
            memset ( ( void * ) vdata[0]->sdate, ( int ) NULL,
                     ( size_t ) ndts*sizeof ( int ) );
            }

        // Create the tile plot - CHECK whether tile window frees vdata when done !! - SRT
        int h, w;
        char *caseString;

        sprintf ( statusMsg, "Displaying vector plot...\n" );
        updateStatus ( statusMsg );
        // SRT 961009 sprintf(strbuf_,"%s TilePlot", _name);
        if ( inputTitleString_[0] )
            {
            strcpy ( str512_, inputTitleString_ );
            inputTitleString_[0] = '\0';
            }
        else
            {
            switch ( tileSliceType_ )
                {
                case XYSLICE:
                case XYTSLICE:
                    sprintf ( str512_, "Layer %d  %s",
                              f[0] ?
                              f[0]->get_selected_level() :
                              f[1]->get_selected_level(),
                              f[0] ?
                              formula1:
                              "Vector Plot" );
                    break;

                case XZSLICE:
                case XZTSLICE:
                    sprintf ( str512_, "Row %d  %s",
                              f[0] ?
                              f[0]->get_selected_row() :
                              f[1]->get_selected_row(),
                              f[0] ?
                              formula1:
                              "Vector Plot" );
                    break;

                case YZSLICE:
                case YZTSLICE:
                    sprintf ( str512_, "Column %d  %s",
                              f[0] ?
                              f[0]->get_selected_column() :
                              f[1]->get_selected_column(),
                              f[0] ?
                              formula1:
                              "Vector Plot" );
                    break;

                default:
                    for ( j = 0; j < 3; j++ )
                        {
                        free_vis ( vdata[i] );
                        vdata[i] = ( VIS_DATA * ) NULL;
                        }
                    sprintf ( statusMsg,
                              "Bad slice type of %d in "
                              "DriverWnd::createTileVectorPlot()!",
                              tileSliceType_ );
                    Message error ( info_window_,
                                    XmDIALOG_ERROR,
                                    statusMsg );
                    return 1;
                }
            }

        sprintf ( strbuf_,"%s", str512_ ); // 961009 SRT
        calcWidthHeight ( &w, &h, vdata[0] );
        if ( f[0] ) caseString = f[0]->getCasesUsedString();
        else caseString = strdup ( " " );
        if ( subTitle1String_[0] )
            {
            if ( vdata[0]->data_label ) free ( vdata[0]->data_label );
            vdata[0]->data_label = strdup ( subTitle1String_ );
            }
        TileWnd *tile  = new TileWnd ( &cfg_, ( void * ) this, app_, strbuf_,
                                       bd_,
                                       vdata[0],
                                       vdata[1],
                                       vdata[2],
                                       "TILE",
                                       str512_,    // titlestring
                                       subTitle2String_[0] ? subTitle2String_ : caseString,

                                       ( Dimension ) w,
                                       ( Dimension ) h,
                                       &frameDelayInTenthsOfSeconds_,
                                       0 );
        free ( caseString );
        caseString = NULL;
        subTitle1String_[0] = '\0';
        subTitle2String_[0] = '\0';

        if ( contourRange_ )
            tile->setContourRange ( minCut_, maxCut_ );

        // Register the tile plot with the synchronize dialog box
        registerSynWnd ( tile );
        }
    return 0;
    }

int DriverWnd::createTileVectorObsPlot ( char *formula1, // u component
        char *formula2, // v component
        char *formula3, // u_obs component
        char *formula4  // v_obs component
                                       )
    {
    Formula     *f[4];
    VIS_DATA    *vdata[4];
    int         i, j;
    char        statusMsg[512], *fn[4];

    // info from map_info for UAM and netCDF data formats
    float       llx[4], lly[4], urx[4], ury[4], grid_type[4],
                xorig[4], yorig[4], xcell[4], ycell[4], xcent[4],
                ycent[4], p_gam[4], p_bet[4], p_alp[4];
    int     utm_zone[4], ncol[4], nrow[4];


    assert ( formula2 &&
             formula3 &&
             formula2[0] &&
             formula3[0] );
    f[0] = f[1] = f[2] = f[3] = ( Formula * ) NULL;
    vdata[0] = vdata[1] = vdata[2] = vdata[3] = ( VIS_DATA * ) NULL;
    fn[0] = formula1;
    fn[1] = formula2;
    fn[2] = formula3;
    fn[3] = formula4;
    statusMsg[0] = '\0';


    // grab copies of the data
    for ( i = 0; i < 4; i++ )
        if ( fn[i] )
            {
            removeWhiteSpace ( fn[i] );

            // Find the currently formula's Formula object
            if ( ! ( f[i] = ( Formula * ) ( formulaList_.find ( fn[i] ) ) ) )
                {
                sprintf ( statusMsg, "Didn't find '%s'\non the formulaList!\n", fn[i] );
                if ( vdata[0] ) free_vis ( vdata[0] );
                vdata[0] = ( VIS_DATA * ) NULL;
                if ( vdata[1] ) free_vis ( vdata[1] );
                vdata[1] = ( VIS_DATA * ) NULL;
                if ( vdata[2] ) free_vis ( vdata[2] );
                vdata[2] = ( VIS_DATA * ) NULL;
                if ( vdata[3] ) free_vis ( vdata[3] );
                vdata[3] = ( VIS_DATA * ) NULL;
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                return 1;
                }

            // Retrieve a copy of that Formula object's data

            if ( ! ( vdata[i] = get_VIS_DATA_struct ( f[i],statusMsg,tileSliceType_ ) ) )
                {
                if ( vdata[0] ) free_vis ( vdata[0] );
                vdata[0] = ( VIS_DATA * ) NULL;
                if ( vdata[1] ) free_vis ( vdata[1] );
                vdata[1] = ( VIS_DATA * ) NULL;
                if ( vdata[2] ) free_vis ( vdata[2] );
                vdata[2] = ( VIS_DATA * ) NULL;
                if ( vdata[3] ) free_vis ( vdata[3] );
                vdata[3] = ( VIS_DATA * ) NULL;
                if ( strstr ( statusMsg, "==" ) )
                    displaySingleNumberFormula ( statusMsg, f[i] );
                else
                    {
                    Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                    return 1;
                    }
                return 0;
                }

            if ( tileSliceType_ != XYTSLICE )
                if ( permute_vdata_to_XYT ( vdata[i], statusMsg ) )
                    {
                    if ( vdata[0] ) free_vis ( vdata[0] );
                    vdata[0] = ( VIS_DATA * ) NULL;
                    if ( vdata[1] ) free_vis ( vdata[1] );
                    vdata[1] = ( VIS_DATA * ) NULL;
                    if ( vdata[2] ) free_vis ( vdata[2] );
                    vdata[2] = ( VIS_DATA * ) NULL;
                    if ( vdata[3] ) free_vis ( vdata[3] );
                    vdata[3] = ( VIS_DATA * ) NULL;
                    Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                    return 1;
                    }

            }

    // verify dataset types's match
    if ( ( vdata[0]->dataset != vdata[1]->dataset ) ||
            ( vdata[0]->dataset != vdata[2]->dataset ) ||
            ( vdata[0]->dataset != vdata[3]->dataset )
       )
        {
        for ( i = 0; i < 4; i++ )
            {
            free_vis ( vdata[i] );
            vdata[i] = ( VIS_DATA * ) NULL;
            }
        Message error ( info_window_, XmDIALOG_ERROR,
                        "Dataset types don't match!" );
        return 1;
        }


    // grab the map_info contents
    for ( i=0; i < 4; i++ )
        {
        switch ( vdata[i]->dataset )
            {

            case UAM_DATA:
            case UAMV_DATA:
                sscanf ( vdata[i]->map_info, "%g%g%g%g%d%d%d",
                         &llx[i], &lly[i], &urx[i], &ury[i], &utm_zone[i],
                         &ncol[i], &nrow[i] );
                break;

            case netCDF_DATA:
                sscanf ( vdata[i]->map_info, "%g%g%g%g%g%g%g%g%g%g%d%d",
                         &grid_type[i], &xorig[i], &yorig[i], &xcell[i],
                         &ycell[i], &xcent[i], &ycent[i], &p_gam[i],
                         &p_bet[i], &p_alp[i], &ncol[i], &nrow[i] );
                break;

            default:
                sprintf ( statusMsg, "Unknown dataset type %d!\n",
                          vdata[i]->dataset );
                    {
                    Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                    }
                for ( j = 0; j < 4; j++ )
                    {
                    free_vis ( vdata[i] );
                    vdata[i] = ( VIS_DATA * ) NULL;
                    }
                return 1;
            }
        }


    // verify that appropriate map_info fields match
    if (
        (
            ( ( vdata[0]->dataset == UAM_DATA ) || ( vdata[0]->dataset == UAMV_DATA ) )
            &&
            (
                ( utm_zone[0] != utm_zone[1] ) ||
                ( utm_zone[0] != utm_zone[2] ) ||
                ( utm_zone[0] != utm_zone[3] )
            )
        )
        ||
        (
            ( vdata[0]->dataset == netCDF_DATA )
            &&
            (
                ( p_alp[0] != p_alp[1] ) ||
                ( p_alp[0] != p_alp[2] ) ||
                ( p_alp[0] != p_alp[3] ) ||
                ( p_bet[0] != p_bet[1] ) ||
                ( p_gam[0] != p_gam[1] ) ||
                ( p_gam[0] != p_gam[2] ) ||
                ( p_gam[0] != p_gam[3] ) ||
                ( xcent[0] != xcent[1] ) ||
                ( xcent[0] != xcent[2] ) ||
                ( xcent[0] != xcent[3] ) ||
                ( ycent[0] != ycent[1] ) ||
                ( ycent[0] != ycent[2] ) ||
                ( ycent[0] != ycent[3] )
            )
        )
    )

        // both vector components' map_info strings must match
        // exactly in their entirety
        if ( strcasecmp ( vdata[0]->map_info, vdata[1]->map_info ) ||
                strcasecmp ( vdata[0]->map_info, vdata[2]->map_info ) ||
                strcasecmp ( vdata[0]->map_info, vdata[3]->map_info )
           )
            {
            for ( j = 0; j < 4; j++ )
                {
                free_vis ( vdata[i] );
                vdata[i] = ( VIS_DATA * ) NULL;
                }
            Message error ( info_window_, XmDIALOG_ERROR,
                            "Vector component variables have different map_info strings!" );
            return 1;
            }

    //ALT: more checking is needed from here on ...
    // verify grids match appropriately
    if (
        ( vdata[0]->level_max-vdata[0]->level_min !=
          vdata[1]->level_max-vdata[1]->level_min ) ||
        ( vdata[1]->level_max != vdata[2]->level_max ) ||
        ( vdata[1]->level_min != vdata[2]->level_min ) ||
        ( vdata[1]->ncol    != vdata[2]->ncol ) ||
        ( vdata[1]->nrow    != vdata[2]->nrow ) ||
        ( vdata[1]->col_min != vdata[2]->col_min ) ||
        ( vdata[1]->col_max != vdata[2]->col_max ) ||
        ( vdata[1]->row_min != vdata[2]->row_min ) ||
        ( vdata[0]->step_max-vdata[0]->step_min !=
          vdata[1]->step_max-vdata[1]->step_min ) ||
        ( vdata[1]->step_max-vdata[1]->step_min !=
          vdata[2]->step_max-vdata[2]->step_min )
    )
        {
        Message error ( info_window_, XmDIALOG_ERROR, "Grids don't match!" );
        }
    else
        {
        int ndts = vdata[2]->step_max-vdata[2]->step_min+1;
        if ( memcmp ( vdata[0]->sdate, vdata[1]->sdate,
                      ( size_t ) ndts*sizeof ( int ) )
                ||
                memcmp ( vdata[0]->stime, vdata[1]->stime,
                         ( size_t ) ndts*sizeof ( int ) )
                ||
                memcmp ( vdata[1]->sdate, vdata[2]->sdate,
                         ( size_t ) ndts*sizeof ( int ) )
                ||
                memcmp ( vdata[1]->stime, vdata[2]->stime,
                         ( size_t ) ndts*sizeof ( int ) ) )
            {
            fprintf ( stderr, "Time steps don't match;"
                      " setting them to all 0's\n" );
            memset ( ( void * ) vdata[0]->stime, ( int ) NULL,
                     ( size_t ) ndts*sizeof ( int ) );
            memset ( ( void * ) vdata[0]->sdate, ( int ) NULL,
                     ( size_t ) ndts*sizeof ( int ) );
            }

        // Create the tile plot - CHECK whether tile window frees vdata when done !! - SRT
        int h, w;
        char *caseString;

        sprintf ( statusMsg, "Displaying vector plot...\n" );
        updateStatus ( statusMsg );
        // SRT 961009 sprintf(strbuf_,"%s TilePlot", _name);
        if ( inputTitleString_[0] )
            {
            strcpy ( str512_, inputTitleString_ );
            inputTitleString_[0] = '\0';
            }
        else
            {
            switch ( tileSliceType_ )
                {
                case XYSLICE:
                case XYTSLICE:
                    sprintf ( str512_, "Layer %d  %s",
                              f[0] ?
                              f[0]->get_selected_level() :
                              f[1]->get_selected_level(),
                              f[0] ?
                              formula1:
                              "Vector Plot" );
                    break;

                case XZSLICE:
                case XZTSLICE:
                    sprintf ( str512_, "Row %d  %s",
                              f[0] ?
                              f[0]->get_selected_row() :
                              f[1]->get_selected_row(),
                              f[0] ?
                              formula1:
                              "Vector Plot" );
                    break;

                case YZSLICE:
                case YZTSLICE:
                    sprintf ( str512_, "Column %d  %s",
                              f[0] ?
                              f[0]->get_selected_column() :
                              f[1]->get_selected_column(),
                              f[0] ?
                              formula1:
                              "Vector Plot" );
                    break;

                default:
                    for ( j = 0; j < 3; j++ )
                        {
                        free_vis ( vdata[i] );
                        vdata[i] = ( VIS_DATA * ) NULL;
                        }
                    sprintf ( statusMsg,
                              "Bad slice type of %d in "
                              "DriverWnd::createTileVectorObsPlot()!",
                              tileSliceType_ );
                    Message error ( info_window_,
                                    XmDIALOG_ERROR,
                                    statusMsg );
                    return 1;
                }
            }

        sprintf ( strbuf_,"%s", str512_ ); // 961009 SRT
        calcWidthHeight ( &w, &h, vdata[0] );
        if ( f[0] ) caseString = f[0]->getCasesUsedString();
        else caseString = strdup ( " " );
        if ( subTitle1String_[0] )
            {
            if ( vdata[0]->data_label ) free ( vdata[0]->data_label );
            vdata[0]->data_label = strdup ( subTitle1String_ );
            }
        TileWnd *tile  = new TileWnd ( &cfg_, ( void * ) this, app_, strbuf_,
                                       bd_,
                                       vdata[0],
                                       vdata[1],
                                       vdata[2],
                                       vdata[3],
                                       "TILE",
                                       str512_,    // titlestring
                                       subTitle2String_[0] ? subTitle2String_ : caseString,

                                       ( Dimension ) w,
                                       ( Dimension ) h,
                                       &frameDelayInTenthsOfSeconds_,
                                       0 );
        free ( caseString );
        caseString = NULL;
        subTitle1String_[0] = '\0';
        subTitle2String_[0] = '\0';

        if ( contourRange_ )
            tile->setContourRange ( minCut_, maxCut_ );

        // Register the tile plot with the synchronize dialog box
        registerSynWnd ( tile );
        }
    return 0;
    }


int DriverWnd::createScatterPlot ( char *formula1, char *formula2 )
    {
    Formula     *f[2];
    VIS_DATA    *vdata[2];
    int         i;
    char        statusMsg[512], *fn[2];

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter createScatterPlot('%s','%s')\n", formula1, formula2 );
#endif // DIAGNOSTICS

    assert ( formula1 && formula2 && formula1[0] && formula2[0] );
    f[0] = f[1] = ( Formula * ) NULL;
    vdata[0] = vdata[1] = ( VIS_DATA * ) NULL;
    fn[0] = formula1;
    fn[1] = formula2;
    statusMsg[0] = '\0';

    for ( i = 0; i < 2; i++ )
        {
        removeWhiteSpace ( fn[i] );

        // Find the currently formula's Formula object
        if ( ! ( f[i] = ( Formula * ) ( formulaList_.find ( fn[i] ) ) ) )
            {
            sprintf ( statusMsg, "Didn't find '%s'\non the formulaList!\n", fn[i] );
            if ( vdata[0] ) free_vis ( vdata[0] );
            vdata[0] = ( VIS_DATA * ) NULL;
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return 1;
            }

        // Retrieve a copy of that Formula object's data
        if ( ! ( vdata[i] = get_VIS_DATA_struct ( f[i],statusMsg,XYZTSLICE ) ) )
            {
            if ( vdata[0] ) free_vis ( vdata[0] );
            vdata[0] = ( VIS_DATA * ) NULL;
            if ( strstr ( statusMsg, "==" ) )
                displaySingleNumberFormula ( statusMsg, f[i] );
            else
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                }
            return 1;
            }
        }

    /* verify map_info's both match */
    if ( strcasecmp ( vdata[0]->map_info, vdata[1]->map_info ) )
        {
        fprintf ( stderr,
                  "\nWARNING - scatter plotting variables with different\n"
                  "map_info's... this may not make sense but \n"
                  "I'll give it a try anyway....\n" );
        }

    /* verify grids match */
    if (
        ( vdata[0]->ncol    != vdata[1]->ncol ) ||
        ( vdata[0]->nrow    != vdata[1]->nrow ) ||
        ( vdata[0]->col_min != vdata[1]->col_min ) ||
        ( vdata[0]->col_max != vdata[1]->col_max ) ||
        ( vdata[0]->row_min != vdata[1]->row_min ) ||
        ( vdata[0]->level_max-vdata[0]->level_min !=
          vdata[1]->level_max-vdata[1]->level_min )
    )
        {
        Message error ( info_window_, XmDIALOG_ERROR, "Grids don't match!" );
        return 1;
        }
    else
        {
        char *legend[] = { "      " };
        char *symbol[] = { "circle" };
        char *color[] = { "red" };
        int npoints[1];
        char message[512];
        int lenXlabel=strlen ( fn[0] ), lenYlabel=strlen ( fn[1] );
        char *xlab, *ylab;

        if ( f[0]->getUnits() ) lenXlabel += strlen ( f[0]->getUnits() )+1;
        if ( f[1]->getUnits() ) lenYlabel += strlen ( f[1]->getUnits() )+1;
        if ( ( ! ( xlab = ( char * ) malloc ( lenXlabel+1 ) ) ) ||
                ( ! ( ylab = ( char * ) malloc ( lenYlabel+1 ) ) ) )
            {
            Message error ( info_window_, XmDIALOG_ERROR,
                            "Can't allocate memory for labels!" );
            }
        xlab[0] = ylab[0] = '\0';
        if ( f[0]->getUnits() )
            {
            strcpy ( xlab, f[0]->getUnits() );
            strcat ( xlab, " " );
            }
        if ( f[1]->getUnits() )
            {
            strcpy ( ylab, f[1]->getUnits() );
            strcat ( ylab, " " );
            }
        strcat ( xlab, fn[0] );
        strcat ( ylab, fn[1] );
        updateStatus ( "Displaying scatter plot..." );
        npoints[0] = ( vdata[0]->col_max-vdata[0]->col_min+1 ) *
                     ( vdata[0]->row_max-vdata[0]->row_min+1 ) *
                     ( vdata[0]->level_max-vdata[0]->level_min+1 );
        if ( !graph2d ( vdata[0]->grid, vdata[1]->grid, 1, npoints,
                        "Scatter Plot", xlab, ylab,
                        legend, symbol, color, message, 0 ) )
            {
            Message error ( info_window_, XmDIALOG_ERROR, message );
            }
        if ( xlab ) free ( xlab );
        xlab = NULL;
        if ( ylab ) free ( ylab );
        ylab = NULL;
        }

    if ( vdata[0] ) free_vis ( vdata[0] );
    vdata[0] = ( VIS_DATA * ) NULL;
    if ( vdata[1] ) free_vis ( vdata[1] );
    vdata[1] = ( VIS_DATA * ) NULL;
    return 0;
    }


void DriverWnd::createUI ( Widget parent )
    {
    int index, i;
    assert ( parent );
    parent_ = parent;

    // Pulldown menu File ------------------------------
    menu_struct pulldown_file[9];

    index = 0;
    pulldown_file[index].name = "Choose Configuration File For New Tile Plots";
    pulldown_file[index].func = &DriverWnd::load_configMenuCB;
    pulldown_file[index].sub_menu = NULL;
    pulldown_file[index].n_sub_items = 0;
    pulldown_file[index].sub_menu_title = NULL;
    index++;

    pulldown_file[index].name = NULL;   // Separator
    pulldown_file[index].func = NULL;
    pulldown_file[index].sub_menu = NULL;
    pulldown_file[index].n_sub_items = 0;
    pulldown_file[index].sub_menu_title = NULL;
    index++;

    pulldown_file[index].name = "Load Dataset List From File";
    pulldown_file[index].func = &DriverWnd::load_caseCB;
    pulldown_file[index].sub_menu = NULL;
    pulldown_file[index].n_sub_items = 0;
    pulldown_file[index].sub_menu_title = NULL;
    index++;

    pulldown_file[index].name = "Load Formula List From File";
    pulldown_file[index].func = &DriverWnd::load_formulaCB;
    pulldown_file[index].sub_menu = NULL;
    pulldown_file[index].n_sub_items = 0;
    pulldown_file[index].sub_menu_title = NULL;
    index++;

    pulldown_file[index].name = NULL;   // Separator
    pulldown_file[index].func = NULL;
    pulldown_file[index].sub_menu = NULL;
    pulldown_file[index].n_sub_items = 0;
    pulldown_file[index].sub_menu_title = NULL;
    index++;

    pulldown_file[index].name = "Save Dataset List To File";
    pulldown_file[index].func = &DriverWnd::save_caseCB;
    pulldown_file[index].sub_menu = NULL;
    pulldown_file[index].n_sub_items = 0;
    pulldown_file[index].sub_menu_title = NULL;
    index++;

    pulldown_file[index].name = "Save Formula List To File";
    pulldown_file[index].func = &DriverWnd::save_formulaCB;
    pulldown_file[index].sub_menu = NULL;
    pulldown_file[index].n_sub_items = 0;
    pulldown_file[index].sub_menu_title = NULL;
    index++;

    pulldown_file[index].name = NULL;   // Separator
    pulldown_file[index].func = NULL;
    pulldown_file[index].sub_menu = NULL;
    pulldown_file[index].n_sub_items = 0;
    pulldown_file[index].sub_menu_title = NULL;
    index++;

    pulldown_file[index].name = "Exit PAVE";
    pulldown_file[index].func = &DriverWnd::exitCB;
    pulldown_file[index].sub_menu = NULL;
    pulldown_file[index].n_sub_items = 0;
    pulldown_file[index].sub_menu_title = NULL;
    index++;


    // Pulldown menu Datasets ------------------------------
    menu_struct pulldown_case[5];

    pulldown_case[0].name = "Edit/Select From Dataset List";
    pulldown_case[0].func = &DriverWnd::modify_caseCB;
    pulldown_case[0].sub_menu = NULL;
    pulldown_case[0].n_sub_items = 0;
    pulldown_case[0].sub_menu_title = NULL;

    pulldown_case[1].name = "Select Time Range of Current Dataset";
    pulldown_case[1].func = &DriverWnd::editDataSetStepsCB;
    pulldown_case[1].sub_menu = NULL;
    pulldown_case[1].n_sub_items = 0;
    pulldown_case[1].sub_menu_title = NULL;

    pulldown_case[2].name = "Select Layer Ranges Matching Current Dataset";
    pulldown_case[2].func = &DriverWnd::editDataSetLayerRangeCB;
    pulldown_case[2].sub_menu = NULL;
    pulldown_case[2].n_sub_items = 0;
    pulldown_case[2].sub_menu_title = NULL;

    pulldown_case[3].name = "Select Regions Of Interest Matching Current Dataset";
    pulldown_case[3].func = &DriverWnd::editDataSetROICB;
    pulldown_case[3].sub_menu = NULL;
    pulldown_case[3].n_sub_items = 0;
    pulldown_case[3].sub_menu_title = NULL;

    pulldown_case[4].name = "View Variables In Current Dataset";
    pulldown_case[4].func = &DriverWnd::access_speciesCB;
    pulldown_case[4].sub_menu = NULL;
    pulldown_case[4].n_sub_items = 0;
    pulldown_case[4].sub_menu_title = NULL;


    // Pulldown menu Formula ------------------------------
    menu_struct pulldown_formula[4];

    pulldown_formula[0].name = "Edit/Select From Formula List";
    pulldown_formula[0].func = &DriverWnd::modify_formulaCB;
    pulldown_formula[0].sub_menu = NULL;
    pulldown_formula[0].n_sub_items = 0;
    pulldown_formula[0].sub_menu_title = NULL;

    pulldown_formula[1].name = "Select Time Range of Current Formula";
    pulldown_formula[1].func = &DriverWnd::editFormulaStepsCB;
    pulldown_formula[1].sub_menu = NULL;
    pulldown_formula[1].n_sub_items = 0;
    pulldown_formula[1].sub_menu_title = NULL;

    pulldown_formula[2].name = "Select Layer Ranges Matching Current Formula";
    pulldown_formula[2].func = &DriverWnd::editFormulaLayerRangeCB;
    pulldown_formula[2].sub_menu = NULL;
    pulldown_formula[2].n_sub_items = 0;
    pulldown_formula[2].sub_menu_title = NULL;

    pulldown_formula[3].name = "Select Regions Of Interest Matching Current Formula";
    pulldown_formula[3].func = &DriverWnd::editFormulaROICB;
    pulldown_formula[3].sub_menu = NULL;
    pulldown_formula[3].n_sub_items = 0;
    pulldown_formula[3].sub_menu_title = NULL;


    // Pulldown menu Graphics-Type ---------------------
    menu_struct pulldown_grp_type[9];
    pulldown_grp_type[0].name = "X Cross Section";
    pulldown_grp_type[0].func = &DriverWnd::grp_type_XcrossCB;
    pulldown_grp_type[0].sub_menu = NULL;
    pulldown_grp_type[0].n_sub_items = 0;
    pulldown_grp_type[0].sub_menu_title = NULL;

    pulldown_grp_type[1].name = "Y Cross Section";
    pulldown_grp_type[1].func = &DriverWnd::grp_type_YcrossCB;
    pulldown_grp_type[1].sub_menu = NULL;
    pulldown_grp_type[1].n_sub_items = 0;
    pulldown_grp_type[1].sub_menu_title = NULL;

    pulldown_grp_type[2].name = "Z Cross Section";
    pulldown_grp_type[2].func = &DriverWnd::grp_type_ZcrossCB;
    pulldown_grp_type[2].sub_menu = NULL;
    pulldown_grp_type[2].n_sub_items = 0;
    pulldown_grp_type[2].sub_menu_title = NULL;

    pulldown_grp_type[3].name = NULL;  // A separator
    pulldown_grp_type[3].func = NULL;
    pulldown_grp_type[3].sub_menu = NULL;
    pulldown_grp_type[3].n_sub_items = 0;
    pulldown_grp_type[3].sub_menu_title = NULL;

    pulldown_grp_type[4].name = "Integrate Over X";
    pulldown_grp_type[4].func = &DriverWnd::grp_type_XintegCB;
    pulldown_grp_type[4].sub_menu = NULL;
    pulldown_grp_type[4].n_sub_items = 0;
    pulldown_grp_type[4].sub_menu_title = NULL;

    pulldown_grp_type[5].name = "Integrate Over Y";
    pulldown_grp_type[5].func = &DriverWnd::grp_type_YintegCB;
    pulldown_grp_type[5].sub_menu = NULL;
    pulldown_grp_type[5].n_sub_items = 0;
    pulldown_grp_type[5].sub_menu_title = NULL;

    pulldown_grp_type[6].name = "Integrate Over Z";
    pulldown_grp_type[6].func = &DriverWnd::grp_type_ZintegCB;
    pulldown_grp_type[6].sub_menu = NULL;
    pulldown_grp_type[6].n_sub_items = 0;
    pulldown_grp_type[6].sub_menu_title = NULL;

    pulldown_grp_type[7].name = NULL;  // A separator
    pulldown_grp_type[7].func = NULL;
    pulldown_grp_type[7].sub_menu = NULL;
    pulldown_grp_type[7].n_sub_items = 0;
    pulldown_grp_type[7].sub_menu_title = NULL;

    pulldown_grp_type[8].name = "Time Series";
    pulldown_grp_type[8].func = &DriverWnd::grp_type_TseriesCB;
    pulldown_grp_type[8].sub_menu = NULL;
    pulldown_grp_type[8].n_sub_items = 0;
    pulldown_grp_type[8].sub_menu_title = NULL;


    // Pulldown menu Graphics ---------------------------
    menu_struct pulldown_grp_nhour[3];
    pulldown_grp_nhour[0].name = "1 hour";
    pulldown_grp_nhour[0].func = &DriverWnd::grp_plot_1hour_tileCB;
    pulldown_grp_nhour[0].sub_menu = NULL;
    pulldown_grp_nhour[0].n_sub_items = 0;
    pulldown_grp_nhour[0].sub_menu_title = NULL;

    pulldown_grp_nhour[1].name = "8 hour";
    pulldown_grp_nhour[1].func = &DriverWnd::grp_plot_8hour_tileCB;
    pulldown_grp_nhour[1].sub_menu = NULL;
    pulldown_grp_nhour[1].n_sub_items = 0;
    pulldown_grp_nhour[1].sub_menu_title = NULL;

#if 0
    pulldown_grp_nhour[2].name = "n-hour";
    pulldown_grp_nhour[2].func = &DriverWnd::grp_plot_nhour_tileCB;
    pulldown_grp_nhour[2].sub_menu = NULL;
    pulldown_grp_nhour[2].n_sub_items = 0;
    pulldown_grp_nhour[2].sub_menu_title = NULL;
#endif

    // Pulldown menu Graphics ---------------------------
    menu_struct pulldown_graphics[14];

    i=0;
    pulldown_graphics[i].name = "Create Tile Plot";
    pulldown_graphics[i].func = &DriverWnd::grp_plot_colortileCB;
    pulldown_graphics[i].sub_menu = NULL;
    pulldown_graphics[i].n_sub_items = 0;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 1
    pulldown_graphics[i].name = "Create Vector Plot";
    pulldown_graphics[i].func = &DriverWnd::create_vectorplotCB;
    pulldown_graphics[i].sub_menu = NULL;
    pulldown_graphics[i].n_sub_items = 0;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 2
    pulldown_graphics[i].name = "Create VectorTile Plot";
    pulldown_graphics[i].func = &DriverWnd::create_tilevectorplotCB;
    pulldown_graphics[i].sub_menu = NULL;
    pulldown_graphics[i].n_sub_items = 0;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 3
    pulldown_graphics[i].name = "Create 3D Mesh Plot";
    pulldown_graphics[i].func = &DriverWnd::grp_plot_meshCB;
    pulldown_graphics[i].sub_menu = NULL;
    pulldown_graphics[i].n_sub_items = 0;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 4
    pulldown_graphics[i].name = "Create Time Series Line Plot";
    pulldown_graphics[i].func = &DriverWnd::grp_plot_linegraphCB;
    pulldown_graphics[i].sub_menu = NULL;
    pulldown_graphics[i].n_sub_items = 0;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 5
    pulldown_graphics[i].name = "Create Time Series Bar Plot";
    pulldown_graphics[i].func = &DriverWnd::grp_plot_barCB;
    pulldown_graphics[i].sub_menu = NULL;
    pulldown_graphics[i].n_sub_items = 0;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 6
    pulldown_graphics[i].name = "Create Scatter Plot";
    pulldown_graphics[i].func = &DriverWnd::create_scatterplotCB;
    pulldown_graphics[i].sub_menu = NULL;
    pulldown_graphics[i].n_sub_items = 0;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 7
    pulldown_graphics[i].name = "Create N-Hour Average Tile Plot";
    pulldown_graphics[i].func = NULL;
    pulldown_graphics[i].sub_menu = pulldown_grp_nhour;
    pulldown_graphics[i].n_sub_items = 2;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 8
    pulldown_graphics[i].name = "Create N-Layer Average Tile Plot";
    pulldown_graphics[i].func = &grp_plot_nlayerAvg_tileCB;
    pulldown_graphics[i].sub_menu = NULL;
    pulldown_graphics[i].n_sub_items = 0;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 9
    pulldown_graphics[i].name = NULL;   // Separator
    pulldown_graphics[i].func = NULL;
    pulldown_graphics[i].sub_menu = NULL;
    pulldown_graphics[i].n_sub_items = 0;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 10
    pulldown_graphics[i].name = "Animate Tile Plots Synchronously..";
    pulldown_graphics[i].func = &DriverWnd::grp_control_synCB;
    pulldown_graphics[i].sub_menu = NULL;
    pulldown_graphics[i].n_sub_items = 0;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 11
    pulldown_graphics[i].name = "Set Minimum Frame Time..";
    pulldown_graphics[i].func = minFrameTime_dialogCB;
    pulldown_graphics[i].sub_menu = pulldown_grp_type;
    pulldown_graphics[i].n_sub_items = 9;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 12
    pulldown_graphics[i].name = "Set Tile Plot Cross Section Type";
    pulldown_graphics[i].func = NULL;
    pulldown_graphics[i].sub_menu = pulldown_grp_type;
    pulldown_graphics[i].n_sub_items = 9;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;

    // 13
    pulldown_graphics[i].name = "Default slices..";
    pulldown_graphics[i].func = &DriverWnd::grp_optionCB;
    pulldown_graphics[i].sub_menu = NULL;
    pulldown_graphics[i].n_sub_items = 0;
    pulldown_graphics[i].sub_menu_title = NULL;
    i++;


    // SRT 960509    // Pulldown menu Domain ------------------------------
    // SRT 960509    menu_struct pulldown_domain[1];
    // SRT 960509    pulldown_domain[0].name = "Edit";
    // SRT 960509    pulldown_domain[0].func = &DriverWnd::domainCB;
    // SRT 960509    pulldown_domain[0].sub_menu = NULL;
    // SRT 960509    pulldown_domain[0].n_sub_items = 0;
    // SRT 960509    pulldown_domain[0].sub_menu_title = NULL;

    // Pulldown menu Export ------------------------------
    menu_struct pulldown_export[5];
    pulldown_export[0].name = "AVS5";
    pulldown_export[0].func = &DriverWnd::avsExportCB;
    pulldown_export[0].sub_menu = NULL;
    pulldown_export[0].n_sub_items = 0;
    pulldown_export[0].sub_menu_title = NULL;

    pulldown_export[1].name = "Data Explorer";
    pulldown_export[1].func = &DriverWnd::dxExportCB;
    pulldown_export[1].sub_menu = NULL;
    pulldown_export[1].n_sub_items = 0;
    pulldown_export[1].sub_menu_title = NULL;

    pulldown_export[2].name = "Iris Explorer";
    pulldown_export[2].func = &DriverWnd::ixExportCB;
    pulldown_export[2].sub_menu = NULL;
    pulldown_export[2].n_sub_items = 0;
    pulldown_export[2].sub_menu_title = NULL;

    pulldown_export[3].name = "netCDF";
    pulldown_export[3].func = &DriverWnd::netCDFExportCB;
    pulldown_export[3].sub_menu = NULL;
    pulldown_export[3].n_sub_items = 0;
    pulldown_export[3].sub_menu_title = NULL;

    pulldown_export[4].name = "Tabbed ASCII";
    pulldown_export[4].func = &DriverWnd::tabbedASCIIExportCB;
    pulldown_export[4].sub_menu = NULL;
    pulldown_export[4].n_sub_items = 0;
    pulldown_export[4].sub_menu_title = NULL;


    // Menu bar -----------------------------------------
    menu_struct menubarData[5];

    index = 0;
    menubarData[index].name = "File";
    menubarData[index].func = NULL;
    menubarData[index].sub_menu = pulldown_file;
    menubarData[index].n_sub_items = 9;
    menubarData[index].sub_menu_title = NULL;
    index++;

    menubarData[index].name = "Datasets";
    menubarData[index].func = NULL;
    menubarData[index].sub_menu = pulldown_case;
    menubarData[index].n_sub_items = 5;
    menubarData[index].sub_menu_title = NULL;
    index++;

    menubarData[index].name = "Formulas";
    menubarData[index].func = NULL;
    menubarData[index].sub_menu = pulldown_formula;
    menubarData[index].n_sub_items = 4;
    menubarData[index].sub_menu_title = NULL;
    index++;

    menubarData[index].name = "Graphics";
    menubarData[index].func = NULL;
    menubarData[index].sub_menu = pulldown_graphics;
    menubarData[index].n_sub_items = i;
    menubarData[index].sub_menu_title = NULL;
    index++;

    menubarData[index].name = "Export";
    menubarData[index].func = NULL;
    menubarData[index].sub_menu = pulldown_export;
    menubarData[index].n_sub_items = 5;
    menubarData[index].sub_menu_title = NULL;
    index++;


    // Create Pulldown menus
    createMenuBar ( parent_, ( XtPointer ) this );
    createMenuButtons ( NULL, menuBar_, menubarData, index );


    // create the help menu on the right hand side of the menu bar
    Widget helpButton, helpMenu, ug, faq, ioapi;
    helpButton = XtVaCreateManagedWidget ( "Help",xmCascadeButtonWidgetClass,menuBar_,NULL );
    XtVaSetValues ( menuBar_, XmNmenuHelpWidget, helpButton, NULL );
    helpMenu = XmCreatePulldownMenu ( menuBar_, "helpMenu", NULL, 0 );
    XtVaSetValues ( helpButton, XmNsubMenuId, helpMenu, NULL );
    ug =    XtCreateManagedWidget ( "User Guide",
                                    xmPushButtonWidgetClass, helpMenu, NULL, 0 );
    faq =   XtCreateManagedWidget ( "Frequently Asked Questions",
                                    xmPushButtonWidgetClass, helpMenu, NULL, 0 );
    ioapi = XtCreateManagedWidget ( "Models-3 IO/API",
                                    xmPushButtonWidgetClass, helpMenu, NULL, 0 );
    XtAddCallback ( ug, XmNactivateCallback, &DriverWnd::userGuideCB, this );
    XtAddCallback ( faq, XmNactivateCallback, &DriverWnd::faqCB, this );
    XtAddCallback ( ioapi, XmNactivateCallback, &DriverWnd::ioapiCB, this );


    // Store the IDs of these buttons to set their sensitivity status later
    colortile_menu_button_ = pulldown_graphics[0].wid;
    linegraph_menu_button_ = pulldown_graphics[4].wid;
    graphics_plot_type_menu_button_ = pulldown_graphics[12].wid;
    graphics_default_slices_menu_button = pulldown_graphics[13].wid;

    graphics_integrateOverX_menu_button = pulldown_grp_type[4].wid;
    graphics_integrateOverY_menu_button = pulldown_grp_type[5].wid;
    graphics_integrateOverZ_menu_button = pulldown_grp_type[6].wid;
    graphics_timeSeries_menu_button = pulldown_grp_type[8].wid;
    // SRT 960509   domain_edit_button_ = pulldown_domain[0].wid;
    export_DX_button_ = pulldown_export[1].wid;
    export_IX_button_ = pulldown_export[2].wid;

    // Create a label widget
    selection_ = XtVaCreateManagedWidget ( "Current Selections:",
                                           xmLabelWidgetClass,     parent_,
                                           XmNtopAttachment,               XmATTACH_WIDGET,
                                           XmNtopWidget,                   menuBar_,
                                           XmNtopOffset,                   10,
                                           XmNleftAttachment,              XmATTACH_FORM,
                                           XmNleftOffset,                  10,
                                           NULL );

    // Create a scroll window widget
    sw_ = XtVaCreateManagedWidget ( "sw_",
                                    xmScrolledWindowWidgetClass,    parent_,
                                    XmNtopAttachment,       XmATTACH_WIDGET,
                                    XmNtopWidget,           selection_,
                                    XmNtopOffset,           10,
                                    XmNleftAttachment,      XmATTACH_FORM,
                                    XmNleftOffset,          10,
                                    XmNrightAttachment,     XmATTACH_FORM,
                                    XmNrightOffset,         10,
                                    XmNscrollBarDisplayPolicy,  XmAS_NEEDED,
                                    XmNscrollingPolicy,     XmAUTOMATIC,
                                    NULL );

    info_window_ = XtVaCreateManagedWidget ( "info",
                   xmRowColumnWidgetClass, sw_,
                   XmNwidth,       800,
                   XmNheight,      300,
                   XmNorientation,     XmVERTICAL,
                   XmNnumColumns,      1,
                   NULL );

    LoadConfigBrowser_ = new LocalFileBrowser
    ( info_window_, ( char * ) "Configuration", ( char * ) NULL, ( void * ) NULL,
      ( void * ) &load_configCB, ( void * ) this );

    // Create a status widget
    Widget label = XtVaCreateManagedWidget ( "PAVE Status Messages: ",
                   xmLabelWidgetClass,    parent_,
                   XmNtopAttachment,       XmATTACH_WIDGET,
                   XmNtopWidget,           sw_,
                   XmNtopOffset,           20,
                   XmNleftAttachment,              XmATTACH_FORM,
                   XmNleftOffset,                  10,
                   XmNleftAttachment,      XmATTACH_FORM,
                   NULL );

    // Create a status widget
    status_ = XtVaCreateManagedWidget ( " ",
                                        xmTextWidgetClass,              parent_,
                                        XmNtopAttachment,       XmATTACH_WIDGET,
                                        XmNtopWidget,           label,
                                        XmNleftAttachment,      XmATTACH_FORM,
                                        XmNleftOffset,          10,
                                        XmNrightAttachment,     XmATTACH_FORM,
                                        XmNrightOffset,         10,
                                        XmNbottomAttachment,        XmATTACH_FORM,
                                        XmNbottomOffset,        10,
                                        XmNeditable,            False,
                                        XmNcursorPositionVisible,   False,
                                        NULL );

    updateStatus ( "Please make a selection." );

    // SRT 951221   XtSetSensitive(graphics_plot_type_menu_button_, False); // SRT
    // SRT 960509   // XtSetSensitive(domain_edit_button_, False);  // SRT
    XtSetSensitive ( export_DX_button_, False ); // SRT
    XtSetSensitive ( export_IX_button_, False ); // SRT
    XtSetSensitive ( graphics_default_slices_menu_button, False ); // SRT
    XtSetSensitive ( graphics_integrateOverX_menu_button, False ); // SRT
    XtSetSensitive ( graphics_integrateOverY_menu_button, False ); // SRT
    XtSetSensitive ( graphics_integrateOverZ_menu_button, False ); // SRT
    XtSetSensitive ( graphics_timeSeries_menu_button, False ); // SRT
    }


void DriverWnd::updateStatus ( char *msg )
    {
    XmTextSetString ( status_, msg );
    }

void DriverWnd::readHistoryFile()
    {
    char idex[256];
    FILE *fp;

    idex[0] = '\0';

    sprintf ( strbuf_, "%s/.pave.alias", getenv ( "HOME" ) );
    loadAliasFile ( strbuf_ );

    sprintf ( strbuf_, "%s/.pave.AA.cases", getenv ( "HOME" ) );
    ( ( CaseServer * ) case_ )->loadSelectionFile ( strbuf_ );
    ( ( CaseServer * ) case_ )->setDoneFirstTime();

    sprintf ( strbuf_, "%s/.pave.AA.formula", getenv ( "HOME" ) );
    formula_->loadSelectionFile ( strbuf_ );
    formula_->setDoneFirstTime();

    if ( ( fp = fopen ( history_file_, "r" ) ) != NULL )
        {
        fgets ( strbuf_, MAX_STR_LEN, fp );
        while ( !feof ( fp ) )
            {
            fscanf ( fp, "%s", idex );

            if ( !strcasecmp ( idex, "CurrCase" ) )
                {
                // SRT 95097 fscanf(fp, "%s", strbuf_);
                if ( fgets ( strbuf_, MAX_STR_LEN, fp ) ) // SRT 950907
                    {
                    int i=0;

                    // strip trailing '\n'
                    if ( strbuf_[strlen ( strbuf_ )-1]=='\n' )
                        strbuf_[strlen ( strbuf_ )-1]='\0';

                    // strip leading spaces
                    while ( ( strbuf_[i] == ' ' ) &&
                            ( i<MAX_STR_LEN ) ) i++;
                    if ( i != MAX_STR_LEN )
                        {
                        ( ( CaseServer * ) case_ )->setCurrentSelection ( &strbuf_[i] );
                        ( ( CaseServer * ) case_ )->verifyCurrentSelection();
                        }
                    }
                }
            else if ( !strcasecmp ( idex, "CurrFormula" ) )
                {
                // SRT 95097 fscanf(fp, "%s", strbuf_);
                if ( fgets ( strbuf_, MAX_STR_LEN, fp ) ) // SRT 950907
                    {
                    int i=0;

                    // strip trailing '\n'
                    if ( strbuf_[strlen ( strbuf_ )-1]=='\n' )
                        strbuf_[strlen ( strbuf_ )-1]='\0';

                    // strip leading spaces
                    while ( ( strbuf_[i] == ' ' ) &&
                            ( i<MAX_STR_LEN ) ) i++;
                    if ( i != MAX_STR_LEN )
                        {
                        formula_->setCurrentSelection ( &strbuf_[i] );
                        formula_->verifyCurrentSelection();
                        }
                    }
                }
            }
        }
    else
        {
        fprintf ( stderr, "History file %s not found.\n"
                  "No default formula & dataset will be selected.\n", history_file_ );
        return;
        }

    fclose ( fp );
    }


void DriverWnd::writeHistoryFile()
    {
    FILE *fp;
    Formula *formula;
    dataSet *dataset;
    int hadProblem;

    if ( startingUpThisPuppy_ ) return;


#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter DriverWnd::writeHistoryFile()\n" );
#endif // DIAGNOSTICS

    // write the alias list to file ~/.pave.alias
    sprintf ( strbuf_, "%s/.pave.alias", getenv ( "HOME" ) );
    writeAliasFile ( strbuf_ );

    // write the formula names out to file ~/.pave.AA.formula
    hadProblem = 0;
    sprintf ( strbuf_, "%s/.pave.AA.formula", getenv ( "HOME" ) );
    if ( ( ( fp = fopen ( strbuf_, "w" ) ) != NULL ) &&
            ( fprintf ( fp, "<FORMULAS>\n" ) > 0 ) )
        {
        formula = ( Formula * ) formulaList_.head();
        while ( formula )
            {
            if ( formula->getFormulaName() )
                {
                if ( strlen ( formula->getFormulaName() ) >0 )
                    {
                    if ( fprintf ( fp, "%s\n", formula->getFormulaName() ) <= 0 )
                        hadProblem = 1;

#ifdef DIAGNOSTICS
                    fprintf ( stderr,
                              "DriverWnd::writeHistoryFile() writing out '%s'\n",
                              formula->getFormulaName() );
#endif // DIAGNOSTICS

                    }

#ifdef DIAGNOSTICS
                else
                    fprintf ( stderr,
                              "DriverWnd::writeHistoryFile() found empty formula!\n" );
#endif // DIAGNOSTICS

                }

#ifdef DIAGNOSTICS
            else
                fprintf ( stderr,
                          "DriverWnd::writeHistoryFile() found NULL formula!\n" );
#endif // DIAGNOSTICS

            formula = ( Formula * ) formulaList_.next();
            }
        }
    else
        hadProblem = 1;
    fclose ( fp );
    if ( hadProblem ) fprintf ( stderr, "Problem writing ~/.pave.AA.formula formula list file !\n" );


    // write the dataset names out to file ~/.pave.AA.cases
    hadProblem = 0;
    sprintf ( strbuf_, "%s/.pave.AA.cases", getenv ( "HOME" ) );
    if ( ( ( fp = fopen ( strbuf_, "w" ) ) != NULL ) &&
            ( fprintf ( fp, "<CASES>\n" ) > 0 ) )
        {
        dataset = ( dataSet * ) datasetList_.head();
        while ( dataset )
            {
            if ( dataset->getFullName() )
                if ( strlen ( dataset->getFullName() ) >0 )
                    if ( fprintf ( fp, "%s\n", dataset->getFullName() ) <= 0 )
                        hadProblem = 1;
            dataset = ( dataSet * ) datasetList_.next();
            }
        }
    else
        hadProblem = 1;
    fclose ( fp );
    if ( hadProblem ) fprintf ( stderr, "Problem writing ~/.pave.AA.cases dataset list file !" );


    // write the current formula and current case out to history file
    if ( ! ( ( ( fp = fopen ( history_file_, "w" ) ) != NULL ) &&
             ( fprintf ( fp, "<HISTORY>\n" ) > 0 ) ) )
        fprintf ( stderr, "Problem writing %s history file !", history_file_ );
    else
        {
        if ( ( ( CaseServer * ) case_ )->getCurrSelection() &&
                strlen ( ( ( CaseServer * ) case_ )->getCurrSelection() ) )
            if ( fprintf ( fp, "CurrCase    %s\n", ( ( CaseServer * ) case_ )->getCurrSelection() ) == 0 )
                fprintf ( stderr, "Problem writing %s history file !", history_file_ );

        if ( formula_->getCurrSelection() &&
                strlen ( formula_->getCurrSelection() ) )
            if ( fprintf ( fp, "CurrFormula %s\n", formula_->getCurrSelection() ) == 0 )
                fprintf ( stderr, "Problem writing %s history file !", history_file_ );
        }
    if ( fp != NULL ) fclose ( fp );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Exit DriverWnd::writeHistoryFile()\n" );
#endif // DIAGNOSTICS
    }



void DriverWnd::avsExportCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->avs_export_cb();
    }


void DriverWnd::avs_export_cb()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter DriverWnd::avs_export_cb()\n" );
#endif // DIAGNOSTICS

    if ( formula_->getCurrSelection() )
        {
        char formulaname[512], statusMsg[512];
        Formula *formula;
        /* SRT 950717 struct */ VIS_DATA *vdata = NULL;

        // Miscellaneous setup junk
        stop_cb();
        strcpy ( formulaname, formula_->getCurrSelection() );
        updateStatus ( "Exporting AVS5 data file..." );

        // Find the currently selected formula's Formula object
        if ( ! ( formula = ( Formula * ) ( formulaList_.find ( formulaname ) ) ) )
            {
            sprintf ( statusMsg, "Didn't find '%s'\non the formulaList!\n", formulaname );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return;
            }

        // Retrieve a copy of that Formula object's data
        if ( ! ( vdata = get_VIS_DATA_struct ( formula,statusMsg,XYZTSLICE ) ) )
            {
            if ( strstr ( statusMsg, "==" ) )
                displaySingleNumberFormula ( statusMsg, formula );
            else
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                }
            return;
            }

        exportAVS_UI_->ShowUI ( ( void * ) vdata );
        }
    else
        {
        Message error ( info_window_, XmDIALOG_ERROR, "There is no formula currently selected!" );
        }
    }


void DriverWnd::dxExportCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->dx_export_cb();
    }


void DriverWnd::dx_export_cb()
    {
    fprintf ( stderr, "Enter DriverWnd::dx_export_cb()\n" );
    }


void DriverWnd::ixExportCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->ix_export_cb();
    }


void DriverWnd::ix_export_cb()
    {
    fprintf ( stderr, "Enter DriverWnd::ix_export_cb()\n" );
    }


void DriverWnd::tabbedASCIIExportCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->tabbed_ascii_export_cb();
    }


void DriverWnd::tabbed_ascii_export_cb()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter DriverWnd::tabbed_ascii_export_cb()\n" );
#endif // DIAGNOSTICS

    if ( formula_->getCurrSelection() )
        {
        char formulaname[512], statusMsg[512];
        Formula *formula;
        /* SRT 950717 struct */ VIS_DATA *vdata = NULL;

        // Miscellaneous setup junk
        stop_cb();
        strcpy ( formulaname, formula_->getCurrSelection() );
        updateStatus ( "Exporting tabbed ascii data file..." );

        // Find the currently selected formula's Formula object
        if ( ! ( formula = ( Formula * ) ( formulaList_.find ( formulaname ) ) ) )
            {
            sprintf ( statusMsg, "Didn't find '%s'\non the formulaList!\n", formulaname );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return;
            }

        // Retrieve a copy of that Formula object's data
        if ( ! ( vdata = get_VIS_DATA_struct ( formula,statusMsg,XYZTSLICE ) ) )
            {
            if ( strstr ( statusMsg, "==" ) )
                displaySingleNumberFormula ( statusMsg, formula );
            else
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                }
            return;
            }

        exportTabbed_UI_->ShowUI ( ( void * ) vdata );
        }
    else
        {
        Message error ( info_window_, XmDIALOG_ERROR, "There is no formula currently selected!" );
        }

    }



void DriverWnd::modify_formulaCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->modify_formula_cb();
    }

void DriverWnd::modify_formula_cb()
    {
    updateStatus ( "Edit/selecting from formula list" );
    formula_->postEditSelectionDialog();
    }



void DriverWnd::load_formulaCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->load_formula_cb();
    }


void DriverWnd::load_formula_cb()
    {
    updateStatus ( "Loading formula list from file" );
    formula_->postLoadSelectionDialog();
    }



void DriverWnd::save_formulaCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->save_formula_cb();
    }

void DriverWnd::save_formula_cb()
    {
    updateStatus ( "Saving formula list to file" );
    formula_->postSaveSelectionDialog();
    }


//*******************************************************************************

void DriverWnd::modify_caseCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->modify_case_cb();
    }

void DriverWnd::modify_case_cb()
    {
    updateStatus ( "Edit/selecting from dataset list" );
    ( ( CaseServer * ) case_ )->postEditSelectionDialog();
    }


void DriverWnd::load_caseCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->load_case_cb();
    }

void DriverWnd::load_case_cb()
    {
    updateStatus ( "Loading dataset list from file" );
    ( ( CaseServer * ) case_ )->postLoadSelectionDialog();
    }


void DriverWnd::load_configMenuCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->load_config_Menucb();
    }

void DriverWnd::load_config_Menucb()
    {
    updateStatus ( "Choosing tile plot configuration file" );
    LoadConfigBrowser_->postLoadSelectionDialog();
    }


void DriverWnd::load_configCB ( void *object, char *fname )

    {
    DriverWnd *obj = ( DriverWnd * ) object;
    obj->load_config_cb ( fname );
    }


void DriverWnd::load_config_cb ( char *fname )
    {
    char    estring[256];

    if ( cfg_.setFile ( fname, estring ) )
        {
        Message error ( info_window_, XmDIALOG_ERROR, estring );
        }
    }


void DriverWnd::save_caseCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->save_case_cb();
    }

void DriverWnd::save_case_cb()
    {
    updateStatus ( "Saving dataset list to file" );
    ( ( CaseServer * ) case_ )->postSaveSelectionDialog();
    }


void DriverWnd::editDataSetStepsCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->edit_data_set_steps_cb();
    }

void DriverWnd::edit_data_set_steps_cb()
    {
    char tstring[512], statusMsg[512];
    dataSet *dset;

    strcpy ( tstring, ( ( CaseServer * ) case_ )->getCurrSelection() );
    sprintf ( statusMsg, "Editing time range of dataset %s",
              getPointerToBaseName ( tstring ) );
    updateStatus ( statusMsg );

    if ( dset = ( dataSet * ) ( datasetList_.find ( tstring ) ) )
        dset->editDataSetSteps();
    else
        {
        sprintf ( statusMsg, "Didn't find '%s' on the dataSetList!\n", tstring );
        updateStatus ( statusMsg );
        }
    }


void  DriverWnd::editDataSetLayerRangeCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->edit_data_set_layer_range_cb();
    }


void DriverWnd::showLevelUI ( int nlev )
    {
    char statusMsg[512];
    Level *lev;

    if ( lev = ( Level * ) ( levelList_.find ( &nlev ) ) )
        {
        if ( nlev <= 1 )
            {
            Message error ( info_window_, XmDIALOG_ERROR,
                            "Can't select min/max layers for data with only one layer!" );
            }
        else
            {
            sprintf ( statusMsg, "Editing layer range for all datasets\n"
                      "and formulas with %d layers", nlev );
            updateStatus ( statusMsg );
            lev->showUI();
            }
        }
    else
        {
        sprintf ( statusMsg,
                  "Didn't find Levels with %d layers on the levelsList!", nlev );
        updateStatus ( statusMsg );
        }
    }



void DriverWnd::edit_data_set_layer_range_cb()
    {

    char datasetname[512], statusMsg[512];
    dataSet *dset;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter DriverWnd::edit_data_set_layer_range_cb()\n" );
#endif // DIAGNOSTICS

    strcpy ( datasetname, ( ( CaseServer * ) case_ )->getCurrSelection() );

    if ( dset = ( dataSet * ) ( datasetList_.find ( datasetname ) ) )
        showLevelUI ( dset->get_nlevel() );
    else
        {
        sprintf ( statusMsg, "Didn't find '%s' on the dataSetList!", datasetname );
        updateStatus ( statusMsg );
        }
    }


void DriverWnd::editDataSetROICB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->edit_data_set_roi_cb();
    }


void DriverWnd::edit_data_set_roi_cb()
    {
    char tstring[512], statusMsg[512];
    dataSet *dset;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter DriverWnd::edit_data_set_roi_cb() where datasetList_ == %ld\n",
              ( long ) &datasetList_ );
#endif // DIAGNOSTICS

    strcpy ( tstring, ( ( CaseServer * ) case_ )->getCurrSelection() );
    sprintf ( statusMsg, "Editing ROI's matching dataset %s",
              getPointerToBaseName ( tstring ) );
    updateStatus ( statusMsg );

    if ( dset = ( dataSet * ) ( datasetList_.find ( tstring ) ) )
        {
        Domain *domain;
        int ni, nj;
        int *target[3];

        ni = dset->get_ncol();
        nj = dset->get_nrow();
        target[0] = &ni;
        target[1] = &nj;;
        target[2] = ( int * ) dset->getMapInfo();

        if ( domain = ( Domain * ) ( domainList_.find ( target ) ) )
            {
            if ( domain->showUI ( statusMsg ) )
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                }
            }
        else
            {
            sprintf ( statusMsg, "Didn't find Domain %dx%d %s on the DomainList!",
                      ni, nj, dset->getMapInfo() );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            }

        }
    else
        {
        sprintf ( statusMsg, "Didn't find '%s' on the dataSetList!\n", tstring );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        }
    }


void DriverWnd::editFormulaStepsCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->edit_formula_steps_cb();
    }


void DriverWnd::editFormulaLayerRangeCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->edit_formula_layer_range_cb();
    }


void DriverWnd::edit_formula_layer_range_cb()
    {
    char formulaname[512], statusMsg[512];
    Formula *formula;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter DriverWnd::edit_formula_layer_range_cb() " );
#endif // DIAGNOSTICS

    strcpy ( formulaname, formula_->getCurrSelection() );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "with formulaname == '%s'\n", formulaname );
#endif // DIAGNOSTICS


    if ( formula = ( Formula * ) ( formulaList_.find ( formulaname ) ) )
        {
#ifdef DIAGNOSTICS
        fprintf ( stderr, "which has nlevels == %d\n", formula->getFormulaNlevel() );
#endif // DIAGNOSTICS
        showLevelUI ( formula->getFormulaNlevel() );
        }
    else
        {
        sprintf ( statusMsg, "Didn't find '%s'\non the formulaList!\n", formulaname );
        updateStatus ( statusMsg );
        }
    }


void DriverWnd::editFormulaROICB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->edit_formula_roi_cb();
    }


void DriverWnd::edit_formula_roi_cb()
    {
    char tstring[512], statusMsg[512];
    Formula *formula;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter DriverWnd::edit_formula_roi_cb()\n" );
#endif // DIAGNOSTICS

    strcpy ( tstring, formula_->getCurrSelection() );
    sprintf ( statusMsg, "Editing ROI's matching formula %s", tstring );
    updateStatus ( statusMsg );

    if ( formula = ( Formula * ) ( formulaList_.find ( tstring ) ) )
        {
        Domain *domain;
        char minfo[192];
        int ni, nj;
        int *target[3];

        if ( formula->getMapInfo ( minfo, statusMsg ) )
            {
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return;
            }

        ni = formula->get_ncol();
        nj = formula->get_nrow();
        target[0] = &ni;
        target[1] = &nj;;
        target[2] = ( int * ) minfo;

        if ( domain = ( Domain * ) ( domainList_.find ( target ) ) )
            {
            if ( domain->showUI ( statusMsg ) )
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                }
            }
        else
            {
            sprintf ( statusMsg, "Didn't find Domain %dx%d %s on the DomainList!",
                      ni, nj, minfo );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            }

        }
    else
        {
        sprintf ( statusMsg, "Didn't find '%s' on the formulaList !\n", tstring );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        }
    }



void DriverWnd::edit_formula_steps_cb()
    {
    char tstring[512], statusMsg[512];
    Formula *formula;

    strcpy ( tstring, formula_->getCurrSelection() );
    sprintf ( statusMsg, "Editing time range of formula %s", tstring );
    updateStatus ( statusMsg );

    if ( formula = ( Formula * ) ( formulaList_.find ( tstring ) ) )
        formula->editFormulaSteps();
    else
        {
        sprintf ( statusMsg, "Didn't find '%s' on the formulaList!\n", tstring );
        updateStatus ( statusMsg );
        }
    }



//**************************************************************************

void DriverWnd::invalidateAllFormulasData ( void )
    {
    int j;
    int nformulas = formulaList_.length();
    Formula *formula;

    for ( j = 0; j < nformulas; j++ )
        {
        int thisFormulaNumber = 0;
        formula = ( Formula * ) formulaList_.head();
        while ( thisFormulaNumber < j )
            {
            formula = ( Formula * ) formulaList_.next();
            thisFormulaNumber++;
            }
        if ( formula )
            if ( formula->getFormulaName() )
                if ( strlen ( formula->getFormulaName() ) >0 )
                    formula->invalidateThisData();
        }
    }



void DriverWnd::grp_type_ZcrossCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_type_Zcross_cb();
    }


void DriverWnd::grp_type_Zcross_cb()
    {
    // SRT 951222   XtSetSensitive(colortile_menu_button_, True);
    // SRT 951222   XtSetSensitive(linegraph_menu_button_, False);
    tileSliceType_ = XYTSLICE;

#ifdef DIAGNOSTICS
    fprintf ( stderr,
              "DriverWnd::grp_type_Zcross_cb() setting tileSliceType_ to XYTSLICE\n" );
#endif // DIAGNOSTICS

    // invalidateAllFormulasData();
    }


void DriverWnd::grp_type_XcrossCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_type_Xcross_cb();
    }

void DriverWnd::grp_type_Xcross_cb()
    {
    // SRT 951222   XtSetSensitive(colortile_menu_button_, True);
    // SRT 951222   XtSetSensitive(linegraph_menu_button_, False);
    tileSliceType_ = YZTSLICE;

#ifdef DIAGNOSTICS
    fprintf ( stderr,
              "DriverWnd::grp_type_Xcross_cb() setting tileSliceType_ to YZTSLICE\n" );
#endif // DIAGNOSTICS

    // invalidateAllFormulasData();
    }


void DriverWnd::grp_type_YcrossCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_type_Ycross_cb();
    }

void DriverWnd::grp_type_Ycross_cb()
    {
    // SRT 951222   XtSetSensitive(colortile_menu_button_, True);
    // SRT 951222   XtSetSensitive(linegraph_menu_button_, False);
    tileSliceType_ = XZTSLICE;

#ifdef DIAGNOSTICS
    fprintf ( stderr,
              "DriverWnd::grp_type_Ycross_cb() setting tileSliceType_ to XZTSLICE\n" );
#endif // DIAGNOSTICS

    invalidateAllFormulasData();
    }


void DriverWnd::grp_type_XintegCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_type_Xinteg_cb();
    }

void DriverWnd::grp_type_Xinteg_cb()
    {
    XtSetSensitive ( colortile_menu_button_, True );
    XtSetSensitive ( linegraph_menu_button_, False );
    }


void DriverWnd::grp_type_YintegCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_type_Yinteg_cb();
    }

void DriverWnd::grp_type_Yinteg_cb()
    {
    XtSetSensitive ( colortile_menu_button_, True );
    XtSetSensitive ( linegraph_menu_button_, False );
    }


void DriverWnd::grp_type_ZintegCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_type_Zinteg_cb();
    }

void DriverWnd::grp_type_Zinteg_cb()
    {
    XtSetSensitive ( colortile_menu_button_, True );
    XtSetSensitive ( linegraph_menu_button_, False );
    }


void DriverWnd::grp_type_TseriesCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_type_Tseries_cb();
    }

void DriverWnd::grp_type_Tseries_cb()
    {
    XtSetSensitive ( colortile_menu_button_, False );
    XtSetSensitive ( linegraph_menu_button_, True );
    }


void DriverWnd::grp_plot_colortileCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_plot_colortile_cb();
    }



int DriverWnd::grp_plot_colortile_cb()
    {
    return grp_plot_XYT ( PAVE_XYT_TILE );
    }



void DriverWnd::grp_plot_1hour_tileCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_plot_nhour_tile_cb ( 1 );
    }

void DriverWnd::grp_plot_8hour_tileCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_plot_nhour_tile_cb ( 8 );
    }

void DriverWnd::grp_plot_nhour_tileCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_plot_nhour_tile_cb ( 24 );
    }

void DriverWnd::grp_plot_nlayerAvg_tileCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_plot_nlayer_avg ( 1 );
    }

int DriverWnd::grp_plot_nhour_tile_cb ( int nhour )
    {
    return grp_plot_nhour_avg ( nhour,1 );
    }


int DriverWnd::grp_plot_mesh_cb()
    {
    return grp_plot_XYT ( PAVE_XYT_MESH );
    }



// permute_vdata_to_XYT() permutes a VIS_DATA struct's grid data
// from an XZSLICE or YZSLICE to an XYSLICE slice, in order to enable the XYTSLICE
// graphics algorithms to still work on it.  However the
// "int slice" will remain as is in order that the
// graphics routines can correctly label the row, col, or level
int DriverWnd::permute_vdata_to_XYT ( VIS_DATA *vdata, char *estring )
    {
    int ncol;                       /* number of columns in grid         */
    int nrow;                       /* number of rows in grid            */
    int nlevel;                     /* number of levels in grid          */
    int col_min;                    /* column min clamp                  */
    int col_max;                    /* column max clamp                  */
    int row_min;                    /* row min clamp                     */
    int row_max;                    /* row max clamp                     */
    int level_min;                  /* level min clamp                   */
    int level_max;                  /* level max clamp                   */
    int selected_col;               /* which column selected, 1-based    */
    int selected_row;               /* which row selected, 1-based       */
    int selected_level;             /* which level selected, 1-based     */


    // do a little error checking

    if ( !vdata )
        {
        sprintf ( estring,
                  "NULL vdata supplied to DriverWnd::permute_vdata_to_XYT()!" );
        return 1;
        }

    if ( !vdata->grid )
        {
        sprintf ( estring,
                  "NULL vdata->grid supplied to DriverWnd::permute_vdata_to_XYT()!" );
        return 1;
        }

    if ( tileSliceType_ == XYSLICE || tileSliceType_ == XYTSLICE )
        return 0;


    // copy all the original col/row/level info into temporary variables

    ncol = vdata->ncol;
    nrow = vdata->nrow;
    nlevel = vdata->nlevel;
    col_min = vdata->col_min;
    col_max = vdata->col_max;
    row_min = vdata->row_min;
    row_max = vdata->row_max;
    level_min = vdata->level_min;
    level_max = vdata->level_max;
    selected_col = vdata->selected_col;
    selected_row = vdata->selected_row;
    selected_level = vdata->selected_level;


    // now do the permuting from:
    //
    //      ^
    //      |
    //  (row) Y | XY
    //      |
    //      ---->
    //        X (col)
    //
    // to:
    //      ^
    //      |       new row   <- old level
    //  (row) Z | YZ        new col   <- old row
    //      |       new level <- old col
    //      ---->
    //        Y (col)
    //
    // or:
    //      ^
    //      |       new row   <- old level
    //  (row) Z | XZ        new level <- old row
    //      |
    //      ---->
    //        X (col)

    switch ( tileSliceType_ )
        {
        case YZSLICE:  /* YZ slice at 1 column & 1 step */
        case YZTSLICE: /* YZ slice at 1 column & all steps */

            vdata->nrow =       nlevel;
            vdata->row_min =    level_min;
            vdata->row_max =    level_max;
            vdata->selected_row =   selected_level;

            vdata->ncol =       nrow;
            vdata->col_min =    row_min;
            vdata->col_max =    row_max;
            vdata->selected_col =   selected_row;

            vdata->nlevel =     ncol;
            vdata->level_min =  col_min;
            vdata->level_max =  col_max;
            vdata->selected_level = selected_col;

            break;


        case XZSLICE:  /* XZ slice at 1 row & 1 step */
        case XZTSLICE: /* XZ slice at 1 row & all steps */

            vdata->nrow =       nlevel;
            vdata->row_min =    level_min;
            vdata->row_max =    level_max;
            vdata->selected_row =   selected_level;

            vdata->nlevel =     nrow;
            vdata->level_min =  row_min;
            vdata->level_max =  row_max;
            vdata->selected_level = level_max;

            break;


        default:
            sprintf ( estring,
                      "DriverWnd::permute_vdata_to_XYT() only handles "
                      "XZSLICE, XZTSLICE, YZSLICE, and YZTSLICE slice types!" );
            return 1;
        }


    return 0;
    }


int DriverWnd::grp_plot_XYT ( int ptype )
    {
    char formulaname[512], statusMsg[512];
    Formula *formula;
    char *caseString;
    VIS_DATA *vdata = NULL;

    if ( formula_->getCurrSelection() &&
            strlen ( formula_->getCurrSelection() ) )
        {
        // Miscellaneous setup junk
        stop_cb();
        strcpy ( formulaname, formula_->getCurrSelection() );

        // Find the currently selected formula's Formula object
        if ( ! ( formula = ( Formula * ) ( formulaList_.find ( formulaname ) ) ) )
            {
            sprintf ( statusMsg, "Can't find '%s'\non the formulaList!\n", formulaname );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return 1;
            }

        // Retrieve a copy of that Formula object's data
        if ( ! ( vdata = get_VIS_DATA_struct ( formula,statusMsg,tileSliceType_ ) ) )
            {
            if ( !statusMsg[0] )
                fprintf ( stderr,
                          "In DriverWnd::grp_plot_XYT() couldn't\n"
                          "get_VIS_DATA_struct() but statusMsg is empty !\n" );
            if ( strstr ( statusMsg, "==" ) )
                displaySingleNumberFormula ( statusMsg, formula );
            else
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                }
            return 1;
            }
        if ( vdata ) vdata->selected_step = formula->get_selected_step();

        if ( tileSliceType_ != XYTSLICE )
            if ( permute_vdata_to_XYT ( vdata, statusMsg ) )
                {
                if ( vdata ) free_vis ( vdata );
                vdata = ( VIS_DATA * ) NULL;
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                return 1;
                }

        if ( ptype == PAVE_XYT_TILE )
            // Create the tile plot - CHECK whether tile window frees vdata when done !! - SRT
            {
            int h, w;

            sprintf ( statusMsg, "Displaying %s tile plot...\n", formulaname );
            updateStatus ( statusMsg );
            // SRT 961009 sprintf(strbuf_,"%s TilePlot", _name);
            if ( inputTitleString_[0] )
                {
                strcpy ( str512_, inputTitleString_ );
                inputTitleString_[0] = '\0';
                }
            else
                {
                switch ( tileSliceType_ )
                    {
                    case XYTSLICE:
                    case XYSLICE:
                        sprintf ( str512_,
                                  "Layer %d  %s",
                                  formula->get_selected_level(),
                                  formulaname );
                        break;

                    case YZTSLICE:
                    case YZSLICE:
                        sprintf ( str512_,
                                  "Column %d  %s",
                                  formula->get_selected_column(),
                                  formulaname );
                        break;

                    case XZTSLICE:
                    case XZSLICE:
                        sprintf ( str512_,
                                  "Row %d  %s",
                                  formula->get_selected_row(),
                                  formulaname );
                        break;
                    }
                }

            calcWidthHeight ( &w, &h, vdata );
            caseString = formula->getCasesUsedString();
            if ( subTitle1String_[0] )
                {
                if ( vdata->data_label ) free ( vdata->data_label );
                vdata->data_label = strdup ( subTitle1String_ );
                }
            sprintf ( strbuf_,"%s", str512_ ); // 961009 SRT
            TileWnd *tile  = new TileWnd ( &cfg_, ( void * ) this, app_,
                                           strbuf_,
                                           bd_,
                                           vdata,
                                           "TILE",
                                           str512_,
                                           subTitle2String_[0] ? subTitle2String_ : caseString,
                                           // SRT 950707 600,
                                           ( Dimension ) w, // SRT 961001 w,
                                           ( Dimension ) h, // SRT 961001 h,
                                           &frameDelayInTenthsOfSeconds_,
                                           0 );
            free ( caseString );
            caseString = NULL;
            subTitle1String_[0] = 0;
            subTitle2String_[0] = 0;

            if ( contourRange_ )
                tile->setContourRange ( minCut_, maxCut_ );

            // Register the tile plot with the synchronize dialog box
            registerSynWnd ( tile );
            }
        else if ( ptype == PAVE_XYT_MESH )
            {
            char unit[80];
            sprintf ( statusMsg, "Displaying %s mesh plot...\n", formulaname );
            updateStatus ( statusMsg );
            unit[0] = '\0';
            if ( formula->getUnits() ) strcpy ( unit, formula->getUnits() );
            caseString = formula->getCasesUsedString();
            if ( caseString )
                if ( caseString[0] )
                    {
                    strcat ( formulaname, "  " );
                    strcat ( formulaname, caseString );
                    }
            if ( !plot_3d ( ( VIS_DATA ) *vdata, formulaname, unit, statusMsg ) )
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                return 1;
                }
            free ( caseString );
            caseString = NULL;
            }

        else
            {
            sprintf ( statusMsg, "Unknown ptype == %d in DriverWnd::grp_plot_XYT()\n", ptype );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return 1;
            }
        }
    else
        {
        Message error ( info_window_, XmDIALOG_ERROR, "There isn't a currently selected formula!" );
        return 1;
        }

    return 0;
    }


int DriverWnd::grp_plot_nhour_avg ( int nhour, int AvgOrSum )
    {
    char formulaname[512], statusMsg[512];
    Formula *formula;
    char *caseString;
    VIS_DATA *vdata = NULL;
    VIS_DATA *vdata_nh = NULL;
    int ts, tspan, nh;
    int nfloats;
    int jdate, jtime, tstep;
    int h, w;
    int t, i;
    int r, c;
    int index;
    int ncol, nrow, nlay;
    int n_per_hour;
    int n_good;
    float sum;
    float val, grid_min, grid_max;
    char title[512];

    if ( formula_->getCurrSelection() &&
            strlen ( formula_->getCurrSelection() ) )
        {
        // Miscellaneous setup junk
        stop_cb();
        strcpy ( formulaname, formula_->getCurrSelection() );

        // Find the currently selected formula's Formula object
        if ( ! ( formula = ( Formula * ) ( formulaList_.find ( formulaname ) ) ) )
            {
            sprintf ( statusMsg,"Can't find '%s'\non the formulaList!\n", formulaname );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return 1;
            }

        // Retrieve a copy of that Formula object's data
        if ( ! ( vdata = get_VIS_DATA_struct ( formula,statusMsg,tileSliceType_ ) ) )
            {
            if ( !statusMsg[0] )
                fprintf ( stderr,
                          "In DriverWnd::grp_plot_nhour_avg() couldn't\n"
                          "get_VIS_DATA_struct() but statusMsg is empty !\n" );
            if ( strstr ( statusMsg, "==" ) )
                displaySingleNumberFormula ( statusMsg, formula );
            else
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                }
            return 1;
            }

#ifdef DEBUG
        fprintf ( stderr,"DEBUG:: in NHourAvg formula=%s Nhours=%d\n",
                  formulaname,nhour );
        fprintf ( stderr,"DEBUG:: nsteps=%d\n", vdata->nstep );
        fprintf ( stderr,"DEBUG:: step_incr=%d\n",vdata->step_incr );
        fprintf ( stderr,"DEBUG:: first_date=%d\n", vdata->first_date );
        fprintf ( stderr,"DEBUG:: first_time=%d\n", vdata->first_time );
        fprintf ( stderr,"DEBUG:: last_date=%d\n", vdata->last_date );
        fprintf ( stderr,"DEBUG:: last_time=%d\n", vdata->last_time );
        fprintf ( stderr,"DEBUG:: incr_secs=%d\n", vdata->incr_sec );
#endif

        ts = vdata->incr_sec;
        if ( ts == 0 )
            {
            fprintf ( stderr,"%s\n",
                      "ERROR: cannot generate N-hour average on time independent file"
                    );
            return 1;
            }
        tspan = secsdiffc ( vdata->first_date, vdata->first_time,
                            vdata->last_date, vdata->last_time );

        if ( tspan < ( nhour-1 ) *3600 )
            {
            fprintf ( stderr,"%s %d %s\n","ERROR: not enough data for",
                      nhour,"hour average" );
            return 1;
            }
        n_per_hour = 3600 / ts;
        if ( n_per_hour == 0 )
            {
            fprintf ( stderr,"%s %d %s\n","ERROR: time step error for",
                      nhour,"hour average" );
            return 1;
            }

        nh = ( tspan - ( nhour-1 ) *3600 ) / ts + 1;
#ifdef DEBUG
        fprintf ( stderr,"DEBUG:: tspan=%d\n",tspan );
        fprintf ( stderr,"DEBUG:: # hours in %d AGV=%d\n", nhour, nh );
        fprintf ( stderr,"DEBUG:: END debugging N-hour average...\n" );
#endif

        vdata_nh = VIS_DATA_dup ( vdata, statusMsg );
        if ( vdata_nh == NULL )
            {
            fprintf ( stderr,"%s %d %s\n","ERROR: cannot duplicate VIS_DATA for",
                      nhour,"hour average" );
            return 1;
            }
        free ( vdata_nh->grid );
        free ( vdata_nh->sdate );
        free ( vdata_nh->stime );

        ncol = vdata->col_max-vdata->col_min+1;
        nrow = vdata->row_max-vdata->row_min+1;
        nlay = vdata->level_max-vdata->level_min+1;
        nfloats = ncol * nrow * nlay * nh;

        if ( ! ( vdata_nh->grid = ( float * ) malloc ( ( size_t ) ( nfloats*sizeof ( float ) ) ) ) )
            {
            fprintf ( stderr, "%s %d %s\n","ERROR: Allocation failure for",
                      nhour,"hour average" );
            return 1;
            }
        if ( ! ( vdata_nh->sdate = ( int * ) malloc ( ( size_t ) ( nh*sizeof ( int ) ) ) ) )
            {
            fprintf ( stderr, "%s %d %s\n","ERROR: Allocation failure for",
                      nhour,"hour average" );
            return 1;
            }
        if ( ! ( vdata_nh->stime = ( int * ) malloc ( ( size_t ) ( nh*sizeof ( int ) ) ) ) )
            {
            fprintf ( stderr, "%s %d %s\n","ERROR: Allocation failure for",
                      nhour,"hour average" );
            return 1;
            }
        tstep = sec2timec ( ts );
        vdata_nh->nstep = nh;
        vdata_nh->step_min = 1;
        vdata_nh->step_max = nh;
        vdata_nh->step_incr = 1;

        jdate = vdata->first_date;
        jtime = vdata->first_time;

        grid_max = BADVAL3;
        grid_min = -grid_max;
        for ( t=0; t<nh; t++ )
            {
            for ( r = 0; r < nrow; r++ )
                {
                for ( c = 0; c < ncol; c++ )
                    {
                    index = INDEX (  c, r, 0, t, ncol, nrow, 1 );
                    sum    = 0.0;
                    n_good = 0;
                    for ( i=0; i<nhour; i++ )
                        {
                        val = vdata->grid[INDEX (  c,r,0, t+i*n_per_hour, ncol, nrow, 1 )];
                        if ( !isnanf ( val ) )
                            {
                            sum += val;
                            n_good++;
                            }
                        }
                    if ( n_good != 0 )
                        {
                        if ( AvgOrSum )
                            {
                            val = sum / n_good;
                            }
                        else
                            {
                            val = sum;
                            }
                        }
                    else
                        {
                        val = setNaNf();
                        }
                    vdata_nh->grid[index] = val;
                    if ( !isnanf ( val ) )
                        {
                        if ( val < grid_min ) grid_min = val;
                        if ( val > grid_max ) grid_max = val;
                        }
                    }
                }
            vdata_nh->sdate[t] = jdate;
            vdata_nh->stime[t] = jtime;
            nextimec ( &jdate, &jtime, tstep );
            }
        vdata_nh->grid_min = grid_min;
        vdata_nh->grid_max = grid_max;
        calcWidthHeight ( &w, &h, vdata );
        caseString = formula->getCasesUsedString();
        if ( subTitle1String_[0] )
            {
            if ( vdata_nh->data_label ) free ( vdata_nh->data_label );
            vdata_nh->data_label = strdup ( subTitle1String_ );
            }
        sprintf ( str512_, "%s",
                  formulaname );

        sprintf ( strbuf_,"%s", str512_ ); // 961009 SRT
        if ( AvgOrSum )
            {
            sprintf ( title,"%d%s:%s",nhour,"-hour average",str512_ );
            }
        else
            {
            sprintf ( title,"%d%s:%s",nhour,"-hour sum",str512_ );
            }
        TileWnd *tile  = new TileWnd ( &cfg_, ( void * ) this, app_,
                                       strbuf_,
                                       bd_,
                                       vdata_nh,
                                       "NHourAVG",
                                       title,
                                       subTitle2String_[0] ? subTitle2String_ : caseString,
                                       // SRT 950707 600,
                                       ( Dimension ) w, // SRT 961001 w,
                                       ( Dimension ) h, // SRT 961001 h,
                                       &frameDelayInTenthsOfSeconds_,
                                       0 );
        // Register the tile plot with the synchronize dialog box
        registerSynWnd ( tile );
        }
    return 0;
    }


/* =:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:=:= */

int DriverWnd::grp_plot_nlayer_avg ( int AvgOrSum )
    {
    char formulaname[512], statusMsg[512];
    Formula *formula;
    char *caseString;
    VIS_DATA **vdata = NULL;
    VIS_DATA *vdata_nl = NULL;
    int nh;
    int nfloats;
    int h, w;
    int r, c, l, t;
    int index;
    int ncol, nrow, nlay;
    int n_good;
    int nlay_org;
    int zmin, zmax;
    float sum;
    float val, grid_min, grid_max;
    char title[512];

    if ( formula_->getCurrSelection() &&
            strlen ( formula_->getCurrSelection() ) )
        {
        // Miscellaneous setup junk
        stop_cb();
        strcpy ( formulaname, formula_->getCurrSelection() );

        // Find the currently selected formula's Formula object
        if ( ! ( formula = ( Formula * ) ( formulaList_.find ( formulaname ) ) ) )
            {
            sprintf ( statusMsg,"Can't find '%s'\non the formulaList!\n", formulaname );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return 1;
            }

        //how many layers in the new vdata
        nlay = 1;
        formula->get_levelrange ( &zmin, &zmax );
        nlay_org = zmax - zmin + 1;
        fprintf ( stderr,"DEBUG: formula %s has levels from %d to %d\n",
                  formulaname, zmin, zmax );

        // alloc vdata
        vdata = ( VIS_DATA ** ) malloc ( nlay_org*sizeof ( vdata[0] ) );
        if ( vdata == NULL )
            {
            fprintf ( stderr,"Allocation error in grp_plot_nlayer_avg\n" );
            return 1;
            }

        // Retrieve a copy of that Formula object's data
        for ( l=0; l<nlay_org; l++ )
            {
            index = l + zmin;
            formula->set_levelRange ( index, index );
            if ( ! ( vdata[l]=get_VIS_DATA_struct ( formula,statusMsg,tileSliceType_ ) ) )
                {
                if ( !statusMsg[0] )
                    fprintf ( stderr,
                              "In DriverWnd::grp_plot_nlayer_avg() couldn't\n"
                              "get_VIS_DATA_struct() but statusMsg is empty !\n" );
                if ( strstr ( statusMsg, "==" ) )
                    displaySingleNumberFormula ( statusMsg, formula );
                else
                    {
                    Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                    }
                return 1;
                }
            }

        vdata_nl = VIS_DATA_dup ( vdata[0], statusMsg );
        if ( vdata_nl == NULL )
            {
            fprintf ( stderr,"%s %s\n","ERROR: cannot duplicate VIS_DATA for",
                      "Nlayer average" );
            return 1;
            }
        free ( vdata_nl->grid );

        ncol = vdata[0]->col_max-vdata[0]->col_min+1;
        nrow = vdata[0]->row_max-vdata[0]->row_min+1;
        nh   = vdata[0]->nstep;
        nfloats = ncol * nrow * nlay * nh;

        if ( ! ( vdata_nl->grid = ( float * ) malloc ( ( size_t ) ( nfloats*sizeof ( float ) ) ) ) )
            {
            fprintf ( stderr, "%s %s\n","ERROR: Allocation failure for",
                      "Nlayer average" );
            return 1;
            }

        vdata_nl->nstep = nh;
        vdata_nl->step_min = 1;
        vdata_nl->step_max = nh;
        vdata_nl->step_incr = 1;

        grid_max = BADVAL3;
        grid_min = -grid_max;
        for ( t=0; t<nh; t++ )
            {
            for ( r = 0; r < nrow; r++ )
                {
                for ( c = 0; c < ncol; c++ )
                    {
                    sum = 0.0;
                    n_good = 0;
                    index = INDEX (  c, r, 0, t, ncol, nrow, 1 );
                    for ( l=0; l<nlay_org; l++ )
                        {
                        val = vdata[l]->grid[index];
                        if ( !isnanf ( val ) )
                            {
                            sum += val;
                            n_good++;
                            }
                        }
                    if ( n_good != 0 )
                        {
                        if ( AvgOrSum )
                            {
                            val = sum / n_good;
                            }
                        else
                            {
                            val = sum;
                            }
                        }
                    else
                        {
                        val = setNaNf();
                        }
                    vdata_nl->grid[index] = val;
                    if ( !isnanf ( val ) )
                        {
                        if ( val < grid_min ) grid_min = val;
                        if ( val > grid_max ) grid_max = val;
                        }
                    }
                }
            }
        vdata_nl->grid_min = grid_min;
        vdata_nl->grid_max = grid_max;
        calcWidthHeight ( &w, &h, vdata[0] );
        caseString = formula->getCasesUsedString();
        if ( subTitle1String_[0] )
            {
            if ( vdata_nl->data_label ) free ( vdata_nl->data_label );
            vdata_nl->data_label = strdup ( subTitle1String_ );
            }
        sprintf ( str512_, "Layer %d  %s",
                  formula->get_selected_level(),
                  formulaname );

        sprintf ( strbuf_,"%s", str512_ ); // 961009 SRT
        if ( AvgOrSum )
            {
            sprintf ( title,"%d-%d%s:%s",
                      zmin,
                      zmax,"-layer average",str512_ );
            }
        else
            {
            sprintf ( title,"%d-%d%s:%s",
                      zmin,
                      zmax,"-layer sum",str512_ );
            }
        TileWnd *tile  = new TileWnd ( &cfg_, ( void * ) this, app_,
                                       strbuf_,
                                       bd_,
                                       vdata_nl,
                                       "NLayerAVG",
                                       title,
                                       subTitle2String_[0] ? subTitle2String_ : caseString,
                                       // SRT 950707 600,
                                       ( Dimension ) w, // SRT 961001 w,
                                       ( Dimension ) h, // SRT 961001 h,
                                       &frameDelayInTenthsOfSeconds_,
                                       0 );
        // Register the tile plot with the synchronize dialog box
        registerSynWnd ( tile );
        }

    for ( l=0; l<nlay_org; l++ )
        {
        free_vis ( vdata[l] );
        }
    free ( vdata );
    vdata = NULL;

    return 0;
    }




void DriverWnd::grp_plot_linegraphCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_plot_linegraph_cb();
    }


int DriverWnd::grp_plot_linegraph_cb()
    {
    return grp_plot_time_series ( PAVE_TIME_LINE ); // ttype either PAVE_TIME_LINE or PAVE_TIME_BAR
    }


int DriverWnd::grp_plot_bar_cb()
    {
    return grp_plot_time_series ( PAVE_TIME_BAR ); // ttype either PAVE_TIME_LINE or PAVE_TIME_BAR
    }


int DriverWnd::grp_plot_time_series ( int ttype )
    {
    char *caseString;

    if ( formula_->getCurrSelection() )
        {
        char formulaname[512], statusMsg[512];
        Formula *formula;
        float *tsdata;

        // Miscellaneous setup junk
        stop_cb();
        strcpy ( formulaname, formula_->getCurrSelection() );

        // Find the currently selected formula's Formula object
        if ( ! ( formula = ( Formula * ) ( formulaList_.find ( formulaname ) ) ) )
            {
            sprintf ( statusMsg, "Didn't find '%s'\non the formulaList!\n", formulaname );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return 1;
            }

        // Retrieve a copy of that Formula object's time series data
        if ( ! ( tsdata = formula->get_time_series_data ( statusMsg ) ) )
            {
            if ( strstr ( statusMsg, "==" ) )
                displaySingleNumberFormula ( statusMsg, formula );
            else
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                return 1;
                }
            }

        else if ( ttype == PAVE_TIME_LINE ) // Create a line plot
            {
            char *legend[] = { "      " };
            char *symbol[] = { "circle" };
            char *color[] = { "red" };
            int npoints[1];
            char message[512];
            int i;
            float *x;
            char unitLabel[128];

            updateStatus ( "Displaying time series line plot..." );
            npoints[0] = formula->get_nsteps();
            x = new float[formula->get_nsteps()];
            for ( i=formula->get_hrMin(); i <= formula->get_hrMax(); i++ )
                x[i-formula->get_hrMin()] = ( float ) i;

            caseString = formula->getCasesUsedString();
            sprintf ( strbuf_, "%s  %s", formula->getFormulaName(), caseString );
            free ( caseString );
            caseString = NULL;

            sprintf ( unitLabel, "Average Value" );
            if ( formula->getUnits() && strlen ( formula->getUnits() ) )
                {
                strcat ( unitLabel, " (" );
                strcat ( unitLabel, formula->getUnits() );
                strcat ( unitLabel, ")" );
                }

            if ( strchr ( formula->getFormulaName(), ( int ) '[' ) )
                npoints[0]--; // don't do the last hour if d[]/dt case

            char timeStepLabel[128];
            sprintf ( timeStepLabel, "Time Step (%s to %s)",
                      formula->getTimeMinString(), formula->getTimeMaxString() );
            strcat ( unitLabel, " " );
            strcat ( unitLabel, formula->getSelectedCellRange() );
            if ( !graph2d ( x, tsdata, 1, npoints, strbuf_,
                            timeStepLabel, unitLabel,
                            legend, symbol, color, message, 1 ) )
                {
                Message error ( info_window_, XmDIALOG_ERROR, message );
                }

            free ( tsdata );
            tsdata = NULL;

            if ( x ) delete [] x;
            x = NULL;
            }

        else if ( ttype == PAVE_TIME_BAR ) // Create a bar plot
            {
            int i;
            char unitLabel[128];

            updateStatus ( "Displaying time series bar plot..." );
            combo_obj_.initialize();
            int obs_num = formula->get_nsteps();
            float *x = new float[obs_num];
            for ( i=formula->get_hrMin(); i <= formula->get_hrMax(); i++ )
                x[i-formula->get_hrMin()] = ( float ) i;
            if ( strchr ( formula->getFormulaName(), ( int ) '[' ) )
                obs_num--; // don't do the last hour if d[]/dt case
            combo_obj_.setDataArray ( x, tsdata, 1, obs_num );
            sprintf ( str512_, "%s Average Concentrations Over Time", formulaname );
            caseString = formula->getCasesUsedString();
            sprintf ( unitLabel, "Avg Value" );
            if ( formula->getUnits() && strlen ( formula->getUnits() ) )
                {
                strcat ( unitLabel, " (" );
                strcat ( unitLabel, formula->getUnits() );
                strcat ( unitLabel, ")" );
                }

            char timeStepLabel[128];
            sprintf ( timeStepLabel, "Time Step (%s to %s)",
                      formula->getTimeMinString(), formula->getTimeMaxString() );

            combo_obj_.setTitles ( formula->getSelectedCellRange(), str512_, caseString /* "Dummy Title 3 For PAVE_TIME_BAR" */, timeStepLabel, unitLabel );
            free ( caseString );
            caseString = NULL;
            sprintf ( strbuf_,"%s BarPlot", formulaname );
            BarWnd *window1 = new BarWnd ( app_, strbuf_, "BAR", &combo_obj_, 500, 450, 0 );
            if ( x ) delete [] x; // SRT 950718 added []
            }

        else
            {
            char msg[80];
            sprintf ( msg, "Unknown ttype == %d in DriverWnd::grp_plot_time_series()!", ttype );
            Message error ( info_window_, XmDIALOG_ERROR, msg );
            return 1;
            }

        }
    else
        {
        Message error ( info_window_, XmDIALOG_ERROR, "There is no formula currently selected!" );
        return 1;
        }

    return 0;
    }



void DriverWnd::grp_plot_meshCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_plot_mesh_cb();
    }



void DriverWnd::grp_plot_barCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_plot_bar_cb();
    }



void DriverWnd::access_speciesCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->access_species_cb();
    }


void DriverWnd::access_species_cb()  // SRT 960410
    {
    updateStatus ( "" );
    if ( ( ( ( CaseServer * ) case_ )->getCurrSelection() ) &&
            ( strlen ( ( ( CaseServer * ) case_ )->getCurrSelection() ) ) )
        {
        char tstring[256];
        int dataSetNumber;
        VIS_DATA *vdata;

        // determine case number
        if ( ! ( dataSetNumber = ( ( CaseServer * ) case_ )->getItemNumber ( ( ( CaseServer * ) case_ )->getCurrSelection() ) ) )
            {
            sprintf ( tstring, "Can't find dataset on list!" );
            Message error ( info_window_, XmDIALOG_ERROR, tstring );
            }
        else

            // post the species list
            if ( vdata = ( ( CaseServer * ) case_ )->get_dataSets_VISDATA ( ( ( CaseServer * ) case_ )->getCurrSelection() ) )
                {
                if ( strip_VIS_DATA_operator_chars ( vdata, tstring ) )
                    {
                    Message error ( info_window_, XmDIALOG_ERROR, tstring );
                    }
                species_->clearList();
                species_->postSpeciesDialog ( vdata->species_short_name,
                                              vdata->nspecies, dataSetNumber );
                }
            else
                {
                Message error ( info_window_, XmDIALOG_ERROR,
                                "Can't find a valid VIS_DATA struct for dataset!\n" );
                }
        }
    else
        species_->clearList();
    }



void DriverWnd::initInfo()
    {
    if ( info.filename ) free ( info.filename );
    if ( info.species_long_name ) free ( info.species_long_name );
    if ( info.species_short_name ) free ( info.species_short_name );
    if ( info.units_name ) free ( info.units_name );
    if ( info.grid ) free ( info.grid );
    if ( info.sdate ) free ( info.sdate );
    if ( info.stime ) free ( info.stime );
    if ( info.map_info ) free ( info.map_info );
    if ( info.data_label ) free ( info.data_label );
    if ( info.filehost.ip ) free ( info.filehost.ip );
    if ( info.filehost.name ) free ( info.filehost.name );

    memset ( ( void * ) &info, 0, ( size_t ) sizeof ( VIS_DATA ) );

    char *tmpstrbuf = strdup ( ( ( CaseServer * ) case_ )->getCurrSelection() );

    char *tmpptr = strchr ( tmpstrbuf, ':' );
    if ( tmpptr )
        {

        char *tmphost = strtok ( tmpstrbuf, ":" );
        tmpstrptr_ = strtok ( NULL, " " );
        info.filehost.name = strdup ( tmphost );
        }
    else
        {
        tmpstrptr_ = ( ( CaseServer * ) case_ )->getCurrSelection();
        }

    info.filename = strdup ( tmpstrptr_ );

    if ( tmpstrbuf ) free ( tmpstrbuf );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "INIT INFO INFO.FILENAME=%s|\n", info.filename );
#endif
    }


///////////////////////////////////////////////////////////////////////////////

void DriverWnd::grp_optionCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_option_cb();
    }

void DriverWnd::grp_option_cb()
    {
    option_->postOptionDialog();
    }


void DriverWnd::grp_control_synCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->grp_control_syn_cb();
    }

void DriverWnd::grp_control_syn_cb()
    {
    updateStatus ( "Displaying control panel to animate\nall tile plots synchronously..." );

    if ( synchronize_dialog_ ) // added 950802 SRT since always want to verify
        // how many hours to range over
        {
        stop_cb();      // added 950913 SRT
        XtUnmanageChild ( synchronize_dialog_ );
        XtDestroyWidget ( synchronize_dialog_ );
        synchronize_dialog_ = NULL;
        }

    max_num_hours_ = get_num_synch_steps() - 1;
    if ( max_num_hours_ > 0 )
        {
        createSynchronizeDialog ( _w );
        if ( XtIsManaged ( synchronize_dialog_ ) ) XtUnmanageChild ( synchronize_dialog_ );
        XtManageChild ( synchronize_dialog_ );
        }
    }



void DriverWnd::grp_control_deleteCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;

    obj->grp_control_delete_cb();
    }

void DriverWnd::grp_control_delete_cb()
    {
    }


char *DriverWnd::getURL ( char *name )
    {
    static char url[512];
    char   *dirname ;

    dirname = getenv ( "PAVE_DIR" ) ;
    if ( dirname && *dirname )
        {
        sprintf ( url, "file:///%s/%s", dirname, name );
        }
    else{
        sprintf ( url, "file://%s", 
                  "https://cjcoats.github.io/pave/PaveManual.html" );
        }

    return url;
    }


void DriverWnd::showURL ( char *url )
    {
    char message[256];

    int moduleId = BusFindModuleByName ( bd_, "Help" );
    if ( moduleId < 0 ) /* we need to start the Help daemon */
        {
        BusVerifyClient ( bd_, NULL, "Help", 1, 18, NULL, message );
        moduleId = BusFindModuleByName ( bd_, "Help" );
        if ( moduleId < 0 ) /* we couldn't start the Help daemon */
            {
            Message error (  info_window_, XmDIALOG_ERROR,
                             "Couldn't start help daemon!" );
            return;
            }
        }

    int typeId = BusFindTypeByName ( bd_, "HTML URL" );
    if ( typeId > 0 )
        {
        struct BusMessage bmsg;
        bmsg.toModule = moduleId;
        bmsg.messageType = typeId;
        bmsg.messageLength = strlen ( url )+1;
        bmsg.message = url;
        BusSendMessage ( bd_, &bmsg ); // send the message via bus-master
        }
    else
        {
        Message error (  info_window_, XmDIALOG_ERROR,
                         "Cannot identify message type: HTML URL" );
        }
    return;
    }


void DriverWnd::userGuideCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->updateStatus ( "Displaying PAVE User Guide using Mosaic..." );
    obj->showURL ( obj->getURL ( "Pave.html" ) );
    }


void DriverWnd::faqCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->updateStatus ( "Displaying PAVE FAQ using Mosaic..." );
    obj->showURL ( obj->getURL ( "Pave.FAQ.html" ) );
    }

void DriverWnd::ioapiCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->updateStatus ( "Displaying I/O API documentation..." );
    obj->showURL ( "https://cjcoats.github.io/ioapi/AA.html" );
    }



extern "C" {
    int BusSendBusByte ( struct BusData *, int, unsigned char, int,  char * );
    }
void DriverWnd::exitCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    pid_t mypid = getpid();
    mypid = -mypid;

    obj->writeHistoryFile();
    BusSendBusByte ( obj->bd_, MASTERID, BUSBYTE_MODULE_LEAVING, 0, NULL ); // SRT 961031
    // pave_log_stop(); // disabled by A.Traynov 04/10/2002

    //   char command[256];
    //   sprintf(command, "rm -rf /tmp/sbus*%s", get_user_name());
    //   system(command);
#ifndef __INSIGHT__
    //   killmykids();
#endif /* #ifndef __INSIGHT__ */
    if ( getenv ( "KILL_PROCESS_GROUP" ) != NULL )
        {
        if ( strcmp ( getenv ( "KILL_PROCESS_GROUP" ),"ON" ) == 0 )
            {
            killpg ( 0, SIGKILL );
            }
        }
    // AME: changed from 1 to 0
    exit ( 0 );
    }



void DriverWnd::closeCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->close_cb();
    }

void DriverWnd::close_cb()
    {
    XtUnmanageChild ( synchronize_dialog_ );
    }


void DriverWnd::stopCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->stop_cb();
    }

void DriverWnd::stop_cb()
    {
    if ( work_proc_id_ )
        {
        XtRemoveWorkProc ( work_proc_id_ );
        work_proc_id_ = ( XtWorkProcId ) NULL;
        }
    }


void DriverWnd::animateCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->animate_cb();
    }

void DriverWnd::animate_cb()
    {
    if ( work_proc_id_ == ( XtWorkProcId ) NULL )
        work_proc_id_ = XtAppAddWorkProc ( app_->appContext(),
                                           &DriverWnd::synTrigger,
                                           ( XtPointer ) this );
    }


Boolean DriverWnd::synTrigger ( XtPointer clientData )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->synchronizeWindows();

    // are we animating through just one cycle?
    if ( obj->num_to_loop_ > 0 )
        {
        obj->num_to_loop_--;

        // have we reached the end of the cycle?  If so, exit program
        if ( obj->num_to_loop_ == 0 )
            {
            obj->stop_cb();
            exitCB ( ( Widget ) NULL , ( XtPointer ) obj, ( XtPointer ) NULL );
            }
        }

    return 0;
    }


void DriverWnd::synchronizeWindows()
    {
    curr_animate_  = animate_frame_++ % ( max_num_hours_+1 ); //SRT 950802 MAX_WINDOWS

    int i;
    int k, offs, skip;

    if ( frameDelayInTenthsOfSeconds_ )
        registerCurrentTime();

    for ( i=0; i < num_tilewnd_; i++ )
        {
        offs = tilewnd_list_[i]->get_offset();
        skip = tilewnd_list_[i]->get_skip();
        k = offs - 1 + curr_animate_*skip;
        tilewnd_list_[i]->synchronizeAnimate ( k );
        }
    if ( frameDelayInTenthsOfSeconds_ )
        verifyElapsedClockTime ( ( float ) ( frameDelayInTenthsOfSeconds_/10.0 ) );

    // Update value on scale widget.
    XtVaSetValues ( animate_scale_, XmNvalue, curr_animate_, NULL );
    }



void DriverWnd::animate_scaleCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    XmScaleCallbackStruct * cbs = ( XmScaleCallbackStruct * ) callData;
    obj->animate_scale_cb ( cbs->value );
    }

void DriverWnd::animate_scale_cb ( int value )
    {
    curr_animate_ = animate_frame_ = value;

    int i;
    int k, offs, skip;

    for ( i=0; i < num_tilewnd_; i++ )
        {
        offs = tilewnd_list_[i]->get_offset();
        skip = tilewnd_list_[i]->get_skip();
        k = offs - 1 + curr_animate_*skip;
        tilewnd_list_[i]->synchronizeAnimate ( k );
        }
    }


void DriverWnd::createSynchronizeDialog ( Widget parent )
    {
    assert ( parent );

    stop_cb(); // SRT 970103 work_proc_id_ = (XtWorkProcId) NULL;

    // Create control dialog box.
    synchronize_dialog_ = XmCreateFormDialog ( parent, "Synchronize", NULL, 0 );
    XtVaSetValues ( synchronize_dialog_,
                    XmNautoUnmanage,    False,
                    NULL );

    Widget form1 = XtVaCreateWidget ( "form1",
                                      xmFormWidgetClass,  synchronize_dialog_,
                                      XmNtopAttachment,   XmATTACH_FORM,
                                      XmNtopOffset,       10,
                                      XmNleftAttachment,  XmATTACH_FORM,
                                      XmNrightAttachment, XmATTACH_FORM,
                                      NULL );

    animate_scale_ = XtVaCreateManagedWidget ( "Step",
                     xmScaleWidgetClass, form1,

                     XtVaTypedArg, XmNtitleString, XmRString, "Timestep", 5,

                     XmNheight,      100,
                     XmNmaximum,         max_num_hours_, //SRT 950802 MAX_WINDOWS
                     XmNminimum,     0,
                     XmNvalue,       0,
                     XmNshowValue,       True,
                     XmNorientation,     XmHORIZONTAL,
                     XmNtopAttachment,   XmATTACH_FORM,
                     XmNleftAttachment,  XmATTACH_FORM,
                     XmNleftOffset,      10,
                     XmNrightAttachment, XmATTACH_FORM,
                     XmNrightOffset,     10,
                     NULL );

    XtAddCallback ( animate_scale_, XmNvalueChangedCallback,  &DriverWnd::animate_scaleCB, ( XtPointer ) this );

    Widget sep = XtVaCreateManagedWidget ( "sep",
                                           xmSeparatorWidgetClass,     form1,
                                           XmNleftAttachment,  XmATTACH_FORM,
                                           XmNrightAttachment, XmATTACH_FORM,
                                           XmNtopAttachment,   XmATTACH_WIDGET,
                                           XmNtopWidget,       animate_scale_,
                                           XmNtopOffset,       10,
                                           NULL );

    // Button to start animating.
    animate_ = XtVaCreateManagedWidget ( "Animate",
                                         xmPushButtonWidgetClass, form1,
                                         XmNtopAttachment,   XmATTACH_WIDGET,
                                         XmNtopWidget,       sep,
                                         XmNtopOffset,       10,
                                         XmNleftAttachment,  XmATTACH_FORM,
                                         XmNleftOffset,      10,
                                         XmNbottomAttachment,    XmATTACH_FORM,
                                         XmNbottomOffset,    10,
                                         XmNwidth,       100,
                                         XmNheight,      40,
                                         NULL );
    XtAddCallback ( animate_, XmNactivateCallback, &DriverWnd::animateCB, ( XtPointer ) this );

    // Button to stop animating.
    stop_ = XtVaCreateManagedWidget ( "Stop",
                                      xmPushButtonWidgetClass, form1,
                                      XmNtopAttachment,   XmATTACH_WIDGET,
                                      XmNtopWidget,       sep,
                                      XmNtopOffset,       10,
                                      XmNleftAttachment,  XmATTACH_WIDGET,
                                      XmNleftWidget,      animate_,
                                      XmNleftOffset,      10,
                                      XmNwidth,       100,
                                      XmNheight,      40,
                                      NULL );
    XtAddCallback ( stop_, XmNactivateCallback, &DriverWnd::stopCB, ( XtPointer ) this );


    close_ = XtVaCreateManagedWidget ( "Close",
                                       xmPushButtonWidgetClass, form1,
                                       XmNtopAttachment,   XmATTACH_WIDGET,
                                       XmNtopWidget,       sep,
                                       XmNtopOffset,       10,
                                       XmNleftAttachment,  XmATTACH_WIDGET,
                                       XmNleftWidget,      stop_,
                                       XmNleftOffset,      10,
                                       XmNrightAttachment, XmATTACH_FORM,
                                       XmNrightOffset,     10,
                                       XmNwidth,       100,
                                       XmNheight,      40,
                                       NULL );
    XtAddCallback ( close_, XmNactivateCallback, &DriverWnd::closeCB, ( XtPointer ) this );

    if ( XtIsManaged ( form1 ) ) XtUnmanageChild ( form1 );
    XtManageChild ( form1 );
    }


void TileWndHasBeenClosed_CB ( void *dwnd )
    {
    DriverWnd *d = ( DriverWnd * ) dwnd;
    if ( d ) d->registerSynWnd ( ( TileWnd * ) NULL );
    }


void DriverWnd::closeWnd ( long windowid )
    {
    int i;
    for ( i=0; i<num_tilewnd_; i++ )
        if ( tilewnd_list_[i]->getWindowId() == windowid )
            {
            tilewnd_list_[i]->exit_cb();
            return;
            }

    Message error ( info_window_,XmDIALOG_ERROR,"Couldn't find window to close!\n" );
    }

void DriverWnd::raiseWnd ( long windowid )
    {
    int i;
    for ( i=0; i<num_tilewnd_; i++ )
        if ( tilewnd_list_[i]->getWindowId() == windowid )
            {
            XRaiseWindow ( XtDisplay ( _w ), windowid ); // added by ALT 2000/01/07
            return;
            }

    Message error ( info_window_,XmDIALOG_ERROR,
                    "Couldn't find window to raise!\n" );
    }

int DriverWnd::animateWnd ( long windowid, int timestep )
    {
    int i;
    int n;
    for ( i=0; i<num_tilewnd_; i++ )
        if ( tilewnd_list_[i]->getWindowId() == windowid )
            {
            n = tilewnd_list_[i]->get_nhours();
            if ( timestep >= 0 && timestep < n )
                {
                tilewnd_list_[i]->animateTileCore ( timestep );
                return 0;
                }
            else
                {
                Message error ( info_window_,XmDIALOG_ERROR,
                                "Illegal range for TIMESTEP!\n" );
                return 1;
                }
            }

    Message error ( info_window_,XmDIALOG_ERROR,"Couldn't find window to animate!\n" );
    return 1;
    }

void DriverWnd::registerSynWnd ( TileWnd *tile )
    {
    int i, oldmax;
    TileWnd **newList;
    char errMessage[256];


    if ( tile == NULL )
        {
        return;
        }

    mostRecentTile_ = tile;
    if ( tile->get_tstep() == 0 ) return; // time independent file

    newList = new TileWnd*[num_tilewnd_+1];

    // Clean up unmanged TileWnd
    int j = 0;
    for ( i=0; i< num_tilewnd_; i++ )
        {
        if ( tilewnd_list_[i]->isManage() )
            {
            newList[j] = tilewnd_list_[i];
            j++;
            }
        else
            {
            if ( mostRecentTile_ == tilewnd_list_[i] ) mostRecentTile_ = NULL;
            delete tilewnd_list_[i];
            }
        }

    delete[] tilewnd_list_;
    tilewnd_list_ = newList;
    num_tilewnd_ = j;

    tilewnd_list_[num_tilewnd_] = tile;
    num_tilewnd_++;

    // now set max_num_hours_

    oldmax = max_num_hours_;
    //   max_num_hours_ = 0;
    //   for (i=0; i<num_tilewnd_; i++)
    //   {
    //   int nhours = tilewnd_list_[i]->get_nhours();
    //   if (nhours-1 > max_num_hours_) max_num_hours_ = nhours-1;
    //   }

    if ( synchronize_dialog_ )
        {
        stop_cb();
        XtUnmanageChild ( synchronize_dialog_ );
        XtDestroyWidget ( synchronize_dialog_ );
        synchronize_dialog_ = NULL;

        sprintf ( errMessage, "%s\n%s\n%s",
                  "WARNING: The open \"Synchronous Animation\" window was closed by PAVE",
                  "when you have opened a new Tile Plot window.",
                  "If you need to use it please open a new one." );

        Message error ( info_window_, XmDIALOG_ERROR, errMessage );
        fprintf ( stderr,"\n" );
        fprintf ( stderr,"%s\n",errMessage );
        fprintf ( stderr,"\n" );
        }

    if ( num_tilewnd_ == 0 )
        {
        stop_cb();
        if ( synchronize_dialog_ )
            {
            XtUnmanageChild ( synchronize_dialog_ );
            XtDestroyWidget ( synchronize_dialog_ );
            synchronize_dialog_ = NULL;
            }
        }

    if ( max_num_hours_ > oldmax ) // added 950802 SRT since always want to reset
        if ( synchronize_dialog_ ) // how many hours to range over
            {
            XtUnmanageChild ( synchronize_dialog_ );
            XtDestroyWidget ( synchronize_dialog_ );
            synchronize_dialog_ = NULL;
            }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "NUM_TILEWND=%d\n", num_tilewnd_ );
#endif
    }


void DriverWnd::busCallback ( char *msg )
    {
    parseMessage ( msg );
    }


int parseLine ( char *msg, char ***pargv, char *errorMsg )
    {
    int argc=0;
    char *var;
    int len;
    char **argv = NULL;
    int i=0;
    int ans=0;
    char c;


    if ( !msg ) goto handle_errors;
    if ( !msg[0] ) goto handle_errors;

    len = strlen ( msg );
    argv = ( char ** ) malloc ( len*sizeof ( argv[0] ) );
    /* allocation size=len is overkill but simplifies the code */
    if ( argv == NULL )
        {
        sprintf ( errorMsg,"Memory allocation error (argv)\n" );
        goto handle_errors;
        }

    while ( msg[i] != '\0' )
        {
        /* ignore white space at beginning of line */
        while ( whitespace ( msg[i] ) && msg[i] != '\0' )
            i++;
        if ( msg[i] == '\0' ) break;

        /* set 'var' to variable name */
        var = msg + i++;          /* skip to next character */

        /* find end of variable name */
        if ( isquote ( var[0] ) )
            {
            c = var[0]; /* save the quote (") or (') character */
            /* remove the (") quotes */
            var++;
            i++;
            while ( msg[i] != c )
                {
                if ( msg[i] == '\0' )
                    {
                    sprintf ( errorMsg, "Unmatched %c.\n",c );
                    ans = 1;
                    goto handle_errors;
                    }
                i++;
                }
            /* remove the (") quotes */
            msg[i++] = '\0';
            }
        else
            {
            while ( !whitespace ( msg[i] ) && msg[i] != '\0' )
                i++;
            }

        msg[i++] = '\0';
        argv[argc] = strdup ( var );
        argc++;
        }

handle_errors:
    if ( ans )
        {
        argc = 0;
        fprintf ( stderr,"ERROR: %s\n",errorMsg );
        }
    *pargv = argv;
    return argc;
    }

void DriverWnd::parseMessage ( char *msg )
    {
    int argc=0;
    int ans;
    char **argv = NULL;
    char errorMsg[512];
    extern int parseLine ( char *, char ***, char * );

    argc = parseLine ( msg, &argv, errorMsg );
    // process the tokens
    ans = processArgs ( argc, argv, errorMsg );

    if ( ( getenv ( "KEDAMODE" ) != NULL ) && ( !strcmp ( getenv ( "KEDAMODE" ), "1" ) ) )
        {
        if ( ans )
            fprintf ( stderr, "KEDA_FAILURE: %s\n", errorMsg );
        else
            {
            fprintf ( stdout, "KEDA_SUCCESS!\n" );
            fflush ( stdout );
            }
        }


    // free up the tokens
#ifdef MITICULOUS_CLEAN
    if ( argv )
        {
        int i;
        for ( i = 0; i < argc; i++ )
            {
            if ( argv[i] )
                {
                free ( argv[i] );
                argv[i] = NULL;
                }
            }
        free ( argv );
        }
#endif

    return;
    }



int DriverWnd::processArgs ( int argc, char *argv[], char *estring )
    {
    int i, j, nfiles = 0, a, b, ans=0;
    char *p, *arg;
    int level = -1, levelMin = -1, levelMax = -1, ts = -1, ti = -1, tf = -1, nformulas;
    int xmin, xmax, ymin, ymax;
    Formula *formula;
    int fdomain, sdomain;
    dataSet *dset;
    char tstring[256];
    extern float default_vector_scale_;
    extern int   default_vector_skip_;

    assert ( estring );
    inputTitleString_[0] = '\0';
    subTitle1String_[0] = '\0';
    subTitle2String_[0] = '\0';
    contourRange_ = 0;

#ifdef DIAGNOSTICS
    for ( i = 0; i < argc; i++ )
        fprintf ( stderr, "arg[%d] == '%s'\n", i, argv[i] );
#endif // DIAGNOSTICS

    i = 0;
    while ( i < argc )
        {
        p=argv[i];
        fdomain = sdomain = 0;

        // remove any leading spaces in the argument
        a = 0;
        while ( p[a] == ' ' ) a++;
        if ( a )
            {
            b = a;
            do
                {
                p[b-a] = p[b];
                b++;
                }
            while ( p[b-1] != '\0' );
            }

#ifdef DIAGNOSTICS
        fprintf ( stderr, "arg[%d] == '%s'\n", i, argv[i] );
#endif // DIAGNOSTICS

        // ignore any arg with PAVE or pave in it
        // (usually this will be argv[0] if it exists at all)
        if ( strstr ( p, "PAVE" ) ||
                strstr ( p, "pave" ) )
            {
#ifdef DIAGNOSTICS
            fprintf ( stderr,
                      "Ignoring PAVE arg arg[%d] == '%s'\n", i, argv[i] );
#endif // DIAGNOSTICS
            }

        else if ( !strcasecmp ( p, "-nodisplay" ) )
            {
#ifdef DIAGNOSTICS
            fprintf ( stderr,
                      "-nodisplay argument!\n", i, argv[i] );
#endif // DIAGNOSTICS
            }

        else if ( !strcasecmp ( p, "-alias" ) )
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "Fewer args supplied to -alias option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            addAlias ( argv[i] );

            }

        else if ( !strcasecmp ( p, "-unalias" ) )
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "Fewer args supplied to -alias option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            removeAlias ( argv[i] );

            }

        else if ( !strcasecmp ( p, "-printAlias" ) )
            {
            formula_->printAlias();
            }

        else if ( !strcasecmp ( p, "-f" ) ) // next arg is [<host>:]<filename>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No filename supplied to -f option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];

#ifdef DIAGNOSTICS
            fprintf ( stderr, "dataSet %c == '%s'\n", ( int ) 'a'+nfiles, arg );
#endif // DIAGNOSTICS
            nfiles++;

            if ( nfiles == 1 )
                {
                // trash all existing formulas
                formulaList_.freeContents();
                formula_->freeSelectionElements();

                // trash all existing datasets
                datasetList_.freeContents();
                ( ( CaseServer * ) case_ )->freeSelectionElements();
                }

            if ( ( ( CaseServer * ) case_ )->addItem ( arg, ( ( CaseServer * ) case_ )->getSelectionDialog() ) )
                {
                ans = 1;
                sprintf ( estring, "Failure adding file '%s'!!", arg );
                }
            ( ( CaseServer * ) case_ )->setCurrentSelection ( arg );
            ( ( CaseServer * ) case_ )->verifyCurrentSelection();
            }

        else if ( !strcasecmp ( p, "-tileYlabelsOnRight" ) )
            {
            static char *tileYlabelsOnRight = "TILEYLABELSONRIGHT=1";
#ifdef DIAGNOSTICS
            fprintf ( stderr,
                      "-tileYlabelsOnRight argument!\n" );
#endif // DIAGNOSTICS
            /*
            tileYlabelsOnRight is declared static as per the man pages
            for putenv's mention of a potential error:

                int putenv(char *string)

                A potential error is to  call  putenv()  with  an  automatic
                variable  as  the  argument,  then exit the calling function
                while string is still part of the environment.
            */
            putenv ( tileYlabelsOnRight );
            }

        else if ( !strcasecmp ( p, "-kedamode" ) )
            {
            static char *kedamode = "KEDAMODE=1";
#ifdef DIAGNOSTICS
            fprintf ( stderr,
                      "-kedamode argument!\n" );
#endif // DIAGNOSTICS
            /*
            kedamode is declared static as per the man pages
            for putenv's mention of a potential error:

                int putenv(char *string)

                A potential error is to  call  putenv()  with  an  automatic
                variable  as  the  argument,  then exit the calling function
                while string is still part of the environment.
            */
            putenv ( kedamode );
            }

        else if ( !strcasecmp ( p, "-LegendBins" ) ) // next arg is <LegendBins>
            {
            static char legendbins[256];

            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No LegendBins string supplied to -LedendBins option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }

            /*
            ledendbins is declared static as per the man pages
            for putenv's mention of a potential error:

                int putenv(char *string)

                A potential error is to  call  putenv()  with  an  automatic
                variable  as  the  argument,  then exit the calling function
                while string is still part of the environment.
            */
            sprintf ( legendbins, "LEGEND_BINS=%s", argv[i] );
            putenv ( legendbins );
            }
        else if ( !strcasecmp ( p, "-MapCounties" ) )
            {
            static char *mapchoice = "MAPCHOICE=1";
            putenv ( mapchoice );
            }

        else if ( !strcasecmp ( p, "-MapMediumRes" ) )
            {
            static char *mapchoice = "MAPCHOICE=2";
            putenv ( mapchoice );
            }

        else if ( !strcasecmp ( p, "-MapHiRes" ) )
            {
            static char *mapchoice = "MAPCHOICE=3";
            putenv ( mapchoice );
            }

        else if ( !strcasecmp ( p, "-MapRivers" ) )
            {
            static char *mapchoice = "MAPCHOICE=4";
            putenv ( mapchoice );
            }

        else if ( !strcasecmp ( p, "-MapRoads" ) )
            {
            static char *mapchoice = "MAPCHOICE=5";
            putenv ( mapchoice );
            }

        else if ( !strcasecmp ( p, "-MapWorld" ) )
            {
            static char *mapchoice = "MAPCHOICE=6";
            putenv ( mapchoice );
            }

        else if ( !strcasecmp ( p, "-barplotYformat" ) ) // next arg is <barplotYformat>
            {
            static char barplotYformat[256];

            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No barplotYformat supplied to -barplotYformat option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }

#ifdef DIAGNOSTICS
            fprintf ( stderr,
                      "-tileYlabelsOnRight argument '%s'!\n", argv[i] );
#endif // DIAGNOSTICS
            /*
            barplotYformat is declared static as per the man pages
            for putenv's mention of a potential error:

                int putenv(char *string)

                A potential error is to  call  putenv()  with  an  automatic
                variable  as  the  argument,  then exit the calling function
                while string is still part of the environment.
            */
            sprintf ( barplotYformat, "BARPLOTYFORMAT=%s", argv[i] );
            putenv ( barplotYformat );
            }

        else if ( !strcasecmp ( p, "-imageMagickArgs" ) ) // next arg is args to pass
            {
            static char imageMagickArgs[256];
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No args supplied after -imageMagickArgs option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }

            sprintf ( imageMagickArgs, "IMAGE_MAGICK_ARGS=%s", argv[i] );
            putenv ( imageMagickArgs );
            }
        else if ( !strcasecmp ( p, "-drawLegend" ) ) // next arg is <on/off>
            {
            static char legendStr[100];
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No on/off supplied after -drawLegend option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }

            sprintf ( legendStr, "DRAWLEGEND=%s", argv[i] );
            putenv ( legendStr );
            }
        else if ( !strcasecmp ( p, "-drawGridLabels" ) ) // next arg is <on/off>
            {
            static char gridLabelStr[100];
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No on/off supplied after -drawLegend option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }

            sprintf ( gridLabelStr, "DRAWGRIDLABELS=%s", argv[i] );
            putenv ( gridLabelStr );
            }

        else if ( !strcasecmp ( p, "-drawTiles" ) ) // next arg is <on/off>
            {
            static char tileStr[100];
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No on/off supplied after -drawTiles option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }

            sprintf ( tileStr, "DRAWTILES=%s", argv[i] );
            putenv ( tileStr );
            }

        else if ( !strcasecmp ( p, "-drawTimeStamp" ) ) // next arg is <on/off>
            {
            static char timeStampStr[100];
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No on/off supplied after -drawTimeStamp option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }

            sprintf ( timeStampStr, "DRAWTIMESTAMP=%s", argv[i] );
            putenv ( timeStampStr );
            }

        else if ( !strcasecmp ( p, "-drawMinMax" ) ) // next arg is <on/off>
            {
            static char minMaxStr[100];
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No on/off supplied after -drawMinMax option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }

            sprintf ( minMaxStr, "DRAWMINMAX=%s", argv[i] );
            putenv ( minMaxStr );
            }

        else if ( !strcasecmp ( p, "-onlyDrawLegend" ) ) // next arg is <on/off>
            {
            static char onlyLegendStr[100];
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No on/off supplied after -onlyDrawLegend option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }

            sprintf ( onlyLegendStr, "DRAWONLYLEGEND=%s", argv[i] );
            putenv ( onlyLegendStr );
            }

        else if ( !strcasecmp ( p, "-mapName" ) ) // next arg is <mapfilename>
            {
            static char mapname[256];

            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No mapName supplied to -mapName option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }

            /*
            mapname is declared static as per the man pages
            for putenv's mention of a potential error:

                int putenv(char *string)

                A potential error is to  call  putenv()  with  an  automatic
                variable  as  the  argument,  then exit the calling function
                while string is still part of the environment.
            */
            sprintf ( mapname, "MAPNAME=%s", argv[i] );
            putenv ( mapname );
            }

        else if ( !strcasecmp ( p, "-display" ) ) // where should this be displayed?
            {
            static char display[256];

            /*
            display is declared static as per the man pages
            for putenv's mention of a potential error:

                int putenv(char *string)

                A potential error is to  call  putenv()  with  an  automatic
                variable  as  the  argument,  then exit the calling function
                while string is still part of the environment.
            */
            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No display supplied to -display option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            sprintf ( display, "DISPLAY=%s", argv[i] );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "%s\n", display );
#endif // DIAGNOSTICS
            putenv ( display );
            }

        else if ( !strcasecmp ( p, "-titleFont" ) ) // next arg is <fontFamily>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "Not enough args supplied to -titleFont option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            changeTitleFontSize ( atoi ( argv[i] ) );
            }

        else if ( !strcasecmp ( p, "-titleString" ) ) // next arg is <titleString>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No titleString supplied to -titleString option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            strcpy ( inputTitleString_, argv[i] );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "titleString == '%s'\n", inputTitleString_ );
#endif // DIAGNOSTICS
            }

        else if ( !strcasecmp ( p, "-system" ) ) // next arg is <unix command>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No command supplied to -system option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
#ifdef DIAGNOSTICS
            fprintf ( stderr, "system(\"%s\")\n", argv[i] );
#endif // DIAGNOSTICS
            XSync ( XtDisplay ( _w ), False );
            system ( argv[i] );
            XSync ( XtDisplay ( _w ), False );
            }


        else if ( !strcasecmp ( p, "-subtitleFont" ) ) // next arg is <fontFamily>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "Not enough args supplied to -subTitleFont option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            changeSubTitleFontSize ( atoi ( argv[i] ) );
            }

        else if ( !strcasecmp ( p, "-subTitle1" ) ) // next arg is <subTitle1>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No subTitle1 supplied to -subTitle1 option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            strcpy ( subTitle1String_, argv[i] );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "subTitle1 == '%s'\n", subTitle1String_ );
#endif // DIAGNOSTICS
            }

        else if ( !strcasecmp ( p, "-subTitle2" ) ) // next arg is <subTitle2>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No subTitle2 supplied to -subTitle2 option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            strcpy ( subTitle2String_, argv[i] );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "subTitle2 == '%s'\n", subTitle2String_ );
#endif // DIAGNOSTICS
            }

        else if ( !strcasecmp ( p, "-unitString" ) ) // next arg is <unitString>
            {
            Formula *formula;

            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No unitString supplied to -unitString option!" );
                fprintf ( stderr, "%s\n", estring );
                }
            else if ( formula = getSelectedFormula ( formula_ ) )
                formula->setUnits ( argv[i] );
            else
                {
                Message error ( info_window_, XmDIALOG_ERROR,
                                "There is no current formula to set units for !" );
                }
            }

        else if ( !strcasecmp ( p, "-s" ) ) // next arg is <formula>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No formula supplied to -f option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
#ifdef DIAGNOSTICS
            fprintf ( stderr, "formula == '%s'\n", arg );
#endif // DIAGNOSTICS
            if ( formula_->addItem ( arg, formula_->getSelectionDialog() ) )
                {
                ans = 1;
                sprintf ( estring, "Failure adding formula '%s'!!", arg );
                }
            formula_->setCurrentSelection ( arg );
            formula_->verifyCurrentSelection();
            }

        else if ( !strcasecmp ( p, "-level" ) ) // next arg is <level>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No level supplied to -level option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            sscanf ( arg, "%d", &level );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "level == %d\n", level );
#endif // DIAGNOSTICS
            if ( level >= 0 )
                {
                nformulas = formulaList_.length();
                for ( j = 0; j < nformulas; j++ )
                    // need to loop like this since
                    // formula->set_selected_level()
                    // messes up the current list position
                    {
                    int thisFormulaNumber = 0;
                    formula = ( Formula * ) formulaList_.head();
                    while ( thisFormulaNumber < j )
                        {
                        formula = ( Formula * ) formulaList_.next();
                        thisFormulaNumber++;
                        }
                    if ( formula )
                        if ( formula->getFormulaName() )
                            if ( strlen ( formula->getFormulaName() ) >0 )
                                formula->set_selected_level ( level );
                    }
                }
            }



        else if ( !strcasecmp ( p, "-crossSectionType" ) ) // next arg is <X|Y|Z>
            {
            char type;

            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No cross section type supplied to -crossSectionType option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            type = arg[0];
#ifdef DIAGNOSTICS
            fprintf ( stderr, "crossSectionType == '%c'\n", type );
#endif // DIAGNOSTICS

            switch ( type )
                {
                case 'x':
                case 'X':
                    grp_type_Xcross_cb();
                    break;

                case 'y':
                case 'Y':
                    grp_type_Ycross_cb();
                    break;

                case 'z':
                case 'Z':
                    grp_type_Zcross_cb();
                    break;

                default:
                    fprintf ( stderr,
                              "Unknown crossSectionType arg == '%c', must be X, Y, or Z!\n", type );
                }
            }


        else if ( !strcasecmp ( p, "-preClip" ) ) // next args are <llLat> <llLon> <urLat> <urLon>
            {
            /*
            env strings are declared static as per the man pages
            for putenv's mention of a potential error:

            int putenv(char *string)

            A potential error is to  call  putenv()  with  an  automatic
            variable  as  the  argument,  then exit the calling function
            while string is still part of the environment.
            */
            static char lllatenv[64], lllonenv[64], urlatenv[64], urlonenv[64];
            float llLat, llLon, urLat, urLon;

            if ( i+4 > argc )
                {
                sprintf ( estring, "Not enough args supplied to -preClip option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i+1];
            sscanf ( arg, "%f", &llLat );
            arg = argv[i+2];
            sscanf ( arg, "%f", &llLon );
            arg = argv[i+3];
            sscanf ( arg, "%f", &urLat );
            arg = argv[i+4];
            sscanf ( arg, "%f", &urLon );
            i+=4;

#ifdef DIAGNOSTICS
            fprintf ( stderr, "preclip (llLat, llLon, urLat, urLon)==%g, %g, %g, %g\n",
                      llLat, llLon, urLat, urLon );
#endif // #ifdef DIAGNOSTICS

            sprintf ( lllatenv, "PRECLIP_LLLAT=%f", llLat );
            putenv ( lllatenv );
            sprintf ( lllonenv, "PRECLIP_LLLON=%f", llLon );
            putenv ( lllonenv );
            sprintf ( urlatenv, "PRECLIP_URLAT=%f", urLat );
            putenv ( urlatenv );
            sprintf ( urlonenv, "PRECLIP_URLON=%f", urLon );
            putenv ( urlonenv );
            }

        else if ( !strcasecmp ( p, "-levelRange" ) ) // next args are <levelMin, levelMax>
            {
            if ( i+2 > argc )
                {
                sprintf ( estring, "Not enough args supplied to -levelRange option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i+1];
            sscanf ( arg, "%d", &levelMin );
            arg = argv[i+2];
            sscanf ( arg, "%d", &levelMax );
            i+=2;

#ifdef DIAGNOSTICS
            fprintf ( stderr, "levelMin == %d, levelMax == %d\n", levelMin, levelMax );
#endif // DIAGNOSTICS
            if ( levelMin >= 0 && levelMax >= 0 )
                {
                nformulas = formulaList_.length();
                for ( j = 0; j < nformulas; j++ )
                    // need to loop like this since
                    // formula->set_selected_levelMin()
                    // messes up the current list position
                    {
                    int thisFormulaNumber = 0;
                    formula = ( Formula * ) formulaList_.head();
                    while ( thisFormulaNumber < j )
                        {
                        formula = ( Formula * ) formulaList_.next();
                        thisFormulaNumber++;
                        }
                    if ( formula )
                        if ( formula->getFormulaName() )
                            if ( strlen ( formula->getFormulaName() ) >0 )
                                formula->set_levelRange ( levelMin, levelMax );
                    }
                }
            }

        else if ( !strcasecmp ( p, "-saveImage" ) ) // next args are <imagetype, filename>
            {
            if ( i+2 > argc )
                {
                sprintf ( estring,
                          "Not enough args supplied to -saveImage option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            if ( !mostRecentTile_ )
                {
                sprintf ( estring, "No tile plot to save!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            if ( mostRecentTile_->dumpImage ( argv[i+1], argv[i+2], estring ) )
                {
                fprintf ( stderr, "\007%s\n", estring );
                ans = 1;
                }
            i+=2;
            }

        else if ( !strcasecmp ( p, "-animateWindows" ) ) // next arg is <single|continuous>
            {
            int looptype; // use 0 for continuous, 1 to do a single loop

            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No looptype (single or continuous) supplied to -configFile option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];

#ifdef DIAGNOSTICS
            fprintf ( stderr, "Looptype == '%s'\n", arg );
#endif // DIAGNOSTICS

            if ( !strcasecmp ( arg, "single" ) ) looptype = 1;
            else if ( !strcasecmp ( arg, "continuous" ) ) looptype = 0;
            else
                {
                sprintf ( estring,
                          "No looptype (single or continuous) supplied to -configFile option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            loopWindows ( looptype );
            }


        else if ( !strcasecmp ( p, "-configFile" ) ) // next arg is <filename>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No file name supplied to -configFile option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];

#ifdef DIAGNOSTICS
            fprintf ( stderr, "Config file == '%s'\n", arg );
#endif // DIAGNOSTICS

            load_config_cb ( arg );
            }


        else if ( !strcasecmp ( p, "-obs" ) ) // next arg is <formula>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No formula name supplied to -obs option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            if ( mostRecentTile_ )
                {
                XSync ( XtDisplay ( _w ), False );
                mostRecentTile_->set_overlay_mode ( OBS_PLOT );
                mostRecentTile_->overlay_create ( arg );
                }
            }
        else if ( !strcasecmp ( p, "-obsIdTable" ) ) // next arg is <filename>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No filename name supplied to -obsIdTable option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            addObsIdTable ( arg );
            }
        else if ( !strcasecmp ( p, "-obsTimeSeries" ) ) // no more args
            {
            getObsTimeSeries();
            }
        else if ( !strcasecmp ( p, "-vectobs" ) ) // next arg is <formula>
            {
            if ( i+2 > argc )
                {
                sprintf ( estring,
                          "2 formula name should be supplied to -vectobs option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            sprintf ( tstring,"%s %s", argv[i+1],argv[i+2] );
            if ( mostRecentTile_ )
                {
                XSync ( XtDisplay ( _w ), False );
                mostRecentTile_->set_overlay_mode ( OBSVECTOR_PLOT );
                mostRecentTile_->overlay_create ( tstring );
                }
            i+=2;
            }
        else if ( !strcasecmp ( p, "-obsSize" ) ) // next arg is <size>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No size supplied to -obsSize option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            if ( mostRecentTile_ )
                {
                int size = atoi ( arg );
                mostRecentTile_->obs_size_cb ( size );
                }
            }
        else if ( !strcasecmp ( p, "-obsThick" ) ) // next arg is <size>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No size supplied to -obsThick option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            if ( mostRecentTile_ )
                {
                int size = atoi ( arg );
                mostRecentTile_->obs_thick_cb ( size );
                }
            }
        else if ( ( !strcasecmp ( p, "-gtype" ) ) || // next arg is <graphics type>
                  ( !strcasecmp ( p, "-g" ) ) ) // for historical reasons -g works too
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No graphics type supplied to -gtype option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];

#ifdef DIAGNOSTICS
            fprintf ( stderr, "graphics type == '%s'\n", arg );
#endif // DIAGNOSTICS

            if (    ( strcasecmp ( arg, "tile" ) ) &&
                    ( strcasecmp ( arg, "line" ) ) &&
                    ( strcasecmp ( arg, "mesh" ) ) &&
                    ( strcasecmp ( arg, "bar" ) ) )
                {
                fprintf ( stderr,
                          "Ignoring unknown graphics type '%s' supplied to -gtype option!\n",
                          arg );
                }
            else
                {
                if ( !strcasecmp ( arg, "tile" ) )
                    {
                    if ( grp_plot_colortile_cb() )
                        {
                        ans = 1;
                        sprintf ( estring, "Tile plot failure!" );
                        }
                    XSync ( XtDisplay ( _w ), False );
                    }

                if ( !strcasecmp ( arg, "line" ) )
                    if ( grp_plot_linegraph_cb() )
                        {
                        ans = 1;
                        sprintf ( estring, "Line graph failure!" );
                        }

                if ( !strcasecmp ( arg, "mesh" ) )
                    if ( grp_plot_mesh_cb() )
                        {
                        ans = 1;
                        sprintf ( estring, "Mesh plot failure!" );
                        }

                if ( !strcasecmp ( arg, "bar" ) )
                    if ( grp_plot_bar_cb() )
                        {
                        ans = 1;
                        sprintf ( estring, "Bar plot failure!" );
                        }

                inputTitleString_[0] = '\0';
                subTitle1String_[0] = '\0';
                subTitle2String_[0] = '\0';
                }
            }

        else if ( !strcasecmp ( p, "-save2ascii" ) ) // next arg is <filename>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No filename supplied to -save2ascii option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            export2file ( arg, PAVE_EXPORT_TABBED );
            }
        else if ( !strcasecmp ( p, "-save2ncf" ) ) // next arg is <filename>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No filename supplied to -save2ncf option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            export2file ( arg, PAVE_EXPORT_NETCDF );
            }
        else if ( !strcasecmp ( p, "-multiVarNcf" ) ) // next args are <formulalist> <varnamelist> <filename>
            {
            i+=3;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "No <formulalist> <varnamelist> <filename> supplied to -multiVarNcf option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            multiVarNcf ( argv[i-2], argv[i-1], argv[i] );
            }
        else if ( !strcasecmp ( p, "-ts" ) ) // next arg is <time step>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring,"No time step supplied to -ts option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            sscanf ( arg, "%d", &ts );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "time step == %d\n", ts );
#endif // DIAGNOSTICS
            if ( ts >= 0 )
                {
                nformulas = formulaList_.length();
                for ( j = 0; j < nformulas; j++ )
                    // need to loop like this since
                    // formula->set_selected_level()
                    // messes up the current list position
                    {
                    int thisFormulaNumber = 0;
                    formula = ( Formula * ) formulaList_.head();
                    while ( thisFormulaNumber < j )
                        {
                        formula = ( Formula * ) formulaList_.next();
                        thisFormulaNumber++;
                        }
                    if ( formula )
                        if ( formula->getFormulaName() )
                            if ( strlen ( formula->getFormulaName() ) >0 )
                                formula->set_selected_step ( ts );
                    }
                }
            }

        else if ( !strcasecmp ( p,"-fulldomain" ) )
            {
            sdomain = 0;
            fdomain = 1;

#ifdef DIAGNOSTICS
            fprintf ( stderr, "Using full domain\n" );
#endif // DIAGNOSTICS
            strcpy ( tstring, ( ( CaseServer * ) case_ )->getCurrSelection() );

            if ( dset = ( dataSet * ) ( datasetList_.find ( tstring ) ) )
                {
                Domain *domain;
                int ni, nj;
                int *target[3];

                ni = dset->get_ncol();
                nj = dset->get_nrow();
                target[0] = &ni;
                target[1] = &nj;;
                target[2] = ( int * ) dset->getMapInfo();

                if ( domain = ( Domain * ) ( domainList_.find ( target ) ) )
                    domain->setRange ( 1, 1, ni, nj );
                else
                    fprintf ( stderr,
                              "Didn't find Domain %dx%d %s on DomainList!\n",
                              ni, nj, dset->getMapInfo() );
                }
            else
                fprintf ( stderr, "Didn't find '%s' on dataSetList!\n",
                          tstring );
            }

        else if ( !strcasecmp ( p, "-contourRange" ) ) //next args are <minCut> <maxCut>
            {
            if ( i+2 >= argc )
                {
                sprintf ( estring,
                          "Not enough args supplied to -contourRange option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            contourRange_ = 1;
            sscanf ( argv[i+1], "%f", &minCut_ );
            sscanf ( argv[i+2], "%f", &maxCut_ );
            i+=2;
#ifdef DIAGNOSTICS
            fprintf ( stderr, "Using contourRange %f to %f\n",
                      minCut_, maxCut_ );
#endif // DIAGNOSTICS
            }

        else if ( !strcasecmp ( p, "-autoContourRange" ) )
            {
            contourRange_ = 0;
#ifdef DIAGNOSTICS
            fprintf ( stderr, "Using autoContourRange\n" );
#endif // DIAGNOSTICS
            }

        else if ( !strcasecmp ( p,"-subdomain" ) ) //next args are <xmin> <ymin> <xmax> <ymax>
            {
            sdomain = 1;
            fdomain = 0;
            if ( i+4 >= argc )
                {
                sprintf ( estring,
                          "Not enough args supplied to -subdomain option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            sscanf ( argv[i+1], "%d", &xmin );
            sscanf ( argv[i+2], "%d", &ymin );
            sscanf ( argv[i+3], "%d", &xmax );
            sscanf ( argv[i+4], "%d", &ymax );
            i+=4;

#ifdef DIAGNOSTICS
            fprintf ( stderr, "subdomain range (%d,%d)->(%d,%d)\n",
                      xmin, ymin, xmax, ymax );
#endif // DIAGNOSTICS

            strcpy ( tstring, ( ( CaseServer * ) case_ )->getCurrSelection() );

            if ( dset = ( dataSet * ) ( datasetList_.find ( tstring ) ) )
                {
                Domain *domain;
                int ni, nj;
                int *target[3];

                ni = dset->get_ncol();
                nj = dset->get_nrow();
                target[0] = &ni;
                target[1] = &nj;;
                target[2] = ( int * ) dset->getMapInfo();

                if ( domain = ( Domain * ) ( domainList_.find ( target ) ) )
                    domain->setRange ( xmin, ymin, xmax, ymax );
                else
                    fprintf ( stderr,
                              "Didn't find Domain %dx%d %s on DomainList!\n",
                              ni, nj, dset->getMapInfo() );
                }
            else
                fprintf ( stderr, "Didn't find '%s' on dataSetList!\n",
                          tstring );
            }

        else if ( !strcasecmp ( p, "-tinit" ) ) // next arg is <initial time step>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No initial time step supplied to -tinit option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            sscanf ( arg, "%d", &ti );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "initial time step == %d\n", ti );
#endif // DIAGNOSTICS

            if ( ti >= 0 )
                {
                nformulas = formulaList_.length();
                for ( j = 0; j < nformulas; j++ )
                    // need to loop like this since
                    // formula->set_selected_level()
                    // messes up the current list position
                    {
                    int thisFormulaNumber = 0;
                    formula = ( Formula * ) formulaList_.head();
                    while ( thisFormulaNumber < j )
                        {
                        formula = ( Formula * ) formulaList_.next();
                        thisFormulaNumber++;
                        }
                    if ( formula )
                        if ( formula->getFormulaName() )
                            if ( strlen ( formula->getFormulaName() ) >0 )
                                formula->set_hr_min ( ti );
                    }
                }
            }

        else if ( !strcasecmp ( p, "-height" ) ) // next arg is <tile plot height in pixels>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No height supplied to -height option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            sscanf ( arg, "%d", &height_ );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "height == %d\n", height_ );
#endif // DIAGNOSTICS
            }

        else if ( !strcasecmp ( p, "-width" ) ) // next arg is <tile plot width in pixels>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No width supplied to -width option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            sscanf ( arg, "%d", &width_ );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "width_ == %d\n", width_ );
#endif // DIAGNOSTICS
            }

        else if ( !strcasecmp ( p, "-tfinal" ) ) // next arg is <final time step>
            {
            i++;
            if ( i == argc )
                {
                sprintf ( estring,
                          "No final time step supplied to -tfinal option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            sscanf ( arg, "%d", &tf );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "final time step == %d\n", tf );
#endif // DIAGNOSTICS
            if ( tf >= 0 )
                {
                nformulas = formulaList_.length();
                for ( j = 0; j < nformulas; j++ )
                    // need to loop like this since
                    // formula->set_selected_level()
                    // messes up the current list position
                    {
                    int thisFormulaNumber = 0;
                    formula = ( Formula * ) formulaList_.head();
                    while ( thisFormulaNumber < j )
                        {
                        formula = ( Formula * ) formulaList_.next();
                        thisFormulaNumber++;
                        }
                    if ( formula )
                        if ( formula->getFormulaName() )
                            if ( strlen ( formula->getFormulaName() ) >0 )
                                formula->set_hr_max ( tf );
                    }
                }
            }

        else if ( ( !strcasecmp ( argv[i], "-?"         ) ) ||
                  ( !strcasecmp ( argv[i], "--?"        ) ) ||
                  ( !strcasecmp ( argv[i], "--help"     ) ) ||
                  ( !strcasecmp ( argv[i], "--fullhelp" ) ) ||
                  ( !strcasecmp ( argv[i], "--usage"    ) ) ||
                  ( !strcasecmp ( argv[i], "-help"      ) ) ||
                  ( !strcasecmp ( argv[i], "-fullhelp"  ) ) ||
                  ( !strcasecmp ( argv[i], "-usage"     ) ) ||
                  ( !strcasecmp ( argv[i], "help"       ) ) ||
                  ( !strcasecmp ( argv[i], "fullhelp"   ) ) ||
                  ( !strcasecmp ( argv[i], "usage"      ) ) )
            {
            pave_usage();
            }

        else if ( !strcasecmp ( p, "-version" ) )
            pave_version();

        else if ( !strcasecmp ( p, "-windowid" ) )
            {
            extern Widget mostRecentlyAddedWindowsWidget;
            if ( mostRecentlyAddedWindowsWidget==NULL )
                {
                sprintf ( estring, "No window is active!" );
                if ( ( getenv ( "KEDAMODE" ) != NULL ) &&
                        ( !strcmp ( getenv ( "KEDAMODE" ), "1" ) ) )
                    {
                    fprintf ( stderr, "KEDA_FAILURE: %s\n", estring );
                    }
                else
                    {
                    fprintf ( stderr, "ERROR: %s\n", estring );
                    }
                }
            else
                {
                printf ( "%lu\n", XtWindow ( mostRecentlyAddedWindowsWidget ) );
                }
            }

        else if ( !strcasecmp ( p, "-closeWindow" ) ) // next arg is windowId
            {
            long windowid;

            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No windowid supplied to -closeWindow option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            sscanf ( arg, "%ld", &windowid );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "windowid == %ld\n", windowid );
#endif // DIAGNOSTICS
            closeWnd ( windowid );
            }

        else if ( !strcasecmp ( p, "-raiseWindow" ) ) // next arg is windowId
            {
            long windowid;

            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No windowid supplied to -raiseWindow option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            sscanf ( arg, "%ld", &windowid );
#ifdef DIAGNOSTICS
            fprintf ( stderr, "windowid == %ld\n", windowid );
#endif // DIAGNOSTICS
            raiseWnd ( windowid );
            }

        else if ( !strcasecmp ( p, "-showWindow" ) ) // next 2 args are windowId and timestep
            {
            long windowid;
            int timestep;

            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No windowid supplied to -showWindow option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            sscanf ( arg, "%ld", &windowid );
            i++;
            if ( i == argc )
                {
                sprintf ( estring, "No timestep supplied to -showWindow option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            arg = argv[i];
            sscanf ( arg, "%d", &timestep );
            ans = animateWnd ( windowid, timestep );
            }

        else if ( !strcasecmp ( p, "-copyright" ) )
            print_copyright_file();

        else if (    ( !strcasecmp ( p, "-quit" ) ) ||
                     ( !strcasecmp ( p, "-exit" ) ) )
            {
            exitCB ( ( Widget ) NULL , ( XtPointer ) this, ( XtPointer ) NULL );
            }

        else if ( !strcasecmp ( p, "-scatter" ) ) // next 2 args are formulas to be
            // scatter plotted
            {
            i+=2;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need 2 formulas for -scatter option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            if ( createScatterPlot ( argv[i-1], argv[i] ) )
                {
                ans = 1;
                sprintf ( estring, "Create scatter plot failure!" );
                }
            }

        else if ( !strcasecmp ( p, "-multitime" ) ) // next few args are formulas to be
            // plotted as time series on same plot
            {
            int nformulas;

            i+=1;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need <nformulas> and formulas for -multitime option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            sscanf ( argv[i], "%d", &nformulas );
            i+=nformulas;
            if ( ( i >= argc ) || ( nformulas <= 0 ) || ( nformulas > 8 ) )
                {
                sprintf ( estring,
                          "1 to 8 formulas required for -multitime option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            if ( createMultiTimePlot ( nformulas, argv+i-nformulas+1 ) )
                {
                ans = 1;
                sprintf ( estring, "Create multi time plot failure!" );
                }
            }

        else if ( !strcasecmp ( p, "-vectorTile" ) ) // next 3 args are formula to be
            // plotted and U and V vector flds
            {
            i+=3;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need 3 formulas for -vectorTile option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            if ( createTileVectorPlot ( argv[i-2], argv[i-1], argv[i] ) )
                {
                ans = 1;
                sprintf ( estring, "Create tile vector plot failure!" );
                }
            }

        else if ( !strcasecmp ( p, "-vector" ) ) // next 2 args U and V vector flds
            {
            i+=2;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need 2 formulas for -vector option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            if ( createTileVectorPlot ( NULL, argv[i-1], argv[i] ) )
                {
                ans = 1;
                sprintf ( estring, "Create tile vector plot failure!" );
                }
            }

        else if ( !strcasecmp ( p, "-vectorObs" ) ) // next 4 args U and V vector flds
            {
            i+=4;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need 4 formulas for -vectorObs option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            if ( createTileVectorObsPlot ( argv[i-3], argv[i-2],
                                           argv[i-1], argv[i] ) )
                {
                ans = 1;
                sprintf ( estring, "Create tile vectorObs plot failure!" );
                }
            }

        else if ( !strcasecmp ( p, "-vectorScale" ) ) // next arg is scale factor
            {
            i++;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need 1 formulas for -vectorScale option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            sscanf ( argv[i],"%f",&default_vector_scale_ );
            }
        else if ( !strcasecmp ( p, "-vectorPlotEvery" ) ) // next arg is scale factor
            {
            i++;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need 1 formulas for -vectorSkip option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            sscanf ( argv[i],"%d",&default_vector_skip_ );
            }
        else if ( !strcasecmp ( p, "-DesignValueObs" ) ) // next args are R C dn form1 form2
            {
            int row, col, delta_n;
            i+=6;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need <col> <row> <Radius> and 3 formulas for -MinMaxOpt option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            sscanf ( argv[i-5],"%d",&col );
            sscanf ( argv[i-4],"%d",&row );
            sscanf ( argv[i-3],"%d",&delta_n );
            createComp ( row, col, delta_n, argv[i-2],argv[i-1],argv[i] );
            }
        else if ( !strcasecmp ( p, "-MinMaxObs" ) ) // next args are R C dn form1 form2
            {
            int row, col, delta_n;
            i+=5;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need <col> <row> <Radius> and 2 formulas for -MinMaxOpt option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            sscanf ( argv[i-4],"%d",&col );
            sscanf ( argv[i-3],"%d",&row );
            sscanf ( argv[i-2],"%d",&delta_n );
            createComp ( row, col, delta_n, 3, argv[i-1],argv[i] );
            }
        else if ( !strcasecmp ( p, "-MinMaxModelObs" ) ) // next args are R C dn form1 form2
            {
            int row, col, delta_n;
            i+=5;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need <col> <row> <Radius> and 2 formulas for -MinMaxOpt option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            sscanf ( argv[i-4],"%d",&col );
            sscanf ( argv[i-3],"%d",&row );
            sscanf ( argv[i-2],"%d",&delta_n );
            createComp ( row, col, delta_n, 4, argv[i-1],argv[i] );
            }
        else if ( !strcasecmp ( p, "-NhourSum" ) )
            {
            int nhr;
            i+=1;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need <hnours> -NhourSum option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            nhr = atoi ( argv[i] );
            grp_plot_nhour_avg ( nhr,0 );

            }
        else if ( !strcasecmp ( p, "-NhourAverage" ) )
            {
            int nhr;
            i+=1;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need <hnours> -NhourAverage option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            nhr = atoi ( argv[i] );
            grp_plot_nhour_avg ( nhr,1 );

            }
        else if ( !strcasecmp ( p, "-NlayerSum" ) )
            {
            grp_plot_nlayer_avg ( 0 );
            }
        else if ( !strcasecmp ( p, "-NlayerAverage" ) )
            {
            grp_plot_nlayer_avg ( 1 );
            }
        else if ( !strcasecmp ( p, "-tzoffset" ) )
            {
            int tzoff;
            i+=1;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need <offset> -tzoffset option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            tzoff = atoi ( argv[i] );
            tzOffset ( tzoff );
            }
        else if ( !strcasecmp ( p, "-tzset" ) )
            {
            i+=2;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need <in timezone> <out timezone> -tzset option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            tzSet ( argv[i-1],argv[i] );
            }
        else if ( !strcasecmp ( p, "-animatedGIF" ) )
            {
            i+=1;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need <filename> -animatedGIF option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }
            animatedGIF ( argv[i] );
            }
        else if ( !strcasecmp ( p, "-save2d" ) )
            {
            Window wid;
            Display *display;
            int writeImage ( Display *, Window, char *, char *, char * );

            i+=2;
            if ( i >= argc )
                {
                sprintf ( estring,
                          "Need Imagetype and filename for -save2d option!" );
                fprintf ( stderr, "%s\n", estring );
                return 1;
                }

            display = XtDisplay ( _w );
            XSync ( display, False );
            wid = getMostRecentGraphWindow();
            if ( wid > 0 )
                {
                XSync ( display, False );
                //sleep(5);
                writeImage ( display, wid,
                             argv[i-1], argv[i], estring );

                XSync ( display, False );
                }
            else
                {
                sprintf ( estring,"%s",
                          "ERROR: no graph windowId is available. Cannot save graph image. Check your BLT config file." );
                fprintf ( stderr,"%s\n", estring );
                ans = 1;
                }
            }
        else
            {
            fprintf ( stderr, "Ignoring unknown argument '%s'...\n", p );
            ans = 1;
            }

        i++;
        }

    return ans;
    }



char *get_copyright_filename ( void )
    {
    static char filename[256];
    char *dirname;

    dirname = getenv ( "PAVE_DIR" ) ;
    if ( dirname && *dirname )
        {
        sprintf ( filename, "%s/COPYRIGHT", dirname );
        return filename;
        }
    fprintf ( stderr, "\007Can't find $PAVE_DIR in get_copyright_filename()!\n" );
    return NULL;
    }



/*
void verify_copyright_file(void)
{
FILE *fp;
long minimum_filesize = 60000;

if (!get_copyright_filename())
    {
    fprintf(stderr, "\007Can't find $PAVE_DIR/COPYRIGHT!  Quitting...\n");
    exit(1);
    }

if (!(fp = fopen(get_copyright_filename(), "r")))
    {
    fprintf(stderr, "\007Can't open %s!\nQuitting...\n", get_copyright_filename());
    exit(1);
    }

if (fseek(fp, 0, SEEK_END))
    {
    fprintf(stderr, "\007Can't fseek in %s!\nQuitting...\n",get_copyright_filename());
    fclose(fp);
    exit(1);
    }

if (ftell(fp) < minimum_filesize)
    {
    fprintf(stderr, "\007Invalid %s!\nPlease obtain valid one from MCNC.\n"
            "Quitting...\n", get_copyright_filename());
    fclose(fp);
    exit(1);
    }

fclose(fp);
}
*/


void print_copyright_file ( void )
    {
    char command[256];

    if ( !get_copyright_filename() )
        {
        fprintf ( stderr, "\007Can't print COPYRIGHT.pave file!\n" );
        return;
        }

    sprintf ( command, "echo more %s; more %s",
              get_copyright_filename(), get_copyright_filename() );
    system ( command );
    }



void pave_usage ( void )
    {
    fprintf ( stdout, "usage: pave_exe\n       "
              "[ -alias <aliasname=definition> ]               \n       "
              "[ -animateWindows <single|continuous> ]         \n       "
              "[ -animatedGIF <filename> ]                     \n       "
              "[ -autoContourRange ]                           \n       "
              "[ -barplotYformat \"<format string>\" ]         \n       "
              "[ -closeWindow <windowid> ]                     \n       "
              "[ -configFile <configFileName> ]                \n       "
              "[ -contourRange <minCut> <maxCut> ]             \n       "
              "[ -copyright ]                                  \n       "
              "[ -crossSectionType X|Y|Z ]                     \n       "
              "[ -display <display> ]                          \n       "
              "[ -drawGridLabels ON|OFF\" ] (NEW!!!)           \n       "
              "[ -drawLegend ON|OFF\" ] (NEW!!!)               \n       "
              "[ -drawMinMax ON|OFF\" ] (NEW!!!)               \n       "
              "[ -drawTiles  ON|OFF\" ] (NEW!!!)               \n       "
              "[ -drawTimeStamp ON|OFF\" ] (NEW!!!)            \n       "
              "[ -f [<host>:]<filename> ]                      \n       "
              "[ -fulldomain ]                                 \n       "
              "[ -g <tile|line|mesh|bar> ]                     \n       "
              "[ -gtype <tile|line|mesh|bar> ]                 \n       "
              "[ -height <tile plot height in pixels> ]        \n       "
              "[ -help|fullhelp|usage ]                        \n       "
              "[ -imageMagickArgs 'args' ] (NEW!!!)             \n       "
              "[ -kedamode ]                                   \n       "
              "[ -legendBins \"<bin0,bin1,...,bin_n>\" ]       \n       "
              "[ -level <level> ]                              \n       "
              "[ -levelRange <levelMax> <levelMin> ]           \n       "
              "[ -mapCounties ]                                \n       "
              "[ -mapName \"<pathname>/<mapFileName>\" ]       \n       "
              "[ -minMaxModelObs <col> <row> <radius> <formula1> <formula2> ] \n       "
              "[ -minMaxObs <col> <row> <radius> <formula1> <formula2> ] \n       "
              "[ -multiVarNcf <formulaList> <varList> <fileName>\" ] \n       "
              "[ -multitime <Nformulas> \"<formula1>\" ... \"<formulaN>\" ] \n       "
              "[ -nHourAverage <nhours> ]                      \n       "
              "[ -nHourSum <nhours> ]                          \n       "
              "[ -nLayerAverage ]                              \n       "
              "[ -nLayerSum ]                                  \n       "
              "[ -obs <formula> ]                              \n       "
              "[ -obsIdTable <filename> ]                      \n       "
              "[ -obsSize <size> ]                             \n       "
              "[ -obsThick<size> ]                             \n       "
              "[ -obsTimeSeries  ]                             \n       "
              "[ -onlyDrawLegend ON|OFF\" ] (NEW!!!)           \n       "
              "[ -preClip <llLat> <llLon> <urLat> <urLon> ]    \n       "
              "[ -printAlias ]                                 \n       "
              "[ -quit|exit ]                                  \n       "
              "[ -raiseWindow <windowid> ]                     \n       "
              "[ -s \"<formula>\" ]                            \n       "
              "[ -save2ascii <filename> ]                      \n       "
              "[ -save2d <imagetype> <filename> ]              \n       "
              "[ -save2ncf <filename> ]                        \n       "
              "[ -saveImage \"<image type>\" <file name> ]     \n       "
              "[ -scatter \"<formula1>\" \"<formula2>\" ]      \n       "
              "[ -showWindow <windowId> <timestep> ]           \n       "
              "[ -subDomain <xmin> <ymin> <xmax> <ymax> ]      \n       "
              "[ -subTitle1 \"<sub title 1 string>\" ]         \n       "
              "[ -subTitle2 \"<sub title 2 string>\" ]         \n       "
              "[ -subTitleFont <fontSize> ]                    \n       "
              "[ -system \"<unix command>\" ]                  \n       "
              "[ -tfinal <final time step> ]                   \n       "
              "[ -tileYlabelsOnRight ]                         \n       "
              "[ -tinit <initial time step> ]                  \n       "
              "[ -titleFont <fontSize> ]                       \n       "
              "[ -titleString \"<title string>\" ]             \n       "
              "[ -ts <time step> ]                             \n       "
              "[ -tzoffset <Timezone offset> ]                 \n       "
              "[ -tzset <in Timezone> <out Timezone> ]         \n       "
              "[ -unalias <aliasname> ]                        \n       "
              "[ -unitString \"<unit string>\" ]               \n       "
              "[ -vectobs <formula> <formula> ]                \n       "
              "[ -vector \"<U>\" \"<V>\"]                      \n       "
              "[ -vectorPlotEvery \"<number>\"]                \n       "
              "[ -vectorScale \"<scale factor>\"]              \n       "
              "[ -vectorTile \"<formula>\" \"<U>\" \"<V>\"]    \n       "
              "[ -version ]                                    \n       "
              "[ -width <tile plot width in pixels> ]          \n       "
              "[ -windowid ]                                   \n" );
    }


void DriverWnd::minFrameTime_dialogCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->minFrameTime_dialog_cb();
    }

void DriverWnd::minFrameTime_dialog_cb()
    {
    if ( minFrameTime_dialog_ == NULL )
        createMinFrameTime_dialog ( _w );
    if ( XtIsManaged ( minFrameTime_dialog_ ) ) XtUnmanageChild ( minFrameTime_dialog_ );
    XtManageChild ( minFrameTime_dialog_ );
    }

void DriverWnd::createMinFrameTime_dialog ( Widget parent )
    {
    Position xpos, ypos;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "In createMinFrameTime_dialog 333\n" );
#endif // DIAGNOSTICS

    assert ( parent );

    // Create control dialog box.
    minFrameTime_dialog_ = XmCreateFormDialog ( parent, "control", NULL, 0 );

    // Position control dialog box so it doesn't obscure the plot
    XtVaGetValues ( parent, XmNx, &xpos, XmNy, &ypos, NULL );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "TileWnd::createMinFrameTime_dialog's parent's xpos=%d, ypos=%d\n",
              ( int ) xpos, ( int ) ypos );
#endif // DIAGNOSTICS
    XtVaSetValues ( minFrameTime_dialog_,
                    XmNautoUnmanage,        False,
                    XmNdefaultPosition,     False,   // so won't be centered - SRT
                    XtNx,                   xpos+35, // SRT 950911
                    XtNy,                   ypos-90, // SRT 950911
                    NULL );

    Widget form1 = XtVaCreateWidget ( "form1",
                                      xmFormWidgetClass,      minFrameTime_dialog_,
                                      XmNtopAttachment,       XmATTACH_FORM,
                                      XmNtopOffset,           10,
                                      XmNleftAttachment,      XmATTACH_FORM,
                                      XmNleftOffset,          10,
                                      XmNrightAttachment,     XmATTACH_FORM,
                                      XmNrightOffset,         10,
                                      NULL );

    delay_scale_ = XtVaCreateManagedWidget ( "Min Tenths of Secs Per Frame",
                   xmScaleWidgetClass,     form1,

                   XtVaTypedArg, XmNtitleString,
                   XmRString, "Min Tenths of Secs Per Frame", 29,

                   XmNheight,              100,
                   XmNmaximum,             50,
                   XmNminimum,             0,
                   XmNvalue,               frameDelayInTenthsOfSeconds_,
                   XmNshowValue,           True,
                   XmNorientation,         XmHORIZONTAL,
                   XmNtopAttachment,       XmATTACH_FORM,
                   XmNleftAttachment,      XmATTACH_FORM,
                   XmNrightAttachment,     XmATTACH_FORM,
                   NULL );

    XtAddCallback ( delay_scale_, XmNvalueChangedCallback,  ( void ( * ) ( _WidgetRec *, void *, void * ) ) &DriverWnd::delay_scaleCB, ( XtPointer ) this );


    if ( XtIsManaged ( form1 ) ) XtUnmanageChild ( form1 );
    XtManageChild ( form1 );

    Widget separator = XtVaCreateManagedWidget ( "sep",
                       xmSeparatorWidgetClass, minFrameTime_dialog_,
                       XmNtopAttachment,       XmATTACH_WIDGET,
                       XmNtopWidget,           form1,
                       XmNtopOffset,           10,
                       XmNleftAttachment,      XmATTACH_FORM,
                       XmNrightAttachment,     XmATTACH_FORM,
                       NULL );


    // Button to close dialog box .
    delay_close_ = XtVaCreateManagedWidget ( "Close",
                   xmPushButtonWidgetClass, minFrameTime_dialog_,
                   XmNtopAttachment,       XmATTACH_WIDGET,
                   XmNtopWidget,           separator,
                   XmNtopOffset,           10,
                   XmNleftAttachment,      XmATTACH_FORM,
                   XmNleftOffset,          10,
                   XmNbottomAttachment,    XmATTACH_FORM,
                   XmNbottomOffset,        10,
                   XmNwidth,               100,
                   XmNheight,              40,
                   NULL );
    XtAddCallback ( delay_close_, XmNactivateCallback, &DriverWnd::delay_closeCB, ( XtPointer ) this );


#ifdef DIAGNOSTICS
    fprintf ( stderr, "Leaving createMinFrameTime_dialog\n" );
#endif // DIAGNOSTICS
    }


void DriverWnd::delay_scaleCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    XmScaleCallbackStruct * cbs = ( XmScaleCallbackStruct * ) callData;
    obj->delay_scale_cb ( cbs->value );
    }

void DriverWnd::delay_scale_cb ( int value )
    {
    frameDelayInTenthsOfSeconds_ = value;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "setting delay to %d tenths of a second\n",
              frameDelayInTenthsOfSeconds_ );
#endif // DIAGNOSTICS
    }


void DriverWnd::delay_closeCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->delay_close_cb();
    }

void DriverWnd::delay_close_cb()
    {
    XtUnmanageChild ( minFrameTime_dialog_ );
    }


// calculates the width and height for a TileWnd
int DriverWnd::calcWidthHeight ( int *w, int *h, VIS_DATA *vdata )
    {
    if ( ( !w ) || ( !h ) || ( !vdata ) )
        return 1;

    *h = 450; // SRT 950721 height (100=top 60=bottom 290=tiles)

#ifdef DIAGNOSTICS
    fprintf ( stderr, "\nEnter DriverWnd::calcWidthHeight() with vdata->slice == " );
    switch ( vdata->slice )
        {
        case XYSLICE:
            fprintf ( stderr, "XY\n" );
            break;
        case XYTSLICE:
            fprintf ( stderr, "XYT\n" );
            break;
        case XZSLICE:
            fprintf ( stderr, "XZ\n" );
            break;
        case XZTSLICE:
            fprintf ( stderr, "XZT\n" );
            break;
        case YZSLICE:
            fprintf ( stderr, "YZ\n" );
            break;
        case YZTSLICE:
            fprintf ( stderr, "YZT\n" );
            break;
        default:
            fprintf ( stderr, "default\n" );
            break;
        }
    fprintf ( stderr, "vdata->col_min == %d\n", vdata->col_min );
    fprintf ( stderr, "vdata->col_max == %d\n", vdata->col_max );
    fprintf ( stderr, "vdata->ncol == %d\n", vdata->ncol );
    fprintf ( stderr, "vdata->row_min == %d\n", vdata->row_min );
    fprintf ( stderr, "vdata->row_max == %d\n", vdata->row_max );
    fprintf ( stderr, "vdata->nrow == %d\n", vdata->nrow );
    fprintf ( stderr, "vdata->level_min == %d\n", vdata->level_min );
    fprintf ( stderr, "vdata->level_max == %d\n", vdata->level_max );
    fprintf ( stderr, "vdata->nlevel == %d\n", vdata->nlevel );
#endif // DIAGNOSTICS

    switch ( vdata->slice )
        {
        case XYSLICE:
        case XYTSLICE:
            *w = ( int ) ( 160+ // 100=left + 60=right
                           ( vdata->col_max-vdata->col_min+1 ) *
                           ( 290.0/ ( vdata->row_max-vdata->row_min+1 ) ) );
            //           (0.91*290.0/(vdata->row_max-vdata->row_min+1)));
            // SRT 0.91 takes into account the non-square pixels on monitor
            break;

        case XZSLICE:
        case XZTSLICE:
        case YZSLICE:
        case YZTSLICE:
        default:
            *w = ( int ) ( 160+290.0 );
            //      *w = (int) (160+0.91*290.0);
            break;
        }

    if ( width_ > 0 ) *w = width_;
    if ( height_ > 0 ) *h = height_;
    return 0;
    }




int DriverWnd::createMultiTimePlot ( int nformulas, char **formulas )
    {
    char formulaname[8][256],
         **legend = formulas, statusMsg[512],
           *symbol[] = { "Square","Circle","Diamond","Plus","Cross",
                         "Square","Circle","Diamond"
                       },
                       *color[] = { "black", "red", "green", "blue", "yellow",
                                    "green", "black", "red"
                                  };
    int i, j, nlines=nformulas, npoints[8], hrMin, totnpoints = 0, index;
    float *tsdata[8], *x, *y;
    Formula *formula[8];
    char title[128];

    // Miscellaneous setup junk
    stop_cb();
    tsdata[0] = tsdata[1] = tsdata[2] = tsdata[3] = tsdata[4] = ( float * ) NULL;
    updateStatus ( "Displaying time series multi line plot..." );

    // do a little error checking
    if ( !formulas )
        {
        sprintf ( statusMsg, "NULL char **formulas in createMultiTimePlot()!\n" );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return 1;
        }
    if ( ( nformulas < 1 ) || ( nformulas > 8 ) )
        {
        sprintf ( statusMsg, "Invalid nformulas==%d in createMultiTimePlot()!\n",
                  nformulas );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return 1;
        }

    // grab the data for the formulas
    for ( i = 0; i < nformulas; i++ )
        {
        if ( !formulas[i] )
            {
            sprintf ( statusMsg, "Empty formula in createMultiTimePlot()!" );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            for ( j = 0; j < i; j++ ) if ( tsdata[j] ) free ( tsdata[j] );
            return 1;
            }

        // Find this formula's Formula object
        strcpy ( formulaname[i], formulas[i] );
        if ( ! ( formula[i] = ( Formula * ) ( formulaList_.find ( formulaname[i] ) ) ) )
            {
            sprintf ( statusMsg, "Didn't find '%s'\non the formulaList!\n",
                      formulaname[i] );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            for ( j = 0; j < i; j++ ) if ( tsdata[j] ) free ( tsdata[j] );
            return 1;
            }

        // Retrieve a copy of that Formula object's time series data
        if ( ! ( tsdata[i] = formula[i]->get_time_series_data ( statusMsg ) ) )
            {
            if ( strstr ( statusMsg, "==" ) )
                displaySingleNumberFormula ( statusMsg, formula[i] );
            else
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                }
            for ( j = 0; j < i; j++ ) if ( tsdata[j] )
                    {
                    free ( tsdata[j] );
                    tsdata[j] = NULL;
                    }
            return 1;
            }

        // how many time steps in this formula?
        // also do they all start with the same hrMin?
        // (if not then we'll just use 1 for the first index)
        npoints[i] = formula[i]->get_nsteps();
        if ( i == 0 )
            hrMin = formula[i]->get_hrMin();
        else if ( hrMin != ( -1 ) )
            if ( formula[i]->get_hrMin() != hrMin )
                hrMin = -1;
        totnpoints += npoints[i];
        }

    // allocate space for the points to be passed to graph2d()
    //   if (hrMin == -1) hrMin = 1; else hrMin++; // make 1 based not 0 based
    if ( hrMin == -1 ) hrMin = 0;
    x = new float[totnpoints];
    y = new float[totnpoints];
    if ( ( !x ) || ( !y ) )
        {
        if ( x ) delete [] x;
        x = NULL;
        if ( y ) delete [] y;
        y = NULL;
        for ( j = 0; j < i; j++ ) if ( tsdata[j] )
                {
                free ( tsdata[j] );
                tsdata[j]=NULL;
                }
        Message error ( info_window_, XmDIALOG_ERROR,
                        "Couldn't allocate memory in DriverWnd::createMultiTimePlot()!" );
        return 1;
        }

    // now fill up the points to be passed to graph2d()
    index = 0;
    for ( i = 0; i < nformulas; i++ )
        for ( j = 0; j < npoints[i]; j++ )
            {
            x[index] = ( float ) hrMin+j;
            y[index] = tsdata[i][j];
            index++;
            }

    // free up the tsdata points
    for ( j = 0; j < nformulas; j++ )
        {
        free ( tsdata[j] );
        tsdata[j]=NULL;
        }

    if ( ( inputTitleString_ != NULL ) && ( inputTitleString_[0]!='\0' ) )
        {
        strcpy ( title, inputTitleString_ );
        }
    else
        {
        strcpy ( title, "Time Series" );
        }
    // now create the plot !!
    if ( !graph2d ( x, y, nlines, npoints,
                    title, "Time Step", "Average Value",
                    legend, symbol, color, statusMsg, 1 ) )
        {
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return 1;
        }

    delete [] x;
    x = NULL;
    delete [] y;
    y = NULL;
    return 0;
    }



void DriverWnd::netCDFExportCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->netCDF_export_cb();
    }


void DriverWnd::netCDF_export_cb()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter DriverWnd::netCDFExportCB()\n" );
#endif // DIAGNOSTICS

    if ( formula_->getCurrSelection() )
        {
        char formulaname[512], statusMsg[512];
        Formula *formula;
        /* SRT 950717 struct */ VIS_DATA *vdata = NULL;

        // Miscellaneous setup junk
        stop_cb();
        strcpy ( formulaname, formula_->getCurrSelection() );
        updateStatus ( "Exporting netCDF data file..." );

        // Find the currently selected formula's Formula object
        if ( ! ( formula = ( Formula * ) ( formulaList_.find ( formulaname ) ) ) )
            {
            sprintf ( statusMsg, "Didn't find '%s'\non the formulaList!\n", formulaname );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return;
            }

        // Retrieve a copy of that Formula object's data
        if ( ! ( vdata = get_VIS_DATA_struct ( formula,statusMsg,XYZTSLICE ) ) )
            {
            if ( strstr ( statusMsg, "==" ) )
                displaySingleNumberFormula ( statusMsg, formula );
            else
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                }
            return;
            }

        exportnetCDF_UI_->ShowUI ( ( void * ) vdata );
        }
    else
        {
        Message error ( info_window_, XmDIALOG_ERROR, "There is no formula currently selected!" );
        }

    }




void DriverWnd::free_up_every_formulas_data() // added 960920 SRT for memory management
    {
    Formula *formula = ( Formula * ) formulaList_.head();
    while ( formula )
        {
        formula->invalidateThisData();
        formula = ( Formula * ) formulaList_.next();
        }
    }



VIS_DATA *DriverWnd::get_VIS_DATA_struct ( Formula *f, char *estring, int slice_type ) // added 960920 SRT for memory management
    {
    VIS_DATA *ans;

    if ( ( !f ) || ( !estring ) )
        {
        sprintf ( estring, "Bogus args to DriverWnd::get_VIS_DATA_struct()!" );
        return ( ( VIS_DATA * ) NULL );
        }

    if ( ! ( ans = f->get_VIS_DATA_struct ( estring,slice_type ) ) )
        {
        free_up_every_formulas_data();
        ans = f->get_VIS_DATA_struct ( estring,slice_type );
        }
    return ans;
    }



void DriverWnd::displaySingleNumberFormula ( char *valueString, Formula *f )
    {
    if ( !valueString )
        {
        updateStatus ( "No valueString supplied to DriverWnd::displaySingleNumberFormula()!" );
        return;
        }

    if ( !f )
        {
        updateStatus ( "No formula supplied to DriverWnd::displaySingleNumberFormula()!" );
        return;
        }

    strcat ( valueString, f->getUnits() );
    strcat ( valueString, "\n" );
    strcat ( valueString, f->getSelectedCellRange() );
    strcat ( valueString, "\nFirst time step: " );
    strcat ( valueString, f->getTimeMinString() );
    strcat ( valueString, "\nLast time step:  " );
    strcat ( valueString, f->getTimeMaxString() );
    updateStatus ( valueString );
    }



void DriverWnd::loopWindows ( int loopType ) // added 970102 SRT
    {
    if ( !max_num_hours_ ) return; // there's isn't window with > 1 time step
    grp_control_syn_cb(); // display the loop dialog box
    if ( loopType )
        {
        num_to_loop_ = max_num_hours_+1;
        //  fprintf(stderr, "setting num_to_loop_ to %d\n", num_to_loop_);
        }

    // start the loop
    animate_cb();
    }


/**********************************************************************/
/**********************************************************************/
int DriverWnd:: get_num_synch_steps ( void )
    {
    int i;
    int k;
    int *sdate, *stime, *edate, *etime, *tstep;
    int get_common_frames ( int, int*, int*, int*, int*, int* );


    if ( num_tilewnd_>1 )
        {
        sdate = new int[num_tilewnd_];
        stime = new int[num_tilewnd_];
        edate = new int[num_tilewnd_];
        etime = new int[num_tilewnd_];
        tstep = new int[num_tilewnd_];

        for ( i=0; i<num_tilewnd_; i++ )
            {
            sdate[i] = tilewnd_list_[i]->get_sdate();
            stime[i] = tilewnd_list_[i]->get_stime();
            edate[i] = tilewnd_list_[i]->get_edate();
            etime[i] = tilewnd_list_[i]->get_etime();
            tstep[i] = tilewnd_list_[i]->get_tstep();
            }

        if ( ( k = get_common_frames ( num_tilewnd_,sdate, stime, tstep,
                                       edate, etime ) ) <= 0 )
            {
            printf ( "DEBUG::WARNING: nothing to synchronize!!!\n" );
            }
        // ALT: 1999:09/30
        // since edate and etime are local vars and they are not needed anymore,
        // to save storage we reuse them to store offset and skip
        for ( i=0; i<num_tilewnd_; i++ )
            {
            tilewnd_list_[i]->set_offset ( edate[i] );
            tilewnd_list_[i]->set_skip ( etime[i] );
            }
        delete sdate;
        delete stime;
        delete edate;
        delete etime;
        delete tstep;
        }
    else if ( num_tilewnd_==1 )
        {
        i =0;
        k = tilewnd_list_[i]->get_nhours();
        tilewnd_list_[i]->set_offset ( 1 );
        tilewnd_list_[i]->set_skip ( 1 );
        }
    else k = 0;

    return k;
    }

/**********************************************************************/

int find_common ( int n, int *sdate, int *stime, int *tstep,
                  int *adate, int *atime, int *astep )
    {
    int jdate, jtime;
    int cdate, ctime;
    int k, i, j;
    int t, step;
    int t0;
    int found;

    jdate = sdate[0];
    jtime = stime[0];

    for ( k=1; k<n; k++ )
        {
        if ( secsdiffc ( jdate, jtime, sdate[k], stime[k] ) > 0 )
            {
            jdate = sdate[k];
            jtime = stime[k];
            }
        }

    t0 = sec2timec ( tstep[0] );
    j=JSTEP3 ( &jdate, &jtime, &sdate[0], &stime[0], &t0 );

    if ( !currstepc ( jdate, jtime, sdate[0], stime[0], t0,
                      &cdate, &ctime ) )
        {
        fprintf ( stderr,"ERROR in CURRSTEP at %07d:%06d\n",jdate, jtime );
        return 0;
        }

    if ( j <= 0 )
        {
        nextimec ( &cdate, &ctime, t0 );
        }

    step = tstep[0];
    for ( k=1; k<n; k++ )
        {
        step = ( ( long long ) step*tstep[k] ) /GCD ( &step, &tstep[k] );
        }

    k = step / tstep[0];

    jdate = cdate;
    jtime = ctime;

    for ( i=0; i<k; i++ )
        {
        found = 1;
        for ( j = 1; j < n; j++ )
            {
            t = sec2timec ( tstep[j] );
            if ( JSTEP3 ( &jdate, &jtime, &sdate[j], &stime[j], &t ) <= 0 )
                {
                found = 0;
                break;
                }
            }
        if ( found ) break;
        nextimec ( &jdate, &jtime, t0 );
        }

    if ( found )
        {
        *adate = jdate;
        *atime = jtime;
        *astep = sec2timec ( step );
        return 1;
        }
    else
        {
        return 0;
        }
    }

/***************************************************************/
int find_end ( int n, int *edate, int *etime, int adate, int atime, int astep )
    {
    int k;
    int jdate, jtime;
    int secs, t;

    jdate = edate[0];
    jtime = etime[0];

    for ( k=1; k<n; k++ )
        {
        if ( secsdiffc ( jdate, jtime, edate[k], etime[k] ) < 0 )
            {
            jdate = edate[k];
            jtime = etime[k];
            }
        }

    secs = secsdiffc ( adate, atime, jdate, jtime );
    if ( secs < 0 ) return 0;
    t = time2secc ( astep );
    if ( t == 0 ) return 0;

    k = secs / t;

    //  printf("Animation starts at %07d:%06d::%06d\t", adate, atime, astep);
    //  printf("ends at %07d:%06d\n", jdate, jtime);

    return k+1;
    }


/***************************************************************/
int get_common_frames ( int n, int *sdate, int *stime, int *tstep,
                        int *edate, int *etime )
    {
    int adate, atime, astep;
    int k, i;
    int *offset = edate;
    int *skip = etime;
    int ts, ti;


    int find_common ( int, int*, int*, int*, int*, int*, int* );
    int find_end   ( int, int*, int*, int, int, int );

    for ( k=0; k<n; k++ ) printf ( "Dataset %d: %07d:%06d %06d <=> %07d:%06d\n",
                                       k, sdate[k], stime[k], sec2timec ( tstep[k] ),
                                       edate[k], etime[k] );


    if ( !find_common ( n,sdate,stime,tstep, &adate, &atime, &astep ) )
        {
        printf ( "ERROR: Incommensurate datasets!!!\n" );
        return 0;
        }

    k = find_end ( n,edate,etime, adate, atime, astep );

    /* compute offset and skip */
    if ( k==0 )
        {
        for ( i=0; i<n; i++ )
            {
            offset[i] = 0;
            skip[i] = 0;
            }
        }
    else
        {
        ts = time2secc ( astep );
        for ( i=0; i<n; i++ )
            {
            ti = sec2timec ( tstep[i] );
            offset[i] = JSTEP3 ( &adate, &atime,&sdate[i], &stime[i], &ti );
            skip[i] = ts / tstep[i];
            }
        }

    return k;
    }
/***************************************************************/
/***************************************************************/
/***************************************************************/
#define min(a,b) ((a) < (b) ? (a): (b))
#define max(a,b) ((a) > (b) ? (a): (b))
void DriverWnd:: createComp ( int row, int col, int delta_n, int nlin,
                              char *fmlstr1, char *fmlstr2 )
    {
    char statusMsg[512];
    Formula *formula1, *formula2;
    VIS_DATA *vdata;

    char *legend[8],
         *symbol[] = { "Cross","Square","Circle","Diamond","Plus",
                       "Square","Circle","Diamond"
                     },
                     *color[] = { "green", "red", "blue", "black", "yellow",
                                  "green", "black", "red"
                                };
    int nlines=2, npoints[8], totnpoints = 0;
    float *x, *y, *tsdata;

    Domain *domain;
    char minfo[192];
    char minfo2[192];
    int ni, nj;
    int ni2, nj2;
    int xmin, xmax, ymin, ymax;
    int xmin1, xmax1, ymin1, ymax1;
    int i, j, k, t, index;
    float val, minval, maxval, val0;
    int *target[3];
    char title[128];
    char yLabel[128];
    char timeStepLabel[128];
    int minmaxset;

    if ( ! ( formula1 = ( Formula * ) ( formulaList_.find ( fmlstr1 ) ) ) )
        {
        sprintf ( statusMsg, "Didn't find '%s' on the formulaList !\n", fmlstr1 );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }
    if ( ! ( formula2 = ( Formula * ) ( formulaList_.find ( fmlstr2 ) ) ) )
        {
        sprintf ( statusMsg, "Didn't find '%s' on the formulaList !\n", fmlstr2 );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }

    npoints[0] = formula2->get_nsteps();
    npoints[1] = npoints[2] = npoints[3] = formula1->get_nsteps();
    totnpoints = npoints[0]+npoints[1]+npoints[2]+npoints[3];

    x = new float[totnpoints];
    y = new float[totnpoints];
    if ( ( !x ) || ( !y ) )
        {
        if ( x ) delete [] x;
        x = NULL;
        if ( y ) delete [] y;
        y = NULL;
        Message error ( info_window_, XmDIALOG_ERROR,
                        "Couldn't allocate memory in DriverWnd::createComp()!" );
        return;
        }
    ni = formula1->get_ncol();
    nj = formula1->get_nrow();

    if ( formula1->getMapInfo ( minfo, statusMsg ) )
        {
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }

    ni2 = formula2->get_ncol();
    nj2 = formula2->get_nrow();

    if ( formula2->getMapInfo ( minfo2, statusMsg ) )
        {
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }

    if ( ( ni != ni2 ) || ( nj != nj2 ) ) //|| strcmp(minfo, minfo2))
        {
        sprintf ( statusMsg, "ERROR:Formulas %s and %s have different domains!",
                  fmlstr1, fmlstr2 );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }

    target[0] = &ni;
    target[1] = &nj;;
    target[2] = ( int * ) minfo;

    if ( ! ( domain = ( Domain * ) ( domainList_.find ( target ) ) ) )
        {
        sprintf ( statusMsg, "Didn't find Domain %dx%d %s on the DomainList!",
                  ni, nj, minfo );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }

    domain->getRange ( &xmin, &ymin, &xmax, &ymax );
    xmin1 = max ( col-delta_n,1 );
    ymin1 = max ( row-delta_n,1 );
    xmax1 = min ( col+delta_n,ni );
    ymax1 = min ( row+delta_n,nj );
    domain->setRange ( col, row, col, row );

    if ( ( inputTitleString_ != NULL ) && ( inputTitleString_[0]!='\0' ) )
        {
        sprintf ( title, "%s", inputTitleString_ );
        }
    else
        {
        sprintf ( title, "Time Series at " );
        strcat ( title, formula2->getSelectedCellRange() );
        }


    if ( ! ( tsdata=formula2->get_time_series_data ( statusMsg ) ) )
        {
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }

    k = formula2->get_hrMin();
    for ( i=0; i<npoints[0]; i++ )
        {
        x[i] = k+i;
        y[i] = tsdata[i];
        }

    domain->setRange ( xmin1, ymin1, xmax1, ymax1 );
    k = formula1->get_hrMin();
    if ( vdata = get_VIS_DATA_struct ( formula1,statusMsg,XYTSLICE ) )
        {
        for ( t=vdata->step_min; t<=vdata->step_max; t++ )
            {
            minmaxset = 0;
            for ( i = xmin1; i <= xmax1; i++ )
                {
                for ( j = ymin1; j <= ymax1; j++ )
                    {
                    index = INDEX (  i-vdata->col_min,
                                     j-vdata->row_min,
                                     0,
                                     t-vdata->step_min,
                                     vdata->col_max-vdata->col_min+1,
                                     vdata->row_max-vdata->row_min+1,
                                     1 );

                   val = vdata->grid[index];
                    if ( isnanf ( val ) ) continue;
                    if ( i==col && j==row ) val0 = val;
                    if ( !minmaxset )
                        {
                        minmaxset = 1;
                        minval = maxval = val;
                        }
                    else
                        {
                        minval = min ( minval, val );
                        maxval = max ( maxval, val );
                        }
                    }
                }
            if ( !minmaxset )
                {
                minval = maxval = setNaNf();
                }
            i = t-vdata->step_min;
            x[i+npoints[0]] =
                x[i+npoints[0]+npoints[1]] =
                    x[i+npoints[0]+npoints[1]+npoints[2]] = k+i;
            //      y[i] = tsdata[i];
            y[i+npoints[0]] = maxval;
            y[i+npoints[0]+npoints[1]] = minval;
            y[i+npoints[0]+npoints[1]+npoints[2]] = val0;
            }

        free ( tsdata );
        tsdata = NULL;

        nlines = nlin;
        legend[0]=new char[5+strlen ( fmlstr2 )];
        legend[1]=new char[5+strlen ( fmlstr1 )];
        legend[2]=new char[5+strlen ( fmlstr1 )];
        legend[3]=new char[1+strlen ( fmlstr1 )];
        sprintf ( legend[0],"Obs_%s",     fmlstr2 );
        sprintf ( legend[1],"Max_%s", fmlstr1 );
        sprintf ( legend[2],"Min_%s", fmlstr1 );
        sprintf ( legend[3],"%s",     fmlstr1 );


        //      sprintf(yLabel, "MinMax Value");
        sprintf ( yLabel,"%s", formula1->getFormulaName() );
        if ( formula1->getUnits() && strlen ( formula1->getUnits() ) )
            {
            strcat ( yLabel, " (" );
            strcat ( yLabel, formula1->getUnits() );
            strcat ( yLabel, ")" );
            }

        strcat ( yLabel, " " );
        strcat ( yLabel, formula1->getSelectedCellRange() );

        sprintf ( timeStepLabel, "Time Step (%s to %s)",
                  formula1->getTimeMinString(), formula1->getTimeMaxString() );

        // now create the plot !!
        if ( !graph2d ( x, y, nlines, npoints,
                        title, timeStepLabel, yLabel,
                        legend, symbol, color, statusMsg, 1 ) )
            {
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return;
            }
        delete [] x;
        x = NULL;
        delete [] y;
        y = NULL;
        }
    else
        {
        fprintf ( stderr,"ERROR:vdata: %s\n", statusMsg );
        }

    // restore domain settings
    domain->setRange ( xmin, ymin, xmax, ymax );
    // free visdata
    free_vis ( vdata );
    for ( i=0; i<nlines; i++ )
        {
        delete [] legend[i];
        }

    }



/*****************************************************/
void DriverWnd:: createComp ( int row, int col, int delta_n,
                              char *fmlstr1, char *fmlstr2, char *fmlobs )
    {
    char statusMsg[512];
    Formula *formula1, *formula2, *formula3;
    VIS_DATA *vdata;

    char *legend[8],
         *symbol[] = { "Cross","Square","Circle","Diamond","Plus",
                       "Square","Circle","Diamond"
                     },
                     *color[] = { "green", "red", "blue", "black", "yellow",
                                  "green", "black", "red"
                                };
    int nlines=2, npoints[8], totnpoints = 0;
    float *x, *y, *tsdata;

    Domain *domain;
    char minfo[192];
    int ni, nj;
    int ni2, nj2;
    int ni3, nj3;
    int xmin, xmax, ymin, ymax;
    int xmin1, xmax1, ymin1, ymax1;
    int i, j, k, t, index;
    float val, maxval;
    int *target[3];
    char title[128];
    char yLabel[128];
    char timeStepLabel[128];
    int maxset;

    if ( ! ( formula1 = ( Formula * ) ( formulaList_.find ( fmlstr1 ) ) ) )
        {
        sprintf ( statusMsg, "Didn't find '%s' on the formulaList !\n", fmlstr1 );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }
    if ( ! ( formula2 = ( Formula * ) ( formulaList_.find ( fmlstr2 ) ) ) )
        {
        sprintf ( statusMsg, "Didn't find '%s' on the formulaList !\n", fmlstr2 );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }

    if ( ! ( formula3 = ( Formula * ) ( formulaList_.find ( fmlobs ) ) ) )
        {
        sprintf ( statusMsg, "Didn't find '%s' on the formulaList !\n", fmlobs );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }
    npoints[0] = formula3->get_nsteps();
    // no checks here (we should do them later!!!
    npoints[1] = formula1->get_nsteps();
    npoints[2] = formula2->get_nsteps();
    totnpoints = npoints[0]+npoints[1]+npoints[2];

    x = new float[totnpoints];
    y = new float[totnpoints];
    if ( ( !x ) || ( !y ) )
        {
        if ( x ) delete [] x;
        x = NULL;
        if ( y ) delete [] y;
        y = NULL;
        Message error ( info_window_, XmDIALOG_ERROR,
                        "Couldn't allocate memory in DriverWnd::createComp()!" );
        return;
        }
    ni = formula1->get_ncol();
    nj = formula1->get_nrow();

    if ( formula1->getMapInfo ( minfo, statusMsg ) )
        {
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }

    ni2 = formula2->get_ncol();
    nj2 = formula2->get_nrow();

    ni3 = formula3->get_ncol();
    nj3 = formula3->get_nrow();


    if ( ( ni != ni2 ) || ( nj != nj2 ) || ( ni != ni3 ) || ( nj != nj3 ) )
        {
        sprintf ( statusMsg, "ERROR:Formulas %s, %s and %s have different domains!",
                  fmlstr1, fmlstr2, fmlobs );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }

    target[0] = &ni;
    target[1] = &nj;;
    target[2] = ( int * ) minfo;

    if ( ! ( domain = ( Domain * ) ( domainList_.find ( target ) ) ) )
        {
        sprintf ( statusMsg, "Didn't find Domain %dx%d %s on the DomainList!",
                  ni, nj, minfo );
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }

    domain->getRange ( &xmin, &ymin, &xmax, &ymax );
    xmin1 = max ( col-delta_n,1 );
    ymin1 = max ( row-delta_n,1 );
    xmax1 = min ( col+delta_n,ni );
    ymax1 = min ( row+delta_n,nj );
    domain->setRange ( col, row, col, row );

    if ( ( inputTitleString_ != NULL ) && ( inputTitleString_[0]!='\0' ) )
        {
        sprintf ( title, "%s", inputTitleString_ );
        }
    else
        {
        sprintf ( title, "Time Series at " );
        strcat ( title, formula2->getSelectedCellRange() );
        }


    if ( ! ( tsdata=formula3->get_time_series_data ( statusMsg ) ) )
        {
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }

    k = formula3->get_hrMin();
    printf ( "\n\nDEBUG OBS k=%d N=%d\n",k,npoints[0] );
    for ( i=0; i<npoints[0]; i++ )
        {
        x[i] = k+i;
        y[i] = tsdata[i];
        }
    free ( tsdata );
    tsdata = NULL;

    domain->setRange ( xmin1, ymin1, xmax1, ymax1 );
    k = formula1->get_hrMin()-formula3->get_hrMin();
    printf ( "\n\nDEBUG Model1 k=%d N=%d\n",k,npoints[1] );
    if ( vdata = get_VIS_DATA_struct ( formula1,statusMsg,XYTSLICE ) )
        {
        for ( t=vdata->step_min; t<=vdata->step_max; t++ )
            {
            maxset = 0;
            for ( i = xmin1; i <= xmax1; i++ )
                {
                for ( j = ymin1; j <= ymax1; j++ )
                    {
                    index = INDEX (  i-vdata->col_min,
                                     j-vdata->row_min,
                                     0,
                                     t-vdata->step_min,
                                     vdata->col_max-vdata->col_min+1,
                                     vdata->row_max-vdata->row_min+1,
                                     1 );

                    val = vdata->grid[index];
                    if ( isnanf ( val ) ) continue;
                    if ( !maxset )
                        {
                        maxset = 1;
                        maxval = val;
                        }
                    else
                        {
                        maxval = max ( maxval, val );
                        }
                    }
                }
            if ( !maxset )
                {
                maxval = setNaNf();
                }
            i = t-vdata->step_min;
            x[i+npoints[0]] = k+i;
            y[i+npoints[0]] = maxval;
            }

        // free visdata
        free_vis ( vdata );
        }
    else
        {
        fprintf ( stderr,"ERROR:vdata: %s\n", statusMsg );
        return;
        }

    k = formula2->get_hrMin()-formula3->get_hrMin();
    printf ( "\n\nDEBUG Model2 k=%d N=%d\n",k,npoints[2] );
    if ( vdata = get_VIS_DATA_struct ( formula2,statusMsg,XYTSLICE ) )
        {
        for ( t=vdata->step_min; t<=vdata->step_max; t++ )
            {
            maxset = 0;
            for ( i = xmin1; i <= xmax1; i++ )
                {
                for ( j = ymin1; j <= ymax1; j++ )
                    {
                    index = INDEX (  i-vdata->col_min,
                                     j-vdata->row_min,
                                     0,
                                     t-vdata->step_min,
                                     vdata->col_max-vdata->col_min+1,
                                     vdata->row_max-vdata->row_min+1,
                                     1 );

                    val = vdata->grid[index];
                    if ( isnanf ( val ) ) continue;
                    if ( !maxset )
                        {
                        maxset = 1;
                        maxval = val;
                        }
                    else
                        {
                        maxval = max ( maxval, val );
                        }
                    }
                }
            if ( !maxset )
                {
                maxval = setNaNf();
                }
            i = t-vdata->step_min;
            x[i+npoints[0]+npoints[1]] = k+i;
            y[i+npoints[0]+npoints[1]] = maxval;
            }

        // free visdata
        free_vis ( vdata );
        }
    else
        {
        fprintf ( stderr,"ERROR:vdata: %s\n", statusMsg );
        return;
        }



    nlines = 3;
    legend[0]=new char[5+strlen ( fmlobs )];
    legend[1]=new char[6+strlen ( fmlstr1 )];
    legend[2]=new char[6+strlen ( fmlstr2 )];
    sprintf ( legend[0],"Obs_%s",     fmlobs );
    sprintf ( legend[1],"Max1_%s", fmlstr1 );
    sprintf ( legend[2],"Max2_%s", fmlstr2 );


    //      sprintf(yLabel, "MinMax Value");
    sprintf ( yLabel,"%s", formula1->getFormulaName() );
    if ( formula1->getUnits() && strlen ( formula1->getUnits() ) )
        {
        strcat ( yLabel, " (" );
        strcat ( yLabel, formula1->getUnits() );
        strcat ( yLabel, ")" );
        }

    strcat ( yLabel, " " );
    strcat ( yLabel, formula1->getSelectedCellRange() );

    sprintf ( timeStepLabel, "Time Step (%s to %s)",
              formula1->getTimeMinString(), formula1->getTimeMaxString() );

    // now create the plot !!
    if ( !graph2d ( x, y, nlines, npoints,
                    title, timeStepLabel, yLabel,
                    legend, symbol, color, statusMsg, 1 ) )
        {
        Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
        return;
        }
    delete [] x;
    x = NULL;
    delete [] y;
    y = NULL;

    // restore domain settings
    domain->setRange ( xmin, ymin, xmax, ymax );
    for ( i=0; i<nlines; i++ )
        {
        delete [] legend[i];
        }

    }
/*****************************************************/




/*
 * Return the ID of the most window;
 */
Window getMostRecentGraphWindow ( void )
    {
    Window win;
    char *winidstr;

    win = 0;
    if ( winidstr = getenv ( "BLT_GRAPH_WINDOW_ID" ) )
        {
        sscanf ( winidstr,"%x", &win );
        }
    return win;
    }



extern "C" {
    int write_frame ( Display *, Window, char * );
    }

int writeImage ( Display *display,
                 Window window,
                 char *imagetype,  // for now only these are supported:
                 // "PNM", "XWD", "GIF", "RGB", "MPEG",
                 // "PS
                 char *fname,      // image file name
                 char *estring ) // error messages will go here
    {
    char  xwdname[256], command[512];
    int   i;
    char *convert_cmd;

    // check the arguments
    if ( ( !imagetype ) || ( !fname ) || ( !estring ) )
        {
        fprintf ( stderr, "\007Bad args to TileWnd::dumpImage()!" );
        return 1;
        }
    if ( strcmp ( imagetype, "PNM" ) && strcmp ( imagetype, "XWD" ) &&
            strcmp ( imagetype, "RGB" ) && strcmp ( imagetype, "GIF" ) &&
            strcmp ( imagetype, "PS" ) )
        {
        sprintf ( estring,
                  "\007Unsupported image type '%s' arg to TileWnd::dumpImage()!",
                  imagetype );
        return 1;
        }

    // dump the image as an XWD
    if ( strcmp ( imagetype, "XWD" ) )
        sprintf ( xwdname, "%s.xwd", fname );
    else
        sprintf ( xwdname, "%s", fname );
    fprintf ( stderr, "\nWriting XWD image '%s'...%x %x\n", xwdname, display, window );
    if ( write_frame ( display, window, xwdname ) )
        {
        sprintf ( estring,
                  "\007writeImage() couldn't write '%s'!", xwdname );
        return 1;
        }


    // convert the to the image desired format (if not XWD) and name
    if ( strcmp ( imagetype, "XWD" ) )
        {
        fprintf ( stderr, "Converting '%s' to %s image '%s'...\n",
                  xwdname, imagetype, fname );

        convert_cmd = getenv ( "CONVERT" );
        sprintf ( command, "'%s' XWD:%s %s:%s",
                  ( convert_cmd != NULL ? convert_cmd: "convert" ),
                  xwdname, imagetype, fname );
        printf ( "DEBUG: executing %s\n",command );
        i=system ( command );
        unlink ( xwdname );
        }

    if ( i == 0 )
        {
        // convert the to the image desired format (if not XWD) and name
        fprintf ( stderr, "Image file '%s' successfully created.\n", fname );
        }
    else
        {
        fprintf ( stderr, "ERROR creating image file '%s'\n", fname );
        }
    return 0;
    }
/*

*/


// ============================================================
void driverWindowGetFormula ( void *dwnd, Widget edit_selection_dialog, int mode )
    {
    DriverWnd *d = ( DriverWnd * ) dwnd;
    d->putFormula2SelectionDialog ( edit_selection_dialog, mode );
    }

// ============================================================
void driverWindowMultiSelectDialog ( void *dwnd, int mode, void *twnd )
    {
    DriverWnd *d = ( DriverWnd * ) dwnd;
    d->createMultiFormulaSelectionDialog ( 2, mode, twnd );
    }

// ============================================================
void DriverWnd::putFormula2SelectionDialog ( Widget edit_selection_dialog,
        int mode )
    {
    int i;
    int obs;
    char *item;
    Formula *formula;

    i=0;
    formula = ( Formula * ) formulaList_.head();
    while ( formula )
        {
        obs = formula->isObs();
        if ( ( obs && ( mode==OBS_PLOT || mode==OBSVECTOR_PLOT ) )
                || ( !obs && mode!=OBS_PLOT ) )
            {
            i++;
            item = formula->getFormulaName();
            if ( item )
                {
                XmString xmstr = XmStringCreateLocalized ( item );
                XmListAddItemUnselected ( XmSelectionBoxGetChild ( edit_selection_dialog,
                                          XmDIALOG_LIST ),
                                          xmstr,i );
                XmStringFree ( xmstr );
                }
            }
        formula = ( Formula * ) formulaList_.next();
        }

    }


// ============================================================
PLOT_DATA *driverWindowGetPlotData ( void *dwnd, char *formulaName, int mode )
    {
    DriverWnd *d = ( DriverWnd * ) dwnd;
    return d->getPlotData ( formulaName, mode );
    }

// ============================================================
PLOT_DATA *DriverWnd::getPlotData ( char *formulaName, int mode )
    {
    static int n_cntrs=0;
    char statusMsg[512];
    Formula *formula;
    Formula *formula_coord[3];
    VIS_DATA *vdata;
    VIS_DATA *vdata_coord[3];
    char tstring[256];
    int i;
    int slice =XYTSLICE;
    //     ***** ALT attention here ********
    PLOT_DATA *pdata=new PLOT_DATA;
    static int cntr_cmap[] =
        {
        0x191970,0x00c8cb, 0xffffff, 0x0
        };
    char *units;
    extern void long2str ( int, char * );


    pdata->plot_type=mode;

    if ( mode == OBSVECTOR_PLOT )
        {
        char *formulaName_x, *formulaName_y;

        pdata->vect2d = new VECTOR2D_DATA();
        formulaName_x = strdup ( formulaName );
        formulaName_y = strtok ( formulaName_x," " );
        formulaName_y = strtok ( NULL," " );

        formula = ( Formula * ) ( formulaList_.find ( formulaName_x ) );
        if ( !formula )
            {
            fprintf ( stderr,"ERROR: cannot find formula %s\n", formulaName_x );
            return NULL;
            }

        if ( ! ( pdata->vect2d->vdata_x =
                     get_VIS_DATA_struct ( formula,statusMsg, slice ) ) )
            {
            fprintf ( stderr,"ERROR: VDATA %s\n", formulaName_x );
            }
        else
            {
            pdata->vect2d->vdata_x->selected_step = formula->get_selected_step();
            }

        formula = ( Formula * ) ( formulaList_.find ( formulaName_y ) );
        if ( !formula )
            {
            fprintf ( stderr,"ERROR: cannot find formula %s\n", formulaName_y );
            return NULL;
            }

        if ( ! ( pdata->vect2d->vdata_y =
                     get_VIS_DATA_struct ( formula,statusMsg, slice ) ) )
            {
            fprintf ( stderr,"ERROR: VDATA %s\n", formulaName_y );
            }
        else
            {
            pdata->vect2d->vdata_y->selected_step = formula->get_selected_step();
            }
        free ( formulaName_x );

        }
    else
        {
        formula = ( Formula * ) ( formulaList_.find ( formulaName ) );
        if ( !formula )
            {
            fprintf ( stderr,"ERROR: cannot find formula %s\n", formulaName );
            return NULL;
            }

        vdata = get_VIS_DATA_struct ( formula,statusMsg, slice );
        if ( !vdata )
            {
            fprintf ( stderr,"ERROR: VDATA %s\n%s\n", formulaName, statusMsg );
            delete pdata;
            pdata = NULL;
            return NULL;
            }
        else
            {
            vdata->selected_step = formula->get_selected_step();
            }
        }

    if ( mode == CONTOUR_PLOT )
        {
        int rgb;
        XColor color;
        Display *display=XtDisplay ( _w );
        int scr = DefaultScreen ( display );
        Colormap cmap = DefaultColormap ( display, scr );

        pdata->vdata = vdata;
        pdata->jstep = NULL;
        pdata->coord_x = pdata->coord_y = NULL;
        pdata->cntr_data = new CONTOUR_DATA();

        pdata->cntr_data->gc = XCreateGC ( display,
                                           DefaultRootWindow ( display ),
                                           ( unsigned long ) NULL,
                                           ( XGCValues * ) NULL );

        rgb = cntr_cmap[n_cntrs%sizeof ( cntr_cmap )];

        /* & to get color element (red, green, blue) with results converted
        from 8-bit to 32-bit numbers */

        color.red   = ( unsigned short ) ( ( rgb & 0xff0000 ) >> 8 );
        color.green = ( unsigned short ) ( ( rgb & 0x00ff00 ) );
        color.blue  = ( unsigned short ) ( ( rgb & 0x0000ff ) << 8 );

        if ( !XAllocColor ( display, cmap, &color ) )
            {
            fprintf ( stderr, "Can't allocate color rgb %d\n", rgb );
            }

        XSetForeground ( display,
                         pdata->cntr_data->gc,
                         color.pixel );



        n_cntrs++;
        return pdata;
        }

    // we need to extract the x,y of the monitors
    char *casesUsed = formula->getCasesUsedString();
    char tmp_str[16];
    VIS_DATA *vdatap;

    // CALT: we do not want to assume partucular names for stationId, Lat, Lon
    // Instead we require particular position in the OBS file:
    // the assumption is the stationId is first var, LAT is next, followed by LON


    vdatap = ( ( CaseServer * ) case_ )->get_dataSets_VISDATA ( casesUsed+2 );

    if ( vdatap )
        {

        for ( i=0; i<3; i++ )
            {
            sprintf ( tmp_str,"%s%c",vdatap->species_short_name[i],casesUsed[0] );
            tmp_str[strlen ( vdatap->species_short_name[i] )+1] = '\0';

            tstring[0]='\0';
            if ( i==0 ) putenv ( "PAVE_NO_INT2FLOAT=1" );
            formula_coord[i] = new Formula ( tmp_str,
                                             bd_,
                                             &datasetList_,
                                             &domainList_,
                                             &levelList_,
                                             getParent(),
                                             ( void * ) formula_,
                                             tstring );

            if ( formula_coord[i] )
                {
                vdata_coord[i] = get_VIS_DATA_struct ( formula_coord[i],statusMsg,slice );

                }
            else
                {
                fprintf ( stderr,"WARNING: cannot find (%s)\n",tstring );
                vdata_coord[i] = NULL;
                }
            if ( i==0 ) putenv ( "PAVE_NO_INT2FLOAT=" );
            }
        // technically the next 4 lines should be allocated with "new", then
        // the grid data copied and vdata_coord freed.
        if ( vdata_coord[0] )
            {
            int nsteps = vdata_coord[0]->step_max - vdata_coord[0]->step_min + 1;
            int ncols  =  vdata_coord[0]->ncol;
            int n = nsteps * ncols;
            int k, ktot;
            int *igrid = ( int * ) vdata_coord[0]->grid;
            int packed_char;
            char tmp[80];
            char *str;
            StringPair *idSubst;
            char *stnidOnce = getenv ( "PAVE_SUBST_OBSID_ONCE" );

            pdata->stnid = ( char ** ) malloc ( n*sizeof ( char * ) );
            units = formula_coord[0]->getUnits();
            if ( units && !strncmp ( units,"CHARACTER",9 ) )
                {
                packed_char = 1;
                }
            else
                {
                packed_char = 0;
                }

            if ( stnidOnce )
                {
                ktot = ncols;
                }
            else
                {
                ktot = n;
                }

            for ( k = 0; k < ktot; k++ )
                {
                if ( packed_char )
                    {
                    long2str ( igrid[k], tmp );
                    }
                else
                    {
                    sprintf ( tmp, "%09d", igrid[k] );
                    }
                str = tmp;
                if ( obsIdListP_ )
                    {
                    if ( idSubst = ( StringPair * ) obsIdListP_->find ( tmp ) )
                        {
                        str = idSubst->getName();
                        printf ( "Found substitution for %s with %s\n",tmp,str );
                        }
                    }
                pdata->stnid[k] = strdup ( str );
                }
            if ( stnidOnce )
                {
                int t, j;
                k = ncols;
                for ( t=1; t<nsteps; t++ )
                    {
                    for ( j=0; j<ncols; j++ )
                        {
                        pdata->stnid[k++] = pdata->stnid[j];
                        }
                    }
                }
            }
        else
            {
            fprintf ( stderr,"WARNING: cannot find stationId\n" );
            pdata->stnid = NULL;
            }
        pdata->coord_y = vdata_coord[1]->grid;
        pdata->coord_x = vdata_coord[2]->grid;
        free_vis ( vdata_coord[0] );

        }
    else
        {
        pdata = NULL;
        return pdata;

        }



    if ( mode == OBS_PLOT ) pdata->vdata = vdata;
    pdata->jstep = NULL;

    return pdata;
    }

// ============================================================
Widget driverWindowSelectionDialog_CB ( void *dwnd, TileWnd *tileWnd )
    {
    char strbuf[100];
    DriverWnd *d = ( DriverWnd * ) dwnd;
    Widget  edit_selection_dialog;
    Widget parent = d->getParent();

    sprintf ( strbuf, "%s","Enter New Overlay" );
    XmString label = XmStringCreateLocalized ( strbuf );

    sprintf ( strbuf, "%s","Overlay" );
    edit_selection_dialog = XmCreateSelectionDialog ( parent, strbuf, NULL, 0 );
    XtVaSetValues ( edit_selection_dialog,
                    XmNautoUnmanage,                True,
                    XmNselectionLabelString,        label,
                    XmNheight,              320, // SRT 960411
                    NULL );
    XtAddCallback ( edit_selection_dialog, XmNokCallback,
                    &TileWnd::overlay_selectedCB, ( XtPointer ) tileWnd );

    XmStringFree ( label );

    // Hide the Apply and Help buttons
    XtUnmanageChild ( XmSelectionBoxGetChild ( edit_selection_dialog,
                      XmDIALOG_APPLY_BUTTON ) );
    XtUnmanageChild ( XmSelectionBoxGetChild ( edit_selection_dialog,
                      XmDIALOG_HELP_BUTTON ) );

    return ( edit_selection_dialog );
    }

// ============================================================
// ============================================================


void DriverWnd::getObsTimeSeries()
    {

    if ( mostRecentTile_ )
        {
        XSync ( XtDisplay ( _w ), False );
        mostRecentTile_->overlay_ts();
        }
    }

// ============================================================

void DriverWnd::tzOffset ( int tzoff )
    {

    if ( mostRecentTile_ )
        {
        mostRecentTile_->resetTZname();
        mostRecentTile_->setTZ ( tzoff );
        }
    else
        {
        fprintf ( stderr,"ERROR:%s\n","Tile plot must be generated first" );
        }
    return;
    }

// ============================================================

void DriverWnd::tzSet ( char *tzin, char *tzout )
    {

    if ( mostRecentTile_ )
        {
        mostRecentTile_->setTZname ( tzin, tzout );
        }
    else
        {
        fprintf ( stderr,"ERROR:%s\n","Tile plot must be generated first" );
        }
    return;
    }

// ============================================================


void DriverWnd::export2file ( char *filename, int exportType )
    {
    char formulaname[512], statusMsg[512];
    Formula *formula;
    VIS_DATA *vdata = NULL;

    if ( formula_->getCurrSelection() )
        {
        // Miscellaneous setup junk
        stop_cb();
        strcpy ( formulaname, formula_->getCurrSelection() );
        updateStatus ( "Exporting netCDF data file..." );

        // Find the currently selected formula's Formula object
        if ( ! ( formula = ( Formula * ) ( formulaList_.find ( formulaname ) ) ) )
            {
            sprintf ( statusMsg, "Didn't find '%s'\non the formulaList!\n",
                      formulaname );
            Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
            return;
            }

        // Retrieve a copy of that Formula object's data
        if ( ! ( vdata = get_VIS_DATA_struct ( formula,statusMsg,XYZTSLICE ) ) )
            {
            if ( strstr ( statusMsg, "==" ) )
                displaySingleNumberFormula ( statusMsg, formula );
            else
                {
                Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                }
            return;
            }

        switch ( exportType )
            {
            case PAVE_EXPORT_TABBED:
                if ( dump_VIS_DATA_to_tabbed_ascii_file ( ( VIS_DATA * ) vdata,
                        filename,
                        statusMsg ) )
                    {
                    Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                    }
                break;

            case PAVE_EXPORT_NETCDF:

                if ( dump_VIS_DATA_to_netCDF_file ( ( VIS_DATA * ) vdata,
                                                    filename,
                                                    statusMsg ) )
                    {
                    Message error ( info_window_, XmDIALOG_ERROR, statusMsg );
                    }
                break;
            default:
                {
                Message error ( info_window_, XmDIALOG_ERROR, "Unknown export type" );
                }
            }
        }
    else
        {
        Message error ( info_window_, XmDIALOG_ERROR,
                        "There is no formula currently selected!" );
        }

    }
// ============================================================

void DriverWnd::animatedGIF ( char *fname )
    {

    XSync ( XtDisplay ( _w ), False );
    sleep ( 5 );
    if ( mostRecentTile_ )
        {
        mostRecentTile_->saveAnimation_cb ( fname,"gif" );
        }
    else
        {
        fprintf ( stderr,"ERROR:%s\n","Tile plot must be generated first" );
        }
    return;
    }

// ============================================================

void DriverWnd::changeTitleFontSize ( int size )
    {

    if ( size <= 0 )
        {
        fprintf ( stderr,"ERROR: negative/zero title size not allowed(%d)\n",size );
        return;
        }
    if ( mostRecentTile_ )
        {
        mostRecentTile_->setTitleFontSize ( size );
        }
    else
        {
        fprintf ( stderr,"ERROR:%s\n","Tile plot must be generated first" );
        }
    return;
    }

// ============================================================

void DriverWnd::changeSubTitleFontSize ( int size )
    {

    if ( size <= 0 )
        {
        fprintf ( stderr,"ERROR: negative/zero title size not allowed(%d)\n",size );
        return;
        }
    if ( mostRecentTile_ )
        {
        mostRecentTile_->setSubTitleFontSize ( size );
        }
    else
        {
        fprintf ( stderr,"ERROR:%s\n","Tile plot must be generated first" );
        }
    return;
    }

// ============================================================
void DriverWnd::multiVarNcf ( char *flist, char *vlist, char *file )
    {

    char *fname[MXVARS3];
    char *vname[MXVARS3];
    int n, n1;
    int i;
    int parseList ( char *list, char *entry[MXVARS3] );
    char tmpname[MXVARS3][256];
    char tmpenv[256];
    char *t;
    char *formulaname=NULL;
    char tmplt[256];
    char command[512];
    char cmdfile[256];
    FILE *cmd;
    Formula *fml;

    // check if the output file exists
    if ( access ( file, F_OK ) == 0 )
        {
        fprintf ( stderr,"WARNING: output file %s already exist, command ignored\n",
                  file );
        return;
        }

    // check if M3MERGE exists
#if !(defined(linux) || defined(_AIX) || defined(__OPENNT))
    if ( !pathfind ( getenv ( "PATH" ), "m3merge", "rx" ) )
        {
        fprintf ( stderr,"WARNING: m3merge not installed, command ignored\n" );
        return;
        }
#else
    if ( system ( "which m3merge" ) != 0 )
        {
        fprintf ( stderr,"WARNING: m3merge not installed, command ignored\n" );
        return;
        }

#endif


    // save current Formula selection
    if ( formula_->getCurrSelection() )
        {
        formulaname = strdup ( formula_->getCurrSelection() );
        }

    n = parseList ( flist, fname );
    if ( n < 0 ) return;

    n1 = parseList ( vlist, vname );
    if ( n1 < 0 ) goto cleanup;

    if ( n != n1 )
        {
        fprintf ( stderr,"ERROR: %s\n",
                  "VarList and FormulaList have different number of entries" );
        goto cleanup;
        }

    sprintf ( tmplt, "/tmp/m3mergeXXXXXX" );
    strcpy ( cmdfile, mktemp ( tmplt ) );

    for ( i=0; i<n; i++ )
        {
        t = fname[i];
        fml = ( Formula * ) ( formulaList_.find ( t ) );
        if ( !fml )
            {
            fprintf ( stderr,"ERROR: %s is not a valid formula\n", t );
            goto cleanup;
            }
        formula_->setCurrentSelection ( t );
        sprintf ( tmpenv, "PAVE_EXPORT_VARNAME=%s",vname[i] );
        putenv ( tmpenv );
        sprintf ( tmplt, "/tmp/svncf%dXXXXXX",i );
        strcpy ( tmpname[i], mktemp ( tmplt ) );
        export2file ( tmpname[i], PAVE_EXPORT_NETCDF );
        }

    // prepare command for m3merge
    cmd = fopen ( cmdfile,"w" );
    if ( cmd == NULL )
        {
        fprintf ( stderr,"ERROR: cannot open command file %s\n", cmdfile );
        goto cleanup;
        }

    fprintf ( cmd,"#! /bin/csh -f\nsetenv OUTFILE %s\n",file );
    for ( i=0; i<n; i++ )
        {
        fprintf ( cmd,"setenv INFILE%d %s\n", i+1, tmpname[i] );
        }
    fprintf ( cmd,"\n%s\n\n","m3merge << _EOF_" );
    for ( i=0; i<n; i++ )
        {
        fprintf ( cmd,"\n\n" );
        }
    fprintf ( cmd,"NONE\n\n\n\n\n\n%s\n","_EOF_" );
    fclose ( cmd );

    sprintf ( command, "csh %s", cmdfile );
    i=system ( command );
    if ( i != 0 )
        {
        fprintf ( stderr,"ERROR:M3MERGE exit status = %d\n",i );
        }

    unlink ( cmdfile );


cleanup:
    // restore current Formula selection
    if ( formulaname )
        {
        formula_->setCurrentSelection ( formulaname );
        free ( formulaname );
        }

    for ( i=0; i<n; i++ )
        {
        unlink ( tmpname[i] );
        }

    for ( i=0; i<MXVARS3; i++ )
        {
        if ( fname[i] ) free ( fname[i] );
        if ( vname[i] ) free ( vname[i] );
        }
    }

// ============================================================
int parseList ( char *list, char *entry[MXVARS3] )
    {
    char *str, *tmp;
    int i, n;


    for ( i=0; i<MXVARS3; i++ )
        {
        entry[i] = NULL;
        }

    str = strdup ( list );
    if ( str == NULL )
        {
        fprintf ( stderr,"strdup ERROR in parseList\n" );
        return -1;
        }

    n = 0;
    tmp = strtok ( str, "," );
    while ( tmp != NULL )
        {
        entry[n] = strdup ( tmp );
        tmp = strtok ( NULL,"," );
        n++;
        if ( n >= MXVARS3 )
            {
            fprintf ( stderr,"ERROR: too many entries in parseList\n" );
            n = -1;
            goto cleanup;
            }
        }

cleanup:
    free ( str );
    return n;
    }

void DriverWnd::addAlias ( char *alias_def )
    {
    char *t;
    char *alias_name;
    char *alias_decl;
    Alias *alias;

#define MAXTOKENS 1024
    char *token[MAXTOKENS];
    int tflag[MAXTOKENS];
    int ntoken;
    int i;

    alias_name = strdup ( alias_def );
    if ( alias_name==NULL )
        {
        fprintf ( stderr,"ERROR: Cannot strdup alias declaraction\n" );
        return;
        }
    t = strchr ( alias_name, '=' );
    if ( t == NULL )
        {
        fprintf ( stderr,"ERROR: Incorrect alias syntax\n" );
        return;
        }
    t[0] = '\0';
    alias_decl = t+1;

    ntoken=evalTokens ( alias_decl, token, tflag );

    if ( ntoken > 0 )
        {
        alias = new Alias ( alias_name, ntoken, token, tflag );
        if ( !formula_->isAliasAdded ( alias_name ) )
            {
            fprintf ( stderr,
                      "WARNING: Alias %s already defined, new definition ignored\n",
                      alias_name );
            delete ( alias );
            }
        else
            {
            formula_->addNewAlias ( alias );
            }
        }

    for ( i=0; i<ntoken; i++ )
        {
        if ( token[i] ) free ( token[i] );
        }
    }

void DriverWnd::removeAlias ( char *aliasname )
    {
    if ( !formula_->findAndRemoveAlias ( aliasname ) )
        {
        fprintf ( stderr,
                  "WARNING: Alias %s is not defined, command ignored\n",
                  aliasname );
        }
    }

void DriverWnd::loadAliasFile ( char *filename )
    {
    FILE *fp;
    char buffer[256];

    fp = fopen ( filename,"r" );
    if ( fp )
        {
        while ( fscanf ( fp,"%s",buffer ) != EOF )
            {
            addAlias ( buffer );
            }
        fclose ( fp );
        }
    }

void DriverWnd::writeAliasFile ( char *filename )
    {
    FILE *fp;

    fp = fopen ( filename,"w" );
    if ( fp )
        {
        formula_->printAliasList ( fp );
        fclose ( fp );
        }
    }

void DriverWnd::create_vectorplotCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->create_vectorplot_cb();
    }


void DriverWnd::create_vectorplot_cb()
    {
    createMultiFormulaSelectionDialog ( 2, VECTOR_PLOT );
    }

void DriverWnd::create_tilevectorplotCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->create_tilevectorplot_cb();
    }


void DriverWnd::create_tilevectorplot_cb()
    {
    createMultiFormulaSelectionDialog ( 3, VECTOR_PLOT );
    }

void DriverWnd::create_scatterplotCB ( Widget, XtPointer clientData, XtPointer )
    {
    DriverWnd *obj = ( DriverWnd * ) clientData;
    obj->create_scatterplot_cb();
    }


void DriverWnd::create_scatterplot_cb()
    {
    createMultiFormulaSelectionDialog ( 2, SCATTER_PLOT );
    }


void DriverWnd::createMultiFormulaSelectionDialog ( int nitems, int type )
    {
    createMultiFormulaSelectionDialog ( nitems, type, NULL );
    }

void DriverWnd::createMultiFormulaSelectionDialog ( int nitems, int type, void *twnd )
    {
    Widget parent = getParent();
    char strbuf[100];
    Widget  edit_selection_dialog;
    assert ( parent );

    sprintf ( strbuf, "%s","Enter Formula for selected component" );
    XmString label = XmStringCreateLocalized ( strbuf );

    sprintf ( strbuf, "%s","Available Formulas" );
    XmString items = XmStringCreateLocalized ( strbuf );

    if ( type == VECTOR_PLOT )
        {
        if ( nitems == 2 )
            {
            sprintf ( strbuf, "%s","VectorPlot" );
            }
        if ( nitems == 3 )
            {
            sprintf ( strbuf, "%s","TileVectorPlot" );
            }
        }
    else if ( type == OBSVECTOR_PLOT )
        {
        sprintf ( strbuf, "%s","VectorPlotObs" );
        }
    else if ( type == SCATTER_PLOT )
        {
        sprintf ( strbuf, "%s","ScatterPlot" );
        }
    else
        {
        sprintf ( strbuf, "%s","UnknownPlot" );
        }
    edit_selection_dialog = XmCreateSelectionDialog ( parent, strbuf, NULL, 0 );
    XtVaSetValues ( edit_selection_dialog,
                    XmNautoUnmanage,                True,
                    XmNselectionLabelString,        label,
                    //                XmNheight,                520,
                    XmNorientation,     XmHORIZONTAL,
                    XmNtopAttachment,   XmATTACH_FORM,
                    XmNleftAttachment,  XmATTACH_FORM,
                    XmNleftOffset,      10,
                    XmNrightAttachment, XmATTACH_FORM,
                    XmNrightOffset,     10,
                    XmNlistLabelString,     items,
                    XmNchildPlacement,      XmPLACE_TOP,
                    NULL );

    XmStringFree ( label );
    XmStringFree ( items );

    MultiSelect *m = new MultiSelect ( edit_selection_dialog, nitems, type, this,
                                       twnd );
    Widget radio_box = XmCreateRadioBox ( edit_selection_dialog, "radio_box",
                                          NULL, 0 );
    Widget one = XtVaCreateManagedWidget ( "Component 1",
                                           xmToggleButtonWidgetClass, radio_box, NULL );
    XmToggleButtonSetState ( one, True, True );
    XtAddCallback ( one, XmNvalueChangedCallback, &MultiSelect::multiselect1CB,
                    ( XtPointer ) m );

    Widget two = XtVaCreateManagedWidget ( "Component 2",
                                           xmToggleButtonWidgetClass, radio_box, NULL );

    XtAddCallback ( two, XmNvalueChangedCallback, &MultiSelect::multiselect2CB,
                    ( XtPointer ) m );
    if ( nitems > 2 )
        {
        Widget three = XtVaCreateManagedWidget ( "Component 3",
                       xmToggleButtonWidgetClass, radio_box, NULL );
        XtAddCallback ( three, XmNvalueChangedCallback, &MultiSelect::multiselect3CB,
                        ( XtPointer ) m );
        }

    XtAddCallback ( edit_selection_dialog, XmNokCallback,
                    &MultiSelect::multiselectOkCB, ( XtPointer ) m );
    XtAddCallback ( edit_selection_dialog, XmNapplyCallback,
                    &MultiSelect::multiselectApplyCB, ( XtPointer ) m );


    if ( XtIsManaged ( radio_box ) ) XtUnmanageChild ( radio_box );
    XtManageChild ( radio_box );


    // Hide the Help button
    //  XtUnmanageChild(XmSelectionBoxGetChild(edit_selection_dialog,
    //                     XmDIALOG_APPLY_BUTTON));

    XtUnmanageChild ( XmSelectionBoxGetChild ( edit_selection_dialog,
                      XmDIALOG_HELP_BUTTON ) );

    putFormula2SelectionDialog ( edit_selection_dialog, type );
    if ( XtIsManaged ( edit_selection_dialog ) ) XtUnmanageChild ( edit_selection_dialog );
    XtManageChild ( edit_selection_dialog );

    }


int DriverWnd::processMultiSelect ( int type, int n, char **name, void *tw )
    {
    int i;
    for ( i=0; i<n; i++ )
        {
        if ( name[i] == NULL ) return 0;
        if ( name[i][0]=='\0' )
            {
            return 0;
            }
        }

    if ( type==VECTOR_PLOT )
        {
        if ( n==2 )
            {
            createTileVectorPlot ( NULL, name[0], name[1] );
            }
        if ( n==3 )
            {
            createTileVectorPlot ( name[0], name[1], name[2] );
            }
        }
    if ( type==SCATTER_PLOT )
        {
        createScatterPlot ( name[0], name[1] );
        }
    if ( type==OBSVECTOR_PLOT )
        {
        char tstring[256];
        TileWnd *twnd = ( TileWnd * ) tw;

        sprintf ( tstring,"%s %s", name[0],name[1] );
        if ( twnd )
            {
            XSync ( XtDisplay ( _w ), False );
            twnd->set_overlay_mode ( OBSVECTOR_PLOT );
            twnd->overlay_create ( tstring );
            }
        }
    return 1;
    }

void DriverWnd::addObsIdTable ( char *filename )
    {

    char buffer[256];
    char *name;
    char *value;
    char **alist;
    char errMsg[512];
    int n;
    StringPair *tableEntry;
    extern int parseLine ( char *, char ***, char * );

    FILE *fp;

    fp = fopen ( filename,"r" );

    if ( fp )
        {
        while ( fgets ( buffer,255,fp ) != NULL )
            {
            if ( !obsIdListP_ ) obsIdListP_ = new linkedList;

            n = parseLine ( buffer, &alist, errMsg );
            if ( n == 2 )
                {
                name = alist[0];
                value = alist[1];
                if ( name[0] != '\n' )
                    {
                    tableEntry = new StringPair ( name, value );
                    if ( ! ( obsIdListP_->find ( tableEntry ) ) )
                        {
                        obsIdListP_->addTail ( tableEntry );
                        }
                    //    delete tableEntry;
                    }
                }
            }
        //    obsIdListP_->print(stdout);
        fclose ( fp );
        }


    }
