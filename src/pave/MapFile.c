/****************************************************************************
 *
 *  Package for Analysis and Visualization of Environmental data
 *  PAVE Version 3.0
 *
 *  File: $Id: MapFile.c 83 2018-03-12 19:24:33Z coats $
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
 *****************************************************************************
 * PURPOSE: MapFile.c - Defines functions for drawing projected map
 *          border lines and 2d grid lines.
 * NOTES:
 * HISTORY: 06/1995, Todd Plessel, EPA/MMTSI, Created.
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include <stdio.h>          /* For printf().                          */
#include <string.h>         /* For memcpy(), memset().                */

#include "Assertions.h"     /* For macros PRE(), POST().              */
#include "Error.h"          /* For error().                           */
#include "Memory.h"         /* For macros NEW(), FREE().              */
#include "File.h"           /* File, open/closeFile(), read/write*(). */
#include "MapFile.h"        /* For public functions.                  */

/*=============================== MACROS ===================================*/

#define SIZE_OF_IEEE_INT   4

/* Segment directory components: */

enum { MIN_LAT, MAX_LAT, MIN_LON, MAX_LON, START, LENGTH,
       NUMBER_OF_SEGMENT_COMPONENTS
     };

enum { LOWER, UPPER };
enum { LAT, LON };

static const char SVN_ID[] = "$Id: MapFile.c 83 2018-03-12 19:24:33Z coats $";

/*================================ TYPES ====================================*/

typedef int ( *MapExportFunction ) ( File*, const MapLines*, const float* );

/*========================= FORWARD DECLARATIONS ============================*/

static int isAllocatedOrZeroed ( const MapLines* mapLines );

static int hasValidStartsAndLengths ( const MapLines* mapLines );

static void zeroMapLines ( MapLines* mapLines );

static int writeARCGraphMap ( File* file, const MapLines* mapLines,
                              const float* z );

static int writeAVSMap (      File* file, const MapLines* mapLines,
                              const float* z );

static int writeDXMap (       File* file, const MapLines* mapLines,
                              const float* z );

static int writeAVSMapDimensionsAndCells ( File* file, const MapLines* mapLines );
static int writeAVSMapTopology (           File* file, const MapLines* mapLines );
static int writeAVSMapCoordinates (        File* file, const MapLines* mapLines,
        const float* z );

static int readNumberOfSegments ( File* file );

static int readAndConvertSegmentDirectory ( File* file,
        int numberOfSegments,
        int* segmentDirectory,
        MapLines* mapLines );

static int readAndConvertVertices ( File* file,
                                    MapLines* mapLines );

static int readSegmentDirectory ( File* file, int numberOfSegments,
                                  int* segmentDirectory );

static int readVertices ( File* file, int numberOfVertices, int vertices[] );

static int computePolylineAndVertexCounts ( const char* mapFileName,
        int numberOfSegments,
        const int segmentDirectory[],
        int* numberOfPOlylines,
        int* numberOfVertices );

static void convertStartsAndLengths ( int numberOfSegments,
                                      const int segmentDirectory[],
                                      int starts[], int lengths[] );

static void convertVertices ( int numberOfVertices, const int intVertices[],
                              float vertices[], double corners[2][2] );

static void updateMinmax ( double value, double* minimum, double* maximum );

static int equal ( double a, double b )
    {
    const double tolerance = 1e-5;

    return a < b ? b - a < tolerance : a - b < tolerance;
    }

/*=========================== PUBLIC FUNCTIONS ==============================*/


/******************************************************************************
 * PURPOSE: allocateMapLines - Allocate a MapLines structure (using malloc()).
 * INPUTS:  int numberOfPolylines
 *          int numberOfVertices
 * OUTPUTS: MapLines* mapLines
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

int allocateMapLines ( int numberOfPolylines, int numberOfVertices,
                       MapLines* mapLines )
    {
    PRE2 ( GT_ZERO2 ( numberOfPolylines, numberOfVertices ), mapLines );

    int ok = 0;

    zeroMapLines ( mapLines );

    mapLines->vertices = NEW ( float, 2 * numberOfVertices );

    if ( mapLines->vertices )
        {
        mapLines->starts = NEW ( int, numberOfPolylines );

        if ( mapLines->starts )
            {
            mapLines->lengths = NEW ( int, numberOfPolylines );

            if ( mapLines->lengths ) ok = 1;
            }
        }

    if ( ! ok ) deallocateMapLines ( mapLines );

    POST4 ( isValidMapLines ( mapLines ),
            IS_ZERO2 ( mapLines->vertexCount, mapLines->polylineCount ),
            IMPLIES ( ok,
                      NON_ZERO3 ( mapLines->vertices, mapLines->starts,
                                  mapLines->lengths  ) ),
            IMPLIES ( ! ok,
                      IS_ZERO3 ( mapLines->vertices, mapLines->starts,
                                 mapLines->lengths ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: deallocateMapLines - Free a MapLines structure (using free()).
 * INPUTS:  MapLines* mapLines
 * OUTPUTS: MapLines* mapLines
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void deallocateMapLines ( MapLines* mapLines )
    {
    PRE ( isValidMapLines ( mapLines ) );

    FREE ( mapLines->starts   );
    FREE ( mapLines->lengths  );
    FREE ( mapLines->vertices );

    zeroMapLines ( mapLines );

    POST2 ( isValidMapLines ( mapLines ),
            IS_ZERO5 ( mapLines->starts, mapLines->lengths, mapLines->vertices,
                       mapLines->polylineCount, mapLines->vertexCount ) );
    }


/******************************************************************************
 * PURPOSE: sizeOfMapFile - Determine the number of polylines and vertices in
 *          a map file.
 * INPUTS:  const char* mapFileName  The name of the file to check.
 * OUTPUTS: int* numberOfPolylines   The number of polylines in the file.
 *          int* numberOfVertices    The number of vertices  in the file.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int sizeOfMapFile ( const char* mapFileName,
                    int* numberOfPolylines, int* numberOfVertices )
    {
    PRE3 ( mapFileName, numberOfVertices, numberOfPolylines );

    int ok = 0;
    File* file = 0;

    *numberOfVertices = *numberOfPolylines = 0;

    file = openFile ( mapFileName, "r" );

    if ( file )
        {
        int numberOfSegments = readNumberOfSegments ( file );

        if ( numberOfSegments )
            {
            /* Allocate and read/process the segment directory. */

            int* segmentDirectory = NEW ( int, numberOfSegments *
                                          NUMBER_OF_SEGMENT_COMPONENTS );

            if ( segmentDirectory )
                {
                if ( readSegmentDirectory ( file, numberOfSegments, segmentDirectory ) )
                    {
                    ok = computePolylineAndVertexCounts ( mapFileName,
                                                          numberOfSegments,
                                                          segmentDirectory,
                                                          numberOfPolylines,
                                                          numberOfVertices  );
                    }
                FREE ( segmentDirectory );
                }
            }
        closeFile ( file );
        }

    POST2 ( IMPLIES (   ok, NON_ZERO2 ( *numberOfPolylines, *numberOfVertices ) ),
            IMPLIES ( ! ok , IS_ZERO2 ( *numberOfPolylines, *numberOfVertices ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readMapFile - Read the line data from the map file.
 * INPUTS:  const char* mapFileName  The name of the file to read.
 * OUTPUTS: MapLines*   mapLines     The map line data read.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *          Requires that mapLines (and its fields) have been allocated and
 *          initialized as follows:
 *          Pre-condition:
 *            mapLines->vertices      != 0 (allocated but uninitialized)
 *            mapLines->starts        != 0 (allocated but uninitialized)
 *            mapLines->lengths       != 0 (allocated but uninitialized)
 *          Post-condition:
 *            mapLines->vertexCount   set to actual file contents.
 *            mapLines->polylineCount set to actual file contents.
 *            mapLines->vertices      initialized with contents from file.
 *            mapLines->starts        initialized with contents from file.
 *            mapLines->lengths       initialized with contents from file.
 *****************************************************************************/

int readMapFile ( const char* mapFileName, MapLines* mapLines )
    {
    PRE2 ( mapFileName, isValidMapLines ( mapLines ) );

    int ok = 0;
    File* file = openFile ( mapFileName, "r" );

    if ( file )
        {
        int numberOfSegments = readNumberOfSegments ( file );

        if ( numberOfSegments )
            {
            /* Allocate and read/process the segment directory. */

            int* segmentDirectory = NEW ( int, numberOfSegments *
                                          NUMBER_OF_SEGMENT_COMPONENTS );

            if ( segmentDirectory )
                {
                /* Read segment directory and convert it to starts[] and lengths[]: */

                ok = readAndConvertSegmentDirectory ( file,
                                                      numberOfSegments,
                                                      segmentDirectory, mapLines );
                FREE ( segmentDirectory );
                }

            /* Read scaled int vertices and convert to float lat-lons in degrees: */

            if ( ok ) ok = readAndConvertVertices ( file, mapLines );

            } /* end if ( numberOfSegments ) */

        closeFile ( file );

        } /* end if ( openFile() ) */

    if ( ! ok ) mapLines->vertexCount = mapLines->polylineCount = 0;

    POST3 ( isValidMapLines ( mapLines ),
            IMPLIES ( ok,
                      NON_ZERO2 ( mapLines->vertexCount, mapLines->polylineCount ) ),
            IMPLIES ( ! ok,
                      IS_ZERO2 ( mapLines->vertexCount, mapLines->polylineCount ) ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeMapFile - Write the line data to the map file.
 * INPUTS:  const char*     mapFileName  Name of file to write or "-stdout".
 *          const MapLines* mapLines     The map line data to output.
 *          const float*    z            Optional z-coordinates for the map
 *                                       vertices.
 *          int             format       ARCGRAPH_FORMAT, AVS_GEOM_FORMAT,...
 * OUTPUTS: None
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeMapFile ( const char* mapFileName, const MapLines* mapLines,
                   const float* z, int format )
    {
    PRE4 ( mapFileName, mapLines, isValidMapLines ( mapLines ),
           IN4 ( format, ARCGRAPH_FORMAT, AVS_GEOM_FORMAT, DX_FORMAT ) );

    static MapExportFunction exportFunctions[ NUMBER_OF_MAP_FORMATS ] =
        {
        writeARCGraphMap,
        writeAVSMap,
        writeDXMap
        };

    MapExportFunction exportFunction = 0; /* Selected export function. */

    int ok = 0;

    if ( mapLines->polylineCount == 0 )
        error ( "Map has no lines! Not writing empty map file '%s'.", mapFileName );
    else
        {
        File* file = 0;
        exportFunction = exportFunctions[ format ]; /* Pick export function. */

        CHECK ( exportFunctions[ NUMBER_OF_MAP_FORMATS - 1 ] != 0 ) /* Trap change.*/;

        file = openFile ( mapFileName, "w" );

        if ( file )
            {
            ok = exportFunction ( file, mapLines, z );

            ok = AND2 ( ok, closeFile ( file ) );
            }

        if ( ! ok ) error ( "Failed to write map file '%s'.", mapFileName );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: isValidMapLines - Verify the validity of a MapLines struct.
 * INPUTS:  const MapLines* mapLines The struct to verify.
 * OUTPUTS: None
 * RETURNS: 1 if the struct is ok, else 0.
 * NOTES:
 *****************************************************************************/

int isValidMapLines ( const MapLines* mapLines )
    {
    int ok = isAllocatedOrZeroed ( mapLines );

    if ( AND2 ( ok, mapLines->vertexCount ) )
        {
        ok = hasValidStartsAndLengths ( mapLines );

        if ( ok )
            {
            /* Check the corners: */

            /*      MapLines mapLinesCopy = *mapLines; */

            MapLines mapLinesCopy;

            mapLinesCopy.vertices      = mapLines->vertices;
            mapLinesCopy.starts        = mapLines->starts;
            mapLinesCopy.lengths       = mapLines->lengths;
            mapLinesCopy.vertexCount   = mapLines->vertexCount;
            mapLinesCopy.polylineCount = mapLines->polylineCount;

            computeMapLinesCorners ( &mapLinesCopy );

            ok = AND4 ( equal ( mapLinesCopy.corners[ LOWER ][ LAT ],
                                mapLines->   corners[ LOWER ][ LAT ] ),
                        equal ( mapLinesCopy.corners[ LOWER ][ LON ],
                                mapLines->   corners[ LOWER ][ LON ] ),
                        equal ( mapLinesCopy.corners[ UPPER ][ LAT ],
                                mapLines->   corners[ UPPER ][ LAT ] ),
                        equal ( mapLinesCopy.corners[ UPPER ][ LON ],
                                mapLines->   corners[ UPPER ][ LON ] ) );
            }
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: computeMapLinesCorners - Computes the corners (bounding box) of a
 *          MapLines's vertices.
 * INPUTS:  MapLines* mapLines  MapLines structure to scan.
 * OUTPUTS: MapLines* mapLines  MapLines structure with initialized corners.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void computeMapLinesCorners ( MapLines* mapLines )
    {
    PRE ( isAllocatedOrZeroed ( mapLines ) );

    memset ( mapLines->corners, 0, sizeof mapLines->corners );

    if ( mapLines->vertexCount )
        {
        const float* vertex = mapLines->vertices;
        const float* end    = mapLines->vertices + 2 * mapLines->vertexCount;

        mapLines->corners[ LOWER ][ LAT ] =
            mapLines->corners[ UPPER ][ LAT ] = vertex[ LAT ];
        mapLines->corners[ LOWER ][ LON ] =
            mapLines->corners[ UPPER ][ LON ] = vertex[ LON ];

        for ( vertex += 2; vertex != end; vertex += 2 )
            {
            updateMinmax ( vertex[ LAT ], &mapLines->corners[ LOWER ][ LAT ],
                           &mapLines->corners[ UPPER ][ LAT ] );

            updateMinmax ( vertex[ LON ], &mapLines->corners[ LOWER ][ LON ],
                           &mapLines->corners[ UPPER ][ LON ] );
            }
        }
    }


/*=========================== PRIVATE FUNCTIONS =============================*/


/******************************************************************************
 * PURPOSE: isAllocatedOrZeroed - Verify that a MapLines struct is either
 *          zeroed-out or fully allocated.
 * INPUTS:  const MapLines* mapLines The struct to verify.
 * OUTPUTS: None
 * RETURNS: 1 if the struct is ok, else 0.
 * NOTES:
 *****************************************************************************/

static int isAllocatedOrZeroed ( const MapLines* mapLines )
    {
    return AND2 ( mapLines,
                  OR2 ( IS_ZERO2 ( mapLines->polylineCount, mapLines->vertexCount ),
                        AND2 ( GT_ZERO2 ( mapLines->polylineCount,
                                          mapLines->vertexCount ),
                               NON_ZERO3 ( mapLines->vertices, mapLines->starts,
                                           mapLines->lengths ) ) ) );
    }


/******************************************************************************
 * PURPOSE: hasValidStartsAndLengths - Verify that a MapLines struct has
 *          valid starts and lengths.
 * INPUTS:  const MapLines* mapLines The struct to verify.
 * OUTPUTS: None
 * RETURNS: 1 if the struct is ok, else 0.
 * NOTES:
 *****************************************************************************/

static int hasValidStartsAndLengths ( const MapLines* mapLines )
    {
    PRE2 ( isAllocatedOrZeroed ( mapLines ), mapLines->vertexCount );

    int ok = 0;
    int polyline;
    int vertexCountSum = AND2 ( mapLines, mapLines->lengths ) ?
                         mapLines->lengths [0 ] : 0;

    ok = AND2 ( mapLines->starts[ 0 ] == 0,
                IN_RANGE ( mapLines->lengths[ 0 ], 1, mapLines->vertexCount ) );

    for ( polyline = 1; AND2 ( ok, polyline < mapLines->polylineCount );
            ++polyline )
        {
        ok = AND3 ( mapLines->starts[  polyline ] > 1,
                    mapLines->lengths[ polyline ] > 0,
                    mapLines->starts[  polyline     ] ==
                    mapLines->starts[  polyline - 1 ] +
                    mapLines->lengths[ polyline - 1 ] );

        if ( ok )
            {
            vertexCountSum += mapLines->lengths[ polyline ];
            ok = vertexCountSum <= mapLines->vertexCount;
            }
        }

    ok = AND2 ( ok, vertexCountSum == mapLines->vertexCount );

    return ok;
    }


/******************************************************************************
 * PURPOSE: zeroMapLines - Zeros all fields of a map lines structure.
 * INPUTS:  None
 * OUTPUTS: MapLines* mapLines  Map lines structure zero'd out.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void zeroMapLines ( MapLines* mapLines )
    {
    memset ( mapLines, 0, sizeof ( MapLines ) );
    POST ( isValidMapLines ( mapLines ) );
    }


/******************************************************************************
 * PURPOSE: writeARCGraphMap - Write map lines to an ARCGraph file.
 * INPUTS:  File* file                The file to write to.
 *          const MapLines* mapLines  The map lines to output.
 *          const float* z            Optional z coordinates for map vertices.
 * OUTPUTS: File* file                The updated file structure.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:
 *****************************************************************************/

static int writeARCGraphMap ( File* file, const MapLines* mapLines,
                              const float* z )
    {
    PRE2 ( file, isValidMapLines ( mapLines ) );

    enum { SET_VECTOR_COLOR_RGB_3D = 106, MOVE_ABS_3D = 102, DRAW_ABS_3D = 104,
           END_FRAME = 99
         };
    enum { X, Y, Z };

    int ok, polyline;
    static const float rgb[ 3 ] = { 1.0, 1.0, 1.0 };

    ok = writeInt ( file, SET_VECTOR_COLOR_RGB_3D );

    ok = AND2 ( ok, writeFloats ( file, rgb, 3 ) );

    for ( polyline = 0; AND2 ( ok, polyline < mapLines->polylineCount );
            ++polyline )
        {
        int start            = mapLines->starts[  polyline ];
        int numberOfVertices = mapLines->lengths[ polyline ];
        const float* vertex  = mapLines->vertices + start * 2;
        float point[ 3 ];

        point[ X ] = vertex[ X ];
        point[ Y ] = vertex[ Y ];
        point[ Z ] = z ? *z++ : 0.0;

        ok = writeInt ( file, MOVE_ABS_3D );

        ok = AND2 ( ok, writeFloats ( file, point, 3 ) );

        for ( vertex += 2, --numberOfVertices; AND2 ( ok, numberOfVertices-- );
                vertex += 2 )
            {
            point[ X ] = vertex[ X ];
            point[ Y ] = vertex[ Y ];
            point[ Z ] = z ? *z++ : 0.0;

            ok = writeInt ( file, DRAW_ABS_3D );
            ok = AND2 ( ok, writeFloats ( file, point, 3 ) );
            }
        }

    ok = AND2 ( ok, writeInt ( file, END_FRAME ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeAVSMap - Write map lines to an AVS ucd file.
 * INPUTS:  File* file               The file to write to.
 *          const MapLines* mapLines The map lines to output.
 *          const float* z            Optional z coordinates for map vertices.
 * OUTPUTS: File* file               The updated file structure.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:   Until I obtain information on the format of AVS '.geom' files,
 *          This routine will output binary UCD files instead.
 *****************************************************************************/

static int writeAVSMap ( File* file, const MapLines* mapLines,
                         const float* z )
    {
    PRE2 ( file, isValidMapLines ( mapLines ) );

    return AND4 ( writeByte ( file, 7 ), /* Magic # 7 (AVS Dev Guide p. E-13). */
                  writeAVSMapDimensionsAndCells ( file, mapLines ),
                  writeAVSMapTopology (           file, mapLines ),
                  writeAVSMapCoordinates (        file, mapLines, z ) );
    }


/******************************************************************************
 * PURPOSE: writeAVSMapDimensionsAndCells - Write dimensions and cell info to
 *          an AVS ucd file.
 * INPUTS:  File* file               The file to write to.
 *          const MapLines* mapLines The map lines to output.
 * OUTPUTS: File* file               The updated file structure.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:
 *****************************************************************************/

static int writeAVSMapDimensionsAndCells ( File* file, const MapLines* mapLines )
    {
    PRE2 ( file, isValidMapLines ( mapLines ) );

    enum { NUMBER_OF_NODES, NUMBER_OF_CELLS,
           NUMBER_OF_NODE_DATA, NUMBER_OF_CELL_DATA, NUMBER_OF_MODEL_DATA,
           NUMBER_OF_NLIST_NODES, NUMBER_OF_DIMENSIONS
         };

    enum { CELL_ID, MATERIAL_ID, NUMBER_OF_NODES_IN_CELL, CELL_TYPE,
           NUMBER_OF_CELL_INFO_COMPONENTS
         };

    enum { UCD_LINE = 1 };

    int ok = 0, polyline, dimensions[ NUMBER_OF_DIMENSIONS ];
    int cellInfo[ NUMBER_OF_CELL_INFO_COMPONENTS ];

    cellInfo[ CELL_ID ]                 = 0;
    cellInfo[ MATERIAL_ID ]             = 0;
    cellInfo[ NUMBER_OF_NODES_IN_CELL ] = 2;
    cellInfo[ CELL_TYPE ]               = UCD_LINE;

    /* Write the dimensions: */

    dimensions[ NUMBER_OF_NODES ]       = mapLines->vertexCount;
    dimensions[ NUMBER_OF_CELLS ]       = mapLines->vertexCount -
                                          mapLines->polylineCount;
    dimensions[ NUMBER_OF_NODE_DATA ]   = 0;
    dimensions[ NUMBER_OF_CELL_DATA ]   = 0;
    dimensions[ NUMBER_OF_MODEL_DATA ]  = 0;
    dimensions[ NUMBER_OF_NLIST_NODES ] = dimensions[ NUMBER_OF_CELLS ] *
                                          cellInfo[ NUMBER_OF_NODES_IN_CELL ];
    ok = writeInts ( file, dimensions, NUMBER_OF_DIMENSIONS );

    /* Write the cell info: */

    for ( polyline = 0; AND2 ( ok, polyline < mapLines->polylineCount );
            ++polyline )
        {
        int cell, numberOfCells = mapLines->lengths[ polyline ] - 1;

        for ( cell = 0; AND2 ( ok, cell < numberOfCells ); ++cell )
            {
            ok = writeInts ( file, cellInfo, NUMBER_OF_CELL_INFO_COMPONENTS );
            ++cellInfo[ CELL_ID ];
            }
        }

    CHECK ( IMPLIES ( ok, cellInfo[ CELL_ID ] == dimensions[ NUMBER_OF_CELLS ] ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeAVSMapTopology - Write cell topology to an AVS ucd file.
 * INPUTS:  File* file               The file to write to.
 *          const MapLines* mapLines The map lines to output.
 * OUTPUTS: File* file               The updated file structure.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:
 *****************************************************************************/

static int writeAVSMapTopology ( File* file, const MapLines* mapLines )
    {
    PRE2 ( file, isValidMapLines ( mapLines ) );

    int ok = 1, polyline;

    for ( polyline = 0; AND2 ( ok, polyline < mapLines->polylineCount );
            ++polyline )
        {
        const int startingIndex = mapLines->starts[ polyline ];
        const int endingIndex   = startingIndex + mapLines->lengths[ polyline ];
        const int endingIndexMinusOne = endingIndex - 1;
        int node;

        for ( node = startingIndex; AND2 ( ok, node < endingIndex ); ++node )
            {
            ok = writeInt ( file, node + 1 );

            if ( AND3 ( ok, node != startingIndex, node != endingIndexMinusOne ) )
                ok = writeInt ( file, node + 1 );
            }
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeAVSMapCoordinates - Write xyz coordinates to an AVS ucd file.
 * INPUTS:  File* file               The file to write to.
 *          const MapLines* mapLines The map lines to output.
 *          const float* z            Optional z coordinates for map vertices.
 * OUTPUTS: File* file               The updated file structure.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:
 *****************************************************************************/

static int writeAVSMapCoordinates ( File* file, const MapLines* mapLines,
                                    const float* z )
    {
    PRE2 ( file, isValidMapLines ( mapLines ) );

    int ok = 1, coordinate;

    for ( coordinate = 0; AND2 ( ok, coordinate < 2 ); ++coordinate )
        {
        int numberOfVertices = mapLines->vertexCount;
        const float* vertex  = mapLines->vertices;

        for ( ; AND2 ( ok, numberOfVertices-- ); vertex += 2 )
            ok = writeFloat ( file, vertex[ coordinate ] );
        }

    if ( ok )
        {
        int numberOfVertices = mapLines->vertexCount;

        if ( z ) ok = writeFloats ( file, z, numberOfVertices );
        else while ( AND2 ( ok, numberOfVertices-- ) ) ok = writeFloat ( file, 0.0 );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeDXMap - Write map lines to a DX file.
 * INPUTS:  File* file               The file to write to.
 *          const MapLines* mapLines The map lines to output.
 *          const float* z            Optional z coordinates for map vertices.
 * OUTPUTS: File* file               The updated file structure.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:   Writes file with XDR-format binary data interleaved with the
 *          descriptions since the DX IMPORT Module does not allow data
 *          offsets to be used with "!" commands.
 *****************************************************************************/

static int writeDXMap ( File* file, const MapLines* mapLines, const float* z )
    {
    PRE2 ( isValidFile ( file ), isValidMapLines ( mapLines ) );

    enum { POSITIONS, EDGES, POLYLINES, MAP, NUMBER_OF_DESCRIPTIONS };

    static const char* const descriptions[ NUMBER_OF_DESCRIPTIONS ] =
        {
        "# DX Field File for a MapLines structure.\n"
        "#data mode xdr\n" /* DX BUG: xdr not supported yet! */
        "data mode msb ieee\n" /* file int/float is equivalent to xdr. */
        "object \"position list\" "
        "class array type float rank 1 shape %d items %d data follows\n",

        "#\nobject \"edge list\" "
        "class array type int rank 0 items %d data follows\n",

        "attribute \"ref\" string \"positions\"\n"
        "#\n"
        "object \"polyline list\" "
        "class array type int rank 0 items %d data follows\n",

        "attribute \"ref\" string \"edges\"\n"
        "#\n"
        "object \"Map\" class field\n"
        "component \"positions\" value \"position list\"\n"
        "component \"edges\"     value \"edge list\"\n"
        "component \"polylines\" value \"polyline list\"\n"
        "#\n"
        "end\n"
        };

    int ok = 0, polyline, vertex;
    const int numberOfCoordinates = z ? 3 : 2;

    /* Write the ASCII header: */

    ok = writeString ( file, descriptions[ POSITIONS ],
                       numberOfCoordinates, mapLines->vertexCount );


    /* Write coordinates: */

    if ( z ) /* Must write an interleaved copy of the x,y,z vertices: */
        {
        float* coordinates = NEW ( float, mapLines->vertexCount * 3 );

        ok = coordinates != 0;

        if ( coordinates )
            {
            int numberOfVertices = mapLines->vertexCount;
            float*       dst  = coordinates;
            const float* src1 = mapLines->vertices;
            const float* src2 = z;

            while ( numberOfVertices-- )
                {
                *dst++ = *src1++;
                *dst++ = *src1++;
                *dst++ = *src2++;
                }

            ok = writeFloats ( file, coordinates, mapLines->vertexCount * 3 );

            FREE ( coordinates );
            }
        }
    else
        {
        ok = AND2 ( ok, writeFloats ( file, mapLines->vertices,
                                      mapLines->vertexCount * 2 ) );
        }


    ok = AND2 ( ok, writeString ( file, descriptions[ EDGES ],
                                  mapLines->vertexCount ) );

    /* Write connectivity: */

    for ( polyline = 0; AND2 ( ok, polyline < mapLines->polylineCount );
            ++polyline )
        {
        const int startingIndex = mapLines->starts[ polyline ];
        const int endingIndex   = startingIndex + mapLines->lengths[ polyline ];

        for ( vertex = startingIndex; AND2 ( ok, vertex < endingIndex ); ++vertex )
            ok = writeInt ( file, vertex );
        }


    ok = AND2 ( ok, writeString ( file, descriptions[ POLYLINES ],
                                  mapLines->polylineCount ) );

    /* Write polyline starts: */

    ok = AND2 ( ok, writeInts ( file, mapLines->starts,
                                mapLines->polylineCount ) );

    ok = AND2 ( ok, writeString ( file, descriptions[ MAP ] ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readAndConvertSegmentDirectory - Read the segment directory of a
 *          map file and initialize the starts and lengths of a MapLines
 *          structure.
 * INPUTS:  File* file               The file to read from.
 *          int numberOfSegments     The number of segments to read.
 * OUTPUTS: File* file               Updated file structure.
 *          int* segmentDirectory    Segment directory buffer to read into.
 *          MapLines* mapLines       mapLines->vertexCount is set to actual.
 *                                   mapLines->starts[]  is initialized.
 *                                   mapLines->lengths[] is initialized.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

static int readAndConvertSegmentDirectory ( File* file,
        int numberOfSegments,
        int* segmentDirectory,
        MapLines* mapLines )
    {
    PRE4 ( file, segmentDirectory, mapLines, numberOfSegments > 0 );

    int ok = 0;

    if ( readSegmentDirectory ( file, numberOfSegments, segmentDirectory ) )
        {
        ok = computePolylineAndVertexCounts ( nameOfFile ( file ), numberOfSegments,
                                              segmentDirectory,
                                              &mapLines->polylineCount,
                                              &mapLines->vertexCount );

        if ( ok ) convertStartsAndLengths ( numberOfSegments, segmentDirectory,
                                                mapLines->starts, mapLines->lengths );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: readAndConvertVertices - Read the vertices of a map file and
 *          initialize the vertices of a MapLines structure.
 * INPUTS:  File* file               The file to read.
 * OUTPUTS: File* file               Updated file structure.
 *          MapLines* mapLines       mapLines->vertices[] is initialized.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

static int readAndConvertVertices ( File* file, MapLines* mapLines )
    {
    PRE4 ( file, mapLines, mapLines->vertices, mapLines->vertexCount > 0 );

    int ok = 0;
    int* intVertices = 0; /* To hold file contents: scaled integer lat-lons. */

    /*
     * First allocate a temporary int array (scaled int lat-lon pairs) and
     * then read and convert the scaled integer vertex data.
     */

    intVertices = NEW ( int, 2 * mapLines->vertexCount );

    if ( intVertices )
        {
        ok = readVertices ( file, mapLines->vertexCount, intVertices );

        if ( ok ) convertVertices ( mapLines->vertexCount, intVertices,
                                        mapLines->vertices, mapLines->corners );

        FREE ( intVertices );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: readNumberOfSegments - Read the number of segments in a map file.
 * INPUTS:  File* file  The file to read from.
 * OUTPUTS: File* file  The updated file structure.
 * RETURNS: int The number of segments in the file (or 0 if unsuccessful).
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

static int readNumberOfSegments ( File* file )
    {
    PRE ( file );

    int numberOfSegments = 0;

    if ( readInt ( file, &numberOfSegments ) != 1 )
        {
        error ( "Failed to read number of segments in map file '%s'.",
                nameOfFile ( file ) );
        }
    else if ( numberOfSegments < 1 )
        {
        error ( "Bad number of segments (%d) in map file '%s'.",
                numberOfSegments, nameOfFile ( file ) );
        numberOfSegments = 0;
        }

    return numberOfSegments;
    }


/******************************************************************************
 * PURPOSE: readSegmentDirectory - Read the segment directory of a map file.
 * INPUTS:  File* file               The file to read from.
 *          int numberOfSegments     The number of segments to read.
 * OUTPUTS: File* file               Updated file structure.
 *          int* segmentDirectory    Filled segment directory.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

static int readSegmentDirectory ( File* file, int numberOfSegments,
                                  int* segmentDirectory )
    {
    PRE3 ( file, segmentDirectory, numberOfSegments > 0 );

    int ok = 1;
    int sizeOfsegmentDirectory = 0;

    sizeOfsegmentDirectory = numberOfSegments * NUMBER_OF_SEGMENT_COMPONENTS;

    if ( ! readInts ( file, segmentDirectory, sizeOfsegmentDirectory ) )
        {
        ok = 0;
        error ( "Could not read segments directory (%d words) of file '%s'\n",
                sizeOfsegmentDirectory, nameOfFile ( file ) );
        }

#ifdef DEBUGGING
        {
        int segment;

        for ( segment = 0; segment < numberOfSegments; ++segment )
            {
            const int* currentSegment = segmentDirectory +
                                        segment * NUMBER_OF_SEGMENT_COMPONENTS;

            printf ( "segment #%5d: "
                     "MIN_LAT = %d, MAX_LAT = %d, MIN_LON = %d, MAX_LON = %d, "
                     "START = %d, LENGTH = %d\n",
                     segment + 1,
                     currentSegment[ MIN_LAT ], currentSegment[ MAX_LAT ],
                     currentSegment[ MIN_LON ], currentSegment[ MAX_LON ],
                     currentSegment[ START   ], currentSegment[ LENGTH  ] );
            }
        }
#endif


    /*
     * Skip trailing garbage from the end of the segment directory.
     * The first start is the word offset to the first vertex so
     * seek to that byte in the file.
     */

    if ( ! seekFile ( file, SIZE_OF_IEEE_INT * segmentDirectory[START], SEEK_SET ) )
        {
        ok = 0;
        error ( "Could not seek to the end of the segments directory (%d words) "
                "of file '%s'\n", SIZE_OF_IEEE_INT * segmentDirectory[START],
                nameOfFile ( file ) );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: readVertices - Read the vertices of a map file.
 * INPUTS:  File* file               The file to read from.
 *          int numberOfVertices     The number of vertices to read.
 * OUTPUTS: File* file               Updated file structure.
 *          int vertices[]           Array of vertices (int lat-lon pairs).
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

static int readVertices ( File* file, int numberOfVertices, int vertices[] )
    {
    PRE3 ( file, vertices, numberOfVertices > 0 );

    int ok = 0;

    if ( ! readInts ( file, vertices, numberOfVertices * 2 ) )
        error ( "Could not read %d vertices from file '%s'\n",
                numberOfVertices, nameOfFile ( file ) );
    else
        {
        /*
         * Check the vertices.
         * They are represented as scaled integers = trunc( degrees x 10,000 )
         */

        const int* vertex = vertices;
        int v;

        for ( v = 0, ok = 1; AND2 ( ok, v < numberOfVertices ); ++v, vertex += 2 )
            {
            if ( ! IN_RANGE ( vertex[ LAT ], -900000, 900000 ) )
                {
                error ( "Bad latitude (%d) for vertex #%d in file '%s'.\n",
                        vertex[ LAT ], v + 1, nameOfFile ( file ) );
                ok = 0;
                }
            else if ( ! IN_RANGE ( vertex[ LON ], -1800000, 1800000 ) )
                {
                error ( "Bad longitude (%d) for vertex #%d in file '%s'.\n",
                        vertex[ LON ], v + 1, nameOfFile ( file ) );
                ok = 0;
                }

            DEBUG ( printf ( "vertex #%5d: lat = %d, lon = %d\n",
                             v + 1, vertex[ LAT ], vertex[ LON ] ); )
            }
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: computePolylineAndVertexCounts - Compute the number of
 *          non-zero-length polylines and the total number of vertices (the sum
 *          of the lengths) from a segment directory.
 * INPUTS:  const char* mapFileName       The name of the file that was read.
 *          int         numberOfSegments  The size of the segment directory.
 *          const int segmentDirectory[]  The segment directory to scan.
 * OUTPUTS: int* numberOfPolylines        The number of polylines.
 *          int* numberOfVertices         The number of vertices.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   Calls error() if an error (bad value) is detected.
 *****************************************************************************/

static int computePolylineAndVertexCounts ( const char* mapFileName,
        int numberOfSegments,
        const int segmentDirectory[],
        int* numberOfPolylines,
        int* numberOfVertices )
    {
    PRE5 ( mapFileName, numberOfSegments > 0, segmentDirectory,
           numberOfPolylines, numberOfVertices );

    int ok = 1;
    const int* previousSegment = 0;
    const int* currentSegment  = segmentDirectory;
    int segment; /* Index on segments. */

    /*
     * Sum the segment lengths to find the total number of vertices.
     * Note: Vertices are not shared. Also, the lengths given are in
     * 'words' and include both lat and lon so we must divide by 2 to
     * obtain the length in '# of vertices'.
     * Zero-length segments are not counted.
     * Negative-length segments are a data error.
     */

    *numberOfPolylines = *numberOfVertices = 0;

    for ( segment = 0; AND2 ( ok, segment < numberOfSegments ); ++segment,
            previousSegment = currentSegment,
            currentSegment += NUMBER_OF_SEGMENT_COMPONENTS )
        {
        int length = currentSegment[ LENGTH ] / 2;

        if ( length > 0 )
            {
            ++*numberOfPolylines;
            *numberOfVertices += length;
            }
        else if ( length < 0 )
            {
            ok = 0;
            error ( "Bad segment length: %d (segment #%d) in file '%s'.\n",
                    length, segment + 1, mapFileName );
            }
        DEBUG ( else printf ( "Skipped zero-length segment #%d\n",
                                  segment + 1 ); )
            if ( ok )
                {
                ok = IMPLIES ( previousSegment,
                               currentSegment[  START ] ==
                               previousSegment[ START ] + previousSegment[ LENGTH ] );
                if ( ! ok )
                    {
                    error ( "Inconsistent segment #%d: START = %d, LENGTH = %d",
                            segment + 1, currentSegment[ START ], currentSegment[ LENGTH ] );
                    }
                }
        }

    if ( AND2 ( ok, *numberOfVertices == 0 ) )
        {
        ok = 0;
        error ( "No positive-length segments found in file '%s'.\n", mapFileName );
        }

    if ( ! ok ) *numberOfPolylines = *numberOfVertices = 0;

    return ok;
    }


/******************************************************************************
 * PURPOSE: convertStartsAndLengths - Convert starts and lengths of segments.
 * INPUTS:  int numberOfSegments     The number of segments in the directory.
 *          const int segmentDirectory[]  Segment directory to scan.
 * OUTPUTS: int starts[]                  Array of indices of segment starts.
 *          int lengths[]                 Array of segment lengths.
 * RETURNS: None
 * NOTES:   The 'starts' given in the file are actually word offsets
 *          from the beginning of the file. These are converted to word
 *          offsets from the beginning of the vertex data before storing
 *          in starts[].
 *          The lengths given are in 'words' and include both lat and
 *          lon so we must divide by 2 to obtain the length in '# of vertices'.
 *****************************************************************************/

static void convertStartsAndLengths ( int numberOfSegments,
                                      const int segmentDirectory[],
                                      int starts[], int lengths[] )
    {
    PRE4 ( numberOfSegments > 0, segmentDirectory, starts, lengths );

    int segment;
    int initializedFirstStart = 0; /* Flag: initialized first start value? */

    for ( segment = 0; segment < numberOfSegments; ++segment )
        {
        const int* currentSegment = segmentDirectory +
                                    segment * NUMBER_OF_SEGMENT_COMPONENTS;

        const int length = currentSegment[ LENGTH ] / 2;

        if ( length > 0 )
            {
            if ( ! initializedFirstStart )
                {
                *starts = 0;
                initializedFirstStart = 1;
                }
            else /* Sum the previous start and length to obtain the new start. */
                {
                *starts = * ( starts - 1 ) + * ( lengths - 1 );
                }

            ++starts;
            *lengths++ = length;

            DEBUG ( printf ( "start = %d, length = %d\n",
                             * ( starts - 1 ), * ( lengths - 1 ) ); )
            }
        DEBUG ( else printf ( "Skipped %d-length segment #%d\n",
                                  length, segment + 1 ); )
            }
    }


/******************************************************************************
 * PURPOSE: convertVertices - Convert the int vertices to (unscaled) floats.
 * INPUTS:  int numberOfVertices  The number of vertices to read.
 *          const int vertices[]  Array of int vertices (scaled lat-lon pairs).
 * OUTPUTS: float vertices[]      2d Array of float vertices (in deg lat-lon).
 *          double corners[2][2]  Minmax lat-lon.
 * RETURNS: None
 * NOTES:   The int vertices are represented as trunc( degrees x 10,000 ) with
 *          Eastern longitudes negated. They will be converted to decimal
 *          degrees with Western longitudes negated.
 *****************************************************************************/

static void convertVertices ( int numberOfVertices, const int intVertices[],
                              float vertices[], double corners[2][2] )
    {
    PRE4 (  numberOfVertices > 0, intVertices, vertices, corners );

    const float unscaleFactor = 0.0001;  /* Convert to decimal degrees.        */
    const int*  intVertex = intVertices; /* Running pointer for intVertices[]. */
    float*      vertex    = vertices;    /* Running pointer for vertices[].    */
    int v;

    /* Initialize range: */

    corners[ LOWER ][ LAT ] =
        corners[ UPPER ][ LAT ] = unscaleFactor *  intVertex[ LAT ];

    corners[ LOWER ][ LON ] =
        corners[ UPPER ][ LON ] = unscaleFactor * -intVertex[ LON ];

    for ( v = 0; v < numberOfVertices; ++v, vertex += 2, intVertex += 2 )
        {
        const float lat = unscaleFactor *  intVertex[ LAT ];
        const float lon = unscaleFactor * -intVertex[ LON ];
        vertex[ LAT ] = lat;
        vertex[ LON ] = lon;
        updateMinmax ( lat, &corners[ LOWER ][ LAT ], &corners[ UPPER ][ LAT ] );
        updateMinmax ( lon, &corners[ LOWER ][ LON ], &corners[ UPPER ][ LON ] );
        DEBUG ( printf ( "vertex #%5d: lat = %f, lon = %f\n", v + 1, lat, lon ); )
        }
    }


/******************************************************************************
 * PURPOSE: updateMinmax - Compares a value to a range and widens the range if
 *          necessary.
 * INPUTS:  double  value    The value to compare.
 *          double* minimum  The current minimum.
 *          double* maximum  The current maximum.
 * OUTPUTS: double* minimum  The updated minimum.
 *          double* maximum  The updated maximum.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void updateMinmax ( double value, double* minimum, double* maximum )
    {
    PRE3 ( minimum, maximum, *minimum <= *maximum );

    if ( value < *minimum ) *minimum = value;

    if ( value > *maximum ) *maximum = value;

    POST ( IN_RANGE ( value, *minimum, *maximum ) );
    }


