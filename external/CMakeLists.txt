add_library(glad glad/src/gl.c)
target_include_directories(glad PUBLIC glad/include)

include(FetchContent)

option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
option(GLFW_BUILD_EXAMPLES "" OFF)

FetchContent_Declare(
    	glfw3
    	GIT_REPOSITORY https://github.com/glfw/glfw
    	GIT_TAG        3.4
)

FetchContent_Declare(
    	glm
    	GIT_REPOSITORY https://github.com/g-truc/glm
    	GIT_TAG        1.0.1
)
FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson
    GIT_TAG        v3.9.4
)
FetchContent_Declare(
    	fastgltf
    	GIT_REPOSITORY https://github.com/spnda/fastgltf
    	GIT_TAG        v0.8.0
)

FetchContent_Declare(
	imgui_internal
	GIT_REPOSITORY https://github.com/ocornut/imgui
	GIT_TAG        v1.91.2
)
FetchContent_Declare(
	stbi
	GIT_REPOSITORY https://github.com/nothings/stb
)
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/external/stb_image.h)
file(
	DOWNLOAD 
	https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h
	${PROJECT_SOURCE_DIR}/external/stb_image.h
	STATUS stb_image
)
if(NOT stb_image)
	message("Failed to download stb_image.h")
endif()
endif()
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/external/stb_image_write.h)
file(
	DOWNLOAD 
	https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image_write.h
	${PROJECT_SOURCE_DIR}/external/stb_image_write.h
	STATUS stb_write
)
if(NOT stb_write)
	message("Failed to download stb_image_write.h")
endif()
endif()
FetchContent_MakeAvailable(glm simdjson fastgltf glfw3 imgui_internal)

add_library(imgui STATIC
	${imgui_internal_SOURCE_DIR}/imgui.cpp
	${imgui_internal_SOURCE_DIR}/imgui_draw.cpp
	${imgui_internal_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_internal_SOURCE_DIR}/imgui_tables.cpp
	${imgui_internal_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
	${imgui_internal_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)
target_include_directories(imgui PUBLIC 
	${imgui_internal_SOURCE_DIR} 
	${imgui_internal_SOURCE_DIR}/backends
	${glfw3_SOURCE_DIR}/include
)
target_link_libraries(imgui PRIVATE glfw)
target_compile_definitions(glm INTERFACE GLM_FORCE_SILENT_WARNINGS)
