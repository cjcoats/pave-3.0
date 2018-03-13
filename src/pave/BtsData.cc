/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: BtsData.cc 83 2018-03-12 19:24:33Z coats $
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
//  BtsData.C
//  K. Eng Pua
//  Jan 11, 1995
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950516  Added logic to fix up HourMin and HourMax bars
// SRT  950516  Added setStepMinMaxIncr() routine
//
//////////////////////////////////////////////////////////////////////
#include "BtsData.h"

BtsData::BtsData()
    {
    formulaStr_ = caseList_ = hostList_ = ( char * ) NULL;
    percents_ = ( char * ) NULL;
    initParserData();
    }


int BtsData::setStepMinMaxIncr ( int scase, /* 0 based */
                                 int smin,
                                 int smax,
                                 int sincr,
                                 char *eString )
    {
    if ( step_min_ == NULL )
        {
        if ( eString )
            sprintf ( eString,
                      "setStepMinMaxIncr(%d,%d,%d,%d):Can't overwrite NULL step_min_[] array!\n",
                      scase, smin, smax, sincr );
        return 1;
        }

    if ( step_max_ == NULL )
        {
        if ( eString )
            sprintf ( eString,
                      "setStepMinMaxIncr(%d,%d,%d,%d):Can't overwrite NULL step_max_[] array!\n",
                      scase, smin, smax, sincr );
        return 1;
        }

    if ( step_incr_ == NULL )
        {
        if ( eString )
            sprintf ( eString,
                      "setStepMinMaxIncr(%d,%d,%d,%d):Can't overwrite NULL step_incr_[] array!\n",
                      scase, smin, smax, sincr );
        return 1;
        }

    if ( scase < 0 )
        {
        if ( eString )
            sprintf ( eString,
                      "setStepMinMaxIncr(%d,%d,%d,%d):Can't overwrite to a negative array index!\n",
                      scase, smin, smax, sincr );
        return 1;
        }

    if ( sincr <= 0 )
        {
        if ( eString )
            sprintf ( eString,
                      "setStepMinMaxIncr(%d,%d,%d,%d):Can't set incr to a non positive value!\n",
                      scase, smin, smax, sincr );
        return 1;
        }

    if ( smin < 0 )
        {
        if ( eString )
            sprintf ( eString,
                      "setStepMinMaxIncr(%d,%d,%d,%d):Can't set step_min to a negative step!\n",
                      scase, smin, smax, sincr );
        return 1;
        }

    if ( smax < smin )
        {
        if ( eString )
            sprintf ( eString,
                      "setStepMinMaxIncr(%d,%d,%d,%d):Can't set step_max to < step_min !\n",
                      scase, smin, smax, sincr );
        return 1;
        }

    step_min_[scase] = smin;
    step_max_[scase] = smax;
    step_incr_[scase] = sincr;

    return 0;
    }


void BtsData::initParserData()
    {
    if ( formulaStr_ != NULL )
        {
        delete formulaStr_;
        formulaStr_ = ( char * ) NULL;
        }
    if ( caseList_ != NULL )
        {
        delete caseList_;
        caseList_ = ( char * ) NULL;
        }

    if ( hostList_ != NULL )
        {
        delete hostList_;
        hostList_ = ( char * ) NULL;
        }
    errString_[0] = postFixQueue_[0] = caseUsed_[0] = '\0';
    dim_ = dateDay_ = dateMonth_ = dateYear_ = hourStart_ = mixCase_ = 0;

    }


//
// This function should be called only after initParserData() and
// parseData() have been called.
//
int BtsData::initRetrieveData()
    {
    int i, j;

    if ( thickValues_ != NULL )
        delete thickValues_;
    /*
       thickValues_ = new float[kmax_];
    */

    if ( ( thickValues_ = ( float * ) malloc ( kmax_*sizeof ( float ) ) ) == NULL )
        {
        fprintf ( stderr, "Can't allocate memory for thickValues_\n" );
        return ( 0 );
        }
    for ( i=0; i < kmax_; i++ )
        thickValues_[i] = 1.0;

    if ( whichLevel_ != NULL )
        free ( whichLevel_ );

    /*
       whichLevel_ = new int[kmax_];
    */

    if ( ( whichLevel_ = ( int * ) malloc ( kmax_*sizeof ( int ) ) ) == NULL )
        {
        fprintf ( stderr, "Can't allocate memory for whichLevel_\n" );
        return ( 0 );
        }

    for ( i=0; i < kmax_; i++ )
        whichLevel_[i] = 1;

    if ( tsdata_ != NULL )
        delete tsdata_;
    /*
       tsdata_ = new float[kmax_*(hrMax_-hrMin_+1)];
    */
    if ( ( tsdata_ = ( float * ) malloc ( kmax_* ( hrMax_-hrMin_+1 ) *sizeof ( float ) ) ) == NULL )
        {
        fprintf ( stderr, "Can't allocate memory for tsdata_\n" );
        return ( 0 );
        }

    if ( percents_ != NULL )
        delete percents_;

    if ( ( percents_ = ( char * ) malloc ( imax_*jmax_*sizeof ( char ) ) ) == NULL )
        {
        fprintf ( stderr, "Can't allocate memory for percents_\n" );
        return ( 0 );
        }

    for ( j=0; j < jmax_; j++ )
        for ( i=0; i< imax_; i++ )
            // SRT percents_[i*jmax_+j] = 100;
            percents_[INDEX ( i,j,0,0,imax_,jmax_,1 )] =
#ifdef DOMFIX
                ( ( i>0 && j>0 ) ? 100 : 0 ); // SRT // SRT DOMFIX
#else  // SRT DOMFIX
                100; // SRT DOMFIX
#endif // DOMFIX

    if ( step_min_ != NULL )
        delete step_min_;

    if ( ( step_min_ = ( int * ) malloc ( caseCount_*sizeof ( int ) ) ) == NULL )
        {
        fprintf ( stderr, "Can't allocate memory for step_min_\n" );
        return ( 0 );
        }

    if ( step_max_ != NULL )
        delete step_max_;

    if ( ( step_max_ = ( int * ) malloc ( caseCount_*sizeof ( int ) ) ) == NULL )
        {
        fprintf ( stderr, "Can't allocate memory for step_max_\n" );
        return ( 0 );
        }

    if ( step_incr_ != NULL )
        delete step_incr_;
    /*
       step_incr_ = new int[caseCount_];
    */
    if ( ( step_incr_ = ( int * ) malloc ( caseCount_*sizeof ( int ) ) ) == NULL )
        {
        fprintf ( stderr, "Can't allocate memory for step_incr_\n" );
        return ( 0 );
        }


    // HARD-WIRED FOR NOW. NEED TO CHANGE LATER!!!!

    for ( i=0; i < caseCount_; i++ )
        {
        step_min_[i] = hrMin_+1;
        // SRT step_max_[i] = 24;  //hrMax_+1;
        step_max_[i] = hrMax_+1;  //hrMax_+1;
        step_incr_[i] = 1;
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "In initRetrieveData set step range to (%d..%d by %d)\n",
              step_min_[0],step_max_[0],step_incr_[0] );
#endif

    selected_step_ = 1;
    use_floor_ = 0;
    floorCut_ = 0.0001;

    return ( 1 );
    }


//
// This function should be called only after initParseData() has been called
//
int BtsData::setPercentArray ( int *domain_mask, int col_max, int row_max )
    {
    if ( ( domain_mask == NULL ) || ( col_max != imax_ ) || ( row_max != jmax_ ) )
        return 1;

    int i, j;

    for ( i=0; i< imax_; i++ )
        for ( j=0; j < jmax_; j++ )
            percents_[i*jmax_+j] = ( char ) domain_mask[i*jmax_+j];
    return 0;

    }



