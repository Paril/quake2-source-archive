cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "_q2pro")
project(${MOD_NAME} C)

file(GLOB MOD_SRC
  "sources/${MOD_NAME}/src/baseq2/*.c"
  "sources/${MOD_NAME}/src/baseq2/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
set_target_properties(${MOD_NAME} PROPERTIES PREFIX "")
set_target_properties(${MOD_NAME} PROPERTIES OUTPUT_NAME "bin/${MOD_NAME}/${OS_HANDLE}/game${Q2_BUILD_ARCH}")
file(MAKE_DIRECTORY bin/${mod}/${OS_HANDLE})
