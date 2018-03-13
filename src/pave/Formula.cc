/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Formula.cc 83 2018-03-12 19:24:33Z coats $
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
// Formula.C
// Steve Thorpe
// thorpe@ncsc.org
// NCSC Environmental Programs, MCNC
// May 29, 1995
//
/////////////////////////////////////////////////////////////
//
//   Every Formula Object:
//
// o Must supply its (in)validity when reqested
//
// o Must retrieve and supply its data when requested
//
// o Needs to request info from the list of available DataSet,
//   Level, and Domain objects in order to determine (in)validity
//   of itself and its data
//
/////////////////////////////////////////////////////////////
//
// Revision History
// SRT  950529  Implemented
// SRT  950831  Added association with FormulaServer objects
// SRT  950831  Added formulaS_->removeThisItem() call in invalidateThisFormula()
// SRT  960416  Added updateCaseNamesAndTimes()
//
/////////////////////////////////////////////////////////////


#include "Formula.h"


// Constructor

Formula::Formula ( char *formulaP,      // infix formula typed in by user

                   struct BusData *bdP,    // needed to communicate with
                   // the SW Bus; this should already
                   // have been initialized with
                   // initVisDataClient()

                   linkedList *dataList,   // the list of DataSet objects to
                   // refer to when parsing a formula

                   linkedList *domList,    // the list of Domain objects to
                   // determine which cells are on
                   // for a formula

                   linkedList *levList,    // the list of Levels objects to
                   // determine which layers are on
                   // for a formula

                   Widget parent,      // parent of this puppy

                   void *formulaS,     // FormulaServer using this puppy

                   char *estring       // for error msgs
                 )
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Formula::Formula('%s')\n", formulaP ); // SRT
#endif // DIAGNOSTICS

    assert ( formulaP && bdP && dataList && domList &&
             levList && estring && parent );
    estring[0] = '\0';

    validFormula_ = unsure_or_no;
    validFormulaData_ = unsure_or_no;
    memset ( ( char * ) &info_, ( int ) NULL, ( size_t ) sizeof ( VIS_DATA ) ); // init to 0's

    bd_ = bdP;
    datasetList_ = dataList;
    domainList_ = domList;
    levelList_ = levList;
    parent_ = parent;
    hostOnlyList_ = caseOnlyList_ = caseList_ = NULL;
    whichLevel_ = NULL;
    ncases_ = 0;
    stepDialogBox_ = ( StepUI * ) NULL;
    formulaS_ = formulaS;
    selectedStep_ = 1;
    // init to all 0's
    memset ( ( char * ) &caseUsed_, ( int ) NULL, ( size_t ) ( MAX_INT*sizeof ( char ) ) );
    if ( ! ( infixFormula_ = strdup ( formulaP ) ) )
        sprintf ( estring, "Couldn't allocate memory in Formula::Formula!" );

    use_floor_ = ( getenv ( "DENOMINATOR_CUTOFF" ) != NULL );
    floorCut_ = use_floor_ ? atof ( getenv ( "DENOMINATOR_CUTOFF" ) ) : 0.0;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "In Formula::Formula(), use_floor_==%d, floorCut_==%g\n",
              use_floor_, floorCut_ );
#endif // DIAGNOSTICS
    userUnit_ = NULL;
    }




Formula::~Formula()             // Destructor
    {
    free_vis ( &info_ );
    if ( stepDialogBox_ )
        {
        delete stepDialogBox_;
        stepDialogBox_ = NULL;
        }
    if ( infixFormula_ ) free ( infixFormula_ );
    infixFormula_ = NULL;
    if ( caseList_ ) free ( caseList_ );
    caseList_ = NULL;
    if ( caseOnlyList_ ) free ( caseOnlyList_ );
    caseOnlyList_ = NULL;
    if ( hostOnlyList_ ) free ( hostOnlyList_ );
    hostOnlyList_ = NULL;
    if ( whichLevel_ ) free ( whichLevel_ );
    whichLevel_ = NULL;
    }



int Formula::match ( void *target ) // override baseType's
// virtual match()
    {
    char *form;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter '%s's Formula::match('%s')\n", infixFormula_, ( char * ) target );
#endif // DIAGNOSTICS

    form = ( char * ) target;
    if ( !form ) return 0;
    if ( !form[0] ) return 0;
    if ( !infixFormula_ ) return 0;
    if ( !infixFormula_[0] ) return 0;
    return ( !strcmp ( form, infixFormula_ ) );
    }



char *Formula::getClassName ( void )    // override baseType's
// virtual getClassName()
    {
    static char *myName = "Formula";
    return myName;
    }



void Formula::print ( FILE *output ) // override linkedList's print()
    {
    fprintf ( output, "Formula print " );
    if ( infixFormula_ )
        fprintf ( output, "('%s') ", infixFormula_ );
    else
        fprintf ( output, "[NULL] " );
    fprintf ( output, "Formula " );
    if ( validFormula_ != yes ) fprintf ( output, "NOT " );
    fprintf ( output, "valid; " );
    fprintf ( output, "data " );
    if ( validFormulaData_ != yes ) fprintf ( output, "NOT " );
    fprintf ( output, "valid" );
    if ( validFormulaData_ == yes )
        fprintf ( output, " slice_type == %d", ( int ) info_.slice );
    dump_VIS_DATA ( &info_, NULL, NULL );
    fprintf ( output, "\n" );
    fflush ( output );
    }


// update the time step information for each case,
// also update the names for each case
// returns 1 if error
int Formula::updateCaseNamesAndTimes ( char *estring ) // error msgs will go here
    {
    int     lenCase = 1;     // 1 for the trailing '\0'
    int     lenHost = 1;     // 1 for the trailing '\0'
    int     lenFullList = 1; // 1 for the trailing '\0'
    dataSet *dset;
    int i;

    if ( caseList_ )     free ( caseList_ );
    caseList_ = NULL;
    if ( caseOnlyList_ ) free ( caseOnlyList_ );
    caseOnlyList_ = NULL;
    if ( hostOnlyList_ ) free ( hostOnlyList_ );
    hostOnlyList_ = NULL;

    ncases_ = 0;
    if ( datasetList_ == NULL )
        dset = NULL;
    else
        dset = ( dataSet * ) datasetList_->head();

    while ( dset )
        {
        caseNhours_[ncases_] = dset->getNStepsInRange();
        caseStepMin_[ncases_] = dset->get_step_min();
        caseStepMax_[ncases_] = dset->get_step_max();
        caseStepIncr_[ncases_] = dset->get_step_incr();
        ncases_++;
        lenCase += strlen ( dset->getPathName() ) + // path name
                   strlen ( dset->getFileName() ) + // file name
                   2;   // ',' plus '/' between path name and file name
        if ( strlen ( dset->getHostName() ) )
            lenHost += strlen ( dset->getHostName() ) + 1; // ','
        else
            lenHost += strlen ( getLocalHostName() ) + 1; // ','
        lenFullList += strlen ( dset->getFullName() ) +
                       1;   // ','
        dset = ( dataSet * ) datasetList_->next(); // dset->next(); SRT
        }

    if ( !ncases_ )
        {
        sprintf ( estring, "Can't find any cases in Formula::updateCaseNamesAndTimes()" );
        return 1;
        }

    caseOnlyList_ = ( char * ) malloc ( lenCase );
    hostOnlyList_ = ( char * ) malloc ( lenHost );
    caseList_     = ( char * ) malloc ( lenFullList );
    assert ( caseOnlyList_ && hostOnlyList_ && caseList_ );
    caseOnlyList_[0] = hostOnlyList_[0] = caseList_[0] = '\0';

    if ( datasetList_ == NULL )
        dset = NULL;
    else
        dset = ( dataSet * ) datasetList_->head();

    for ( i = 0; i < ncases_; i++ )
        {
        strcat ( caseList_, dset->getFullName() );
        if ( i < ncases_-1 ) strcat ( caseList_, "," );

        if ( strlen ( dset->getHostName() ) )
            strcat ( hostOnlyList_, dset->getHostName() );
        else
            strcat ( hostOnlyList_, getLocalHostName() );
        if ( i < ncases_-1 ) strcat ( hostOnlyList_, "," );

        strcat ( caseOnlyList_, dset->getPathName() );
        strcat ( caseOnlyList_, "/" );
        strcat ( caseOnlyList_, dset->getFileName() );
        if ( i < ncases_-1 ) strcat ( caseOnlyList_, "," );

        dset = ( dataSet * ) datasetList_->next(); // dset->next(); SRT
        }

    return 0;
    }

extern "C" {
    int evalTokens ( char *, char **, int * );
    }

int Formula::isFormulaValid
( char *estring ) // is this a valid formula?
// returns 1 if valid formula
// error msgs will be written here
    {
    char    *formula;
    int     r;
#define MAXTOKENS 1024
    char *token[MAXTOKENS];
    int tflag[MAXTOKENS];
    int ntoken;
    int i;
    int len;
    char caseletter;

    char fmlBuffer[2048];
    linkedList *alList;


    if ( validFormula_ == yes )
        return 1;

    if ( updateCaseNamesAndTimes ( estring ) )
        return 0;

    formula = ( char * ) malloc ( strlen ( infixFormula_ )+100 ); // leave 100 xtra for
    // "prettying up" the formula
    assert ( formula );
    strcpy ( formula, infixFormula_ );

    alList = new linkedList;
    //fprintf(stderr,"DEBUG:calling evalTokens with %s n=%d\n",formula,ntoken);
    ntoken=evalTokens ( formula, token, tflag );
    fmlBuffer[0]='\0';
    for ( i=0; i<ntoken; i++ )
        {
        //fprintf(stderr,"token[%d]=%s\n",i,token[i]);
        if ( tflag[i] == 0 )
            {
            len = strlen ( fmlBuffer );
            sprintf ( fmlBuffer+len,"%s",token[i] );
            }
        else
            {
            len = strlen ( token[i] ) - 1;
            caseletter = tolower ( token[i][len] );
            if ( caseletter  < 'a' || caseletter > 'z' )
                {
                fprintf ( stderr,"Error in alias parsing...wrong case %c\n",caseletter );
                break;
                }
            token[i][len] = '\0';
            if ( ( ( FormulaServer * ) formulaS_ )->checkAlias ( alList,token[i] ) <0 )
                {
                printf ( "RECURSION for formula '%s'\n",formula );
                return 0;
                }
            if ( ! ( ( FormulaServer * ) formulaS_ )->expandAlias ( fmlBuffer,token[i],caseletter ) )
                {
                len = strlen ( fmlBuffer );
                sprintf ( fmlBuffer+len,"%s%c",token[i],caseletter );
                }
            }
        }
    //fprintf(stdout,"DEBUG::Expanded formula=%s\n",fmlBuffer);

    for ( i=0; i<ntoken; i++ )
        {
        if ( token[i] ) free ( token[i] );
        }

    //fprintf(stdout,"DEBUG::calling parseFormula=%s\n",fmlBuffer);
    r = parseFormula (

            // INPUTS TO parseFormula

            //                formula,        // infix formula typed by user
            fmlBuffer,

            caseOnlyList_,  // a list of cases (ie data file
            // names) in order (a..?),
            // separated by commas

            hostOnlyList_,       // a list of hosts (ie
            // "todd.hpcc.epa.gov,ozone,
            // "flyer.ncsc.org") separated by
            // commas, one for each case in
            // caseList

            bd_,            // to communicate with the SW Bus


            // MODIFIED BY parseFormula

            estring,        // errormsg if any

            postFixQueue_,  // postfix formula result

            caseUsed_,      // "010" if ncases = 3 and only
            // case b in formula

            whichUnit_,     // the units of the formula's output

            &dim_,          // ndim of formula result's data

            &dateDay_,      // the day of formula result

            &dateMonth_,    // the month of formula result

            &dateYear_,     // the year of formula result

            &hourStart_,    // starting hour in GMT

            &mixCase_,      // 1 if can't put a time on the
            // starting hour, otherwise 0

            &IMAX_,     // imax of data

            &JMAX_,     // jmax of data

            &KMAX_      // kmax of data
        );

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Formula::isFormulaValid() just set '%s''s whichUnit_ to '%s'\n",
              formula, whichUnit_ );
#endif // DIAGNOSTICS

    if ( !r )
        {
        validFormula_ = yes;

        // set the hour range for this formula
        r = getAndSetMaxHourRange ( estring );
        }

    free ( formula );
    return ( !r );
    }


int Formula::getAndSetMaxHourRange ( char *estring )
    {
    int     i;
    char    title[1028];
    char    minLabel[128];
    char    maxLabel[128];
    char    tcase[256];
    dataSet *dset;
    int     foundCase = 0, myinc, mysdate, mystime, mynsteps,
            inc, sdate, stime, nsteps;
    void    ( *valsModifiedParentCB ) ( void * );
    int *sdates = NULL, *stimes = NULL, *saveTimes = NULL, *saveDates = NULL;

    if ( validFormula_ != yes )
        {
        sprintf ( estring, "Can't Formula::getAndSetMaxHourRange() since"
                  " validFormula_ != yes" );
        return 1;
        }

    if ( updateCaseNamesAndTimes ( estring ) )
        return 1;

    /*
    if all datasets used by this formula have the same sdate/stime/secs increment
    for that dataset's selected time step range, then we can use Julian dates/times
    for this formula's selected time step range.  Otherwise we'll be limited
    to using just time step numbers
    */

    for ( i = 0; i < ncases_; i++ )                 // loop over all the cases
        if ( caseUsed_[i] == '1' )                  // are we using this case ?
            {
            if ( getNthItem ( i+1, caseList_, tcase ) )
                // there *should* have been a saved case name
                // here but for some reason there isn't
                {
                invalidateThisFormula();
                sprintf ( estring,
                          "Bad caseList_ in Formula::getAndSetMaxHourRange() !" );
                return 1;
                }
            else
                {
                if ( dset = ( dataSet * ) datasetList_->find ( tcase ) )
                    {
                    if ( dset->getIncrSdateStimeNsteps ( &inc, &sdate,
                                                         &stime, &nsteps,
                                                         &sdates, &stimes ) )
                        return 1;
                    if ( !foundCase )
                        {
                        foundCase = 1;
                        myinc = inc;
                        mysdate = sdate;
                        mystime = stime;
                        mynsteps = nsteps;
                        saveTimes = stimes;
                        saveDates = sdates;
                        }
                    else
                        {
                        if ( nsteps < mynsteps ) mynsteps = nsteps;
                        if ( saveTimes && saveDates )
                            // we have full julian info for each step
                            {
                            int size = sizeof ( int ) *mynsteps;
                            if ( ( memcmp ( saveTimes, stimes, size ) ) ||
                                    ( memcmp ( saveDates, sdates, size ) ) )
                                {
                                myinc = mysdate = mystime = 0;
                                saveTimes = saveDates = NULL;
                                }
                            }
                        else if ( ( myinc != inc ) || ( mysdate != sdate ) ||
                                  ( mystime != stime ) )
                            myinc = mysdate = mystime = 0;
                        }
                    }
                else
                    {
                    invalidateThisFormula();
                    sprintf ( estring, "Couldn't find dataset %s in "
                              "Formula::getAndSetMaxHourRange() !",
                              tcase );
                    return 1;
                    }
                }
            }

    sdates = stimes = NULL;

    if ( !foundCase )
        {
        invalidateThisFormula();
        sprintf ( estring, "This formula doesn't use any cases "
                  "in Formula::getAndSetMaxHourRange() !" );
        return 1;
        }

    if ( saveTimes && saveDates ) // we can use Julian dates/times for this
        // formula's selected time step range, and
        // they are already all figured out for us
        {
        sdates = ( int * ) malloc ( sizeof ( int ) *mynsteps );
        stimes = ( int * ) malloc ( sizeof ( int ) *mynsteps );
        if ( ( !sdates ) || ( !stimes ) )
            {
            if ( sdates ) free ( sdates );
            sdates = NULL;
            if ( stimes ) free ( stimes );
            stimes = NULL;
            }
        else
            {
            memcpy ( sdates, saveDates, sizeof ( int ) *mynsteps );
            memcpy ( stimes, saveTimes, sizeof ( int ) *mynsteps );
            }
        }
    else if ( myinc && mysdate ) // we can use Julian dates/times for this
        // formula's selected time step range, but
        // we need to figure them all out
        {
        sdates = ( int * ) malloc ( sizeof ( int ) *mynsteps );
        stimes = ( int * ) malloc ( sizeof ( int ) *mynsteps );
        if ( ( !sdates ) || ( !stimes ) )
            {
            if ( sdates ) free ( sdates );
            sdates = NULL;
            if ( stimes ) free ( stimes );
            stimes = NULL;
            }
        else
            {
            int hrinc, mininc, secinc;

            // myinc is now number of seconds
            hrinc = myinc/3600;
            mininc = ( myinc- ( hrinc*3600 ) ) /60;
            secinc = ( myinc- ( hrinc*3600 )- ( mininc*60 ) );
            myinc =  hrinc*10000+mininc*100+secinc;
            // myinc is now in IO/API format not seconds

            // figure out all the sdates/stimes
            sdates[0] = mysdate;
            stimes[0] = mystime;
            for ( i = 1; i < mynsteps; i++ )
                {
                nextimec ( &mysdate , &mystime , myinc );
                sdates[i] = mysdate;
                stimes[i] = mystime;
                }

            }
        }

    maxNumHours_ = mynsteps;
    hrMin_ = 0;
    hrMax_ = mynsteps-1;

    // SRT 960424 if (stepDialogBox_) { delete stepDialogBox_; stepDialogBox_ = NULL; }

    sprintf ( title, "%s Times of Interest", infixFormula_ );
    sprintf ( minLabel, "Min Time" );
    sprintf ( maxLabel, "Max Time" );
    valsModifiedParentCB = ( void ( * ) ( void * ) ) modifiedTheseStepsCB;

    estring[0] = '\0';
    if ( !stepDialogBox_ )
        stepDialogBox_ = new StepUI (
            title,                          // title of dialog box
            parent_,                        // to base position on
            minLabel,                       // "Step Min", "Layer Min", etc
            maxLabel,                       // "Step Max", "Layer Max", etc
            0,                              // 0, 1, etc
            ( maxNumHours_==1 ) ?1:maxNumHours_-1,    // 24, 72, etc
            sdates,       /* Julian start date for time steps 960412 SRT */
            stimes,       /* Julian start time for time steps 960412 SRT */
            &hrMin_,                        // current Min
            &hrMax_,                        // current Max
            valsModifiedParentCB,           // call this when changed
            ( void * ) this,                // this object
            estring                         // to hold error msgs
        );
    else
        {
        int oldDateMin, oldTimeMin, oldDateMax, oldTimeMax;

        stepDialogBox_->getCurrentJulianTimeCutoffs
        ( &oldDateMin, &oldTimeMin, &oldDateMax, &oldTimeMax );

        stepDialogBox_->updateValues (
            minLabel,                       // "Step Min", "Layer Min", etc
            maxLabel,                       // "Step Max", "Layer Max", etc
            0,                              // 0, 1, etc
            ( maxNumHours_==1 ) ?1:maxNumHours_-1,    // 24, 72, etc
            sdates,       /* Julian start date for time steps 960412 SRT */
            stimes,       /* Julian start time for time steps 960412 SRT */
            &hrMin_,                        // current Min
            &hrMax_,                        // current Max
            estring                         // to hold error msgs
        );

        // now let's see how close we can come to using the originally set hours
#if 0
        stepDialogBox_->tryToSetJulianTimeCutoffs
        ( oldDateMin, oldTimeMin, oldDateMax, oldTimeMax );
#endif
        }

    if ( sdates ) free ( sdates );
    sdates = NULL;
    if ( stimes ) free ( stimes );
    stimes = NULL;

    if ( ( !stepDialogBox_ ) || ( estring[0] != '\0' ) )
        {
        delete stepDialogBox_;
        stepDialogBox_ = NULL;
        return 1;
        }

    return 0;
    }



int Formula::isFormulaDataValid ( void ) // is there valid data for this
// formula already here ?
    {
    return ( validFormulaData_ == yes );
    }


static void modifiedTheseStepsCB ( void *obj ) // run this if user subselects
// on the formula's range
    {
    Formula *f = ( Formula * ) obj;

#ifdef DIAGNOSTICS
    fprintf ( stderr, "\nEnter Formula.cc's modifiedTheseStepsCB() for formula '%s':\n", f->getFormulaName() );
#endif // DIAGNOSTICS

    if ( f ) f->invalidateThisData();
    }



int Formula::dataSetListWasChanged      // to notify of changed dataSet list
( int *removeStatus,
  char *estring )       // error msgs will be written here
    {
    char tcase[512];
    dataSet *dset;
    int i;

    *removeStatus = 0;

#ifdef DIAGNOSTICS
    fprintf ( stderr,
              "0 '%s''s Formula::dataSetListWasChanged \n",
              getFormulaName() );
#endif // DIAGNOSTICS


    if ( validFormula_ == unsure_or_no )
        return 0;   // irrelevant if formula is invalid

    // now determine, do we need to invalidate the formula and data?
    //
    // if the datasets being used by the formula are still there in
    // the same order, then the formula and data are still valid
    // no problem
    //
    // if the datasets aren't there in the same order, then we
    // definitely need to invalidate the data, and we can't even
    // be certain of the formula's validity anymore unless we
    // recheck it


    if ( datasetList_ == NULL )
        dset = NULL;
    else
        {
        dset = ( dataSet * ) datasetList_->head();
        }

    for ( i = 0; i < ncases_; i++ )
        {
        if ( caseUsed_[i] == '1' ) // has this case remained there?
            {
            if ( dset == NULL ) // no, there is no case here anymore
                {
#ifdef DIAGNOSTICS
                fprintf ( stderr,
                          "1 Formula::dataSetListWasChanged INVALIDATING '%s' %I=%d\n",
                          getFormulaName(),i );
#endif // DIAGNOSTICS
                *removeStatus = 1;

                invalidateThisFormula();
                return 0;
                }

            if ( getNthItem ( i+1, caseList_, tcase ) )
                // there *should* have been a saved case name
                // here but for some reason there isn't
                {
#ifdef DIAGNOSTICS
                fprintf ( stderr,
                          "2 Formula::dataSetListWasChanged INVALIDATING '%s'\n",
                          getFormulaName() );
#endif // DIAGNOSTICS
                *removeStatus = 1;
                invalidateThisFormula();
                sprintf ( estring,
                          "Bad caseList_ in Formula::dataSetsWasChanged !" );
                return 1;
                }

            if ( strcmp ( dset->getFullName(), tcase ) )
                // a case used in this formula has changed
                {
#ifdef DIAGNOSTICS
                fprintf ( stderr,
                          "3 Formula::dataSetListWasChanged INVALIDATING '%s' I=%d, datasetname=%s\n",
                          getFormulaName(),i,dset->getFullName() );
#endif // DIAGNOSTICS
                *removeStatus = 1;
                invalidateThisFormula();
                return 0;
                }
            }

        dset = ( dataSet * ) datasetList_->next();
        }

#ifdef DIAGNOSTICS
    fprintf ( stderr, "4 Formula::dataSetListWasChanged still OK with '%s'\n",
              getFormulaName() );
#endif // DIAGNOSTICS
    // we made it through unscathed - we still have validity here !

    return 0;
    }



int Formula::levelsWereChanged          // to notify of a changed
( int nlayers )     // Levels object
    {
#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Formula::levelsWereChanged(%d) on formula \"%s\" with nlevels==%d)\n",
              nlayers, getFormulaName(), getFormulaNlevel() );
#endif // DIAGNOSTICS

    if ( validFormulaData_ == unsure_or_no )
        return 0;   // irrelevant if data is invalid

    if ( getFormulaNlevel() == nlayers )
        invalidateThisData();

    return 0;
    }



int Formula::domainWasChanged           // to notify of a changed
( int ni,       // Domain object
  int nj,
  char *mapinfo )
    {
    char tstring[128];

#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Formula::domainWasChanged(%d,%d,'%s')\n", ni, nj, mapinfo );
#endif // DIAGNOSTICS

    if ( validFormulaData_ == unsure_or_no )
        return 0;   // irrelevant if data is invalid

    if (    ( info_.ncol == ni ) &&
            ( info_.nrow == nj ) &&
            ( map_infos_areReasonablyEquivalent ( info_.map_info, mapinfo, tstring ) )
       )
        invalidateThisData();

    return 0;
    }


int Formula::getMapInfo ( char *s, char *estring ) // fills s with map_info string for this formula
    {
    int i;
    char tcase[256];
    dataSet *dset;

    // if the data is valid we can just snag
    // the map_info from the valid VIS_DATA struct
    if ( validFormulaData_ == yes )
        {
        strcpy ( s, info_.map_info );
        return 0;
        }

    // find the first dataset name used in this formula
    for ( i = 0; i < ncases_; i++ )         // loop over all the cases
        if ( caseUsed_[i] == '1' )          // are we using this case ?
            {
            if ( getNthItem ( i+1, caseList_, tcase ) )
                // there *should* have been a saved case name
                // here but for some reason there isn't
                {
                invalidateThisFormula();
                sprintf ( estring, "Bad caseList_ in Formula::getMapInfo() !" );
                return 1;
                }
            else
                {
                if ( dset = ( dataSet * ) datasetList_->find ( tcase ) )
                    sprintf ( s, dset->getMapInfo() );
                else
                    {
                    invalidateThisFormula();
                    sprintf ( estring, "Couldn't find dataset %s in Formula::getMapInfo() !", tcase );
                    return 1;
                    }
                }
            }

    return 0;
    }




int Formula::editFormulaSteps ( void )  // edit the step_min, step_max,
// and step for this formula
    {
    if ( stepDialogBox_ )
        stepDialogBox_->postOptionDialog();

#ifdef DIAGNOSTICS
    else
        fprintf ( stderr, "No stepDialogBox to edit with "
                  "in Formula::editFormulaSteps()!\n" ); // SRT
#endif // DIAGNOSTICS

    return 0;
    }



int Formula::dataSetStepsWereChanged    // to notify of changed step-min &
( char *fullname,       // step-max settings in a dataset
  int newmin,
  int newmax )
    {
    int i;
    char tcase[256];
    char estring[256];
    int usingThisCase = 0;
    int oldDateMin, oldTimeMin, oldDateMax, oldTimeMax;


#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter \"%s\"'s Formula::dataSetStepsWereChanged('%s')\n", infixFormula_, fullname );
#endif // DIAGNOSTICS

    // if this isn't a valid formula to begin with, then who cares ?
    if ( validFormula_ != yes )
        {
        fprintf ( stderr, "Formula::dataSetStepsWereChanged"
                  " can't continue as validFormula != yes\n" );
        return 1;
        }

    // if we're not using this case, then who cares ?
    for ( i = 0; i < ncases_; i++ )
        if ( caseUsed_[i] == '1' )
            {
            getNthItem ( i+1, caseList_, tcase );
            if ( !strcmp ( fullname, tcase ) )
                {
                caseNhours_[i] = newmax-newmin+1;
                usingThisCase = 1;
                }
            }
    if ( !usingThisCase ) return 0; // we're NOT using this case so get out

    if ( stepDialogBox_ )
        stepDialogBox_->getCurrentJulianTimeCutoffs
        ( &oldDateMin, &oldTimeMin, &oldDateMax, &oldTimeMax );

    // any data is now invalid
    invalidateThisData();

    // valid formula & using this case - so let's reset the max hour range
    if ( getAndSetMaxHourRange ( estring ) )
        {
        fprintf ( stderr, "Formula::dataSetStepsWereChanged"
                  " error calling getAndSetMaxHourRange: '%s'\n", estring );
        return 1;
        }

    // now let's see how close we can come to using the originally set hours
    if ( stepDialogBox_ )
        stepDialogBox_->tryToSetJulianTimeCutoffs
        ( oldDateMin, oldTimeMin, oldDateMax, oldTimeMax );

    return 0;
    }



void Formula::invalidateThisData ( void ) // sets formulaDataValidity
// to unsure_or_no and clears
// out any info struct
    {
    validFormulaData_ = unsure_or_no;
    free_vis ( &info_ );
    }



void Formula::invalidateThisFormula // sets formulaValidity
( void )        // to unsure_or_no, frees up
// some stuff, and invalidates this data
    {
    invalidateThisData();
    validFormula_ = unsure_or_no;
    if ( stepDialogBox_ )
        {
        delete stepDialogBox_;
        stepDialogBox_ = NULL;
        }
    ( ( FormulaServer * ) formulaS_ )->removeThisItem ( infixFormula_ );
#if !defined(linux)
    if ( infixFormula_ ) free ( infixFormula_ );
#endif
    infixFormula_ = NULL;
    if ( caseList_ )     free ( caseList_ );
    caseList_ = NULL;
    if ( caseOnlyList_ ) free ( caseOnlyList_ );
    caseOnlyList_ = NULL;
    if ( hostOnlyList_ ) free ( hostOnlyList_ );
    hostOnlyList_ = NULL;
    if ( whichLevel_ )  free ( whichLevel_ );
    whichLevel_ = NULL;
    }



// grab a copy of the time series data - returns NULL if error

float *Formula::get_time_series_data ( char *estring ) // grab a copy of time series data for this formula
    {
    Level *level = NULL;
    Domain *domain = NULL;
    char mapinfo[128];
    int *target[3];
    int thislevel;
    int failed = 0;
    float *thickValues = NULL;
    char *percents = NULL;
    int i;
    float *tsdata = NULL;
    int orig_hrMin = hrMin_, orig_hrMax = hrMax_;
    VIS_DATA temp_vdata;
    memset ( &temp_vdata, ( int ) NULL, sizeof ( VIS_DATA ) );
    int minlevel;


#ifdef DIAGNOSTICS
    fprintf ( stderr, "Enter Formula::get_time_series_data()\n" );
#endif // DIAGNOSTICS

    assert ( estring );
    estring[0] = '\0';

    // is this a valid formula?
    if ( !isFormulaValid ( estring ) )
        {
        return NULL;
        }

    if ( ! ( tsdata = ( float * ) malloc ( ( size_t ) ( sizeof ( float ) *get_nsteps() ) ) ) )
        {
        sprintf ( estring, "Formula::get_time_series_data() can't malloc tsdata!" );
        return NULL;
        }

    // what level are we interested in ?
    if ( ! ( level = ( Level * ) levelList_->find ( &KMAX_ ) ) )
        {
        sprintf ( estring, "Formula::get_time_series_data() can't find &d Level object!" );
        if ( tsdata ) free ( tsdata );
        return NULL;
        }
    minlevel = level->get_min_level();
    thislevel = level->get_max_level(); // bogus for now; needs to be extended somehow SRT
    if ( minlevel != thislevel )
        {
        strcpy ( estring,"Cannot handle multiple layers for now" );
        return NULL;
        }
    if ( whichLevel_ ) free ( whichLevel_ );
    if ( ! ( whichLevel_ = level->getCopyOfLevels() ) )
        {
        sprintf ( estring, "level->getCopyOfLevels() failure in Formula::get_time_series_data()!" );
        if ( tsdata ) free ( tsdata );
        return NULL;
        }


    // for now let's treat the relative thicknesses of all layers as the same
    if ( ! ( thickValues = ( float * ) malloc ( KMAX_ * sizeof ( float ) ) ) )
        {
        sprintf ( estring, "thickValues malloc() failure in Formula::get_time_series_data()!" );
        free ( whichLevel_ );
        whichLevel_ = NULL;
        if ( tsdata ) free ( tsdata );
        return NULL;
        }
    for ( i = 0; i < KMAX_; i++ )
        thickValues[i] = 1.0;


    // get a copy of the percents array representing this formula's domain
    if ( getMapInfo ( mapinfo, estring ) )
        {
        free ( whichLevel_ );
        whichLevel_ = NULL;
        free ( thickValues );
        thickValues = NULL;
        if ( tsdata ) free ( tsdata );
        return NULL;
        }
    target[0] = &IMAX_;
    target[1] = &JMAX_;
    target[2] = ( int* ) mapinfo;
    if ( ! ( domain = ( Domain * ) domainList_->find ( target ) ) )
        {
        sprintf ( estring, "Couldn't find matching domain in Formula::get_time_series_data()!" );
        free ( whichLevel_ );
        whichLevel_ = NULL;
        free ( thickValues );
        thickValues = NULL;
        if ( tsdata ) free ( tsdata );
        return NULL;
        }
    if ( ! ( percents = domain->getCopyOfPercents ( estring ) ) )
        {
        free ( whichLevel_ );
        whichLevel_ = NULL;
        free ( thickValues );
        thickValues = NULL;
        if ( tsdata ) free ( tsdata );
        return NULL;
        }


    // actually retrieve the data
    failed = retrieveData (

                 IMAX_,

                 JMAX_,

                 KMAX_,

                 infixFormula_,

                 postFixQueue_,

                 caseOnlyList_,

                 hostOnlyList_,

                 bd_,

                 1,        // SRT selectedStep_ - doesn't matter for XYTSLICE slices anyway

                 TIME_INT,     // SRT integrate over time

                 thickValues,

                 whichLevel_,

                 use_floor_,
                 // If use_floor is non-zero, then divisions will be checked
                 // to avoid divide by zero conditions.  If (use_floor) then for each
                 // division, if the denonimator is less than or equal to floorCut
                 // then the result of the divide is set to 0

                 ( float ) floorCut_, // floorCut (see use_floor description above)

                 percents,

                 1,    // selected_col_ - doesn't matter for XYTSLICE slices anyway

                 1,    // selected_row_ - doesn't matter for XYTSLICE slices anyway

                 thislevel,  // selected_level_ - 1 based (1 .. KMAX_)

                 caseStepMin_,

                 caseStepMax_,

                 caseStepIncr_,

                 XYTSLICE, // SRT need to allow other slice types later

                 &hrMin_,

                 &hrMax_,

                 tsdata, // tsdata_ - used only for time series and other integrations

                 &temp_vdata,

                 ( float * ) NULL, // float *tot_value - used only for stats

                 estring );

    free ( whichLevel_ );
    whichLevel_ = NULL;
    free ( thickValues );
    thickValues = NULL;
    free ( percents );
    percents = NULL;
    if ( ( orig_hrMin != hrMin_ ) || ( orig_hrMax != hrMax_ ) ) // becasue retrieveData()
        // will sometimes return
        // "partial" time series
        // data if it only gets some
        {
        hrMin_ = orig_hrMin;
        hrMax_ = orig_hrMax;
        free ( tsdata );
        tsdata = NULL;
        return NULL;
        }
    if ( failed )
        {
        free ( tsdata );
        tsdata = NULL;
        return NULL;
        }

    return tsdata;
    }




// grab an actual copy of the data
VIS_DATA *Formula::get_VIS_DATA_struct ( char *estring, int slice_type )
    {
    VIS_DATA *ans = ( VIS_DATA * ) NULL;
    long gridsize;
    assert ( estring );
    estring[0] = '\0';

    // is this a valid formula?
    if ( !isFormulaValid ( estring ) )
        {
        return NULL;
        }

    if ( info_.slice!=slice_type ) invalidateThisData(); // added SRT 951228


    // does this valid formula have invalid data?
    if ( !isFormulaDataValid() )
        {
        Level *level;
        Domain *domain;
        char mapinfo[128];
        int *target[3];
        int thislevel;
        int failed = 0;
        float *thickValues;
        char *percents;
        int i, imin, imax, jmin, jmax;
        int selected_col = 1, selected_row = 1;

        // free any existing VIS_DATA struct's tendrills of allocated memory
        free_vis ( &info_ );

        // what level are we interested in ?
        if ( ! ( level = ( Level * ) levelList_->find ( &KMAX_ ) ) )
            {
            sprintf ( estring, "Formula::get_VIS_DATA_struct() can't find a Level object!" );
            return NULL;
            }

        if ( slice_type == XYSLICE || slice_type == XYTSLICE )
            if ( level->get_max_level() != level->get_min_level() )
                {
                sprintf ( estring, "Formula::get_VIS_DATA_struct() doesn't support multiple layers yet!" );
                return NULL;
                }
        thislevel = level->get_max_level(); // bogus for now; needs to be extended somehow SRT
        if ( whichLevel_ ) free ( whichLevel_ );
        if ( ! ( whichLevel_ = level->getCopyOfLevels() ) )
            {
            sprintf ( estring, "level->getCopyOfLevels() failure in Formula::get_VIS_DATA_struct()!" );
            return NULL;
            }

        // for now let's treat the relative thicknesses of all layers as the same
        if ( ! ( thickValues = ( float * ) malloc ( KMAX_ * sizeof ( float ) ) ) )
            {
            sprintf ( estring, "thickValues malloc() failure in Formula::get_VIS_DATA_struct()!" );
            free ( whichLevel_ );
            whichLevel_ = NULL;
            return NULL;
            }
        for ( i = 0; i < KMAX_; i++ )
            thickValues[i] = 1.0;


        // get a copy of the percents array representing this formula's domain
        if ( getMapInfo ( mapinfo, estring ) )
            {
            free ( whichLevel_ );
            whichLevel_ = NULL;
            free ( thickValues );
            thickValues = NULL;
            return NULL;
            }
        target[0] = &IMAX_;
        target[1] = &JMAX_;
        target[2] = ( int* ) mapinfo;
        if ( ! ( domain = ( Domain * ) domainList_->find ( target ) ) )
            {
            sprintf ( estring, "Couldn't find matching domain in Formula::get_VIS_DATA_struct()!" );
            free ( whichLevel_ );
            whichLevel_ = NULL;
            free ( thickValues );
            thickValues = NULL;
            return NULL;
            }

        if ( ! ( percents = domain->getCopyOfPercents ( estring ) ) )
            {
            free ( whichLevel_ );
            whichLevel_ = NULL;
            free ( thickValues );
            thickValues = NULL;
            return NULL;
            }

        if ( slice_type != XYSLICE && slice_type != XYTSLICE )
            {
            int i, j;

            imin = IMAX_;
            jmin = JMAX_;
            imax = -1;
            jmax = -1;
            for ( j = 0; j < JMAX_; j++ )
                for ( i = 0; i < IMAX_; i++ )
                    if ( percents [INDEX ( i, j, 0, 0, IMAX_, JMAX_, 1 )] )
                        {
                        if ( i < imin ) imin = i;
                        if ( i > imax ) imax = i;
                        if ( j < jmin ) jmin = j;
                        if ( j > jmax ) jmax = j;
                        }

            if ( ( imin == IMAX_ ) || ( jmin == JMAX_ ) || ( imax == -1 ) || ( jmax == -1 ) )
                {
                sprintf ( estring, "Couldn't find any cells on in Formula::get_VIS_DATA_struct()!" );
                free ( whichLevel_ );
                whichLevel_ = NULL;
                free ( thickValues );
                thickValues = NULL;
                free ( percents );
                percents = NULL;
                return NULL;
                }

            if ( ( slice_type == XZSLICE ) || ( slice_type == XZTSLICE ) )
                {
                if ( jmin == jmax )
                    selected_row = jmin+1;
                else
                    {
                    sprintf ( estring,
                              "More than one row selected for XZSLICE slice in Formula::get_VIS_DATA_struct()!" );
                    free ( whichLevel_ );
                    whichLevel_ = NULL;
                    free ( thickValues );
                    thickValues = NULL;
                    free ( percents );
                    percents = NULL;
                    return NULL;
                    }
                }

            if ( ( slice_type == YZSLICE ) || ( slice_type == YZTSLICE ) )
                {
                if ( imin == imax )
                    selected_col = imin+1;
                else
                    {
                    sprintf ( estring,
                              "More than one col selected for XZSLICE slice in Formula::get_VIS_DATA_struct()!" );
                    free ( whichLevel_ );
                    whichLevel_ = NULL;
                    free ( thickValues );
                    thickValues = NULL;
                    free ( percents );
                    percents = NULL;
                    return NULL;
                    }
                }
            }

        // actually retrieve the data
        failed = retrieveData (

                     IMAX_,

                     JMAX_,

                     KMAX_,

                     infixFormula_,

                     postFixQueue_,

                     caseOnlyList_,

                     hostOnlyList_,

                     bd_,

                     1,        // SRT selectedStep_ - doesn't matter for XYTSLICE slices anyway

                     NO_INT,       // SRT need to allow other types of integration later

                     thickValues,

                     whichLevel_,

                     use_floor_,// If use_floor is non-zero, then divisions will be checked
                     // to avoid divide by zero conditions.  If (use_floor) then for each
                     // division, if the denonimator is less than or equal to floorCut
                     // then the result of the divide is set to 0

                     ( float ) floorCut_, // floorCut (see use_floor description above)

                     percents,

                     selected_col, // selected_col_ - 1 based

                     selected_row, // selected_row_ - 1 based

                     thislevel,  // selected_level_ - 1 based (1 .. KMAX_)

                     caseStepMin_,

                     caseStepMax_,

                     caseStepIncr_,

                     slice_type, // XYTSLICE SRT need to allow other slice types later

                     &hrMin_,

                     &hrMax_,

                     NULL, // tsdata_ - used only for time series and other integrations

                     &info_,

                     ( float * ) NULL, // float *tot_value - used only for stats

                     estring );

        free ( whichLevel_ );
        whichLevel_ = NULL;
        free ( thickValues );
        thickValues = NULL;
        free ( percents );
        percents = NULL;


        if ( failed )
            {
            return NULL;
            }
        validFormulaData_ = yes;
        }


    if ( userUnit_ )
        {
        if ( info_.units_name[0] ) free ( info_.units_name[0] );
        info_.units_name[0] = strdup ( userUnit_ );
        }

    // make a copy of this formula's valid data and send it back
    // return VIS_DATA_dup(&info_, estring);        /* 960920 SRT replaced for memory management */
    gridsize = info_.grid ? ( ( long ) ( info_.col_max-info_.col_min+1 ) ) * /*960923 SRT memory*/
               ( ( long ) ( info_.row_max-info_.row_min+1 ) ) * /*960923 SRT memory*/
               ( ( long ) ( info_.level_max-info_.level_min+1 ) ) * /*960923 SRT memory*/
               ( ( long ) ( info_.step_max-info_.step_min+1 ) ) * /*960923 SRT memory*/
               ( ( long ) ( sizeof ( float ) ) ) : 0;   /*960923 SRT memory*/

    if ( gridsize < 10000000 )                       /*960923 SRT memory*/
        ans = VIS_DATA_dup ( &info_, estring );     /* 960920 SRT memory */
    if ( !ans )                     /* 960920 SRT memory */
        {
        /* 960920 SRT memory */
        if ( ! ( ans= ( VIS_DATA * ) malloc ( sizeof ( VIS_DATA ) ) ) ) /* 960920 SRT memory */
            {
            /* 960920 SRT memory */
            invalidateThisData();             /* 960920 SRT memory */
            sprintf ( estring,            /* 960920 SRT memory */
                      "Not enough memory in Formula::get_VIS_DATA_struct()!!" ); /*960920 SRT memory*/
            return ans;                   /* 960920 SRT memory */
            }                     /* 960920 SRT memory */
        memcpy ( ans, &info_, sizeof ( VIS_DATA ) );    /* 960920 SRT memory */
        memset ( ( void * ) &info_,         /* 960920 SRT memory */
                 ( int ) NULL, ( size_t ) sizeof ( VIS_DATA ) ); /* 960920 SRT memory */
        validFormulaData_ = unsure_or_no;       /* 960920 SRT memory */
        }                       /* 960920 SRT memory */
    return ans;                     /* 960920 SRT memory */
    }


int Formula::set_selected_level ( int l ) // 1 based added 950909 SRT
    {
    Level *level = NULL;

    if ( ( l < 1 ) || ( l > KMAX_ ) || ( KMAX_==1 ) ) return 1;

    if ( ! ( level = ( Level * ) levelList_->find ( &KMAX_ ) ) )
        {
        fprintf ( stderr,
                  "Formula::set_selected_level() can't find Level object!" );
        return 1;
        }
    else
        level->setMinAndMaxLevelTo ( l );

    return 0;
    }


int Formula::set_levelRange ( int lmin, int lmax ) // 1 based added 951230 SRT
    {
    Level *level = NULL;

    if ( ( lmin < 1 ) || ( lmin > KMAX_ ) || ( KMAX_==1 ) ) return 1;
    if ( ( lmax < 1 ) || ( lmax > KMAX_ ) || ( KMAX_==1 ) ) return 1;

    if ( ! ( level = ( Level * ) levelList_->find ( &KMAX_ ) ) )
        {
        fprintf ( stderr,
                  "Formula::set_selected_level() can't find Level object!" );
        return 1;
        }
    else
        level->setLevelRange ( lmin, lmax );

    return 0;
    }





int Formula::set_selected_step ( int t ) // 1 based added 950909 SRT
    {
    if ( t < 1 ) return 1;
    selectedStep_ = t; // this isn't really used anywhere
    return 0;
    }


int Formula::set_hr_min ( int t )        // 0 based added 950909 SRT
    {
    if ( t < 0 ) return 1;
    if ( hrMin_ == t ) return 0;
    if ( t > hrMax_ ) t = hrMax_;
    stepDialogBox_->setMinMax ( t, hrMax_ );
    invalidateThisData(); // added SRT 960913
    hrMin_ = t; // added SRT 960913
    return 0;
    }


int Formula::set_hr_max ( int t )        // 0 based added 950909 SRT
    {
    if ( t < 0 ) return 1;
    if ( hrMax_ == t ) return 0;
    if ( t < hrMin_ ) t = hrMin_;
    stepDialogBox_->setMinMax ( hrMin_, t );
    invalidateThisData(); // added SRT 960913
    hrMax_ = t; // added SRT 960913
    return 0;
    }


char *Formula::getCasesUsedString ( void ) // added 950911 SRT
    {
    int i, len, ncasesAdded = 0;
    char tcase[512], *ans;

    // allocate as much space as caseList_ uses + 3 characters for each
    // possible "a=", "b=", etc) although this may well be longer
    // than necessary, since we're probably not using all the
    // datasets, and also we're stripping out the path name of
    // each dataset that we are using
    if ( !caseList_ )
        len = 1;
    else if ( !caseList_[0] )
        len = 1;
    else
        len = strlen ( caseList_ )+1+ ( ncases_-1 ) *3;
    if ( ! ( ans = ( char * ) malloc ( len ) ) )
        return NULL;
    ans[0] = '\0';
    if ( len == 1 ) return ans;

    // now fill up the ans string and return it
    for ( i = 0; i < ncases_; i++ )
        if ( caseUsed_[i] == '1' )
            {
            if ( ncasesAdded ) strcat ( ans, ", " );
            sprintf ( tcase, "%c=", 'a'+i );
            strcat ( ans, tcase );
            getNthItem ( i+1, caseList_, tcase );
            strcat ( ans, getPointerToBaseName ( tcase ) );
            ncasesAdded++;
            }
    return ans;
    }


void Formula::setUnits ( char *u )
    {
    if ( userUnit_ ) free ( userUnit_ );
    userUnit_ = NULL;
    userUnit_ = strdup ( u );
    if ( strlen ( userUnit_ ) >= sizeof ( whichUnit_ ) )
        {
        userUnit_[sizeof ( whichUnit_ )-1] = '\0';
        strcpy ( whichUnit_, userUnit_ );
        strcpy ( userUnit_, u );
        }
    else
        strcpy ( whichUnit_, userUnit_ );
    }



char *Formula::getTimeMinString ( void )
    {
    static char timeMinString[64];
    int datemin, datemax, timemin, timemax;

    if ( !stepDialogBox_ )
        sprintf ( timeMinString, "%d", get_hrMin() );
    else
        {
        stepDialogBox_->getCurrentJulianTimeCutoffs ( &datemin, &timemin, &datemax, &timemax );
        julian2shorttext ( timeMinString, datemin, timemin );
        }
    return timeMinString;
    }


char *Formula::getTimeMaxString ( void )
    {
    static char timeMaxString[64];
    int datemin, datemax, timemin, timemax;

    if ( !stepDialogBox_ )
        sprintf ( timeMaxString, "%d", get_hrMax() );
    else
        {
        stepDialogBox_->getCurrentJulianTimeCutoffs ( &datemin, &timemin, &datemax, &timemax );
        julian2shorttext ( timeMaxString, datemax, timemax );
        }
    return timeMaxString;
    }



char *Formula::getSelectedCellRange ( void )
    {
    static char selectedCellRange[64];
    int xmin, ymin, xmax, ymax, zmin, zmax;

    if ( get_cellrange ( &xmin, &xmax, &ymin, &ymax ) )
        {
        fprintf ( stderr,
                  "\007Coudn't find cell range in Formula::getSelectedCellRange()!\n" );
        return "???";
        }
    if ( get_levelrange ( &zmin, &zmax ) )
        {
        fprintf ( stderr,
                  "\007Coudn't find level range in Formula::getSelectedCellRange()!\n" );
        return "???";
        }
    if ( xmin==xmax &&  ymin==ymax && zmin==zmax )
        {
        sprintf ( selectedCellRange, "Cell (%d,%d,%d)", xmin, ymin, zmin );
        }
    else
        {
        sprintf ( selectedCellRange, "Cells (%d,%d,%d)->(%d,%d,%d)",
                  xmin, ymin, zmin, xmax, ymax, zmax );
        }
    return selectedCellRange;
    }



int Formula::get_levelrange ( int *zmin, int *zmax )
    {
    Level *level;

    if ( ! ( level = ( Level * ) levelList_->find ( &KMAX_ ) ) )
        {
        fprintf ( stderr, "\007Coudn't find level in Formula::getSelectedCellRange()!\n" );
        return 1;
        }
    *zmin = level->get_min_level();
    *zmax = level->get_max_level();
    return 0;
    }


int Formula::get_cellrange ( int *xmin, int *xmax, int *ymin, int *ymax )
    {
    char    mapinfo[192], estring[80];
    int    *target[3];
    Domain *domain;

    if ( getMapInfo ( mapinfo, estring ) )
        {
        fprintf ( stderr, "\007Formula::get_cellrange() %s\n", estring );
        return 1;
        }
    target[0] = &IMAX_;
    target[1] = &JMAX_;
    target[2] = ( int* ) mapinfo;
    if ( ! ( domain = ( Domain * ) domainList_->find ( target ) ) )
        {
        fprintf ( stderr, "\007Coudn't find domain in Formula::get_cellrange()!\n" );
        return 1;
        }
    if ( domain->getRange ( xmin, ymin, xmax, ymax ) )
        {
        fprintf ( stderr,
                  "\007Coudn't find cell range in Formula::get_cellrange()!\n" );
        return 1;
        }
    return 0;
    }



int Formula::get_selected_level ( void )
    {
    int zmin, zmax;

    if ( get_levelrange ( &zmin, &zmax ) )
        {
        fprintf ( stderr, "\007Couldn't get_levelrange() in Formula::get_selected_level()!\n" );
        return -1;
        }
    if ( zmin != zmax )
        {
        fprintf ( stderr, "\007zmin!=zmax in Formula::get_selected_level()!\n" );
        return -1;
        }
    return zmin;
    }


int Formula::get_selected_row ( void )
    {
    int xmin, xmax, ymin, ymax;

    if ( get_cellrange ( &xmin, &xmax, &ymin, &ymax ) )
        {
        fprintf ( stderr, "\007Couldn't get_cellrange() in Formula::get_selected_row()!\n" );
        return -1;
        }
    if ( ymin != ymax )
        {
        fprintf ( stderr, "\007ymin!=ymax in Formula::get_selected_row()!\n" );
        return -1;
        }
    return ymin;
    }


int Formula::get_selected_column ( void )
    {
    int xmin, xmax, ymin, ymax;

    if ( get_cellrange ( &xmin, &xmax, &ymin, &ymax ) )
        {
        fprintf ( stderr, "\007Couldn't get_cellrange() in Formula::get_selected_column()!\n" );
        return -1;
        }
    if ( xmin != xmax )
        {
        fprintf ( stderr, "\007xmin!=xmax in Formula::get_selected_column()!\n" );
        return -1;
        }
    return xmin;
    }

// ********************************************************
int Formula::isObs ( void )
    {
    dataSet *dset;
    int is_obs;
    int i;
    int j;
    char tcase[256];

    is_obs=0;
    for ( i = 0; i < ncases_; i++ )                  // loop over all the cases
        {
        if ( caseUsed_[i] == '1' )                  // are we using this case ?
            {
            if ( !getNthItem ( i+1, caseList_, tcase ) )
                {
                if ( dset = ( dataSet * ) datasetList_->find ( tcase ) )
                    {
                    j = dset->isObs();
                    is_obs = is_obs || j;
                    }
                }
            }
        }
    return is_obs;
    }

