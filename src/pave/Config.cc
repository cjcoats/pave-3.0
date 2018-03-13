/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: Config.cc 83 2018-03-12 19:24:33Z coats $
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
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// File:    Config.cc
// Author:  Steve Thorpe
// Date:    September 5, 1996
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//  Modification history:
//
//  960905 SRT Implemented
//
//////////////////////////////////////////////////////////////////////////////


#include "Config.h"

Config::Config()
    {
    initConfig();
    }


Config::~Config()
    {
    }


void Config::initConfig()
    {
    fname_[0] = '\0';
    hasColorMapType_ = 0;
    hasLegend_Max_ = 0;
    hasLegend_Min_ = 0;
    hasLegend_Format_ = 0;
    hasNumber_Labels_ = 0;
    hasInvert_Colormap_ = 0;
    hasNumber_Tiles_ = 0;
    hasColors_ = 0;
    hasSave_MPEG_Files_ = 0;
    hasDisable_Map_ = 0;
    hasSmooth_Plot_ = 0;
    hasDraw_Grid_Lines_ = 0;
    hasScale_Vectors_ = 0;
    }



// returns 1 if error
int Config::setFile ( char *fname, char *estring )
    {
    FILE    *fp = fopen ( fname, "r" );
    char    line[256], type[80], sval[64], *cp;
    int index;


    if ( !fp )
        {
        sprintf ( estring, "Couldn't open '%s' for reading!", fname );
        return 1;
        }

    initConfig();
    strcpy ( fname_, fname );

    while ( fgets ( line, sizeof ( line ), fp ) )
        {
        if ( strstr ( line, "ColorMapType" ) )
            {
            hasColorMapType_ = 1;
            sscanf ( line, "%s %s", type, sval );
            switch ( sval[0] )
                {
                case 'n':
                case 'N':
                    ColorMapType_ = 'N';
                    break;

                case 'j':
                case 'J':
                    ColorMapType_ = 'J';
                    break;

                case 'g':
                case 'G':
                    ColorMapType_ = 'G';
                    break;

                default:
                    hasColorMapType_ = 0;
                }
            }

        else if ( strstr ( line, "Legend_Max" ) )
            {
            sscanf ( line, "%s %f", type, &Legend_Max_ );
            hasLegend_Max_ = 1;
            }

        else if ( strstr ( line, "Legend_Min" ) )
            {
            sscanf ( line, "%s %f", type, &Legend_Min_ );
            hasLegend_Min_ = 1;
            }

        else if ( strstr ( line, "Legend_Format" ) )
            {
            sscanf ( line, "%s %s", type, Legend_Format_ );
            hasLegend_Format_ = 1;
            }

        else if ( strstr ( line, "Number_Labels" ) )
            {
            sscanf ( line, "%s %d", type, &Number_Labels_ );
            hasNumber_Labels_ = 1;
            }

        else if ( strstr ( line, "Invert_Colormap" ) )
            {
            sscanf ( line, "%s %d", type, &Invert_Colormap_ );
            hasInvert_Colormap_ = 1;
            }

        else if ( strstr ( line, "Number_Tiles" ) )
            {
            sscanf ( line, "%s %d", type, &Number_Tiles_ );
            hasNumber_Tiles_ = 1;
            }

        else if ( cp = strstr ( line, "ColorNumber" ) )
            {
            sscanf ( cp+11, "%d", &index );
            if ( ( index >= 1 ) && ( index <= 64 ) )
                {
                index--;
                sscanf ( line, "%d %d %d", &red_[index], &green_[index], &blue_[index] );
                }
            hasColors_ = 1;
            }

        else if ( strstr ( line, "Save_MPEG_Files" ) )
            {
            sscanf ( line, "%s %d", type, &Save_MPEG_Files_ );
            hasSave_MPEG_Files_ = 1;
            }

        else if ( strstr ( line, "Disable_Map" ) )
            {
            sscanf ( line, "%s %d", type, &Disable_Map_ );
            hasDisable_Map_ = 1;
            }

        else if ( strstr ( line, "Smooth_Plot" ) )
            {
            sscanf ( line, "%s %d", type, &Smooth_Plot_ );
            hasSmooth_Plot_ = 1;
            }

        else if ( strstr ( line, "Draw_Grid_Lines" ) )
            {
            sscanf ( line, "%s %d", type, &Draw_Grid_Lines_ );
            hasDraw_Grid_Lines_ = 1;
            }

        else if ( strstr ( line, "Scale_Vectors" ) )
            {
            sscanf ( line, "%s %d", type, &Scale_Vectors_ );
            hasScale_Vectors_ = 1;
            }
        }

    fclose ( fp );
    return 0;
    }


void Config::print ( FILE *fp )
    {
    int i;

    fprintf ( fp, "---------------\n" );
    fprintf ( fp, "Config::print()\n" );
    fprintf ( fp, "---------------\n" );
    fprintf ( fp, "   hasColorMapType_ == %d\n", hasColorMapType_ );
    fprintf ( fp, "     hasLegend_Max_ == %d\n", hasLegend_Max_ );
    fprintf ( fp, "     hasLegend_Min_ == %d\n", hasLegend_Min_ );
    fprintf ( fp, "  hasLegend_Format_ == %d\n", hasLegend_Format_ );
    fprintf ( fp, "  hasNumber_Labels_ == %d\n", hasNumber_Labels_ );
    fprintf ( fp, "hasInvert_Colormap_ == %d\n", hasInvert_Colormap_ );
    fprintf ( fp, "   hasNumber_Tiles_ == %d\n", hasNumber_Tiles_ );
    fprintf ( fp, "         hasColors_ == %d\n", hasColors_ );
    fprintf ( fp, "hasSave_MPEG_Files_ == %d\n", hasSave_MPEG_Files_ );
    fprintf ( fp, "    hasDisable_Map_ == %d\n", hasDisable_Map_ );
    fprintf ( fp, "    hasSmooth_Plot_ == %d\n", hasSmooth_Plot_ );
    fprintf ( fp, "hasDraw_Grid_Lines_ == %d\n", hasDraw_Grid_Lines_ );
    fprintf ( fp, "  hasScale_Vectors_ == %d\n", hasScale_Vectors_ );
    fprintf ( fp, "             fname_ == '%s'\n", fname_ );
    fprintf ( fp, "      ColorMapType_ == '%c'\n", ColorMapType_ );
    fprintf ( fp, "        Legend_Max_ == %f\n", Legend_Max_ );
    fprintf ( fp, "        Legend_Min_ == %f\n", Legend_Min_ );
    fprintf ( fp, "     Legend_Format_ == '%s'\n", Legend_Format_ );
    fprintf ( fp, "     Number_Labels_ == %d\n", Number_Labels_ );
    fprintf ( fp, "   Invert_Colormap_ == %d\n", Invert_Colormap_ );
    fprintf ( fp, "      Number_Tiles_ == %d\n", Number_Tiles_ );
    if ( hasColors_ )
        for ( i = Number_Tiles_; i >= 1; i-- )
            fprintf ( fp, "%d\t%d\t%d\tColorNumber%d\n", red_[i-1], green_[i-1], blue_[i-1], i );
    fprintf ( fp, "   Save_MPEG_Files_ == %d\n", Save_MPEG_Files_ );
    fprintf ( fp, "       Disable_Map_ == %d\n", Disable_Map_ );
    fprintf ( fp, "       Smooth_Plot_ == %d\n", Smooth_Plot_ );
    fprintf ( fp, "   Draw_Grid_Lines_ == %d\n", Draw_Grid_Lines_ );
    fprintf ( fp, "     Scale_Vectors_ == %d\n", Scale_Vectors_ );
    }
