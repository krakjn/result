cmake_minimum_required(VERSION 3.23)
project(result LANGUAGES CXX)

# Header-only. The examples and tests are the only translation units.

add_library(result INTERFACE)
target_sources(result INTERFACE
  FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
  FILES result.h
)
target_include_directories(result INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(result INTERFACE cxx_std_11)

install(TARGETS result FILE_SET HEADERS)

enable_testing()

set(RESULT_CXX_STANDARD "" CACHE STRING
  "Build every program at this standard, skipping those that need newer. Empty: each at its minimum.")

function(result_program name source standard)
  if(RESULT_CXX_STANDARD)
    if(standard GREATER RESULT_CXX_STANDARD)
      return()
    endif()
    set(standard ${RESULT_CXX_STANDARD})
  endif()
  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE result)
  set_target_properties(${name} PROPERTIES
    CXX_STANDARD ${standard}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
  )
  if(MSVC)
    target_compile_options(${name} PRIVATE /W4 /WX)
  else()
    target_compile_options(${name} PRIVATE -Wall -Wextra -pedantic -Werror)
  endif()
  add_test(NAME ${name} COMMAND ${name})
endfunction()

foreach(example IN ITEMS
  and_then
  arithmetic
  complex_data
  defaults
  error_codes
  file_operation
  move_only
  or_else
  transform
  try
  void_result
)
  result_program(${example} examples/${example}.cpp 11)
endforeach()

result_program(optional examples/optional.cpp 17)

if(RESULT_CXX_STANDARD)
  result_program(api tests/api.cpp ${RESULT_CXX_STANDARD})
else()
  foreach(standard IN ITEMS 11 14 17 20)
    result_program(api_cxx${standard} tests/api.cpp ${standard})
  endforeach()
endif()
