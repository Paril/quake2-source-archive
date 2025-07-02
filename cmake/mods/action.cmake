cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "action")
project(${MOD_NAME} C)

file(GLOB MOD_SRC
  "sources/${MOD_NAME}/src/source/*.c"
  "sources/${MOD_NAME}/src/source/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
