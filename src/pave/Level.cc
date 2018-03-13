/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Level.cc 83 2018-03-12 19:24:33Z coats $
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
/////////////////////////////////////////////////////////////
//
// Level.C
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 29, 1995
//
/////////////////////////////////////////////////////////////
//
//   Every Level Object:
//
// o When requested, must provide a UI which manages a
//   dynamic "mask" of its currently selected levels
//
// o Must notify each of the Formulas when its currently
//   selected levels change
//
// o Must retrieve and supply its data when requested
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT  950529  Implemented
//
/////////////////////////////////////////////////////////////


#include "Level.h"

// Constructor

Level::Level ( linkedList *formulaList, // points to the list of
               // Formula objects this
               // Object may need to
               // interact with

               int nlevels,          // the number of layers this
               // object needs to manage

               Widget parent,        // to base position of UI on

               char *estring )
    {
    assert ( estring != NULL );
    estring[0] = '\0';
    KMAX_ = nlevels;
    formulaList_ = formulaList;
    parent_ = parent;
    levelDialogBox_ = NULL;
    minLevel_ = maxLevel_ = 1;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Level::Level(%d)\n", nlevels );
#endif // DIAGNOSTICS

    if ( nlevels <= 0 )
        sprintf ( estring, "Bad nlevels == %d", nlevels );
    else if ( nlevels > 1 )
        {
        char title[50];
        char minLabel[50];
        char maxLabel[50];
        void ( *valsModifiedParentCB ) ( void * );

        sprintf ( title, "%d Layer Models", KMAX_ );
        sprintf ( minLabel, "Lowest Layer Of Interest" );
        sprintf ( maxLabel, "Highest Layer Of Interest" );
        valsModifiedParentCB = ( void ( * ) ( void * ) ) modifiedTheseLevelsCB;

        levelDialogBox_ = new StepUI (
            title,                          // title of dialog box
            parent_,                        // to base position on
            minLabel,                       // "Step Min", "Layer Min", etc
            maxLabel,                       // "Step Max", "Layer Max", etc
            1,                              // 0, 1, etc
            KMAX_,                      // 24, 72, etc
            NULL,       /* Julian start date for time steps 960412 SRT */
            NULL,       /* Julian start time for time steps 960412 SRT */
            &minLevel_,                     // current Min
            &maxLevel_,                     // current Max
            valsModifiedParentCB,           // call this when changed
            ( void * ) this,                // this object
            estring                         // to hold error msgs
        );
        }
    }



Level::~Level()                 // Destructor
    {
    if ( levelDialogBox_ ) delete levelDialogBox_;
    }



int Level::match ( void *target )   // override baseType's
// virtual match()
    {
    int *ip = ( int * ) target;

    return ( *ip == KMAX_ );
    }



char *Level::getClassName ( void )      // override baseType's
// virtual getClassName()
    {
    static char *myName = "Level";
    return myName;
    }



void Level::print ( FILE *output )  // override linkedList's print()
    {
    fprintf ( output, "-----------\n" );
    fprintf ( output, "Level print\n" );
    fprintf ( output, "-----------\n" );
    fprintf ( output, "KMAX_ == %d\n", KMAX_ );
    fprintf ( output, "minLevel_ == %d\n", minLevel_ );
    fprintf ( output, "maxLevel_ == %d\n", maxLevel_ );
    }



int *Level::getCopyOfLevels ( void ) // to supply a copy of its data in array format -
// 0 for off, 1 for on
    {
    int i, *ans;

    ans = ( int * ) malloc ( KMAX_*sizeof ( int ) );
    if ( ans != NULL )
        for ( i = 0; i < KMAX_; i++ )
            ans[i] =  ( ( i+1 >= minLevel_ ) && ( i+1 <= maxLevel_ ) );

    return ans;
    }



int Level::showUI ( void )      // brings up a UI which allows
// the user to modify the layers
// selected
    {
    if ( levelDialogBox_ )
        levelDialogBox_->postOptionDialog();

#ifdef DIAGNOSTICS
    else if ( KMAX_ == 1 )
        fprintf ( stderr, "Can't select min/max layers when there is only one layer\n" );
    else
        fprintf ( stderr, "No levelDialogBox_ to edit with "
                  "in Level::showUI()!\n" ); // SRT
#endif // DIAGNOSTICS

    return 0;
    }


int Level::setMinAndMaxLevelTo ( int l )
    {
    if ( ( l >= 1 ) && ( l <= get_nLevels() ) )
        levelDialogBox_->setMinMax ( l, l );

    return 0;
    }


int Level::setLevelRange ( int lmin, int lmax )
    {
    int t;

    if ( lmin > lmax )
        {
        t = lmin;
        lmin = lmax;
        lmax = t;
        }

    if ( ( lmin >= 1 ) && ( lmin <= get_nLevels() ) )
        if ( ( lmax >= 1 ) && ( lmax <= get_nLevels() ) )
            levelDialogBox_->setMinMax ( lmin, lmax );

    return 0;
    }




static void modifiedTheseLevelsCB ( void *obj ) // run this if user subselects
// on this Level's range
    {
    Level *lev = ( Level * ) obj;
    Formula *formula;

    // NOTIFY EACH FORMULA THAT WE'VE CHANGED A LEVEL RANGE
    formula = ( Formula * ) lev->get_formula_list()->head();
    while ( formula )
        {
#ifdef DIAGNOSTICS
        fprintf ( stderr, "Level::modifiedTheseLevelsCB() about to call "
                  "formula(%s)->levelsWereChanged(%d)\n", formula->getFormulaName(), lev->get_nLevels() );
#endif // DIAGNOSTICS

        formula->levelsWereChanged ( lev->get_nLevels() );
        formula = ( Formula * ) lev->get_formula_list()->next();
        }
    }
