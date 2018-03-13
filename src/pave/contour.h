

typedef struct _cntr_line_struct
    {
    int color;
    int npoints;
    float *x;
    float *y;
    struct _cntr_line_struct *next;
    } Cntr_line ;
