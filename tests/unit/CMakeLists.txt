include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.15.0
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

add_executable(test_hproto test_hproto.cpp)
target_link_libraries(test_hproto PRIVATE hproto Catch2::Catch2WithMain)
target_compile_features(test_hproto PRIVATE cxx_std_20)

catch_discover_tests(test_hproto)
