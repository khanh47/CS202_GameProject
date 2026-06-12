cmake_minimum_required(VERSION 3.15)
project(Mario LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_STATIC_LIBRARIES TRUE)

set(SFML_DIR "${CMAKE_SOURCE_DIR}/external/SFML-3.0.2/lib/cmake/SFML")
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

include_directories(include)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SRC_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE FALSE) #Set TRUE in the final product



target_link_libraries(${PROJECT_NAME}
        PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
)

add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_BINARY_DIR}/assets"
)

add_dependencies(${PROJECT_NAME} copy_assets)

target_compile_options(${PROJECT_NAME} PRIVATE -O2)