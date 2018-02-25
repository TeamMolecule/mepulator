cmake_minimum_required(VERSION 2.8.11)
project(mepulator)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -O0 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -O0 -g")

include_directories(src)
include_directories(deps/mbedtls/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(generate_insn
    COMMAND python3 ${CMAKE_SOURCE_DIR}/src/generate_insn.py ${CMAKE_SOURCE_DIR}/src/insn.in ${CMAKE_CURRENT_BINARY_DIR}/insn_gen.h
    DEPENDS ${CMAKE_SOURCE_DIR}/src/generate_insn.py ${CMAKE_SOURCE_DIR}/src/insn.in
)

add_executable(mepulator
	src/arm.cpp
	src/cpu.cpp
	src/insn.cpp
	src/main.cpp
	src/memory.cpp
	src/debugger.cpp
	src/devices/armcomm.cpp
	src/devices/bigmac.cpp
	src/devices/e001device.cpp
	src/devices/eeprom.cpp
	src/devices/unknowndevice.cpp
	src/util.cpp

	deps/mbedtls/library/aes.c
	deps/mbedtls/library/aesni.c
	deps/mbedtls/library/sha256.c
)

target_link_libraries(mepulator
	pthread
)

add_dependencies(mepulator
	generate_insn
)
