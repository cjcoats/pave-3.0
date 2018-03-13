/******************************************************************************
 * PURPOSE: TopographyFile.c - Defines functions for reading and writing
 *          unprojected McIDAS topography files.
 * NOTES:   Required source-code control string:
 *          "$Id: TopographyFile.c 83 2018-03-12 19:24:33Z coats $"
 *          Based on description of McIDAS topography files as gleaned from
 *          reading the maketopo.c and topo.h and topo.c files that are part
 *          of the Vis5d distribution.
 * HISTORY: 12/96, Todd Plessel, EPA/MMTSI, Created.
 *****************************************************************************/

/*================================ INCLUDES =================================*/

#include <string.h>         /* For memcpy(), memset().                */

#include "Assertions.h"     /* For macros PRE(), POST().              */
#include "Error.h"          /* For error().                           */
#include "Memory.h"         /* For macros NEW(), FREE().              */
#include "File.h"           /* File, open/closeFile(), read/write*(). */
#include "TopographyFile.h" /* For public functions.                  */

/*================================= DEFINES =================================*/

#define OFFSET2( j, i, IDIM ) ( (j) * (IDIM) + (i) )

/*================================== TYPES ==================================*/

enum { SIZE_OF_IEEE_FLOAT = 4 };

enum { LOWER, UPPER };
enum { LAT, LON };

typedef int ( *TopographyExportFunction ) ( File*, const Topography* );

/*
 * McIDAS topography file header. See Vis5d code.
 * Assumes IEEE-754 4-byte ints and floats. With header totaling 64-bytes.
 */

enum { ID_SIZE = 40 };

typedef struct
    {
    char id[ ID_SIZE ]; /* "TOPO" uses integer bounds. "TOPO2" uses floats.   */
    float westlon;  /* West longitude degrees of bounds. (Negative if East!). */
    float eastlon;  /* East longitude degrees of bounds. (Negative if East!). */
    float northlat; /* North latitude degrees of bounds. (Negative if South). */
    float southlat; /* South latitude degrees of bounds. (Negative if South). */
    int rows;       /* Number of rows:    the latitude  dimension.            */
    int columns;    /* Number of columns: the longitude dimension.            */
    } Header;


/*============================ PRIVATE VARIABLES ============================*/

/* Required source-code control string. */
static char* TopographyFile_version = "$Id: TopographyFile.c 83 2018-03-12 19:24:33Z coats $";

/*========================== FORWARD DECLARATIONS ===========================*/

static void zeroTopography ( Topography* topography );

static int isValidLatitudeLongitude ( double latitude, double longitude );

static int areValidDomainCorners ( const double corners[ 2 ][ 2 ] );

static int readHeader ( File* file, Header* header );

static int writeHeader ( File* file, const Topography* topography );

static int isValidHeader ( const Header* header );

static void decodeElevations ( int numberOfRows,
                               int numberOfColumns,
                               const short* encodedElevations,
                               float* elevations );

static void encodeElevations ( int numberOfRows,
                               int numberOfColumns,
                               const float* elevations,
                               short* encodedElevations );

static int writeFASTTopography ( File* file, const Topography* topography );

static int writeAVSTopography ( File* file, const Topography* topography );

static int writeDXTopography ( File* file, const Topography* topography );

static int writeMcIDASTopography ( File* file, const Topography* topography );

static void range ( const float data[], size_t numberOfValues,
                    float* minimum, float* maximum );

static void transposeData2 ( const float* data, size_t numberOfRows,
                             size_t numberOfColumns, float* transposedData );

/*============================ PUBLIC FUNCTIONS =============================*/


/******************************************************************************
 * PURPOSE: isValidTopography - Verify the validity of a Topography struct.
 * INPUTS:  const Topography* topography The struct to verify.
 * OUTPUTS: None
 * RETURNS: 1 if the struct is ok, else 0.
 * NOTES:
 *****************************************************************************/

int isValidTopography ( const Topography* topography )
    {
    int ok = AND5 ( topography,
                    topography->elevations,
                    topography->dimensions[ LAT ] > 0,
                    topography->dimensions[ LON ] > 0,
                    areValidDomainCorners ( ( const double ( * ) [2] )
                                            topography->corners ) );

    return ok;
    }


/******************************************************************************
 * PURPOSE: allocateTopography - Allocate a Topography structure
 *          (using malloc()).
 * INPUTS:  int numberOfRows
 *          int numberOfColumns
 * OUTPUTS: Topography* topography
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

int allocateTopography ( int numberOfRows, int numberOfColumns,
                         Topography* topography )
    {
    int ok = 0;
    const size_t numberOfVertices = numberOfRows * numberOfColumns;

    PRE3 ( numberOfRows > 0, numberOfColumns > 0, topography )

    zeroTopography ( topography );

    topography->elevations = NEW ( float, numberOfVertices );

    ok = topography->elevations != 0;

    if ( ok )
        {
        memset ( topography->elevations, 0, numberOfVertices );
        topography->dimensions[ LAT ] = numberOfRows;
        topography->dimensions[ LON ] = numberOfColumns;
        }

    POST2 ( IMPLIES ( ok, AND4 ( isValidTopography ( topography ),
                                 topography->elevations,
                                 topography->dimensions[ LAT ] == numberOfRows,
                                 topography->dimensions[ LON ] == numberOfColumns ) ),
            IMPLIES ( ! ok, AND4 ( isValidTopography ( topography ),
                                   topography->elevations,
                                   topography->dimensions[ LAT ],
                                   topography->dimensions[ LON ] ) ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: deallocateTopography - Free a Topography structure (using free()).
 * INPUTS:  Topography* topography
 * OUTPUTS: Topography* topography
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

void deallocateTopography ( Topography* topography )
    {
    PRE ( isValidTopography ( topography ) )

    FREE ( topography->elevations );

    zeroTopography ( topography );

    POST ( ! isValidTopography ( topography ) )
    }


/******************************************************************************
 * PURPOSE: sizeOfTopographyFile - Determine the number of rows and
 *          columns in a topography file.
 * INPUTS:  const char* fileName  The name of the topography file to check.
 * OUTPUTS: int* numberOfRows     The number of rows    in the file.
 *          int* numberOfColumns  The number of columns in the file.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int sizeOfTopographyFile ( const char* fileName,
                           int* numberOfRows, int* numberOfColumns )
    {
    int ok = 0;
    File file;

    PRE3 ( fileName, numberOfColumns, numberOfRows )

    *numberOfColumns = *numberOfRows = 0;

    if ( openFile ( fileName, "r", &file ) )
        {
        Header header;

        if ( readHeader ( &file, &header ) )
            {
            *numberOfRows    = header.rows;
            *numberOfColumns = header.columns;

            ok = 1;
            }

        closeFile ( &file );
        }

    POST2 ( IMPLIES (   ok, GT_ZERO2 ( *numberOfRows, *numberOfColumns ) ),
            IMPLIES ( ! ok, IS_ZERO2 ( *numberOfRows, *numberOfColumns ) ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: readTopographyFile - Read topography data from a McIDAS file.
 * INPUTS:  const char* fileName    The name of the file to read.
 * OUTPUTS: Topography* topography  The topography data read.
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *          The topography structure must be allocated appropriately before
 *          a call to this routine.
 *****************************************************************************/

int readTopographyFile ( const char* fileName, Topography* topography )
    {
    int  ok = 0;
    File file;

    PRE2 ( fileName, isValidTopography ( topography ) )

    if ( openFile ( fileName, "r", &file ) )
        {
        Header header;

        if ( readHeader ( &file, &header ) )
            {
            if ( OR2 ( header.rows    > topography->dimensions[ LAT ],
                       header.columns > topography->dimensions[ LON ] ) )
                {
                error ( "The topography structure (%d rows x %d columns )\n"
                        "is not large enough to hold data (%d rows x %d columns)\n"
                        "from the file '%s'\n",
                        topography->dimensions[ LAT ], topography->dimensions[ LON ],
                        header.rows, header.columns, file.name );
                }
            else
                {
                const int numberOfVertices = topography->dimensions[ LAT ] *
                                             topography->dimensions[ LON ];

                /* Allocate and read/process the encoded elevations. */

                short* encodedElevations = NEW ( short, numberOfVertices );

                /* Initialize the corners: */

                topography->corners[ LOWER ][ LAT ] = header.southlat;
                topography->corners[ UPPER ][ LAT ] = header.northlat;
                topography->corners[ LOWER ][ LON ] = header.westlon;
                topography->corners[ UPPER ][ LON ] = header.eastlon;

                if ( encodedElevations )
                    {
                    /* Read encoded elevations into encodedElevations: */

                    ok = readShorts2 ( &file, encodedElevations, numberOfVertices );

                    /* Decode them into topography->elevations: */

                    if ( ok )
                        {
                        decodeElevations ( topography->dimensions[ LAT ],
                                           topography->dimensions[ LON ],
                                           encodedElevations,
                                           topography->elevations );
                        }

                    FREE ( encodedElevations );
                    }
                }
            }

        closeFile ( &file );
        }

    POST ( IMPLIES ( ok, isValidTopography ( topography ) ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeTopographyFile - Write the topography data to a file.
 * INPUTS:  const char*       fileName    The name of the file to write or
 *                                        "-stdout".
 *          const Topography* topography  The topography data to output.
 *          int               format      FAST_GRID_FORMAT, AVS_FIELD_FORMAT,...
 * OUTPUTS: None
 * RETURNS: int 1 if successful, else 0.
 * NOTES:   If unsuccessful then error() is called.
 *****************************************************************************/

int writeTopographyFile ( const char* fileName, const Topography* topography,
                          int format )
    {
    static TopographyExportFunction
    exportFunctions[ NUMBER_OF_TOPOGRAPHY_FORMATS ] =
        {
        writeFASTTopography,
        writeAVSTopography,
        writeDXTopography,
        writeMcIDASTopography
        };

    TopographyExportFunction exportFunction = 0; /* Selected export function. */
    File file;
    int ok = 0;

    PRE4 ( fileName, topography, isValidTopography ( topography ),
           IN_RANGE ( format, 0, NUMBER_OF_TOPOGRAPHY_FORMATS - 1 ) )

    exportFunction = exportFunctions[ format ]; /* Pick export function. */

    /* Trap change: */

    CHECK ( exportFunctions[ NUMBER_OF_TOPOGRAPHY_FORMATS - 1 ] != 0 )

    if ( openFile ( fileName, "w", &file ) )
        {
        ok = exportFunction ( &file, topography );

        ok = AND2 ( ok, closeFile ( &file ) );
        }

    if ( ! ok ) error ( "Failed to write topography file '%s'.", fileName );

    return ok;
    }


/*=========================== PRIVATE FUNCTIONS =============================*/


/******************************************************************************
 * PURPOSE: zeroTopography - Zeros all fields of a topography structure.
 * INPUTS:  None
 * OUTPUTS: Topography* topography  Topography structure zero'd out.
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void zeroTopography ( Topography* topography )
    {
    memset ( topography, 0, sizeof ( Topography ) );
    POST ( ! isValidTopography ( topography ) )
    }


/******************************************************************************
 * PURPOSE: isValidLatitudeLongitude - Determine if the given latitude and
 *          longitude point is valid.
 * INPUTS:  double latitude   The latitude  coordinate.
 *          double longitude  The longitude coordinate.
 * OUTPUTS: None
 * RETURNS: int 1 if valid else 0.
 * NOTES:
 *****************************************************************************/

static int isValidLatitudeLongitude ( double latitude, double longitude )
    {
    return AND2 ( IN_RANGE ( latitude,   -90.0,  90.0 ),
                  IN_RANGE ( longitude, -180.0, 180.0 ) );
    }


/******************************************************************************
 * PURPOSE: areValidDomainCorners - Verify that the domain corners are valid.
 * INPUTS:  const double corners[ 2 ][ 2 ]  [ LOWER | UPPER ][ LAT | LON ].
 * OUTPUTS: None
 * RETURNS: int 1 if valid, else 0.
 * NOTES:
 *****************************************************************************/

static int areValidDomainCorners ( const double corners[ 2 ][ 2 ] )
    {
    int ok;

    ok = AND5 ( corners,
                isValidLatitudeLongitude ( corners[ LOWER ][ LAT ],
                                           corners[ LOWER ][ LON ] ),
                isValidLatitudeLongitude ( corners[ UPPER ][ LAT ],
                                           corners[ UPPER ][ LON ] ),
                corners[ LOWER ][ LAT ] <= corners[ UPPER ][ LAT ],
                corners[ LOWER ][ LON ] <= corners[ UPPER ][ LON ] );

    return ok;
    }


/******************************************************************************
 * PURPOSE: readHeader - Read the header of a McIDAS topography file.
 * INPUTS:  File* file                The opened file to read the header of.
 * OUTPUTS: File* file                The updated file structure.
 *          Header* header            The McIDAS file header.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:
 *****************************************************************************/

static int readHeader ( File* file, Header* header )
    {
    int ok = 0;

    PRE2 ( isValidFile ( file ), header )

    ok = readBytes ( file, header->id, ID_SIZE );

    if ( strcmp ( header->id, "TOPO2" ) == 0 ) /* New format: float bounds.*/
        {
        ok = AND2 ( ok, readFloat ( file, &header->westlon  ) );

        ok = AND2 ( ok, readFloat ( file, &header->eastlon  ) );

        ok = AND2 ( ok, readFloat ( file, &header->northlat ) );

        ok = AND2 ( ok, readFloat ( file, &header->southlat ) );
        }
    else if ( strcmp ( header->id, "TOPO" ) == 0 ) /* Old format:int bounds. */
        {
        int westlon, eastlon, northlat, southlat;

        ok = AND2 ( ok, readInt ( file, &westlon  ) );

        ok = AND2 ( ok, readInt ( file, &eastlon  ) );

        ok = AND2 ( ok, readInt ( file, &northlat ) );

        ok = AND2 ( ok, readInt ( file, &southlat ) );

        if ( ok ) /* Convert int to float and unscale: */
            {
            header->westlon  = westlon  / 100.0;
            header->eastlon  = eastlon  / 100.0;
            header->northlat = northlat / 100.0;
            header->southlat = southlat / 100.0;
            }
        }
    else ok = 0;

    if ( ok )
        {
        /* Negate longitudes so that Western longitudes are negative: */

        header->westlon = -header->westlon;
        header->eastlon = -header->eastlon;
        }

    ok = AND2 ( ok, readInt (   file, &header->rows     ) );

    ok = AND2 ( ok, readInt (   file, &header->columns  ) );

    ok = AND2 ( ok, isValidHeader ( header ) );

    if ( ! ok )
        {
        error ( "Invalid header in McIDAS TOPO file '%s':\n"
                "  id = '%s'\n"
                "  westlon  = %f, eastlon  = %f\n"
                "  northlat = %f, southlat = %f\n"
                "  rows     = %d, columns  = %d\n",
                file->name,
                header->id,
                header->westlon,  header->eastlon,
                header->northlat, header->southlat,
                header->rows,     header->columns );
        }

    POST ( IMPLIES ( ok, isValidHeader ( header ) ) )

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeHeader - Write the header of a McIDAS topography file.
 * INPUTS:  File* file                   The opened file to read the header of.
 *          const Topography* topography The Topography to write.
 * OUTPUTS: File* file                   The updated file structure.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:
 *****************************************************************************/

static int writeHeader ( File* file, const Topography* topography )
    {
    int ok = 0;
    char id[ ID_SIZE ];

    PRE2 ( isValidFile ( file ), isValidTopography ( topography ) )

    memset ( id, 0, sizeof id );
    strcpy ( id, "TOPO2" ); /* Use float bounds. */

    ok = writeBytes ( file, id, ID_SIZE );

    ok = AND2 ( ok, writeFloat ( file,
                                 ( float ) -topography->corners[ LOWER ][ LON ] ) );

    ok = AND2 ( ok, writeFloat ( file,
                                 ( float ) -topography->corners[ UPPER ][ LON ] ) );

    ok = AND2 ( ok, writeFloat ( file,
                                 ( float ) topography->corners[ UPPER ][ LAT ] ) );

    ok = AND2 ( ok, writeFloat ( file,
                                 ( float ) topography->corners[ LOWER ][ LAT ] ) );

    ok = AND2 ( ok, writeInt ( file, topography->dimensions[ LAT ] ) );

    ok = AND2 ( ok, writeInt ( file, topography->dimensions[ LON ] ) );

    if ( ! ok )
        error ( "Couldn't write header of McIDAS TOPO file '%s'!\n", file->name );

    return ok;
    }


/******************************************************************************
 * PURPOSE: isValidHeader - Verify the validity of a McIDAS header.
 * INPUTS:  const Header* header  The header to verify.
 * OUTPUTS: None
 * RETURNS: 1 if the header is ok, else 0.
 * NOTES:
 *****************************************************************************/

static int isValidHeader ( const Header* header )
    {
    int ok = AND3 ( header,
                    OR2 ( strcmp ( header->id, "TOPO"  ) == 0,
                          strcmp ( header->id, "TOPO2" ) == 0 ),
                    GT_ZERO2 ( header->rows, header->columns ) );

    if ( ok )
        {
        double corners[ 2 ][ 2 ];

        corners[ LOWER ][ LON ] = header->westlon;
        corners[ UPPER ][ LON ] = header->eastlon;
        corners[ LOWER ][ LAT ] = header->southlat;
        corners[ UPPER ][ LAT ] = header->northlat;

        ok = areValidDomainCorners ( ( const double ( * ) [2] ) corners );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: decodeElevations - Decode an array of McIDAS-format topography
 *          elevation values.
 * INPUTS:  int numberOfRows                The number of rows of data.
 *          int numberOfColumns             The number of columns of data.
 *          const short* encodedElevations  The McIDAS-encoded elevation data.
 * OUTPUTS: float* elevations               The decimal meters elevation data.
 * RETURNS: None
 * NOTES:   It is assumed that encodedElevations and elevations arrays have
 *          already been appropriately allocated.
 *****************************************************************************/

static void decodeElevations ( int numberOfRows,
                               int numberOfColumns,
                               const short* encodedElevations,
                               float* elevations )
    {
    const int numberOfRows_1 = numberOfRows - 1;

    int row;

    PRE3 ( GT_ZERO2 ( numberOfRows, numberOfColumns ),
           encodedElevations, elevations )

    /*
     * According to the Vis5d code, the elevation values
     * (in units of meters above mean sealevel)
     * are encoded as follows:
     *  1. Convert from float to two-byte big-endian short.
     *  2. Multiply by two.
     *  3. Add one if 'water flag' is set - i.e., the vertex is over water.
     *
     * To decode them, the 'water flag' is ignored and they are just halved
     * and stored as a float.
     *
     * Also, the encoded data are stored as data[ row ][ column ] but with
     * row = 0 at the North Pole so it must be flipped in Y.
     */

    for ( row = 0; row < numberOfRows; ++row )
        {
        int column;

        for ( column = 0; column < numberOfColumns; ++column )
            {
            const float value = *encodedElevations++ >> 1;
            const int offset = OFFSET2 ( numberOfRows_1 - row, column,
                                         numberOfColumns );
            elevations[ offset ] = value;
            }
        }
    }


/******************************************************************************
 * PURPOSE: encodeElevations - Encode an array of topography elevation values
 *          to McIDAS-format .
 * INPUTS:  int numberOfRows                The number of rows of data.
 *          int numberOfColumns             The number of columns of data.
 *          const float* elevations         The decimal meters elevation data.
 * OUTPUTS: short* encodedElevations        The McIDAS-encoded elevation data.
 * RETURNS: None
 * NOTES:   It is assumed that encodedElevations and elevations arrays have
 *          already been appropriately allocated.
 *****************************************************************************/

static void encodeElevations ( int numberOfRows,
                               int numberOfColumns,
                               const float* elevations,
                               short* encodedElevations )
    {
    const int numberOfRows_1 = numberOfRows - 1;

    int row;

    PRE3 ( GT_ZERO2 ( numberOfRows, numberOfColumns ),
           elevations, encodedElevations )

    /*
     * According to the Vis5d code, the elevation values
     * (in units of meters above mean sealevel)
     * are encoded as follows:
     *  1. Convert from float to two-byte big-endian short.
     *  2. Multiply by two.
     *  3. Add one if 'water flag' is set - i.e., the vertex is over water.
     *     Here the 'water flag' is never set.
     *
     * Also, the encoded data are stored as data[ row ][ column ] but with
     * row = 0 at the North Pole so it must be flipped in Y.
     */

    for ( row = 0; row < numberOfRows; ++row )
        {
        int column;

        for ( column = 0; column < numberOfColumns; ++column )
            {
            const int ivalue  = ( int ) *elevations++;
            const short value = ivalue << 1;
            const int offset = OFFSET2 ( numberOfRows_1 - row, column,
                                         numberOfColumns );
            encodedElevations[ offset ] = value;
            }
        }
    }


/******************************************************************************
 * PURPOSE: writeFASTTopography - Write topography to a FAST file.
 * INPUTS:  File* file                    The file to write to.
 *          const Topography* topography  The topography data to output.
 * OUTPUTS: File* file                    The updated file structure.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:   Write the data as a binary 3D grid file.
 *****************************************************************************/

static int writeFASTTopography ( File* file, const Topography* topography )
    {
    int ok = 1;

    PRE2 ( isValidFile ( file ), isValidTopography ( topography ) )

    if ( ok )
        {
        const int numberOfRows     = topography->dimensions[ LAT ];
        const int numberOfColumns  = topography->dimensions[ LON ];

        const int numberOfVertices = numberOfRows * numberOfColumns;

        const double xOrigin       = topography->corners[ LOWER ][ LON ];
        const double yOrigin       = topography->corners[ LOWER ][ LAT ];

        const double xDelta        = ( topography->corners[ UPPER ][ LON ] -
                                       topography->corners[ LOWER ][ LON ] )
                                     / numberOfColumns;

        const double yDelta        = ( topography->corners[ UPPER ][ LAT ] -
                                       topography->corners[ LOWER ][ LAT ] )
                                     / numberOfRows;

        int dimensions[ 3 ];

        const int numberOfGridDimensions = 2;

        float* gridCoordinates = NEW ( float, numberOfGridDimensions *
                                       numberOfVertices );

        ok = gridCoordinates != 0;

        if ( ok )
            {
            int    vertex; /* Index on vertices. */
            float* coordinate = gridCoordinates; /* Pointer to each. */
            double  x = xOrigin;
            double  y = yOrigin - yDelta;

            for ( vertex = 0; vertex < numberOfVertices; ++vertex )
                {
                if ( vertex % numberOfColumns == 0 ) x = xOrigin;

                *coordinate++ = ( float ) x;
                x += xDelta;
                }

            for ( vertex = 0; vertex < numberOfVertices; ++vertex )
                {
                if ( vertex % numberOfColumns == 0 ) y += yDelta;

                *coordinate++ = ( float ) y;
                }

            dimensions[ 0 ] = numberOfColumns;
            dimensions[ 1 ] = numberOfRows;
            dimensions[ 2 ] = 1;

            ok = AND2 ( ok, writeInts ( file, dimensions,
                                        numberOfGridDimensions + 1 ) );

            ok = AND2 ( ok, writeFloats ( file, gridCoordinates,
                                          numberOfGridDimensions *
                                          numberOfVertices ) );

            ok = AND2 ( ok, writeFloats ( file, topography->elevations,
                                          numberOfVertices ) );

            FREE ( gridCoordinates );
            }
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeAVSTopography - Write topography to an AVS ucd file.
 * INPUTS:  File* file                   The file to write to.
 *          const Topography* topography The topography to output.
 * OUTPUTS: File* file                   The updated file structure.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:   Until I obtain information on the format of AVS '.geom' files,
 *          This routine will output binary UCD files instead.
 *****************************************************************************/

static int writeAVSTopography ( File* file, const Topography* topography )
    {
    int ok = 1;

    PRE2 ( isValidFile ( file ), isValidTopography ( topography ) )

    if ( ok )
        {
        const int numberOfRows     = topography->dimensions[ LAT ];
        const int numberOfColumns  = topography->dimensions[ LON ];

        const int numberOfVertices = numberOfRows * numberOfColumns;

        const double xOrigin       = topography->corners[ LOWER ][ LON ];
        const double yOrigin       = topography->corners[ LOWER ][ LAT ];

        const double xDelta        = ( topography->corners[ UPPER ][ LON ] -
                                       topography->corners[ LOWER ][ LON ] )
                                     / numberOfColumns;

        const double yDelta        = ( topography->corners[ UPPER ][ LAT ] -
                                       topography->corners[ LOWER ][ LAT ] )
                                     / numberOfRows;

        const double xMax          = topography->corners[ UPPER ][ LON ];
        const double yMax          = topography->corners[ UPPER ][ LAT ];

        const char controlL = 12; /* Form-feed character. */

        const int numberOfRectilinearCoordinates = numberOfColumns + numberOfRows;

        float* rectilinearCoordinates = NEW ( float, numberOfRectilinearCoordinates );

        float minimumElevation, maximumElevation;

        ok = rectilinearCoordinates != 0;

        if ( ok )
            {
            range ( topography->elevations, numberOfVertices,
                    &minimumElevation, &maximumElevation );
            }

        ok = AND2 ( ok, writeString ( file, "# AVS field file\n" ) );

        ok = AND2 ( ok, writeString ( file, "#\n# Unprojected topography"
                                      " data:\n" ) );

        ok = AND2 ( ok, writeString ( file, "#   2D regular grid (in degrees "
                                      "longitude, latitude)\n" ) );

        ok = AND2 ( ok, writeString ( file, "#   elevation data (in meters)\n" ) );

        ok = AND2 ( ok, writeString ( file, "#\n" ) );

        ok = AND2 ( ok, writeString ( file, "ndim    = 2           "
                                      "# Longitude and Latitude.\n" ) );

        ok = AND2 ( ok, writeString ( file, "dim1    = %-11d # Longitude.\n",
                                      numberOfColumns ) );

        ok = AND2 ( ok, writeString ( file, "dim2    = %-11d # Latitude.\n",
                                      numberOfRows ) );

        ok = AND2 ( ok, writeString ( file, "nspace  = 2           "
                                      "# 2D grid coordinates.\n" ) );

        ok = AND2 ( ok, writeString ( file, "veclen  = 1           "
                                      "# One data value - elevation.\n" ) );

        ok = AND2 ( ok, writeString ( file, "data    = xdr_float   "
                                      "# Portable binary format.\n" ) );

        ok = AND2 ( ok, writeString ( file, "field   = rectilinear "
                                      "# Equally spaced coordinates.\n" ) );

        ok = AND2 ( ok, writeString ( file, "min_ext = %-5g %-5g "
                                      "# Lower-left (lon, lat).\n",
                                      xOrigin, yOrigin ) );

        ok = AND2 ( ok, writeString ( file, "max_ext = %-5g %-5g "
                                      "# Upper-right (lon, lat).\n",
                                      xMax, yMax ) );

        ok = AND2 ( ok, writeString ( file, "label   = elevation   "
                                      "# Elevation is the only data.\n" ) );

        ok = AND2 ( ok, writeString ( file, "unit    = m           "
                                      "# Elevation is in meters above/"
                                      "below mean sealevel.\n" ) );

        ok = AND2 ( ok, writeString ( file, "min_val = %-11g "
                                      "# Minimum elevation.\n",
                                      minimumElevation ) );

        ok = AND2 ( ok, writeString ( file, "max_val = %-11g "
                                      "# Maximum elevation.\n",
                                      maximumElevation ) );

        ok = AND2 ( ok, writeString ( file, "#\n# End of ASCII header.\n" ) );

        ok = AND2 ( ok, writeString ( file, "# Below (following two <control-L>"
                                      " characters) is the elevation data"
                                      "\n# in XDR binary float format"
                                      " (%d x %d x %d = %d bytes).\n",
                                      numberOfColumns, numberOfRows,
                                      SIZE_OF_IEEE_FLOAT,
                                      numberOfVertices *
                                      SIZE_OF_IEEE_FLOAT ) );

        ok = AND2 ( ok, writeString ( file, "# Followed by the grid coordinates"
                                      " in XDR binary float"
                                      " rectilinear form:\n# "
                                      " (%d x's + %d y's) * %d = %d bytes"
                                      ".\n",
                                      numberOfColumns, numberOfRows,
                                      SIZE_OF_IEEE_FLOAT,
                                      numberOfRectilinearCoordinates
                                      * SIZE_OF_IEEE_FLOAT ) );

        /*
         * Write two <control-L> (formfeed) characters to separate the header and
         * the binary data sections. See AVS User's Guide p 2-13.
         */

        ok = AND2 ( ok, writeString ( file, "%c%c", controlL ) );

        /* Write data section - elevation values followed by grid coordinates: */

        ok = AND2 ( ok, writeFloats ( file, topography->elevations,
                                      numberOfVertices ) );

        if ( ok ) /* Compute then write x and y rectilinear coordinates: */
            {
            int    count; /* Index on rectilinear coordinates. */
            float* coordinate = rectilinearCoordinates; /* Pointer to each. */
            double value = xOrigin; /* Begin with x's. */

            for ( count = 0; count < numberOfColumns; ++count )
                {
                *coordinate++ = ( float ) value;
                value += xDelta;
                }

            value = yOrigin; /* Now do y's: */

            for ( count = 0; count < numberOfRows; ++count )
                {
                *coordinate++ = ( float ) value;
                value += yDelta;
                }

            ok = AND2 ( ok, writeFloats ( file, rectilinearCoordinates,
                                          numberOfRectilinearCoordinates ) );
            }

        FREE ( rectilinearCoordinates );
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeDXTopography - Write topography to a DX file.
 * INPUTS:  File* file                   The file to write to.
 *          const Topography* topography The topography to output.
 * OUTPUTS: File* file                   The updated file structure.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:
 *****************************************************************************/

static int writeDXTopography ( File* file, const Topography* topography )
    {
    int ok = 1;

    PRE2 ( isValidFile ( file ), isValidTopography ( topography ) )

    if ( ok )
        {
        const int numberOfRows     = topography->dimensions[ LAT ];
        const int numberOfColumns  = topography->dimensions[ LON ];
        const int numberOfVertices = numberOfRows * numberOfColumns;
        const double xOrigin       = topography->corners[ LOWER ][ LON ];
        const double yOrigin       = topography->corners[ LOWER ][ LAT ];

        const double xDelta        = ( topography->corners[ UPPER ][ LON ] -
                                       topography->corners[ LOWER ][ LON ] )
                                     / numberOfColumns;

        const double yDelta        = ( topography->corners[ UPPER ][ LAT ] -
                                       topography->corners[ LOWER ][ LAT ] )
                                     / numberOfRows;

        const char* const format   = "msb ieee"; /* "msb ieee" or "ascii". */

        ok = AND2 ( ok, writeString ( file, "# DX field file\n" ) );

        ok = AND2 ( ok, writeString ( file, "#\n# Unprojected topography data:\n" ) );

        ok = AND2 ( ok, writeString ( file, "#   2D regular grid (in degrees "
                                      "longitude, latitude)\n" ) );

        ok = AND2 ( ok, writeString ( file, "#   elevation data (in meters)\n" ) );

        ok = AND2 ( ok, writeString ( file, "#\nobject \"grid-positions\" class"
                                      " gridpositions counts %d %d\n",
                                      numberOfColumns, numberOfRows ) );

        ok = AND2 ( ok, writeString ( file, "  origin %g %g\n", xOrigin, yOrigin ) );

        ok = AND2 ( ok, writeString ( file, "  delta  %g 0\n", xDelta ) );
        ok = AND2 ( ok, writeString ( file, "  delta  0 %g\n", yDelta ) );

        ok = AND2 ( ok, writeString ( file, "#\nobject \"elevation-data\""
                                      " class array type float rank 0"
                                      " items %d %s data follows\n",
                                      numberOfVertices, format ) );

        /*
         * Write interleaved data section since the DX Import Module does not
         * allow data offsets to be used with "!" commands.
         */

        if ( ok )
            {
            /*
             * Elevations must be transposed from data[ row ][ column ] to
             * data[ column ][ row ] as DX expects.
             */

            float* transposedElevations = NEW ( float, numberOfVertices );

            ok = transposedElevations != 0;

            if ( transposedElevations )
                {
                transposeData2 ( topography->elevations, numberOfRows, numberOfColumns,
                                 transposedElevations );

                if ( strcmp ( format, "ascii" ) == 0 )
                    {
                    int vertex;

                    for ( vertex = 0; AND2 ( ok, vertex < numberOfVertices ); ++vertex )
                        ok = writeString ( file, "%g\n", transposedElevations[ vertex ] );
                    }
                else
                    {
                    ok = AND2 ( ok, writeFloats ( file, transposedElevations,
                                                  numberOfVertices ) );
                    }

                FREE ( transposedElevations );
                }
            }

        ok = AND2 ( ok, writeString ( file, "#\nobject \"grid-connections\""
                                      " class gridconnections "
                                      "counts %d %d\n",
                                      numberOfColumns, numberOfRows ) );

        ok = AND2 ( ok, writeString ( file, "#\nobject \"topography\" "
                                      "class field\n" ) );

        ok = AND2 ( ok, writeString ( file, "  component \"positions\"  "
                                      " \"grid-positions\"\n" ) );

        ok = AND2 ( ok, writeString ( file, "  component \"connections\" "
                                      " \"grid-connections\"\n" ) );

        ok = AND2 ( ok, writeString ( file, "  component \"data\" "
                                      " \"elevation-data\"\n" ) );

        ok = AND2 ( ok, writeString ( file, "  attribute \"grid-units\" string"
                                      " \"degrees\"\n" ) );

        ok = AND2 ( ok, writeString ( file, "  attribute \"units\" string"
                                      " \"m\"\n" ) );

        ok = AND2 ( ok, writeString ( file, "#\nend\n" ) );

        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: writeMcIDASTopography - Write topography to a McIDAS file.
 * INPUTS:  File* file                    The file to write to.
 *          const Topography* topography  The topography data to output.
 * OUTPUTS: File* file                    The updated file structure.
 * RETURNS: int 1 if unsuccessful, else 0.
 * NOTES:   Write the data as a binary 3D grid file.
 *****************************************************************************/

static int writeMcIDASTopography ( File* file, const Topography* topography )
    {
    int ok;

    PRE2 ( isValidFile ( file ), isValidTopography ( topography ) )

    ok = writeHeader ( file, topography ); /* Write the McIDAS header. */

    if ( ok )
        {
        const int numberOfRows    = topography->dimensions[ LAT ];
        const int numberOfColumns = topography->dimensions[ LON ];

        const int numberOfVertices = numberOfRows * numberOfColumns;

        short* encodedData = NEW ( short, numberOfVertices );

        ok = encodedData != 0;

        if ( encodedData ) /* Encode and write the elevation data: */
            {
            encodeElevations ( numberOfRows, numberOfColumns, topography->elevations,
                               encodedData );

            ok = writeShorts2 ( file, encodedData, numberOfVertices );

            FREE ( encodedData );
            }
        }

    return ok;
    }


/******************************************************************************
 * PURPOSE: range - Compute the range of an array of data.
 * INPUTS:  const float data[]          The data to check.
 *          size_t      numberOfValues  The number of values in data[].
 * OUTPUTS: float* minimum              The minimum value in data[].
 *          float* maximum              The maximum value in data[].
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void range ( const float data[], size_t numberOfValues,
                    float* minimum, float* maximum )
    {
    float theMinimum, theMaximum;

    PRE4 ( data, numberOfValues, minimum, maximum )

    theMinimum = theMaximum = data[ 0 ];

    while ( --numberOfValues )
        {
        const float currentValue = *data++;

        if      ( currentValue < theMinimum ) theMinimum = currentValue;
        else if ( currentValue > theMaximum ) theMaximum = currentValue;
        }

    *minimum = theMinimum;
    *maximum = theMaximum;

    POST ( *minimum <= *maximum )
    }


/******************************************************************************
 * PURPOSE: transposeData2 - Transpose data from [ row ][ column ] to
 *          data[ column ][ row ].
 * INPUTS:  float* data            The data in [ row ][ column ] form.
 *          size_t numberOfRows    The number of rows    of data.
 *          size_t numberOfColumns The number of columns of data.
 * OUTPUTS: float* transposedData  The data in [ column ][ row ].
 * RETURNS: None
 * NOTES:
 *****************************************************************************/

static void transposeData2 ( const float* data, size_t numberOfRows,
                             size_t numberOfColumns, float* transposedData )
    {
    size_t row, column;

    PRE4 ( data, transposedData, numberOfRows, numberOfColumns )

    for (   row    = 0;    row < numberOfRows;    ++row    )
        for ( column = 0; column < numberOfColumns; ++column )
            transposedData[ OFFSET2 ( column, row, numberOfRows ) ] = *data++;
    }


