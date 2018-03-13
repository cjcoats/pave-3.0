#ifndef TOPOGRAPHYFILE_H
#define TOPOGRAPHYFILE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PURPOSE: TopographyFile.h - Declares functions for reading and writing
 *          unprojected McIDAS topography files.
 *
 * NOTES:
 *  Source-code control string:
 *  "$Id: TopographyFile.h 84 2018-03-12 21:26:53Z coats $"
 *
 * HISTORY: 
 *  Created 12/1996, Todd Plessel, EPA/MMTSI
 *
 *  Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 *****************************************************************************/

/*================================ TYPES ====================================*/

/* For writeTopographyFile(): */

enum { FAST_GRID_FORMAT, AVS_FIELD_FORMAT, DX_FIELD_FORMAT, MCIDAS_TOPO_FORMAT,
       NUMBER_OF_TOPOGRAPHY_FORMATS };

typedef struct
{
  float*  elevations;    /* 2D Array of elevations in meters above/below sea.*/
                         /* elevation[ LAT ][ LON ].                         */
  int     dimensions[2]; /* dimensions[ LAT | LON ].                         */
  double  corners[2][2]; /* corners[ LOWER | UPPER ][ LAT | LON ].           */
} Topography;

/*=============================== FUNCTIONS =================================*/

extern int readTopographyFile(  const char* topographyFileName,
                                Topography* topography );

extern int writeTopographyFile( const char* topographyFileName,
                                const Topography* topography,
                                int format );

extern int sizeOfTopographyFile( const char* topographyFileName,
                                 int* numberOfRows, int* numberOfColumns );

extern int allocateTopography( int numberOfRows, int numberOfColumns,
                               Topography* topography );

extern void deallocateTopography( Topography* topography );

extern int isValidTopography( const Topography* topography );

#ifdef __cplusplus
}
#endif

#endif /* TOPOGRAPHYFILE_H */

