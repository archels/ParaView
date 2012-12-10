/*=========================================================================

   Program: ParaView
   Module:    pqSelectionReaction.cxx

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
#include "pqSelectionReaction.h"

#include "pqActiveObjects.h"
#include "pqCoreUtilities.h"
#include "pqFiltersMenuReaction.h"
#include "pqHelpReaction.h"
#include "pqPVApplicationCore.h"
#include "pqQueryDialog.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"
#include "vtkPVConfig.h"
#include "pqSelectionInspectorPanel.h"

#include <QApplication>
#include <QEventLoop>
#include <QMessageBox>

//-----------------------------------------------------------------------------
pqSelectionReaction::pqSelectionReaction(QAction* parentObject)
  : Superclass(parentObject)
{
  pqSelectionInspectorPanel* inspector = this->findInspector();
  this->connectInspector(inspector);
}

//-----------------------------------------------------------------------------
pqSelectionReaction::~pqSelectionReaction()
{
}

//-----------------------------------------------------------------------------
void pqSelectionReaction::onExtractSelection()
{
  pqFiltersMenuReaction::createFilter("filters", "ExtractSelection");
}

//-----------------------------------------------------------------------------
void pqSelectionReaction::onExtractOverTime()
{
  pqFiltersMenuReaction::createFilter("filters", "ExtractSelectionOverTime");
}

//-----------------------------------------------------------------------------
void pqSelectionReaction::showInspector()
{
  pqSelectionInspectorPanel* inspector = this->findInspector();
  if (inspector)
    {
    inspector->parentWidget()->show();
    }
}

//-----------------------------------------------------------------------------
pqSelectionInspectorPanel* pqSelectionReaction::findInspector()
{
  pqSelectionInspectorPanel* inspector = 0;

  if (qApp)
    {
    int ntop = static_cast<int>(qApp->topLevelWidgets().size());
    for (int i = 0; i < ntop; ++i)
      {
      QWidget* widg = qApp->topLevelWidgets().at(i);
      foreach (inspector, widg->findChildren<pqSelectionInspectorPanel*>())
        {
        return inspector;
        break;
        }
      }
    }
  return inspector;
}

void pqSelectionReaction::connectInspector(pqSelectionInspectorPanel* ins)
{
  if (ins)
    {
    QObject::connect(
      ins, SIGNAL(onExtractSelection()),
      this, SLOT(onExtractSelection()));
    QObject::connect(
      ins, SIGNAL(onExtractOverTime()),
      this, SLOT(onExtractOverTime()));
    }
}
