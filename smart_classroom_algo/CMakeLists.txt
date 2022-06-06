cmake_minimum_required(VERSION 2.9)

project(smart_classroom_algo)
#workspace
set(WORKSPACE ${PROJECT_SOURCE_DIR}/workspace)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${WORKSPACE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${WORKSPACE})

add_subdirectory(third_party/sort)


find_package(CURL REQUIRED)
find_package(CUDA REQUIRED)
find_package(OpenCV REQUIRED)

add_definitions(-DNOSSL)
add_definitions(-DNOLFS)
if (WIN32)
    add_definitions(-DNOMINMAX)
endif ()

# =====================define custom param===========================

add_definitions(-DSMC_RTMP_SERVER="127.0.0.1")
add_definitions(-DSMC_SERVER_HOST="127.0.0.1")
add_definitions(-DMINIO_HOST="127.0.0.1:9000")
add_definitions(-DMINIO_ACCESS_KEY="hongyaohongyao")
add_definitions(-DMINIO_SECRET_KEY="hongyaohongyao123")
add_definitions(-DSMC_SERVER_PORT=12121)
add_definitions(-DSMC_MAX_RESOURCE=99999999)
add_definitions(-DSMC_LOOP_VIDEO=true)

if (WIN32)
    add_definitions(-DUSE_IMPORT_EXPORT)
    add_definitions(-DUSE_WINDOWS_DLL_SEMANTICS)
    add_definitions(-DSMC_FILE_SYS_DIR="${WORKSPACE}/local")
endif ()

# =====================define custom param end=======================

add_definitions(-std=c++17)
#add_definitions(-DAPI_EXPORTS)
option(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-permissive- -Wall -Ofast")


set(CUDA_DIR $ENV{CUDA_PATH})
set(CUDNN_DIR $ENV{CUDNN_HOME})
set(TENSORRT_DIR $ENV{TENSORRT_HOME})
set(OPENCV_DIR $ENV{OPENCV_HOME})
set(FFMPEG_DIR $ENV{FFMPEG_HOME})
set(MYSQL_DIR $ENV{MYSQL_HOME})
set(AWSSDK_DIR $ENV{AWSSDK_HOME})
set(DROGON_DIR $ENV{DROGON_HOME})
set(JSONCPP_DIR $ENV{JSONCPP_HOME})
find_package(AWSSDK REQUIRED COMPONENTS s3)
find_package(DROGON REQUIRED)
find_package(Jsoncpp REQUIRED)
#add_subdirectory(third_party/drogon) # if not DROGON_HOME

# 因为protobuf，需要用特定版本，所以这里指定路径
set(PROTOBUF_DIR "./lean/protobuf")

#cuda
include_directories(${CUDA_DIR}/include)
link_directories(${CUDA_DIR}/lib/x64)
#protobuf
include_directories(${PROTOBUF_DIR}/include)
link_directories(${PROTOBUF_DIR}/lib)
#tensorrt
include_directories(${TENSORRT_DIR}/include)
link_directories(${TENSORRT_DIR}/lib)
#cudnn
include_directories(${CUDNN_DIR}/include)
link_directories(${CUDNN_DIR}/lib/x64)
#opencv
include_directories(${OpenCV_INCLUDE_DIRS})
#ffmpeg
include_directories(${FFMPEG_DIR}/include)
link_directories(${FFMPEG_DIR}/lib)
#libcurl
include_directories(${CURL_INCLUDE_DIR})
#jsoncpp
include_directories(${JSONCPP_INCLUDE_DIRS})
#drogon
include_directories(${DROGON_INCLUDE_DIRS})
##mysql
#include_directories(${MYSQL_DIR}/include)
#link_directories(${MYSQL_DIR}/lib)

include_directories(
        ${PROJECT_SOURCE_DIR}/modules/smart_classroom/include
        ${PROJECT_SOURCE_DIR}/modules/smc_server/include
        ${PROJECT_SOURCE_DIR}/modules/tensorRT
        ${PROJECT_SOURCE_DIR}/modules/tensorRT/common
        ${PROJECT_SOURCE_DIR}/third_party/sort/include
)
set(CUDA_GEN_CODE "-gencode=arch=compute_75,code=\"sm_75,compute_75\"")
set(CMAKE_CXX_FLAGS_RELEASE "-MT")
set(CMAKE_CXX_FLAGS_DEBUG "-MT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -w -g")
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} ${CUDA_GEN_CODE} -Xcompiler \"/EHsc /nologo /FS\" ")

file(GLOB_RECURSE srcs
        ${PROJECT_SOURCE_DIR}/modules/*.cpp
        ${PROJECT_SOURCE_DIR}/modules/*.h
        ${PROJECT_SOURCE_DIR}/modules/*.hpp
        ${PROJECT_SOURCE_DIR}/modules/*.cuh
        ${PROJECT_SOURCE_DIR}/modules/*.cu
        )

cuda_add_executable(${PROJECT_NAME} ${srcs})

target_link_libraries(${PROJECT_NAME} nvinfer nvinfer_plugin sort_cpp) # link tensorrt
target_link_libraries(${PROJECT_NAME} cuda cublas cudart cudnn) # link cuda
target_link_libraries(${PROJECT_NAME} libprotobuf libprotobufd) # link protobuf
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS}) # link opencv
target_link_libraries(${PROJECT_NAME} avutil avcodec avfilter avdevice avformat postproc) # link ffmpeg
target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES}) # link libcurl
target_link_libraries(${PROJECT_NAME} ${AWSSDK_LINK_LIBRARIES}) # aws-sdk
target_link_libraries(${PROJECT_NAME} ${JSONCPP_LIBRARIES} ${DROGON_LIBRARIES}) # jsoncpp drogon






