cmake_minimum_required(VERSION 3.15)
project(the_first_era)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(the_first_era ${SOURCES})

target_link_libraries(the_first_era sfml-graphics sfml-window sfml-system)

target_compile_definitions(the_first_era PRIVATE
    PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/config")
