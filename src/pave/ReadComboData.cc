/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: ReadComboData.cc 83 2018-03-12 19:24:33Z coats $
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
//  ReadComboData.C
//  K. Eng Pua
//  Copyright (C)
//  Jan 28, 1995
//
//  NOTE: :
//  NOTE: not finish yet.  need more work
//////////////////////////////////////////////////////////////////////

#include "ReadComboData.h"

ReadComboData::ReadComboData ( char *inpfile, char *drawtype ) : ComboData()
    {
    assert ( drawtype );
    drawtype_ = strdup ( drawtype );

    ComboData::initialize();

    readData ( inpfile );

    fclose ( fp_ );
    }

ReadComboData::~ReadComboData()
    {
    fclose ( fp_ );
    }


void ReadComboData::readData ( char *infilename )
    {
    char strbuff[MAX_BUFF_LEN];

    if ( ( fp_ = fopen ( infilename, "r" ) ) == NULL )
        {
        fprintf ( stderr, "Cannot open map info file %s\n", infilename );
        return;
        }

    fgets ( strbuff, MAX_BUFF_LEN, fp_ );
    if ( strncmp ( strbuff,"<VISINPUT>", 10 ) != 0 )
        {
        fprintf ( stderr, "Bad input format.\n" );
        return;
        }
    readHeaders();
    readControls();
    readOptions();
    countDataSets();
    readBar();
    }



void ReadComboData::readHeaders()
    {
    char idex[20];
    char strbuf[MAX_BUFF_LEN];

    fscanf ( fp_, "%s", idex );

    while ( strncmp ( idex, "<CONTROL>", 9 ) )
        {
        if ( !strncmp ( idex, "TITLE1", 6 ) )
            {
            fgets ( strbuf, MAX_BUFF_LEN, fp_ );
            title1_ = strdup ( strbuf );
            }
        else if ( !strncmp ( idex, "TITLE2", 6 ) )
            {
            fgets ( strbuf, MAX_BUFF_LEN, fp_ );
            title2_ = strdup ( strbuf );
            }
        else if ( !strncmp ( idex, "XTI", 3 ) )
            {
            fgets ( strbuf, MAX_BUFF_LEN, fp_ );
            xtitle_ = strdup ( strbuf );
            }
        else if ( !strncmp ( idex, "YTI", 3 ) )
            {
            fgets ( strbuf, MAX_BUFF_LEN, fp_ );
            ytitle_ = strdup ( strbuf );
            }
        fscanf ( fp_, "%s", idex );
        }
    }


void ReadComboData::readControls()
    {
    char idex[20];
    char strbuf[MAX_BUFF_LEN];

    fscanf ( fp_, "%s", idex );
    while ( strncmp ( idex, "<OPTION>", 8 ) )
        {
        if ( !strcmp ( idex, "PLOTTYPE" ) )
            {
            fgets ( strbuf, MAX_BUFF_LEN, fp_ );
            plottype_ = strdup ( strbuf );
            }
        else if ( !strcmp ( idex, "NUMCOL" ) )
            {
            fscanf ( fp_, "%d", &y_data_num_ );
            y_data_num_--;
            }

        fscanf ( fp_, "%s", idex );
        }

    }


void ReadComboData::readOptions()
    {
    char idex[20];
    char strbuff[MAX_BUFF_LEN];
    int i;

    fscanf ( fp_, "%s", idex );
    while ( !feof ( fp_ ) && strncmp ( idex, "<BEGINDAT", 9 ) )
        {
        if ( !strncmp ( idex, "FORMAT", 6 ) )
            fscanf ( fp_, "%s %s\n", plotformatX_, plotformatY_ );

        else if ( !strncmp ( idex, "OFFSET", 6 ) )
            fscanf ( fp_, "%d %d %d %d\n",
                     &offset_left_, &offset_right_, &offset_top_, &offset_bottom_ );


        else if ( !strncmp ( idex, "LABEL", 5 ) )
            {
            /* Skip the x label. */
            fscanf ( fp_, "%s ", strbuff );

            /* Allocate memory for namelist */
            if ( ( y_label_list_ = ( str20 * )
                                   malloc ( y_data_num_ * sizeof ( str20 ) ) ) == NULL )
                {
                fprintf ( stderr, "Can't allocate memory for y_label_list_.\n" );
                return;
                }
            for ( i=0; i < y_data_num_; i++ )
                {
                if ( i == y_data_num_-1 )
                    fscanf ( fp_, "%s\n", y_label_list_[i] );
                else
                    fscanf ( fp_, "%s", y_label_list_[i] );
                }

            }

        else if ( !strncmp ( idex, "STYLE", 5 ) )
            {
            /* Skip the X style label. */
            fscanf ( fp_, "%s ", strbuff );

            if ( ( plotstyle_ = ( int * ) malloc ( y_data_num_ * sizeof ( int ) ) ) == NULL )
                {
                fprintf ( stderr, "Can't allocate memory for plotstyle_.\n" );
                return;
                }

            for ( i = 0; i < y_data_num_; i++ )
                {
                if ( i == y_data_num_-1 )
                    fscanf ( fp_, "%s\n", strbuff );
                else
                    fscanf ( fp_, "%s", strbuff );

                if ( !strcmp ( strbuff, "POINT_ONLY" ) )
                    plotstyle_[i] = POINT_ONLY;
                else if ( !strcmp ( strbuff, "LINE_ONLY" ) )
                    plotstyle_[i] = LINE_ONLY;
                else if ( !strcmp ( strbuff, "POINT_LINE" ) )
                    plotstyle_[i] = POINT_LINE;
                }

            }

        fscanf ( fp_, "%s", idex );
        }
    fscanf ( fp_, "\n" );

    if ( feof ( fp_ ) )
        {
        fprintf ( stderr, "Missing keyword BEGINDATA.\n" );
        return;
        }

    }




void ReadComboData::countDataSets()
    {
    char strbuff[MAX_BUFF_LEN];

    // Count the number of observations (data sets).
    obs_num_ = 0;
    do
        {
        fgets ( strbuff, MAX_BUFF_LEN, fp_ );
        obs_num_++;
        }
    while ( strncmp ( strbuff,"<ENDDATA", 8 ) !=0 );
    obs_num_--;

    // Rewind and get ready to read in the data value
    rewind ( fp_ );
    do
        {
        fgets ( strbuff, MAX_BUFF_LEN, fp_ );
        }
    while ( strncmp ( strbuff,"<BEGIN", 6 ) !=0 );

    // Allocate space for val_y array
    val_y_ = new float[obs_num_ * y_data_num_];


    }


void ReadComboData::readBar()
    {
    // SRT 950717 int  true=1;
    int  i, j;
    int idy = 0;


    val_xs_ = new str20[obs_num_];

    val_ymax_ = -999.9;
    val_ymin_ = 999.9;

    for ( i = 0; i < obs_num_; i++ )
        {
        fscanf ( fp_, "%s ", val_xs_[i] );

        for ( j = 0; j < y_data_num_; j++ )
            {
            idy = i*y_data_num_ + j;
            fscanf ( fp_, "%f", &val_y_[idy] );
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


