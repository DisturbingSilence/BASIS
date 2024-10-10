cmake_minimum_required(VERSION 3.28)
project(BASIS VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external)
add_library(lib_basis
	src/app.cpp
	src/pipeline.cpp
	src/buffer.cpp
	src/manager.cpp
	src/texture.cpp
	src/rendering.cpp
	src/context.cpp
	src/timer.cpp
)

target_link_libraries(lib_basis PUBLIC glm::glm imgui glfw)
target_link_libraries(lib_basis PRIVATE glfw fastgltf glad simdjson)

target_include_directories(lib_basis PUBLIC include external)

