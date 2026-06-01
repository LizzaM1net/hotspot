add_library(hproto
    hlog.h
    hlog.cpp
    hbuffer.h
    hproto.h
    hproto_router.h
)

target_include_directories(hproto
    PUBLIC .
)
