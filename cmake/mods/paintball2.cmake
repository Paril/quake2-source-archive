cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "paintball2")
project(${MOD_NAME} C)

file(GLOB MOD_SRC
  "sources/${MOD_NAME}/paintball2/game/*.c"
  "sources/${MOD_NAME}/paintball2/game/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
