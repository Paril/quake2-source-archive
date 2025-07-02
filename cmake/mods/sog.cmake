cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "sog")
project(${MOD_NAME} C)

file(GLOB MOD_SRC
  "sources/${MOD_NAME}/gamex86/*.c"
  "sources/${MOD_NAME}/gamex86/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
