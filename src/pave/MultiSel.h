#ifndef MULTISEL_H
#define MULTISEL_H

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/SelectioB.h>

class MultiSelect {
 public:
  MultiSelect(Widget, int, int, void *, void *);
  //  virtual ~MultiSelect();      	// Destructor
  static void multiselectOkCB(Widget, XtPointer, XtPointer);
  static void multiselectApplyCB(Widget, XtPointer, XtPointer);
  void multiselectOk();
  void multiselectApply();
  static void multiselect1CB(Widget, XtPointer, XtPointer);
  static void multiselect2CB(Widget, XtPointer, XtPointer);
  static void multiselect3CB(Widget, XtPointer, XtPointer);
  void multiselect(int n);

  void showMultiSelection();

 private:
  int nSelection_;
  int currentSelection_;
  int type_;
  char **name_;
  Widget msdialog_;
  void *dwnd_; // this will be a pointer to the DriverWnd object that 
               // instantiated this object
  void *twnd_; // this will be a pointer to the TileWnd object (or NULL)


};

#endif
