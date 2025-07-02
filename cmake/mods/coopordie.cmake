cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "coopordie")
project(${MOD_NAME} C)

file(GLOB MOD_SRC
  "sources/${MOD_NAME}/game/trunk/code/game/*.c"
  "sources/${MOD_NAME}/game/trunk/code/game/*.h"
  "sources/${MOD_NAME}/game/trunk/code/game/wsrv/*.c"
  "sources/${MOD_NAME}/game/trunk/code/game/wsrv/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
target_include_directories(${MOD_NAME} PRIVATE "sources/${MOD_NAME}/game" "sources/${MOD_NAME}/game/wsrv")
