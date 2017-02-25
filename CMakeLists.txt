cmake_minimum_required(VERSION 2.8.11)
project(mepulator)

add_definitions("-std=c++14")

include_directories(src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(generate_insn
    COMMAND python3 ${CMAKE_SOURCE_DIR}/src/generate_insn.py ${CMAKE_SOURCE_DIR}/src/insn.in ${CMAKE_CURRENT_BINARY_DIR}/insn_gen.h
    DEPENDS ${CMAKE_SOURCE_DIR}/src/generate_insn.py ${CMAKE_SOURCE_DIR}/src/insn.in
)

add_executable(mepulator
	src/cpu.cpp
	src/insn.cpp
	src/main.cpp
	src/memory.cpp
	src/devices/bigmac.cpp
	src/devices/e001device.cpp
	src/devices/unknowndevice.cpp
)

add_dependencies(mepulator
	generate_insn
)
