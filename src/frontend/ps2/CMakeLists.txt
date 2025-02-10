include(CMakeDependentOption)

include(FixInterfaceIncludes)

set(SRC_DIR ../..)

# Set the target binary
set(EE_BIN melonDS.elf)

# Define PS2 SDK paths (adjust these as needed)
#set(PS2SDK /path/to/ps2sdk)
#set(PS2DEV /path/to/ps2dev)

# GIT Version info
execute_process(
    COMMAND git describe --abbrev=6 --dirty --always --tags
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Define the bin2s tool
set(BIN2S ${PS2SDK}/bin/bin2s)

# Source directories
set(CPPSOURCES src src/ps2)
set(INCLUDES src)

# Libraries to link against
set(EE_LIBS
    -L${PS2SDK}/ports/lib
    -L${PS2DEV}/gsKit/lib/
    -lpatches
    -lfileXio
    -lpad
    -ldebug
    -lgskit_toolkit
    -lgskit
    -ldmakit
    -lpng
    -lz
    -lmc
    -laudsrv
)

# Include directories
include_directories(${PS2DEV}/gsKit/include ${PS2SDK}/ports/include ${PS2SDK}/ports/include/zlib ${INCLUDES})

# IOP module source files
set(IOP_MODULES
    ${SRC_DIR}/iomanx.o
    ${SRC_DIR}/filexio.o
    ${SRC_DIR}/sio2man.o
    ${SRC_DIR}/mcman.o
    ${SRC_DIR}/mcserv.o
    ${SRC_DIR}/padman.o
    ${SRC_DIR}/libsd.o
    ${SRC_DIR}/usbd.o
    ${SRC_DIR}/audsrv.o
    ${SRC_DIR}/bdm.o
    ${SRC_DIR}/bdmfs_vfat.o
    ${SRC_DIR}/usbmass_bd.o
)

# Find all CPP source files
file(GLOB_RECURSE CPPFILES ${CPPSOURCES}/*.cpp)
file(GLOB BINFILES ${DATA}/*.bin)

# Object files to compile
set(EE_OBJS ${IOP_MODULES} ${BINFILES} ${CPPFILES})

# Compiler flags
set(EE_CXXFLAGS -mtune=r5900 -msingle-float -fno-exceptions -std=gnu++11 -D__PS2__)

# Create the main executable target
add_executable(${EE_BIN} ${EE_OBJS})

# Link libraries
target_link_libraries(${EE_BIN} ${EE_LIBS})

# Custom command to embed IOP modules
foreach(module ${IOP_MODULES})
    get_filename_component(MODULE_NAME ${module} NAME_WE)
    add_custom_command(
        TARGET ${EE_BIN} POST_BUILD
        COMMAND ${BIN2S} ${PS2SDK}/iop/irx/${MODULE_NAME}.irx ${SRC_DIR}/${MODULE_NAME}.s ${MODULE_NAME}_irx
        COMMENT "Embedding ${MODULE_NAME} Driver..."
    )
endforeach()

# Clean up command
add_custom_target(clean
    COMMAND rm -rf ${EE_BIN} ${EE_OBJS}
    COMMAND rm -f ${SRC_DIR}/sio2man.s
    COMMAND rm -f ${SRC_DIR}/mcman.s
    COMMAND rm -f ${SRC_DIR}/mcserv.s
    COMMAND rm -f ${SRC_DIR}/padman.s
    COMMAND rm -f ${SRC_DIR}/libsd.s
    COMMAND rm -f ${SRC_DIR}/bdm.s
    COMMAND rm -f ${SRC_DIR}/usbd.s
    COMMAND rm -f ${SRC_DIR}/audsrv.s
    COMMAND rm -f ${SRC_DIR}/bdmfs_vfat.s
    COMMAND rm -f ${SRC_DIR}/usbmass_bd.s
)

# Include external Makefiles
include(${PS2SDK}/samples/Makefile.pref)
include(${PS2SDK}/samples/Makefile.eeglobal_cpp)
