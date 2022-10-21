add_library( token_native
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/eosio.token.cpp
    main.cpp
)

set_target_properties(token_native PROPERTIES CXX_VISIBILITY_PRESET hidden)


target_compile_options(token_native PRIVATE
    ${COVERAGE_OPTIONS}
    -DEOSIO_NATIVE
    -Wno-unknown-attributes
    -Wno-deprecated-declarations
    -Wno-unused-value
)

set(CDT_INCLUDE_PATH
    ${CMAKE_SOURCE_DIR}/external/cpp-chaintester/libs/cdt/include
    ${CMAKE_SOURCE_DIR}/external/cpp-chaintester/libs/cdt/include/eosiolib/capi
    ${CMAKE_SOURCE_DIR}/external/cpp-chaintester/libs/cdt/include/eosiolib/native
    ${CMAKE_SOURCE_DIR}/external/cpp-chaintester/libs/cdt/include/eosiolib/core
    ${CMAKE_SOURCE_DIR}/external/cpp-chaintester/libs/cdt/include/eosiolib/contracts
)

target_include_directories(token_native PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../eosio.token/include
    ${CDT_INCLUDE_PATH}
)

target_link_options(token_native PRIVATE ${COVERAGE_OPTIONS})

target_link_libraries (token_native
    chain_static
)

