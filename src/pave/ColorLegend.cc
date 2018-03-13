/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: ColorLegend.cc 83 2018-03-12 19:24:33Z coats $
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
//////////////////////////////////////////////////////////////////////////////
// File:        ColorLegend.C
// Author:      Kathy Pearson and K. Eng Pua
// Date:        Mar 2, 1995
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950630  Added disable map drawing radio button
// SRT  950707  Added smooth plots radio button
// SRT  951115  Added setContourRange() routine
// SRT  951212  Added draw grid lines radio button & scale vectors button
//
//////////////////////////////////////////////////////////////////////////////


#include "ColorLegend.h"

float *legend_bins_;
int    legend_irregular_bins_;
int    legend_nbins_;

static int newtonmap[] =
    {

    // 0
    /*0x0000ff*/ 0xdfdfdf, 0x0003ff, 0x0007ff, 0x000bff, 0x000fff, 0x0013ff, 0x0017ff, 0x001bff,

    // 8
    0x001fff, 0x0023ff, 0x0027ff, 0x002bff, 0x002fff, 0x0033ff, 0x0037ff, 0x003bff,

    // 16
    0x003fff, 0x0043ff, 0x0047ff, 0x004bff, 0x004fff, 0x0053ff, 0x0057ff, 0x005bff,

    // 24
    0x005fff, 0x0063ff, 0x0067ff, 0x006bff, 0x006fff, 0x0073ff, 0x0077ff, 0x007bff,

    // 32
    0x007fff, 0x0083ff, 0x0087ff, 0x008bff, /*0x008fff*/ 0x005fff, 0x0093ff, 0x0097ff, 0x009bff,

    // 40
    0x009fff, 0x00a3ff, 0x00a7ff, 0x00abff, 0x00afff, 0x00b3ff, 0x00b7ff, 0x00bbff,

    // 48
    0x00bfff, 0x00c3ff, 0x00c7ff, 0x00cbff, 0x00cfff, 0x00d3ff, 0x00d7ff, 0x00dbff,

    // 56
    0x00dfff, 0x00e3ff, 0x00e7ff, 0x00ebff, 0x00efff, 0x00f3ff, 0x00f7ff, 0x00fbff,

    // 64
    0x00fffe, 0x00fffa, 0x00fff6, 0x00fff2, 0x00ffee, 0x00ffea, 0x00ffe6, 0x00ffe2,

    // 72
    /*0x00ffdd*/ 0x008fff, 0x00ffd9, 0x00ffd5, 0x00ffd1, 0x00ffce, 0x00ffca, 0x00ffc6, 0x00ffc2,

    // 80
    0x00ffbe, 0x00ffba, 0x00ffb6, 0x00ffb2, 0x00ffae, /*0x00ffaa*/ 0x00ffdd, 0x00ffa6, 0x00ffa2,

    // 88
    0x00ff9d, 0x00ff99, 0x00ff95, 0x00ff91, 0x00ff8e, 0x00ff8a, 0x00ff86, 0x00ff82,

    // 96
    0x00ff7e, 0x00ff7a, 0x00ff76, 0x00ff72, 0x00ff6e, 0x00ff6a, 0x00ff66, 0x00ff62,

    // 104
    0x00ff5d, 0x00ff59, 0x00ff55, 0x00ff51, 0x00ff4e, /*0x00ff4a*/ 0x00ffdd, 0x00ff46, 0x00ff42,

    0x00ff3e, 0x00ff3a, 0x00ff36, 0x00ff32, 0x00ff2e, 0x00ff2a, 0x00ff26, 0x00ff22,
    0x00ff1e, 0x00ff1a, 0x00ff16, 0x00ff12, 0x00ff0e, 0x00ff0a, 0x00ff06, 0x00ff02,
    0x02ff00, 0x06ff00, 0x0aff00, 0x0eff00, 0x12ff00, 0x16ff00, 0x1aff00, 0x1eff00,
    0x22ff00, 0x26ff00, 0x2aff00, 0x2dff00, 0x32ff00, 0x36ff00, 0x3aff00, 0x3eff00,
    0x42ff00, 0x46ff00, 0x4aff00, 0x4eff00, 0x52ff00, 0x56ff00, 0x5aff00, 0x5eff00,
    0x62ff00, 0x66ff00, 0x6aff00, 0x6dff00, 0x72ff00, 0x76ff00, 0x7aff00, 0x7eff00,
    0x82ff00, 0x86ff00, 0x8aff00, 0x8eff00, 0x92ff00, 0x96ff00, 0x9aff00, 0x9eff00,
    0xa2ff00, 0xa6ff00, 0xaaff00, 0xadff00, 0xb2ff00, 0xb6ff00, 0xbaff00, 0xbeff00,
    0xc2ff00, 0xc6ff00, 0xcaff00, 0xceff00, 0xd2ff00, 0xd6ff00, 0xdaff00, 0xdeff00,
    0xe2ff00, 0xe6ff00, 0xeaff00, 0xeeff00, 0xf2ff00, 0xf6ff00, 0xfaff00, 0xfeff00,
    0xfffb00, 0xfff700, 0xfff300, 0xffef00, 0xffeb00, 0xffe800, 0xffe300, 0xffdf00,
    0xffdb00, 0xffd700, 0xffd300, 0xffcf00, 0xffcb00, 0xffc800, 0xffc300, 0xffbf00,
    0xffbb00, 0xffb700, 0xffb300, 0xffaf00, 0xffab00, 0xffa800, 0xffa300, 0xff9f00,
    0xff9b00, 0xff9700, 0xff9300, 0xff8f00, 0xff8b00, 0xff8700, 0xff8300, 0xff7f00,
    0xff7b00, 0xff7700, 0xff7400, 0xff6f00, 0xff6b00, 0xff6700, 0xff6400, 0xff5f00,
    0xff5b00, 0xff5700, 0xff5400, 0xff4f00, 0xff4b00, 0xff4700, 0xff4300, 0xff3f00,
    0xff3b00, 0xff3700, 0xff3300, 0xff2f00, 0xff2b00, 0xff2700, 0xff2300, 0xff1f00,
    0xff1b00, 0xff1700, 0xff1300, 0xff0f00, 0xff0b00, 0xff0700, 0xff0300, 0xff0000
    };


static int jetmap[] =
    {
    0x000083, 0x000087, 0x00008b, 0x00008f, 0x000093, 0x000097, 0x00009b, 0x00009f,
    0x0000a3, 0x0000a7, 0x0000ab, 0x0000af, 0x0000b3, 0x0000b7, 0x0000bb, 0x0000bf,
    0x0000c3, 0x0000c7, 0x0000cb, 0x0000cf, 0x0000d3, 0x0000d7, 0x0000db, 0x0000df,
    0x0000e3, 0x0000e7, 0x0000eb, 0x0000ef, 0x0000f3, 0x0000f7, 0x0000fb, 0x0000ff,
    0x0003ff, 0x0007ff, 0x000bff, 0x000fff, 0x0013ff, 0x0017ff, 0x001bff, 0x001fff,
    0x0023ff, 0x0027ff, 0x002bff, 0x002fff, 0x0033ff, 0x0037ff, 0x003bff, 0x003fff,
    0x0043ff, 0x0047ff, 0x004bff, 0x004fff, 0x0053ff, 0x0057ff, 0x005bff, 0x005fff,
    0x0063ff, 0x0067ff, 0x006bff, 0x006fff, 0x0073ff, 0x0077ff, 0x007bff, 0x007fff,
    0x0083ff, 0x0087ff, 0x008bff, 0x008fff, 0x0093ff, 0x0097ff, 0x009bff, 0x009fff,
    0x00a3ff, 0x00a7ff, 0x00abff, 0x00afff, 0x00b3ff, 0x00b7ff, 0x00bbff, 0x00bfff,
    0x00c3ff, 0x00c7ff, 0x00cbff, 0x00cfff, 0x00d3ff, 0x00d7ff, 0x00dbff, 0x00dfff,
    0x00e3ff, 0x00e7ff, 0x00ebff, 0x00efff, 0x00f3ff, 0x00f7ff, 0x00fbff, 0x00ffff,
    0x03ffff, 0x07fffb, 0x0bfff7, 0x0ffff3, 0x13ffef, 0x17ffeb, 0x1bffe7, 0x1fffe3,
    0x23ffdf, 0x27ffdb, 0x2bffd7, 0x2fffd3, 0x33ffcf, 0x37ffcb, 0x3bffc7, 0x3fffc3,
    0x43ffbf, 0x47ffbb, 0x4bffb7, 0x4fffb3, 0x53ffaf, 0x57ffab, 0x5bffa7, 0x5fffa3,
    0x63ff9f, 0x67ff9b, 0x6bff97, 0x6fff93, 0x73ff8f, 0x77ff8b, 0x7bff87, 0x7fff83,
    0x83ff7f, 0x87ff7b, 0x8bff77, 0x8fff73, 0x93ff6f, 0x97ff6b, 0x9bff67, 0x9fff63,
    0xa3ff5f, 0xa7ff5b, 0xabff57, 0xafff53, 0xb3ff4f, 0xb7ff4b, 0xbbff47, 0xbfff43,
    0xc3ff3f, 0xc7ff3b, 0xcbff37, 0xcfff33, 0xd3ff2f, 0xd7ff2b, 0xdbff27, 0xdfff23,
    0xe3ff1f, 0xe7ff1b, 0xebff17, 0xefff13, 0xf3ff0f, 0xf7ff0b, 0xfbff07, 0xfeff03,
    0xfeff00, 0xfffb00, 0xfff700, 0xfff300, 0xffef00, 0xffeb00, 0xffe700, 0xffe300,
    0xffdf00, 0xffdb00, 0xffd700, 0xffd300, 0xffcf00, 0xffcb00, 0xffc700, 0xffc300,
    0xffbf00, 0xffbb00, 0xffb700, 0xffb300, 0xffaf00, 0xffab00, 0xffa700, 0xffa300,
    0xff9f00, 0xff9b00, 0xff9700, 0xff9300, 0xff8f00, 0xff8b00, 0xff8700, 0xff8300,
    0xff7f00, 0xff7b00, 0xff7700, 0xff7300, 0xff6f00, 0xff6b00, 0xff6700, 0xff6300,
    0xff5f00, 0xff5b00, 0xff5700, 0xff5300, 0xff4f00, 0xff4b00, 0xff4700, 0xff4300,
    0xff3f00, 0xff3b00, 0xff3700, 0xff3300, 0xff2f00, 0xff2b00, 0xff2700, 0xff2300,
    0xff1f00, 0xff1b00, 0xff1700, 0xff1300, 0xff0f00, 0xff0b00, 0xff0700, 0xff0300,
    0xff0000, 0xfb0000, 0xf70000, 0xf30000, 0xef0000, 0xeb0000, 0xe70000, 0xe30000,
    0xdf0000, 0xdb0000, 0xd70000, 0xd30000, 0xcf0000, 0xcb0000, 0xc70000, 0xc30000,
    0xbf0000, 0xbb0000, 0xb70000, 0xb30000, 0xaf0000, 0xab0000, 0xa70000, 0xa30000,
    0x9f0000, 0x9b0000, 0x970000, 0x930000, 0x8f0000, 0x8b0000, 0x870000, 0x830000
    };


static int greymap[] =
    {

    // 0
    0xffffff, 0xfdfdfd, 0xfdfdfd, 0xfbfbfb, 0xfbfbfb, 0xf9f9f9, 0xf9f9f9, 0xf8f8f8,

    // 8
    0xf7f7f7, 0xf6f6f6, 0xf4f4f4, 0xf4f4f4, 0xf2f2f2, 0xf2f2f2, 0xf0f0f0, 0xf0f0f0,

    // 16
    0xefefef, 0xededed, 0xededed, 0xebebeb, 0xebebeb, 0xe9e9e9, 0xe9e9e9, 0xe8e8e8,

    // 24
    0xe6e6e6, 0xe6e6e6, 0xe4e4e4, 0xe4e4e4, 0xe2e2e2, 0xe2e2e2, 0xe1e1e1, 0xdfdfdf,

    // 32
    0xdfdfdf, 0xdddddd, 0xdddddd, 0xdbdbdb, 0xdbdbdb, 0xdadada, 0xd8d8d8, 0xd8d8d8,

    0xd6d6d6, 0xd6d6d6, 0xd4d4d4, 0xd4d4d4, 0xd3d3d3, 0xd1d1d1, 0xd1d1d1, 0xcfcfcf,
    0xcfcfcf, 0xcdcdcd, 0xcdcdcd, 0xcccccc, 0xcbcbcb, 0xcacaca, 0xc8c8c8, 0xc8c8c8,
    0xc6c6c6, 0xc6c6c6, 0xc4c4c4, 0xc4c4c4, 0xc3c3c3, 0xc1c1c1, 0xc1c1c1, 0xbfbfbf,
    0xbfbfbf, 0xbdbdbd, 0xbcbcbc, 0xbcbcbc, 0xbababa, 0xbababa, 0xb8b8b8, 0xb8b8b8,
    0xb6b6b6, 0xb6b6b6, 0xb5b5b5, 0xb3b3b3, 0xb3b3b3, 0xb1b1b1, 0xb1b1b1, 0xafafaf,
    0xafafaf, 0xaeaeae, 0xacacac, 0xacacac, 0xaaaaaa, 0xaaaaaa, 0xa8a8a8, 0xa8a8a8,
    0xa7a7a7, 0xa5a5a5, 0xa5a5a5, 0xa3a3a3, 0xa3a3a3, 0xa1a1a1, 0xa1a1a1, 0xa0a0a0,
    0x9e9e9e, 0x9e9e9e, 0x9c9c9c, 0x9c9c9c, 0x9a9a9a, 0x9a9a9a, 0x999999, 0x979797,
    0x979797, 0x959595, 0x959595, 0x939393, 0x939393, 0x919191, 0x909090, 0x909090,
    0x8e8e8e, 0x8e8e8e, 0x8c8c8c, 0x8c8c8c, 0x8a8a8a, 0x898989, 0x898989, 0x878787,
    0x878787, 0x858585, 0x858585, 0x838383, 0x838383, 0x828282, 0x808080, 0x808080,
    0x7e7e7e, 0x7e7e7e, 0x7c7c7c, 0x7c7c7c, 0x7b7b7b, 0x797979, 0x797979, 0x777777,
    0x777777, 0x757575, 0x757575, 0x747474, 0x727272, 0x727272, 0x707070, 0x707070,
    0x6e6e6e, 0x6e6e6e, 0x6d6d6d, 0x6b6b6b, 0x6b6b6b, 0x696969, 0x696969, 0x676767,
    0x676767, 0x666666, 0x646464, 0x646464, 0x626262, 0x626262, 0x606060, 0x606060,
    0x5e5e5e, 0x5d5d5d, 0x5d5d5d, 0x5b5b5b, 0x5b5b5b, 0x595959, 0x595959, 0x575757,
    0x565656, 0x565656, 0x545454, 0x545454, 0x525252, 0x525252, 0x505050, 0x505050,
    0x4f4f4f, 0x4d4d4d, 0x4d4d4d, 0x4b4b4b, 0x4b4b4b, 0x494949, 0x494949, 0x484848,
    0x464646, 0x464646, 0x444444, 0x444444, 0x424242, 0x424242, 0x414141, 0x3f3f3f,
    0x3f3f3f, 0x3d3d3d, 0x3d3d3d, 0x3b3b3b, 0x3b3b3b, 0x3a3a3a, 0x383838, 0x383838,
    0x363636, 0x363636, 0x343434, 0x343434, 0x333333, 0x313131, 0x313131, 0x2f2f2f,
    0x2f2f2f, 0x2d2d2d, 0x2d2d2d, 0x2b2b2b, 0x2a2a2a, 0x2a2a2a, 0x282828, 0x282828,
    0x262626, 0x262626, 0x242424, 0x232323, 0x232323, 0x212121, 0x212121, 0x1f1f1f,
    0x1f1f1f, 0x1d1d1d, 0x1c1c1c, 0x1c1c1c, 0x1a1a1a, 0x1a1a1a, 0x181818, 0x181818,
    0x161616, 0x161616, 0x151515, 0x131313, 0x131313, 0x111111, 0x111111, 0x0f0f0f,
    0x0f0f0f, 0x0e0e0e, 0x0c0c0c, 0x0c0c0c, 0x0a0a0a, 0x0a0a0a, 0x080808, 0x080808,
    0x070707, 0x050505, 0x050505, 0x030303, 0x030303, 0x010101, 0x010101, 0x000000
    };

// SRT 961014 added the following 4 lines to keep an original
// copy of the three colormaps, so the original values can
// easily be restored if the user uses some config file for
// a while, then uses another without the color info in it.
static int first_time = 1;
static char newtonmap_backup[sizeof ( newtonmap )];
static char jetmap_backup[sizeof ( jetmap )];
static char greymap_backup[sizeof ( greymap )];
// ALT 123098 added the next 2 lines to help command line args set default vectorScale
float default_vector_scale_ = 1.0;
int default_vector_skip_ = 1;

void ColorLegend::refreshColor() { } // SRT 950929
void ColorLegend::toggleMapDrawing() { } // SRT 950929
void ColorLegend::toggleSmoothPlots() { } // SRT 950929
void ColorLegend::toggleGridLines() { } // SRT 951212
void ColorLegend::toggleScaleVectors() { } // SRT 951212

ColorLegend::ColorLegend()
    {
    if ( first_time )
        {
        first_time = 0;
        memcpy ( newtonmap_backup, newtonmap, sizeof ( newtonmap ) );
        memcpy ( jetmap_backup,    jetmap,    sizeof ( jetmap ) );
        memcpy ( greymap_backup,   greymap,   sizeof ( greymap ) );
        }
    }



ColorLegend::~ColorLegend()                  /* SRT memory 960924 */
    {
    /* SRT memory 960924 */
    if ( data_table_ )                       /* SRT memory 960924 */
        free ( ( char * ) data_table_ );
    data_table_ = ( float * ) NULL; /* SRT memory 960924 */
    if ( color_gc_table_ != NULL )                                   /* SRT memory 960924 */
        delete [] color_gc_table_;
    color_gc_table_ = ( GC * ) NULL; /* SRT memory 960924 */
    }                                /* SRT memory 960924 */


void ColorLegend::initColorLegendObject()
    {
    colorChooser_ = ( ColorChooser * ) NULL;
    legend_dialog_ = ( Widget ) NULL;
    color_gc_table_ = ( GC * ) NULL;
    data_table_ = ( float * ) NULL;

    vector_scale_ = default_vector_scale_;
    legend_nskip_ = default_vector_skip_;

    legend_ntile_ = ( cfgp_ && cfgp_->hasNumber_Tiles() ) ? cfgp_->getNumber_Tiles() : 8;

    legend_invert_ = ( cfgp_ && cfgp_->hasInvert_Colormap() ) ? cfgp_->getInvert_Colormap() : 0;

    if ( ( cfgp_ && cfgp_->hasColorMapType() ) )
        switch ( cfgp_->getColorMapType() )
            {
            case 'J':
                legend_cmap_ = JET_COLORMAP;
                break;
            case 'G':
                legend_cmap_ = GREY_COLORMAP;
                break;
            case 'N':
            default:
                legend_cmap_ = NEWTON_COLORMAP;
            }
    else
        legend_cmap_ = NEWTON_COLORMAP;


    if ( cfgp_ && cfgp_->hasNumber_Tiles() && cfgp_->hasColors() )
        {
        int     *red   = cfgp_->getred(),
                *green = cfgp_->getgreen(),
                *blue  = cfgp_->getblue(),
                *current_cmap,
                 i, index;

        if      ( legend_cmap_ == NEWTON_COLORMAP ) current_cmap = &newtonmap[0];
        else if ( legend_cmap_ == JET_COLORMAP    ) current_cmap = &jetmap[0];
        else if ( legend_cmap_ == GREY_COLORMAP   ) current_cmap = &greymap[0];
        else
            {
            fprintf ( stderr,
                      "\007ColorLegend::initColorLegendObject() couldn't fine colormap!!\n" );
            return;
            }

        for ( i=0; i<legend_ntile_; i++ )
            {
            if ( i == 0 ) index = 0;
            else if ( i == legend_ntile_-1 ) index = 255;
            else
                index = ( int ) ( i * ( 255.0/ ( legend_ntile_ - 1 ) ) );
            if ( legend_invert_ ) index = 255 - index;
            current_cmap[index] = ( ( red[i]   & 0x0000ff ) << 16 ) |
                                  ( ( green[i] & 0x0000ff ) << 8 ) |
                                  ( blue[i] );
            }
        }

    // SRT 961014 added the following 6 lines to restore the original
    // copy of the three colormaps, in case the user has
    // used some config file with another colormap for a while,
    // then uses another without the color info in it, in which case
    // we need to use the original colormap information
    else
        {
        memcpy ( newtonmap, newtonmap_backup, sizeof ( newtonmap ) );
        memcpy ( jetmap,    jetmap_backup,    sizeof ( jetmap ) );
        memcpy ( greymap,   greymap_backup,   sizeof ( greymap ) );
        }

    legend_nlabel_ = ( cfgp_ && cfgp_->hasNumber_Labels() )   ? cfgp_->getNumber_Labels()   : 5;

    legend_map_off_ = ( cfgp_ && cfgp_->hasDisable_Map() )    ? cfgp_->getDisable_Map()     :
                      ( ( getenv ( "DISABLE_MAPS" ) != NULL ) && ( !strcmp ( getenv ( "DISABLE_MAPS" ), "1" ) ) );

    smooth_plots_on_ = ( cfgp_ && cfgp_->hasSmooth_Plot() )   ? cfgp_->getSmooth_Plot()     :
                       ( ( getenv ( "SMOOTH_PLOTS" ) != NULL ) && ( !strcmp ( getenv ( "SMOOTH_PLOTS" ), "1" ) ) );

    grid_lines_on_ = ( cfgp_ && cfgp_->hasDraw_Grid_Lines() ) ? cfgp_->getDraw_Grid_Lines() :
                     ( ( getenv ( "DRAW_GRID_LINES" ) != NULL ) && ( !strcmp ( getenv ( "DRAW_GRID_LINES" ), "1" ) ) );

    scale_vectors_on_ = ( cfgp_ && cfgp_->hasScale_Vectors() ) ? cfgp_->getScale_Vectors()   :
                        ( ( getenv ( "SCALE_VECTORS" ) != NULL ) && ( !strcmp ( getenv ( "SCALE_VECTORS" ), "1" ) ) );
    if ( default_vector_scale_ != 1.0 ) scale_vectors_on_ = 1;

    fill_arrowheads_ = 0;

    strcpy ( legend_format_, ( cfgp_ && cfgp_->hasLegend_Format() )
             ? cfgp_->getLegend_Format() : "%8.3f" );

    saveMPEGControls_ = ( cfgp_ && cfgp_->hasSave_MPEG_Files() )    ? cfgp_->getSave_MPEG_Files() : 0;

    legend_range_ = 1;
    legend_min_choice_ = ( Widget ) NULL;
    legend_max_choice_ = ( Widget ) NULL;
    legend_title_ = ( char ** ) NULL;

    unitString_[0] = '\0';


    char *str, *cstr;
    char *tmp;
    int len, n;

    str = getenv ( "LEGEND_BINS" );
    if ( str != NULL && strcasecmp ( str,"Default" ) )
        {
        legend_irregular_bins_ = 1;
        len = strlen ( str );
        n = ( len + 1 ) /2;

        legend_bins_ = ( float * ) malloc ( n*sizeof ( legend_bins_[0] ) );
        if ( legend_bins_ == NULL )
            {
            fprintf ( stderr,"Malloc ERROR in initColorLegendObject\n" );
            return;
            }

        cstr = strdup ( str );
        if ( cstr == NULL )
            {
            fprintf ( stderr,"strdup ERROR in initColorLegendObject\n" );
            return;
            }

        n = 0;
        tmp = strtok ( cstr, "," );
        while ( tmp != NULL )
            {
            sscanf ( tmp,"%f",&legend_bins_[n] );
            tmp = strtok ( NULL,"," );
            n++;
            }

        free ( cstr );

        legend_ntile_ = legend_nbins_ = n-1;
        //     legend_nlabel_ = n;
        legend_nlabel_ = ( cfgp_ && cfgp_->hasNumber_Labels() )   ? cfgp_->getNumber_Labels()   : n;

        }
    else
        {
        legend_irregular_bins_ = 0;
        }
    }



void ColorLegend::initColorLegend ( Widget parent, Drawable pix, GC gc, DrawScale *s, float val_min, float val_max, char *unitString, char **title, char **subtitle1, char **subtitle2 )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter ColorLegend::initColorLegend()\n" );
#endif // #ifdef DIAGNOSTICS

    assert ( parent );

    if ( unitString )
        if ( unitString[0] )
            if ( !unitString_[0] )
                strcpy ( unitString_, unitString );

    cl_canvas_ = parent;
    cl_dpy_ = XtDisplay ( cl_canvas_ );
    cl_drw_ = pix;
    cl_gc_ = gc;
    cl_s_ = s;

    if ( ( val_min != -1.0 ) || ( val_max != -1.0 ) )
        {
        val_min_ = val_min;
        val_max_ = val_max;
        }

    if ( color_gc_table_ == NULL )
        setup_color_gc_table();

    if ( data_table_ == NULL )
        setup_data_table();

    legend_title_ = title;
    legend_subtitle1_ = subtitle1;
    legend_subtitle2_ = subtitle2;
    }



int ColorLegend::colorIndex ( float valu )
    {
    int num = legend_ntile_;

    if ( valu <= data_table_[0] )
        return ( 0 );
    if ( valu >= data_table_[num] )
        return ( num-1 );
    for ( int i = 0; i < num; i++ )
        if ( ( valu >= data_table_[i] ) && ( valu < data_table_[i+1] ) )
            return ( i );
    return ( -1 );
    }


int ColorLegend::get_ramp_pixel ( int icolor )
    {
    int *current_cmap;
    int rgb;
    XColor color;

    int scr  = DefaultScreen ( cl_dpy_ );
    Colormap cmap = DefaultColormap ( cl_dpy_, scr );

    if ( legend_cmap_ == NEWTON_COLORMAP )
        current_cmap = &newtonmap[0];
    else if ( legend_cmap_ == JET_COLORMAP )
        current_cmap = &jetmap[0];
    else if ( legend_cmap_ == GREY_COLORMAP )
        current_cmap = &greymap[0];
    else
        {
        fprintf ( stderr, "Error selecting color map!\n" );
        return ( BlackPixel ( cl_dpy_, scr ) );
        }

    if ( legend_invert_ )
        rgb = current_cmap[255-icolor];
    else
        rgb = current_cmap[icolor];

    /* & to get color element (red, green, blue) with results converted
    from 8-bit to 32-bit numbers */

    color.red   = ( unsigned short ) ( ( rgb & 0xff0000 ) >> 8 );
    color.green = ( unsigned short ) ( ( rgb & 0x00ff00 ) );
    color.blue  = ( unsigned short ) ( ( rgb & 0x0000ff ) << 8 );

    if ( !XAllocColor ( cl_dpy_, cmap, &color ) )
        {
        fprintf ( stderr, "Can't allocate ramp color %d\n", icolor );
        return ( BlackPixel ( cl_dpy_, scr ) );
        }
    return ( color.pixel );
    }


void ColorLegend::setup_color_gc_table()
    {

    int num = legend_ntile_;
    //   val_delta_ = (val_max_ - val_min_) / (float) num;

    if ( color_gc_table_ != NULL )
        delete [] color_gc_table_; // SRT 951212 changed from free()

    color_gc_table_ = new GC[num];

    int i, ramp_index;
    for ( i=0; i<num; i++ )
        {
        color_gc_table_[i] = XCreateGC ( cl_dpy_,
                                         DefaultRootWindow ( cl_dpy_ ),
                                         ( unsigned long ) NULL, ( XGCValues * ) NULL );
        if ( num > 1 )
            {
            if ( i==num-1 )
                {
                ramp_index = 255;
                }
            else
                {
                ramp_index = ( int ) ( i * ( 255.0/ ( num - 1 ) ) );
                }
            }
        else
            ramp_index = 0;
        XSetForeground ( cl_dpy_,
                         color_gc_table_[i],
                         get_ramp_pixel ( ramp_index ) );
        }
    }



void ColorLegend::setup_data_table()
    {
    int i;
    int num = legend_ntile_;
    float value;
    float incr = ( val_max_ - val_min_ ) / ( ( float ) num );

    if ( data_table_ != NULL )
        {
        free ( data_table_ );
        data_table_ = NULL;
        }
    if ( ( data_table_ = ( ( float * ) malloc ( sizeof ( float ) * ( num + 1 ) ) ) ) != NULL )
        {
        if ( legend_irregular_bins_ )
            {
            num = legend_ntile_ = legend_nbins_;
            val_min_ = legend_bins_[0];
            val_max_ = legend_bins_[num-1];
            for ( i=0; i<= num; i++ )
                {
                data_table_[i] = legend_bins_[i];
                }
            }
        else
            {
            value = val_min_;
            for ( i = 0; i <= num; i++, value += incr )
                data_table_[i] = value;
            }
        if ( ( legend_min_choice_ != NULL ) || ( legend_max_choice_ != NULL ) )
            reset_legend_minmax();
        }
    }



int ColorLegend::get_named_pixel ( char *colorname )
    {
    XColor       color, skip;
    int          scr  = DefaultScreen ( cl_dpy_ );
    Colormap     cmap = DefaultColormap ( cl_dpy_, scr );

    if ( XAllocNamedColor ( cl_dpy_, cmap, colorname, &color, &skip ) )
        return ( color.pixel );
    else
        {
        fprintf ( stderr, "Can't allocate color %s\n",colorname );
        return ( BlackPixel ( cl_dpy_, scr ) );
        }
    }


void ColorLegend::legend_tilesCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->legend_tiles_cb ( cbs->value );
    }

void ColorLegend::legend_tiles_cb ( int valu )
    {

    if ( valu != legend_ntile_ )
        {
        legend_ntile_ = valu;
        setup_color_gc_table();
        setup_data_table();

        refreshColor();

        if ( legend_nlabel_ > legend_ntile_ + 1 )
            {
            legend_nlabel_ = legend_ntile_ + 1;
            XtVaSetValues ( legend_labels_,
                            XmNvalue, legend_nlabel_, NULL );
            }

        if ( legend_range_ > legend_ntile_ )
            {
            legend_range_ = legend_ntile_;
            char str[40];
            sprintf ( str, "*%2d*", legend_range_ );
            XmString string = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( str );
            XtVaSetValues ( legend_range_choice_,
                            XmNlabelString, string, NULL );
            XmStringFree ( string );
            }
        reset_legend_minmax();
        }
    }

void ColorLegend::legend_labelsCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->legend_labels_cb ( cbs->value );
    }


void ColorLegend::legend_labels_cb ( int valu )
    {
    if ( valu != legend_nlabel_ )
        {
        legend_nlabel_ = valu;
        if ( legend_nlabel_ > legend_ntile_ + 1 )
            legend_nlabel_ = legend_ntile_ + 1;
        XtVaSetValues ( legend_labels_, XmNvalue, legend_nlabel_, NULL );
        refreshColor();
        }
    }


void ColorLegend::legend_skipCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->legend_skip_cb ( cbs->value );
    }

void ColorLegend::legend_skip_cb ( int valu )
    {
    if ( valu != legend_nskip_ )
        {
        legend_nskip_ = valu;
        refreshColor();
        }
    }


void ColorLegend::legend_disable_mapCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->legend_disable_map_cb ( cbs->value );
    }


void ColorLegend::legend_disable_map_cb ( int valu )
    {
    if ( legend_map_off_ != valu )
        {
        legend_map_off_ = valu;
        toggleMapDrawing();
        XtVaSetValues ( legend_disable_map_, XmNvalue, legend_map_off_, NULL );
        }
    }


void ColorLegend::legend_smooth_plotCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->legend_smooth_plot_cb ( cbs->value );
    }


void ColorLegend::legend_smooth_plot_cb ( int valu )
    {
    if ( smooth_plots_on_ != valu )
        {
        smooth_plots_on_ = valu;
        toggleSmoothPlots();
        XtVaSetValues ( legend_smooth_plot_, XmNvalue, smooth_plots_on_, NULL );
        }
    }


void ColorLegend::legend_grid_linesCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->legend_grid_lines_cb ( cbs->value );
    }


void ColorLegend::legend_grid_lines_cb ( int valu )
    {
    if ( grid_lines_on_ != valu )
        {
        grid_lines_on_ = valu;
        toggleGridLines();
        XtVaSetValues ( legend_grid_lines_, XmNvalue, grid_lines_on_, NULL );
        }
    }


void ColorLegend::legend_scale_vectorsCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->legend_scale_vectors_cb ( cbs->value );
    }


void ColorLegend::legend_scale_vectors_cb ( int valu )
    {
    if ( scale_vectors_on_ != valu )
        {
        scale_vectors_on_ = valu;
        toggleScaleVectors();
        XtVaSetValues ( legend_scale_vectors_, XmNvalue, scale_vectors_on_, NULL );
        }
    }


void ColorLegend::legend_fill_arrowheadsCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->legend_fill_arrowheads_cb ( cbs->value );
    }


void ColorLegend::legend_fill_arrowheads_cb ( int valu )
    {
    if ( fill_arrowheads_ != valu )
        {
        fill_arrowheads_ = valu;
        toggleScaleVectors();
        XtVaSetValues ( legend_fill_arrowheads_, XmNvalue, fill_arrowheads_, NULL );
        }
    }





void ColorLegend::legend_invertCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->legend_invert_cb ( cbs->value );
    }


void ColorLegend::legend_invert_cb ( int valu )
    {
    if ( legend_invert_ != valu )
        {
        legend_invert_ = valu;
        setup_color_gc_table();

        refreshColor();

        legend_panel_redraw();
        }
    }



void ColorLegend::legend_cmap_newtonCB ( Widget, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_cmap_newton_cb();
    }


void ColorLegend::legend_cmap_newton_cb()
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter ColorLegend::legend_cmap_newton_cb()\n" );
#endif // DIAGNOSTICS

    if ( legend_cmap_ != NEWTON_COLORMAP )
        {
        legend_cmap_ = NEWTON_COLORMAP;
        setup_color_gc_table();

        refreshColor();

        legend_panel_redraw();
        }
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Exit ColorLegend::legend_cmap_newton_cb()\n" );
#endif // DIAGNOSTICS
    }



void ColorLegend::legend_cmap_jetCB ( Widget, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_cmap_jet_cb();
    }


void ColorLegend::legend_cmap_jet_cb()
    {

    if ( legend_cmap_ != JET_COLORMAP )
        {
        legend_cmap_ = JET_COLORMAP;
        setup_color_gc_table();

        refreshColor();

        legend_panel_redraw();
        }
    }



void ColorLegend::legend_cmap_greyCB ( Widget, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_cmap_grey_cb();
    }


void ColorLegend::legend_cmap_grey_cb()
    {

    if ( legend_cmap_ != GREY_COLORMAP )
        {
        legend_cmap_ = GREY_COLORMAP;
        setup_color_gc_table();

        refreshColor();

        legend_panel_redraw();
        }
    }


void ColorLegend::legend_range_upCB ( Widget, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_range_up_cb();
    }


void ColorLegend::legend_range_up_cb()
    {
    legend_range_++;
    if ( legend_range_ > legend_ntile_ )
        legend_range_ = 1;

    char str[40];
    sprintf ( str, "*%2d*", legend_range_ );
    XmString string = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( str );
    XtVaSetValues ( legend_range_choice_, XmNlabelString, string, NULL );
    XmStringFree ( string );
    reset_legend_minmax();
    update_legend_range_color ( legend_range_ );
    }



void ColorLegend::legend_range_downCB ( Widget, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_range_down_cb();
    }


void ColorLegend::legend_range_down_cb()
    {
    legend_range_--;
    if ( legend_range_ < 1 )
        legend_range_ = legend_ntile_;

    char str[40];
    sprintf ( str, "*%2d*", legend_range_ );
    XmString string = /*XmStringCreateSimple SRT 950724*/ XmStringCreateLocalized ( str );
    XtVaSetValues ( legend_range_choice_, XmNlabelString, string, NULL );
    XmStringFree ( string );
    reset_legend_minmax();
    update_legend_range_color ( legend_range_ );
    }


void ColorLegend::update_legend_range_color ( int i )
    {
    XtDestroyWidget ( editableColor_ );
    XFillRectangle ( cl_dpy_, pixmap_, color_gc_table_[i-1], 0, 0, 10, 10 );
    editableColor_ = XtVaCreateManagedWidget ( "EditableColor",
                     xmPushButtonWidgetClass, bin_, XmNlabelType, XmPIXMAP,
                     XmNlabelPixmap, pixmap_, NULL );
    XtAddCallback ( editableColor_, XmNactivateCallback, legend_editableColorCB, ( XtPointer ) this );
    }


void ColorLegend::closeLegendDialogCB ( Widget, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->close_legend_dialog_cb();
    }


void ColorLegend::close_legend_dialog_cb()
    {
    XtUnmanageChild ( legend_dialog_ );
    }


void ColorLegend::legend_colorbinCB ( Widget w, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_colorbin_cb ( w );
    }


void ColorLegend::legend_colorbin_cb ( Widget w )
    {
    int item_no;

    XtVaGetValues ( w, XmNuserData, &item_no, NULL );

    int ramp_index;
    int which_tile = legend_range_;

    legend_bincolor_ = item_no;
    ramp_index = ( int ) ( legend_bincolor_ * ( 255.0/ ( 64 - 1 ) ) );
    XSetForeground ( cl_dpy_, color_gc_table_[which_tile - 1],
                     get_ramp_pixel ( ramp_index ) );

    update_legend_range_color ( which_tile );
    refreshColor();

    }


void ColorLegend::legend_editableColorCB ( Widget, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_editableColor_cb();
    }


void ColorLegend::legend_editableColor_cb()
    {
    XColor      color;
    int         scr = DefaultScreen ( cl_dpy_ );
    Colormap    cmap = DefaultColormap ( cl_dpy_, scr );
    XGCValues   values;

    if ( !colorChooser_ )
        colorChooser_ = new ColorChooser ( cl_canvas_, "colorChooser" );
    XGetGCValues ( cl_dpy_, color_gc_table_[legend_range_-1], GCForeground, &values );
    color.pixel = values.foreground;
    XQueryColor ( cl_dpy_, cmap, &color );
    colorChooser_->setRgb ( color.red/256, color.green/256, color.blue/256 );
    colorChooser_->pickColor ( &colorSelectedCB, NULL, ( void * ) this );
    }


void ColorLegend::colorSelectedCB ( int red, int green, int blue, void *cp )
    {
    ColorLegend *c = ( ColorLegend * ) cp;
    c->colorSelected_cb ( red, green, blue );
    }


void ColorLegend::colorSelected_cb ( int red, int green, int blue )
    {
    XColor color;
    int scr  = DefaultScreen ( cl_dpy_ );
    Colormap cmap = DefaultColormap ( cl_dpy_, scr );

    color.red = red * 256;
    color.green = green * 256;
    color.blue = blue * 256;
    if ( !XAllocColor ( cl_dpy_, cmap, &color ) )
        fprintf ( stderr, "\007Can't allocate color!!\n" );
    else
        {
        XSetForeground ( cl_dpy_, color_gc_table_[legend_range_-1], color.pixel );
        update_legend_range_color ( legend_range_ );
        refreshColor();
        }
    }



int ColorLegend::setContourRange ( float minCut, float maxCut ) // SRT 951115
    {
    char text[64];
    float tf;

    if ( minCut > maxCut )
        {
        tf = minCut;
        minCut = maxCut;
        maxCut = tf;
        }

    sprintf ( text, legend_format_, minCut );
    if ( legend_min_choice_ ) XmTextSetString ( legend_min_choice_, text );
    val_min_ = minCut;

    sprintf ( text, legend_format_, maxCut );
    if ( legend_max_choice_ ) XmTextSetString ( legend_max_choice_, text );
    val_max_ = maxCut;

    setup_color_gc_table();
    setup_data_table();
    if ( legend_min_choice_ && legend_max_choice_ ) drawColorLegend ( 5,10 );
    refreshColor();
    return 0;
    }


void ColorLegend::legend_minCB ( Widget w, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_min_cb ( w );
    }


void ColorLegend::legend_min_cb ( Widget w )
    {
    char *text = XmTextGetString ( w );
    float tval;
    char tstring[100];

    sscanf ( text, "%f", &tval ); // SRT 950721
    XtFree ( text );
    if ( tval <= val_max_ )
        {
        val_min_ = tval;
        setup_data_table(); // SRT 950721
        drawColorLegend ( 5,10 ); // SRT 950721
        refreshColor();
        }
    sprintf ( tstring, legend_format_, val_min_ );
    XmTextSetString ( legend_min_choice_, tstring );
    }


void ColorLegend::legend_maxCB ( Widget w, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_max_cb ( w );
    }

void ColorLegend::legend_max_cb ( Widget w )
    {
    char *text = XmTextGetString ( w );
    float tval;
    char tstring[100];

    sscanf ( text, "%f", &tval ); // SRT 950721
    XtFree ( text );
    if ( tval >= val_min_ )
        {
        val_max_ = tval;
        setup_data_table(); // SRT 950721
        drawColorLegend ( 5,10 ); // SRT 950721
        refreshColor();
        }
    sprintf ( tstring, legend_format_, val_max_ );
    XmTextSetString ( legend_max_choice_, tstring );
    }

void ColorLegend::reset_legend_minmax()
    {
    char str[40];
    sprintf ( str, legend_format_, val_min_ /* SRT 950721 data_table_[legend_range_-1] */ );
    XmTextSetString ( legend_min_choice_, str );
    sprintf ( str, legend_format_, val_max_ /* SRT 950721 data_table_[legend_range_] */ );
    XmTextSetString ( legend_max_choice_, str );
    }


void ColorLegend::legend_formatCB ( Widget w, XtPointer clientData, XtPointer )
    {
    char *text = XmTextGetString ( w );
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_format_cb ( text );
    XtFree ( text );
    }


void ColorLegend::legend_format_cb ( char *text )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter ColorLegend::legend_format_cb '%s'\n", text );
#endif DIAGNOSTICS

    if ( text != NULL )
        strcpy ( legend_format_, text );
    else
        {
        strcpy ( legend_format_, "%8.3f" );
        XmTextSetString ( legend_format_choice_, text );
        }
    refreshColor();
    reset_legend_minmax();

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Exit ColorLegend::legend_format_cb\n" );
#endif DIAGNOSTICS
    }


void ColorLegend::createColorLegendDialog()
    {
    Widget legend_cmap_choice, separator, close,
           newton, jet, grey, label, rc,
           w, frame;
    Widget tzWidget;
    char str[64];


    // Create control dialog box.

    legend_dialog_ = XmCreateFormDialog ( cl_canvas_, "Legend Control", NULL, 0 );
    XtVaSetValues ( legend_dialog_,
                    XmNwidth,               550,
                    XmNheight,             740,  //795,
                    XmNautoUnmanage,        False,
                    NULL );

    // Create tiles widget.

    legend_tiles_ = XtVaCreateManagedWidget ( "Legend Tiles",
                    xmScaleWidgetClass, legend_dialog_,
                    XtVaTypedArg, XmNtitleString, XmRString, "# Tiles", 8,
                    XmNmaximum,             64,
                    XmNminimum,             1,
                    XmNvalue,               legend_ntile_,
                    XmNorientation,         XmHORIZONTAL,
                    XmNtopAttachment,       XmATTACH_FORM,
                    XmNleftAttachment,      XmATTACH_FORM,
                    XmNleftOffset,          300,
                    XmNrightAttachment,     XmATTACH_FORM,
                    XmNrightOffset,         130,
                    XmNshowValue,           True,
                    NULL );

    XtAddCallback ( legend_tiles_, XmNvalueChangedCallback, &ColorLegend::legend_tilesCB, ( XtPointer ) this );

    // Create labels widget.

    legend_labels_ = XtVaCreateManagedWidget ( "Legend Labels",
                     xmScaleWidgetClass, legend_dialog_,
                     XtVaTypedArg, XmNtitleString, XmRString, "# Labels", 9,
                     XmNmaximum,             21,
                     XmNminimum,             2,
                     XmNvalue,               legend_nlabel_,
                     XmNorientation,         XmHORIZONTAL,
                     XmNtopAttachment,       XmATTACH_FORM,
                     XmNleftAttachment,      XmATTACH_FORM,
                     XmNleftOffset,          430,
                     XmNrightAttachment,     XmATTACH_FORM,
                     XmNrightOffset,         0,
                     XmNshowValue,           True,
                     NULL );

    XtAddCallback ( legend_labels_, XmNvalueChangedCallback, &ColorLegend::legend_labelsCB, ( XtPointer ) this );

    // Create toggle widget indicating whether to invert color map
    int box_y = 295, y_delta = 20;
    legend_invert_cmap_ = XtVaCreateManagedWidget ( "Invert Colormap",
                          xmToggleButtonWidgetClass, legend_dialog_,
                          XmNvalue,               legend_invert_,
                          XmNorientation,         XmHORIZONTAL,
                          XmNtopAttachment,       XmATTACH_FORM,
                          XmNtopOffset,           box_y,
                          XmNleftAttachment,      XmATTACH_FORM,
                          XmNleftOffset,          300,
                          XmNrightAttachment,     XmATTACH_FORM,
                          XmNrightOffset,         40,
                          XmNshowValue,           True,
                          NULL );
    box_y = box_y + y_delta;
    XtAddCallback ( legend_invert_cmap_, XmNvalueChangedCallback, &ColorLegend::legend_invertCB, ( XtPointer ) this );
    if ( legend_invert_ ) XmToggleButtonSetState ( legend_invert_cmap_, 1, 0 );

    // Create toggle widget indicating whether to save mpeg config
    legend_save_mpegControls_ = XtVaCreateManagedWidget ( "Keep MPEG Input",
                                xmToggleButtonWidgetClass, legend_dialog_,
                                XmNvalue,               saveMPEGControls_,
                                XmNorientation,         XmHORIZONTAL,
                                XmNtopAttachment,       XmATTACH_FORM,
                                XmNtopOffset,           box_y,
                                XmNleftAttachment,      XmATTACH_FORM,
                                XmNleftOffset,          300,
                                XmNrightAttachment,     XmATTACH_FORM,
                                XmNrightOffset,         40,
                                XmNshowValue,           True,
                                NULL );
    box_y = box_y + y_delta;
    XtAddCallback ( legend_save_mpegControls_, XmNvalueChangedCallback, &ColorLegend::legend_save_mpegControlsCB, ( XtPointer ) this );
    if ( saveMPEGControls_ ) XmToggleButtonSetState ( legend_save_mpegControls_, 1, 0 );

    // Create toggle widget indicating whether to disable the map drawing
    legend_disable_map_ = XtVaCreateManagedWidget ( "Disable Map",
                          xmToggleButtonWidgetClass, legend_dialog_,
                          XmNvalue,               legend_map_off_,
                          XmNorientation,         XmHORIZONTAL,
                          XmNtopAttachment,       XmATTACH_FORM,
                          XmNtopOffset,           box_y,
                          XmNleftAttachment,      XmATTACH_FORM,
                          XmNleftOffset,          300,
                          XmNrightAttachment,     XmATTACH_FORM,
                          XmNrightOffset,         40,
                          XmNshowValue,           True,
                          NULL );
    box_y = box_y + y_delta;
    XtAddCallback ( legend_disable_map_, XmNvalueChangedCallback, &ColorLegend::legend_disable_mapCB, ( XtPointer ) this );
    if ( legend_map_off_ ) XmToggleButtonSetState ( legend_disable_map_, 1, 0 );

    // Create toggle widget indicating whether to smooth the plot drawing
    legend_smooth_plot_ = XtVaCreateManagedWidget ( "Smooth Plot",
                          xmToggleButtonWidgetClass, legend_dialog_,
                          XmNvalue,               smooth_plots_on_,
                          XmNorientation,         XmHORIZONTAL,
                          XmNtopAttachment,       XmATTACH_FORM,
                          XmNtopOffset,           box_y,
                          XmNleftAttachment,      XmATTACH_FORM,
                          XmNleftOffset,          300,
                          XmNrightAttachment,     XmATTACH_FORM,
                          XmNrightOffset,         40,
                          XmNshowValue,           True,
                          NULL );
    box_y = box_y + y_delta;
    XtAddCallback ( legend_smooth_plot_, XmNvalueChangedCallback, &ColorLegend::legend_smooth_plotCB, ( XtPointer ) this );
    if ( smooth_plots_on_ ) XmToggleButtonSetState ( legend_smooth_plot_, 1, 0 );


    // Create toggle widget indicating whether to draw grid lines
    legend_grid_lines_ = XtVaCreateManagedWidget ( "Draw Grid Lines",
                         xmToggleButtonWidgetClass, legend_dialog_,
                         XmNvalue,               grid_lines_on_,
                         XmNorientation,         XmHORIZONTAL,
                         XmNtopAttachment,       XmATTACH_FORM,
                         XmNtopOffset,           box_y,
                         XmNleftAttachment,      XmATTACH_FORM,
                         XmNleftOffset,          300,
                         XmNrightAttachment,     XmATTACH_FORM,
                         XmNrightOffset,         40,
                         XmNshowValue,           True,
                         NULL );
    box_y = box_y + y_delta;
    XtAddCallback ( legend_grid_lines_, XmNvalueChangedCallback,
#ifdef _AIX
                    //(void *)
#endif
#ifdef __osf__
                    ( void ( * ) ( _WidgetRec *, void *, void * ) )
#endif
                    &ColorLegend::legend_grid_linesCB, ( XtPointer ) this );
    if ( grid_lines_on_ ) XmToggleButtonSetState ( legend_grid_lines_, 1, 0 );

    // Create toggle widget indicating whether to scale the vectors by magnitude
    legend_scale_vectors_ = XtVaCreateManagedWidget ( "Scale Vectors",
                            xmToggleButtonWidgetClass, legend_dialog_,
                            XmNvalue,               scale_vectors_on_,
                            XmNorientation,         XmHORIZONTAL,
                            XmNtopAttachment,       XmATTACH_FORM,
                            XmNtopOffset,           box_y,
                            XmNleftAttachment,      XmATTACH_FORM,
                            XmNleftOffset,          300,
                            XmNrightAttachment,     XmATTACH_FORM,
                            XmNrightOffset,         40,
                            XmNshowValue,           True,
                            NULL );
    box_y = box_y + y_delta;
    XtAddCallback ( legend_scale_vectors_, XmNvalueChangedCallback,
#ifdef _AIX
                    //(void *)
#endif
#ifdef __osf__
                    ( void ( * ) ( _WidgetRec *, void *, void * ) )
#endif
                    &ColorLegend::legend_scale_vectorsCB, ( XtPointer ) this );

    // Create toggle widget indicating whether to scale the vectors by magnitude
    legend_fill_arrowheads_ = XtVaCreateManagedWidget ( "Fill Vector Arrowheads",
                              xmToggleButtonWidgetClass, legend_dialog_,
                              XmNvalue,               fill_arrowheads_,
                              XmNorientation,         XmHORIZONTAL,
                              XmNtopAttachment,       XmATTACH_FORM,
                              XmNtopOffset,           box_y,
                              XmNleftAttachment,      XmATTACH_FORM,
                              XmNleftOffset,          300,
                              XmNrightAttachment,     XmATTACH_FORM,
                              XmNrightOffset,         40,
                              XmNshowValue,           True,
                              NULL );
    box_y = box_y + y_delta;
    XtAddCallback ( legend_fill_arrowheads_, XmNvalueChangedCallback,
#ifdef _AIX
                    //(void *)
#endif
#ifdef __osf__
                    ( void ( * ) ( _WidgetRec *, void *, void * ) )
#endif
                    &ColorLegend::legend_fill_arrowheadsCB, ( XtPointer ) this );


    if ( scale_vectors_on_ ) XmToggleButtonSetState ( legend_scale_vectors_, 1, 0 );

    /*
       // Create toggle widget indicating whether to invert color map
       legend_invert_cmap_ = XtVaCreateManagedWidget("Invert Colormap",
            xmToggleButtonWidgetClass, legend_dialog_,
                    XmNvalue,               legend_invert_,
                    XmNorientation,         XmHORIZONTAL,
                    XmNtopAttachment,       XmATTACH_FORM,
                    XmNtopOffset,           50,
                    XmNleftAttachment,      XmATTACH_FORM,
                    XmNleftOffset,          300,
                    XmNrightAttachment,     XmATTACH_FORM,
                    XmNrightOffset,         40,
                    XmNshowValue,           True,
                    NULL);
       XtAddCallback(legend_invert_cmap_, XmNvalueChangedCallback, &ColorLegend::legend_invertCB, (XtPointer)this);
       if (legend_invert_) XmToggleButtonSetState(legend_invert_cmap_, 1, 0);
    */


    // Create radio box for choice of color map

    legend_cmap_choice =
        XtVaCreateManagedWidget ( "frame", xmFrameWidgetClass, legend_dialog_,
                                  XmNorientation,         XmHORIZONTAL,
                                  XmNtopAttachment,       XmATTACH_FORM,
                                  XmNleftAttachment,      XmATTACH_FORM,
                                  XmNleftOffset,          10,
                                  XmNrightAttachment,     XmATTACH_FORM,
                                  XmNrightOffset,         280,
                                  NULL );
    label = XtVaCreateManagedWidget ( "Color Map Type", xmLabelWidgetClass,
                                      legend_cmap_choice,
                                      XmNchildType,XmFRAME_TITLE_CHILD,
                                      NULL );

    rc = XmCreateRadioBox ( legend_cmap_choice, "rc", NULL, 0 );

    newton =XtVaCreateManagedWidget ( "Newton RGB (AVS)",
                                      xmToggleButtonWidgetClass, rc,
                                      XmNvalue, 1,
                                      XmNindicatorType, XmONE_OF_MANY,
                                      XmNset, ( legend_cmap_ == NEWTON_COLORMAP ) ? True : False,
                                      // XmNvalue, (legend_cmap_ == NEWTON_COLORMAP) ? True : False,
                                      NULL );
    XtAddCallback ( newton, XmNvalueChangedCallback, &ColorLegend::legend_cmap_newtonCB, ( XtPointer ) this );

    jet =XtVaCreateManagedWidget ( "Newton RGB (InkJet)",
                                   xmToggleButtonWidgetClass, rc,
                                   XmNset, ( legend_cmap_ == JET_COLORMAP ) ? True : False,
                                   NULL );
    XtAddCallback ( jet, XmNvalueChangedCallback, &ColorLegend::legend_cmap_jetCB, ( XtPointer ) this );

    grey =XtVaCreateManagedWidget ( "Grey Scale (AVS)",
                                    xmToggleButtonWidgetClass, rc,
                                    XmNset, ( legend_cmap_ == GREY_COLORMAP ) ? True : False,
                                    NULL );
    XtAddCallback ( grey, XmNvalueChangedCallback, &ColorLegend::legend_cmap_greyCB, ( XtPointer ) this );

    if ( XtIsManaged ( rc ) ) XtUnmanageChild ( rc );
    XtManageChild ( rc );

    // Create frame for choice of min/max range for each bin

    legend_range_choice_ =
        XtVaCreateManagedWidget ( "frame", xmFrameWidgetClass, legend_dialog_,
                                  XmNorientation,         XmHORIZONTAL,
                                  XmNtopAttachment,       XmATTACH_FORM,
                                  XmNleftAttachment,      XmATTACH_FORM,
                                  XmNtopOffset,           160,
                                  XmNleftOffset,          10,
                                  XmNrightAttachment,     XmATTACH_FORM,
                                  XmNrightOffset,         280,
                                  XmNheight,              200,
                                  NULL );

    label = XtVaCreateManagedWidget ( "Legend Range", xmLabelWidgetClass,
                                      legend_range_choice_,
                                      XmNchildType,XmFRAME_TITLE_CHILD,
                                      NULL );

    rc = XtVaCreateManagedWidget ( "rc", xmRowColumnWidgetClass,
                                   legend_range_choice_,
                                   XmNnumColumns,  1,
                                   XmNpacking,     XmPACK_TIGHT,
                                   XmNleftAttachment,  XmATTACH_FORM,
                                   XmNtopAttachment,   XmATTACH_FORM,
                                   XmNtopOffset,           165,
                                   XmNleftOffset,          10,
                                   XmNrightAttachment,     XmATTACH_FORM,
                                   XmNrightOffset,         380,
                                   NULL );

    XtVaCreateManagedWidget ( "Max", xmLabelWidgetClass, rc, NULL );

    legend_max_choice_ =
        XtVaCreateManagedWidget ( "Max", xmTextWidgetClass, rc,
                                  NULL );
    sprintf ( str, legend_format_, val_max_ /* SRT 072195 legend_max_ */ );
    XmTextSetString ( legend_max_choice_, str );

    XtVaCreateManagedWidget ( "Min", xmLabelWidgetClass, rc, NULL );
    legend_min_choice_ =
        XtVaCreateManagedWidget ( "Min", xmTextWidgetClass, rc, NULL );
    sprintf ( str, legend_format_, val_min_ /* SRT 072195 legend_min_ */ );
    XmTextSetString ( legend_min_choice_, str );

    label = XtVaCreateManagedWidget ( "Units", xmLabelWidgetClass,
                                      rc, NULL );

    unit_title_choice_ =
        XtVaCreateManagedWidget ( "Units", xmTextWidgetClass, rc,
                                  NULL );
    if ( unitString_[0] != '\0' )
        XmTextSetString ( unit_title_choice_, unitString_ );
    else
        XmTextSetString ( unit_title_choice_, "" );

    XtAddCallback ( legend_min_choice_, XmNactivateCallback, &ColorLegend::legend_minCB, ( XtPointer ) this );
    XtAddCallback ( legend_max_choice_, XmNactivateCallback, &ColorLegend::legend_maxCB, ( XtPointer ) this );
    XtAddCallback ( unit_title_choice_, XmNactivateCallback, &ColorLegend::legend_unitCB, ( XtPointer ) this );

    if ( XtIsManaged ( rc ) ) XtUnmanageChild ( rc );
    XtManageChild ( rc );

    // Create frame for color legend format widget
    box_y = 370;
    y_delta = 65;
    frame =
        XtVaCreateManagedWidget ( "frame", xmFrameWidgetClass, legend_dialog_,
                                  XmNorientation,         XmHORIZONTAL,
                                  XmNtopAttachment,       XmATTACH_FORM,
                                  XmNleftAttachment,      XmATTACH_FORM,
                                  XmNtopOffset,           box_y,
                                  XmNleftOffset,          10,
                                  XmNrightAttachment,     XmATTACH_FORM,
                                  XmNrightOffset,         280,
                                  NULL );
    box_y += y_delta;
    rc = XtVaCreateManagedWidget ( "rc", xmRowColumnWidgetClass, frame,
                                   XmNnumColumns,  1,
                                   XmNpacking,     XmPACK_COLUMN,
                                   XmNleftAttachment,  XmATTACH_FORM,
                                   XmNtopAttachment,   XmATTACH_FORM,
                                   NULL );

    label = XtVaCreateManagedWidget ( "Format", xmLabelWidgetClass,
                                      frame,
                                      XmNchildType,XmFRAME_TITLE_CHILD,
                                      NULL );

    legend_format_choice_ =
        XtVaCreateManagedWidget ( "Format", xmTextWidgetClass, rc,
                                  NULL );
    if ( legend_format_ != NULL )
        {
        sprintf ( str, "%s", legend_format_ );
        XmTextSetString ( legend_format_choice_, str );
        }
    XtAddCallback ( legend_format_choice_, XmNactivateCallback, &ColorLegend::legend_formatCB, ( XtPointer ) this );
    if ( XtIsManaged ( rc ) ) XtUnmanageChild ( rc );
    XtManageChild ( rc );


    // Create frame for title typein widget
    frame =
        XtVaCreateManagedWidget ( "frame", xmFrameWidgetClass, legend_dialog_,
                                  XmNorientation,         XmHORIZONTAL,
                                  XmNtopAttachment,       XmATTACH_FORM,
                                  XmNleftAttachment,      XmATTACH_FORM,
                                  XmNtopOffset,           box_y,
                                  XmNleftOffset,          10,
                                  XmNrightAttachment,     XmATTACH_FORM,
                                  XmNrightOffset,         10,
                                  NULL );
    box_y += y_delta;
    rc = XtVaCreateManagedWidget ( "rc", xmRowColumnWidgetClass, frame,
                                   XmNnumColumns,  1,
                                   XmNpacking,     XmPACK_COLUMN,
                                   XmNleftAttachment,  XmATTACH_FORM,
                                   XmNtopAttachment,   XmATTACH_FORM,
                                   NULL );

    label = XtVaCreateManagedWidget ( "Title", xmLabelWidgetClass,
                                      frame,
                                      XmNchildType,XmFRAME_TITLE_CHILD,
                                      NULL );

    legend_title_choice_ =
        XtVaCreateManagedWidget ( "Title", xmTextWidgetClass, rc,
                                  NULL );
    if ( *legend_title_ != NULL )
        XmTextSetString ( legend_title_choice_, *legend_title_ );
    else
        XmTextSetString ( legend_title_choice_, "" );

    XtAddCallback ( legend_title_choice_, XmNactivateCallback, &ColorLegend::legend_titleCB, ( XtPointer ) this );
    if ( XtIsManaged ( rc ) ) XtUnmanageChild ( rc );
    XtManageChild ( rc );



    // Create frame for subtitle1 typein widget
    frame =
        XtVaCreateManagedWidget ( "frame", xmFrameWidgetClass, legend_dialog_,
                                  XmNorientation,         XmHORIZONTAL,
                                  XmNtopAttachment,       XmATTACH_FORM,
                                  XmNleftAttachment,      XmATTACH_FORM,
                                  XmNtopOffset,           box_y,
                                  XmNleftOffset,          10,
                                  XmNrightAttachment,     XmATTACH_FORM,
                                  XmNrightOffset,         10,
                                  NULL );
    box_y += y_delta;
    rc = XtVaCreateManagedWidget ( "rc", xmRowColumnWidgetClass, frame,
                                   XmNnumColumns,  1,
                                   XmNpacking,     XmPACK_COLUMN,
                                   XmNleftAttachment,  XmATTACH_FORM,
                                   XmNtopAttachment,   XmATTACH_FORM,
                                   NULL );

    label = XtVaCreateManagedWidget ( "Subtitle 1", xmLabelWidgetClass,
                                      frame,
                                      XmNchildType,XmFRAME_TITLE_CHILD,
                                      NULL );

    legend_subtitle1_choice_ =
        XtVaCreateManagedWidget ( "Subtitle 1", xmTextWidgetClass, rc,
                                  NULL );
    if ( *legend_subtitle1_ != NULL )
        XmTextSetString ( legend_subtitle1_choice_, *legend_subtitle1_ );
    else
        XmTextSetString ( legend_subtitle1_choice_, "" );

    XtAddCallback ( legend_subtitle1_choice_, XmNactivateCallback, &ColorLegend::legend_subTitle1CB, ( XtPointer ) this );
    if ( XtIsManaged ( rc ) ) XtUnmanageChild ( rc );
    XtManageChild ( rc );


    // Create frame for subtitle2 typein widget
    frame =
        XtVaCreateManagedWidget ( "frame", xmFrameWidgetClass, legend_dialog_,
                                  XmNorientation,         XmHORIZONTAL,
                                  XmNtopAttachment,       XmATTACH_FORM,
                                  XmNleftAttachment,      XmATTACH_FORM,
                                  XmNtopOffset,           box_y,
                                  XmNleftOffset,          10,
                                  XmNrightAttachment,     XmATTACH_FORM,
                                  XmNrightOffset,         10,
                                  NULL );
    box_y += y_delta;
    rc = XtVaCreateManagedWidget ( "rc", xmRowColumnWidgetClass, frame,
                                   XmNnumColumns,  1,
                                   XmNpacking,     XmPACK_COLUMN,
                                   XmNleftAttachment,  XmATTACH_FORM,
                                   XmNtopAttachment,   XmATTACH_FORM,
                                   NULL );

    label = XtVaCreateManagedWidget ( "Subtitle 2", xmLabelWidgetClass,
                                      frame,
                                      XmNchildType,XmFRAME_TITLE_CHILD,
                                      NULL );

    legend_subtitle2_choice_ =
        XtVaCreateManagedWidget ( "Subtitle 2", xmTextWidgetClass, rc,
                                  NULL );
    if ( *legend_subtitle2_ != NULL )
        XmTextSetString ( legend_subtitle2_choice_, *legend_subtitle2_ );
    else
        XmTextSetString ( legend_subtitle2_choice_, "" );

    XtAddCallback ( legend_subtitle2_choice_, XmNactivateCallback, &ColorLegend::legend_subTitle2CB, ( XtPointer ) this );
    if ( XtIsManaged ( rc ) ) XtUnmanageChild ( rc );
    XtManageChild ( rc );


#if 0
    // Create frame for unit typein widget
    frame =
        XtVaCreateManagedWidget ( "frame", xmFrameWidgetClass, legend_dialog_,
                                  XmNorientation,         XmHORIZONTAL,
                                  XmNtopAttachment,       XmATTACH_FORM,
                                  XmNleftAttachment,      XmATTACH_FORM,
                                  XmNtopOffset,           box_y,
                                  XmNleftOffset,          10,
                                  XmNwidth,               275,
                                  //                XmNrightAttachment,     XmATTACH_FORM,
                                  //                XmNrightOffset,         10,
                                  NULL );
    rc = XtVaCreateManagedWidget ( "rc", xmRowColumnWidgetClass, frame,
                                   XmNnumColumns,  1,
                                   XmNpacking,     XmPACK_COLUMN,
                                   XmNleftAttachment,  XmATTACH_FORM,
                                   XmNtopAttachment,   XmATTACH_FORM,
                                   NULL );

    label = XtVaCreateManagedWidget ( "Units", xmLabelWidgetClass,
                                      frame,
                                      XmNchildType,XmFRAME_TITLE_CHILD,
                                      NULL );

    unit_title_choice_ =
        XtVaCreateManagedWidget ( "Units", xmTextWidgetClass, rc,
                                  NULL );
    if ( unitString_[0] != '\0' )
        XmTextSetString ( unit_title_choice_, unitString_ );
    else
        XmTextSetString ( unit_title_choice_, "" );

    XtAddCallback ( unit_title_choice_, XmNactivateCallback, &ColorLegend::legend_unitCB, ( XtPointer ) this );
    if ( XtIsManaged ( rc ) ) XtUnmanageChild ( rc );
    XtManageChild ( rc );

#endif

    // Create frame for vector scale typein widget
    frame =
        XtVaCreateManagedWidget ( "frame", xmFrameWidgetClass, legend_dialog_,
                                  XmNorientation,         XmHORIZONTAL,
                                  XmNtopAttachment,       XmATTACH_FORM,
                                  XmNleftAttachment,      XmATTACH_FORM,
                                  XmNtopOffset,           box_y,
                                  XmNleftOffset,          10,
                                  //                XmNrightAttachment,     XmATTACH_FORM,
                                  //                XmNrightOffset,         10,
                                  NULL );
    rc = XtVaCreateManagedWidget ( "rc", xmRowColumnWidgetClass, frame,
                                   XmNnumColumns,  1,
                                   XmNpacking,     XmPACK_COLUMN,
                                   XmNleftAttachment,  XmATTACH_FORM,
                                   XmNtopAttachment,   XmATTACH_FORM,
                                   NULL );

    label = XtVaCreateManagedWidget ( "Vector Scale", xmLabelWidgetClass,
                                      frame,
                                      XmNchildType,XmFRAME_TITLE_CHILD,
                                      NULL );

    vector_scale_choice_ =
        XtVaCreateManagedWidget ( "Vector Scale", xmTextWidgetClass, rc,
                                  NULL );
    sprintf ( str,"%f",default_vector_scale_ );
    XmTextSetString ( vector_scale_choice_, str );

    XtAddCallback ( vector_scale_choice_, XmNactivateCallback, &ColorLegend::vector_scale_typeinCB, ( XtPointer ) this );
    if ( XtIsManaged ( rc ) ) XtUnmanageChild ( rc );
    XtManageChild ( rc );


    // Create plot every n vectors widget.

    legend_skip_ = XtVaCreateManagedWidget ( "Legend Skip",
                   xmScaleWidgetClass, legend_dialog_,
                   XtVaTypedArg, XmNtitleString, XmRString, "Plot every n vectors", 1,
                   XmNmaximum,             10,
                   XmNminimum,             1,
                   XmNvalue,               legend_nskip_,
                   XmNorientation,         XmHORIZONTAL,
                   XmNtopAttachment,       XmATTACH_FORM,
                   XmNleftAttachment,      XmATTACH_WIDGET,
                   XmNleftWidget,          label,
                   XmNtopOffset,           box_y,
                   XmNleftOffset,          10,
                   XmNrightAttachment,     XmATTACH_FORM,
                   XmNrightOffset,         10,
                   XmNshowValue,           True,
                   NULL );

    XtAddCallback ( legend_skip_, XmNvalueChangedCallback, &ColorLegend::legend_skipCB, ( XtPointer ) this );

    // Create frame for choice bin for range adjustment
    frame =
        XtVaCreateManagedWidget ( "frame", xmFrameWidgetClass, legend_dialog_,
                                  XmNorientation,         XmHORIZONTAL,
                                  XmNtopAttachment,       XmATTACH_FORM,
                                  XmNleftAttachment,      XmATTACH_FORM,
                                  XmNtopOffset,           108, //125,
                                  XmNleftOffset,          10,
                                  XmNrightAttachment,     XmATTACH_FORM,
                                  XmNrightOffset,         280,
                                  NULL );

    label = XtVaCreateManagedWidget ( "Edit Color #", xmLabelWidgetClass,
                                      frame,
                                      XmNchildType,XmFRAME_TITLE_CHILD,
                                      NULL );

    bin_ = XtVaCreateWidget ( "rowcol",
                              xmRowColumnWidgetClass, frame,
                              XmNorientation, XmHORIZONTAL,
                              NULL );

    w = XtVaCreateManagedWidget ( "arrow_up",
                                  xmArrowButtonWidgetClass, bin_,
                                  XmNarrowDirection,   XmARROW_UP,
                                  NULL );
    XtAddCallback ( w, XmNarmCallback, &ColorLegend::legend_range_upCB, ( XtPointer ) this );

    w = XtVaCreateManagedWidget ( "arrow_dn",
                                  xmArrowButtonWidgetClass, bin_,
                                  XmNarrowDirection,   XmARROW_DOWN,
                                  NULL );
    XtAddCallback ( w, XmNarmCallback, &ColorLegend::legend_range_downCB, ( XtPointer ) this );

    ValueRange range;
    range.value = 1;
    range.min = 1;
    range.max = legend_ntile_;
    legend_range_choice_ = XtVaCreateManagedWidget ( "label",
                           xmLabelWidgetClass, bin_,
                           XtVaTypedArg, XmNlabelString, XmRString, "* 1*", 5,
                           XmNuserData, &range,
                           NULL );
    // SRT 960911   if (XtIsManaged(bin_)) XtUnmanageChild(bin_); XtManageChild(bin_);

    // Create color pix choices for each tile color bin
    legend_color_bin_ =
        XtVaCreateManagedWidget ( "frame", xmFrameWidgetClass, legend_dialog_,
                                  XmNorientation,         XmHORIZONTAL,
                                  XmNtopAttachment,       XmATTACH_FORM,
                                  XmNleftAttachment,      XmATTACH_FORM,
                                  XmNtopOffset,           60,
                                  XmNleftOffset,          300,
                                  XmNrightAttachment,     XmATTACH_FORM,
                                  XmNrightOffset,         40,
                                  NULL );
    label = XtVaCreateManagedWidget ( "Color Choices", xmLabelWidgetClass,
                                      legend_color_bin_,
                                      XmNchildType,XmFRAME_TITLE_CHILD,
                                      NULL );
    set_legend_color_widget();


    // now create the single editable color box
    pixmap_ = XCreatePixmap ( cl_dpy_, RootWindowOfScreen ( XtScreen ( bin_ ) ),
                              10, 10, DefaultDepthOfScreen ( XtScreen ( bin_ ) ) );
    XSetForeground ( cl_dpy_, cl_gc_, get_ramp_pixel ( 0 ) );
    XFillRectangle ( cl_dpy_, pixmap_, cl_gc_, 0, 0, 10, 10 );
    editableColor_ = XtVaCreateManagedWidget ( "EditableColor",
                     xmPushButtonWidgetClass, bin_,
                     XmNlabelType, XmPIXMAP,
                     XmNlabelPixmap, pixmap_,
                     NULL );
    XtAddCallback ( editableColor_, XmNactivateCallback, legend_editableColorCB, ( XtPointer ) this );
    if ( XtIsManaged ( bin_ ) ) XtUnmanageChild ( bin_ );
    XtManageChild ( bin_ );

    box_y = box_y + 10;
    separator = XtVaCreateManagedWidget ( "sep",
                                          xmSeparatorWidgetClass, legend_dialog_,
                                          XmNtopAttachment,       XmATTACH_WIDGET,
                                          XmNtopWidget,           legend_labels_,
                                          XmNtopOffset,           box_y,
                                          XmNleftAttachment,      XmATTACH_FORM,
                                          XmNrightAttachment,     XmATTACH_FORM,
                                          NULL );

    close = XtVaCreateManagedWidget ( "Close",
                                      xmPushButtonWidgetClass, legend_dialog_,
                                      XmNtopAttachment,       XmATTACH_WIDGET,
                                      XmNtopWidget,           separator,
                                      XmNtopOffset,           0,
                                      XmNrightAttachment,     XmATTACH_FORM,
                                      XmNrightOffset,         0,
                                      XmNbottomOffset,        0,
                                      XmNwidth,               100,
                                      XmNheight,              35,
                                      NULL );
    XtAddCallback ( close, XmNactivateCallback, &ColorLegend::closeLegendDialogCB, ( XtPointer ) this );

#if 0
    tzWidget = XtVaCreateManagedWidget ( "Change Timezone",
                                         xmToggleButtonWidgetClass, legend_dialog_,
                                         XmNvalue,               legend_invert_,
                                         XmNshowValue,           True,
                                         XmNtopAttachment,       XmATTACH_WIDGET,
                                         XmNtopWidget,           separator,
                                         XmNtopOffset,           0,
                                         XmNleftAttachment,     XmATTACH_FORM,
                                         XmNleftOffset,          20,
                                         XmNbottomOffset,        0,
                                         XmNwidth,               150,
                                         XmNheight,              25,
                                         NULL );

    //   XtAddCallback(tzWidget, XmNactivateCallback, &ColorLegend::timezone_dialogCB, (XtPointer) this);
#endif
    }



void ColorLegend::set_legend_color_widget()
    {
    Widget pb;

    Pixmap pixmap;
    GC pixgc;
    XGCValues gcv;
    static char str[16];

    register int i, ramp_index;

    wrc_ =  XtVaCreateWidget ( "wrc", xmRowColumnWidgetClass,
                               legend_color_bin_, XmNnumColumns,      8,
                               XmNpacking,         XmPACK_COLUMN,
                               XmNleftAttachment,  XmATTACH_FORM,
                               XmNtopAttachment,   XmATTACH_FORM,
                               NULL );

    gcv.foreground = WhitePixelOfScreen ( XtScreen ( legend_color_bin_ ) );
    pixgc = XCreateGC ( cl_dpy_,
                        RootWindowOfScreen ( XtScreen ( legend_color_bin_ ) ), GCForeground, &gcv );

    for ( i = 0; i < 64; i++ )
        {

        // Create a single tile (pixmap) for each color
        pixmap = XCreatePixmap ( cl_dpy_, RootWindowOfScreen ( XtScreen ( wrc_ ) ),
                                 10, 10, DefaultDepthOfScreen ( XtScreen ( wrc_ ) ) );

        ramp_index = ( int ) ( i * ( 255.0/ ( 64 - 1 ) ) );
        XSetForeground ( cl_dpy_, pixgc,
                         get_ramp_pixel ( ramp_index ) );
        XFillRectangle ( cl_dpy_, pixmap, pixgc, 0, 0, 10, 10 );

        sprintf ( str, "c%02d", i );
        pb = XtVaCreateManagedWidget ( str, xmPushButtonWidgetClass, wrc_,
                                       XmNlabelType, XmPIXMAP, XmNlabelPixmap, pixmap, NULL );

        XtVaSetValues ( pb, XmNuserData, i, NULL );

        // callback for this pushbutton sets the current color
        XtAddCallback ( pb, XmNactivateCallback, legend_colorbinCB, ( XtPointer ) this );

        }

    if ( XtIsManaged ( wrc_ ) ) XtUnmanageChild ( wrc_ );
    XtManageChild ( wrc_ );
    }



void ColorLegend::legend_panel_redraw()
    {
    XtUnmanageChild ( wrc_ );
    XtDestroyWidget ( wrc_ );
    set_legend_color_widget();
    }



void ColorLegend::drawColorLegend ( int offset, int width )
    {
    int i, idx;
    static char str[256];
    float ypos, dy;
    int lpos, ly, oldly;
    int nlabels;
    int ulx, uly, rectwidth, rectheight; // added 961014 SRT to draw frame around legend
    if ( legend_nlabel_ < legend_ntile_ + 1 )
        nlabels = legend_nlabel_;
    else
        nlabels = legend_ntile_ + 1;

    dy = ( cl_s_->scaley ( cl_s_->ymin_ ) - cl_s_->scaley ( cl_s_->ymax_ ) ) / legend_ntile_;
    ypos = cl_s_->scaley ( cl_s_->ymax_ );


    ly = ( int ) ( dy + .5 );
    lpos = ( int ) ( ypos + .5 );

    for ( i = legend_ntile_-1, idx =0; i >= 0; i--, idx++ )
        {

        oldly = ly;
        XFillRectangle ( cl_dpy_,
                         cl_drw_,
                         color_gc_table_[i],
                         offset, lpos, width, ly );

        if ( i == legend_ntile_-1 )         // added 961014 SRT
            {
            // added 961014 SRT
            ulx = ( offset>0 ) ? offset-1 : 0;  // added 961014 SRT
            uly = ( lpos>0 ) ? lpos-1 : 0;      // added 961014 SRT
            }                   // added 961014 SRT
        // added 961014 SRT
        if ( i == 0 )               // added 961014 SRT
            {
            // added 961014 SRT
            rectwidth = width+1;            // added 961014 SRT
            rectheight = lpos+ly-uly-1;     // added 961014 SRT
            }                   // added 961014 SRT

        if ( ( i == legend_ntile_ -1 ) || ( ! ( ( i+1 ) % ( legend_ntile_/ ( nlabels-1 ) ) ) ) )
            {
            sprintf ( str, legend_format_, data_table_[i+1] );

            XSetForeground ( cl_dpy_,
                             cl_gc_,
                             BlackPixel ( cl_dpy_, DefaultScreen ( cl_dpy_ ) ) );

            XDrawString ( cl_dpy_,
                          cl_drw_,
                          cl_gc_,
                          offset+width+5, ( int ) ( ypos+.5+5 ), str, strlen ( str ) );

            }
        ypos += dy;
        ly = ( int ) ( ypos - ( ( float ) lpos ) + .5 );
        lpos += oldly;
        }

    sprintf ( str, legend_format_, data_table_[0] );
    XDrawString ( cl_dpy_,
                  cl_drw_,
                  cl_gc_,
                  offset+width+5, lpos+5, str, strlen ( str ) );

    // draw the units if they're available
    if ( unitString_[0] )
        {
        XDrawString ( cl_dpy_,
                      cl_drw_,
                      cl_gc_,
                      offset, lpos+20, unitString_, strlen ( unitString_ ) );
        }

    // Reset foreground color
    XSetForeground ( cl_dpy_,
                     cl_gc_,
                     BlackPixel ( cl_dpy_, DefaultScreen ( cl_dpy_ ) ) );

    // Draw a framing rectangle around the legend    // added 961014 SRT
    XDrawRectangle ( cl_dpy_, cl_drw_, cl_gc_,       // added 961014 SRT
                     ulx, uly, rectwidth, rectheight );   // added 961014 SRT

    }


void ColorLegend::legend_titleCB ( Widget w, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_title_cb ( w );
    }


void ColorLegend::legend_title_cb ( Widget w )
    {
    char *text = XmTextGetString ( w );

#ifdef DIAGNOSTICS
    if ( text != NULL )
        fprintf ( stderr, "Enter ColorLegend::legend_title_cb('%s')\n",
                  text );
    else
        fprintf ( stderr, "Enter ColorLegend::legend_title_cb('NULL')\n" );
#endif // DIAGNOSTICS

    if ( legend_title_ )
        {
        if ( *legend_title_ )
            {
            free ( *legend_title_ );
            *legend_title_ = NULL;
            }
        *legend_title_ = strdup ( text );
        XtFree ( text );
        refreshColor();
        reset_legend_minmax();
        }
    }


void ColorLegend::legend_subTitle1CB ( Widget w, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_subTitle1_cb ( w );
    }


void ColorLegend::legend_subTitle1_cb ( Widget w )
    {
    char *text = XmTextGetString ( w );

    if ( legend_subtitle1_ )
        {
        if ( *legend_subtitle1_ )
            {
            free ( *legend_subtitle1_ );
            *legend_subtitle1_ = NULL;
            }
        *legend_subtitle1_ = strdup ( text );
        XtFree ( text );
        refreshColor();
        reset_legend_minmax();
        }
    }



void ColorLegend::legend_subTitle2CB ( Widget w, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_subTitle2_cb ( w );
    }


void ColorLegend::legend_subTitle2_cb ( Widget w )
    {
    char *text = XmTextGetString ( w );

    if ( legend_subtitle2_ )
        {
        if ( *legend_subtitle2_ )
            {
            free ( *legend_subtitle2_ );
            *legend_subtitle2_ = NULL;
            }
        *legend_subtitle2_ = strdup ( text );
        XtFree ( text );
        refreshColor();
        reset_legend_minmax();
        }
    }



void ColorLegend::legend_unitCB ( Widget w, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->legend_unit_cb ( w );
    }


void ColorLegend::legend_unit_cb ( Widget w )
    {
    char *text = XmTextGetString ( w );
    if ( !text[0] ) strcpy ( unitString_, " " );
    else  strcpy ( unitString_, text );
    XtFree ( text );
    resize(); // forces redraw
    }


void ColorLegend::vector_scale_typeinCB ( Widget w, XtPointer clientData, XtPointer )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    obj->vector_scale_typein_cb ( w );
    }


void ColorLegend::vector_scale_typein_cb ( Widget w )
    {
    char *text = XmTextGetString ( w );
    float f;
    char tstring[32];

    sscanf ( text, "%f", &f );
    if ( f > 0 ) vector_scale_ = f;
    sprintf ( tstring, "%g", vector_scale_ );
    XmTextSetString ( w, tstring );
    XtFree ( text );
    resize();  // force a redraw
    }


void ColorLegend::legend_save_mpegControlsCB ( Widget, XtPointer clientData, XtPointer callData )
    {
    ColorLegend *obj = ( ColorLegend * ) clientData;
    XmScaleCallbackStruct *cbs = ( XmScaleCallbackStruct * ) callData;
    obj->legend_save_mpegControls_cb ( cbs->value );
    }


void ColorLegend::legend_save_mpegControls_cb ( int )
    {
    saveMPEGControls_ = !saveMPEGControls_;
    XtVaSetValues ( legend_save_mpegControls_, XmNvalue, saveMPEGControls_, NULL );
    }

void ColorLegend::timezone_dialogCB ( Widget w, XtPointer clientData, XtPointer )
    {
    }

// WARNING THIS IS APPARENTLY NOT WORKING AT THIS TIME - SRT 960910
int ColorLegend::setConfig ( Config *cfgp )
    {
    int i=1;

    fprintf ( stderr, "Enter ColorLegend::setConfig() why are you here???!!??!?!\n" );
    if ( cfgp->hasFile() )
        {

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( cfgp->hasColorMapType() )
            switch ( cfgp->getColorMapType() )
                {
                case 'N':
                    legend_cmap_newton_cb();
                    break;
                case 'J':
                    legend_cmap_jet_cb();
                    break;
                case 'G':
                    legend_cmap_grey_cb();
                    break;
                }

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( ( cfgp->hasLegend_Max() ) && ( cfgp->hasLegend_Min() ) )
            setContourRange ( cfgp->getLegend_Min(), cfgp->getLegend_Max() );

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( cfgp->hasLegend_Format() )
            legend_format_cb ( cfgp->getLegend_Format() );

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( cfgp->hasNumber_Labels() )
            legend_labels_cb ( cfgp->getNumber_Labels() );

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( cfgp->hasInvert_Colormap() )
            legend_invert_cb ( cfgp->getInvert_Colormap() );

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( cfgp->hasNumber_Tiles() )
            legend_tiles_cb ( cfgp->getNumber_Tiles() );

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( cfgp->hasColors() )
            ; // FIX THIS

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( cfgp->hasSave_MPEG_Files() )
            if ( saveMPEGControls_ != cfgp->getSave_MPEG_Files() )
                legend_save_mpegControls_cb ( cfgp->getSave_MPEG_Files() );

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( cfgp->hasDisable_Map() )
            if ( legend_map_off_ != cfgp->getDisable_Map() )
                legend_disable_map_cb ( cfgp->getDisable_Map() );

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( cfgp->hasSmooth_Plot() )
            if ( smooth_plots_on_ != cfgp->getSmooth_Plot() )
                legend_smooth_plot_cb ( cfgp->getSmooth_Plot() );

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( cfgp->hasDraw_Grid_Lines() )
            if ( grid_lines_on_ != cfgp->getDraw_Grid_Lines() )
                legend_grid_lines_cb ( cfgp->getDraw_Grid_Lines() );

        fprintf ( stderr, "In ColorLegend::setConfig %d\n", i++ );
        if ( cfgp->hasScale_Vectors() )
            if ( scale_vectors_on_ != cfgp->getScale_Vectors() )
                legend_scale_vectors_cb ( cfgp->getScale_Vectors() );
        }

    return 0;
    }
