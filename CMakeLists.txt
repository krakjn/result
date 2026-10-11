cmake_minimum_required(VERSION 3.23)
project(result LANGUAGES CXX)

# Header-only. The examples are the only translation units.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

install(TARGETS result FILE_SET HEADERS)

set(RESULT_EXAMPLES
  and_then
  arithmetic
  complex_data
  error_codes
  file_operation
  or_else
  transform
)

enable_testing()

foreach(example IN LISTS RESULT_EXAMPLES)
  add_executable(${example} examples/${example}.cpp)
  target_link_libraries(${example} PRIVATE result)
  target_compile_options(${example} PRIVATE
    -Wall
    -Wextra
    -pedantic
    -Werror
  )
  add_test(NAME ${example} COMMAND ${example})
endforeach()
