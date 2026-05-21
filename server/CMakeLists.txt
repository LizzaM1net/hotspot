add_executable(hServer main.cpp)
target_link_libraries(hServer PRIVATE hproto)
target_compile_features(hServer PRIVATE cxx_std_20)
