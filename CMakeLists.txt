cmake_minimum_required(VERSION 3.10)

# set the project name
project(dePIKczer C CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


message(STATUS "Configuring libjpeg-turbo...")
add_subdirectory(external/libjpeg-turbo EXCLUDE_FROM_ALL)

message(STATUS "Configuring dePIKczer...")
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
    src/resource.rc
)

target_include_directories(dePIKczer_CLI PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/external/miniz"
    "${PROJECT_SOURCE_DIR}/external/libjpeg-turbo"
    "${PROJECT_SOURCE_DIR}/external/piklib8_shim/include"
)

target_link_directories(dePIKczer_CLI PRIVATE
    "${PROJECT_SOURCE_DIR}/external/piklib8_shim"
)

target_link_libraries(dePIKczer_CLI turbojpeg piklib8_shim.dll)
