cmake_minimum_required(VERSION 3.16)
project(cpp_reportlib VERSION 0.1.0 LANGUAGES CXX)

add_library(reportlib INTERFACE)
add_library(reportlib::reportlib ALIAS reportlib)
target_include_directories(reportlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(reportlib INTERFACE cxx_std_17)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(_reportlib_standalone ON)
else()
  set(_reportlib_standalone OFF)
endif()
option(REPORTLIB_BUILD_TESTS "Build tests" ${_reportlib_standalone})
option(REPORTLIB_BUILD_EXAMPLES "Build examples" ${_reportlib_standalone})

if(REPORTLIB_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    doctest
    GIT_REPOSITORY "https://github.com/doctest/doctest.git"
    GIT_TAG "v2.5.3"
  )
  FetchContent_MakeAvailable(doctest)
  enable_testing()
  add_executable(reportlib_tests tests/renderers.cpp)
  target_link_libraries(reportlib_tests PRIVATE reportlib::reportlib doctest::doctest)
  add_test(NAME reportlib_tests COMMAND reportlib_tests)
endif()

if(REPORTLIB_BUILD_EXAMPLES)
  add_executable(atmospheric_report examples/atmospheric.cpp)
  target_link_libraries(atmospheric_report PRIVATE reportlib::reportlib)
endif()

if(REPORTLIB_BUILD_EXAMPLES)
  find_program(REPORTLIB_PDFLATEX pdflatex)
  if(REPORTLIB_PDFLATEX)
    add_custom_target(reportlib_demo
      COMMAND ${CMAKE_COMMAND}
        -DREPORTLIB_EXAMPLE=$<TARGET_FILE:atmospheric_report>
        -DREPORTLIB_FIGURE=${CMAKE_CURRENT_SOURCE_DIR}/examples/output/density.png
        -DREPORTLIB_PDFLATEX=${REPORTLIB_PDFLATEX}
        -DREPORTLIB_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/demo
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/all_formats.cmake
      DEPENDS atmospheric_report
      COMMENT "Generating Markdown, RST, LaTeX, and PDF demo reports"
      VERBATIM
    )
    if(REPORTLIB_BUILD_TESTS)
      add_test(
        NAME reportlib_all_formats
        COMMAND ${CMAKE_COMMAND}
          -DREPORTLIB_EXAMPLE=$<TARGET_FILE:atmospheric_report>
          -DREPORTLIB_FIGURE=${CMAKE_CURRENT_SOURCE_DIR}/examples/output/density.png
          -DREPORTLIB_PDFLATEX=${REPORTLIB_PDFLATEX}
          -DREPORTLIB_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/integration
          -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/all_formats.cmake
      )
    endif()
  endif()
endif()
