#include "vtkKWApplication.h"
#include "vtkKWLabel.h"
#include "vtkKWThumbWheel.h"
#include "vtkKWWindow.h"

#include "vtkKWWidgetsTourExample.h"

class vtkKWThumbWheelItem : public KWWidgetsTourItem
{
public:
  virtual int GetType();
  virtual void Create(vtkKWWidget *parent, vtkKWWindow *);
};

void vtkKWThumbWheelItem::Create(vtkKWWidget *parent, vtkKWWindow *)
{
  vtkKWApplication *app = parent->GetApplication();

  // -----------------------------------------------------------------------

  // Create a thumbwheel

  vtkKWThumbWheel *thumbwheel1 = vtkKWThumbWheel::New();
  thumbwheel1->SetParent(parent);
  thumbwheel1->Create();
  thumbwheel1->SetLength(150);
  thumbwheel1->DisplayEntryOn();
  thumbwheel1->DisplayLabelOn();
  thumbwheel1->DisplayEntryAndLabelOnTopOff();
  thumbwheel1->GetLabel()->SetText("A thumbwheel:");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    thumbwheel1->GetWidgetName());

  // -----------------------------------------------------------------------

  // Create another thumbwheel, but put the label and entry on top

  vtkKWThumbWheel *thumbwheel2 = vtkKWThumbWheel::New();
  thumbwheel2->SetParent(parent);
  thumbwheel2->Create();
  thumbwheel2->ClampMinimumValueOn();
  thumbwheel2->ClampMaximumValueOn();
  thumbwheel2->ClampResolutionOn();
  thumbwheel2->SetRange(-0.5, 1.0);
  thumbwheel2->SetResolution(0.75);
  thumbwheel2->SetValue(-0.5);
  thumbwheel2->SetLinearThreshold(0);
  thumbwheel2->SetLength(275);
  thumbwheel2->SetSizeOfNotches(thumbwheel2->GetSizeOfNotches() * 3);
  thumbwheel2->DisplayEntryAndLabelOnTopOn();
  thumbwheel2->DisplayLabelOn();
  thumbwheel2->DisplayEntryOn();
  thumbwheel2->GetLabel()->SetText("A clamped thumbwheel:");
  thumbwheel2->SetBalloonHelpString(
    "This time, the label and entry are on top, we clamp the range and "
    "resolution, and bigger notches");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    thumbwheel2->GetWidgetName());

  // -----------------------------------------------------------------------

  // Create another thumbwheel, popup mode

  vtkKWThumbWheel *thumbwheel3 = vtkKWThumbWheel::New();
  thumbwheel3->SetParent(parent);
  thumbwheel3->PopupModeOn();
  thumbwheel3->Create();
  thumbwheel3->SetRange(0.0, 100.0);
  thumbwheel3->SetResolution(1.0);
  thumbwheel3->DisplayEntryOn();
  thumbwheel3->DisplayLabelOn();
  thumbwheel3->GetLabel()->SetText("A popup thumbwheel:");

  app->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    thumbwheel3->GetWidgetName());

  thumbwheel1->Delete();
  thumbwheel2->Delete();
  thumbwheel3->Delete();
}

int vtkKWThumbWheelItem::GetType()
{
  return KWWidgetsTourItem::TypeComposite;
}

KWWidgetsTourItem* vtkKWThumbWheelEntryPoint()
{
  return new vtkKWThumbWheelItem();
}
