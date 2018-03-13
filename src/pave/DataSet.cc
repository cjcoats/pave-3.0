/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: DataSet.cc 83 2018-03-12 19:24:33Z coats $
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
// DataSet.C
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 23, 1995
//
/////////////////////////////////////////////////////////////
//
// DataSet Class
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT  950523  Implemented
// SRT  960416  Added getIncrSdateStimeNsteps()
//
/////////////////////////////////////////////////////////////


#include "DataSet.h"




dataSet::dataSet ( char *hname, char *pname, char *fname,
                   linkedList *formulaList,
                   linkedList *domainList,
                   linkedList *levelList,
                   linkedList *dataSetList,   // parent's datasetList
                   void *speciesS,
                   AppInit *app,
                   Widget parent )
    {
    init ( hname, pname, fname, formulaList, domainList,
           levelList, dataSetList, speciesS, app, parent );
    }



dataSet::dataSet ( char *longname,
                   linkedList *formulaList,
                   linkedList *domainList,
                   linkedList *levelList,
                   linkedList *dataSetList,   // parent's datasetList
                   void *speciesS,
                   AppInit *app,
                   Widget parent )
    {
    char    hname[256], // host name
            pname[256], // path name
            fname[256]; // file name

    parseLongDataSetName ( longname, hname, pname, fname ); // find 3 components

    init ( hname, pname, fname, formulaList, domainList,
           levelList, dataSetList, speciesS, app, parent );
    }


dataSet::~dataSet()
    {
    free_vis ( &info_ ); // free up any memory that has been allocated for Meta data
    if ( stepDialogBox_ ) delete stepDialogBox_;
    }


void dataSet::init ( char *hname, char *pname, char *fname,
                     linkedList *formulaList,
                     linkedList *domainList,
                     linkedList *levelList,
                     linkedList *dataSetList,   // parent's datasetList
                     void *speciesS,
                     AppInit *app,
                     Widget parent )
    {
    assert ( fname && formulaList && domainList && levelList && speciesS && parent && app && dataSetList );
    memset ( fullname_, ( int ) 0, sizeof ( fullname_ ) );
    memset ( hostname_, ( int ) 0, sizeof ( hostname_ ) );
    memset ( pathname_, ( int ) 0, sizeof ( pathname_ ) );
    memset ( filename_, ( int ) 0, sizeof ( filename_ ) );
    memset ( ( void * ) &info_, ( int ) NULL, ( size_t ) sizeof ( VIS_DATA ) ); // init to all 0's
    setHostName ( hname );
    setPathName ( pname );
    setFileName ( fname );
    parent_ = parent;
    step_min_ = step_max_ = step_incr_ = 0;
    validDataSet_ = unsure_or_no;
    formList_ = formulaList;
    domainList_ = domainList;
    levelList_ = levelList;
    datasetList_ = dataSetList;
    speciesServer_ = speciesS;
    app_ = app;
    stepDialogBox_ = ( StepUI * ) NULL;
    }


// returns 1 if matches
int dataSet::match ( void *target )         // override baseType's
    {
    // virtual match()
    char *targetFullName = ( char * ) target;   // target should be a full name

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter dataSet::match(%s) in dataSet %s\n", ( char * ) target, getFileName() );
#endif // DIAGNOSTICS

    return ( ( !strcmp ( targetFullName, fullname_ ) ) || // for a dataset
             ( !strcmp ( targetFullName, filename_ ) ) );
    }


char *dataSet::getClassName ( void )        // override baseType's
    {
    // virtual getClassName()
    static char *myName = "dataSet";
    return myName;
    }


void dataSet::print ( FILE *output )
    {
    assert ( output );
    fprintf ( output, "dataSet's fullname == '%s'\n", fullname_ );

#ifdef DIAGNOSTICS
    fprintf ( output, " hostname_ == '%s'\n", hostname_ );
    fprintf ( output, " pathname_ == '%s'\n", pathname_ );
    fprintf ( output, " filename_ == '%s'\n", filename_ );
    fprintf ( output, " step_min_ == '%d'\n", step_min_ );
    fprintf ( output, " step_max_ == '%d'\n", step_max_ );
    fprintf ( output, "step_incr_ == '%d'\n", step_incr_ );
    dump_VIS_DATA ( &info_, NULL, NULL );
#endif
    }


int dataSet::setHostName ( char *hname )        // returns 0 if success
    {
    assert ( hname );
    strcpy ( hostname_, hname );
    updateFullName();
    return 0;
    }


int dataSet::setFileName ( char *fname )        // returns 0 if success
    {
    assert ( fname );
    strcpy ( filename_, fname );
    updateFullName();
    return 0;
    }


int dataSet::setPathName ( char *pname )        // returns 0 if success
    {
    assert ( pname );
    strcpy ( pathname_, pname );
    updateFullName();
    return 0;
    }


int dataSet::updateMetaData ( struct BusData *bdp, char *estring )
// returns 0 if success
    {
    Domain *domain;
    Level *level;
    int *target[3], g;

    assert ( bdp );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter dataSet::updateMetaData()\n" );
#endif

    // clear out & free up any previous Meta data that we're storing

    free_vis ( &info_ );

    // set up hostname info in VIS_DATA struct

    if ( strlen ( hostname_ ) )
        {
        if ( ! ( info_.filehost.name = ( char * ) malloc ( strlen ( hostname_ )+1 ) ) )
            {
            sprintf ( estring, "Couldn't allocate memory in dataSet::updateMetaData()" );
            free_vis ( &info_ );
            return 1;
            }
        strcpy ( info_.filehost.name, hostname_ );
        }

    // set up filename info in VIS_DATA struct

    if ( ! ( info_.filename = ( char * ) malloc ( strlen ( filename_ )+strlen ( pathname_ )+2 ) ) )
        {
        sprintf ( estring, "Couldn't allocate memory in dataSet::updateMetaData()" );
        free_vis ( &info_ );
        return 1;
        }
    info_.filename[0] = '\0';
    if ( pathname_[0] )
        {
        strcat ( info_.filename, pathname_ );
        strcat ( info_.filename, "/" );
        }
    strcat ( info_.filename, filename_ );


    // call get_info to find out about the dataset contents

#ifdef DIAGNOSTICS
    fflush ( stdout );
    fprintf ( stderr,
              "XXX DataSet.cc calling get_info() with info_.filename == '%s'...\n",
              info_.filename );
    fflush ( stderr );
#endif /* DIAGNOSTICS */

    g = get_info ( bdp, &info_, estring );

#ifdef DIAGNOSTICS
    fflush ( stdout );
    fprintf ( stderr, "XXX DataSet.cc just got back from get_info()...\n" );
    dump_VIS_DATA ( &info_, NULL, NULL );
    fflush ( stderr );
#endif /* DIAGNOSTICS */

    if ( !g )
        {
        free_vis ( &info_ );
        return 1;
        }
    estring[0] = '\0';
    if ( info_.dataset==netCDF_OBS )
        {
        isObs_=1;
        }
    else
        {
        isObs_=0;
        }

#ifndef USE_OLDMAP
    // now fix the M3IO header's map_info if necessary
    /* SRT 9607120 bogus data shouldn't be "fixed" */   M3IO_parameter_fix ( &info_ );
#endif /* USE_OLDMAP */

    // now keep copies of the stuff we actually care about
    step_min_ = 1;
    step_max_ = info_.nstep;
    step_incr_ = 1;

    // Make a stepDiagBox if we don't already have one
    if ( !stepDialogBox_ )
        {
        char title[255];
        char minLabel[255];
        char maxLabel[255];
        void ( *valsModifiedParentCB ) ( void * );
        int nbytes, mydate, mytime, myincr, hrinc, mininc, secinc, i;

        sprintf ( title, "%s Times of Interest", getFileName() );
        sprintf ( minLabel, "Min Time" );
        sprintf ( maxLabel, "Max Time" );
        valsModifiedParentCB = ( void ( * ) ( void * ) ) modifiedTheseStepsCB;

        if ( ( !info_.sdate ) || ( !info_.stime ) ) // added 960502 SRT as per AT
            {
            // get_info did not fill up the sdate and stime fields
            // of a VIS_DATA struct, as does get_data.  BUT, the StepUI
            // needs sdate and stime fields in order to display the
            // date/time info in the dialog box.  So... go ahead and
            // allocate sdate and stime fields, and fill them up
            // based on the first_date, first_time, and incr_sec fields.
            // Basically this is a HACK, as ideally you should grab all
            // the sdate/stime integers from the file.  This HACK is
            // assuming that sdate/stime returned by any get_data calls
            // will match the sdate/stime fields calculated here - SRT 960412
            nbytes = sizeof ( int ) *info_.nstep;
            info_.sdate = ( int * ) malloc ( nbytes );
            info_.stime = ( int * ) malloc ( nbytes );
            if ( ( !info_.sdate ) || ( !info_.stime ) )
                {
                if ( info_.sdate ) free ( info_.sdate );
                info_.sdate = NULL;
                if ( info_.stime ) free ( info_.stime );
                info_.stime = NULL;
                }
            else
                {
                mydate = info_.sdate[0] = info_.first_date;
                mytime = info_.stime[0] = info_.first_time;
                hrinc = info_.incr_sec/3600;
                mininc = ( info_.incr_sec- ( hrinc*3600 ) ) /60;
                secinc = ( info_.incr_sec- ( hrinc*3600 )- ( mininc*60 ) );
                myincr =  hrinc*10000+mininc*100+secinc;

#ifdef DIAGNOSTICS
                fprintf ( stderr, "\n\n" );
                fprintf ( stderr, "first_date = %d, first_time = %d, incr_sec = %d, ",
                          info_.first_date, info_.first_time, info_.incr_sec );
                fprintf ( stderr, "hrinc = %d, mininc = %d, secinc = %d, myincr = %d\n",
                          hrinc, mininc, secinc, myincr );
                i = 0;
                fprintf ( stderr, "info_.sdate[%d] == %d, info_.stime[%d] == %d\n",
                          i, info_.sdate[i], i, info_.stime[i] );
#endif // DIAGNOSTICS

                for ( i = 1; i < info_.nstep; i++ )
                    {
                    nextimec ( &mydate , &mytime , myincr );
                    info_.sdate[i] = mydate;
                    info_.stime[i] = mytime;
#ifdef DIAGNOSTICS
                    fprintf ( stderr, "info_.sdate[%d] == %d, info_.stime[%d] == %d\n",
                              i, info_.sdate[i], i, info_.stime[i] );
#endif // DIAGNOSTICS
                    }
                }
            }

        stepDialogBox_ = new StepUI (
            title,                      // title of dialog box
            parent_,                    // to base position on
            minLabel,           // "Step Min", "Layer Min", etc
            maxLabel,           // "Step Max", "Layer Max", etc
            step_min_,          // 0, 1, etc
            ( step_max_<=step_min_ ) ?step_min_+1:step_max_,        // 24, 72, etc
            info_.sdate,  /* Julian start date for time steps 960412 SRT */
            info_.stime,  /* Julian start time for time steps 960412 SRT */
            &step_min_,         // current Min
            &step_max_,             // current Max
            valsModifiedParentCB,       // call this when changed
            ( void * ) this,        // this object
            estring                     // to hold error msgs
        );

        // 960502 SRT if (info_.sdate) free(info_.sdate); info_.sdate = NULL;
        // 960502 SRT if (info_.stime) free(info_.stime); info_.stime = NULL;

        if ( estring[0] != '\0' )
            {
            delete stepDialogBox_;
            stepDialogBox_ = ( StepUI * ) NULL;
            return 1;
            }
        }

    // Make sure we have a domain if there isn't already one
    target[0] = &info_.ncol;
    target[1] = &info_.nrow;
    target[2] = ( int* ) info_.map_info;
    if ( ! ( domain = ( Domain * ) domainList_->find ( target ) ) )
        {
        if ( ( ! ( domain = new Domain (    formList_,
                                            datasetList_,
                                            info_.ncol,
                                            info_.nrow,
                                            info_.map_info,
                                            app_,
                                            estring ) ) )
                ||
                ( estring[0] != '\0' ) )
            {
            delete domain;
            return 1;
            }
        else
            {
            domainList_->addTail ( domain );
            domain = ( Domain * ) NULL;
            }
        }
#ifdef DIAGNOSTICS
    else
        fprintf ( stderr,
                  "In DataSet::updateMetaData Domain (%d,%d,%s) already added!",
                  info_.ncol, info_.nrow, info_.map_info );
#endif


    // Make sure we have a Level Object if there isn't already one
    if ( ! ( level = ( Level * ) levelList_->find ( &info_.nlevel ) ) )
        {
        if ( ( ! ( level = new Level (  formList_,
                                        info_.nlevel,
                                        parent_,
                                        estring ) ) )
                ||
                ( estring[0] != '\0' ) )
            {
            delete level;
            return 1;
            }
        else
            {
            levelList_->addTail ( level );
            level = ( Level * ) NULL;
            }
        }
#ifdef DIAGNOSTICS
    else
        fprintf ( stderr,
                  "In DataSet::updateMetaData Level (%d) already added!",
                  info_.nlevel );
#endif

    return 0;
    }


int dataSet::updateFullName ( void )        // returns 0 if success;
    {
    memset ( fullname_, ( int ) 0, sizeof ( fullname_ ) );
    if ( hostname_[0] ) sprintf ( fullname_, "%s:", hostname_ );
    if ( pathname_[0] )
        {
        strcat ( fullname_, pathname_ );
        strcat ( fullname_, "/" );
        }
    strcat ( fullname_, filename_ );
    return 0;
    }



static void modifiedTheseStepsCB ( void *obj ) // run this if user subselects
// on the dataset
    {
    Formula *formula;
    dataSet *dset = ( dataSet * ) obj;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter \"%s\"'s modifiedTheseStepsCB() in DataSet.cc\n",
              dset->getFullName() );
#endif // DIAGNOSTICS

    // NOTIFY EACH FORMULA THAT WE'VE CHANGED THE DATASET STEP RANGE
    formula = ( Formula * ) dset->getFormulaList()->head();
    while ( formula )
        {
        formula->dataSetStepsWereChanged ( dset->getFullName(),
                                           dset->get_step_min(),
                                           dset->get_step_max() );
        formula = ( Formula * ) dset->getFormulaList()->next();
        }
    }



// is this a valid dataSet?  Returns 1 if yes
int dataSet::isValid ( struct BusData *bdp, char *estring )
    {
    if ( validDataSet_ == yes ) return 1;

    if ( updateMetaData ( bdp, estring ) )
        {
        validDataSet_ = unsure_or_no;
        return 0;
        }

    validDataSet_ = yes;
    return 1;
    }


// provide a UI to edit this DataSet's step, step_min, step_max
int dataSet::editDataSetSteps ( void )
    {
    stepDialogBox_->postOptionDialog();
    return 0;
    }


VIS_DATA *dataSet::get_vdata_ptr ( void )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "dataSet::get_vdata_ptr() about to return %ld\n", ( long ) &info_ );
#endif // DIAGNOSTICS

    return &info_;
    }


// returns time step increment (in secs),
// first sdate/stime in selected range (in IO/API Julian format),
// and number time steps in selected range for this dataset
// returns 1 if error
int dataSet::getIncrSdateStimeNsteps
( int *incr, int *sdate, int *stime, int *nsteps,
  int **sdates, int **stimes )

    {
    if ( ( !incr ) || ( !sdate ) || ( !stime ) || ( !nsteps ) )
        {
        fprintf ( stderr, "Bogus args to dataSet::getIncrSdateStimeNsteps()!\n" );
        return 1;
        }

    if ( validDataSet_ != yes )
        {
        fprintf ( stderr, "Called dataSet::getIncrSdateStimeNsteps() on "
                  "an invalid dataset!\n" );
        return 1;
        }

    if ( !stepDialogBox_ )
        {
        fprintf ( stderr, "Called dataSet::getIncrSdateStimeNsteps() on "
                  "a dataset with no StepUI!\n" );
        return 1;
        }

    *incr = info_.incr_sec;
    *sdate = stepDialogBox_->getFirst_sdate_InRange();
    *stime = stepDialogBox_->getFirst_stime_InRange();
    *nsteps = stepDialogBox_->getNStepsInRange();

    if ( sdates ) *sdates = &info_.sdate[stepDialogBox_->getFirst_Offset_InRange()];
    if ( stimes ) *stimes = &info_.stime[stepDialogBox_->getFirst_Offset_InRange()];

    if ( ( ( ! ( *incr ) ) && ( ( *nsteps ) !=1 ) ) || ( *sdate == -1 ) ||
            ( *stime == -1 ) || ( *nsteps <= 0 ) )
        {
        fprintf ( stderr, "Bogus results from "
                  "dataSet::getIncrSdateStimeNsteps():\n" );
        fprintf ( stderr,
                  "*incr == %d, *sdate == %d, *stime == %d, *nsteps == %d\n",
                  *incr, *sdate, *stime, *nsteps );
        return 1;
        }

    return 0;
    }
