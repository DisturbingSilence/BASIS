cmake_minimum_required(VERSION 3.28)
project(BASIS VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

include(bundle.cmake)

add_subdirectory(external)
add_library(lib_basis STATIC
	src/app.cpp
	src/pipeline.cpp
	src/buffer.cpp
	src/manager.cpp
	src/texture.cpp
	src/rendering.cpp
	src/context.cpp
	src/timer.cpp
)

# requires "ar" tool
option(BUNDLE_STATIC_LIBS "Bundle together all third party dependencies" ON)

target_link_libraries(lib_basis PUBLIC glm::glm imgui glfw)
target_link_libraries(lib_basis PRIVATE fastgltf glad simdjson ktx)

target_include_directories(lib_basis PUBLIC include external)

if(BUNDLE_STATIC_LIBS)
	bundle_static_library(lib_basis lib_basis_bundled)
endif()
