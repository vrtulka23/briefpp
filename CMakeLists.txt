cmake_minimum_required(VERSION 3.16)
project(briefpp VERSION 0.1.0 LANGUAGES CXX)

add_library(briefpp INTERFACE)
add_library(briefpp::briefpp ALIAS briefpp)
target_include_directories(briefpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(briefpp INTERFACE cxx_std_17)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(_briefpp_standalone ON)
else()
  set(_briefpp_standalone OFF)
endif()
option(BRIEFPP_BUILD_TESTS "Build tests" ${_briefpp_standalone})
option(BRIEFPP_BUILD_EXAMPLES "Build examples" ${_briefpp_standalone})

if(BRIEFPP_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    doctest
    GIT_REPOSITORY "https://github.com/doctest/doctest.git"
    GIT_TAG "v2.5.3"
  )
  FetchContent_MakeAvailable(doctest)
  enable_testing()
  add_executable(briefpp_tests tests/renderers.cpp)
  target_link_libraries(briefpp_tests PRIVATE briefpp::briefpp doctest::doctest)
  add_test(NAME briefpp_tests COMMAND briefpp_tests)
endif()

if(BRIEFPP_BUILD_EXAMPLES)
  add_executable(atmospheric_report examples/atmospheric.cpp)
  target_link_libraries(atmospheric_report PRIVATE briefpp::briefpp)
endif()

if(BRIEFPP_BUILD_EXAMPLES)
  find_program(BRIEFPP_PDFLATEX pdflatex)
  add_custom_target(briefpp_demo
    COMMAND ${CMAKE_COMMAND}
      -DBRIEFPP_EXAMPLE=$<TARGET_FILE:atmospheric_report>
      -DBRIEFPP_FIGURE=${CMAKE_CURRENT_SOURCE_DIR}/examples/output/density.png
      -DBRIEFPP_PDFLATEX=${BRIEFPP_PDFLATEX}
      -DBRIEFPP_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/demo
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/all_formats.cmake
    DEPENDS atmospheric_report
    COMMENT "Generating all text formats and optional PDF demo"
    VERBATIM
  )
  if(BRIEFPP_BUILD_TESTS)
    add_test(
      NAME briefpp_all_formats
      COMMAND ${CMAKE_COMMAND}
        -DBRIEFPP_EXAMPLE=$<TARGET_FILE:atmospheric_report>
        -DBRIEFPP_FIGURE=${CMAKE_CURRENT_SOURCE_DIR}/examples/output/density.png
        -DBRIEFPP_PDFLATEX=${BRIEFPP_PDFLATEX}
        -DBRIEFPP_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/integration
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/all_formats.cmake
    )
  endif()
endif()
