cmake_minimum_required(VERSION 3.10)

set(ProjectName "OpenGL-Environment")

set(Lib "D:/noam4/Libraries/VS/lib/x64/")
set(Include "D:/noam4/Libraries/VS/include")

# set the project name
project(${ProjectName} VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

#packages
if(UNIX)
    find_package(glfw3 3.3 REQUIRED)
endif()

#add executables
file(GLOB SOURCES src/*)
file(GLOB OGL_RENDER src/opengl-render/*)
file(GLOB OGL_RENDER_RESOURCES src/opengl-render/resources/*)
add_executable(${ProjectName}   ${SOURCES}
                                ${OGL_RENDER}
                                ${OGL_RENDER_RESOURCES}
                                ${PROJ_INCLUDE})

#link
target_include_directories(${ProjectName} PUBLIC include/)
if(UNIX)
    target_link_libraries(${ProjectName} glfw)
elseif(MSVC)
    target_include_directories(${ProjectName} PUBLIC ${Include})
    target_link_libraries(${ProjectName} ${Lib}glfw3.lib)
endif()