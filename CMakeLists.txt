cmake_minimum_required(VERSION 3.10)
project(Advanced_OpenGL_Projects)

set(CMAKE_CXX_STANDARD 17)

# --------- 第三方库 ---------
find_package(assimp CONFIG REQUIRED)

# 统一的 include 路径（所有项目共享）
set(COMMON_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/third_party/glad/include
    ${CMAKE_SOURCE_DIR}/third_party/glfw/include
    ${CMAKE_SOURCE_DIR}/third_party/KHR
)

# glad 源文件（所有 target 共用）
set(GLAD_SRC ${CMAKE_SOURCE_DIR}/third_party/glad/glad.c)

# glfw 库所在目录
set(GLFW_LIB_DIR ${CMAKE_SOURCE_DIR}/third_party/glfw/lib)

# --------- 封装一个函数：添加一个 OpenGL 示例工程 ----------
function(add_gl_example TARGET_NAME SRC_DIR)
    # 找到该子目录下所有 cpp
    file(GLOB EXAMPLE_SOURCES
        ${SRC_DIR}/*.cpp
    )

    add_executable(${TARGET_NAME}
        ${EXAMPLE_SOURCES}
        ${GLAD_SRC}
    )

    # 统一 include
    target_include_directories(${TARGET_NAME} PRIVATE
        ${COMMON_INCLUDE_DIRS}
    )

    # 统一库路径 & 链接
    target_link_directories(${TARGET_NAME} PRIVATE
        ${GLFW_LIB_DIR}
    )

    target_link_libraries(${TARGET_NAME}
        opengl32
        glfw3
        assimp::assimp
    )

    # 将资源文件 media 复制到可执行文件同级目录
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/media/Texture"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/Texture"

        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/media/Shader"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/Shader"

        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/media/Model"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/Model"
)

endfunction()

# --------- 在这里登记每一个子项目 -----------

add_gl_example(00_Example
    "${CMAKE_SOURCE_DIR}/src/00_Example"
)

add_gl_example(01_Depth_StencilTest
    "${CMAKE_SOURCE_DIR}/src/01_Depth_StencilTest"
)

add_gl_example(02_FrameBufferObject
    "${CMAKE_SOURCE_DIR}/src/02_FrameBufferObject"
)

add_gl_example(03_Deferred_Rendering
    "${CMAKE_SOURCE_DIR}/src/03_Deferred_Rendering"
)

add_gl_example(04_SSAO
    "${CMAKE_SOURCE_DIR}/src/04_SSAO"
)

