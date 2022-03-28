cmake_minimum_required(VERSION 3.10)

set(ProjectName "OpenGL-Environment")
# set the project name
project(${ProjectName} VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

#packages

find_package(glfw3 3.3 REQUIRED)


#add executables
file(GLOB SOURCES src/*)
file(GLOB OGL_RENDER src/opengl-render/*)
add_executable(${ProjectName} ${SOURCES}
                              ${OGL_RENDER}
                              ${PROJ_INCLUDE})

#link
target_include_directories(${ProjectName} PUBLIC include/)
target_link_libraries(${ProjectName} glfw)
