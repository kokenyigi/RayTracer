cmake_minimum_required(VERSION 3.16)
project(MyApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
    src/*.cpp
	thirdParty/STB_IMAGE/src/*.cpp
	thirdParty/IMGUI/src/*.cpp
)

add_executable(MyApp ${SOURCES})

target_include_directories(MyApp PRIVATE
    include
    thirdParty/GLM/include
    thirdParty/GLFW/include
    thirdParty/GLEW/include
    thirdParty/STB_IMAGE/include
	thirdParty/IMGUI/include
	thirdParty/OPENCL/include
)


target_compile_definitions(MyApp PRIVATE
    GLEW_STATIC
    WIN32
)

target_link_directories(MyApp PRIVATE
    thirdParty/GLFW/lib-vc2022
    thirdParty/GLEW/lib/Release/Win32
	thirdParty/OPENCL/lib/Win32
)

target_link_libraries(MyApp PRIVATE
    glfw3_mt.lib
    glew32s.lib
	OpenCL.lib
    opengl32
)

set_target_properties(MyApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}
)

if (MSVC)
    set_property(TARGET MyApp PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()