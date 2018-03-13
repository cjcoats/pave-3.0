/******************************************************************************
 * PURPOSE: topo - Convert Arcinfo ASCII topography files to McIDAS topography
 *          format.
 * NOTES:   Usage:
 *            topo <infile> <outfile> <nrows> <ncols> <zval>
 *          Example:
 *            topo usa.out US.TOPO 596 1387 -9999
 *
 *            Reads usa.out and creates US.TOPO with a grid of 596 rows by
 *            1387 columns using -9999 as the 'no data value' (which is
 *            taken to be sealevel and stored as 0).
 *
 *          To compile:
 *
 *            cd /project/models3/unofficial/development/Models3Vis/code
 *            cd External/misc
 *
 *            SGI:
 *              cc  -xansi -fullwarn -g \
 *                  -I/usr/include -I/usr/include/sys -I/usr/include/rpc \
 *                  -I../../VisualizationSubsystem/include \
 *                  -L../../VisualizationSubsystem/lib/IRIX \
 *                  -o topo topo.c \
 *                  -lTopographyFile \
 *                  -lFile -lMemory -lError \
 *                  -lnsl -lmalloc
 *
 *          The Arcinfo files must be ASCII, with coordinates in lon-lat with
 *          western hemisphere longitudes and southern hemisphere latitudes
 *          negated. Here is an excerpt of a
 *          sample file:
 *
 *            -124.740,49.357,-9999
 *            -124.698,49.357,-9999
 *
 * HISTORY: 03/1996, Todd Plessel, EPA/MMTS, Created.
 *          02/2018, by Carlie J. Coats, Jr.:  Version for PAVE-2.4
 *****************************************************************************/

/*================================ INCLUDES =================================*/

#include <stdio.h>      /* For fgets().                                    */

#include "Assertions.h" /* For PRE(), POST(), CHECK().                     */
#include "Error.h"      /* For error().                                    */
#include "Memory.h"     /* For NEW(), FREE().                              */
#include "File.h"       /* For openFile(), closeFile(), read*(), write*(). */
#include "TopographyFile.h" /* For Topography, *Topography().              */

/*================================== TYPES ==================================*/

enum { X, Y, Z };
enum { LAT, LON };
enum { LOWER, UPPER };

/*========================== FORWARD DECLARATIONS ===========================*/

static int getArguments ( int argc, const char* argv[],
                          const char** inputFileName,
                          const char** outputFileName,
                          int* numberOfRows, int* numberOfColumns, int* zval );

static void usage ( const char* programName );

static int readArcTopoFile ( const char* fileName, int numberOfRows,
                             int numberOfColumns, int zval,
                             Topography* topography );

static void createTopography ( const float* arcData, int numberOfRows,
                               int numberOfColumns, Topography* topography );

/*============================ PUBLIC FUNCTIONS =============================*/


/******************************************************************************
 * PURPOSE: main - Calls routines to check arguments and process files.
 * INPUTS:  int   argc     The argument count.
 *          char* argv[]   The argument strings. See usage().
 * OUTPUTS: None
 * RETURNS: int 0 if successful, else 1 if failure
 * NOTES:
 *****************************************************************************/

static char topo_id[] = "$Id: topo.c 83 2018-03-12 19:24:33Z coats $" ;

int main ( int argc, const char* argv[] )
    {
    int numberOfRows           = 0;
    int numberOfColumns        = 0;
    int zval                   = 0;
    const char*  inputFileName = 0;
    const char* outputFileName = 0;

    int ok = getArguments ( argc, ( const char** ) argv,
                            &inputFileName, &outputFileName,
                            &numberOfRows, &numberOfColumns, &zval );

    if ( ok )
        {
        Topography topography;

        if ( allocateTopography ( numberOfRows, numberOfColumns,
                                  &topography ) )
            {
            ok = readArcTopoFile ( inputFileName, numberOfRows, numberOfColumns,
                                   zval, &topography );

            ok = NON_ZERO2 ( ok, writeTopographyFile ( outputFileName, &topography,
                             MCIDAS_TOPO_FORMAT ) );

            deallocateTopography ( &topography );
            }
        }

    return ! ok;
    }


/*=========================== PRIVATE FUNCTIONS =============================*/


/******************************************************************************
 * PURPOSE: getArguments - Read and check the command-line arguments.
 * INPUTS:  int argc                     The argument count.
 *          const char* argv[]           The argument strings.
 * OUTPUTS: const char** inputFileName   Name of the Arc topo file to read.
 *          const char** outputFileName  Name of the McIDAS topo file to write.
 *          int* numberOfRows            Number of row    lines in grid. [>1]
 *          int* numberOfColumns         Number of column lines in grid. [>1]
 *          int* zval                    Value used to denote 'no data'.
 *                                       (Such as sealevel.)
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

static int getArguments ( int argc, const char* argv[],
                          const char** inputFileName,
                          const char** outputFileName,
                          int* numberOfRows, int* numberOfColumns, int* zval )
    {
    int ok = argc == 6;

    PRE ( NON_ZERO7 ( argc > 0, argv, inputFileName, outputFileName,
                      numberOfRows, numberOfColumns, zval ) )

    /* Initialize outputs: */

    *inputFileName = *outputFileName  = 0;
    *numberOfRows  = *numberOfColumns = *zval = 0;

    if ( ! ok ) usage ( argv[ 0 ] );
    else
        {
        *inputFileName  = argv[ 1 ];
        *outputFileName = argv[ 2 ];

        *numberOfRows    = atoi ( argv[ 3 ] );
        *numberOfColumns = atoi ( argv[ 4 ] );
        *zval            = atoi ( argv[ 5 ] );

        ok = NON_ZERO2 ( *numberOfRows > 1, *numberOfColumns > 1 );

        if ( ! ok )
            {
            error ( "Number of rows (%d) and columns (%d) must be > 1.\n",
                    *numberOfRows, *numberOfColumns );
            }
        }

    if ( ! ok ) /* Re-initialize outputs: */
        {
        *inputFileName = *outputFileName  = 0;
        *numberOfRows  = *numberOfColumns = *zval = 0;
        }

    POST ( IMPLIES ( ok, NON_ZERO4 ( *inputFileName, *outputFileName,
                                     *numberOfRows > 1, *numberOfColumns > 1 ) ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: usage - Explain the proper usage for this program.
 * INPUTS:  const char* programName
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void usage ( const char* programName )
    {
    PRE ( programName )
    printf ( "\n\n%s\n\n", programName );
    printf ( "Converts an ARCinfo topography file to McIDAS TOPO format.\n" );
    printf ( "Usage: %s <infile> <outfile> <nrows> <ncols> <zval>\n", programName );
    printf ( "Example: " );
    printf ( "%s usa.out US.TOPO 596 1387 -9999\n\n", programName );
    printf ( "  Reads usa.out and creates US.TOPO with a grid of 596 rows by\n" );
    printf ( "  1387 columns using -9999 as the 'no data value' (which is\n" );
    printf ( "  taken to be sealevel and stored as 0).\n\n" );
    printf ( "returns 0 if successful, else 1 if failed.\n\n" );
    printf ( "\nVersion $Id: topo.c 83 2018-03-12 19:24:33Z coats $ \n\n" );
    }


/******************************************************************************
 * PURPOSE: readArcTopoFile - Read an Arcinfo ASCII topography file.
 * INPUTS:  const char* fileName         Name of the Arcinfo file to read.
 *          int         numberOfRows     Number of row    lines in grid. [>1]
 *          int         numberOfColumns  Number of column lines in grid. [>1]
 *          int         zval             Value used to denote 'no data'.
 *                                       (Such as sealevel.)
 * OUTPUTS: Topography* topography       The topography data.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

static int readArcTopoFile ( const char* fileName, int numberOfRows,
                             int numberOfColumns, int zval,
                             Topography* topography )
    {
    int ok = 0;
    File file;

    PRE ( NON_ZERO2 ( fileName, topography ) )

    ok = openFile ( fileName, "r", &file );

    if ( ok )
        {
        const int numberOfCoordinates = 3;
        const int size = numberOfRows * numberOfColumns * numberOfCoordinates;
        float* arcData = NEW ( float, size );

        ok = arcData != 0;

        if ( arcData )
            {
            float* data = arcData;

            for ( data = arcData; NON_ZERO2 ( ok, ! isEndOfFile ( &file ) );
                    data += numberOfCoordinates )
                {
                char line[ 80 ];
                int z = 0;

                ok = readString ( &file, line, 80 );

                ok = NON_ZERO2 ( ok, sscanf ( line, "%f,%f,%d\n", data + X, data + Y, &z )
                                 == numberOfCoordinates );

                ok = NON_ZERO3 ( ok, IN_RANGE ( data[ X ], -180.0, 180.0 ),
                                 IN_RANGE ( data[ Y ],  -90.0,  90.0 ) );

                if ( z != zval ) data[ Z ] = z;
                else             data[ Z ] = 0.0;

                if ( ! ok ) error ( "Bad line in '%s': '%s'\n", fileName, line );
                }

            if ( ok )
                createTopography ( arcData, numberOfRows, numberOfColumns, topography );

            FREE ( arcData );
            }

        ok = NON_ZERO2 ( closeFile ( &file ), ok );
        }

    POST ( IMPLIES ( ok, isValidTopography ( topography ) ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: createTopography - Create a Topography from arcinfo topography data.
 * INPUTS:  const float* arcData  The Arcinfo topography data.
 *                                arcData[ N ][ LON | LAT | ELEVATION ]
 *          double longitude      The longitude of the point to set.
 * OUTPUTS: double corners[2][2]  The corners initialized to the point.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void createTopography ( const float* arcData, int numberOfRows,
                               int numberOfColumns, Topography* topography )
    {
#ifndef OFFSET2
#define OFFSET2( j, i, IDIM ) ( (j) * (IDIM) + (i) )
#endif

    int row, column;
    const int numberOfRowsMinusOne = numberOfRows - 1;
    const float* vertex = arcData;
    float* elevations = topography ? topography->elevations : 0;
    double minLat, maxLat, minLon, maxLon;

    PRE ( NON_ZERO4 ( arcData, numberOfRows > 1, numberOfColumns > 1,
                      isValidTopography ( topography ) ) )

    topography->dimensions[ LAT ] = numberOfRows;
    topography->dimensions[ LON ] = numberOfColumns;

    /*
     * Compute the range of the latitude and longitude values.
     * Also, when copying the elevations, flip the rows since
     * the first row in Arcinfo is at the top while the first
     * row in Topography is at the bottom.
     */

    minLon = maxLon = vertex[ X ];
    minLat = maxLat = vertex[ Y ];

    for ( row = 0; row < numberOfRows; ++row )
        {
        for ( column = 0; column < numberOfColumns; ++column, vertex += 3 )
            {
            const double lon = vertex[ X ];
            const double lat = vertex[ Y ];

            const int offset = OFFSET2 ( numberOfRowsMinusOne - row, column,
                                         numberOfColumns );

            elevations[ offset ] = vertex[ Z ];

            if      ( lon < minLon ) minLon = lon;
            else if ( lon > maxLon ) maxLon = lon;

            if      ( lat < minLat ) minLat = lat;
            else if ( lat > maxLat ) maxLat = lat;
            }
        }

    topography->corners[ LOWER ][ LON ] = minLon;
    topography->corners[ UPPER ][ LON ] = maxLon;
    topography->corners[ LOWER ][ LAT ] = minLat;
    topography->corners[ UPPER ][ LAT ] = maxLat;

    POST ( isValidTopography ( topography ) )
    }

