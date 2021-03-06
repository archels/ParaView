#==========================================================================
#
#     Program: ParaView
#
#     Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
#     All rights reserved.
#
#     ParaView is a free software; you can redistribute it and/or modify it
#     under the terms of the ParaView license version 1.2.
#
#     See License_v1.2.txt for the full ParaView license.
#     A copy of this license can be obtained by contacting
#     Kitware Inc.
#     28 Corporate Drive
#     Clifton Park, NY 12065
#     USA
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#==========================================================================
if (NOT TARGET vtkPVServerManagerApplication)
  message(STATUS "vtkPVServerManagerApplication module is not enabled. "
    "Command line executables will not be built.")
  return()
endif()

vtk_module_dep_includes(vtkPVServerManagerApplication)
include_directories(${vtkPVServerManagerApplication_DEPENDS_INCLUDE_DIRS}
                    ${vtkPVServerManagerApplication_INCLUDE_DIRS})

foreach (name pvserver pvdataserver pvrenderserver)
  pv_add_executable(${name} ${name}.cxx)
  target_link_libraries(${name} vtkPVServerManagerApplication)
  if (NOT BUILD_SHARED_LIBS)
    target_link_libraries(${name} ${PARAVIEW_PLUGINLIST})
  endif()
endforeach()

if (PARAVIEW_ENABLE_PYTHON)
  set (VTK_WRAP_PYTHON_FIND_LIBS 1)
  include(vtkWrapPython)
  include_directories(${PYTHON_INCLUDE_DIRS})

  vtk_module_dep_includes(vtkPVPythonSupport)
  include_directories(${vtkPVPythonSupport_DEPENDS_INCLUDE_DIRS}
                      ${vtkPVPythonSupport_INCLUDE_DIRS}
                      ${CMAKE_CURRENT_BINARY_DIR})

  set (dependencies)

  # when building static ParaView, generate a header file to initialize
  # all the python wrapped modules statically.
  vtk_write_python_modules_header_for_wrapped_modules(
    "${CMAKE_CURRENT_BINARY_DIR}/pvpythonmodules.h"
    dependencies)

  foreach (name pvbatch pvpython)
    pv_add_executable(${name} ${name}.cxx)
    target_link_libraries(${name}
      vtkPVServerManagerApplication
      vtkPVPythonSupport
      ${dependencies}
      ${VTK_PYTHON_LIBRARIES})
  endforeach()

  # add python module dependencies to the server-executables for static builds.
  foreach (name pvserver pvdataserver pvrenderserver)
    target_link_libraries(${name}
        vtkPVPythonSupport
        ${dependencies}
        ${VTK_PYTHON_LIBRARIES})
  endforeach()

endif()
