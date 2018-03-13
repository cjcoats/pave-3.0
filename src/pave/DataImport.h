#ifndef DATAIMPORT_H
#define DATAIMPORT_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PURPOSE: DataImport.h - Declares functions for reading and writing Models-3
 *          data files. This library is essentially a supplement to Carlie's
 *          libm3io to provide some convenience routines and work-around some
 *          of its deficiencies.
 * NOTES:   Required source-code control string:
 *          "@(#)DataImport.h	2.2 /env/proj/archive/edss/src/pave/pave_drawmap/SCCS/s.DataImport.h 11/09/99 14:03:05"
 * HISTORY: 04/96, Todd Plessel, EPA/MMTSI, Created.
 *****************************************************************************/

/*================================ INCLUDES =================================*/

#include <stdio.h>   /* For FILE.                       */
#include <stddef.h>  /* For size_t.                     */

#include "iodecl3.h" /* For IOAPI_Bdesc3, IOAPI_Cdesc3. */

#include "File.h"    /* For File.                       */

/*================================= TYPES ===================================*/

typedef struct
{
  char logicalFileName[ 257 ];
  char fileName[ 257 ];
  IOAPI_Bdesc3 bdesc;
  IOAPI_Cdesc3 cdesc;
} M3IOFile;

/* For subsets: */

enum { TIMESTEP, VARIABLE, LAYER, ROW, COLUMN, NUMBER_OF_DATA_DIMENSIONS };
enum { FIRST, LAST };
enum { COUNT };
enum { MINIMUM, MAXIMUM };

/* For corners[][] in createM3IOIddataFile(): */

#ifndef LOWER_UPPER_LAT_LON
#define LOWER_UPPER_LAT_LON
enum { LOWER, UPPER };
enum { LAT,   LON   };
#endif

/* IDDATA3 structure read/written at each timestep: */

typedef struct
{
  int*   count;      /* [1, NROWS]: # of stations reporting at this timestep.*/
  int*   ids;        /* ids[ count ]. Id of each station reporting.          */
  float* fvariables; /* ivariables[ NVARS ][ NLAYS ][ count ].               */
  int*   ivariables; /* fvariables[ NVARS ][ NLAYS ][ count ].               */
  int*   buffer;     /* Allocated buffer where the above pointers reference. */
} Iddata;


/*=============================== FUNCTIONS =================================*/

extern int readM3IOFileDescription( const char* fileName, M3IOFile* file );

extern int readM3IOFileDescriptionFromFile( File* file, M3IOFile* m3ioFile );

extern int writeM3IOFileDescriptionToFile( File* file,
                                           const M3IOFile* m3ioFile );

extern int initializeM3IO( void ); /* Calls libm3io's init3c(). */

extern int finalizeM3IO( void );   /* Calls libm3io's shut3c(). */

extern int isInitializedM3IO( void ); 

extern int openM3IOFileForReading( const char* fileName, M3IOFile* file );

extern int openM3IOFileForWriting( const char* fileName, M3IOFile* file );

extern int closeM3IOFile( M3IOFile* file );

extern int readM3IOVariable( const M3IOFile* file, int timestep, int variable,
                             void* data );

extern int readM3IOVariables( const M3IOFile* file, int timestep, void* data );

extern int writeM3IOVariable( const M3IOFile* file,
                              int timestep,
                              int variable,
                              const void* data );

extern int writeM3IOVariables( const M3IOFile* file, int timestep,
                               const void* data );

extern int readM3IOSubset( const M3IOFile* file,
                           const int subset[][ 2 ],
                           const char* variableNames[],
                           void* data );

extern int isValidM3IOFile( const M3IOFile* file );

extern int isValidIOAPI_Bdesc3( const IOAPI_Bdesc3* bdesc );

extern int isValidIOAPI_Cdesc3( const IOAPI_Cdesc3* cdesc, int nvars );

extern int checkM3IOFile( const M3IOFile* file );

extern int checkIOAPI_Bdesc3( const IOAPI_Bdesc3* bdesc );

extern int checkIOAPI_Cdesc3( const IOAPI_Cdesc3* cdesc, int nvars );

extern int isM3IOCrossGrid( const M3IOFile* file );

extern size_t sizeOfM3IOSubset( const int subset[][ 2 ]);

extern int isValidSubsetOfM3IOFile( const M3IOFile* file,
                                    const int subset[][ 2 ],
                                    const char* variableNames[] );

extern void adjustM3IOSubset( const M3IOFile* file, int subset[][ 2 ] );

extern void createSubsetOfM3IOFile( const M3IOFile* inputFile,
                                    const int subset[][ 2 ],
                                    const char* variableNames[],
                                    M3IOFile* outputFile );

extern int numberOfM3IOVariable( const M3IOFile* file,
                                 const char* variableName );

extern const char* nameOfM3IOVariable( const M3IOFile* file,
                                       int variableNumber );

extern void getM3IODateTime( int  startDate,    int  startTime,
                             int  timestepSize, int  timestep,
                             int* theDate,      int* theTime );

extern void getM3IODateTimeString( const M3IOFile* file, int timestep,
                                   size_t bufferSize, char* buffer );

extern int isValidDate( int yyyyddd );

extern int isValidTime( int hhmmss );

extern int isValidTimestepSize( int hhmmss );

extern void compressString( const char* src, size_t n, char* dst );

extern void computeM3IOGridZ( const IOAPI_Bdesc3* bdesc, int computeZAtLevels, 
                              double z[] );

extern double pressureAtSigmaLevel( double sigmaLevel, double pressureAtTop );

extern double heightAtPressure( double pressure );

extern void printM3IOFileDescription( const M3IOFile* file );

extern int computeM3IORange( const M3IOFile* file,
                             const int subset[][ 2 ],
                             const char* variableNames[],
                             float* range );

extern void expandString( const char* src, size_t size, char* dst );

extern int createM3IOIddataFile( const char* fileName,
                                 int numberOfTimesteps,
                                 int numberOfStations,
                                 int numberOfVariables,
                                 int startingDate,
                                 int startingTime,
                                 int timestepSize,
                                 const int   variableTypes[],
                                 const char* variableNames[],
                                 const char* variableUnits[],
                                 const char* variableDescriptions[],
                                 const float corners[ 2 ][ 2 ],
                                 M3IOFile* file );

extern int readM3IOIddata( const M3IOFile* file, int timestep,
                           Iddata* iddata );

extern int writeM3IOIddata( const M3IOFile* file, int timestep,
                            const Iddata* iddata );

extern int isValidIddata( const Iddata* iddata );

extern Iddata* allocateIddata( int numberOfStations, int numberOfVariables );

extern void deallocateIddata( Iddata* iddata );

extern int isValidCorners( const float corners[ 2 ][ 2 ] );

extern int isValidLatLon( float latitude, float longitude );


#ifdef __cplusplus
}
#endif

#endif /* DATAIMPORT_H */
