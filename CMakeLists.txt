# Specify minimum supported cmake version
cmake_minimum_required(VERSION 3.5)

# set the project name
project(FantaInMySystem)

## C++ 17
##set(CMAKE_CXX_STANDARD 17)
##set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# Set default build type to Release if not specified
#if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
#endif()

option(ASSIMP_BUILD_TESTS OFF)
option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)

# add_subdirectorys to be CMaked
add_subdirectory(ext/kgfw)
add_subdirectory(ext/glfw-3.3.2)
add_subdirectory(ext/glm-0.9.9.7)
add_subdirectory(ext/assimp-5.0.1)

# Specify include directories to include glad
include_directories("${GLFW_SOURCE_DIR}/deps")

# Specify the stb-image include directory
include_directories("ext/stb-master")

# Specify the ImGui include directory
include_directories("ext/imgui")

# Find all header files
file(GLOB_RECURSE HDR_FILES "src/*.h")
source_group("Header Files", FILES ${HDR_FILES})

# Add ImGui source files
file(GLOB_RECURSE IMGUI_SOURCES 
    "ext/imgui/*.cpp"
)
# source_group makes a folder for the files
source_group("ImGui" FILES ${IMGUI_SOURCES})

# Define glad source files
set(GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h" "${GLFW_SOURCE_DIR}/deps/glad_gl.c")
source_group("Glad" FILES ${GLAD_GL})

# Set the assets directory relative to the project root
set(ASSET_DIR "${CMAKE_SOURCE_DIR}/assets")

# Add this as a preprocessor definition
add_compile_definitions(ASSET_DIR="${ASSET_DIR}")

# Find all source files in the src folder
# file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/*.h")
file(GLOB_RECURSE SRC_FILES "src/*.cpp")
source_group("Source Files", FILES ${SRC_FILES})

# add the executables
add_executable(FantaInMySystem ${HDR_FILES} ${SRC_FILES} ${GLAD_GL} ${IMGUI_SOURCES})

# Link game_object-executables with needed libraries. Since the kgfw and glfw libraries 
# has public include directories we will use those link directories when building binaries.
target_link_libraries(FantaInMySystem PUBLIC kgfw PUBLIC glfw PUBLIC glm PUBLIC assimp)

# Additional just in case
find_package(OpenGL REQUIRED)
target_link_libraries(FantaInMySystem PUBLIC OpenGL::GL)

