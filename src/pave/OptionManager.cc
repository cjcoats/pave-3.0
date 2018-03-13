/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: OptionManager.cc 83 2018-03-12 19:24:33Z coats $
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
// File:        OptionManager.C
// Author:      K. Eng Pua
// Date:        April 25, 1995
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950516  Added HourMin and HourMax bars
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "OptionManager.h"

OptionManager::OptionManager ( Widget parent )
    {
    assert ( parent );
    parent_ = parent;
    createOptionDialog();
    selected_level_ = 1;
    selected_row_ = 1;
    selected_col_ = 1;
    hourMin_ = 1;    // SRT  UNHARDWIRE THIS SOON
    hourMax_ = 24;   // SRT  UNHARDWIRE THIS SOON (used to be 12)
    }



void OptionManager::postOptionDialog()
    {
    if ( XtIsManaged ( option_dialog_ ) ) XtUnmanageChild ( option_dialog_ );
    XtManageChild ( option_dialog_ );
    }


void OptionManager::createOptionDialog()
    {
    // Create control dialog box.
    option_dialog_ = XmCreateFormDialog ( parent_, "Option", NULL, 0 );
    XtVaSetValues ( option_dialog_,
                    XmNautoUnmanage,        False,
                    NULL );

    Widget form1 = XtVaCreateWidget ( "form1",
                                      xmFormWidgetClass,      option_dialog_,
                                      XmNtopAttachment,       XmATTACH_FORM,
                                      XmNtopOffset,           10,
                                      XmNleftAttachment,      XmATTACH_FORM,
                                      XmNrightAttachment,     XmATTACH_FORM,
                                      NULL );

    selected_level_scale_ = XtVaCreateManagedWidget ( "Layer",
                            xmScaleWidgetClass,     form1,
                            XtVaTypedArg, XmNtitleString, XmRString, "Layer", 5,
                            XmNheight,              100,
                            XmNmaximum,             15,
                            XmNminimum,             1,
                            XmNvalue,               1,
                            XmNshowValue,           True,
                            XmNorientation,         XmHORIZONTAL,
                            XmNtopAttachment,       XmATTACH_FORM,
                            XmNleftAttachment,      XmATTACH_FORM,
                            XmNleftOffset,          10,
                            XmNrightAttachment,     XmATTACH_FORM,
                            XmNrightOffset,         10,
                            NULL );

    // SRT   XtAddCallback(selected_level_scale_, XmNactivateCallback, &OptionManager::selected_level_scaleCB, (XtPointer) this);

    selected_row_scale_ = XtVaCreateManagedWidget ( "Row",
                          xmScaleWidgetClass,     form1,
                          XtVaTypedArg, XmNtitleString, XmRString, "Row", 4,
                          XmNheight,              100,
                          XmNmaximum,             200,
                          XmNminimum,             1,
                          XmNvalue,               1,
                          XmNshowValue,           True,
                          XmNorientation,         XmHORIZONTAL,
                          XmNtopAttachment,       XmATTACH_WIDGET,
                          XmNtopWidget,       selected_level_scale_,
                          XmNleftAttachment,      XmATTACH_FORM,
                          XmNleftOffset,          10,
                          XmNrightAttachment,     XmATTACH_FORM,
                          XmNrightOffset,         10,
                          NULL );


    selected_col_scale_ = XtVaCreateManagedWidget ( "Column",
                          xmScaleWidgetClass,     form1,
                          XtVaTypedArg, XmNtitleString, XmRString, "Column", 7,
                          XmNheight,              100,
                          XmNmaximum,             200,
                          XmNminimum,             1,
                          XmNvalue,               1,
                          XmNshowValue,           True,
                          XmNorientation,         XmHORIZONTAL,
                          XmNtopAttachment,       XmATTACH_WIDGET,
                          XmNtopWidget,       selected_row_scale_,
                          XmNleftAttachment,      XmATTACH_FORM,
                          XmNleftOffset,          10,
                          XmNrightAttachment,     XmATTACH_FORM,
                          XmNrightOffset,         10,
                          NULL );

    hourMin_scale_ = XtVaCreateManagedWidget ( "Step Min",
                     xmScaleWidgetClass,     form1,
                     XtVaTypedArg, XmNtitleString, XmRString, "Step Min", 7,
                     XmNheight,              100,
                     XmNmaximum,             200,
                     XmNminimum,             1,
                     XmNvalue,               1,
                     XmNshowValue,           True,
                     XmNorientation,         XmHORIZONTAL,
                     XmNtopAttachment,       XmATTACH_WIDGET,
                     XmNtopWidget,       selected_col_scale_,
                     XmNleftAttachment,      XmATTACH_FORM,
                     XmNleftOffset,          10,
                     XmNrightAttachment,     XmATTACH_FORM,
                     XmNrightOffset,         10,
                     NULL );

    hourMax_scale_ = XtVaCreateManagedWidget ( "Step Max",
                     xmScaleWidgetClass,     form1,
                     XtVaTypedArg, XmNtitleString, XmRString, "Step Max", 7,
                     XmNheight,              100,
                     XmNmaximum,             200,
                     XmNminimum,             1,
                     XmNvalue,               1,
                     XmNshowValue,           True,
                     XmNorientation,         XmHORIZONTAL,
                     XmNtopAttachment,       XmATTACH_WIDGET,
                     XmNtopWidget,       hourMin_scale_,
                     XmNleftAttachment,      XmATTACH_FORM,
                     XmNleftOffset,          10,
                     XmNrightAttachment,     XmATTACH_FORM,
                     XmNrightOffset,         10,
                     NULL );

    Widget sep = XtVaCreateManagedWidget ( "sep",
                                           xmSeparatorWidgetClass,     form1,
                                           XmNleftAttachment,      XmATTACH_FORM,
                                           XmNrightAttachment,     XmATTACH_FORM,
                                           XmNtopAttachment,       XmATTACH_WIDGET,
                                           XmNtopWidget,           hourMax_scale_,
                                           XmNtopOffset,           10,
                                           NULL );

    // Button to start animating.
    apply_ = XtVaCreateManagedWidget ( "Apply",
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
    XtAddCallback ( apply_, XmNactivateCallback, &OptionManager::applyCB, ( XtPointer ) this );

    Widget close = XtVaCreateManagedWidget ( "Close",
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
    XtAddCallback ( close, XmNactivateCallback, &OptionManager::closeCB, ( XtPointer ) this );

    if ( XtIsManaged ( form1 ) ) XtUnmanageChild ( form1 );
    XtManageChild ( form1 );
    }


void OptionManager::selected_level_scaleCB ( Widget, XtPointer clientData, XtPointer )
    {
    OptionManager *obj = ( OptionManager * ) clientData;
    obj->selected_level_scale_cb();
    }

void OptionManager::selected_level_scale_cb()
    {
    XtVaGetValues ( selected_level_scale_, XmNvalue, &selected_level_, NULL );
    }


void OptionManager::applyCB ( Widget, XtPointer clientData, XtPointer )
    {
    OptionManager *obj = ( OptionManager * ) clientData;
    obj->apply_cb();
    }

void OptionManager::apply_cb()
    {
    XtVaGetValues ( selected_level_scale_, XmNvalue, &selected_level_, NULL );
    XtVaGetValues ( selected_row_scale_, XmNvalue, &selected_row_, NULL );
    XtVaGetValues ( selected_col_scale_, XmNvalue, &selected_col_, NULL );
    XtVaGetValues ( hourMin_scale_, XmNvalue, &hourMin_, NULL );
    XtVaGetValues ( hourMax_scale_, XmNvalue, &hourMax_, NULL );
    }

void OptionManager::closeCB ( Widget, XtPointer clientData, XtPointer )
    {
    OptionManager *obj = ( OptionManager * ) clientData;
    obj->close_cb();
    }


void OptionManager::close_cb()
    {
    XtUnmanageChild ( option_dialog_ );
    }


void OptionManager::setLevel_hi ( int valu )
    {
    XtVaSetValues ( selected_level_scale_, XmNmaximum, valu, NULL );
    }

void OptionManager::setRow_hi ( int valu )
    {
    XtVaSetValues ( selected_row_scale_, XmNmaximum, valu, NULL );
    }

void OptionManager::setColumn_hi ( int valu )
    {
    XtVaSetValues ( selected_col_scale_, XmNmaximum, valu, NULL );
    }
