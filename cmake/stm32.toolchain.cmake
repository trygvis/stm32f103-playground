include(CMakeForceCompiler)

set(TRIPLE "arm-none-eabi")

if (TOOLCHAIN_ROOT)
    message("Setting ENV: ${TOOLCHAIN_ROOT}")
    set(ENV[_TOOLCHAIN_ROOT} "${TOOLCHAIN_ROOT}")
else ()
    message("Setting locally ENV: ${TOOLCHAIN_ROOT}")
    set(TOOLCHAIN_ROOT "$ENV{_TOOLCHAIN_ROOT}")
endif ()

if (TOOLCHAIN_ROOT STREQUAL "")
    set(TOOLCHAIN_ROOT /usr)
    message("Using default TOOLCHAIN_ROOT: ${TOOLCHAIN_ROOT}")
else ()
    message("Using TOOLCHAIN_ROOT: ${TOOLCHAIN_ROOT}")
endif ()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_CROSSCOMPILING 1)

set(TARGET_FLAGS "-mcpu=cortex-m3 -mthumb")
set(BASE_FLAGS "-Wall -g -ffunction-sections -fdata-sections ${TARGET_FLAGS}")

set(CMAKE_C_FLAGS "${BASE_FLAGS}" CACHE STRING "c flags") # XXX Generate TIME_T dynamically.
set(CMAKE_CXX_FLAGS "${BASE_FLAGS} -fno-exceptions -fno-rtti -felide-constructors -std=c++14" CACHE STRING "c++ flags")

#set(LINKER_FLAGS "-Os -Wl,--gc-sections ${TARGET_FLAGS} -T${TEENSY_ROOT}/mk20dx256.ld")
#set(LINKER_FLAGS "-Os -Wl,--gc-sections ${TARGET_FLAGS}")
set(LINKER_FLAGS "-O0 -Wl,--gc-sections ${TARGET_FLAGS}")
set(LINKER_FLAGS "-O0 ${TARGET_FLAGS}")
set(LINKER_LIBS "-larm_cortexM4l_math -lm")
#set(CMAKE_SHARED_LINKER_FLAGS "${LINKER_FLAGS}" CACHE STRING "linker flags" FORCE)
#set(CMAKE_MODULE_LINKER_FLAGS "${LINKER_FLAGS}" CACHE STRING "linker flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${LINKER_FLAGS}" CACHE STRING "linker flags" FORCE)

CMAKE_FORCE_C_COMPILER("${TOOLCHAIN_ROOT}/bin/${TRIPLE}-gcc" GNU)
CMAKE_FORCE_CXX_COMPILER("${TOOLCHAIN_ROOT}/bin/${TRIPLE}-g++" GNU)
# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# add_definitions(-MMD)
