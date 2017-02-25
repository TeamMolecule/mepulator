cmake_minimum_required(VERSION 2.8.11)
project(mepulator)

add_definitions("-std=c++14")

add_executable(mepulator
	src/cpu.cpp
	src/insn.cpp
	src/main.cpp
	src/memory.cpp
)
