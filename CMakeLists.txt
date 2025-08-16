cmake_minimum_required(VERSION 3.13.5)
project(MandelbrotProject VERSION 0.1.0)

# include(CTest)
# enable_testing()

# Set output directories for release and debug
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin-debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin-release)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/lib)
# include_directories(${CMAKE_SOURCE_DIR}/lib/GLFW/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/GLAD/include)

# Compile definitions
add_compile_definitions(MandelbrotProject GLFW_INCLUDE_NONE)

# Static analysis (only debug)
include(cmake/StaticAnalysis.cmake)

# Exectutable
add_executable(MandelbrotApp "")
target_sources(MandelbrotApp PRIVATE
    src/main.cpp

    src/shader.h
    src/shader.cpp
    src/app_utility.h
    src/app_utility.cpp
    src/ini_file.h
    src/saved_view.h
    src/saved_view.cpp

    lib/GLAD/glad.c
)

set_property(TARGET MandelbrotApp PROPERTY CXX_STANDARD 20)

# set compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(MandelbrotApp)

# sanitizer options if supported by compiler (only debug)
include(cmake/Sanitizers.cmake)
enable_sanitizers(MandelbrotApp)

# ImGui Library
add_library(ImGuiLib STATIC "")
target_sources(ImGuiLib PRIVATE
    lib/ImGui/imgui.h
    lib/ImGui/imgui.cpp
    lib/ImGui/imgui_internal.h
    lib/ImGui/imgui_tables.cpp
    lib/ImGui/imgui_widgets.cpp
    lib/ImGui/imgui_draw.cpp
    lib/ImGui/imstb_rectpack.h
    lib/ImGui/imstb_textedit.h
    lib/ImGui/imstb_truetype.h
    lib/ImGui/imconfig.h
    lib/ImGui/imgui_impl_opengl3.h
    lib/ImGui/imgui_impl_opengl3.cpp
    lib/ImGui/imgui_impl_glfw.cpp
    lib/ImGui/imgui_impl_glfw.h
)

# sanitizer options if supported by compiler (only debug)
enable_sanitizers(ImGuiLib)

#link ImGui
target_link_libraries(MandelbrotApp ImGuiLib)

# link glfw
find_package(glfw3 3.3 REQUIRED) # doesn't work (with X11)?
target_link_libraries(MandelbrotApp pthread glfw) # doesn't work (with X11)?
# target_link_libraries(MandelbrotApp pthread ${CMAKE_SOURCE_DIR}/lib/GLFW/bin/libglfw3.a)

# needed for glad (for opening libraries)
target_link_libraries(MandelbrotApp ${CMAKE_DL_LIBS})

# CPack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
