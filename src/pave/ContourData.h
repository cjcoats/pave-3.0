#include <X11/Xlib.h>
#include "contour.h"
class CONTOUR_DATA {
 public:
  CONTOUR_DATA(void);
  int n_levels;
  int cntr_thickness;
  int labels_on;
  float *cntr_lvls;
  Cntr_line **cntr_list;
  GC gc;
};
