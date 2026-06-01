find_package(Qt6 REQUIRED COMPONENTS Quick)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(hchat
    main.cpp
)

qt_add_qml_module(hchat
    URI hClient
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES hotspotchat.h hotspotchat.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(hchat PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.hchat
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(hchat
    PRIVATE
    Qt6::Quick
    hproto
)

include(GNUInstallDirs)
install(TARGETS hchat
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
