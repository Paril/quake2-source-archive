cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "vortex")
project(${MOD_NAME} C)

file(GLOB MOD_SRC
  "sources/${MOD_NAME}/src/src/**/*.c"
  "sources/${MOD_NAME}/src/src/**/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
target_include_directories(${MOD_NAME} PRIVATE "sources/${MOD_NAME}/src/src")
