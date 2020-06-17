cmake_minimum_required(VERSION 3.10)

# set the project name
project(dePIKczer C CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_subdirectory(external/libjpeg-turbo EXCLUDE_FROM_ALL)

# link source files for the main executable file
add_executable(dePIKczer_CLI
    src/main.cpp
    src/console.cpp
    src/exceptions.cpp
    src/path.cpp
    src/ImageFormat.cpp
    src/IMGFormat.cpp
    src/BMPFormat.cpp
    src/JPGFormat.cpp
    src/PNGFormat.cpp
    external/miniz/miniz.c
)

target_include_directories(dePIKczer_CLI PRIVATE
    "${PROJECT_BINARY_DIR}/src"
    "${PROJECT_BINARY_DIR}/external/miniz"
    "${PROJECT_BINARY_DIR}/external/libjpeg-turbo"
    "${PROJECT_BINARY_DIR}/external/piklib8_shim/include"
)

target_link_libraries(dePIKczer_CLI turbojpeg "D:\\Programowanie\\dePIKczer\\external\\piklib8_shim\\piklib8_shim.dll")
