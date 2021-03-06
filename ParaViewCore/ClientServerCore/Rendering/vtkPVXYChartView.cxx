/*=========================================================================

  Program:   ParaView
  Module:    vtkPVXYChartView.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPVXYChartView.h"

#include "vtkAnnotationLink.h"
#include "vtkAxis.h"
#include "vtkChartLegend.h"
#include "vtkChartParallelCoordinates.h"
#include "vtkChartXY.h"
#include "vtkCommand.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkDoubleArray.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPen.h"
#include "vtkPVPlotTime.h"
#include "vtkStringArray.h"
#include "vtkTextProperty.h"
#include "vtkXYChartRepresentation.h"

#include <string>
#include <vtksys/ios/sstream>

class vtkPVXYChartView::vtkInternals
{
public:
  // Keeps track of custom labels for each of the axis.
  vtkNew<vtkDoubleArray> CustomLabelPositions[4];
  bool UseCustomLabels[4];
  vtkInternals()
    {
    this->UseCustomLabels[0] = this->UseCustomLabels[1]
      = this->UseCustomLabels[2] = this->UseCustomLabels[3] = false;
    }
};


vtkStandardNewMacro(vtkPVXYChartView);

//----------------------------------------------------------------------------
vtkPVXYChartView::vtkPVXYChartView()
{
  this->Internals = new vtkInternals();

  this->Chart = NULL;
  this->InternalTitle = NULL;
  this->PlotTime = vtkPVPlotTime::New();

  // Use the buffer id - performance issues are fixed.
  this->ContextView->GetScene()->SetUseBufferId(true);
  this->ContextView->GetScene()->SetScaleTiles(false);
}

//----------------------------------------------------------------------------
vtkPVXYChartView::~vtkPVXYChartView()
{
  if (this->Chart)
    {
    this->Chart->Delete();
    this->Chart = NULL;
    }
  this->PlotTime->Delete();
  this->PlotTime = NULL;

  this->SetInternalTitle(NULL);

  delete this->Internals;
}

//----------------------------------------------------------------------------
vtkAbstractContextItem* vtkPVXYChartView::GetContextItem()
{
  return this->GetChart();
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetSelection(
  vtkChartRepresentation* repr, vtkSelection* selection)
{
  (void)repr;

  if (this->Chart)
    {
    // we don't support multiple selection for now.
    this->Chart->GetAnnotationLink()->SetCurrentSelection(selection);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetChartType(const char *type)
{
  if (this->Chart)
    {
    this->Chart->Delete();
    this->Chart = NULL;
    }

  // Construct the correct type of chart
  if (strcmp(type, "Line") == 0 || strcmp(type, "Bar") == 0)
    {
    this->Chart = vtkChartXY::New();
    }
  else if (strcmp(type, "ParallelCoordinates") == 0)
    {
    this->Chart = vtkChartParallelCoordinates::New();
    }

  if (this->Chart)
    {
    // Default to empty axis titles
    this->SetAxisTitle(0, "");
    this->SetAxisTitle(1, "");
    this->Chart->AddPlot(this->PlotTime);

    this->Chart->AddObserver(vtkCommand::SelectionChangedEvent,
      this, &vtkPVXYChartView::SelectionChanged);
    this->ContextView->GetScene()->AddItem(this->Chart);

    // setup the annotation link.
    // Unlike vtkScatterPlotMatrix, vtkChart doesn't have valid annotation link
    // setup on creation, so create one.
    if (!this->Chart->GetAnnotationLink())
      {
      vtkNew<vtkAnnotationLink> annLink;
      this->Chart->SetAnnotationLink(annLink.GetPointer());
      }
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetTitle(const char* title)
{
  if (this->Chart)
    {
    std::string tmp(title);
    if (tmp.find("${TIME}") != std::string::npos)
      {
      this->SetInternalTitle(title);
      }
    else
      {
      this->Chart->SetTitle(title);
      this->SetInternalTitle(NULL);
      }
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetTitleFont(const char* family, int pointSize,
                                         bool bold, bool italic)
{
  if (this->Chart)
    {
    this->Chart->GetTitleProperties()->SetFontFamilyAsString(family);
    this->Chart->GetTitleProperties()->SetFontSize(pointSize);
    this->Chart->GetTitleProperties()->SetBold(static_cast<int>(bold));
    this->Chart->GetTitleProperties()->SetItalic(static_cast<int>(italic));
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetTitleColor(double red, double green, double blue)
{
  if (this->Chart)
    {
    this->Chart->GetTitleProperties()->SetColor(red, green, blue);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetTitleAlignment(int alignment)
{
  if (this->Chart)
    {
    this->Chart->GetTitleProperties()->SetJustification(alignment);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetLegendVisibility(int visible)
{
  if (this->Chart)
    {
    this->Chart->SetShowLegend(visible? true : false);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetLegendLocation(int location)
{
  if (this->Chart)
    {
    vtkChartLegend *legend = this->Chart->GetLegend();
    legend->SetInline(location < 4);
    switch(location)
      {
      case 0: // TOP-LEFT
        legend->SetHorizontalAlignment(vtkChartLegend::LEFT);
        legend->SetVerticalAlignment(vtkChartLegend::TOP);
        break;
      case 1: // TOP-RIGHT
        legend->SetHorizontalAlignment(vtkChartLegend::RIGHT);
        legend->SetVerticalAlignment(vtkChartLegend::TOP);
        break;
      case 2: // BOTTOM-RIGHT
        legend->SetHorizontalAlignment(vtkChartLegend::RIGHT);
        legend->SetVerticalAlignment(vtkChartLegend::BOTTOM);
        break;
      case 3: // BOTTOM-LEFT
        legend->SetHorizontalAlignment(vtkChartLegend::LEFT);
        legend->SetVerticalAlignment(vtkChartLegend::BOTTOM);
        break;
      case 4: // LEFT
        legend->SetHorizontalAlignment(vtkChartLegend::LEFT);
        legend->SetVerticalAlignment(vtkChartLegend::CENTER);
        break;
      case 5: // TOP
        legend->SetHorizontalAlignment(vtkChartLegend::CENTER);
        legend->SetVerticalAlignment(vtkChartLegend::TOP);
        break;
      case 6: // RIGHT
        legend->SetHorizontalAlignment(vtkChartLegend::RIGHT);
        legend->SetVerticalAlignment(vtkChartLegend::CENTER);
        break;
      case 7: // BOTTOM
        legend->SetHorizontalAlignment(vtkChartLegend::CENTER);
        legend->SetVerticalAlignment(vtkChartLegend::BOTTOM);
        break;
      }
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisVisibility(int index, bool visible)
{
  if (this->Chart)
    {
    this->Chart->GetAxis(index)->SetVisible(visible);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetGridVisibility(int index, bool visible)
{
  if (this->Chart)
    {
    this->Chart->GetAxis(index)->SetGridVisible(visible);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisColor(int index, double red, double green,
                                         double blue)
{
  if (this->Chart)
    {
    this->Chart->GetAxis(index)->GetPen()->SetColorF(red, green, blue);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetGridColor(int index, double red, double green,
                                         double blue)
{
  if (this->Chart)
    {
    this->Chart->GetAxis(index)->GetGridPen()->SetColorF(red, green, blue);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelVisibility(int index, bool visible)
{
  if (this->Chart)
    {
    this->Chart->GetAxis(index)->SetLabelsVisible(visible);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelFont(int index, const char* family,
                                             int pointSize, bool bold,
                                             bool italic)
{
  if (this->Chart)
    {
    vtkTextProperty *prop = this->Chart->GetAxis(index)->GetLabelProperties();
    prop->SetFontFamilyAsString(family);
    prop->SetFontSize(pointSize);
    prop->SetBold(static_cast<int>(bold));
    prop->SetItalic(static_cast<int>(italic));
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelColor(int index, double red,
                                              double green, double blue)
{
  if (this->Chart)
    {
    this->Chart->GetAxis(index)->GetLabelProperties()->SetColor(red, green,
                                                                blue);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelNotation(int index, int notation)
{
  if (this->Chart)
    {
    this->Chart->GetAxis(index)->SetNotation(notation);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelPrecision(int index, int precision)
{
  if (this->Chart)
    {
    this->Chart->GetAxis(index)->SetPrecision(precision);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisRange(int index, double min, double max)
{
  if (this->Chart)
    {
    vtkAxis* axis = this->Chart->GetAxis(index);
    axis->SetBehavior(vtkAxis::FIXED);
    axis->SetMinimum(min);
    axis->SetMaximum(max);
    this->Chart->RecalculateBounds();
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::UnsetAxisRange(int index)
{
  if (this->Chart)
    {
    vtkAxis* axis = this->Chart->GetAxis(index);
    axis->SetBehavior(vtkAxis::AUTO);

    // we set some random min and max so we can notice them when they get used.
    axis->SetMinimum(0.0);
    axis->SetMaximum(6.66);
    this->Chart->RecalculateBounds();
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLogScale(int index, bool logScale)
{
  if (this->Chart)
    {
    this->Chart->GetAxis(index)->SetLogScale(logScale);
    this->Chart->Update();
    this->Chart->RecalculateBounds();
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisTitle(int index, const char* title)
{
  if (this->Chart && this->Chart->GetAxis(index))
    {
    this->Chart->GetAxis(index)->SetTitle(title);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisTitleFont(int index, const char* family,
                                             int pointSize, bool bold,
                                             bool italic)
{
  if (this->Chart)
    {
    vtkTextProperty *prop = this->Chart->GetAxis(index)->GetTitleProperties();
    prop->SetFontFamilyAsString(family);
    prop->SetFontSize(pointSize);
    prop->SetBold(static_cast<int>(bold));
    prop->SetItalic(static_cast<int>(italic));
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisTitleColor(int index, double red,
                                              double green, double blue)
{
  if (this->Chart)
    {
    this->Chart->GetAxis(index)->GetTitleProperties()->SetColor(red, green,
                                                                blue);
    }
}


//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisUseCustomLabels(int index, bool use_custom_labels)
{
  if (index >= 0 && index < 4)
    {
    this->Internals->UseCustomLabels[index] = use_custom_labels;
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelsNumber(int axis, int n)
{
  if (axis >=0 && axis < 4)
    {
    this->Internals->CustomLabelPositions[axis]->SetNumberOfTuples(n);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabels(int axis, int i, double value)
{
  if (axis >=0 && axis < 4)
    {
    this->Internals->CustomLabelPositions[axis]->SetValue(i, value);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelsLeftNumber(int n)
{
  this->SetAxisLabelsNumber(vtkAxis::LEFT, n);
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelsLeft(int i, double value)
{
  this->SetAxisLabels(vtkAxis::LEFT, i, value);
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelsBottomNumber(int n)
{
  this->SetAxisLabelsNumber(vtkAxis::BOTTOM, n);
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelsBottom(int i, double value)
{
  this->SetAxisLabels(vtkAxis::BOTTOM, i, value);
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelsRightNumber(int n)
{
  this->SetAxisLabelsNumber(vtkAxis::RIGHT, n);
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelsRight(int i, double value)
{
  this->SetAxisLabels(vtkAxis::RIGHT, i, value);
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelsTopNumber(int n)
{
  this->SetAxisLabelsNumber(vtkAxis::TOP, n);
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetAxisLabelsTop(int i, double value)
{
  this->SetAxisLabels(vtkAxis::TOP, i, value);
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetTooltipNotation(int notation)
{
  for(int i = 0; i < this->Chart->GetNumberOfPlots(); i++)
    {
    this->Chart->GetPlot(i)->SetTooltipNotation(notation);
    }
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SetTooltipPrecision(int precision)
{
  for(int i = 0; i < this->Chart->GetNumberOfPlots(); i++)
    {
    this->Chart->GetPlot(i)->SetTooltipPrecision(precision);
    }
}

//----------------------------------------------------------------------------
vtkChart* vtkPVXYChartView::GetChart()
{
  return this->Chart;
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::Render(bool interactive)
{
  if (!this->Chart)
    {
    return;
    }
  if (this->InternalTitle)
    {
    vtksys_ios::ostringstream new_title;
    std::string title(this->InternalTitle);
    size_t pos = title.find("${TIME}");
    if (pos != std::string::npos)
      {
      // The string was found - replace it and set the chart title.
      new_title << title.substr(0, pos)
                << this->GetViewTime()
                << title.substr(pos + strlen("${TIME}"));
      this->Chart->SetTitle(new_title.str().c_str());
      }
    }

  this->PlotTime->SetTime(this->GetViewTime());
  this->PlotTime->SetTimeAxisMode(vtkPVPlotTime::NONE);

  // Decide if time is being shown on any of the axis.
  // Iterate over all visible representations and check is they have the array
  // named "Time" on either of the axes.
  int num_reprs = this->GetNumberOfRepresentations();
  for (int cc=0; cc < num_reprs; cc++)
    {
    vtkXYChartRepresentation * repr = vtkXYChartRepresentation::SafeDownCast(
      this->GetRepresentation(cc));
    if (repr && repr->GetVisibility())
      {
      if (repr->GetXAxisSeriesName() &&
        strcmp(repr->GetXAxisSeriesName(), "Time") == 0)
        {
        this->PlotTime->SetTimeAxisMode(vtkPVPlotTime::X_AXIS);
        break;
        }
      }
    }
  // For now we only handle X-axis time. If needed we can add support for Y-axis.

  // handle custom labels. We specify custom labels in render since vtkAxis will
  // discard the custom labels when the mode was set to not use custom labels,
  // so we need to provide the labels each time to the chart.
  for (int axis=0; axis < 4 && axis < this->Chart->GetNumberOfAxes(); axis++)
    {
    vtkAxis* chartAxis= this->Chart->GetAxis(axis);
    if(!chartAxis)
      {
      continue;
      }
    if (this->Internals->UseCustomLabels[axis])
      {
      chartAxis->SetCustomTickPositions(this->Internals->CustomLabelPositions[axis].GetPointer());
      }
    else
      {
      chartAxis->SetCustomTickPositions(NULL);
      }
    chartAxis->Update();
    }

  this->Superclass::Render(interactive);
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::SelectionChanged()
{
  this->InvokeEvent(vtkCommand::SelectionChangedEvent);
}

//----------------------------------------------------------------------------
void vtkPVXYChartView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
