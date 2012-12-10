/*=========================================================================

   Program: ParaView
   Module:    pqQueryDialog.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "pqQueryDialog.h"
#include "ui_pqQueryDialog.h"

#include "pqActiveObjects.h"
#include "pqActiveView.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqOutputPort.h"
#include "pqOutputPortComboBox.h"
#include "pqPipelineSource.h"
#include "pqQueryClauseWidget.h"
#include "pqSelectionInspectorPanel.h"
#include "pqServer.h"
#include "pqSignalAdaptors.h"
#include "pqUndoStack.h"
#include "vtkDataObject.h"
#include "vtkPVArrayInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkSmartPointer.h"
#include "vtkSMGlobalPropertiesManager.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProperty.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMViewProxy.h"
#include "vtkQuerySelectionSource.h"
#include "vtkPVCompositeDataInformation.h"

#include <QList>

class pqQueryDialog::pqInternals : public Ui::pqQueryDialog
{
public:
  QList<pqQueryClauseWidget*> Clauses;
};

//-----------------------------------------------------------------------------
pqQueryDialog::pqQueryDialog(
  pqOutputPort* _producer,
  QWidget* parentObject, Qt::WindowFlags _flags)
  : Superclass(parentObject, _flags)
{
  this->Internals = new pqInternals();
  this->Internals->setupUi(this);

  this->Producer = NULL;

  // Update the GUI
  this->Internals->source->setAutoUpdateIndex(false);
  this->Internals->source->fillExistingPorts();
  if(_producer != NULL)
    {
    this->Internals->source->setCurrentPort(_producer);
    this->populateSelectionType();
    }

  // Ensure that there's only 1 clause
  this->resetClauses();

  QObject::connect(this->Internals->selectionType,
    SIGNAL(currentIndexChanged(int)),
    this, SLOT(resetClauses()));

  QObject::connect(this->Internals->addRow,
    SIGNAL(clicked()), this, SLOT(addClause()));

  /// Currently we don't support multiple clauses.
  this->Internals->addRow->hide();

  QObject::connect(this->Internals->runQuery,
    SIGNAL(clicked()), this, SLOT(runQuery()));

  QObject::connect(this->Internals->extractSelection,
    SIGNAL(clicked()), this, SLOT(onExtractSelection()));
  QObject::connect(this->Internals->extractSelectionOverTime,
    SIGNAL(clicked()), this, SLOT(onExtractSelectionOverTime()));

  // Make sure that when the input port selection changes we are aware of it
  QObject::connect(
    this->Internals->source, SIGNAL(currentIndexChanged(pqOutputPort*)),
    this, SLOT(onSelectionChange(pqOutputPort*)));

  this->onSelectionChange(_producer);
}

//-----------------------------------------------------------------------------
pqQueryDialog::~pqQueryDialog()
{
  delete this->Internals;
  this->Internals = 0;
}

#include <QInputEvent>
#include <pqCoreUtilities.h>
#include "QVTKWidget.h"
//-----------------------------------------------------------------------------
void pqQueryDialog::populateSelectionType()
{
  this->Internals->selectionType->clear();
  vtkPVDataInformation* dataInfo =
      this->Internals->source->currentPort()->getDataInformation();
  if (dataInfo->DataSetTypeIsA("vtkGraph"))
    {
    this->Internals->selectionType->addItem("Vertex", vtkDataObject::VERTEX);
    this->Internals->selectionType->addItem("Edge",   vtkDataObject::EDGE);
    }
  else if (dataInfo->DataSetTypeIsA("vtkTable"))
    {
    this->Internals->selectionType->addItem("Row", vtkDataObject::ROW);
    }
  else
    {
    this->Internals->selectionType->addItem("Cell",  vtkDataObject::CELL);
    this->Internals->selectionType->addItem("Point", vtkDataObject::POINT);
    }
}

//-----------------------------------------------------------------------------
void pqQueryDialog::resetClauses()
{
  foreach (pqQueryClauseWidget* clause, this->Internals->Clauses)
    {
    delete clause;
    }
  this->Internals->Clauses.clear();

  delete this->Internals->queryClauseFrame->layout();
  QVBoxLayout *vbox = new QVBoxLayout(this->Internals->queryClauseFrame);
  vbox->setMargin(0);

  this->addClause();
}

//-----------------------------------------------------------------------------
void pqQueryDialog::addClause()
{
  if(this->Internals->source->currentPort() == NULL ||
     this->Internals->source->currentPort()->getSource()->getProxy()->GetObjectsCreated() != 1)
    {
    return; // We can not create a Clause on nothing...
    }

  pqQueryClauseWidget* clause = new pqQueryClauseWidget(this);

  // connect help requested signal
  connect(clause, SIGNAL(helpRequested()), this, SIGNAL(helpRequested()));

  int attr_type = this->Internals->selectionType->itemData(
    this->Internals->selectionType->currentIndex()).toInt();
  clause->setProducer(this->Internals->source->currentPort());
  clause->setAttributeType(attr_type);
  clause->initialize();

  this->Internals->Clauses.push_back(clause);

  QVBoxLayout* vbox =
    qobject_cast<QVBoxLayout*>(this->Internals->queryClauseFrame->layout());
  vbox->addWidget(clause);
}

//-----------------------------------------------------------------------------
void pqQueryDialog::runQuery()
{
  if(this->Internals->Clauses.isEmpty())
    {
    return;
    }

  // create selection source
  vtkSMProxy* selectionSource = this->Internals->Clauses[0]->newSelectionSource();
    if (!selectionSource)
      {
      return;
      }

  int attr_type = this->Internals->selectionType->itemData(
    this->Internals->selectionType->currentIndex()).toInt();

  if(attr_type == vtkDataObject::FIELD_ASSOCIATION_CELLS)
    {
    vtkSMPropertyHelper(selectionSource, "FieldType").Set(vtkSelectionNode::CELL);
    }
  else if(attr_type == vtkDataObject::FIELD_ASSOCIATION_POINTS)
    {
    vtkSMPropertyHelper(selectionSource, "FieldType").Set(vtkSelectionNode::POINT);
    }

  selectionSource->UpdateVTKObjects();

  this->Internals->source->currentPort()->setSelectionInput(
      vtkSMSourceProxy::SafeDownCast(selectionSource), 0);
  selectionSource->Delete();

  this->Internals->source->currentPort()->renderAllViews();

  this->Internals->extractSelection->setEnabled(true);
  this->Internals->extractSelectionOverTime->setEnabled(true);

  emit this->selected(this->Internals->source->currentPort());

  // Hide this dialog:
  this->accept();
  // Bring up the selection inspector if it is not already visible:
  if (qApp)
    {
    int ntop = static_cast<int>(qApp->topLevelWidgets().size());
    for (int i = 0; i < ntop; ++i)
      {
      QWidget* widg = qApp->topLevelWidgets().at(i);
      foreach (pqSelectionInspectorPanel* inspector, widg->findChildren<pqSelectionInspectorPanel*>())
        {
        inspector->parentWidget()->show();
        return;
        }
      }
    }
}

//-----------------------------------------------------------------------------
void pqQueryDialog::onSelectionChange(pqOutputPort* newSelectedPort)
{

  // Reset the spreadsheet view
  this->resetClauses();

  this->Producer = newSelectedPort;

  if(this->Producer != NULL)
    {
    vtkPVDataInformation* dataInfo = this->Internals->source->currentPort()->getDataInformation();
    if (dataInfo->GetTimeSpan()[0] >= dataInfo->GetTimeSpan()[1])
      {
      // don't show the extract selection over time option is there's not time!
      this->Internals->extractSelectionOverTime->hide();
      }
    else
      {
      this->Internals->extractSelectionOverTime->show();
      }
    }
}
