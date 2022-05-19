cmake_minimum_required(VERSION 3.10)

set(ProjectName "OpenGL-Environment")

#windows only
set(Lib "D:/noam4/Libraries/VS/lib/x64/")
set(Include "D:/noam4/Libraries/VS/include")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# set the project name
project(${ProjectName} VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

#packages
if(UNIX)
    find_package(glfw3 3.3 REQUIRED)
    find_package(Freetype REQUIRED)
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
    target_include_directories(${ProjectName} PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(${ProjectName} glfw
                                         assimp
                                         ${FREETYPE_LIBRARIES}
                                         sndfile
                                         portaudio
                                         ${CMAKE_DL_LIBS})
elseif(MSVC)
    target_include_directories(${ProjectName} PUBLIC ${Include})
    target_link_libraries(${ProjectName} ${Lib}glfw3.lib
                                         ${Lib}freetype.lib
                                         ${Lib}assimp-vc140-mt.lib
                                         ${Lib}portaudio_x64.lib
                                         ${Lib}sndfile.lib)
endif()
