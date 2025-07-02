cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "lazarus")
project(${MOD_NAME} C)

file(GLOB MOD_SRC
  "sources/${MOD_NAME}/Quake2/lazarus/game/*.c"
  "sources/${MOD_NAME}/Quake2/lazarus/game/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
