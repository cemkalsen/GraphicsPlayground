cmake_minimum_required(VERSION 3.22)

project(GraphicsPlayground LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

# Assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(external/assimp)

# Renderer
add_executable(Renderer
    src/main.cpp
    src/stb_image.cpp

    # GLAD
    external/glad/src/glad.c

    # ImGui core
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp

    # ImGui backends
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp

    # Project headers
    src/shader.h
    src/camera.h
    src/mesh.h
    src/model.h
)

target_include_directories(Renderer PRIVATE
    external/glad/include
    external/glm

    # ImGui
    external/imgui
    external/imgui/backends
)

target_link_libraries(Renderer PRIVATE
    glfw
    opengl32
    assimp::assimp
)

# Copy runtime assets next to Renderer.exe
add_custom_command(TARGET Renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:Renderer>/assets
)