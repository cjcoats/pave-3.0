/******************************************************************************
 * PURPOSE: arc2mcidas - Convert Arcinfo '.gen' files to McIDAS format.
 * NOTES:   Usage: arc2mcidas infile outfile
 *          Example: arc2mcidas us_streams.gen OUTLSTREAMS
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
 *                  -o arc2mcidas arc2mcidas.c \
 *                  -lMapFile -lFile -lMemory -lError \
 *                  -lnsl -lmalloc
 *            Sun:
 *              cc  -Xc -g
 *                  -I/usr/include -I/usr/include/sys -I/usr/include/rpc \
 *                  -I../../VisualizationSubsystem/include \
 *                  -L../../VisualizationSubsystem/lib/IRIX \
 *                  -o arc2mcidas arc2mcidas.c \
 *                  -lMapFile -lFile -lMemory -lError \
 *                  -lnsl -lmalloc
 *            DEC:
 *              cc  -ansi -g
 *                  -I/usr/include -I/usr/include/sys -I/usr/include/rpc \
 *                  -I../../VisualizationSubsystem/include \
 *                  -L../../VisualizationSubsystem/lib/IRIX \
 *                  -o arc2mcidas arc2mcidas.c \
 *                  -lMapFile -lFile -lMemory -lError \
 *                  -lrpcsvs -lmalloc
 *
 *          The Arcinfo files must be ASCII, with coordinates in lon-lat with
 *          western hemisphere longitudes negated. Here is an excerpt of a
 *          sample file:
 *
 *                2079
 *                  -85.684761         34.982353
 *                  -85.683899         34.990894
 *          END
 *                2078
 *                  -85.691048         34.946201
 *                  -85.688698         34.947845
 *                  -85.685623         34.953197
 *                  -85.684258         34.958153
 *                  -85.682869         34.963562
 *                  -85.683784         34.967682
 *                  -85.684746         34.970894
 *                  -85.684639         34.973164
 *                  -85.684547         34.974976
 *                  -85.684479         34.976337
 *                  -85.684433         34.977245
 *                  -85.684341         34.979057
 *                  -85.684814         34.980892
 *                  -85.684761         34.982353
 *          END
 *                2074
 *                  -85.691048         34.946201
 *
 * HISTORY: 03/96, Todd Plessel, EPA/MMTS, Created.
 *****************************************************************************/

/*================================ INCLUDES =================================*/

#include <stdio.h>      /* For fgets().                                    */
#include <string.h>     /* For strchr(), strstr().                         */
#include <ctype.h>    /* For isspace().                                  */

#include "Assertions.h" /* For PRE(), POST(), CHECK().                     */
#include "Error.h"      /* For error().                                    */
#include "Memory.h"     /* For NEW(), FREE().                              */
#include "File.h"       /* For openFile(), closeFile(), read*(), write*(). */
#include "MapFile.h"    /* For MapLines, *MapFile().                       */

/*================================= TYPES ===================================*/

enum { LOWER, UPPER };
enum { LAT,   LON   };

/* Segment directory components: */

enum { MIN_LAT, MAX_LAT, MIN_LON, MAX_LON, START, LENGTH,
       NUMBER_OF_SEGMENT_COMPONENTS
     };

/*========================== FORWARD DECLARATIONS ===========================*/

static void usage ( const char* programName );

static void printMapLinesInfo ( const MapLines* mapLines );

static void printHistogram ( const MapLines* mapLines );

static void normExtrema ( const MapLines* mapLines,
                          double* minimum, double* maximum );

static void normHistogram ( const MapLines* mapLines, int numberOfBuckets,
                            int counts[] );

static int sizeOfGenFile ( const char* fileName,
                           int* vertexCount,
                           int* polylineCount );

static int readGenFile ( const char* fileName, MapLines* mapLines );

static void initializeCorners ( double latitude, double longitude,
                                double corners[2][2] );

static void updateCorners ( double latitude, double longitude,
                            double corners[2][2] );

static int writeMcIDASFile ( const char* fileName, const MapLines* mapLines );

static int writeSegmentDirectory ( File* file, const MapLines* mapLines );

static int writeVertices ( File* file, const MapLines* mapLines );

static void initializeSegmentDirectory ( const MapLines* mapLines,
        int* segmentDirectory );

static void initializeSegmentMinMax ( const float* vertices,
                                      int numberOfVertices,
                                      int* segment );

static void scaleVertex ( double latitude, double longitude,
                          int* scaledLatitude, int* scaledLongitude );

static int isValidVertex ( double latitude, double longitude );

static int isValidScaledVertex ( int latitude, int longitude );

static int wordCount ( const char* s );

static void swap ( int* a, int* b )
    {
    int t = *a;
    *a = *b;
    *b = t;
    }

static double sqr ( float x )
    {
    return x * x;
    }

static double norm ( const float v1[2], const float v2[2] )
    {
    return sqr ( v1[0] - v2[0] ) + sqr ( v1[1] - v2[1] );
    }

static char arc2mcidas_id[] = "$Id: arc2mcidas.c 83 2018-03-12 19:24:33Z coats $" ;

/*============================ PUBLIC FUNCTIONS =============================*/


/******************************************************************************
 * PURPOSE: main - Calls routines to check arguments and process files.
 * INPUTS:  int   argc     The argument count.
 *          char* argv[]   The arguments: "us_streams.gen", "OUTLSTREAMS".
 * OUTPUTS: None
 * RETURNS: int 0 if successful, else 1 if failure
 * NOTES:
 *****************************************************************************/

int main ( int argc, const char* argv[] )
    {
    int ok = 0;

    if ( argc != 3 ) usage ( argv[0] );
    else
        {
        const char* const  inputFileName = argv[1];
        const char* const outputFileName = argv[2];

        int polylineCount; /* Total number of polylines in the file. */
        int vertexCount;   /* Total number of vertices in the file.  */

        if ( sizeOfGenFile ( inputFileName, &vertexCount, &polylineCount ) )
            {
            MapLines mapLines;

            if ( allocateMapLines ( polylineCount, vertexCount, &mapLines ) )
                {
                mapLines.vertexCount   = vertexCount;
                mapLines.polylineCount = polylineCount;

                ok = readGenFile ( inputFileName, &mapLines );
                ok = ok && writeMcIDASFile ( outputFileName, &mapLines );

                if ( ok )
                    {
                    printMapLinesInfo ( &mapLines );
                    printHistogram (    &mapLines );
                    }

                deallocateMapLines ( &mapLines );
                }
            }
        }

    return ! ok;
    }


/*=========================== PRIVATE FUNCTIONS =============================*/


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
    printf ( "Converts an ARCinfo '.gen' file to McIDAS format.\n" );
    printf ( "Usage: %s infile outfile\n", programName );
    printf ( "Examples:\n" );
    printf ( "  %s us_streams.gen OUTLSTREAMS\n", programName );
    printf ( "returns 0 if successful, else 1 if failed.\n\n" );
    printf ( "\nVersion $Id: arc2mcidas.c 83 2018-03-12 19:24:33Z coats $ \n\n" );
    }


/******************************************************************************
 * PURPOSE: printMapLinesInfo - Print some information on a MapLines.
 * INPUTS:  const MapLines* mapLines  The map lines to print.
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void printMapLinesInfo ( const MapLines* mapLines )
    {
    int vertex;

    PRE ( isValidMapLines ( mapLines ) )

    printf ( "Number of polylines = %d\n", mapLines->polylineCount );
    printf ( "Number of vertices  = %d\n", mapLines->vertexCount   );
    printf ( "Lat-Lon extents     = (%f, %f) to (%f, %f)\n",
             mapLines->corners[ LOWER ][ LAT ],
             mapLines->corners[ LOWER ][ LON ],
             mapLines->corners[ UPPER ][ LAT ],
             mapLines->corners[ UPPER ][ LON ] );

    DEBUG ( for ( vertex = 0; vertex < mapLines->vertexCount; ++vertex )
            printf ( "(%f, %f)\n",
                     mapLines->vertices[ 2 * vertex + LAT ],
                     mapLines->vertices[ 2 * vertex + LON ] ); )
        }


/******************************************************************************
 * PURPOSE: printHistogram - Print histogram of vertex norms of a MapLines.
 * INPUTS:  const MapLines* mapLines  The map lines to print.
 * OUTPUTS: None
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void printHistogram ( const MapLines* mapLines )
    {
#define NUMBER_OF_BUCKETS 10
    int histogram[ NUMBER_OF_BUCKETS ];
    int bucket;

    PRE ( isValidMapLines ( mapLines ) )

    normHistogram ( mapLines, NUMBER_OF_BUCKETS, histogram );

    for ( bucket = 0; bucket < NUMBER_OF_BUCKETS; ++bucket )
        printf ( "bucket %d count = %d\n", bucket + 1, histogram[ bucket ] );
    }


/******************************************************************************
 * PURPOSE: normHistogram - Compute the norm histogram of the vertices.
 * INPUTS:  const MapLines* mapLines  The map lines to read.
 *          int numberOfBuckets       The number of histogram buckets.
 * OUTPUTS: int counts[]              The counts.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void normHistogram ( const MapLines* mapLines, int numberOfBuckets,
                            int counts[] )
    {
    int polyline;
    int vertex;
    const float* v;
    double minimum;
    double maximum;
    double bucketSize;
    int bucket;

    PRE ( isValidMapLines ( mapLines ) && numberOfBuckets > 0 && counts )

    for ( bucket = 0; bucket < numberOfBuckets; ++bucket ) counts[ bucket ] = 0;

    normExtrema ( mapLines, &minimum, &maximum );

    printf ( "norm extrema: %f to %f\n", minimum, maximum );

    bucketSize = ( maximum - minimum ) / numberOfBuckets;

    v = mapLines->vertices;

    for ( polyline = 0; polyline < mapLines->polylineCount; ++polyline )
        {
        for ( vertex = 1, v += 2; vertex < mapLines->lengths[ polyline ];
                ++vertex, v += 2 )
            {
            const double theNorm = norm ( v - 2, v );
            int counted = 0;

            for ( bucket = 0; ! counted && bucket < numberOfBuckets; ++bucket )
                {
                if ( theNorm <= minimum + ( bucket + 1 ) * bucketSize )
                    {
                    ++counts[ bucket ];
                    counted = 1;
                    }
                }
            }
        }
    }


/******************************************************************************
 * PURPOSE: normExtrema - Compute the minimum and maximum norm of adjacent
 *          vertices.
 * INPUTS:  const MapLines* mapLines  The map lines to read.
 * OUTPUTS: double* minimum            The minimum norm between vertices.
 *          double* maximum            The maximum norm between vertices.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void normExtrema ( const MapLines* mapLines,
                          double* minimum, double* maximum )
    {
    int polyline;
    int vertex;
    const float* v;

    PRE ( isValidMapLines ( mapLines ) && NON_ZERO2 ( minimum, maximum ) )

    v = mapLines->vertices;

    *minimum = *maximum = norm ( v, v + 2 );

    for ( polyline = 0; polyline < mapLines->polylineCount; ++polyline )
        {
        DEBUG ( printf ( " polyline = %d\n", polyline ); )

        for ( vertex = 1, v += 2; vertex < mapLines->lengths[ polyline ];
                ++vertex, v += 2 )
            {
            const double theNorm = norm ( v - 2, v );

            DEBUG ( printf ( "    v - 2 = (%f, %f)\n", * ( v - 2 ), * ( v - 1 ) ); )
            DEBUG ( printf ( "    v     = (%f, %f)\n", *v, * ( v + 1 ) ); )

            if      ( theNorm < *minimum ) *minimum = theNorm;
            else if ( theNorm > *maximum ) *maximum = theNorm;

            DEBUG ( printf ( "cur min max = %f %f %f\n", theNorm, *minimum, *maximum ); )
            }
        }

    POST ( *minimum <= *maximum )
    }


/******************************************************************************
 * PURPOSE: sizeOfGenFile - Scan an Arcinfo '.gen' file to obtain the required
 *          size information prior to reading.
 * INPUTS:  const char* fileName     The name of the file to read.
 * OUTPUTS: int*  vertexCount        The total number of vertices in the file.
 *          int*  polylineCount      The total number of polylines in the file.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

static int sizeOfGenFile ( const char* fileName,
                           int* vertexCount, int* polylineCount )
    {
    int ok = 0;
    File file;

    PRE ( NON_ZERO3 ( fileName, vertexCount, polylineCount ) )

    *vertexCount = *polylineCount = 0;

    ok = openFile ( fileName, "r", &file );

    if ( ok )
        {
        char line[81];
        int currentVertexCount = 0;

        while ( ok && ! isEndOfFile ( &file ) )
            {
            ok = readString ( &file, line, 80 );

            if ( ok )
                {
                if ( strchr ( line, '.' ) && wordCount ( line ) == 2 )
                    {
                    ++currentVertexCount;
                    }
                else if ( strstr ( line, "END" ) &&  currentVertexCount > 0 )
                    {
                    ++*polylineCount;
                    *vertexCount += currentVertexCount;

                    if ( currentVertexCount == 1 )
                        fprintf ( stderr, "\n\007Warning: polyline #%d is degenerate.\n",
                                  *polylineCount );

                    currentVertexCount = 0;
                    }
                }
            }

        ok = ok && GT_ZERO2 ( *vertexCount, *polylineCount );

        ok = closeFile ( &file ) && ok;
        }

    if ( ! ok ) *vertexCount = *polylineCount = 0;

    POST ( IMPLIES ( ok, GT_ZERO2 ( *vertexCount, *polylineCount ) ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: readGenFile - Read an Arcinfo '.gen' file into a MapLines.
 * INPUTS:  const char* fileName               The name of the file to read.
 *          MapLines*   mapLines  The MapLines struct with:
 *                                  vertices      allocated.
 *                                  starts        allocated.
 *                                  lengths       allocated.
 *                                  vertexCount   initialized.
 *                                  polylineCount initialized.
 * OUTPUTS: MapLines*   mapLines  The MapLines struct with:
 *                                  vertices      initialized.
 *                                  starts        initialized.
 *                                  lengths       initialized.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   Upon input, mapLines is assumed to have been already allocated
 *          with allocateMapLines().
 *****************************************************************************/

static int readGenFile ( const char* fileName, MapLines* mapLines )
    {
    int ok = 0;
    int polylineCount  = 0;
    int vertexCount    = 0;
    File file;

    PRE ( NON_ZERO2 ( fileName, mapLines ) )

    ok = openFile ( fileName, "r", &file );

    if ( ok )
        {
        char line[81];
        int polylineLength = 0;
        float* vertex      = mapLines->vertices;

        mapLines->starts[0] = 0;

        while ( ok && ! isEndOfFile ( &file ) )
            {
            ok = readString ( &file, line, 80 );

            if ( ok )
                {
                if ( strchr ( line, '.' ) && wordCount ( line ) == 2 )
                    {
                    float latitude, longitude;

                    ok = sscanf ( line, "%f %f", &longitude, &latitude ) == 2 &&
                         isValidVertex ( latitude, longitude );

                    if ( ok )
                        {
                        vertex[ LAT ] = latitude;
                        vertex[ LON ] = longitude;
                        vertex += 2;

                        if ( vertexCount == 0 )
                            initializeCorners ( latitude, longitude, mapLines->corners );
                        else  updateCorners ( latitude, longitude, mapLines->corners );

                        ++vertexCount;
                        ++polylineLength;
                        }
                    }
                else if ( strstr ( line, "END" ) && polylineLength > 0 )
                    {
                    if ( polylineLength == 1 )
                        fprintf ( stderr, "\n\007Warning: polyline #%d is degenerate.\n",
                                  polylineCount );

                    mapLines->lengths[ polylineCount ] = polylineLength;

                    if ( polylineCount < mapLines->polylineCount - 1 )
                        {
                        mapLines->starts[ polylineCount + 1 ] =
                            mapLines->starts[ polylineCount     ] + polylineLength;
                        }

                    ++polylineCount;
                    polylineLength = 0;
                    }
                }
            }

        ok = closeFile ( &file ) && ok;
        }

    ok = ok && vertexCount   == mapLines->vertexCount &&
         polylineCount == mapLines->polylineCount;

    POST ( IMPLIES ( ok, isValidMapLines ( mapLines ) ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: initializeCorners - Initializes a corners with a lat-lon point.
 * INPUTS:  double latitude       The latitude  of the point to set.
 *          double longitude      The longitude of the point to set.
 * OUTPUTS: double corners[2][2]  The corners initialized to the point.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void initializeCorners ( double latitude, double longitude,
                                double corners[2][2] )
    {
    corners[ LOWER ][ LAT ] = corners[ UPPER ][ LAT ] = latitude;
    corners[ LOWER ][ LON ] = corners[ UPPER ][ LON ] = longitude;
    }


/******************************************************************************
 * PURPOSE: updateCorners - Updates a corners to encompass a lat-lon point.
 * INPUTS:  double latitude       The latitude  of the point to compare.
 *          double longitude      The longitude of the point to compare.
 * *        double corners[2][2]  The current extent.
 * OUTPUTS: double corners[2][2]  The expanded extent encompassing the point.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void updateCorners ( double latitude, double longitude,
                            double corners[2][2] )
    {
    if      ( latitude < corners[ LOWER ][ LAT ] )
        corners[ LOWER ][ LAT ] = latitude;
    else if ( latitude > corners[ UPPER ][ LAT ] )
        corners[ UPPER ][ LAT ] = latitude;

    if      ( longitude < corners[ LOWER ][ LON ] )
        corners[ LOWER ][ LON ] = longitude;
    else if ( longitude > corners[ UPPER ][ LON ] )
        corners[ UPPER ][ LON ] = longitude;
    }


/******************************************************************************
 * PURPOSE: writeMcIDASFile - Write a MapLines to a McIDAS file.
 * INPUTS:  const char*     fileName  The name of the file to write.
 *          const MapLines* mapLines  The allocated & initialized MapLines.
 * OUTPUTS: None
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

static int writeMcIDASFile ( const char* fileName, const MapLines* mapLines )
    {
    int ok = 0;
    File file;

    PRE ( fileName && isValidMapLines ( mapLines ) )

    if ( openFile ( fileName, "w", &file ) )
        {
        ok = writeInt ( &file, mapLines->polylineCount ); /* Write # of segments. */

        ok = ok && writeSegmentDirectory ( &file, mapLines );

        ok = ok && writeVertices ( &file, mapLines );

        ok = closeFile ( &file ) && ok;
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeSegmentDirectory - Write a segment directory to a McIDAS file.
 * INPUTS:  File* file                The opened file to write to.
 *          const MapLines* mapLines  The MapLines to get segment info from.
 * OUTPUTS: None
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

static int writeSegmentDirectory ( File* file, const MapLines* mapLines )
    {
    int ok = 0;
    int numberOfSegments;

    PRE ( isValidFile ( file ) && isValidMapLines ( mapLines ) )

    numberOfSegments = mapLines->polylineCount;

    if ( numberOfSegments )
        {
        /* Allocate and read/process the segment directory. */

        const int numberOfSegmentValues = numberOfSegments *
                                          NUMBER_OF_SEGMENT_COMPONENTS;

        int* segmentDirectory = NEW ( int, numberOfSegmentValues );

        if ( segmentDirectory )
            {
            initializeSegmentDirectory ( mapLines, segmentDirectory );

            ok = writeInts ( file, segmentDirectory, numberOfSegmentValues );

            FREE ( segmentDirectory );
            }
        }

    PRE ( isValidFile ( file ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: initializeSegmentDirectory - Initialize a segment directory from a
 *          MapLines.
 * INPUTS:  const MapLines* mapLines  The MapLines to read from.
 * OUTPUTS: int* segmentDirectory     The initialized segment directory.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void initializeSegmentDirectory ( const MapLines* mapLines,
        int* segmentDirectory )
    {
    int segment, offset, numberOfSegments, sizeOfSegmentDirectory;
    int* length;
    int* start;
    float* vertex;

    PRE ( isValidMapLines ( mapLines ) && segmentDirectory )

    /*
     * The offset is the file offset in words of the first 'start' value.
     * This equals 1 word for 'number of segments' plus the size of the
     * segment directory.
     */

    numberOfSegments          = mapLines->polylineCount;
    sizeOfSegmentDirectory    = numberOfSegments * NUMBER_OF_SEGMENT_COMPONENTS;
    offset                    = 1 + sizeOfSegmentDirectory;
    start                     = mapLines->starts;
    length                    = mapLines->lengths;
    vertex                    = mapLines->vertices;

    for ( segment = 0; segment < numberOfSegments; ++segment, ++start, ++length )
        {
        int* currentSegment = segmentDirectory +
                              segment * NUMBER_OF_SEGMENT_COMPONENTS;

        const int numberOfCoordinates = 2; /* LAT and LON */

        currentSegment[ LENGTH ] = *length * numberOfCoordinates;
        currentSegment[ START  ] = *start  * numberOfCoordinates + offset;

        initializeSegmentMinMax ( vertex, *length, currentSegment );

        vertex += 2 * *length;
        }
    }


/******************************************************************************
 * PURPOSE: initializeSegmentMinMax - Initialize a segment directory with the
 *          minimum and maximum vertices of a polyline.
 * INPUTS:  const float* vertices     The vertices of a polyline.
 *          int numberOfVertices      The number of vertices in the polyline.
 * OUTPUTS: int* segment              The segment to initialize.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   McIDAS longitudes are negated. I.e., western longitudes are +.
 *****************************************************************************/

static void initializeSegmentMinMax ( const float* vertices,
                                      int numberOfVertices,
                                      int* segment )
    {
    int vertex;
    double corners[2][2];

    PRE ( NON_ZERO2 ( vertices, segment ) && numberOfVertices > 0 )

    initializeCorners ( vertices[ LAT ], vertices[ LON ], corners );

    for ( vertex = 1; vertex < numberOfVertices; ++vertex )
        updateCorners ( vertices[ 2 * vertex + LAT ],
                        vertices[ 2 * vertex + LON ],
                        corners );

    DEBUG ( printf ( "corners: (%f, %f) to (%f, %f)\n",
                     corners[ LOWER ][ LAT ], corners[ LOWER ][ LON ],
                     corners[ UPPER ][ LAT ], corners[ UPPER ][ LON ] ); )

    scaleVertex ( corners[ LOWER ][ LAT ], corners[ LOWER ][ LON ],
                  &segment[ MIN_LAT ], &segment[ MIN_LON ] );

    scaleVertex ( corners[ UPPER ][ LAT ], corners[ UPPER ][ LON ],
                  &segment[ MAX_LAT ], &segment[ MAX_LON ] );

    swap ( &segment[ MIN_LON ], &segment[ MAX_LON ] ); /* McIDAS reverses LON. */

    DEBUG ( printf ( "segment: (%d, %d) to (%d, %d)\n",
                     segment[ MIN_LAT ], segment[ MIN_LON ],
                     segment[ MAX_LAT ], segment[ MAX_LON ] ); )

    POST ( isValidScaledVertex ( segment[ MIN_LAT ], segment[ MIN_LON ] ) &&
           isValidScaledVertex ( segment[ MAX_LAT ], segment[ MAX_LON ] ) &&
           segment[ MIN_LAT ] <= segment[ MAX_LAT ] &&
           segment[ MIN_LON ] <= segment[ MAX_LON ] )
    }


/******************************************************************************
 * PURPOSE: scaleVertex - Scale a vertex with coordinates in decimal degrees
 *          latitude-logitude to McIDAS 'scaled integer' format.
 * INPUTS:  double latitude      The latitude  of the vertex in decimal degrees
 *          double longitude     The longitude of the vertex in decimal degrees
 * OUTPUTS: int* scaledLatitude  The scaled latitude  of the vertex.
 *          int* scaledLongitude The scaled longitude of the vertex.
 * RETURNS: None
 * NOTES:   McIDAS longitudes are negated. I.e., western longitudes are +.
 *****************************************************************************/

static void scaleVertex ( double latitude, double longitude,
                          int* scaledLatitude, int* scaledLongitude )
    {
    const double scaleFactor = 10000.0;

    PRE ( NON_ZERO2 ( scaledLatitude, scaledLongitude ) &&
          isValidVertex ( latitude, longitude ) )

    *scaledLatitude  = ( int ) ( latitude  *  scaleFactor );
    *scaledLongitude = ( int ) ( longitude * -scaleFactor );

    POST ( isValidScaledVertex ( *scaledLatitude, *scaledLongitude ) )
    }


/******************************************************************************
 * PURPOSE: isValidVertex - Verify that a vertex is in the correct range.
 * INPUTS:  double latitude   The latitude  to check.
 *          double longitude  The longitude to check.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   If invalid, error() is called.
 *****************************************************************************/

static int isValidVertex ( double latitude, double longitude )
    {
    int ok = 1;

    if ( ! ( IN_RANGE ( latitude,  -90.0,   90.0 ) &&
             IN_RANGE ( longitude, -180.0, 180.0 ) ) )
        {
        error ( "Invalid latitude-longitude vertex: (%f, %f)\n",
                latitude, longitude );
        ok = 0;
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: isValidScaledVertex - Verify that a scaled vertex is in the correct
 *          range.
 * INPUTS:  int latitude   The scaled latitude  to check.
 *          int longitude  The scaled longitude to check.
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:   If invalid, error() is called.
 *****************************************************************************/

static int isValidScaledVertex ( int latitude, int longitude )
    {
    int ok = 1;

    if ( ! ( IN_RANGE ( latitude,   -900000,  900000 ) &&
             IN_RANGE ( longitude, -1800000, 1800000 ) ) )
        {
        error ( "Invalid scaled latitude-longitude vertex: (%d, %d)\n",
                latitude, longitude );
        ok = 0;
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeVertices - Write vertices to a McIDAS file.
 * INPUTS:  File* file                The opened file to write to.
 *          const MapLines* mapLines  The MapLines to extract vertices from.
 * OUTPUTS: None
 * RETURNS: int 1 if successful, else 0.
 * NOTES:
 *****************************************************************************/

static int writeVertices ( File* file, const MapLines* mapLines )
    {
    int ok = 1;
    int vertexCount;
    int numberOfVertices;
    float* vertex;

    PRE ( isValidFile ( file ) && isValidMapLines ( mapLines ) )

    vertex = mapLines->vertices;
    numberOfVertices = mapLines->vertexCount;

    for ( vertexCount = 0; ok && vertexCount < numberOfVertices; ++vertexCount )
        {
        int scaledLatitude, scaledLongitude;

        scaleVertex ( vertex[ LAT ], vertex[ LON ],
                      &scaledLatitude, &scaledLongitude );

        ok = writeInt ( file, scaledLatitude  ) &&
             writeInt ( file, scaledLongitude );

        vertex += 2;
        }

    PRE ( isValidFile ( file ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: wordCount - Compute the number of words in a string.
 * INPUTS:  const char* s  The string to scan.
 * OUTPUTS: None
 * RETURNS: int number of words.
 * NOTES:   A word is a whitespace-delimited sequence of characters.
 *****************************************************************************/

static int wordCount ( const char* s )
    {
    int count = 0;

    PRE ( s )

    while ( *s )
        {
        while ( isspace ( *s ) ) ++s;

        if ( *s ) ++count;

        while ( *s && ! isspace ( *s ) ) ++s;
        }

    POST ( count >= 0 )

    return count;
    }

