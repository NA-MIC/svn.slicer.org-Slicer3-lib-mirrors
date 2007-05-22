/*=========================================================================

  Module:    vtkKWWidgetSet.h,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWWidgetSet - an abstract set of a specific type of vtkKWWidget
// .SECTION Description
// This class is a composite widget that can be used to conveniently 
// allocate, store and layout a set of widgets of the same type. 
// Each widget is created, removed or queried based on a unique ID provided
// by the user (ids are *not* handled by the class since it is likely that 
// they will be defined as enum's or #define by the user for easier retrieval).
// Widgets are packed (gridded) in the order they were added to the set.
//
// IMPORTANT: this is an abstract superclass, it can not be used as-is.
// A subclass of vtkKWWidgetSet is tailored for a *specific* type of widget
// (see vtkKWPushButtonSet, which is a set of vtkKWPushButton); it can not be
// used to store or layout a set of heterogeneous widgets. 
//
// This class acts as a container of widgets of the same type; the main point
// of this container is not to provide a layout mechanism, but to make it a
// little more convenient to create and allocate a whole bunch of widgets
// of the same type without explicitly declaring a pointer to each and 
// everyone of them. Say, if you need ten buttons, and you do not want to
// make 10 calls to vtkKWPushButton::New(), 10 calls to 
// vtkKWPushButton::Create(), and 10 calls to vtkKWPushButton::Delete(): just
// create one instance of a vtkKWPushButtonSet, then call
// vtkKWPushButtonSet::AddWidget to automatically allocate and create an
// instance of a vtkKWPushButton. This widget will be packed in a grid 
// fashion with the other widgets in the set, as a bonus. Widgets that are
// added, allocated and created that way are automatically de-allocated when
// the vtkKWWidgetSet instance is deleted. 
//
// Be aware that the pretty much all subclasses of vtkKWWidgetSet are 
// generated automatically out of the vtkKWWidgetSetSubclass template located
// in the Templates directory. Therefore, even though the source code for
// those vtkKWWidgetSet subclasses does not exist in the KWWidgets repository,
// they are still generated automatically and documented in the API online;
// check the vtkKWWidgetSet API online for its subclasses, as well as the
// \subpage kwwidgets_autogenerated_page page. 
// Classes related to the same template can be found in the 
// \ref kwwidgets_autogenerated_widget_set_group section.
// Subclasses need to implement AllocateAndCreateWidget
// .SECTION See Also
// vtkKWCheckButtonSet vtkKWComboBoxSet vtkKWEntrySet vtkKWLabelSet vtkKWLabelWithLabelSet vtkKWPushButtonSet vtkKWScaleSet vtkKWScaleWithEntrySet vtkKWSpinBoxSet

#ifndef __vtkKWWidgetSet_h
#define __vtkKWWidgetSet_h

#include "vtkKWCompositeWidget.h"

class vtkKWWidget;
class vtkKWWidgetSetInternals;

class KWWidgets_EXPORT vtkKWWidgetSet : public vtkKWCompositeWidget
{
public:
  vtkTypeRevisionMacro(vtkKWWidgetSet,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the number of widget in the set.
  virtual int GetNumberOfWidgets();

  // Description:
  // Retrieve the id of the n-th widget (-1 if not found)
  virtual int GetIdOfNthWidget(int rank);

  // Description:
  // Check if a widget is in the set, given its unique id.
  // Return 1 if exists, 0 otherwise.
  virtual int HasWidget(int id);

  // Description:
  // Retrieve the position in the set the widget was inserted at.
  // Return pos if exists, -1 otherwise
  virtual int GetWidgetPosition(int id);

  // Description:
  // Hide/show a widget, given its unique id.
  // Get the number of visible widget in the set.
  virtual void HideWidget(int id);
  virtual void ShowWidget(int id);
  virtual int GetWidgetVisibility(int id);
  virtual void SetWidgetVisibility(int id, int flag);
  virtual int GetNumberOfVisibleWidgets();
  virtual int GetIdOfNthVisibleWidget(int rank);

  // Description:
  // Delete all widgets.
  virtual void DeleteAllWidgets();

  // Description:
  // Set the packing direction to be horizontal (default is vertical).
  virtual void SetPackHorizontally(int);
  vtkBooleanMacro(PackHorizontally, int);
  vtkGetMacro(PackHorizontally, int);

  // Description:
  // Set the maximum number of widgets that will be packed in the packing
  // direction (i.e. horizontally or vertically).
  // For example, if set to 3 and the packing direction is horizontal, 
  // the layout ends up as 3 columns of widgets.
  // The default is 0, i.e. all widgets are packed along the same direction. 
  virtual void SetMaximumNumberOfWidgetsInPackingDirection(int);
  vtkGetMacro(MaximumNumberOfWidgetsInPackingDirection, int);

  // Description:
  // Set/Get the padding that will be applied around each widget.
  // (default to 0).
  virtual void SetWidgetsPadX(int);
  vtkGetMacro(WidgetsPadX, int);
  virtual void SetWidgetsPadY(int);
  vtkGetMacro(WidgetsPadY, int);

  // Description:
  // Set/Get the internal padding that will be left around each widget.
  // This space is added inside the widget border.
  // (default to 0).
  virtual void SetWidgetsInternalPadX(int);
  vtkGetMacro(WidgetsInternalPadX, int);
  virtual void SetWidgetsInternalPadY(int);
  vtkGetMacro(WidgetsInternalPadY, int);

  // Description:
  // Set the layout to allow the widgets to expand automatically
  // within the set.
  virtual void SetExpandWidgets(int);
  vtkBooleanMacro(ExpandWidgets, int);
  vtkGetMacro(ExpandWidgets, int);

  // Description:
  // Set/Get if the column/row layout should be uniform (enforce same size).
  virtual void SetUniformColumns(int);
  vtkBooleanMacro(UniformColumns, int);
  vtkGetMacro(UniformColumns, int);
  virtual void SetUniformRows(int);
  vtkBooleanMacro(UniformRows, int);
  vtkGetMacro(UniformRows, int);

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

protected:
  vtkKWWidgetSet();
  ~vtkKWWidgetSet();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  int PackHorizontally;
  int MaximumNumberOfWidgetsInPackingDirection;
  int WidgetsPadX;
  int WidgetsPadY;
  int WidgetsInternalPadX;
  int WidgetsInternalPadY;
  int ExpandWidgets;
  int UniformColumns;
  int UniformRows;

  // Description:
  // To be implemented by superclasses.
  // Allocate and create a widget of the right type.
  // Return a pointer to the superclass though.
  virtual vtkKWWidget* AllocateAndCreateWidget() = 0;

  // BTX
  // PIMPL Encapsulation for STL containers

  vtkKWWidgetSetInternals *Internals;
  //ETX

  // Helper methods

  virtual vtkKWWidget* GetWidgetInternal(int id);
  virtual vtkKWWidget* InsertWidgetInternal(int id, int pos);

  // Description:
  // Pack the widgets
  virtual void Pack();

private:
  vtkKWWidgetSet(const vtkKWWidgetSet&); // Not implemented
  void operator=(const vtkKWWidgetSet&); // Not implemented
};

#endif
