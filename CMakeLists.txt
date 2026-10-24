cmake_minimum_required(VERSION 3.20)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" SLAMHAUS_VERSION LIMIT_COUNT 1)
project(Slamhaus VERSION ${SLAMHAUS_VERSION} LANGUAGES C CXX)

include(FetchContent)
include(CTest)
include(GNUInstallDirs)

# Older pinned dependencies predate CMake 4's policy floor.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(nlohmann_json 3.12 QUIET)
if(NOT nlohmann_json_FOUND)
  FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(nlohmann_json)
endif()

find_package(cmark-gfm 0.29 QUIET)
if(NOT cmark-gfm_FOUND)
  set(CMARK_TESTS OFF CACHE BOOL "" FORCE)
  set(CMARK_SHARED OFF CACHE BOOL "" FORCE)
  set(CMARK_STATIC ON CACHE BOOL "" FORCE)
  FetchContent_Declare(
    cmark-gfm
    GIT_REPOSITORY https://github.com/github/cmark-gfm.git
    GIT_TAG 0.29.0.gfm.13
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(cmark-gfm)
  set_property(DIRECTORY "${cmark-gfm_SOURCE_DIR}" PROPERTY EXCLUDE_FROM_ALL TRUE)
endif()

find_package(yaml-cpp 0.9 QUIET)
if(NOT yaml-cpp_FOUND)
  set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
  set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG yaml-cpp-0.9.0
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(yaml-cpp)
endif()

# Generated parsers are compiled into the CLI. No Tree-sitter CLI or editor
# extension is required on an installed system.
FetchContent_Declare(
  tree_sitter_source
  URL https://github.com/tree-sitter/tree-sitter/archive/refs/tags/v0.26.9.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SOURCE_SUBDIR slamhaus-no-cmake)
FetchContent_Declare(
  tree_sitter_cpp_source
  URL https://github.com/tree-sitter/tree-sitter-cpp/archive/refs/tags/v0.23.4.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SOURCE_SUBDIR slamhaus-no-cmake)
FetchContent_Declare(
  tree_sitter_python_source
  URL https://github.com/tree-sitter/tree-sitter-python/archive/refs/tags/v0.25.0.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SOURCE_SUBDIR slamhaus-no-cmake)
FetchContent_Declare(
  tree_sitter_c_sharp_source
  URL https://github.com/tree-sitter/tree-sitter-c-sharp/archive/refs/tags/v0.23.5.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SOURCE_SUBDIR slamhaus-no-cmake)
FetchContent_MakeAvailable(tree_sitter_source tree_sitter_cpp_source
  tree_sitter_python_source tree_sitter_c_sharp_source)
add_library(slamhaus_tree_sitter STATIC
  "${tree_sitter_source_SOURCE_DIR}/lib/src/lib.c")
target_include_directories(slamhaus_tree_sitter PUBLIC
  "${tree_sitter_source_SOURCE_DIR}/lib/include"
  PRIVATE "${tree_sitter_source_SOURCE_DIR}/lib/src")
set_target_properties(slamhaus_tree_sitter PROPERTIES C_STANDARD 11)
add_library(slamhaus_tree_sitter_grammars STATIC
  "${tree_sitter_cpp_source_SOURCE_DIR}/src/parser.c"
  "${tree_sitter_cpp_source_SOURCE_DIR}/src/scanner.c"
  "${tree_sitter_python_source_SOURCE_DIR}/src/parser.c"
  "${tree_sitter_python_source_SOURCE_DIR}/src/scanner.c"
  "${tree_sitter_c_sharp_source_SOURCE_DIR}/src/parser.c"
  "${tree_sitter_c_sharp_source_SOURCE_DIR}/src/scanner.c")
target_include_directories(slamhaus_tree_sitter_grammars PRIVATE
  "${tree_sitter_cpp_source_SOURCE_DIR}/src"
  "${tree_sitter_python_source_SOURCE_DIR}/src"
  "${tree_sitter_c_sharp_source_SOURCE_DIR}/src")
set_target_properties(slamhaus_tree_sitter_grammars PROPERTIES C_STANDARD 11)

configure_file(src/version.hpp.in generated/version.hpp @ONLY)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/sdk/slam/slam.hpp" SLAMHAUS_SDK_HEADER)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/runtime.js" SLAMHAUS_RUNTIME_SOURCE)
configure_file(src/wasm/sdk.hpp.in generated/wasm_sdk.hpp @ONLY)
configure_file(src/wasm/runtime.hpp.in generated/wasm_runtime.hpp @ONLY)

add_library(slamhaus_lib
  src/core/project.cpp
  src/language/language.cpp
  src/lsp/lsp.cpp
  src/syntax/syntax.cpp
  src/toolchain/toolchain.cpp
  src/util/process.cpp
  src/wasm/wasm.cpp)
target_include_directories(slamhaus_lib PUBLIC src "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(slamhaus_lib PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(slamhaus_lib PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(slamhaus_lib PRIVATE slamhaus_tree_sitter
  slamhaus_tree_sitter_grammars)
if(TARGET libcmark-gfm_static)
  target_link_libraries(slamhaus_lib PRIVATE libcmark-gfm-extensions_static libcmark-gfm_static)
  target_include_directories(slamhaus_lib PRIVATE
    "${cmark-gfm_SOURCE_DIR}/src"
    "${cmark-gfm_SOURCE_DIR}/extensions"
    "${cmark-gfm_BINARY_DIR}/src"
    "${cmark-gfm_BINARY_DIR}/extensions")
else()
  target_link_libraries(slamhaus_lib PRIVATE libcmark-gfm libcmark-gfm-extensions)
endif()

if(MSVC)
  target_compile_options(slamhaus_lib PRIVATE /W4 /permissive-)
else()
  target_compile_options(slamhaus_lib PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

add_executable(slamhaus src/main.cpp src/cli/cli.cpp)
target_link_libraries(slamhaus PRIVATE slamhaus_lib)

install(TARGETS slamhaus RUNTIME DESTINATION bin)
install(FILES LICENSE THIRD_PARTY_NOTICES.md DESTINATION share/licenses/slamhaus)
install(FILES sdk/slam/slam.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/slam)
install(FILES README.md CHANGELOG.md AGENTS.md docs/index.md docs/syntax-reference.md
              docs/project-configuration.md docs/cpp-sdk.md docs/deployment.md docs/pwa.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES toolchains.lock.json DESTINATION ${CMAKE_INSTALL_DATADIR}/slamhaus)

find_package(Doxygen QUIET)
if(Doxygen_FOUND)
  set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs-build/doxygen")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                 ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doxygen
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${DOXYGEN_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DOXYGEN_OUTPUT_DIRECTORY}
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating Slamhaus API documentation")
endif()

if(BUILD_TESTING)
  find_package(Catch2 3 QUIET)
  if(NOT Catch2_FOUND)
    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.15.3
      GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(Catch2)
  endif()
  add_executable(slamhaus_tests
    tests/language_test.cpp
    tests/project_test.cpp
    tests/lsp_test.cpp
    tests/sdk_test.cpp)
  target_include_directories(slamhaus_tests PRIVATE sdk)
  target_link_libraries(slamhaus_tests PRIVATE slamhaus_lib Catch2::Catch2WithMain)
  # Run the Catch2 binary as one test. This avoids configure-time discovery
  # dependence on the host runtime directory while preserving all test cases.
  add_test(NAME slamhaus_unit_tests COMMAND slamhaus_tests)
  set_tests_properties(slamhaus_unit_tests PROPERTIES
    ENVIRONMENT "EM_CACHE=${CMAKE_SOURCE_DIR}/.emscripten_cache")
  add_test(NAME cli_version COMMAND slamhaus --version)
  set_tests_properties(cli_version PROPERTIES PASS_REGULAR_EXPRESSION "slamhaus ${SLAMHAUS_VERSION}")
  add_test(NAME cli_help COMMAND slamhaus --help)
  set_tests_properties(cli_help PROPERTIES
    PASS_REGULAR_EXPRESSION "slamhaus examples")
  add_test(NAME cli_completion COMMAND slamhaus completion bash)
  set_tests_properties(cli_completion PROPERTIES PASS_REGULAR_EXPRESSION "complete -F _slamhaus")
  add_test(NAME cli_fmt_help COMMAND slamhaus --help)
  set_tests_properties(cli_fmt_help PROPERTIES PASS_REGULAR_EXPRESSION "slamhaus fmt")
  add_test(NAME cli_toolchain_status COMMAND slamhaus toolchain status --format json)
  set_tests_properties(cli_toolchain_status PROPERTIES
    PASS_REGULAR_EXPRESSION "\"version\": \"4.0.20\"")
  add_test(NAME cli_explain COMMAND slamhaus explain SH1124)
  set_tests_properties(cli_explain PROPERTIES PASS_REGULAR_EXPRESSION "C\\+\\+ function")
  add_test(NAME project_self_check COMMAND slamhaus check ${CMAKE_SOURCE_DIR} --strict)
  add_test(NAME project_format_check COMMAND slamhaus fmt ${CMAKE_SOURCE_DIR} --check)
endif()

option(SLAMHAUS_FUZZING "Build the parser fuzz target" OFF)
if(SLAMHAUS_FUZZING AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_executable(slamhaus_parser_fuzz fuzz/parser_fuzz.cpp)
  target_link_libraries(slamhaus_parser_fuzz PRIVATE slamhaus_lib)
  target_compile_options(slamhaus_parser_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
  target_link_options(slamhaus_parser_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
endif()

set(CPACK_PACKAGE_NAME "slamhaus")
set(CPACK_PACKAGE_VENDOR "Slamhaus")
set(CPACK_PACKAGE_VERSION "${SLAMHAUS_VERSION}")
set(CPACK_PACKAGE_FILE_NAME
    "slamhaus-${SLAMHAUS_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_GENERATOR "ZIP")
include(CPack)
