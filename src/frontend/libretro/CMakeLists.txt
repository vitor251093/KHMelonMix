if (ENABLE_OGLRENDERER)
    # The OpenGL sources in `core` need MELONDS_GL_HEADER, which is provided by
    # the Qt/SDL frontend's CMakeLists. The libretro core renders in software
    # and does not supply a GL loader, so `core` would fail to compile.
    message(FATAL_ERROR
        "BUILD_LIBRETRO requires -DENABLE_OGLRENDERER=OFF; the libretro core uses the software renderer")
endif()

# src/plugins/Plugin.h includes OpenGLSupport.h unconditionally, so PlatformOGL.h
# is pulled into `core` even in a software-only build. Its built-in default for
# MELONDS_GL_HEADER expands to a string literal that still contains its own
# quotes, which is not a valid #include argument, so the definition has to come
# from the frontend. This is the same value src/frontend/qt_sdl/CMakeLists.txt
# passes.
set(MELONDS_GL_HEADER \"frontend/glad/glad.h\" CACHE STRING
    "Path to a header that contains OpenGL function and type declarations.")
target_compile_definitions(core PUBLIC MELONDS_GL_HEADER=${MELONDS_GL_HEADER})

add_library(khmelonmix_libretro MODULE
    Core.cpp
    Core.h
    CoreOptions.cpp
    CoreOptions.h
    Input.cpp
    Input.h
    Platform.cpp
    PluginJoystick.cpp
    Screen.cpp
    Screen.h
    libretro.h

    # src/plugins/Plugin.cpp calls the GL entry points unconditionally, without
    # an OGLRENDERER_ENABLED guard, so `core` leaves glad's function pointers
    # undefined even in a software-only build. The Qt frontend compiles glad for
    # the same reason. glad resolves everything through dlopen/GetProcAddress at
    # runtime, so this adds no link-time dependency on a GL library.
    ../glad/glad.c
)

target_include_directories(khmelonmix_libretro PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(khmelonmix_libretro PRIVATE core ${CMAKE_DL_LIBS})

# libretro cores are loaded by name and never link against anything, so they
# drop the "lib" prefix and always use the platform's native module suffix.
set_target_properties(khmelonmix_libretro PROPERTIES PREFIX "")

if (APPLE)
    set_target_properties(khmelonmix_libretro PROPERTIES SUFFIX ".dylib")
elseif (ANDROID)
    set_target_properties(khmelonmix_libretro PROPERTIES SUFFIX "_android.so")
elseif (WIN32)
    set_target_properties(khmelonmix_libretro PROPERTIES SUFFIX ".dll")
elseif (UNIX)
    set_target_properties(khmelonmix_libretro PROPERTIES SUFFIX ".so")
endif()

# A core is dropped into a frontend's cores directory on its own, with no
# runtime alongside it, so the GCC libraries have to travel inside the module.
if (MINGW)
    target_link_options(khmelonmix_libretro PRIVATE -static-libgcc -static-libstdc++ -static)
endif()

# Several melonDS-derived cores can be loaded into the same frontend process.
# Without a version script they would all export their melonDS symbols and
# resolve each other's, so hide everything that is not part of the libretro
# entry point set.
if (NOT WIN32 AND NOT APPLE)
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/link.T")
    check_cxx_source_compiles("int main() { return 0; }" HAVE_VERSION_SCRIPT)
    unset(CMAKE_REQUIRED_FLAGS)

    if (HAVE_VERSION_SCRIPT)
        target_link_options(khmelonmix_libretro PRIVATE
            "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/link.T")
    endif()
endif()

# RetroArch pairs a core with its info file by name, and reads the info file to
# decide which content the core can open and which playlist it belongs to, so
# the two ship together. This is a tracked output rather than a POST_BUILD step
# because editing only the info file has to redeploy it without relinking.
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/khmelonmix_libretro.info"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/khmelonmix_libretro.info"
        "${CMAKE_CURRENT_BINARY_DIR}/khmelonmix_libretro.info"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/khmelonmix_libretro.info"
    COMMENT "Copying khmelonmix_libretro.info"
)

add_custom_target(khmelonmix_libretro_info ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/khmelonmix_libretro.info")

add_dependencies(khmelonmix_libretro khmelonmix_libretro_info)

# Keeps the info file next to the module even if a parent scope redirects the
# library output somewhere else.
set_target_properties(khmelonmix_libretro PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
