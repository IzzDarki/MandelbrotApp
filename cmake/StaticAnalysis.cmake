# cmake/StaticAnalysis.cmake
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)

# Call this AFTER you create a target. It will apply clang-tidy for Debug config only.
function(enable_clang_tidy_for_target target_name)
  if(NOT ENABLE_CLANG_TIDY)
    return()
  endif()

  find_program(CLANG_TIDY_EXE NAMES clang-tidy)
  if(NOT CLANG_TIDY_EXE)
    message(FATAL_ERROR "clang-tidy requested but executable not found")
  endif()

  # Apply clang-tidy only for Debug config using generator expression
  set_property(TARGET ${target_name} PROPERTY
    CXX_CLANG_TIDY "$<$<CONFIG:Debug>:${CLANG_TIDY_EXE};-extra-arg=-Wno-unknown-warning-option>"
  )
endfunction()
