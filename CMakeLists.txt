cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

# Name of the project (will be the name of the plugin)
project(torchjs)
set(TORCH_VERSION "1.7.1")
set(TORCH_DEFAULT_CUDA "102")
set(TORCHVISION_VERSION "v0.9.0-rc2")

# Not had libtorch at source dir
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/libtorch/")
  find_package(CUDA)
  if(CUDA_FOUND)
    # Check for unsupported CUDA version
    if(NOT "${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR}" MATCHES
       "^(101|102|110)$")
      message(
        FATAL_ERROR
          "${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR} are not provided by libtorch. Please build from source and place it at ${CMAKE_CURRENT_BINARY_DIR}/libtorch/"
      )
    endif()
    set(LIBTORCH_SAVE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/libtorch-cu${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR}.zip"
    )
    if(NOT EXISTS ${LIBTORCH_SAVE_PATH})
      if(WIN32)
        if("${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR}" STREQUAL
           ${TORCH_DEFAULT_CUDA})
          set(DOWNLOAD_LINK
              "https://download.pytorch.org/libtorch/cu${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR}/libtorch-win-shared-with-deps-${TORCH_VERSION}.zip"
          )
          message(
            STATUS "Downloading libtorch for Windows from ${DOWNLOAD_LINK}")
          file(DOWNLOAD ${DOWNLOAD_LINK} ${LIBTORCH_SAVE_PATH})
        else()
          set(DOWNLOAD_LINK
              "https://download.pytorch.org/libtorch/cu${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR}/libtorch-win-shared-with-deps-${TORCH_VERSION}%2Bcu${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR}-.zip"
          )
          message(
            STATUS "Downloading libtorch for Windows from ${DOWNLOAD_LINK}")
          file(DOWNLOAD ${DOWNLOAD_LINK} ${LIBTORCH_SAVE_PATH})
        endif()
      elseif(APPLE)
        message(
          SEND_ERROR
            "MacOS does not support NVIDIA GPU, hence CUDA, will use the CPU version instead"
        )
        set(DOWNLOAD_LINK
            "https://download.pytorch.org/libtorch/cu${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR}/libtorch-macos-${TORCH_VERSION}.zip"
        )
        message(STATUS "Downloading libtorch for MacOS from ${DOWNLOAD_LINK}")
        file(DOWNLOAD ${DOWNLOAD_LINK} ${LIBTORCH_SAVE_PATH})
      else()
        if("${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR}" STREQUAL
           ${TORCH_DEFAULT_CUDA})
          set(DOWNLOAD_LINK
              "https://download.pytorch.org/libtorch/cu${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR}/libtorch-shared-with-deps-${TORCH_VERSION}.zip"
          )
          message(STATUS "Downloading libtorch for Linux from ${DOWNLOAD_LINK}")
          file(DOWNLOAD ${DOWNLOAD_LINK} ${LIBTORCH_SAVE_PATH})
        else()
          set(DOWNLOAD_LINK
              "https://download.pytorch.org/libtorch/cu${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR}/libtorch-shared-with-deps-${TORCH_VERSION}%2Bcu${CUDA_VERSION_MAJOR}${CUDA_VERSION_MINOR}.zip"
          )
          message(STATUS "Downloading libtorch for Linux from ${DOWNLOAD_LINK}")
          file(DOWNLOAD ${DOWNLOAD_LINK} ${LIBTORCH_SAVE_PATH})
        endif()
      endif()
    endif()
    message(STATUS "Unziping ${LIBTORCH_SAVE_PATH}")
    execute_process(COMMAND unzip -qq -o ${LIBTORCH_SAVE_PATH})
  else()
    set(LIBTORCH_SAVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libtorch-cpu.zip")
    if(NOT EXISTS ${LIBTORCH_SAVE_PATH})
      if(WIN32)
        set(DOWNLOAD_LINK
            "https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-${TORCH_VERSION}%2Bcpu.zip"
        )
        message(STATUS "Downloading libtorch for Windows from ${DOWNLOAD_LINK}")
        file(DOWNLOAD ${DOWNLOAD_LINK} ${LIBTORCH_SAVE_PATH})
      elseif(APPLE)
        set(DOWNLOAD_LINK
            "https://download.pytorch.org/libtorch/cpu/libtorch-macos-${TORCH_VERSION}.zip"
        )
        message(STATUS "Downloading libtorch for MacOS from ${DOWNLOAD_LINK}")
        file(DOWNLOAD ${DOWNLOAD_LINK} ${LIBTORCH_SAVE_PATH})
      else()
        set(DOWNLOAD_LINK
            "https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-${TORCH_VERSION}%2Bcpu.zip"
        )
        message(STATUS "Downloading libtorch for Linux from ${DOWNLOAD_LINK}")
        file(DOWNLOAD ${DOWNLOAD_LINK} ${LIBTORCH_SAVE_PATH})
      endif()
    endif()
    message(STATUS "Unziping ${LIBTORCH_SAVE_PATH}")
    execute_process(COMMAND unzip -qq -o ${LIBTORCH_SAVE_PATH})
  endif(CUDA_FOUND)
else()
  message(STATUS "libtorch found in ${CMAKE_CURRENT_BINARY_DIR}/libtorch/")
endif()

# Not had torchvision at sourcedir
set(LIB_TORCHVISION_SAVE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib_torchvision/${TORCHVISION_VERSION}")
if(NOT EXISTS ${LIB_TORCHVISION_SAVE_DIR})
    set(LIB_TORCHVISION_SAVE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/lib_torchvision.zip"
    )
    if(NOT EXISTS ${LIB_TORCHVISION_SAVE_PATH})
      # Download it form git
        set(DOWNLOAD_LINK
            "https://github.com/pytorch/vision/archive/${TORCHVISION_VERSION}.zip"
        )
        message(STATUS "Downloading torchvision source from ${DOWNLOAD_LINK}")
        file(DOWNLOAD ${DOWNLOAD_LINK} ${LIB_TORCHVISION_SAVE_PATH})
    endif()
    message(STATUS "Unziping ${LIB_TORCHVISION_SAVE_PATH}")
    execute_process(COMMAND unzip -qq -o ${LIB_TORCHVISION_SAVE_PATH} -d ${LIB_TORCHVISION_SAVE_DIR})
    endif()
else()
  message(STATUS "lib_torchvision found in ${LIB_TORCHVISION_SAVE_DIR}")
endif()

# execute_process( COMMAND rm ${CMAKE_CURRENT_SOURCE_DIR}/libtorch-cpu.zip )

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR}/libtorch/;${LIB_TORCHVISION_SAVE_DIR})

find_package(Torch REQUIRED)
find_package(TorchVision REQUIRED)

# Essential include files to build a node addon, you should add this line in
# every CMake.js based project.
include_directories(${CMAKE_JS_INC})
include_directories(${CMAKE_JS_INC} ../nan)

# Declare the location of the source files
file(GLOB SOURCE_FILES "src/*.cc" "src/*.h")

# This line will tell CMake that we're building a shared library from the above
# source files named after the project's name
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)

# This line will give our library file a .node extension without any "lib"
# prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Essential library files to link to a node addon, you should add this line in
# every CMake.js based project.
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB};${TORCH_LIBRARIES})
target_link_libraries(${PROJECT_NAME} TorchVision::TorchVision)