include(ParaViewTestingMacros)

set(MantaView_USE_NEW_PANELS TRUE)
if (PARAVIEW_DATA_ROOT AND PARAVIEW_BUILD_QT_GUI AND BUILD_SHARED_LIBS)
  add_client_tests("pv"
    BASELINE_DIR ${PARAVIEW_DATA_ROOT}/Baseline
    TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/MantaView.xml
  )
endif()
