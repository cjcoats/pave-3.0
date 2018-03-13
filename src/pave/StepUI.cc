/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: StepUI.cc 83 2018-03-12 19:24:33Z coats $
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
// File:        StepUI.cc
// Author:      Steve Thorpe
// Date:        June 5, 1995
//////////////////////////////////////////////////////////////////////
//
//    StepUI Class
//
//    StepUI                               Concrete
//        1. Creates/Posts an step edit dialog
//        2. Sets the range for a given step min/max
//        3. Obtains the current value of a selected step widgets
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950605  Implemented
// SRT  960412  Added Julian start date/time
// SRT  960416  Added getFirst_sdate_InRange(), getFirst_stime_InRange(),
//                    and getNStepsInRange()
// SRT  960416  Added getFirst_sdate_InRange(), getFirst_stime_InRange(),
//                    getNStepsInRange(), getCurrentJulianTimeCutoffs(),
//                    tryToSetJulianTimeCutoffs()
// SRT  960419  Added apply button
// SRT  960424  Added updateValues()
// SRT  960502  Added getFirst_Offset_InRange()
//
//////////////////////////////////////////////////////////////////////

#include "StepUI.h"

StepUI::StepUI  (
    char *title,            // title of dialog box
    Widget parent,          // to base position on
    char *minLabel,         // "Step Min", "Layer Min", etc
    char *maxLabel,         // "Step Max", "Layer Max", etc
    int rangeMin,           // 0, 1, etc
    int rangeMax,           // 24, 72, etc
    int *sdate,    /* Julian start date for time steps 960412 SRT */
    int *stime,    /* Julian start time for time steps 960412 SRT */
    int *currentMinP,       // current Min
    int *currentMaxP,       // current Max
    void ( *valsModifiedParentCB ) ( void * ), // call this when
    // vals change
    void *obj,      // to hold pointer to an object
    char *estring           // to hold error msgs
)
    {
    assert ( /*parent &&*/ title && minLabel && maxLabel && estring );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter StepUI::StepUI(%d to %d)\n", rangeMin, rangeMax );
#endif // DIAGNOSTICS

    strcpy ( title_, title );
    parent_ = parent;
    strcpy ( minLabel_, minLabel );
    strcpy ( maxLabel_, maxLabel );
    rangeMin_ = rangeMin;
    rangeMax_ = rangeMax;
    currentMinP_ = currentMinP;
    currentMaxP_ = currentMaxP;
    currentMin_ = *currentMinP;
    currentMax_ = *currentMaxP;
    parent_OK_CB_ = valsModifiedParentCB;
    obj_ = obj;

    if ( rangeMax_ < rangeMin_ )
        {
        sprintf ( estring, "rangeMax %d < rangeMin %d - resetting to %d",
                  rangeMax_, rangeMin_, rangeMin_ );
#ifdef DIAGNOSTICS
        fprintf ( stderr, "%s\n", estring );
#endif // DIAGNOSTICS
        rangeMax_ = rangeMin_;
        }

    if ( currentMin_ < rangeMin_ )
        {
        sprintf ( estring, "currentMin %d < rangeMin %d - resetting to %d",
                  currentMin_, rangeMin_, rangeMin_ );
#ifdef DIAGNOSTICS
        fprintf ( stderr, "%s\n", estring );
#endif // DIAGNOSTICS
        currentMin_ = rangeMin_;
        }

    if ( currentMax_ < currentMin_ )
        {
        sprintf ( estring, "currentMax %d < currentMin %d - resetting to %d",
                  currentMax_, currentMin_, currentMin_ );
#ifdef DIAGNOSTICS
        fprintf ( stderr, "%s\n", estring );
#endif // DIAGNOSTICS
        currentMax_ = currentMin_;
        }

    if ( currentMax_ > rangeMax_ )
        {
        sprintf ( estring, "currentMax %d > rangeMax %d - resetting to %d",
                  currentMax_, rangeMax_, rangeMax_ );
#ifdef DIAGNOSTICS
        fprintf ( stderr, "%s\n", estring );
#endif // DIAGNOSTICS
        currentMax_ = rangeMax_;
        }

    if ( ( !sdate ) || ( !stime ) )
        sdate_ = stime_ = NULL;
    else
        {
        int nbytes = sizeof ( int ) * ( rangeMax_-rangeMin_+1 );

        sdate_ = ( int * ) malloc ( nbytes );
        stime_ = ( int * ) malloc ( nbytes );
        if ( ( !sdate_ ) || ( !stime_ ) )
            {
            if ( sdate_ ) free ( sdate_ );
            sdate_ = NULL;
            if ( stime_ ) free ( stime_ );
            stime_ = NULL;
            }
        else
            {
            memcpy ( sdate_, sdate, nbytes );
            memcpy ( stime_, stime, nbytes );
            }
        }

    createOptionDialog();
    }


int StepUI::updateValues (
    char *minLabel,         // "Step Min", "Layer Min", etc
    char *maxLabel,         // "Step Max", "Layer Max", etc
    int rangeMin,           // 0, 1, etc
    int rangeMax,           // 24, 72, etc
    int *sdate,    /* Julian start date for time steps 960412 SRT */
    int *stime,    /* Julian start time for time steps 960412 SRT */
    int *currentMinP,       // current Min
    int *currentMaxP,       // current Max
    char *estring           // to hold error msgs
)
    {
    char extendedTitle[256];

    assert ( /*parent &&*/ minLabel && maxLabel && estring );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter StepUI::updateValues(%d to %d)\n", rangeMin, rangeMax );
#endif // DIAGNOSTICS

    // free up any already existing stuff
    if ( sdate_ ) free ( sdate_ );
    sdate_ = NULL;
    if ( stime_ ) free ( stime_ );
    stime_ = NULL;

    strcpy ( minLabel_, minLabel );
    strcpy ( maxLabel_, maxLabel );
    rangeMin_ = rangeMin;
    rangeMax_ = rangeMax;
    currentMinP_ = currentMinP;
    currentMaxP_ = currentMaxP;
    currentMin_ = *currentMinP;
    currentMax_ = *currentMaxP;

    if ( rangeMax_ < rangeMin_ )
        {
        sprintf ( estring, "rangeMax %d < rangeMin %d - resetting to %d",
                  rangeMax_, rangeMin_, rangeMin_ );
#ifdef DIAGNOSTICS
        fprintf ( stderr, "%s\n", estring );
#endif // DIAGNOSTICS
        rangeMax_ = rangeMin_;
        }

    if ( currentMin_ < rangeMin_ )
        {
        sprintf ( estring, "currentMin %d < rangeMin %d - resetting to %d",
                  currentMin_, rangeMin_, rangeMin_ );
#ifdef DIAGNOSTICS
        fprintf ( stderr, "%s\n", estring );
#endif // DIAGNOSTICS
        currentMin_ = rangeMin_;
        }

    if ( currentMax_ < currentMin_ )
        {
        sprintf ( estring, "currentMax %d < currentMin %d - resetting to %d",
                  currentMax_, currentMin_, currentMin_ );
#ifdef DIAGNOSTICS
        fprintf ( stderr, "%s\n", estring );
#endif // DIAGNOSTICS
        currentMax_ = currentMin_;
        }

    if ( currentMax_ > rangeMax_ )
        {
        sprintf ( estring, "currentMax %d > rangeMax %d - resetting to %d",
                  currentMax_, rangeMax_, rangeMax_ );
#ifdef DIAGNOSTICS
        fprintf ( stderr, "%s\n", estring );
#endif // DIAGNOSTICS
        currentMax_ = rangeMax_;
        }

    if ( ( !sdate ) || ( !stime ) )
        sdate_ = stime_ = NULL;
    else
        {
        int nbytes = sizeof ( int ) * ( rangeMax_-rangeMin_+1 );

        sdate_ = ( int * ) malloc ( nbytes );
        stime_ = ( int * ) malloc ( nbytes );
        if ( ( !sdate_ ) || ( !stime_ ) )
            {
            if ( sdate_ ) free ( sdate_ );
            sdate_ = NULL;
            if ( stime_ ) free ( stime_ );
            stime_ = NULL;
            }
        else
            {
            memcpy ( sdate_, sdate, nbytes );
            memcpy ( stime_, stime, nbytes );
            }
        }


    sprintf ( extendedTitle, "%s (%d to %d)", minLabel_, rangeMin_, rangeMax_ );

    XtVaSetValues ( currentMin_scale_,
                    XtVaTypedArg, XmNtitleString, XmRString, extendedTitle, 4,
                    XmNmaximum,             rangeMax_,
                    XmNminimum,             rangeMin_,
                    XmNvalue,               currentMin_,
                    NULL );

    sprintf ( extendedTitle, "%s (%d to %d)", maxLabel_, rangeMin_, rangeMax_ );

    XtVaSetValues ( currentMax_scale_,
                    XtVaTypedArg, XmNtitleString, XmRString, extendedTitle, 4,
                    XmNmaximum,             rangeMax_,
                    XmNminimum,             rangeMin_,
                    XmNvalue,               currentMax_,
                    NULL );

    setMinMaxLabel ( 0 ); // set the min label SRT 960412
    setMinMaxLabel ( 1 ); // set the max label SRT 960412

    return 0;
    }


StepUI::~StepUI ( void )
    {
    XtUnmanageChild ( step_dialog_ );
    if ( sdate_ ) free ( sdate_ );
    sdate_ = NULL;
    if ( stime_ ) free ( stime_ );
    stime_ = NULL;
    }


void StepUI::setMinMaxLabel ( int min_or_max ) // use 0 for min, non-0 for max
    {
    char label[128], tstring[128];
    int  currentValSlider, offset;
    Widget scaleWidget;

    scaleWidget = min_or_max ? currentMax_scale_ : currentMin_scale_;
    XtVaGetValues ( scaleWidget, XmNvalue, &currentValSlider, NULL );
    offset = currentValSlider - rangeMin_;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "currentValSlider == %d, rangeMin_ == %d, ", currentValSlider,
              rangeMin_ );
    fprintf ( stderr, "offset == %d\nsdate_ == %ld, stime_ == %ld\n",
              offset, ( long ) sdate_, ( long ) stime_ );
    if ( sdate_ && sdate_[offset] ) fprintf ( stderr, "sdate_[offset] == %ld"
                ", stime_[offset] = %ld",
                ( long ) sdate_[offset], ( long ) stime_[offset] );
    fprintf ( stderr, "\n" );
#endif // DIAGNOSTICS

    if ( ( !sdate_ ) || ( !stime_ ) || ( !sdate_[offset] ) )
        sprintf ( label, "%s (%d through %d)",
                  min_or_max ? maxLabel_:minLabel_, rangeMin_, rangeMax_ );
    else
        {
        julian2text ( tstring, sdate_[offset], stime_[offset] );
        sprintf ( label, "%s (%s)", min_or_max ? maxLabel_:minLabel_, tstring );
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "About to set %s label to '%s'\n",
              min_or_max ? "max" : "min", label );
#endif // DIAGNOSTICS

    XtVaSetValues ( scaleWidget,
                    XtVaTypedArg, XmNtitleString, XmRString, label, 4,
                    NULL );
    }


void StepUI::createOptionDialog()
    {
    char extendedTitle[256];

    // Create control dialog box.
    step_dialog_ = XmCreateFormDialog ( parent_, title_, NULL, 0 );

    XtVaSetValues ( step_dialog_,
                    XmNautoUnmanage,        False,
                    NULL );

    Widget form1 = XtVaCreateWidget ( "form1",
                                      xmFormWidgetClass,      step_dialog_,
                                      XmNtopAttachment,       XmATTACH_FORM,
                                      XmNtopOffset,           10,
                                      XmNleftAttachment,      XmATTACH_FORM,
                                      XmNrightAttachment,     XmATTACH_FORM,
                                      NULL );

    sprintf ( extendedTitle, "%s (%d to %d)", minLabel_, rangeMin_, rangeMax_ );
    currentMin_scale_ = XtVaCreateManagedWidget ( minLabel_,
                        xmScaleWidgetClass,     form1,
                        XtVaTypedArg, XmNtitleString, XmRString, extendedTitle, 4,
                        XmNheight,              100,
                        XmNmaximum,             rangeMax_,
                        XmNminimum,             rangeMin_,
                        XmNvalue,               currentMin_,
                        XmNshowValue,           True,
                        XmNorientation,         XmHORIZONTAL,
                        XmNtopAttachment,       XmATTACH_FORM,
                        XmNleftAttachment,      XmATTACH_FORM,
                        XmNleftOffset,          10,
                        XmNrightAttachment,     XmATTACH_FORM,
                        XmNrightOffset,         10,
                        NULL );
    XtAddCallback ( currentMin_scale_, XmNvalueChangedCallback,
                    &StepUI::minOrMaxSliderMovedCB, ( XtPointer ) this );

    sprintf ( extendedTitle, "%s (%d to %d)", maxLabel_, rangeMin_, rangeMax_ );
    currentMax_scale_ = XtVaCreateManagedWidget ( maxLabel_,
                        xmScaleWidgetClass,     form1,
                        XtVaTypedArg, XmNtitleString, XmRString, extendedTitle, 5,
                        XmNheight,              100,
                        XmNmaximum,             rangeMax_,
                        XmNminimum,             rangeMin_,
                        XmNvalue,               currentMax_,
                        XmNshowValue,           True,
                        XmNorientation,         XmHORIZONTAL,
                        XmNtopAttachment,       XmATTACH_WIDGET,
                        XmNtopWidget,       currentMin_scale_,
                        XmNleftAttachment,      XmATTACH_FORM,
                        XmNleftOffset,          10,
                        XmNrightAttachment,     XmATTACH_FORM,
                        XmNrightOffset,         10,
                        NULL );
    XtAddCallback ( currentMax_scale_, XmNvalueChangedCallback,
                    &StepUI::minOrMaxSliderMovedCB, ( XtPointer ) this );


    Widget sep = XtVaCreateManagedWidget ( "sep",
                                           xmSeparatorWidgetClass,     form1,
                                           XmNleftAttachment,      XmATTACH_FORM,
                                           XmNrightAttachment,     XmATTACH_FORM,
                                           XmNtopAttachment,       XmATTACH_WIDGET,
                                           XmNtopWidget,           currentMax_scale_,
                                           XmNtopOffset,           10,
                                           NULL );

    ok_ = XtVaCreateManagedWidget ( "OK",
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
    XtAddCallback ( ok_, XmNactivateCallback, &StepUI::okCB, ( XtPointer ) this );

    apply_ = XtVaCreateManagedWidget ( "Apply",
                                       xmPushButtonWidgetClass, form1,
                                       XmNtopAttachment,       XmATTACH_WIDGET,
                                       XmNtopWidget,           sep,
                                       XmNtopOffset,           10,
                                       XmNleftAttachment,      XmATTACH_WIDGET,
                                       XmNleftWidget,          ok_,
                                       XmNleftOffset,          10,
                                       XmNwidth,               100,
                                       XmNheight,              40,
                                       NULL );
    XtAddCallback ( apply_, XmNactivateCallback, &StepUI::applyCB,
                    ( XtPointer ) this );

    cancel_ = XtVaCreateManagedWidget ( "Cancel",
                                        xmPushButtonWidgetClass, form1,
                                        XmNtopAttachment,       XmATTACH_WIDGET,
                                        XmNtopWidget,           sep,
                                        XmNtopOffset,           10,
                                        XmNleftAttachment,      XmATTACH_WIDGET,
                                        XmNleftWidget,          apply_,
                                        XmNleftOffset,          10,
                                        XmNrightAttachment,     XmATTACH_FORM,
                                        XmNrightOffset,         10,
                                        XmNwidth,               100,
                                        XmNheight,              40,
                                        NULL );
    XtAddCallback ( cancel_, XmNactivateCallback, &StepUI::cancelCB,
                    ( XtPointer ) this );

    setMinMaxLabel ( 0 ); // set the min label SRT 960412
    setMinMaxLabel ( 1 ); // set the max label SRT 960412

    if ( XtIsManaged ( form1 ) ) XtUnmanageChild ( form1 );
    XtManageChild ( form1 );
    }



void StepUI::minOrMaxSliderMovedCB ( Widget, XtPointer clientData, XtPointer )
    {
    StepUI *obj = ( StepUI * ) clientData;
    obj->minOrMaxSliderMoved_cb();
    }


void StepUI::minOrMaxSliderMoved_cb()
    {
    int minSliderVal, maxSliderVal;

    XtVaGetValues ( currentMin_scale_, XmNvalue, &minSliderVal, NULL );
    XtVaGetValues ( currentMax_scale_, XmNvalue, &maxSliderVal, NULL );
    if ( minSliderVal > maxSliderVal )
        {
        int t = maxSliderVal;
        maxSliderVal = minSliderVal;
        minSliderVal = t;
        XtVaSetValues ( currentMin_scale_, XmNvalue, minSliderVal, NULL );
        XtVaSetValues ( currentMax_scale_, XmNvalue, maxSliderVal, NULL );
        }
    setMinMaxLabel ( 0 );
    setMinMaxLabel ( 1 );
    }


void StepUI::okCB ( Widget, XtPointer clientData, XtPointer )
    {
    StepUI *obj = ( StepUI * ) clientData;
    obj->ok_cb();
    }

void StepUI::ok_cb()
    {
    XtUnmanageChild ( step_dialog_ );
    apply_cb();
    }


void StepUI::cancelCB ( Widget, XtPointer clientData, XtPointer )
    {
    StepUI *obj = ( StepUI * ) clientData;
    obj->cancel_cb();
    }

void StepUI::cancel_cb()
    {
    XtUnmanageChild ( step_dialog_ );
    currentMin_ = *currentMinP_;
    currentMax_ = *currentMaxP_;
    XtVaSetValues ( currentMin_scale_, XmNvalue, currentMin_, NULL );
    XtVaSetValues ( currentMax_scale_, XmNvalue, currentMax_, NULL );
    }


void StepUI::applyCB ( Widget, XtPointer clientData, XtPointer )
    {
    StepUI *obj = ( StepUI * ) clientData;
    obj->apply_cb();
    }

void StepUI::apply_cb()
    {
    XtVaGetValues ( currentMin_scale_, XmNvalue, &currentMin_, NULL );
    XtVaGetValues ( currentMax_scale_, XmNvalue, &currentMax_, NULL );
    if ( currentMin_ > currentMax_ )
        {
        int t = currentMax_;
        currentMax_ = currentMin_;
        currentMin_ = t;
        XtVaSetValues ( currentMin_scale_, XmNvalue, currentMin_, NULL );
        XtVaSetValues ( currentMax_scale_, XmNvalue, currentMax_, NULL );
        }
    *currentMinP_ = currentMin_;
    *currentMaxP_ = currentMax_;
    ( *parent_OK_CB_ ) ( obj_ );
    }


void StepUI::setMinMax ( int currentMin, // 0, 1, etc
                         int currentMax ) // 24, 72, etc
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter StepUI::setMinMax(currentMin=%d, currentMax=%d) rangeMin_==%d rangeMax_==%d\n",
              currentMin, currentMax, rangeMin_, rangeMax_ );
#endif // DIAGNOSTICS

    if ( currentMax < currentMin )
        {
#ifdef DIAGNOSTICS
        fprintf ( stderr, "currentMax %d < currentMin %d - resetting to %d",
                  currentMax, currentMin, currentMin );
#endif // DIAGNOSTICS
        currentMax = currentMin;
        }

    if ( currentMin < rangeMin_ ) currentMin = rangeMin_;
    if ( currentMax > rangeMax_ ) currentMax = rangeMax_;
    currentMin_ = currentMin;
    currentMax_ = currentMax;
    updateStepWidgets();

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Exiting StepUI::setMinMax() with currentMin_=%d and currentMax_=%d\n", currentMin_, currentMax_ );
#endif // DIAGNOSTICS
    }


void StepUI::updateStepWidgets ( void )
    {
    XtVaSetValues ( currentMin_scale_, XmNminimum, rangeMin_, NULL );
    XtVaSetValues ( currentMin_scale_, XmNvalue,   currentMin_, NULL );
    XtVaSetValues ( currentMin_scale_, XmNmaximum, rangeMax_, NULL );
    XtVaSetValues ( currentMax_scale_, XmNminimum, rangeMin_, NULL );
    XtVaSetValues ( currentMax_scale_, XmNvalue,   currentMax_, NULL );
    XtVaSetValues ( currentMax_scale_, XmNmaximum, rangeMax_, NULL );
    minOrMaxSliderMoved_cb(); // added 960412 SRT
    apply_cb();
    }



void StepUI::postOptionDialog()
    {
    if ( XtIsManaged ( step_dialog_ ) ) XtUnmanageChild ( step_dialog_ );
    XtManageChild ( step_dialog_ );
    }



int StepUI::getFirst_sdate_InRange ( void ) // returns Julian start date
// for 1st time step in range
// returns -1 if unavailable
    {
    return ( sdate_ ? sdate_[*currentMinP_-rangeMin_] : -1 );
    }


int StepUI::getFirst_stime_InRange ( void ) // returns Julian start time
// for 1st time step in range
// returns -1 if unavailable
    {
    return ( stime_ ? stime_[*currentMinP_-rangeMin_] : -1 );
    }


int StepUI::getNStepsInRange ( void )      // returns Num steps in range
    {
    return ( *currentMaxP_ - *currentMinP_ + 1 );
    }



void StepUI::getCurrentJulianTimeCutoffs
( int *dateMin, int *timeMin, int *dateMax, int *timeMax )
    {
    assert ( dateMin && timeMin && dateMax && timeMax );
    *dateMin = sdate_ ? sdate_[*currentMinP_-rangeMin_] : -1 ;
    *dateMax = sdate_ ? sdate_[*currentMaxP_-rangeMin_] : -1 ;
    *timeMin = stime_ ? stime_[*currentMinP_-rangeMin_] : -1 ;
    *timeMax = stime_ ? stime_[*currentMaxP_-rangeMin_] : -1 ;
    }



void StepUI::tryToSetJulianTimeCutoffs
( int dateMin, int timeMin, int dateMax, int timeMax )
    {
    int i, minOffset = -1, maxOffset = -1, t;

    if ( ( dateMin == -1 ) ||
            ( timeMin == -1 ) ||
            ( dateMax == -1 ) ||
            ( timeMax == -1 ) ||
            ( !sdate_ ) ||
            ( !stime_ ) )
        return;

    for ( i = 0; i < rangeMax_-rangeMin_+1; i++ )
        {
        if ( ( sdate_[i] == dateMin ) && ( stime_[i] == timeMin ) )
            minOffset = i;
        if ( ( sdate_[i] == dateMax ) && ( stime_[i] == timeMax ) )
            maxOffset = i;
        }

    if ( ( minOffset >= 0 ) || ( maxOffset >= 0 ) )
        {
        if ( minOffset < 0 ) minOffset = 0;
        if ( maxOffset < 0 ) maxOffset = rangeMax_-rangeMin_;
        if ( maxOffset < minOffset )
            {
            t = maxOffset;
            maxOffset = minOffset;
            minOffset = t;
            }
        setMinMax ( minOffset-rangeMin_, maxOffset-rangeMin_ );
        }
    }




int StepUI::getFirst_Offset_InRange ( void ) // returns offset to
// 1st time step in range
// (*0* based)
    {
    return *currentMinP_-rangeMin_;
    }
