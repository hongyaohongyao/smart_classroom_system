cmake_minimum_required(VERSION 2.6)
project(sort_cpp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-permissive- -Wall -Ofast")
set(CMAKE_CXX_FLAGS_RELEASE "-MT")
set(CMAKE_CXX_FLAGS_DEBUG "-MT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if (WIN32)
    add_definitions(-D_USE_MATH_DEFINES)
endif ()

# Try to find OpenCV
find_package(OpenCV REQUIRED)
if (OpenCV_FOUND)
    # If the package has been found, several variables will
    # be set, you can find the full list with descriptions
    # in the OpenCVConfig.cmake file.
    # Print some message showing some of them
    message(STATUS "OpenCV library status:")
    message(STATUS "    version: ${OpenCV_VERSION}")
    message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR "Could not locate OpenCV")
endif ()

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/Eigen)

file(GLOB SOURCE_FILES src/*.cpp)

add_library(sort_cpp ${SOURCE_FILES})

target_link_libraries(
        sort_cpp
        ${OpenCV_LIBS}
)
