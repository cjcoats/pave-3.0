/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: ComboData.cc 83 2018-03-12 19:24:33Z coats $
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
//  ComboData.C
//  K. Eng Pua
//  Copyright (C)
//  Jan 28, 1995
//
//  NOTE: :
//  NOTE: not finish yet.  need more work
//////////////////////////////////////////////////////////////////////

#include "ComboData.h"


void ComboData::initialize()
    {
    title1_ = NULL;
    title2_ = NULL;
    title3_ = NULL;
    xtitle_ = NULL;
    ytitle_ = NULL;
    val_x_  = NULL;
    val_xs_ = NULL;
    val_y_  = NULL;
    y_label_list_ = NULL;

    // Defaults
    sprintf ( plotformatX_, "%cg", '%' );
    if ( ( getenv ( "BARPLOTYFORMAT" ) != NULL ) && ( strlen ( getenv ( "BARPLOTYFORMAT" ) ) ) )
        sprintf ( plotformatY_, "%s", getenv ( "BARPLOTYFORMAT" ) );
    else
        sprintf ( plotformatY_, "%cg", '%' );
    offset_left_ = offset_top_ = 100;
    offset_right_ = offset_bottom_ = 60;
    }



void ComboData::setTitles ( char *t1, char *t2, char *t3, char *xt, char *yt )
    {
    if ( title1_ )
        {
        delete title1_;
        title1_ = NULL;
        }
    if ( title2_ )
        {
        delete title2_;
        title2_ = NULL;
        }
    if ( title3_ )
        {
        delete title3_;
        title3_ = NULL;
        }
    if ( xtitle_ )
        {
        delete xtitle_;
        xtitle_ = NULL;
        }
    if ( ytitle_ )
        {
        delete ytitle_;
        ytitle_ = NULL;
        }

    if ( t1 ) title1_ = strdup ( t1 );
    if ( t2 ) title2_ = strdup ( t2 );
    if ( t3 ) title3_ = strdup ( t3 );
    if ( xt ) xtitle_ = strdup ( xt );
    if ( yt ) ytitle_ = strdup ( yt );
    }


void ComboData::setDataArray ( float *x, float *y, int y_num, int obs_num )
    {
    int idy = 0;
    int i, j;

    if ( val_x_ ) delete val_x_;
    if ( val_y_ ) delete val_y_;

    y_data_num_ = y_num;
    obs_num_ = obs_num;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Y_DATA_NUM=%d OBS_NUM=%d\n", y_num, obs_num );
    fprintf ( stderr, "Y_DATA_NUM=%d OBS_NUM=%d\n", y_data_num_, obs_num_ );
#endif // DIAGNOSTICS

    val_x_  = new float[obs_num_];
    val_y_  = new float[obs_num_*y_data_num_];
    val_xs_ = new str20[obs_num_];

    val_ymax_ = -999.9;
    val_ymin_ = 999.9;

    for ( i = 0; i < obs_num_; i++ )
        {
        val_x_[i] = x[i];
        sprintf ( val_xs_[i], "%g ", val_x_[i] );

        for ( j = 0; j < y_data_num_; j++ )
            {
            idy = i*y_data_num_ + j;
            val_y_[idy] = y[idy];
            if ( val_y_[idy] > val_ymax_ )
                val_ymax_ = val_y_[idy];
            if ( val_y_[idy] < val_ymin_ )
                val_ymin_ = val_y_[idy];
            }
        }
    val_xmin_ = 0;
    val_xmax_ = ( float ) ( ( y_data_num_+1 ) * obs_num_ ) + 1.0;
    if ( val_ymin_ > 0.0 ) val_ymin_ -= val_ymin_*0.1;
    }



