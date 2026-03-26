cmake_minimum_required(VERSION 3.16)
project(PhysicsSimulator)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL3 REQUIRED)

add_executable(simulator main.cpp)
target_link_libraries(simulator SDL3::SDL3)

add_executable(assign_colors assign_colors.cpp)

add_executable(generate_scene generate_scene.cpp)
