cmake_minimum_required(VERSION 3.22)
project(GraphicsPlayground)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# Assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/assimp)

add_executable(Renderer
    src/main.cpp
    src/stb_image.cpp
    external/glad/src/glad.c
    src/shader.h
    src/camera.h
    src/mesh.h
    src/model.h
)

target_include_directories(Renderer PRIVATE
    external/glad/include
    external/glm
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