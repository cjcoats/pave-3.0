#ifndef MAPPROJECTIONSINFO_H
#define MAPPROJECTIONSINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PURPOSE: MapProjectionsInfo.h - Declare types and functions for getting
 *          map projections information useful for building a user interface.
 * NOTES:   Required source-code control string:
 *          "@(#)MapProjectionsInfo.h	2.2 /env/proj/archive/edss/src/pave/pave_drawmap/SCCS/s.MapProjectionsInfo.h 11/09/99 14:03:08"
 * HISTORY: 10/95, Mark Bolstad, EPA/MMTSI, Created.
 *****************************************************************************/

/*=============================== INCLUDES ==================================*/

#include "MapProjections.h" /* NUMBER_OF_PARAMETERS,NUMBER_OF_ELL_PARAMETERS.*/

/*=============================== MACROS ===================================*/

#define MAX_ALTERNATIVES  3
#define MAX_PARAMETERS   10

/*================================ TYPES ====================================*/

typedef struct
{
  const char* name;
  const char* projName;
  double      initVal;
  double      extrema[2];
} ParameterInfo;

typedef struct 
{
  const char*   name;
  const char*   projName;
  int           numAlternatives;
  int           enumMapping[MAX_ALTERNATIVES][NUMBER_OF_PARAMETERS];
  ParameterInfo parameters[MAX_ALTERNATIVES][MAX_PARAMETERS];
} ProjectionInfo;

typedef struct
{
  const char* name;
  const char* projName;
  int         visibility[NUMBER_OF_ELL_PARAMETERS];
  double      initVal[NUMBER_OF_ELL_PARAMETERS];
} EllipseInfo;

/*=============================== FUNCTIONS =================================*/

extern int           numAlternatives( int id );

extern const char*   getMapProjectionParamTag( int proj, int param,
                                               int alternative );

extern const char*   getEllipseParamTag( int id );
extern double        getEllipseParamIniter( int ellipse, int parameter );

extern const char*   getMapProjectionName( int id );
extern const char*   getMapProjectionProjName( int id );
extern const char*   getEllipseName( int id );
extern const char*   getEllipseProjName( int id );
extern const int*    getMapProjectionVisibility( int id, int alternative );

extern double        getMapProjectionParamIniter( int proj, int param,
                                                  int alternative );

extern const double* getParameterExtrema( int proj, int param, int alternative);

#ifdef __cplusplus
}
#endif

#endif /* MAPPROJECTIONSINFO_H */


