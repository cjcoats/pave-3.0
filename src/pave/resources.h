
#include <X11/Intrinsic.h>
#define NCOL 254     /* No. of colors limited to 254 here */
#define NDEFCOL 33  /* No. of colors from resource file + 1*/
typedef struct { Dimension     width, height;
		         int           ncol, rescol, mode, symb_mode,
                                         leg_mode, cell_mode;
                 int           lwidth[3],cheight[3],
                               min_height, sad_height, max_height;
                 int           xannincr, yannincr, cannincr,
			       tickdir, ticklen, ticklenl,
                                        cannstart, cannmax;
                 int           xcannincr, ycannincr, min_p_anno;
                 int           xmarg,ymarg, xlen,ylen, xscal, yscal, xdist;
                 int           np_prof;
                 int           gray_min, gray_max;
		         String        filnam,psfil,palfil,symfil,
                               prof_fil,mms_fil,l_form,
                               min_form, sad_form, max_form;
                 String        xannform, yannform, cannform;
                 String        plot_title; 
		         Pixel         bg,fg,col[NCOL+NDEFCOL+3+8];                               
                 int           pat[NDEFCOL];
                 int           linecol[NDEFCOL], linecolor;
                 Font          font1,font2,font3;
                 Boolean       autolev,autopal,legend,
                               autoxgr,autoygr,ratio,cann,
                               posta4,posta4p,posta3,xann,yann,
                               min_anno, sad_anno, max_anno,
			       mono, nc_exact;
                }
		 res_data, *res_data_ptr;

/* reso.col = pixel values:   (NDEFCOL= 33)
           [0] = Foreground (in postscript always black)
	   [1...start_of_symb_colors-1] = colors for fill area contours
           [...start_of_symb_colors + NDEFCOL -2]= symbol_colors
           [start_of_symb_colors + NDEFCOL-1]= Background
	   [...start_of_symb_colors + NDEFCOL+7]= label_colors

Default: 0:        Foreground
         1...NCOL: fill area colors
	 NCOL+1...NCOL+NDEFCOL-1: symbol_colors
	 NCOL+NDEFCOL: Background
	 NCOL+NDEFCOL+1... NCOL+NDEFCOL+8: label_colors
*/
