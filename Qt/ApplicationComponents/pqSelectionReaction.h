/*=========================================================================

   Program: ParaView
   Module:    pqSelectionReaction.h

   Copyright (c) 2005,2006,2012 Sandia Corporation, Kitware Inc.
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
#ifndef __pqSelectionReaction_h
#define __pqSelectionReaction_h

#include "pqReaction.h"

class pqSelectionInspectorPanel;

/// @ingroup Reactions
/// pqSelectionReaction is a reaction that pops up the selection inspector.
/// Its main purpose, however, is to connect the selection inspector's signals
/// requesting that the selection be extracted or plotted to the corresponding
/// filters in ParaView's filter menu. This cannot be done easily in the
/// pqSelectionInspectorPanel itself since the pqFiltersMenu reaction is in the
/// pqApplicationComponents library which is dependent on the pqComponents
/// library containing the selection inspector.
class PQAPPLICATIONCOMPONENTS_EXPORT pqSelectionReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;
public:
  /// Constructor. Parent cannot be NULL.
  pqSelectionReaction(QAction* parent);
  virtual ~pqSelectionReaction();

  /// Show the query dialog for querying the data from the active source.
  void showInspector();

public slots:
  void onExtractSelection();
  void onExtractOverTime();

protected:
  /// Called when the action is triggered.
  virtual void onTriggered()
    { pqSelectionReaction::showInspector(); }

  pqSelectionInspectorPanel* findInspector();
  void connectInspector(pqSelectionInspectorPanel* ins);

private:
  Q_DISABLE_COPY(pqSelectionReaction)
};

#endif
