# Specify minimum supported cmake version
cmake_minimum_required(VERSION 3.5)

# Add all .cpp files from folder "src".
file(GLOB KGFW_SOURCES "src/*.cpp")

# Add all .h files from folder "include/kgfw".
file(GLOB KGFW_HEADERS "include/kgfw/*.h")

# Create a library called kgfw.
add_library(kgfw ${KGFW_SOURCES} ${KGFW_HEADERS})

# Add include directories to a target.
target_include_directories(kgfw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)