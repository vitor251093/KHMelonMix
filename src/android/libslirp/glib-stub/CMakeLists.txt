add_library(glib
        glib.c
        strlcompat.cpp
)

target_include_directories(glib PRIVATE ".")
target_include_directories(glib INTERFACE ".")
target_include_directories(glib INTERFACE "../src")