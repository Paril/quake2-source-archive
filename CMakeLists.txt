cmake_minimum_required(VERSION 3.10)
project(q2mods)

# Variables used to control destination filenames and 32bit builds
string(TOLOWER ${CMAKE_SYSTEM_NAME} OS_HANDLE)
option(Q2_BUILD_ARCH "Target architecture to build (default: host architecture)" ${CMAKE_SYSTEM_PROCESSOR})

# Add cmake module path for special-case modules for certain mods
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Loop over all subdirectories in `sources` and add each one as a mod target to build
file(GLOB MODS LIST_DIRECTORIES true "sources/*")
foreach(item ${MODS})
	if(IS_DIRECTORY ${item})
    cmake_path(GET item FILENAME mod)

    # If we have a special-case cmake module for the mod, use that
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mods/${mod}.cmake")
      message(STATUS "Adding mod: cmake/mods/${mod}.cmake")
      include(mods/${mod})

    # Otherwise assume a standard mod setup
    else()
      set(MOD_SRC "")
      file(GLOB MOD_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/sources/${mod}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/sources/${mod}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/sources/${mod}/*.C"
        "${CMAKE_CURRENT_SOURCE_DIR}/sources/${mod}/*.H"
        "${CMAKE_CURRENT_SOURCE_DIR}/sources/${mod}/src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/sources/${mod}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/sources/${mod}/src/*.C"
        "${CMAKE_CURRENT_SOURCE_DIR}/sources/${mod}/src/*.H"
      )

      # Warn on mods that don't find source files with the above setup
      # so special case modules can be added
      if(NOT MOD_SRC)
        message(WARNING "No source files found for ${mod}")
      else()
        message("-- Adding mod: ${mod}")
        add_library(${mod} SHARED ${MOD_SRC})
        set_target_properties(${mod} PROPERTIES PREFIX "")
        set_target_properties(${mod} PROPERTIES OUTPUT_NAME "bin/${mod}/${OS_HANDLE}/game${Q2_BUILD_ARCH}")
        file(MAKE_DIRECTORY bin/${mod}/${OS_HANDLE})

        # Handle 32bit builds
        if(LINUX)
          if (Q2_BUILD_ARCH STREQUAL "x86")
            target_include_directories(${mod} SYSTEM BEFORE PUBLIC /usr/i686-linux-gnu/include)
            target_compile_options(${mod} PUBLIC -m32)
            target_link_directories(${mod} BEFORE PUBLIC
              /usr/i686-linux-gnu/lib
              /usr/lib/gcc-cross/i686-linux-gnu/13
            )
            target_link_options(${mod} PUBLIC -m32)
          elseif (Q2_BUILD_ARCH STREQUAL "armhf")
            target_include_directories(${mod} SYSTEM BEFORE PUBLIC /usr/arm-linux-gnueabihf/include)
            target_compile_options(${mod} PUBLIC -m32 -mfloat-abi=hard -target arm-linux-gnueabihf)
            target_link_directories(${mod} BEFORE PUBLIC
              /usr/arm-linux-gnueabihf/lib
              /usr/lib/gcc-cross/arm-linux-gnueabihf/13
            )
            target_link_options(${mod} PUBLIC -m32 -mfloat-abi=hard -target arm-linux-gnueabihf)
          endif()
        endif()

      endif()
    endif()

    if(TARGET ${mod})
      set_target_properties(${mod} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "game${Q2_BUILD_ARCH}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${mod}/${OS_HANDLE}/"
        CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${mod}/${OS_HANDLE}/"
      )
    endif()

  endif()
endforeach()

