find_package(Qt6 REQUIRED COMPONENTS Quick)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(apphClient
    main.cpp
)

qt_add_qml_module(apphClient
    URI hClient
    VERSION 1.0
    QML_FILES
        Main.qml
    SOURCES
        hotspotchat.h
        hotspotchat.cpp
)

set_target_properties(apphClient PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apphClient
    PRIVATE Qt6::Quick hproto
)

include(GNUInstallDirs)
install(TARGETS apphClient
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
