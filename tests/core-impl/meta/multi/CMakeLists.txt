include_directories(
  ${AMAROK_TEST_TREE}
  ${AMAROK_SOURCE_TREE}
  ${AMAROK_UTILITY_TREE}
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}/tests
  ${AMAROK_COLLECTION_SUPPORT_DIR}
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDES}
  ${GOOGLEMOCK_INCLUDE_DIR}
  )

#------------------------ Test MetaMultiTrack -----------------------------

set( testmetamultitrack_SRCS 
    TestMetaMultiTrack.cpp 
    ${AMAROK_SOURCE_TREE}/EngineController.cpp
    ${AMAROK_SOURCE_TREE}/core/capabilities/BoundedPlaybackCapability.cpp
    ${AMAROK_SOURCE_TREE}/core/capabilities/MultiSourceCapability.cpp
    ${AMAROK_SOURCE_TREE}/core-impl/meta/multi/MultiTrack.cpp
    ${AMAROK_SOURCE_TREE}/core-impl/capabilities/multisource/MultiSourceCapabilityImpl.cpp
    ${AMAROK_SOURCE_TREE}/playback/PowerManager.cpp )


kde4_add_unit_test( testmetamultitrack
    ${testmetamultitrack_SRCS}
    )

add_dependencies( testmetamultitrack amarokcore )
add_dependencies( testmetamultitrack amaroklib )

target_link_libraries( testmetamultitrack
    amarokcore
    amaroklib
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_PHONON_LIBRARY}
    ${KDE4_SOLID_LIBS}
    ${KDE4_THREADWEAVER_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${GOOGLEMOCK_LIBRARIES}
    ${CMAKE_DL_LIBS})
