/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: ReadVisData.cc 83 2018-03-12 19:24:33Z coats $
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
//  ReadVisData.C
//  K. Eng Pua
//  Copyright (C)
//  Nov 23, 1994
//////////////////////////////////////////////////////////////////////
//
//  Modification history
//
//  SRT 950705 changed setTileData() to use VIS_DATA_dup();
//         Changed map_overlay failure from ERROR to Warning and
//         also reset npolylines to 0
//  SRT 950911 added getUnits() routine
//
//////////////////////////////////////////////////////////////////////

/* bald messes this up in some header file */
#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */

#include "ReadVisData.h"


ReadVisData::ReadVisData ( char *drawtype, BusData *ibd, VIS_DATA *vdata )
    {
    assert ( drawtype );
    drawtype_ = ( char * ) NULL; // added 960918 SRT for memory purposes
    info = ( VIS_DATA * ) NULL; // added 960918 SRT for memory purposes
    title1_ = ( char * ) NULL;  // added 960918 SRT for memory purposes
    title2_ = ( char * ) NULL;  // added 960918 SRT for memory purposes
    title3_ = ( char * ) NULL;  // added 960918 SRT for memory purposes
    xtitle_ = ( char * ) NULL;  // added 960918 SRT for memory purposes
    ytitle_ = ( char * ) NULL;  // added 960918 SRT for memory purposes
#ifndef USE_OLDMAP
    map_x_ = ( float * ) NULL;  // added 960918 SRT for memory purposes
    map_y_ = ( float * ) NULL;  // added 960918 SRT for memory purposes
    map_n_ = ( int * ) NULL;    // added 960918 SRT for memory purposes
#endif /* #ifndef USE_OLDMAP */

    drawtype_ = strdup ( drawtype );

    bd = ibd;
    initialize();

    tmp_info = vdata;
    setTileData();
    }


ReadVisData::~ReadVisData() // 960918 added SRT for memory management
    {
    if ( drawtype_ ) free ( drawtype_ );
    drawtype_ = ( char * ) NULL;
    if ( info )
        {
        free_vis ( info );
        free ( ( char * ) info );
        info = ( VIS_DATA * ) NULL;
        }
    if ( title1_ ) free ( title1_ );
    title1_ = NULL;
    if ( title2_ ) free ( title2_ );
    title2_ = NULL;
    if ( title3_ ) free ( title3_ );
    title3_ = NULL;
    if ( xtitle_ ) free ( xtitle_ );
    xtitle_ = NULL;
    if ( ytitle_ ) free ( ytitle_ );
    ytitle_ = NULL;
    }



void ReadVisData::initialize()
    {
    char    *name ;
    title1_ = NULL;
    title2_ = NULL;
    title3_ = NULL;
    xtitle_ = NULL;
    ytitle_ = NULL;

    ARCINFO_MAP_1[0] = '\0';
    ARCINFO_MAP_2[0] = '\0';

    // Defaults
    sprintf ( plotformatX_, "%c1.0f", '%' );
    sprintf ( plotformatY_, "%c1.0f", '%' );
    offset_left_ = offset_top_ = 100;
    offset_right_ = offset_bottom_ = 60;

#ifndef USE_OLDMAP
    map_x_ = ( float * ) NULL;
    map_y_ = ( float * ) NULL;
    map_n_ = ( int * ) NULL;
    map_npolyline_ = 0; // SRT
    name = getenv ( "MAPNAME" ) ;
    if ( name && *name )
        sprintf ( mapName_, "%s", name );
    else
        name = getenv ( "PAVE_MAPDIR" ) ;
        if ( name && *name )
            sprintf ( mapName_, "%s/OUTLUSAM", name );
        else
            sprintf ( mapName_, "OUTLUSAM" );
#endif
    }




void ReadVisData::setTileData()
    {
    char estring[256];

#ifdef DIAGNOSTICS
    fprintf ( stderr, "+++In ReadVisData::setTileData\n" );
#endif


    info = tmp_info;          // 960918 SRT replaced below for memory mngmnt
    if ( !strcmp ( drawtype_, "Domain" ) ) // 960918 SRT replaced below for memory mngmnt
        if ( info->grid )     // 960918 SRT replaced below for memory mngmnt
            {
            // 960918 SRT replaced below for memory mngmnt
            free ( info->grid ); // 960918 SRT replaced below for memory mngmnt
            info->grid = ( float * ) NULL; // 960918 SRT replaced below for memory mngmnt
            }         // 960918 SRT replaced below for memory mngmnt

    //if (strcmp(drawtype_, "Domain"))
    //  {
    //  fprintf(stderr, "ReadVisData::setTileData() about to call VIS_DATA_dup()\n");
    //  info = VIS_DATA_dup(tmp_info, estring);
    //  }
    //else
    //  {
    //  info = (VIS_DATA *) malloc(sizeof(VIS_DATA));
    //  if (!info)
    //      sprintf(estring, "malloc failure!");
    //  else
    //      {
    //      memcpy (info, tmp_info, sizeof(VIS_DATA));
    //      info->filename           = (char *) NULL;
    //      info->filehost.ip        = (char *) NULL;
    //      info->filehost.name      = (char *) NULL;
    //      info->species_short_name = (char **) NULL;
    //      info->species_long_name  = (char **) NULL;
    //      info->units_name         = (char **) NULL;
    //      if (tmp_info->map_info)
    //          info->map_info = strdup(tmp_info->map_info);
    //      info->data_label         = (char *) NULL;
    //      info->grid               = (float *) NULL;
    //      info->sdate              = (int *) NULL;
    //      info->stime              = (int *) NULL;
    //      }
    //  }

    if ( !info )
        sprintf ( estring, "\007info setup failed in ReadVisData::"
                  "setTileData() with error message '%s'\n", estring );

    else
        {
        col_min_ = info->col_min;
        row_min_ = info->row_min;
        step_min_= info->step_min;
        col_max_ = info->col_max;
        row_max_ = info->row_max;
        step_max_= info->step_max;

#ifdef DIAGNOSTICS
        fprintf ( stderr, "+++info->map_info=%s|\n", tmp_info->map_info );
#endif

        if ( tmp_info->data_label )
            title2_  = strdup ( tmp_info->data_label );
        else
            title2_  = strdup ( " " );

#ifdef DIAGNOSTICS
        fprintf ( stderr, "+++ info->grid_max=%f info->grid_min=%f\n",info->grid_max,info->grid_min );
#endif

        setMapData();
        }
    }


void ReadVisData::setMapData()
    {
    message_[0] = '\0';

#ifdef USE_OLDMAP

    if ( !map_overlay ( *info, map_x_, map_y_, map_n_, &map_npolyline_,
                        ( int ) MAP_MAXPTS_, 0, message_ ) )
        {
        fprintf ( stderr, "\nWARNING - map unavailable: %s", message_ );
        map_npolyline_ = 0; // SRT
        }

#else

    if ( projmap_overlay
            ( info, &map_x_, &map_y_, &map_n_, &map_npolyline_, mapName_, message_ ) )
        {
        fprintf ( stderr, "\nWARNING - map unavailable: %s", message_ );
        map_x_ = ( float * ) NULL;
        map_y_ = ( float * ) NULL;
        map_n_ = ( int * ) NULL;
        map_npolyline_ = 0; // SRT
        }

#endif

#ifdef DIAGNOSTICS

    float minx, maxx, miny, maxy;

    if ( map_npolyline_ )
        {
        int lineno, pointno, index=0;
        minx=map_x_[0];
        maxx=map_x_[0];
        miny=map_y_[0];
        maxy=map_y_[0];

        index = 0;
        for ( lineno = 0; lineno < map_npolyline_; lineno++ )
            {
            for ( pointno = 0; pointno < map_n_[lineno]; pointno++ )
                {
                if ( map_x_[index] < minx ) minx = map_x_[index];
                if ( map_x_[index] > maxx ) maxx = map_x_[index];
                if ( map_y_[index] < miny ) miny = map_y_[index];
                if ( map_y_[index] > maxy ) maxy = map_y_[index];
                index++;
                }
            }
        }

    fprintf ( stderr, "\nIn ReadVisData::setTileData(), with %s map:\n",

#ifdef USE_OLDMAP
              "OLD" );
#else
              "NEW" );
#endif // USE_OLDMAP

    fprintf ( stderr, "map_npolyline_ == %d\n", map_npolyline_ );
    if ( map_npolyline_ )
        fprintf ( stderr, "Range is (%f, %f) -> (%f, %f)\n", minx, miny, maxx, maxy );

#endif // DIAGNOSTICS
    }




char *ReadVisData::getUnits()
    {
    return ( ( info ) &&
             ( info->units_name ) &&
             ( info->selected_species ) &&
             ( info->units_name[info->selected_species-1] ) &&
             ( info->units_name[info->selected_species-1][0] ) ) ?
           info->units_name[info->selected_species-1] :
           ( char * ) NULL;
    }

