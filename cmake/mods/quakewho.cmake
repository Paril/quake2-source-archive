cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "quakewho")
project(${MOD_NAME} CXX)


file(GLOB MOD_SRC
  "sources/${MOD_NAME}/*.cpp"
  "sources/${MOD_NAME}/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
