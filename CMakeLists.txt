cmake_minimum_required(VERSION 3.27)
project(cpp_driver_test)

set(CMAKE_CXX_STANDARD 17)
link_directories(${PROJECT_SOURCE_DIR}/lib)
include_directories(${PROJECT_SOURCE_DIR}/include)
# add_library(typedb-driver-cpp ${PROJECT_SOURCE_DIR}/lib)

add_executable(cpp_driver_test quickstart_code.cpp)
IF (WIN32)
    target_link_libraries(cpp_driver_test ${PROJECT_SOURCE_DIR}/lib/typedb-driver-cpp.if.lib)
ELSE()
    target_link_libraries(cpp_driver_test typedb-driver-cpp)
ENDIF()

target_link_libraries(cpp_driver_test "-framework CoreFoundation")