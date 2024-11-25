project(melonDS-lib)

set(ANDROID_SOURCES
        InputAndroid.cpp
        LAN_PCap.cpp
        LAN_Socket.cpp
        MelonAudioStreamErrorCallback.cpp
        MelonDS.cpp
        OboeCallback.cpp
        MicInputOboeCallback.cpp
        OpenGLContext.cpp
        PlatformAndroid.cpp
        FileUtils.cpp
        ifaddrs.c
        android_fopen.c
        AndroidARCodeFile.cpp
        Config.cpp
        ROMManager.cpp
        SaveManager.cpp
        LocalMultiplayer.cpp
        retroachievements/RetroAchievements.cpp

        fd_recv.cpp
        fd_send.cpp

        ../frontend/Util_Video.cpp
        ../frontend/Util_Audio.cpp
        ../frontend/RewindManager.cpp
        ../frontend/mic_blow.h
)

set(OBOE ./oboe)
set(RCHEEVOS ./rcheevos)
set(LIBSLIRP ./libslirp)
add_subdirectory(${OBOE} ./oboe)
add_subdirectory(${RCHEEVOS} ./rcheevos)
add_subdirectory(${LIBSLIRP} ./libslirp)
add_library(melonDS-lib SHARED ${ANDROID_SOURCES})
find_library(android-lib android)
target_include_directories(${PROJECT_NAME} PUBLIC ${OBOE}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${RCHEEVOS}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBSLIRP}/src)
target_include_directories(${PROJECT_NAME} PUBLIC ./)
target_include_directories(${PROJECT_NAME} PUBLIC ../)
target_include_directories(${PROJECT_NAME} PUBLIC ../frontend)
target_link_libraries(melonDS-lib ${android-lib} core oboe rcheevos libslirp EGL GLESv3)