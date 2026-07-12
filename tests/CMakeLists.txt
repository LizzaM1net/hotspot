set(CACHE{CMAKE_CTEST_ARGUMENTS} VALUE "--output-on-failure")

find_package(Catch2 3 REQUIRED)

add_executable(test_hproto test_hproto.cpp)
target_link_libraries(test_hproto PRIVATE hproto Catch2::Catch2WithMain)
add_test(NAME hproto COMMAND test_hproto)

add_executable(test_hvariant test_hvariant.cpp)
target_link_libraries(test_hvariant PRIVATE hproto Catch2::Catch2WithMain)
add_test(NAME hvariant COMMAND test_hvariant)

add_executable(test_reflection test_reflection.cpp)
target_link_libraries(test_reflection PRIVATE hproto Catch2::Catch2WithMain)
add_test(NAME reflection COMMAND test_reflection)
