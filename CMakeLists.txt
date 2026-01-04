cmake_minimum_required(VERSION 3.10)
project(DysonSphere)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(target "DysonSphere")

# 收集源文件
# file(GLOB_RECURSE src 
#     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
#     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
# )
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/third/src third_src)

# 添加执行依赖
add_executable(${target} ${src} ${third_src})

# 添加头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third/include)

# 添加库链接路径
target_link_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third/lib)
target_link_libraries(${target} glfw3)

# macOS特定的框架链接
if(APPLE)
    target_link_libraries(${target}
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework CoreVideo"
        "-framework CoreGraphics"
    )
endif()

# Create symlink for assets in build directory (Better for development)
# This allows editing shaders/textures without rebuilding
add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_CURRENT_BINARY_DIR}/assets)

# 若需链接Windows或Linux特定框架，请在下面自行添加
