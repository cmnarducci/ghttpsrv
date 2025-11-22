cmake_minimum_required(VERSION 3.15)
project(ghttpsrv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
   set(CMAKE_CXX_EXTENSIONS ON)
else()
   set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
   set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type" FORCE)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

option(WITH_SSL "Enable SSL support" ON)

set(SOURCES
   common/base_sock.cpp
   common/http_srv.cpp
   common/srv_sock.cpp
   common/thread.cpp
   common/tools.cpp
   common/wxhlp.cpp
   main.cpp
   server.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
   add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES})
   target_sources(${PROJECT_NAME} PRIVATE app.icns)
   set_source_files_properties(${CMAKE_SOURCE_DIR}/app.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
   set(MACOSX_BUNDLE_ICON_FILE app.icns)
else()
   add_executable(${PROJECT_NAME} ${SOURCES})
endif()

if (WIN32)
   target_sources(${PROJECT_NAME} PRIVATE resources.rc)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/common
)

target_compile_options(${PROJECT_NAME} PRIVATE
   $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra -Werror -DDEBUG -DSOCK_DEBUG>
   $<$<CONFIG:Release>:-O2 -Wall -Wextra -Werror>
)

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
if (CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
   message(FATAL_ERROR "Cygwin is not supported, use -DCMAKE_TOOLCHAIN_FILE=common/cmake/toolchain-mingw64.cmake")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
   target_compile_definitions(${PROJECT_NAME} PRIVATE MACOS)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
   target_compile_definitions(${PROJECT_NAME} PRIVATE MINGW)
endif()

find_path(UNISTD_INCLUDE_DIR unistd.h)
if(UNISTD_INCLUDE_DIR)
   message(STATUS "UNISTD_INCLUDE_DIR: ${UNISTD_INCLUDE_DIR}")
   target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_UNISTD_H)
endif()

# Threads
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_PTHREAD)

# OpenSSL
if(WITH_SSL)
   find_package(OpenSSL)
   if(OpenSSL_FOUND)
      target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
      target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SSL)
   else()
      message(WARNING "OpenSSL not found! SSL support will be disabled or try -DOPENSSL_ROOT_DIR=/path/to/openssl.")
   endif()
endif()

find_package(wxWidgets 3.0.0 REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})
target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES})

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
   target_link_libraries(${PROJECT_NAME} PRIVATE iphlpapi)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
   add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} ${STRIP_FLAGS} $<TARGET_FILE:${PROJECT_NAME}>)
endif()
