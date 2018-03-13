#ifndef DRIVERWINDOW_H
#define DRIVERWINDOW_H

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

//////////////////////////////////////////////////////////////////////////////
//
// DriverWnd.h 
// K. Eng Pua
// Dec 14, 1994
//
//////////////////////////////////////////////////////////////////////////////
//
// --------------------
// Modification History
// --------------------
//
// ---  ----    ----
// Who  When    What
// ---  ----    ----
// SRT  950529  Added DriverWnd::processArgs() method; added int argc, 
//	        char *argv[], and char *errString args to DriverWnd 
// SRT  950530  added graphics_plot_type_menu_button_ and
//        	domain_edit_button_ // SRT
// SRT  950604  added export_DX_button_ and export_IX_button_, and
//		callbacks for these export functions
// SRT  950607  Added linked list variables
// SRT  950612  Added editDataSetLayerRangeCB(), edit_data_set_layer_range_cb(),
//		editDataSetROICB(), edit_data_set_roi_cb(), editFormulaLayerRangeCB(),
//		edit_formula_layer_range_cb(), editFormulaROICB(), and editFormulaROICB()  
// SRT  951218  Added logic for minimum time between animation frames
// SRT  951227  Added logic to handle tile/mesh plots of YZT and XZT planes
// SRT  960517  Added hooks to netCDF exporting
// SRT  960826  Added documentation menu
//
//////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#define __CSTRING__
#endif /* __GNUG__ SRT gets around annoying inline problem with gnu .h files */

        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

        /* bald messes this up in some header file */
#ifdef NOclockid_t
#define clockid_t int
#endif /* #ifdef NOclockid_t */

#include "Config.h"
#include "AppInit.h"
#include "TileWnd.h"
#include "Menus.h"
#include "DrawWnd.h"
#include "CaseServer.h"
#include "FormulaServer.h"
#include "SpeciesServer.h"
#include "DomainWnd.h"
#include "BtsData.h"
#include "BusConnect.h"
#include "OptionManager.h"
#include "ComboData.h"
#include "ComboWnd.h"
#include "Shell.h"
#include "BarWnd.h"
#include "LinkedList.h"
#include "DataSet.h"
#include "Formula.h"
#include "Level.h"
#include "Domain.h"
#include "ExportServer.h"
#include "MapServer.h"
#include "LocalFileBrowser.h"
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/Scale.h>
#include <Xm/ScrolledW.h>
#include <assert.h>
#include <string.h>
#include "bts.h" // added 950911 SRT
#include "MultiSel.h"
#include "StringPair.h"

extern char *get_user_name(void); // in Main.cc, added 951106 SRT

extern "C" {  // Might want to get rid of this later
#include "vis_proto.h"

   int avg_plot(char *filename,
        int species_no, int first_level, int last_level, char *message);
}


	// different drawing types for the same exact kinds of data
#define PAVE_TIME_LINE 0
#define PAVE_TIME_BAR  1

#define PAVE_XYT_TILE  0
#define PAVE_XYT_MESH  1


class DriverWnd : public Shell, public Menus, public BusConnect {
   public:
        DriverWnd(AppInit *app, char *name, char *historyfile, 
		  int argc, char *argv[], char *errorMsg);

	virtual ~DriverWnd();

	void registerSynWnd(TileWnd *tile);

	void closeWnd(long windowid);

	void raiseWnd(long windowid);

	int animateWnd(long windowid, int timestep);

	void synchronizeWindows();

	void readHistoryFile();

	void writeHistoryFile();

	void parseMessage(char *msg); 

	int createScatterPlot(char *formula1, char *formula2);

	int createMultiTimePlot(int nformulas, char **formulas); 

	int createTileVectorPlot(char *formula1,char *formula2,char *formula3);

        int createTileVectorObsPlot(char *formula1, char *formula2,
				    char *formula3, char *formula4);

	int processMultiSelect(int, int, char **, void *);
	void access_species_cb(); // SRT 960411 moved from private

	Widget createEditSelectionDialog();
	Widget getParent() {return parent_;}
	void putFormula2SelectionDialog(Widget, int);
	PLOT_DATA *getPlotData(char *, int);
	void getObsTimeSeries();
	void createMultiFormulaSelectionDialog(int nitems, int type, void *);


   protected:
	XtWorkProcId    work_proc_id_;
	void createUI ( Widget );

   private:

	Config		cfg_;

	int		width_;

	int		height_;

	int		startingUpThisPuppy_;

	int		frameDelayInTenthsOfSeconds_;

	char		inputTitleString_[256];
	char		subTitle1String_[256];
	char		subTitle2String_[256];
	int		contourRange_;
	float		minCut_;
	float		maxCut_;


	DomainWnd	*dom_obj_;
	ComboData	combo_obj_;
	linkedList      datasetList_;
	linkedList      formulaList_;
	linkedList      domainList_;
	linkedList      levelList_;
	linkedList      *obsIdListP_;

	VIS_DATA	info;
	enum		{ MAX_STR_LEN = 1024 };
	enum		{ MAX_WINDOWS = 20 };
        int             max_num_hours_; // the max nhours for all plots 
	char		strbuf_[MAX_STR_LEN];
	char		str512_[512];

	int		num_tilewnd_;
	TileWnd		**tilewnd_list_;
	TileWnd		*mostRecentTile_;
	int 		tileSliceType_;

	void updateStatus(char *msg);

	char *getURL(char *name);
	void showURL(char *url);

	void loopWindows(int loopType); // added 970102 SRT

	static void create_vectorplotCB(Widget, XtPointer, XtPointer);
	void        create_vectorplot_cb();
	static void create_tilevectorplotCB(Widget, XtPointer, XtPointer);
	void        create_tilevectorplot_cb();
	static void create_scatterplotCB(Widget, XtPointer, XtPointer);
	void        create_scatterplot_cb();
	void        createMultiFormulaSelectionDialog(int nitems, int type);

	static void overlayCB(Widget, XtPointer, XtPointer);
	void        overlay_cb();
	static void userGuideCB(Widget, XtPointer, XtPointer);
	static void faqCB(Widget, XtPointer, XtPointer);
	static void ioapiCB(Widget, XtPointer, XtPointer);

	static void exitCB(Widget, XtPointer, XtPointer);

	static void animate_scaleCB(Widget, XtPointer, XtPointer);
	void        animate_scale_cb(int);

	static void delay_scaleCB(Widget, XtPointer, XtPointer);
	void        delay_scale_cb(int);

	static void animateCB(Widget, XtPointer, XtPointer);
	void        animate_cb();

	static void stopCB(Widget, XtPointer, XtPointer);
	void        stop_cb();

	static void closeCB(Widget, XtPointer, XtPointer);
	void        close_cb();

	static void delay_closeCB(Widget, XtPointer, XtPointer);
	void        delay_close_cb();

	void        createSynchronizeDialog( Widget parent );
	static Boolean synTrigger(XtPointer clientData); 

	static void minFrameTime_dialogCB(Widget, XtPointer, XtPointer);
	void minFrameTime_dialog_cb(void);
	void createMinFrameTime_dialog( Widget parent);

	void invalidateAllFormulasData(void);

	Widget		parent_;
	Widget		selection_;
	Widget		info_window_;
	Widget		sw_; // added 950804 SRT
	Widget		synchronize_dialog_;
	Widget		minFrameTime_dialog_;
	Widget		animate_scale_;
	Widget		delay_scale_;
	Widget		animate_;
	Widget		stop_;
	Widget		close_;
	Widget		delay_close_;
	Widget		status_;

	Widget		colortile_menu_button_;
	Widget		linegraph_menu_button_;
	Widget		graphics_plot_type_menu_button_; // SRT
	Widget		domain_edit_button_; // SRT
	Widget		export_DX_button_; // SRT
	Widget		export_IX_button_; // SRT
	Widget		graphics_default_slices_menu_button; // SRT
	Widget		graphics_integrateOverX_menu_button; // SRT
	Widget		graphics_integrateOverY_menu_button; // SRT
	Widget		graphics_integrateOverZ_menu_button; // SRT
	Widget		graphics_timeSeries_menu_button; // SRT
	int		animate_frame_;
	int		curr_animate_;
	int		num_to_loop_;

	AppInit		*app_;

	char		*history_file_;
	void		*case_; // changed from CaseServer * SRT 960411
	FormulaServer	*formula_;
	SpeciesServer	*species_;
	OptionManager	*option_;
	char		*tmpstrptr_;

	ExportServer	*exportAVS_UI_;
	ExportServer	*exportTabbed_UI_;
	ExportServer	*exportnetCDF_UI_;

	static void modify_formulaCB(Widget, XtPointer clientData, XtPointer callData);
	void        modify_formula_cb();

	static void load_formulaCB(Widget, XtPointer clientData, XtPointer callData);
	void        load_formula_cb();

	static void save_formulaCB(Widget, XtPointer clientData, XtPointer callData);
	void        save_formula_cb();

	static void access_speciesCB(Widget, XtPointer clientData, XtPointer callData);

        static void modify_caseCB(Widget, XtPointer clientData, XtPointer callData);
        void        modify_case_cb();

        static void load_caseCB(Widget, XtPointer clientData, XtPointer callData);
        void        load_case_cb();

        static void load_configMenuCB(Widget, XtPointer clientData, XtPointer callData);
        void        load_config_Menucb();
        static void load_configCB(void *object, char *fname);
        void        load_config_cb(char *fname);

        static void save_caseCB(Widget, XtPointer clientData, XtPointer callData);
        void        save_case_cb();

        static void editDataSetStepsCB(Widget, XtPointer clientData, XtPointer callData);
        void        edit_data_set_steps_cb();

        static void editDataSetLayerRangeCB(Widget, XtPointer clientData, XtPointer callData);
        void        edit_data_set_layer_range_cb();

        static void editDataSetROICB(Widget, XtPointer clientData, XtPointer callData);
        void        edit_data_set_roi_cb();

	static void editFormulaStepsCB(Widget, XtPointer clientData, XtPointer callData);
	void        edit_formula_steps_cb();

        static void editFormulaLayerRangeCB(Widget, XtPointer clientData, XtPointer callData);
        void        edit_formula_layer_range_cb();

        static void editFormulaROICB(Widget, XtPointer clientData, XtPointer callData);
        void        edit_formula_roi_cb();

	static void grp_control_synCB(Widget, XtPointer, XtPointer);
	void        grp_control_syn_cb();

	static void grp_control_deleteCB(Widget, XtPointer, XtPointer);
	void        grp_control_delete_cb();

	static void grp_type_XcrossCB(Widget, XtPointer, XtPointer);
	void        grp_type_Xcross_cb();

	static void grp_type_YcrossCB(Widget, XtPointer, XtPointer);
	void        grp_type_Ycross_cb();

	static void grp_type_ZcrossCB(Widget, XtPointer, XtPointer);
	void        grp_type_Zcross_cb();

	static void grp_type_XintegCB(Widget, XtPointer, XtPointer);
	void        grp_type_Xinteg_cb();

	static void grp_type_YintegCB(Widget, XtPointer, XtPointer);
	void        grp_type_Yinteg_cb();

	static void grp_type_ZintegCB(Widget, XtPointer, XtPointer);
	void        grp_type_Zinteg_cb();

	static void grp_type_TseriesCB(Widget, XtPointer, XtPointer);
	void        grp_type_Tseries_cb();

	static void grp_optionCB(Widget, XtPointer, XtPointer);
	void        grp_option_cb();

	static void grp_plot_colortileCB(Widget, XtPointer, XtPointer);
	int        grp_plot_colortile_cb();

	static void grp_plot_1hour_tileCB(Widget, XtPointer, XtPointer);
	static void grp_plot_8hour_tileCB(Widget, XtPointer, XtPointer);
	static void grp_plot_nhour_tileCB(Widget, XtPointer, XtPointer);
	int         grp_plot_nhour_tile_cb(int nhour);

	static void grp_plot_nlayerAvg_tileCB(Widget, XtPointer clientData, XtPointer);
	static void grp_plot_meshCB(Widget, XtPointer, XtPointer);
	int         grp_plot_mesh_cb();

	int grp_plot_XYT(int ptype);
	int grp_plot_nhour_avg(int, int);
	int grp_plot_nlayer_avg(int);

	static void grp_plot_linegraphCB(Widget, XtPointer, XtPointer);
	int         grp_plot_linegraph_cb();

	static void grp_plot_barCB(Widget, XtPointer, XtPointer);
	int         grp_plot_bar_cb();

	int         grp_plot_time_series(int ttype); // ttype either PAVE_TIME_LINE or PAVE_TIME_BAR

// SRT 960509	static void domainCB(Widget, XtPointer, XtPointer);
// SRT 960509	void domain_cb();

	static void avsExportCB(Widget, XtPointer, XtPointer); // SRT
	void        avs_export_cb(); // SRT

	static void ixExportCB(Widget, XtPointer, XtPointer); // SRT
	void        ix_export_cb(); // SRT

	static void dxExportCB(Widget, XtPointer, XtPointer); // SRT
	void        dx_export_cb(); // SRT

	static void tabbedASCIIExportCB(Widget, XtPointer, XtPointer); // SRT
	void        tabbed_ascii_export_cb(); // SRT

	static void netCDFExportCB(Widget, XtPointer, XtPointer); // SRT
	void        netCDF_export_cb(); // SRT

	void        showLevelUI(int nlev);

	void        initInfo();

        void        busCallback(char *msg);

	char	*name_; // added 950804 SRT
	int	bus_hrMin_;
	int	bus_hrMax_;
	
	int processArgs(int argc, char *argv[], char *estring);

	int permute_vdata_to_XYT(VIS_DATA *vdata, char *estring);

	int get_num_synch_steps(void);

	int calcWidthHeight(int *w, int *h, VIS_DATA *vdata);

	VIS_DATA *get_VIS_DATA_struct(Formula *f, char *estring, int slice_type); // grab a copy of this formula's data; added 960920 SRT for memory management

	void free_up_every_formulas_data(void); // added 960920 SRT for memory management

	LocalFileBrowser *LoadConfigBrowser_;

	void	displaySingleNumberFormula(char *valueString, Formula *f);

        void  createComp(int, int, int, int, char *, char *);
	void  createComp(int, int, int, char *, char *, char *);

	void tzOffset(int);

	void tzSet(char *, char *);
	void export2file(char *, int);
	void animatedGIF(char *);
	void multiVarNcf(char *flist, char *vlist, char *fname);
        void changeTitleFontSize(int size);
        void changeSubTitleFontSize(int size);
	void addAlias(char *);
	void removeAlias(char *aliasname);
	void loadAliasFile(char *);
	void writeAliasFile(char *);
	void addObsIdTable(char *);

};


void	TileWndHasBeenClosed_CB(void *dwnd);

void	pave_usage(void);

void	verify_copyright_file(void);

void	print_copyright_file(void);

char 	*get_copyright_filename(void);

#endif
