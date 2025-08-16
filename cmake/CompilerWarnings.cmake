# cmake/CompilerWarnings.cmake
function(set_project_warnings project_name)
  option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
  option(WARNINGS_OFF "Turn warnings off" OFF)
  option(WARN_CSTYLE_CASTS "Warn for C-style casts" OFF)

  if(WARNINGS_OFF)
    return()
  endif()

  # Prepare warning lists
  set(MSVC_WARNINGS
    /W4
    /w14242
    /w14254
    /w14263
    /w14265
    /w14287
    /we4289
    /w14296
    /w14311
    /w14545
    /w14546
    /w14547
    /w14549
    /w14555
    /w14619
    /w14640
    /w14826
    /w14905
    /w14906
    /w14928
    /permissive-
  )

  set(CLANG_WARNINGS
    -Wall
    -Wextra
    -Wshadow
    -Wnon-virtual-dtor
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
  )

  if(WARN_CSTYLE_CASTS)
    list(APPEND CLANG_WARNINGS -Wold-style-cast)
  endif()

  set(GCC_WARNINGS ${CLANG_WARNINGS})
  list(APPEND GCC_WARNINGS
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast
  )

  if(WARNINGS_AS_ERRORS)
    list(APPEND CLANG_WARNINGS -Werror)
    list(APPEND MSVC_WARNINGS /WX)
  endif()

  if(MSVC)
    set(PROJECT_WARNINGS ${MSVC_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(PROJECT_WARNINGS ${GCC_WARNINGS})
  else()
    message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    set(PROJECT_WARNINGS "")
  endif()

  if(NOT PROJECT_WARNINGS STREQUAL "")
    target_compile_options(${project_name} PUBLIC ${PROJECT_WARNINGS})
  endif()
endfunction()
