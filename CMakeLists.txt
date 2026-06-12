cmake_minimum_required(VERSION 3.15)
project(DSA_Visualizer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(run ${SRC_FILES})

target_include_directories(run PRIVATE 
    include
    include/Commands
    include/Scene
    include/Scene/ConcreteScene
    include/Button
    "${CMAKE_CURRENT_SOURCE_DIR}/external/SFML-3.0.2/include"
)

target_link_directories(run PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/external/SFML-3.0.2/lib")
target_link_libraries(run PRIVATE sfml-graphics sfml-window sfml-system)

target_compile_options(run PRIVATE -O2)