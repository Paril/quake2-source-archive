cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "kots")
project(${MOD_NAME} C)

file(GLOB MOD_SRC
  "sources/${MOD_NAME}/kots/*.c"
  "sources/${MOD_NAME}/kots/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
