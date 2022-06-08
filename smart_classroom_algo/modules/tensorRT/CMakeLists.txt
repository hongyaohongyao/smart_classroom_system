cmake_minimum_required(VERSION 2.9)

project(tensorrt_cpp)

add_definitions(-std=c++14)

option(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
set(CMAKE_CXX_STANDARD 14)
find_package(CUDA REQUIRED)

# 因为protobuf，需要用特定版本，所以这里指定路径
set(Protobuf_DIR $ENV{PROTOBUF_HOME})
set(CUDNN_DIR "../../lean/cudnn")
set(CUDA_DIR "../../lean/cuda")
set(TENSORRT_DIR "../../lean/TensorRT")
find_package(Protobuf REQUIRED CONFIG)
find_package(OpenCV REQUIRED)
#cuda
include_directories(${CUDA_DIR}/include)
link_directories(${CUDA_DIR}/lib)
#protobuf
include_directories(${Protobuf_INCLUDE_DIR})
#tensorrt
include_directories(${TENSORRT_DIR}/include)
link_directories(${TENSORRT_DIR}/lib)
#cudnn
include_directories(${CUDNN_DIR}/include)
link_directories(${CUDNN_DIR}/lib)
#opencv
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(
        ../..
        ${PROJECT_SOURCE_DIR}/common
)

set(CMAKE_CXX_FLAGS "-permissive- -Wall -Ofast")
set(CUDA_GEN_CODE "-gencode=arch=compute_75,code=\"sm_75,compute_75\"")

set(CMAKE_CXX_FLAGS_RELEASE "-MT")
set(CMAKE_CXX_FLAGS_DEBUG "-MT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -w -g")
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} ${CUDA_GEN_CODE} -Xcompiler \"/EHsc /nologo /FS\" ")

file(GLOB_RECURSE srcs
        ${PROJECT_SOURCE_DIR}/*.cpp
        ${PROJECT_SOURCE_DIR}/*.h
        ${PROJECT_SOURCE_DIR}/*.hpp
        ${PROJECT_SOURCE_DIR}/*.cuh
        ${PROJECT_SOURCE_DIR}/*.cu
        )

cuda_add_library(${PROJECT_NAME} SHARED ${srcs})

target_link_libraries(${PROJECT_NAME} nvinfer nvinfer_plugin) # link tensorrt
target_link_libraries(${PROJECT_NAME} cuda cublas cudart cudnn) # link cuda
target_link_libraries(${PROJECT_NAME} ${Protobuf_LIBRARIES}) # link protobuf
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS}) # link opencv