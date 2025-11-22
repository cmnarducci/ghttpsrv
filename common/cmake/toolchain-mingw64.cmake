set(CMAKE_SYSTEM_NAME Windows)

set(HOST_ARCH x86_64-w64-mingw32)

set(CMAKE_C_COMPILER /usr/bin/${HOST_ARCH}-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/${HOST_ARCH}-g++)
set(CMAKE_RC_COMPILER /usr/bin/${HOST_ARCH}-windres)

foreach(compiler CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_RC_COMPILER)
   if(NOT EXISTS ${${compiler}})
      message(FATAL_ERROR "Cross-compiler ${${compiler}} not found.")
   endif()
endforeach()

set(CMAKE_EXE_LINKER_FLAGS_INIT "-static -Wl,--allow-multiple-definition")

add_compile_definitions(_WIN32_WINNT=0x0600)

set(CMAKE_FIND_ROOT_PATH /usr/${HOST_ARCH}/sys-root)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
