#ifndef TILEWINDOW_H
#define TILEWINDOW_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: %W%
 *     Pathname: %P%
 * Last updated: %G% %U%
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

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// File:	TileWnd.h 
// Author:	K. Eng Pua
// Date:	Dec 3, 1994
///////////////////////////////////////////////////////////////////////////////
//
//   TileWnd Class
//
//   TileWnd : DrawWnd, RubberBand,               Concrete
//                Menus
//  
//        1. Creates a top-level window           AppInit
//        2. Supports a user interface that       DrawWnd
//           includes an application-specific
//           drawing area widget
//        3. Creates a pulldown menu at the top   Menus
//           of the window
//        4. Reads or copies the information      ReadVisData
//           associated with the modeling domain
//           and modeling data
//        5. Initializes the draw scale for       DrawScale
//           the plot
//        6. Draws the tile plot and superimpose
//           the state and county maps on top
//        7. Animates the tile plot
//        8. Supports zooming and data probing    RubberBand
//
//////////////////////////////////////////////////////////////////////////////
//
//  Modification history:
//
//  950616 SRT Included "bts.h" for INDEX macro
//  951130 SRT Added stuff for vector drawing
//  951212 SRT Added drawGridLines(), toggleGridLines(), & toggleScaleVectors()
//  960522 SRT Began adding code to incorporate Suresh Balu's animation export
//  960529 SRT Added callbacks for map setting routines
//  960530 SRT Added logic for saving RGB, XWD, and GIF Images
//
//////////////////////////////////////////////////////////////////////////////


#include <sys/types.h>
#include <unistd.h>
 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/ScrolledW.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "ReadVisData.h"
#include "DrawScale.h"
#include "DrawWnd.h"
#include "Menus.h"
#include "AppInit.h"
#include "RubberBand.h"
#include "ColorLegend.h"
#include "bts.h"
#include "LocalFileBrowser.h"
#include "MapServer.h"
#include "utils.h"
#include "PlotData.h"

static char *default_colornames[] = {
   "Blue",
   "Cyan",
   "LimeGreen",
   "Green",
   "GreenYellow",
   "Yellow",
   "Orange",
   "Red"
};


extern "C" {
#include "visDataClient.h"
}


class TileWnd : public DrawWnd, public Menus, public RubberBand, public ColorLegend {

   public:

	TileWnd(Config *cfgp, 
		void *dwnd,
		AppInit *app, 
		char *name, 
		BusData *ibd, 
		VIS_DATA *vdata,
                char *drawtype,
		char *title2,  // Species/Fomula String 
		char *title3,  // Dataset (file) name(s) string
                Dimension width, Dimension height, 
		int *frameDelayInTenthsOfSeconds,
		int exit_button_on = 1,  // 1 means Exit button sensitive. Otherwise insensitive
                char **colornames = default_colornames, int numcolornames = 8); 

        TileWnd(Config *cfgp, 
		void *dwnd,
		AppInit *app, 
		char *name, 
		ReadVisData *vis,
                char *drawtype,
		char *title2,
		char *title3,  // Dataset (file) name(s) string
                Dimension width, Dimension height,
		int *frameDelayInTenthsOfSeconds,
                int exit_button_on = 1,  // 1 means Exit button sensitive. Otherwise insensitive
                char **colornames = default_colornames, int numcolornames = 8);

	TileWnd(Config *cfgp, 
		void *dwnd,
		AppInit *app, 
		char *name, 
		BusData *ibd, 
		VIS_DATA *vdata,
                VIS_DATA *uwind,
                VIS_DATA *vwind,
                char *drawtype,
		char *title2,  // Species/Fomula String 
		char *title3,  // Dataset (file) name(s) string
                Dimension width, Dimension height, 
		int *frameDelayInTenthsOfSeconds,
		int exit_button_on = 1,  // 1 means Exit button sensitive. Otherwise insensitive
                char **colornames = default_colornames, int numcolornames = 8); 

	TileWnd(Config *cfgp, 
		void *dwnd,
		AppInit *app, 
		char *name, 
		BusData *ibd, 
                VIS_DATA *uwind,
                VIS_DATA *vwind,
                VIS_DATA *uwind_obs,
                VIS_DATA *vwind_obs,
                char *drawtype,
		char *title2,  // Species/Fomula String 
		char *title3,  // Dataset (file) name(s) string
                Dimension width, Dimension height, 
		int *frameDelayInTenthsOfSeconds,
		int exit_button_on = 1,  // 1 means Exit button sensitive. Otherwise insensitive
                char **colornames = default_colornames, int numcolornames = 8); 

	virtual ~TileWnd(); // 960918 added SRT for memory management

	virtual void drawTile(int);
	virtual void drawBlockTile(int);
	virtual void drawSmoothTile(int);
	virtual void drawMap();
	virtual void drawVectors(int);
	virtual void drawVect(float n1, float n2, float n3, float n4);
	virtual void drawVect(float n1, float n2, float n3, float n4, int headsize);

	void animateTile();
	void synchronizeAnimate(int animate); 

	int isManage() const { return manage_; }

        int get_nhours(void);
        int get_sdate(void);
        int get_stime(void);
        int get_tstep(void);
        int get_edate(void);
        int get_etime(void);
        int get_offset(void);
        int get_skip(void);
        void set_offset(int);
        void set_skip(int);

	void toggleMapDrawing(void);
	void toggleSmoothPlots(void);
        void toggleGridLines(void);
        void toggleScaleVectors(void);

	int dumpImage(  char *imagetype,// for now only supports
					// "PNM", "XWD", "GIF", "RGB", "PNG"
                        char *fname,    // image file name
                        char *estring); // error messages will go here

	static void configureCB(Widget, XtPointer, XtPointer);
	void configure_cb();
	static void configureobsCB(Widget, XtPointer, XtPointer);
	void configureobs_cb();
	static void closeObsDialogCB(Widget, XtPointer, XtPointer);
	void close_obs_dialog_cb();
	static void closeCntrDialogCB(Widget, XtPointer, XtPointer);
	void close_cntr_dialog_cb();
	static void obs_sizeCB(Widget, XtPointer, XtPointer);
	void obs_size_cb(int);
	static void obs_thickCB(Widget, XtPointer, XtPointer);
	void obs_thick_cb(int);
	static void ncontoursCB(Widget, XtPointer, XtPointer);
	void ncontours_cb(int);
	static void cnfg_cntrCB(Widget, XtPointer, XtPointer);
	void cnfg_cntr_cb(int);
	static void cntr_thickCB(Widget, XtPointer, XtPointer);
	void cntr_thick_cb(int);
	static void configurecntrCB(Widget, XtPointer, XtPointer);
	void configurecntr_cb();
	static void cntr_levelCB(Widget, XtPointer, XtPointer);
	void cntr_level_cb(int, float);
	static void fontSliderMovedCB(Widget, XtPointer, XtPointer);
	void fontSliderMoved_cb();

	long getWidgetId(); // SRT 970326
	Window getWindowId(); // ALT 110899
        void animateTileCore(int);
	static void overlay_selectedCB(Widget, XtPointer, XtPointer);
	void overlay_selected_cb(Widget);
	void overlay_create(char *);
	void set_overlay_mode(int);
	void overlay_ts(void);
	void overlay_ts(int x1, int x2, int y1, int y2);
	void saveAnimation_cb(char *fname, char *ftype);

	enum { MAX_PATH_LEN = 256 };

	ReadVisData	*vis_;
	VIS_DATA 	*uwind_;
	VIS_DATA	*vwind_;
	VIS_DATA 	*uwind_obs_;
	VIS_DATA 	*vwind_obs_;
        DrawScale 	s;

        Widget          canvas_;

        float           val_xytdelta_;

        Pixmap          mappix1_;
        Pixmap          mappix2_;

        Widget          animate_dialog_;
        Widget          animate_scale_;
        Widget          animate_;
        Widget          stop_;

        Widget          titlefont_dialog_;
        Widget          titlefont_scale_;
        Widget          subtitlefont_scale_;

        int             titlefont_size_;
        int             subtitlefont_size_;
	int             tSliderVal_;
        int             sSliderVal_;
	
        Widget          obs_dialog_;
        Widget          cntr_dialog_;

        // SRT inherit from RubberBand 951017  Widget          close_;

        int             animate_frame_;
        int             curr_animate_;
        int             selected_step_;

        int             offset_;
        int             skip_;

        void createAnimateDialog( Widget parent );
        void createTitleFontDialog( Widget parent );

        void createObsDialog();
        void createCntrDialog();

        virtual void createUI ( Widget );

        virtual void drawDetail();

//      void animateTileCore(int);

        void writeProbeFile(float x1, float x2, float y1, float y2);
        void writeProbeObsFile(int x1, int x2, int y1, int y2);

	void refreshColor();



	void adjustZoomDialogPosition(void); // added 950913 SRT to
					     // override RubberBand.cc's

	void setTZ(int);
	void setTZname(char *, char *);
	void resetTZname(void);
	void setTitleFontSize(int size);
	void setSubTitleFontSize(int size);
   private:

	int		manage_;
        XtWorkProcId    work_proc_id_;

	int	exit_button_on_;
	int	vectors_on_;
	int	tiles_on_;
        int     overlay_;
        int     draw_dist_counties_;
        int     mapChoices_;
	int     overlay_mode_;
	linkedList *plotDataListP_;

        int     obs_size_; // ALT we need to remove this from here!!!
        int     obs_thick_; // ALT we need to remove this from here!!!
	PLOT_DATA *current_pdata_obs_config_;
	PLOT_DATA *current_pdata_cntr_config_;
	void	initTileWnd();
	void    drawOverlays(int);

	static void overlay_obsCB(Widget, XtPointer, XtPointer);
	static void overlay_vectorobsCB(Widget, XtPointer, XtPointer);
	static void overlay_cntrCB(Widget, XtPointer, XtPointer);
	void overlay_cb(int);
	static void timeSeriesCB(Widget, XtPointer clientData, XtPointer);
	static void timeSeriesObsCB(Widget, XtPointer clientData, XtPointer);
	static void maxPointTimeSeriesCB(Widget, XtPointer clientData, XtPointer);
	static void minPointTimeSeriesCB(Widget, XtPointer clientData, XtPointer);
	void minOrMaxPointTimeSeriesCB(int minOrMax);

	static void resizeCB(Widget, XtPointer, XtPointer);
	void resize();

	static void redisplayCB(Widget, XtPointer, XtPointer);
	void redisplay(XExposeEvent *event);

	static void probeCB(Widget, XtPointer, XtPointer);
	void probe_cb();

	static void probeobsCB(Widget, XtPointer, XtPointer);
	void probeobs_cb();

	static void zoomCB(Widget, XtPointer, XtPointer);
	void zoom_cb();

	static void animate_dialogCB(Widget, XtPointer, XtPointer);
	void animate_dialog_cb();

	static void titlefont_dialogCB(Widget, XtPointer, XtPointer);
	void titlefont_dialog_cb();
	static void titlefont_okCB(Widget, XtPointer, XtPointer);
	void titlefont_ok_cb();
	static void titlefont_cancelCB(Widget, XtPointer, XtPointer);
	void titlefont_cancel_cb();

	static void zoom_dialogCB(Widget, XtPointer, XtPointer);
	void zoom_dialog_cb();

        static void zoom_scaleCB(Widget, XtPointer, XtPointer);
        void zoom_scale_cb(int);

	static void exitCB(Widget, XtPointer, XtPointer);

	void timezone_dialog_cb();

	int zone2GMT(char *);

public:
	void exit_cb();

private:

	static void animate_scaleCB(Widget, XtPointer, XtPointer);
	void animate_scale_cb(int);

	static void animateCB(Widget, XtPointer, XtPointer);
	void animate_cb();

	static void stopCB(Widget, XtPointer, XtPointer);
	void stop_cb();

	static void closeCB(Widget, XtPointer, XtPointer);
	void close_cb();

	static void saveMPEGMenuCB(Widget, XtPointer, XtPointer);
	void saveMPEGMenu_cb();
	static void saveMPEGCB(void *object, char *fname);

	static void printMenuCB(Widget, XtPointer, XtPointer);
	void printMenu_cb();

	static void savePNGMenuCB(Widget, XtPointer, XtPointer);
	void savePNGMenu_cb();
	static void savePNGCB(void *object, char *fname);
	void savePNG_cb(char *fname);

	static void saveGIFMenuCB(Widget, XtPointer, XtPointer);
	void saveGIFMenu_cb();
	static void saveGIFCB(void *object, char *fname);
	void saveGIF_cb(char *fname);

	static void saveAnimatedGIFMenuCB(Widget, XtPointer, XtPointer);
	void saveAnimatedGIFMenu_cb();
	static void saveAnimatedGIFCB(void *object, char *fname);


	static void saveConfigMenuCB(Widget, XtPointer clientData, XtPointer );
	void        saveConfigMenu_cb();
	static void saveConfigCB(void *object, char *fname);
	void        saveConfig_cb(char *fname);

	static void savePSMenuCB(Widget, XtPointer, XtPointer);
	void savePSMenu_cb();

	static void savePSCB(void *object, char *fname);
	void savePS_cb(char *fname);

	static void saveRGBMenuCB(Widget, XtPointer, XtPointer);
	void saveRGBMenu_cb();

	static void saveRGBCB(void *object, char *fname);
	void saveRGB_cb(char *fname);

	static void saveXWDMenuCB(Widget, XtPointer, XtPointer);
	void saveXWDMenu_cb();

	static void saveXWDCB(void *object, char *fname);
	void saveXWD_cb(char *fname);

	void fillCanvasBackground(char *);

	static Boolean animateTileTrigger(XtPointer clientData);

	void drawGridLines(void);

	void fillTheBackgroundWithWhite(void);

	static void OUTLCO_CB(Widget, XtPointer, XtPointer);
	void OUTLCO_cb(int);

	static void OUTLRIVERS3000_CB(Widget, XtPointer, XtPointer);
	void OUTLRIVERS3000_cb();

	static void OUTLROADS3000_CB(Widget, XtPointer, XtPointer);
	void OUTLROADS3000_cb();

	static void OUTLSTATES3000_CB(Widget, XtPointer, XtPointer);
	void OUTLSTATES3000_cb(int);

	static void OUTLUSAM_CB(Widget, XtPointer, XtPointer);
	void OUTLUSAM_cb();

	static void OUTLHRES_CB(Widget, XtPointer, XtPointer);
	void OUTLHRES_cb();

	int compute_this_contour(PLOT_DATA *, int, int, float *);

	int *frameDelayInTenthsOfSecondsP_;

	LocalFileBrowser SaveMPEGBrowser_;

	LocalFileBrowser SaveAnimatedGIFBrowser_;

	LocalFileBrowser SavePNGBrowser_;

	LocalFileBrowser SaveGIFBrowser_;

	LocalFileBrowser SaveConfigBrowser_;

	LocalFileBrowser SaveRGBBrowser_;

	LocalFileBrowser SaveXWDBrowser_;

	LocalFileBrowser SavePSBrowser_;

        Widget createMPEGAnimationMenuButton_;

        Widget createPNGMenuButton_;

        Widget createGIFMenuButton_;

        Widget createRGBMenuButton_;

	Widget printmenuButton_;

	Widget createPSMenuButton_;

        Widget probeobsButton_, configobsButton_, configcntrButton_,
	       configvectobsButton_, tsobsButton_;

	Widget  fileButton_,
		interactButton_,
		controlButton_,
		mapButton_;

        Widget animateButton_;

	void drawMinMax(int t);

	int 	printmenuIndex_, PNGmenuIndex_, GIFmenuIndex_, PSmenuIndex_,
		RGBmenuIndex_, MPEGmenuIndex_, nFileMenuItems_;

	int timezone_;
	char *tzname_;

	void setButtonSensitiviy();

	void timeSeriesProbe(int x1, int x2, int y1, int y2); // override RubberBand.cc's 

	void *dwnd_; // this will be a pointer to the DriverWnd object that 
		     // instantiated this object
	void *getDriverWnd() {return dwnd_;}


};

enum map_line_choices {
   MapDefault,
   MapCounties,
   MapMediumResStates,
   MapHighResStates,
   MapRivers,
   MapRoads,
   MapWorld
};

class TransferObject {
 public:
  TransferObject(void);
  void *obj;
  int lvl;
};

#endif

