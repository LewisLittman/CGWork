# Invoking the CMake build from the command line is a two step process, first, *generate* a build by running the following:
#
#   cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Release
# 
# Where `Release` can be replaced with `Debug`, which contains gdb and address sanitiser definitions already written for you.
# Once a build is created, proceed with *compilation*:
# 
#   cmake --build build --target RunEngine --config Release # optionally, for parallel build, append -j $(nproc)
#
# This creates the executable in the build directory. You only need to *generate* a build if you modify the CMakeList.txt file.
# For any other changes to the source code, simply recompile.


cmake_minimum_required(VERSION 3.12)
project(RunEngine)

set(CMAKE_CXX_STANDARD 14)

# Note, we do this for glm because it's a header only library and because we shipped it with the project
# normally you would use find_package(<package_name>) for libraries with actual objects
set(GLM_INCLUDE_DIRS libs/glm-0.9.7.2)

find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    libs/sdw
    src
    src/camera
    src/scene
    src/render
    src/drawing
)

add_executable(RunEngine
        libs/sdw/CanvasPoint.cpp
        libs/sdw/CanvasTriangle.cpp
        libs/sdw/Colour.cpp
        libs/sdw/DrawingWindow.cpp
        libs/sdw/ModelTriangle.cpp
        libs/sdw/RayTriangleIntersection.cpp
        libs/sdw/TextureMap.cpp
        libs/sdw/TexturePoint.cpp
        libs/sdw/Utils.cpp
        src/camera/Camera.cpp
        src/drawing/Projection.cpp
        src/drawing/Line.cpp
        src/drawing/Fill.cpp
        src/render/PointCloudRenderer.cpp
        src/render/WireFrameRenderer.cpp
        src/render/RasterisedRenderer.cpp
        src/render/DepthBuffer.cpp
        src/render/RayTraceRenderer.cpp
        src/scene/Scene.cpp
        src/scene/SceneLoader.cpp
        src/scene/ObjParser.cpp
        src/scene/MtlParser.cpp
        src/other/Constants.cpp
        src/main.cpp
)

if (MSVC)
    target_compile_options(RunEngine
            PUBLIC
            /W3
            /Zc:wchar_t
            )
    set(DEBUG_OPTIONS /MTd)
    set(RELEASE_OPTIONS /MT /GF /Gy /O2 /fp:fast)
    if (NOT DEFINED SDL2_LIBRARIES)
        set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
    endif()
else ()
    target_compile_options(RunEngine
        PUBLIC
        -Wall
        -Wextra
        -Wcast-align
        -Wfatal-errors
        -Werror=return-type
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-ignored-attributes)

    set(DEBUG_OPTIONS -O2 -fno-omit-frame-pointer -g)
    set(RELEASE_OPTIONS -O3 -march=native -mtune=native)
    target_link_libraries(RunEngine PUBLIC $<$<CONFIG:Debug>:-Wl,-lasan>)

endif()


target_compile_options(RunEngine PUBLIC "$<$<CONFIG:RelWithDebInfo>:${RELEASE_OPTIONS}>")
target_compile_options(RunEngine PUBLIC "$<$<CONFIG:Release>:${RELEASE_OPTIONS}>")
target_compile_options(RunEngine PUBLIC "$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>")
 
target_link_libraries(RunEngine PRIVATE ${SDL2_LIBRARIES} Threads::Threads)
