
# Lets make compiling stuff easy as
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(virsient_challenge LANGUAGES C CXX) 
set (CMAKE_CXX_STANDARD 11)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

if(MSVC)
  add_compile_options(/wd4459 /wd4324)
else()
  add_compile_options(-fPIC)
endif()

include_directories("src/")

set(virsient_lib_srcs 
src/client.c 
src/server.c
src/linked_list.c
src/file_io.c)
add_library(virsient_lib STATIC ${virsient_lib_srcs}) 

set(virsient_client_srcs src/client_main.c)
add_executable(virsient_client ${virsient_client_srcs})
target_link_libraries(virsient_client virsient_lib) 

set(virsient_server_srcs src/server_main.c)
add_executable(virsient_server ${virsient_server_srcs})
target_link_libraries(virsient_server virsient_lib) 

# The server srcs 
set(COMPILE_TESTS ON) 
# The unit tests
if(COMPILE_TESTS)
    add_subdirectory(tests)
endif() 