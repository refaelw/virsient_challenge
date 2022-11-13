
# Lets make compiling stuff easy as
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(virsient_challenge LANGUAGES C CXX) 
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

if(MSVC)
  add_compile_options(/wd4459 /wd4324)
else()
  add_compile_options(-fPIC)
endif()

include_directories("src/")

set(virsient_client_srcs src/client.c)
add_executable(virsient_client ${virsient_client_srcs})

# The server srcs 
set(COMPILE_TESTS ON) 
# The unit tests
if(COMPILE_TESTS)
    add_subdirectory(tests)
endif() 