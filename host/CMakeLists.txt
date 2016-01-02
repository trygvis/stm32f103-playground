cmake_minimum_required(VERSION 3.2)

project(host C CXX ASM)

add_executable(host1.elf host1.cpp
        ../tinyprintf/tinyprintf.h ../tinyprintf/tinyprintf.c
        )

target_include_directories(host1.elf PUBLIC
        ../tinyprintf
        ../playground/include)
target_compile_definitions(host1.elf PUBLIC TINYPRINTF_DEFINE_TFP_SPRINTF=0 TINYPRINTF_OVERRIDE_LIBC=0)

add_executable(elf3 elf3.cpp)
target_link_libraries(elf3 elf)

add_executable(elf4 elf4.cpp)
target_link_libraries(elf4 elf bsd)

add_executable(elfinfo elfinfo.cpp)
target_compile_options(elfinfo PUBLIC "--std=c++14")
target_link_libraries(elfinfo elf)

INSTALL(TARGETS elfinfo
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
