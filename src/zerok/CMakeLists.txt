cmake_minimum_required(VERSION 3.10)
project(zerok LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-g")
# target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)

set(SOURCES
    main.cc
)

add_executable(zerok ${SOURCES})

set_target_properties(zerok PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(zerok PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)