add_executable(test_hproto test_hproto.cpp)
target_link_libraries(test_hproto PRIVATE hproto)

add_test(NAME hproto COMMAND test_hproto)
