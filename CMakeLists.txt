cmake_minimum_required(VERSION 3.0)

project("video_filter" VERSION 3.0
             DESCRIPTION "contains multiple little executables for video filtering."
             HOMEPAGE_URL "https://github.com/Jakobimatrix/video_filter")


#set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE
  ${OpenCV_LIBS}
  Eigen3::Eigen)

target_include_directories(${PROJECT_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)


# Eigen without EIGEN_MAKE_ALIGNED_OPERATOR_NEW requires GCC>=7, clang>=5, MSVC>=19.12 and CPP17
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION LESS 1912)
        message( FATAL_ERROR "If you compile this with MSVC you need at least the version 19.12." )
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION LESS 7)
        message( FATAL_ERROR "If you compile this with GNU you need at least the version 7." )
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION LESS 5)
        message( FATAL_ERROR "If you compile this with Clang you need at least the version 5." )
    endif()
endif()


add_subdirectory(executable)
