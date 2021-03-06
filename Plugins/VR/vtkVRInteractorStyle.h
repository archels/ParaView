/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

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
#ifndef __vtkVRInteractorStyle_h
#define __vtkVRInteractorStyle_h

#include <vtkObject.h>
#include <vector>

class vtkPVXMLElement;
class vtkSMProxyLocator;
class vtkSMProxy;
class vtkSMDoubleVectorProperty;
struct vtkVREventData;

class vtkVRInteractorStyle : public vtkObject
{
public:
  static vtkVRInteractorStyle *New();
  vtkTypeMacro(vtkVRInteractorStyle, vtkObject)
  void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Specify the proxy and property to control. The property needs to have 16
  // elements and must be a numerical property.
  virtual void SetControlledProxy(vtkSMProxy *);
  vtkGetObjectMacro(ControlledProxy, vtkSMProxy)

  vtkSetStringMacro(ControlledPropertyName)
  vtkGetStringMacro(ControlledPropertyName)

  virtual bool HandleEvent(const vtkVREventData& data);
  virtual bool Update();

  // Description:
  // Returns true if the specified input type is needed.
  vtkGetMacro(NeedsAnalog, bool);
  vtkGetMacro(NeedsButton, bool);
  vtkGetMacro(NeedsTracker, bool);

  // Description:
  // Set/Get the name of the associated input
  vtkSetStringMacro(AnalogName)
  vtkGetStringMacro(AnalogName)
  vtkSetStringMacro(ButtonName)
  vtkGetStringMacro(ButtonName)
  vtkSetStringMacro(TrackerName)
  vtkGetStringMacro(TrackerName)

  /// Load state for the style from XML.
  virtual bool Configure(vtkPVXMLElement* child, vtkSMProxyLocator*);

  /// Save state to xml.
  virtual vtkPVXMLElement* SaveConfiguration() const;

protected:
  vtkVRInteractorStyle();
  virtual ~vtkVRInteractorStyle();

  virtual void HandleButton ( const vtkVREventData& data );
  virtual void HandleAnalog ( const vtkVREventData& data );
  virtual void HandleTracker( const vtkVREventData& data );

  static std::vector<std::string> Tokenize( std::string input);

  vtkSMProxy *ControlledProxy;
  char *ControlledPropertyName;

  bool NeedsAnalog;
  bool NeedsButton;
  bool NeedsTracker;

  char *AnalogName;
  char *ButtonName;
  char *TrackerName;

private:
  vtkVRInteractorStyle(const vtkVRInteractorStyle&); // Not implemented.
  void operator=(const vtkVRInteractorStyle&); // Not implemented.
};

#endif
