/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Domain.cc 83 2018-03-12 19:24:33Z coats $
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
// Domain.h
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 29, 1995
//
/////////////////////////////////////////////////////////////
//
//   Every Domain Object:
//
// o When requested, must provide a UI which manages a
//   dynamic "mask" of its currently selected cells
//
// o Must notify each of the Formulas when its currently
//   selected cells change
//
// o Must retrieve and supply its data when requested
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT  950529  Implemented
// SRT  951107  Added setRange() and verify_rvd()
// SRT  960525  Added saveDomain() and loadDomain()
//
/////////////////////////////////////////////////////////////


/* bald messes this up in some header file */
#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */


#include "Domain.h"


// Constructor

Domain::Domain ( linkedList *formulaList, // points to the list of
                 // Formula objects this
                 // Object may need to
                 // interact with

                 linkedList *dataSetList,  // points to the list of
                 // DataSet objects this
                 // Object may need to
                 // interact with

                 int ni,           // Number columns of full domain

                 int nj,           // Number rows of full domain

                 char *mapInfo,        // map location info string

                 AppInit *app,             //

                 char *estring )       // for error msgs
    {
    int i, j;

    assert ( estring );
    assert ( ( ni>0 ) );
    assert ( ( nj>0 ) );
    assert ( ( mapInfo ) );
    assert ( ( formulaList ) );
    assert ( ( dataSetList ) );
    assert ( ( app ) );
    assert ( estring &&
             ( ni>0 ) &&
             ( nj>0 ) &&
             ( mapInfo ) &&
             ( formulaList ) &&
             ( dataSetList ) &&
             ( mapinfo_ = strdup ( mapInfo ) ) &&
             ( app ) &&
             ( whichCells_ = ( char * ) malloc ( ni*nj ) ) );
    estring[0] = '\0';
    formulaList_ = formulaList;
    datasetList_ = dataSetList;
    IMAX_ = ni;
    JMAX_ = nj;
    for ( j = 0; j < JMAX_; j++ )
        for ( i = 0; i < IMAX_; i++ )
            whichCells_[INDEX ( i,j,0,0,IMAX_,JMAX_,1 )] =

#ifdef DOMFIX

                ( ( ( i>0 ) && ( j>0 ) ) ? 100 : 0 ); // HACK !! should all be 100's!  // SRT DOMFIX
    // this is a quick short term HACK to get the domain selecting working for Ambro's demo
    // this HACK gets around a problem with plots with maps on them (ie DomainWnd, TileWnd plots, etc)
    // this HACK sets the left most column and bottom row to 0's, since the windows don't show them

#else // SRT DOMFIX
                100; // SRT // SRT DOMFIX
#endif // DOMFIX

    rvd_ = ( ReadVisData * ) NULL;
    dw_ = ( DomainWnd * ) NULL;
    app_ = app;
    }



Domain::~Domain()                   // Destructor
    {
    free ( mapinfo_ );
    free ( whichCells_ );
    if ( rvd_ ) delete rvd_;
    if ( dw_ ) delete dw_;
    }


int Domain::setRange ( int xmin, int ymin, int xmax, int ymax ) // 1 based
    {
    char tstring[256];
    int returnval;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Domain::setRange(%d,%d,%d,%d)\n",xmin,ymin,xmax,ymax );
#endif // DIAGNOSTICS

    if ( verify_rvd ( tstring ) )
        {
        fprintf ( stderr, "Domain::setRange() error : \"%s\"\n", tstring );
        return 1;
        }

    if ( !dw_ ) // create a DomainWnd for this domain
        {
        char name[255];

        sprintf ( name, "%s %dx%d", mapinfo_, IMAX_, JMAX_ );
        dw_ = new DomainWnd ( app_, name, rvd_, "DOMAIN",
                              // SRT 950721 (Dimension)(50+450*(float)IMAX_/JMAX_), 450,

                              /* SRT width 950721 */  ( Dimension ) ( 160+ // 100=left + 60=right
                                      IMAX_* ( 0.91*290.0/ ( JMAX_ ) ) ),
                              // SRT 0.91 takes into account the non-square pixels on monitor
                              450, // SRT 950721 height (100=top 60=bottom 290=tiles)
                              whichCells_,
                              mapinfo_, formulaList_, 0 );
        }

    returnval = dw_->setRange ( xmin, ymin, xmax, ymax );
    // dw_->showWindow(); // 960913 Added SRT
    // dw_->drawDetail(); // 960913 Added SRT
    dw_->hideWindow();
    return returnval;
    }

int Domain::match ( void *target )  // override baseType's
// virtual match()
    {
    int **targetpp = ( int ** ) target;
    int ni, nj;
    char *mapInfo;
    int returnval;
    char estring[128];

    assert ( target );
    estring[0] = '\0';
    ni = * ( targetpp[0] );
    nj = * ( targetpp[1] );
    mapInfo = ( char * ) targetpp[2];

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Domain %s's::match(%d, %d, '%s')\n",
              mapinfo_, ni, nj, mapInfo );
#endif // DIAGNOSTICS

    returnval = ( ( ni == IMAX_ ) &&
                  ( nj == JMAX_ ) &&
                  ( map_infos_areReasonablyEquivalent ( mapInfo, mapinfo_, estring ) ) );

#ifdef DIAGNOSTICS
    if ( returnval )
        fprintf ( stderr, "Domain::match() matched !!\n" );
    else
        fprintf ( stderr, "Domain::match() did NOT match :(\n" );
    if ( estring[0] ) fprintf ( stderr, "%s\n", estring );
#endif // DIAGNOSTICS

    return returnval;
    }



char *Domain::getClassName ( void )     // override baseType's
// virtual getClassName()
    {
    static char *myName = "Domain";
    return myName;
    }



void Domain::print ( FILE *output ) // override linkedList's print()
    {
    fprintf ( output, "------------\n" );
    fprintf ( output, "Domain print\n" );
    fprintf ( output, "------------\n" );
    fprintf ( output, "   IMAX_ == %d\n", IMAX_ );
    fprintf ( output, "   JMAX_ == %d\n", JMAX_ );
    fprintf ( output, "mapinfo_ == '%s'\n", mapinfo_ );
    }


int Domain::verify_rvd ( char *estring )
    {
    char tstring[128];

    if ( !rvd_ ) // we need a ReadVisData object to create the DrawWnd
        {
        dataSet *dset;
        VIS_DATA *vdp = NULL;

        if ( datasetList_ == NULL )
            dset = ( dataSet * ) NULL;
        else
            dset = ( dataSet * ) datasetList_->head();

        while ( dset && ( !vdp ) )
            {
            if (    ( dset->get_ncol() == IMAX_ ) &&
                    ( dset->get_nrow() == JMAX_ ) &&
                    map_infos_areReasonablyEquivalent
                    ( dset->getMapInfo(), mapinfo_, tstring ) )

                // we've got a match !
                vdp = dset->get_vdata_ptr();

            dset = ( dataSet * ) datasetList_->next(); // dset->next(); SRT
            }

        if ( !vdp )
            {
            sprintf ( estring, "There are no dataSets with this Domain !" );
            return 1;
            }

        if ( ! ( rvd_ = new ReadVisData ( "Domain", ( BusData * ) NULL, vdp ) ) )
            {
            sprintf ( estring, "Couldn't allocate a ReadVisData in Domain::showUI()!" );
            return 1;
            }
        }
    return 0;
    }



int Domain::showUI ( char *estring ) // brings up a UI which allows
// the user to modify the layers
// selected
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Domain::showUI()\n" );
#endif // DIAGNOSTICS


    if ( verify_rvd ( estring ) ) // we need a ReadVisData object to create the DrawWnd
        return 1;

    if ( !dw_ ) // create and display a DomainWnd for the domain
        {
        char name[255];

        sprintf ( name, "%s %dx%d", mapinfo_, IMAX_, JMAX_ );
        dw_ = new DomainWnd ( app_, name, rvd_, "DOMAIN",
                              // SRT 950721 (Dimension)(50+450*(float)IMAX_/JMAX_), 450,

                              /* SRT width 950721 */  ( Dimension ) ( 160+ // 100=left + 60=right
                                      IMAX_* ( 0.91*290.0/ ( JMAX_ ) ) ),
                              // SRT 0.91 takes into account the non-square pixels on monitor
                              450, // SRT 950721 height (100=top 60=bottom 290=tiles)
                              whichCells_,
                              mapinfo_, formulaList_, 0 );
        }
    else
        // display the already existing DomainWnd for the domain
        dw_->showWindow();

    return 0;
    }



char *Domain::getCopyOfPercents ( char *estring )   // to supply a copy of its data
    {
    int i;
    char *ans;

    ans = ( char * ) malloc ( IMAX_*JMAX_ );
    if ( ans )
        {
        for ( i = 0; i < IMAX_*JMAX_; i++ )
            ans[i] = whichCells_[i];
        }
    else
        sprintf ( estring, "malloc failure in Domain::getCopyOfPercents()" );
    return ans;
    }



int Domain::getRange ( int *xmin, int *ymin, int *xmax, int *ymax ) // returns *1* based
    {
    int i, j, on = 0;
    *xmin = IMAX_+1;
    *xmax = -1;
    *ymin = JMAX_+1;
    *ymax = -1;


    for ( j = 0; j < JMAX_; j++ )
        for ( i = 0; i < IMAX_; i++ )
            if ( whichCells_[INDEX ( i,j,0,0,IMAX_,JMAX_,1 )] )
                {
                if ( i < *xmin ) *xmin = i;
                if ( i > *xmax ) *xmax = i;
                if ( j < *ymin ) *ymin = j;
                if ( j > *ymax ) *ymax = j;
                on = 1;
                }
    *xmin = *xmin + 1;
    *xmax = *xmax + 1;
    *ymin = *ymin + 1;
    *ymax = *ymax + 1;
    return ( on ? 0 : 1 );
    }
