add_library( system_native
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/eosio.system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/delegate_bandwidth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/exchange_state.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/name_bidding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/native.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/producer_pay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/powerup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/rex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/voting.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/limit_auth_changes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/block_info.cpp
    main.cpp
)

set_target_properties(system_native PROPERTIES CXX_VISIBILITY_PRESET hidden)

target_compile_options(system_native PRIVATE
    # -fprofile-arcs -ftest-coverage
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

target_include_directories(system_native PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../eosio.token/include
    ${CDT_INCLUDE_PATH}
)

# target_link_options(system_native PRIVATE -fprofile-arcs -ftest-coverage)

target_link_libraries (system_native
    chain_static
)

