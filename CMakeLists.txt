set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.22)
project(Mario LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(SFML_STATIC_LIBRARIES TRUE)
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/external/SFML-3.0.2/lib/cmake")
elseif(UNIX)
    set(SFML_STATIC_LIBRARIES FALSE)
    list(APPEND CMAKE_PREFIX_PATH "/home/linuxbrew/.linuxbrew")
endif()

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)
add_subdirectory(external/Box2D-3.1.1)

file(GLOB_RECURSE MARIO_CORE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(REMOVE_ITEM MARIO_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(MarioCore STATIC ${MARIO_CORE_SOURCES})
target_include_directories(MarioCore
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/external
)
target_link_libraries(MarioCore
    PUBLIC
        box2d::box2d
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Audio
)
target_compile_options(MarioCore PRIVATE -O2)

add_executable(Mario src/main.cpp)
set_property(TARGET Mario PROPERTY WIN32_EXECUTABLE FALSE)
target_link_libraries(Mario PRIVATE MarioCore)

add_executable(MarioAiTrainer tests/ai.cpp)
target_link_libraries(MarioAiTrainer PRIVATE MarioCore)

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_BINARY_DIR}/assets"
)
add_dependencies(Mario copy_assets)
add_dependencies(MarioAiTrainer copy_assets)

include(CTest)
if(BUILD_TESTING)
    add_executable(MarioAiTests tests/AiTests.cpp)
    target_link_libraries(MarioAiTests PRIVATE MarioCore)
    target_compile_definitions(MarioAiTests PRIVATE
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )
    add_dependencies(MarioAiTests copy_assets)

    add_test(NAME MarioAiTests COMMAND MarioAiTests)
    set_tests_properties(MarioAiTests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

    add_test(
        NAME MarioAiTrainerSmoke
        COMMAND MarioAiTrainer
            --map "${CMAKE_SOURCE_DIR}/assets/datas/minigames/ai/minigame.json"
            --output "${CMAKE_BINARY_DIR}/ai-smoke-model.json"
            --population 2
            --generations 1
            --max-steps 5
            --seed 1337
    )
    set_tests_properties(MarioAiTrainerSmoke PROPERTIES
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        TIMEOUT 60
    )

    add_test(
        NAME MarioAiEvalVsHeuristicSmoke
        COMMAND MarioAiTrainer
            --test-model "${CMAKE_SOURCE_DIR}/assets/ai/minigame.json"
            --rounds 2
            --max-steps 60
    )
    set_tests_properties(MarioAiEvalVsHeuristicSmoke PROPERTIES
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        TIMEOUT 60
    )

    add_test(
        NAME MarioAiTrainerOutputLoads
        COMMAND MarioAiTests
            --model "${CMAKE_BINARY_DIR}/ai-smoke-model.json"
    )
    set_tests_properties(MarioAiTrainerOutputLoads PROPERTIES
        DEPENDS MarioAiTrainerSmoke
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()
