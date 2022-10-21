set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

add_subdirectory(${CMAKE_SOURCE_DIR}/external/cpp-chaintester ${CMAKE_BINARY_DIR}/external/cpp-chaintester)
add_subdirectory(${CMAKE_SOURCE_DIR}/contracts/eosio.system/native ${CMAKE_BINARY_DIR}/contracts/eosio.system/native)
add_subdirectory(${CMAKE_SOURCE_DIR}/contracts/eosio.token/native ${CMAKE_BINARY_DIR}/contracts/eosio.token/native)

include(ExternalProject)
# if no cdt root is given use default path
if(CDT_ROOT STREQUAL "" OR NOT CDT_ROOT)
   find_package(cdt)
endif()

ExternalProject_Add(
   active-contract-project
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/contracts/activate
   BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/contracts/activate
   CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CDT_ROOT}/lib/cmake/cdt/CDTWasmToolchain.cmake
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   TEST_COMMAND ""
   INSTALL_COMMAND ""
   BUILD_ALWAYS 0
)

add_executable( contracts_tests 
    src/test_system.cpp
    src/test_token.cpp
    src/main.cpp
)

target_compile_options(contracts_tests PRIVATE 
    ${COVERAGE_OPTIONS}
    -DEOSIO_NATIVE
    -Wno-unknown-attributes
    -Wno-deprecated-declarations
    -Wno-unused-value
)

target_link_options(contracts_tests PRIVATE ${COVERAGE_OPTIONS})

target_link_libraries ( contracts_tests
    system_native
    token_native
    chaintester
)

add_dependencies(contracts_tests chaintester)

set(CDT_INCLUDE_PATH
    ${CMAKE_SOURCE_DIR}/external/cpp-chaintester/libs/cdt/include
    ${CMAKE_SOURCE_DIR}/external/cpp-chaintester/libs/cdt/include/eosiolib/capi
    ${CMAKE_SOURCE_DIR}/external/cpp-chaintester/libs/cdt/include/eosiolib/native
    ${CMAKE_SOURCE_DIR}/external/cpp-chaintester/libs/cdt/include/eosiolib/core
    ${CMAKE_SOURCE_DIR}/external/cpp-chaintester/libs/cdt/include/eosiolib/contracts
)

target_include_directories(contracts_tests PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${CDT_INCLUDE_PATH}
    "${CMAKE_SOURCE_DIR}/external/cpp-chaintester/libs/Catch2/src"
    "${CMAKE_SOURCE_DIR}/external/cpp-chaintester/chaintester/src"
    "${CMAKE_BINARY_DIR}/generated-includes"
)

configure_file(src/generated.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated.h @ONLY )

add_dependencies(contracts_tests contracts_project active-contract-project)
