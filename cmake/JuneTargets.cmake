#
# newJuneTarget(
#   name
#   source1 [source2 source3 ...]
#
#   [BINARY] (ignores STATIC and other lib options)
#   [STATIC] (ignored when BINARY is enabled)
#   [NO_COMPILE_DB] (disables copying compile_commands.json) to ${PROJECT_SOURCE_DIR}
#   [INCLUDE_INSTALL_DIR [/path/to/inc/install/dir]] (default: '', prefix: "${PREFIX}/include/")
#   [INSTALL_DIR [/path/to/lib/install/dir]] (default: '', prefix: "${PREFIX}/lib/" or "${PREFIX}/bin/" for BINARY)
#   [LINK_LIBS [lib1 lib2 lib3 ...]]
#   [TESTS [test1 test2 test3 ...]]
# )
# 
#
# name
#   Name of target
#
# sources...
#   Sources for target
#
# LINK_LIBS libs...
#   Libraries to link with 
#
# TESTS tests...
#   Test executables to build
#
function(newJuneTarget targetName)
  cmake_parse_arguments(JT "STATIC;BINARY;NO_COMPILE_DB" "INCLUDE_INSTALL_DIR;INSTALL_DIR" "LINK_LIBS;TESTS" ${ARGN})

  if (__JT_${targetName}_exists)
    message(FATAL_ERROR "newJuneTarget: Target `${targetName}` already exists")
  endif()

  # get all the sources in the arguments
  set(JT_SOURCES ${JT_UNPARSED_ARGUMENTS})

  get_filename_component(base_dir ${CMAKE_CURRENT_SOURCE_DIR} NAME)

  file(GLOB_RECURSE JT_HEADERS
      ${PROJECT_SOURCE_DIR}/include/${base_dir}/*.h
      ${PROJECT_SOURCE_DIR}/include/${base_dir}/*.hpp
      ${PROJECT_SOURCE_DIR}/include/*.h
      ${PROJECT_SOURCE_DIR}/include/*.hpp)
  
  set(JT_ALL_SOURCES ${JT_SOURCES} ${JT_HEADERS})

  if(JT_BINARY)
    add_executable(
      ${targetName}
      ${JT_ALL_SOURCES}
      )
  elseif(JT_STATIC)
    add_library(
      ${targetName}
      ${JT_ALL_SOURCES}
      )
  else()
    add_library(
      ${targetName}
      SHARED
      ${JT_ALL_SOURCES}
      )
  endif()

  target_include_directories(
    ${targetName}
    PUBLIC ${PROJECT_SOURCE_DIR}/include/
    PUBLIC ${PROJECT_SOURCE_DIR}/include/${base_dir}
  )

  # Add vendor libraries
  target_link_libraries(
    ${targetName}
    PRIVATE cxxopts::cxxopts
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE fmt::fmt
  )

  if(JT_LINK_LIBS)
    target_link_libraries(
      ${targetName}
      PUBLIC ${JT_LINK_LIBS}
      PUBLIC ${CMAKE_DL_LIBS}
    )
  endif()

  if(JT_BINARY)
    set_target_properties(
      ${targetName}
      PROPERTIES
      OUTPUT_NAME ${targetName}
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
      INSTALL_RPATH_USE_LINK_PATH ON
    )

    install(
      TARGETS ${targetName}
      RUNTIME
        DESTINATION bin/${JT_INSTALL_DIR}
        COMPONENT Binaries
      PUBLIC_HEADER
        DESTINATION include/${JT_INCLUDE_INSTALL_DIR}
    )
  else()
    set_target_properties(
      ${targetName}
      PROPERTIES
      PREFIX "libJune"
      OUTPUT_NAME ${targetName}
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${NJT_LIBRARY_INSTALL_DIR}"
      INSTALL_RPATH_USE_LINK_PATH ON
    )

    install(
      TARGETS ${targetName}
      RUNTIME
        DESTINATION lib/${JT_INSTALL_DIR}
        COMPONENT Libraries
      PUBLIC_HEADER
        DESTINATION include/${JT_INCLUDE_INSTALL_DIR}
    )
  endif()

  set(__JT_${targetName}_exists TRUE PARENT_SCOPE)
  set(__JT_${targetName}_base_dir ${base_dir} PARENT_SCOPE)
  set(__JT_${targetName}_link_libs ${JT_LINK_LIBS} PARENT_SCOPE)
  set(__JT_${targetName}_is_binary ${JT_BINARY} PARENT_SCOPE)

  if(EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json" AND NOT "${JT_NO_COMPILE_DB}")
    add_custom_command(
      OUTPUT ${PROJECT_SOURCE_DIR}/compile_commands.json
      POST_BUILD
      COMMAND cp ${CMAKE_BINARY_DIR}/compile_commands.json ${PROJECT_SOURCE_DIR}
      VERBATIM
    )
  endif()
endfunction()

# newJuneTest
function(newJuneTest testName)
  cmake_parse_arguments(JT "TEST;BENCH" "AGAINST" "SOURCES" ${ARGN})

  if(NOT __JT_${JT_AGAINST}_exists)
    message(FATAL_ERROR "newJuneTest: Target `${JT_AGAINST}` does not exist or has not been defined")
  endif()

  if(__JT_${testName}_exists)
    message(FATAL_ERROR "newJuneTest: Test `${testName}` already exists")
  endif()

  add_executable(
    ${testName}
    ${JT_SOURCES}
  )

  target_include_directories(
    ${testName}
    PUBLIC ${PROJECT_SOURCE_DIR}/include/
    PUBLIC ${PROJECT_SOURCE_DIR}/include/${__JT_${JT_AGAINST}_base_dir}
  )

  # Vendor libraries
  if (JT_TEST)
    target_link_libraries(
      ${testName}
      PRIVATE nlohmann_json::nlohmann_json
      PRIVATE GTest::gtest_main
      PRIVATE fmt::fmt
    )
  elseif(JT_BENCH)
    target_link_libraries(
      ${testName}
      PRIVATE nlohmann_json::nlohmann_json
      PRIVATE fmt::fmt
      PRIVATE nanobench
    )
  endif()

  if (NOT __JT_${JT_AGAINST}_is_binary)
    target_link_libraries(
      ${testName}
      PRIVATE ${JT_AGAINST}
    )
  endif()

  if (JT_BENCH)
    # For benchmarks, we'll store the executable under a `bench` directory
    set_target_properties(
      ${testName}
      PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bench"
    )

    if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      install(
        TARGETS ${testName}
        RUNTIME
          DESTINATION bench
          COMPONENT Benchmarks
      )
    endif()

    return()
  endif()

  include(GoogleTest)
  gtest_discover_tests(${testName})
endfunction()
