cmake_minimum_required(VERSION 3.2)

project(host C CXX ASM)

add_executable(elfinfo elfinfo.cpp)
target_compile_options(elfinfo PUBLIC "--std=c++14")
target_link_libraries(elfinfo elf)

INSTALL(TARGETS elfinfo
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
