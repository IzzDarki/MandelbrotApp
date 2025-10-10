# cmake/Sanitizers.cmake
function(enable_sanitizers project_name)
  if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang"))
    return()
  endif()

  option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)

  if(ENABLE_COVERAGE)
    target_compile_options(${project_name} PRIVATE "$<$<CONFIG:Debug>:--coverage -O0 -g>")
    target_link_libraries(${project_name} PRIVATE "$<$<CONFIG:Debug>:--coverage>")
  endif()

  set(SANITIZERS "")

  option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" FALSE)
  if(ENABLE_SANITIZER_ADDRESS)
    list(APPEND SANITIZERS "address")
  endif()

  option(ENABLE_SANITIZER_LEAK "Enable leak sanitizer" FALSE)
  if(ENABLE_SANITIZER_LEAK)
    list(APPEND SANITIZERS "leak")
  endif()

  option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitizer" FALSE)
  if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
    list(APPEND SANITIZERS "undefined")
  endif()

  option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" FALSE)
  if(ENABLE_SANITIZER_THREAD)
    if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
      message(WARNING "Thread sanitizer does not work with Address and Leak sanitizer enabled")
    else()
      list(APPEND SANITIZERS "thread")
    endif()
  endif()

  option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer (Clang only)" FALSE)
  if(ENABLE_SANITIZER_MEMORY AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    if("address" IN_LIST SANITIZERS OR "thread" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
      message(WARNING "Memory sanitizer does not work with Address, Thread and Leak sanitizer enabled")
    else()
      list(APPEND SANITIZERS "memory")
    endif()
  endif()

  list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)

  if(LIST_OF_SANITIZERS)
    if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
      target_compile_options(${project_name} PUBLIC "$<$<CONFIG:Debug>:-fsanitize=${LIST_OF_SANITIZERS}>")
      target_link_options(${project_name} PUBLIC "$<$<CONFIG:Debug>:-fsanitize=${LIST_OF_SANITIZERS}>")
    endif()
  endif()

  # LeakSanitizer suppressions
  if("leak" IN_LIST SANITIZERS)
    # Path to the suppression file (in your source or build directory)
    set(LSAN_SUPPRESSIONS_FILE "${CMAKE_SOURCE_DIR}/cmake/lsan.supp")

    # Only set if the file exists — avoids warnings in builds without it
    if(EXISTS "${LSAN_SUPPRESSIONS_FILE}")
      message(STATUS "Using LeakSanitizer suppressions from ${LSAN_SUPPRESSIONS_FILE}")
      set(LSAN_ENV "LSAN_OPTIONS=suppressions=${LSAN_SUPPRESSIONS_FILE}:print_suppressions=0")
      set_property(TARGET ${project_name} APPEND PROPERTY VS_DEBUGGER_ENVIRONMENT "${LSAN_ENV}")
      set_target_properties(${project_name} PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "${LSAN_ENV}"
      )

      # For normal runs (Linux/macOS)
      add_custom_target(run_${project_name}
        COMMAND ${LSAN_ENV} $<TARGET_FILE:${project_name}>
        DEPENDS ${project_name}
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
        COMMENT "Running ${project_name} with LeakSanitizer suppressions"
      )
    else()
      message(STATUS "LeakSanitizer running without suppressions, since no file was given at ${LSAN_SUPPRESSIONS_FILE}")
    endif()
  endif()


endfunction()
