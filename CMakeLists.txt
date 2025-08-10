# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Define the project name
project(OpenGLShaderExample)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the GLFW library
# You must have GLFW installed on your system for this to work
find_package(glfw3 REQUIRED)

# Find the OpenGL library
find_package(OpenGL REQUIRED)

# Add the GLAD source file and include directory
# This assumes GLAD is located in a "glad" subfolder as per the structure above
include_directories(glad/include)

# Define the executable and its source files
# We include main.cpp and the GLAD C source file
add_executable(MyApp main.cpp glad/src/glad.c)

# Link the necessary libraries to our executable
target_link_libraries(MyApp PRIVATE glfw ${OPENGL_LIBRARIES})

# Copy the shaders directory to the build output directory
# This ensures the program can find the shader files at runtime
add_custom_command(TARGET MyApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    ${CMAKE_BINARY_DIR}/shaders
)