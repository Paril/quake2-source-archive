cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "arquade")
project(${MOD_NAME} CXX)

file(GLOB MOD_SRC
  "sources/${MOD_NAME}/game/*.cpp"
  "sources/${MOD_NAME}/game/*.h"
  "sources/${MOD_NAME}/shared/*.cpp"
  "sources/${MOD_NAME}/shared/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
target_include_directories(${MOD_NAME} PRIVATE "sources/${MOD_NAME}/game" "sources/${MOD_NAME}/shared")
