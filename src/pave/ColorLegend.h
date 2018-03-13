#ifndef COLORLEGEND_H
#define COLORLEGEND_H

/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)ColorLegend.h	2.2
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.ColorLegend.h
 * Last updated: 01/05/99 14:41:54
 *
 * Made available by MCNC and the Carolina Environmental Program of UNC Chapel
 * Hill under terms of the GNU Public License.  See gpl.txt for more details.
 *
 * See file COPYRIGHT for license information on this and supporting software.
 *
 * Carolina Environmental Program
 * University of North Carolina at Chapel Hill
 * 137 E. Franklin St.
 * Chapel Hill, NC 27599-6116
 *
 * See http://www.cep.unc.edu, http://www.cmascenter.org, http://bugz.unc.edu
 *
 ****************************************************************************/

////////////////////////////////////////////////////
// File:	ColorLegend.h
// Author:	Kathy Pearson and Kah Eng Pua
// Date:	Mar 2, 1995
////////////////////////////////////////////////////
//
//   ColorLegend Class                            Concrete
//   ColorLegend
//        1. Creates color legend dialog box
//        2. Allows customization of 3 colormaps
//        3. Supports setting of the number of        
//           color bins and the color of each
//           bin
//        4. Supports selection of different
//           label formats in the color legend
//
////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950630  Added disable map drawing radio button
// SRT  950707  Added smooth plot drawing radio button
// SRT  951115	Added setContourRange() routine
// SRT  951212  Added draw grid lines radio button & scale vectors button
//
//////////////////////////////////////////////////////////////////////////////


#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/Scale.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/ArrowB.h>
#include <Xm/Separator.h>
#include <assert.h>
#include <stdlib.h>

#include "ColorChooser.h"
#include "DrawScale.h"
#include "Config.h"
#include "bts.h"

class ColorLegend {
   public:
	ColorLegend();
	virtual ~ColorLegend();
	void initColorLegendObject();
        void createColorLegendDialog();
        void initColorLegend(Widget canvas, Drawable drw, GC gc, DrawScale *s,  float val_min, float val_max, char *unitString, char **title, char **subtitle1, char **subtitle2);

	Widget		legend_dialog_; 

	virtual void resize() = 0;
	virtual void refreshColor() = 0;
	virtual void toggleMapDrawing() = 0;
	virtual void toggleSmoothPlots() = 0;
	virtual void toggleGridLines() = 0;
	virtual void toggleScaleVectors() = 0;

	int		legend_map_off_;
	int		saveMPEGControls_;

	int 		saveMPEGControls() { return saveMPEGControls_; }

	int		smooth_plots_on_;
	int		grid_lines_on_;
	int		scale_vectors_on_;
	int		fill_arrowheads_;

	int		setContourRange(float minCut, float maxCut);

	enum { NEWTON_COLORMAP, JET_COLORMAP, GREY_COLORMAP };

	Widget		legend_invert_cmap_;
	int		legend_cmap_;
	float		val_min_;
	float		val_max_;
	int		legend_ntile_;
	int		legend_nlabel_;
	int		legend_nskip_;
	Display		*cl_dpy_;

	int setConfig (Config *cfgp);
	float		vector_scale_;
	Widget		vector_scale_choice_;

   protected:
        GC		*color_gc_table_;

	void drawColorLegend(int offset, int width); 
        int  colorIndex(float);
	char		legend_format_[20];

	Config 		*cfgp_;

   private:   

	Pixmap pixmap_;

	ColorChooser	*colorChooser_;

	struct ValueRange {
		int value, min, max;
	};

// SRT 960910	float		legend_min_;
// SRT 960910	float		legend_max_;

	Widget		cl_canvas_;
	Drawable	cl_drw_;
	GC		cl_gc_;

	DrawScale	*cl_s_;


	Widget		bin_;
	Widget		legend_tiles_;
	Widget		legend_labels_;
	Widget		legend_skip_;
	Widget		legend_disable_map_;
	Widget		legend_save_mpegControls_;
	Widget		legend_smooth_plot_;
	Widget		legend_grid_lines_;
	Widget		legend_scale_vectors_;
	Widget		legend_fill_arrowheads_;
	Widget		legend_range_choice_;
	Widget		legend_format_choice_;
	Widget		legend_title_choice_;
	Widget		legend_subtitle1_choice_;
	Widget		legend_subtitle2_choice_;
	Widget		legend_color_bin_;
	Widget		legend_min_choice_;
	Widget		legend_max_choice_;
	Widget		wrc_;
	Widget		editableColor_;

	Widget		unit_title_choice_;

	int		legend_range_;
	int		legend_invert_;
	int		legend_bincolor_;
	char		**legend_title_;
	char		**legend_subtitle1_;
	char		**legend_subtitle2_;
	char		unitString_[128];


	float		*data_table_;

	void reset_legend_minmax(); 
	void legend_panel_redraw(); 
	void set_legend_color_widget(); 
	void setup_color_gc_table(); 
	int  get_named_pixel(char *colorname); 
	void setup_data_table(); 
	int  get_ramp_pixel(int icolor); 


	static void colorSelectedCB (int red, int green, int blue, void * );
	void        colorSelected_cb (int red, int green, int blue);

	static void legend_tilesCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_tiles_cb(int); 

	static void legend_labelsCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_labels_cb(int); 

	static void legend_skipCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_skip_cb(int); 

	static void legend_invertCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_invert_cb(int); 

	static void legend_disable_mapCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_disable_map_cb(int); 

	static void legend_save_mpegControlsCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_save_mpegControls_cb(int); 

	static void legend_smooth_plotCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_smooth_plot_cb(int); 

	static void legend_grid_linesCB(Widget, XtPointer clientData, XtPointer callData);
	void        legend_grid_lines_cb(int valu);

	static void legend_scale_vectorsCB(Widget, XtPointer clientData, XtPointer callData);
	void        legend_scale_vectors_cb(int valu);

	static void legend_fill_arrowheadsCB(Widget, XtPointer clientData, XtPointer callData);
	void        legend_fill_arrowheads_cb(int valu);

	static void legend_cmap_newtonCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_cmap_newton_cb(); 

	static void legend_cmap_jetCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_cmap_jet_cb(); 

	static void legend_cmap_greyCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_cmap_grey_cb(); 

	static void legend_range_upCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_range_up_cb(); 

	static void legend_range_downCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_range_down_cb(); 

	void        update_legend_range_color(int i); 

	static void closeLegendDialogCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        close_legend_dialog_cb(); 

	static void legend_colorbinCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_colorbin_cb(Widget w); 

	static void legend_editableColorCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_editableColor_cb(); 

	static void legend_minCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_min_cb(Widget w); 

	static void legend_maxCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_max_cb(Widget w); 

	static void legend_formatCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_format_cb(char *); 

	static void legend_titleCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_title_cb(Widget w); 

	static void legend_subTitle1CB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_subTitle1_cb(Widget w); 

	static void legend_subTitle2CB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_subTitle2_cb(Widget w); 

	static void legend_unitCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        legend_unit_cb(Widget w); 

	static void vector_scale_typeinCB(Widget w, XtPointer clientData, XtPointer callData); 
	void        vector_scale_typein_cb(Widget w); 

	static void timezone_dialogCB(Widget w, XtPointer clientData, XtPointer);

}; 


#endif

