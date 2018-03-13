#include "LinkedList.h"
#include "ContourData.h"
#include "Vector2d.h"

enum plt_type {
	UNDETERMINED_PLOT = 0,		/* undetermined data format type */
	TILE_PLOT = 1,
	VECTOR_PLOT,
	OBS_PLOT,
	CONTOUR_PLOT,
	OBSVECTOR_PLOT,
	SCATTER_PLOT
	};


class PLOT_DATA: public linkedList {
 public:
   int plot_type;
   int *jstep;
   float *coord_x;
   float *coord_y;
   char **stnid;
   union {
     VIS_DATA *vdata;
     VECTOR2D_DATA *vect2d;
   };
   union {
     int *xxx;
     CONTOUR_DATA *cntr_data;
   };
 };
