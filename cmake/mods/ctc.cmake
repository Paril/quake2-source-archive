cmake_minimum_required(VERSION 3.10)
set(MOD_NAME "ctc")
project(${MOD_NAME} C)

file(GLOB MOD_SRC
  "sources/${MOD_NAME}/*.c"
  "sources/${MOD_NAME}/*.h"
  "sources/${MOD_NAME}/chicken/*.c"
  "sources/${MOD_NAME}/chicken/*.h"
  "sources/${MOD_NAME}/q2cam/*.c"
  "sources/${MOD_NAME}/q2cam/*.h"
  "sources/${MOD_NAME}/StdLog/*.c"
  "sources/${MOD_NAME}/StdLog/*.h"
)

add_library(${MOD_NAME} SHARED ${MOD_SRC})
target_include_directories(${MOD_NAME} PRIVATE "sources/${MOD_NAME}" "sources/${MOD_NAME}/chicken" "sources/${MOD_NAME}/q2cam" "sources/${MOD_NAME}/StdLog")

