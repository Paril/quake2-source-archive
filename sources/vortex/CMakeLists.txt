# Cmake script based off several sources, including cleancodequake2.

# Specify minimum version of CMake required
cmake_minimum_required (VERSION 2.8)

# Project name
project (VortexQuake2)

include(CMakeDependentOption)
option(VRX_USE_MYSQL "Use MySQL based GDS" FALSE)
option(VRX_USE_MULTITHREAD "Use multithreading (only valid when using mysql)" FALSE)
option(VRX_ALLOW_ADMIN "Allow usage of admin commands via rcon/sv commands." TRUE)
option(VRX_LOCKDEFAULTS "Disallow changing exp/credits rates via cvar." FALSE)
option(VRX_REMOVERESPAWNS "Remove the usage of respawns." FALSE)
option(VRX_USEHASH "Use hashing on commands instead of iterating through the list." TRUE)
option(VRX_Q2PRO "Use map/gamemap differenciation (for q2pro)." TRUE)

# Source files
file (GLOB VRX_FILES ./*.c)
file (GLOB VRX_BOT_FILES ./ai/*.c)

# Set the platform-specific library name and use platform-specific files.
if (WIN32)
  set (CC_LIB_NAME gamex86)
  #file (GLOB CC_PLATFORM_FILES source/Platform/windows/*)
elseif (UNIX)
  # Get machine hardware name (arch), force 386 (if applicable), strip newlines, and store in ARCH
  execute_process (COMMAND uname -m COMMAND sed s/i.86/i386/ COMMAND tr -d \n OUTPUT_VARIABLE ARCH)
  set (CC_LIB_NAME game${ARCH})
  #file (GLOB CC_PLATFORM_FILES source/Platform/unix/*)
else ()
  message (FATAL "Unknown platform")
endif ()

# Specify the source files for the game library
add_library (${CC_LIB_NAME} SHARED ${VRX_FILES} ${VRX_BOT_FILES})

if (VRX_USE_MYSQL AND WIN32)
    set (CC_LINK_LIBS ${CC_LINK_LIBS} libmysql)
endif()

if (VRX_USE_MYSQL AND UNIX)
    set (CC_LINK_LIBS ${CC_LINK_LIBS} mysqlclient)
endif ()

if (WIN32)
  set (CC_LINK_LIBS ${CC_LINK_LIBS} Wininet Winmm)
endif ()

if (UNIX AND VRX_USE_MULTITHREAD)
  find_package (Threads)
  set (CC_LINK_LIBS ${CC_LINK_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (WIN32 AND VRX_USE_MULTITHREAD)
  set (CC_LINK_LIBS ${CC_LINK_LIBS} pthreadVC2)
endif ()

if (VRX_REMOVERESPAWNS)
  add_definitions(-DREMOVE_RESPAWNS)
endif ()

if (NOT VRX_USE_MULTITHREAD)
  add_definitions(-DGDS_NOMULTITHREADING)
endif ()

if (NOT VRX_USE_MYSQL)
  add_definitions(-DNO_GDS)
endif()

if (VRX_USEHASH)
  add_definitions(-DCMD_USEHASH)
endif()

if (VRX_Q2PRO)
  add_definitions(-DQ2PRO_COMPATIBILITY)
endif()

if (VRX_ALLOW_ADMIN)
  add_definitions(-DALLOW_ADMIN)
endif()

if (VRX_LOCKDEFAULTS)
  add_definitions(-DLOCK_DEFAULTS)
endif()



target_link_libraries (${CC_LIB_NAME} ${CC_LINK_LIBS})

# If host is Unix-like, remove "lib" prefix from the library's file name
if (UNIX)
  add_custom_command (TARGET ${CC_LIB_NAME} POST_BUILD COMMAND mv lib${CC_LIB_NAME}.so ${CC_LIB_NAME}.so)
else()
  # add_custom_command (TARGET ${CC_LIB_NAME} POST_BUILD COMMAND copy lib${CC_LIB_NAME}.dll ${CC_LIB_NAME}.dll)
endif ()

if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()
