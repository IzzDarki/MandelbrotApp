cmake_minimum_required(VERSION 3.13.5)
project(MandelbrotProject VERSION 0.1.0 LANGUAGES C CXX)

# -----------------------------------------------------------------------------
# Options (toggle these with -D on cmake command line)
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
# Sanitizer options are handled inside cmake/Sanitizers.cmake via options
# -----------------------------------------------------------------------------

# Executable and sources
add_executable(MandelbrotApp
    src/main.cpp
    src/shader.cpp
    src/app_utility.cpp
    src/saved_view.cpp
    src/screenshot.cpp
    src/model/model.cpp
    src/model/model_rk45.cpp
    src/model/model_super_sampling.cpp
    src/model/model_double_pendulum.cpp
    src/model/model_mandelbrot.cpp
)

# optional: headers for IDE visibility
target_sources(MandelbrotApp PRIVATE
    src/shader.h
    src/app_utility.h
    src/ini_file.h
    src/saved_view.h
    src/screenshot.h
    src/model/model.h
    src/model/model_rk45.h
    src/model/model_super_sampling.h
    src/model/model_double_pendulum.h
    src/model/model_mandelbrot.h
)

# C++ standard
set_property(TARGET MandelbrotApp PROPERTY CXX_STANDARD 20)

# -----------------------------------------------------------------------------
# Target-scoped includes and defines (DO NOT use global include_directories / add_compile_definitions)
target_include_directories(MandelbrotApp
  # PRIVATE # my own include paths
    # nothing here
  SYSTEM PRIVATE # library include paths (=> no warnings from them)
    ${CMAKE_SOURCE_DIR}/lib
    # ${CMAKE_SOURCE_DIR}/lib/GLAD/include
)


target_compile_definitions(MandelbrotApp PRIVATE GLFW_INCLUDE_NONE)

# -----------------------------------------------------------------------------
# Output directory per configuration (multi-config friendly)
# e.g. bin-Release, bin-Debug
set_target_properties(MandelbrotApp PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin-$<CONFIG>
)

# -----------------------------------------------------------------------------
# Warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(MandelbrotApp)

# Sanitizers (function will add generator-expr-based options so they only apply in Debug)
include(cmake/Sanitizers.cmake)
enable_sanitizers(MandelbrotApp)

# Static analysis (clang-tidy) applied per-target for Debug via generator-expression
include(cmake/StaticAnalysis.cmake)
if(ENABLE_CLANG_TIDY)
  enable_clang_tidy_for_target(MandelbrotApp)
endif()



# -----------------------------------------------------------------------------
# Dependencies

# Find and link system libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED) # system glfw3

# prefer imported target if available
if(TARGET glfw::glfw)
  set(GLFW_TARGET glfw::glfw)
elseif(TARGET glfw)
  set(GLFW_TARGET glfw)
else()
  set(GLFW_TARGET glfw)
endif()

# ImGui Library (static) and GLAD
add_library(ImGuiLib STATIC
    lib/ImGui/imgui.cpp
    lib/ImGui/imgui_tables.cpp
    lib/ImGui/imgui_widgets.cpp
    lib/ImGui/imgui_draw.cpp
    lib/ImGui/imgui_impl_opengl3.cpp
    lib/ImGui/imgui_impl_glfw.cpp
)

# Make sure ImGui has its headers and the GL loader header (glad) available
target_include_directories(ImGuiLib
  PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/ImGui
    ${CMAKE_SOURCE_DIR}/lib/GLAD/include   # <-- provide glad.h
)

# Tell ImGui backend to use GLAD as the loader
target_compile_definitions(ImGuiLib PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

# If needed (optional), keep ImGui-specific defs
# target_compile_definitions(ImGuiLib PRIVATE IMGUI_IMPL_OPENGL_LOADER_CUSTOM="\"path/to/custom_loader.h\"")

# GLAD
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/lib/GLAD/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/lib/GLAD/include)
# if GLAD needs any compile defs, add them:
# target_compile_definitions(glad PRIVATE SOME_DEF_IF_NEEDED)

# Link glad into ImGuiLib (so backend links to implementation,
# and final executable will also link glad via ImGuiLib dependency)
target_link_libraries(ImGuiLib PUBLIC glad ${GLFW_TARGET})

# Also link glad into your final executable explicitly (safe)
target_link_libraries(MandelbrotApp PRIVATE
  ImGuiLib       # brings glad transitively
  ${GLFW_TARGET}
  OpenGL::GL
  ${CMAKE_DL_LIBS}
)

# On Unix, prefer Threads::Threads (this is portable)
if(UNIX AND NOT APPLE)
  find_package(Threads REQUIRED)
  target_link_libraries(MandelbrotApp PRIVATE Threads::Threads)
endif()

# -----------------------------------------------------------------------------
# CPack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
