cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "ctf")
project(${MOD_NAME} C)

file(GLOB MOD_SRC
  "sources/_quake2/ctf/*.c"
  "sources/_quake2/ctf/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
