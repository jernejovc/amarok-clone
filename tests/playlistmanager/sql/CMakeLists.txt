include_directories( .
    ${AMAROK_SOURCE_TREE}/collection
    ${AMAROK_SOURCE_TREE}/browsers/playlistbrowser
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_BINARY_DIR}/tests
)

#------------------------ Test SqlUserPlaylistProvider -----------------------------

set( testsqluserplaylistprovider_SRCS
     TestSqlUserPlaylistProvider.cpp
     ${AMAROK_SOURCE_TREE}/playlistmanager/sql/SqlUserPlaylistProvider.cpp
     ${AMAROK_SOURCE_TREE}/playlistmanager/sql/SqlPlaylistGroup.cpp
     ${AMAROK_SOURCE_TREE}/playlistmanager/sql/SqlPlaylist.cpp
     ${AMAROK_SOURCE_TREE}/core-impl/meta/timecode/TimecodeMeta.cpp
     ${AMAROK_SOURCE_TREE}/core-impl/capabilities/timecode/TimecodeBoundedPlaybackCapability.cpp
     ${AMAROK_SOURCE_TREE}/core/capabilities/BoundedPlaybackCapability.cpp
     ${AMAROK_SOURCE_TREE}/core-impl/playlists/types/file/PlaylistFileSupport.cpp
    )
kde4_add_unit_test( testsqluserplaylistprovider ${testsqluserplaylistprovider_SRCS} )
target_link_libraries( testsqluserplaylistprovider ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} amarokcore amaroklib )
