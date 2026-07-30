set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.22)
project(Mario LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(SFML_STATIC_LIBRARIES TRUE)
    set(SFML_DIR "${CMAKE_SOURCE_DIR}/external/SFML-3.0.2/lib/cmake/SFML")
elseif(UNIX)
    set(SFML_STATIC_LIBRARIES FALSE)
    set(SFML_DIR "/home/linuxbrew/.linuxbrew/lib/cmake/SFML")
endif()

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

include_directories(include)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER SRC_FILES EXCLUDE REGEX "[/\\\\]main\\.cpp$")

add_library(MarioCore STATIC ${SRC_FILES})
add_executable(${PROJECT_NAME} src/main.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE FALSE) #Set TRUE in the final product

target_include_directories(MarioCore
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/external
)

add_subdirectory(external/Box2D-3.1.1)

target_link_libraries(MarioCore
        PUBLIC
        box2d::box2d
        SFML::Graphics
        SFML::Window
        SFML::System
)
target_link_libraries(${PROJECT_NAME} PRIVATE MarioCore)

add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_BINARY_DIR}/assets"
)

add_dependencies(${PROJECT_NAME} copy_assets)

target_compile_options(MarioCore PRIVATE -O2)

if(BUILD_TESTING)
    add_executable(WorldRefactorTests
        tests/WorldRefactorTests.cpp
    )
    target_link_libraries(WorldRefactorTests PRIVATE MarioCore)
    target_compile_definitions(WorldRefactorTests PRIVATE
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )
    add_test(
        NAME WorldRefactorTests
        COMMAND WorldRefactorTests
    )
endif()
