#ifndef MAPUTILITIES_H
#define MAPUTILITIES_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PURPOSE: MapUtilities.h - Declares functions for creating projected maps
 *          and grids based on Models-3 data files.
 * NOTES:   Required source-code control string:
 *          "@(#)MapUtilities.h	2.2 /env/proj/archive/edss/src/pave/pave_drawmap/SCCS/s.MapUtilities.h 11/09/99 14:03:09"
 * HISTORY: 05/96, Todd Plessel, EPA/MMTSI, Created.
 *          06/96, Mark Bolstad, EPA/MMTSI, Added lower-level projection code
 *                                          moved from DrawMap Library.
 *****************************************************************************/

/*================================ INCLUDES =================================*/

#include "iodecl3.h"        /* For IOAPI_Bdesc3.                          */

#include "MapFile.h"        /* For MapLines.                              */
#include "MapProjections.h" /* For MapProjection, LOWER, UPPER, LAT, LON. */

/*================================= TYPES ===================================*/

enum { LAT_LON_GRID, PHYSICAL_GRID }; /* gridType: */
enum { X, Y };

/* Relevant parameters from M3IO data file header: */

typedef struct
{
  int    gdtyp;  /* Projection type: LAMGRD3, etc.                           */
  double p_alp;  /* Alpha.                                                   */
  double p_bet;  /* Beta.                                                    */
  double p_gam;  /* Gamma.                                                   */
  double xcent;  /* X-coordinate of center of projection (degrees,except UTM)*/
  double ycent;  /* Y-coordinate of center of projection (degrees,except UTM)*/
  int    nrows;  /* Number of rows    in the grid.                           */
  int    ncols;  /* Number of columns in the grid.                           */
  double xorig;  /* X-coordinate of lower-left corner of grid (meters).      */
  double yorig;  /* Y-coordinate of lower-left corner of grid (meters).      */
  double xcell;  /* X-dimension of grid cell (meters).                       */
  double ycell;  /* Y-dimension of grid cell (meters).                       */
  double corners[ 2 ][ 2 ]; /* [ LOWER | UPPER ][ LAT | LON ] of clip domain.*/
  int    ellipse; /* Ellipse type: CUSTOM_ELLIPSE, WGS_84, etc.              */
  double radius;  /* If ellipse == CUSTOM_ELLIPSE then radius of sphere.     */
} M3IOParameters;

typedef struct
{
  int    gridType;        /* LAT_LON_GRID | PHYSICAL_GRID.                   */
  double center[ 2 ];     /* Projection center[ LAT | LON ] (PHYSICAL_GRID). */
  double origin[ 2 ];     /* Lower-left corner of grid: origin[ LAT | LON ]. */
  double delta[  2 ];     /* Cell dimensions: delta[  LAT | LON ].           */
  int    dimensions[ 2 ]; /* Number of cells: dimensions[ LAT | LON ].       */
} GridInfo;

/*=============================== FUNCTIONS =================================*/

extern MapLines* createProjectedMapLinesClippedToGrid( const char* mapFileName,
                                                       const char* dataFileName,
                                             const M3IOParameters* parameters );

extern int readMapLines( const char* mapFileName, MapLines* mapLines );

extern void printMapLines( const MapLines* mapLines );

extern int areValidM3IOParameters( const M3IOParameters* parameters );

extern int areValidDomainCorners( const double corners[ 2 ][ 2 ] );

extern int isValidEllipse( int ellipse, double radius );

extern int readM3IOParameters( const char* fileName,
                               M3IOParameters* parameters );

extern int createGridInfo( const M3IOParameters* parameters,
                           GridInfo* gridInfo );

extern void createMapProjection( const M3IOParameters* parameters,
                                 MapProjection* mapProjection );

extern int createProjectedGridLines( const GridInfo* gridInfo,
                                     const MapProjection* mapProjection,
                                     MapLines* projectedGridLines );

extern int createProjectedMapLines( const MapProjection* mapProjection,
                                    const MapLines* unprojectedMapLines,
                                    MapLines* projectedMapLines );

extern int createClippedProjectedMapLines( const MapLines* projectedMapLines,
                                           const MapLines* projectedGridLines,
                                           MapLines* clippedProjectedMapLines );

extern void initializeMapProjectionToDefaults( MapProjection* mapProjection,
                                               int ellipse,
                                               double radius,
                                               int projType,
                                               int alternative,
                                               const double corners[ 2 ][ 2 ] );

extern int computeProjectedGridOrigin( const M3IOParameters* parameters,
                                       double* projectedXOrigin,
                                       double* projectedYOrigin );

extern int computeGridCell( const M3IOParameters* parameters,
                            const double projectedPoint[],
                            double gridCell[] );

extern void createIOAPI_Bdesc3( const M3IOParameters* parameters,
                                IOAPI_Bdesc3* bdesc );

extern void createM3IOParameters( const IOAPI_Bdesc3* bdesc,
                                  const double corners[ 2 ][ 2 ],
                                  int ellipse,
                                  double radius,
                                  M3IOParameters* parameters );

extern void projectMap( const MapProjection* mapProjection,
                        const MapLines*      mapLines,
                              MapLines*      projectedMapLines );

extern void projectGrid( const MapProjection* mapProjection,
                         const GridInfo*      gridInfo,
                               MapLines*      projectedGridLines );

extern void clipMapLines( const MapLines* mapLines,
                          const double    corners[ 2 ][ 2 ],
                                MapLines* clippedMapLines );

extern int latLonToGridCell( const GridInfo*      gridInfo,
                             double latitude, double longitude,
                             double* column,  double* row );

extern int metersToGridCell( const GridInfo*      gridInfo,
                             double xPoint,   double yPoint,
                             double* column,  double* row );

extern void sizeOfProjectedMap( const MapLines*  map,
                                const MapProjection* mapProjection,
                                int* numberOfPolylines,
                                int* numberOfVertices );

extern void sizeOfProjectedGrid( const GridInfo*  gridInfo,
                                 const MapProjection* mapProjection,
                                 int* numberOfPolylines,
                                 int* numberOfVertices );

extern void sizeOfClippedMapLines( const MapLines*  mapLines,
                                   const double corners[ 2 ][ 2 ],
                                   int* numberOfPolylines,
                                   int* numberOfVertices );

extern int computeCornerMinMax( const MapLines* map[], size_t n,
                                double corners[ 2 ][ 2 ] );

extern int isValidGridInfo( const GridInfo* gridInfo );

extern int isValidLatitudeLongitude( double latitude, double longitude );


#ifdef __cplusplus
}
#endif

#endif /* MAPUTILITIES_H */
