find_package(Catch2 3 REQUIRED)
#include(Catch)
find_package( Threads )

# Add the test files here. 
set(test_srcs client_test.cpp) 
add_executable(client_tests ${test_srcs}) 
target_link_libraries(client_tests PRIVATE Catch2::Catch2WithMain virsient_lib Threads::Threads)

include(CTest)
include(Catch)
catch_discover_tests(client_tests)