cmake_minimum_required(VERSION 3.10)

project(CVM C)

include_directories(include)

add_executable(cvmc src/frontend/cvmc.c src/backend/to_cvm_opcodes.c)
add_executable(cvmd src/frontend/cvmd.c src/backend/from_cvm_opcodes.c)
add_executable(cvmr src/frontend/cvmr.c src/backend/cvm_src.c)
