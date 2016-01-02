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

set(LINKER_FLAGS "-O3 ${TARGET_FLAGS}")
set(LINKER_LIBS "-larm_cortexM4l_math -lm")

set(CMAKE_EXE_LINKER_FLAGS "${LINKER_FLAGS}" CACHE STRING "linker flags" FORCE)

cmake_force_c_compiler("${TOOLCHAIN_ROOT}/bin/${TRIPLE}-gcc" GNU)
cmake_force_cxx_compiler("${TOOLCHAIN_ROOT}/bin/${TRIPLE}-g++" GNU)
# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
