cmake_minimum_required(VERSION 3.16)

project(hClient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(CapnProto REQUIRED CONFIG)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(${CAPNP_DEFINITIONS})
capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS hotspot.capnp)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(apphClient
    main.cpp
)

qt_add_qml_module(apphClient
    URI hClient
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES hotspotchat.h hotspotchat.cpp
        SOURCES ${CAPNP_SRCS} ${CAPNP_HDRS}
        RESOURCES hotspot.capnp
        SOURCES hproto.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(apphClient PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apphClient
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apphClient
    PRIVATE Qt6::Quick CapnProto::capnp
)

include(GNUInstallDirs)
install(TARGETS apphClient
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
