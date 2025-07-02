cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "sw")
project(${MOD_NAME} C)

file(GLOB MOD_SRC
  "sources/${MOD_NAME}/source/*.c"
  "sources/${MOD_NAME}/source/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
