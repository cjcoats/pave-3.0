#ifndef MAPFILE_H
#define MAPFILE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PURPOSE: MapFile.h - Declares functions for readig and writing McIDAS map
 *          border line files.
 *
 * NOTES:
 *  Source-code control string:
 *  "$Id: MapFile.h 84 2018-03-12 21:26:53Z coats $"
 *
 * HISTORY: 
 *  Created  06/1995, Todd Plessel, EPA/MMTSI
 *
 *  Version 2/2018 by Carlie J. Coats, Jr. for PAVE-2.4
 *****************************************************************************/

/*================================ TYPES ====================================*/

/* For writeMapFile(): */

enum { ARCGRAPH_FORMAT, AVS_GEOM_FORMAT, DX_FORMAT, NUMBER_OF_MAP_FORMATS };

typedef struct
{
  float*  vertices; /* 2D Array of map vertices (in degrees lat-lon).         */
  int*    starts;   /* Array of indices of the start of each polyline.        */
  int*    lengths;  /* Array of lengths of each (# of vertices per) polyline. */
  int     vertexCount;   /* The total number of vertices in vertices[].       */
  int     polylineCount; /* The total number of polylines in the map.         */
                         /* starts[polylineCount], lengths[polylineCount].    */
  double  corners[2][2]; /* corners[ LOWER | UPPER ][ LAT | LON ].            */
} MapLines;

/*=============================== FUNCTIONS =================================*/

extern int readMapFile(  const char* mapFileName, MapLines* mapLines );

extern int writeMapFile( const char* mapFileName, const MapLines* mapLines,
                         const float* z, int format );

extern int sizeOfMapFile( const char* mapFileName,
                          int* numberOfPolylines,
                          int* numberOfVertices );

extern int allocateMapLines( int numberOfPolylines,
                             int numberOfVertices,
                             MapLines* mapLines );

extern void deallocateMapLines( MapLines* mapLines );

extern int isValidMapLines( const MapLines* mapLines );

extern void computeMapLinesCorners( MapLines* mapLines );

#ifdef __cplusplus
}
#endif

#endif /* MAPFILE_H */

