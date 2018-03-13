#ifndef COMBODATA_H
#define COMBODATA_H
/****************************************************************************
 *
 * Project Title: Environmental Decision Support System
 *
 *         File: @(#)ComboData.h	2.1
 *     Pathname: /env/proj/archive/edss/src/pave/pave_include/SCCS/s.ComboData.h
 * Last updated: 12/15/97 16:25:54
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

//////////////////////////////////////////////////////////////////////
//	ComboData.h
//	K. Eng Pua
//	Copyright (C)
//	Jan 28, 1995
//	
//////////////////////////////////////////////////////////////////////
//
//   ComboData Class
//
//   ComboData                                    Concrete
//        1. Contains the data items for 2D
//           plots
//
//////////////////////////////////////////////////////////////////////
        // unistd.h messes up compilation sometimes because of "link"
#ifndef __UNISTD_H__
#define __UNISTD_H__
#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


class ComboData {
  public:
	ComboData() { };
	typedef char str20[20];

	float val_xmin() const { return val_xmin_; }
	float val_xmax() const { return val_xmax_; }
	float val_ymin() const { return val_ymin_; }
	float val_ymax() const { return val_ymax_; }
	float val_x(int ix) const { return val_x_[ix]; }
	float val_y(int iy) const { return val_y_[iy]; }

	int y_data_num() const { return y_data_num_; }
	int obs_num() const { return obs_num_; }

	int  exists_y_label_list() const { return (y_label_list_ != NULL); }
	char *y_label_list(int i) const { return y_label_list_[i]; }
	char *val_xs(int i) const { return val_xs_[i]; }
	char *title1() const { return title1_; }
	char *title2() const { return title2_; }
	char *title3() const { return title3_; }
	char *xtitle() const { return xtitle_; }
	char *ytitle() const { return ytitle_; }
	char *plotformatX() const { return (char *)plotformatX_; }
	char *plotformatY() const { return (char *)plotformatY_; }

	virtual void initialize();

/*
	void set_val_x(float *fptr) { if (val_x_) delete val_x_; val_x_ = fptr; } 
	void set_val_y(float *fptr) { if (val_y_) delete val_y_; val_y_ = fptr; } 
*/

	void setDataArray(float *x, float *y, int y_num, int obs_num);
	void setTitles(char *t1, char *t2, char *t3, char *xti, char *yti);

  protected:
	char	*drawtype_;

	float	*val_x_;		// Array for x-axis, used by xy,area,scatter,err 
	float	*val_y_;		// Array for y-axis
	str20	*val_xs_;		// String array for x-axis, used by bar,pie,stackedbar,boxwhisker
	str20	*y_label_list_;

	int	y_data_num_;		// Number of y data (columns);
	int	obs_num_;		// Number of observations (rows);

	float	val_xmin_, val_xmax_;	// Min and max values of x array
	float	val_ymin_, val_ymax_;	// Mmin and max values of y array

	char	*title1_;		// Main title of plot
	char	*title2_;		// Subtitle of plot
	char	*title3_;		// Dataset name subtitle of plot
	char	*xtitle_;		// Title on x-axis
	char	*ytitle_;		// Title on y-axis

	int	*plotstyle_;		// Used by XY-Plot. Values: POINT_ONLY, LINE_ONLY, or POINT_LINE
	char	*plottype_;		// may want to use drawttype_

	int	offset_left_;		// Offsets of the plot
	int	offset_right_;
	int	offset_top_;
	int	offset_bottom_;
	int	plot_left_;		// Offsets of the drawing area
	int	plot_top_;
	str20	plotformatX_;		// Format of X-tic labels
	str20	plotformatY_;		// Format of Y-tic labels
};

#endif


