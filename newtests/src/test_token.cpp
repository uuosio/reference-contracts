#include <cstdlib>
#include <eosio/eosio.hpp>
#include <eosio/privileged.hpp>
#include <eosio/asset.hpp>

#include "config.hpp"
#include "test.h"

using namespace eosio;

void init_test(ChainTester& t) {
    set_native_apply(token_native_apply);

    string permissions = R"(
        {
            "eosio": "active"
        }
    )";

    string args = R"(
        {
            "creator": "eosio",
            "name": "%s",
            "owner": {
                "threshold": 1,
                "keys": [{"key": "EOS6AjF6hvF7GSuSd4sCgfPKq5uWaXvGM2aQtEUCwmEHygQaqxBSV", "weight": 1}],
                "accounts": [],
                "waits": []
            },
            "active": {
                "threshold": 1,
                "keys": [{"key": "EOS6AjF6hvF7GSuSd4sCgfPKq5uWaXvGM2aQtEUCwmEHygQaqxBSV", "weight": 1}],
                "accounts": [],
                "waits": []
            }
        }
    )";

    char buffer[args.size() + 13 + 1];
    for (auto& account: { "alice", "bob", "carol", "eosio.token" }) {
        snprintf(buffer, sizeof(buffer), args.c_str(), account);
        t.push_action("eosio", "newaccount", string(buffer), permissions);
    }
    t.deploy_contract("eosio.token", TOKEN_WASM, TOKEN_ABI);
    t.produce_block();
}

std::shared_ptr<JsonObject> create(ChainTester& t, name issuer, asset maximum_supply ) {
    return t.push_action( "eosio.token", "create", pack(std::make_tuple(issuer, maximum_supply)) );
}

TEST_CASE( "create_tests", "eosio_token_tester" ) {
    ChainTester t(false);
    t.enable_debug_contract("eosio", false);

    init_test(t);
    create(t, "alice"_n, asset(1000000, symbol("TKN", 4)));//asset::from_string("1000.000 TKN"));

    auto rows = t.get_table_rows(true, "eosio.token", "TKN", "stat", "TKN", "", 1);
    WARN(rows->to_string());
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"0.0000 TKN","max_supply":"100.0000 TKN","issuer":"alice"})");
}

// BOOST_FIXTURE_TEST_CASE( create_negative_max_supply, eosio_token_tester ) try {

//    BOOST_REQUIRE_EQUAL( wasm_assert_msg( "max-supply must be positive" ),
//       create( "alice"_n, asset::from_string("-1000.000 TKN"))
//    );

// } FC_LOG_AND_RETHROW()

TEST_CASE( "create_negative_max_supply", "eosio_token_tester" ) {
    ChainTester t(false);
    t.enable_debug_contract("eosio", false);
    init_test(t);
    // name(symbol_code("EOS").raw());
    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "create",
        pack(std::make_tuple("alice"_n, asset(-1000000, symbol("TKN", 4)))),
        "eosio.token", "max-supply must be positive"
    );
}

// BOOST_FIXTURE_TEST_CASE( symbol_already_exists, eosio_token_tester ) try {

//    auto token = create( "alice"_n, asset::from_string("100 TKN"));
//    auto stats = get_stats("0,TKN");
//    REQUIRE_MATCHING_OBJECT( stats, mvo()
//       ("supply", "0 TKN")
//       ("max_supply", "100 TKN")
//       ("issuer", "alice")
//    );
//    produce_blocks(1);

//    BOOST_REQUIRE_EQUAL( wasm_assert_msg( "token with symbol already exists" ),
//                         create( "alice"_n, asset::from_string("100 TKN"))
//    );

// } FC_LOG_AND_RETHROW()

TEST_CASE( "symbol_already_exists", "eosio_token_tester" ) {
    ChainTester t(false);
    t.enable_debug_contract("eosio", false);
    init_test(t);
    create(t, "alice"_n, asset(100, symbol("TKN", 0)));
    t.produce_block();

    auto rows = t.get_table_rows(true, "eosio.token", "TKN", "stat", "TKN", "", 1);
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"0 TKN","max_supply":"100 TKN","issuer":"alice"})");

    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "create",
        pack(std::make_tuple("alice"_n, asset(100, symbol("TKN", 0)))),
        "eosio.token", "token with symbol already exists"
    );
}


// BOOST_FIXTURE_TEST_CASE( create_max_supply, eosio_token_tester ) try {

//    auto token = create( "alice"_n, asset::from_string("4611686018427387903 TKN"));
//    auto stats = get_stats("0,TKN");
//    REQUIRE_MATCHING_OBJECT( stats, mvo()
//       ("supply", "0 TKN")
//       ("max_supply", "4611686018427387903 TKN")
//       ("issuer", "alice")
//    );
//    produce_blocks(1);

//    asset max(10, symbol(SY(0, NKT)));
//    share_type amount = 4611686018427387904;
//    static_assert(sizeof(share_type) <= sizeof(asset), "asset changed so test is no longer valid");
//    static_assert(std::is_trivially_copyable<asset>::value, "asset is not trivially copyable");
//    memcpy(&max, &amount, sizeof(share_type)); // hack in an invalid amount

//    BOOST_CHECK_EXCEPTION( create( "alice"_n, max) , asset_type_exception, [](const asset_type_exception& e) {
//       return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
//    });


// } FC_LOG_AND_RETHROW()

using share_type          = int64_t;

TEST_CASE( "create_max_supply", "eosio_token_tester" ) {
    ChainTester t(false);
    t.enable_debug_contract("eosio.token", true);
    init_test(t);
    create(t, "alice"_n, asset(4611686018427387903, symbol("TKN", 0)));
    t.produce_block();

    auto rows = t.get_table_rows(true, "eosio.token", "TKN", "stat", "TKN", "", 1);
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"0 TKN","max_supply":"4611686018427387903 TKN","issuer":"alice"})");
 
    asset max(10, symbol("NKT", 0));
    share_type amount = 4611686018427387904;
    static_assert(sizeof(share_type) <= sizeof(asset), "asset changed so test is no longer valid");
    static_assert(std::is_trivially_copyable<asset>::value, "asset is not trivially copyable");
    memcpy(&max, &amount, sizeof(share_type)); // hack in an invalid amount

//    BOOST_CHECK_EXCEPTION( create( "alice"_n, max) , asset_type_exception, [](const asset_type_exception& e) {
//       return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
//    });

    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "create",
        pack(std::make_tuple("alice"_n, max)),
        "eosio.token",
        "invalid supply" //"magnitude of asset amount must be less than 2^62"
    );
}


// BOOST_FIXTURE_TEST_CASE( create_max_decimals, eosio_token_tester ) try {

//    auto token = create( "alice"_n, asset::from_string("1.000000000000000000 TKN"));
//    auto stats = get_stats("18,TKN");
//    REQUIRE_MATCHING_OBJECT( stats, mvo()
//       ("supply", "0.000000000000000000 TKN")
//       ("max_supply", "1.000000000000000000 TKN")
//       ("issuer", "alice")
//    );
//    produce_blocks(1);

//    asset max(10, symbol(SY(0, NKT)));
//    //1.0000000000000000000 => 0x8ac7230489e80000L
//    share_type amount = 0x8ac7230489e80000L;
//    static_assert(sizeof(share_type) <= sizeof(asset), "asset changed so test is no longer valid");
//    static_assert(std::is_trivially_copyable<asset>::value, "asset is not trivially copyable");
//    memcpy(&max, &amount, sizeof(share_type)); // hack in an invalid amount

//    BOOST_CHECK_EXCEPTION( create( "alice"_n, max) , asset_type_exception, [](const asset_type_exception& e) {
//       return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
//    });

// } FC_LOG_AND_RETHROW()

TEST_CASE( "create_max_decimals", "eosio_token_tester" ) {
    ChainTester t(false);
    t.enable_debug_contract("eosio.token", true);
    init_test(t);
    create(t, "alice"_n, asset(1000000000000000000, symbol("TKN", 18)));
    t.produce_block();

    auto rows = t.get_table_rows(true, "eosio.token", "TKN", "stat", "TKN", "", 1);
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"0.000000000000000000 TKN","max_supply":"1.000000000000000000 TKN","issuer":"alice"})");
 
    asset max(10, symbol("NKT", 0));
    //1.0000000000000000000 => 0x8ac7230489e80000L
    share_type amount = 0x8ac7230489e80000L;
    static_assert(sizeof(share_type) <= sizeof(asset), "asset changed so test is no longer valid");
    static_assert(std::is_trivially_copyable<asset>::value, "asset is not trivially copyable");
    memcpy(&max, &amount, sizeof(share_type)); // hack in an invalid amount

//    BOOST_CHECK_EXCEPTION( create( "alice"_n, max) , asset_type_exception, [](const asset_type_exception& e) {
//       return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
//    });

    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "create",
        pack(std::make_tuple("alice"_n, max)),
        "eosio.token",
        "invalid supply" //"magnitude of asset amount must be less than 2^62"
    );
}

// BOOST_FIXTURE_TEST_CASE( issue_tests, eosio_token_tester ) try {

//    auto token = create( "alice"_n, asset::from_string("1000.000 TKN"));
//    produce_blocks(1);

//    issue( "alice"_n, asset::from_string("500.000 TKN"), "hola" );

//    auto stats = get_stats("3,TKN");
//    REQUIRE_MATCHING_OBJECT( stats, mvo()
//       ("supply", "500.000 TKN")
//       ("max_supply", "1000.000 TKN")
//       ("issuer", "alice")
//    );

//    auto alice_balance = get_account("alice"_n, "3,TKN");
//    REQUIRE_MATCHING_OBJECT( alice_balance, mvo()
//       ("balance", "500.000 TKN")
//    );

//    BOOST_REQUIRE_EQUAL( wasm_assert_msg( "quantity exceeds available supply" ),
//                         issue( "alice"_n, asset::from_string("500.001 TKN"), "hola" )
//    );

//    BOOST_REQUIRE_EQUAL( wasm_assert_msg( "must issue positive quantity" ),
//                         issue( "alice"_n, asset::from_string("-1.000 TKN"), "hola" )
//    );

//    BOOST_REQUIRE_EQUAL( success(),
//                         issue( "alice"_n, asset::from_string("1.000 TKN"), "hola" )
//    );


// } FC_LOG_AND_RETHROW()

TEST_CASE( "issue_tests", "eosio_token_tester" ) {
    ChainTester t(false);
    t.enable_debug_contract("eosio.token", true);

    init_test(t);
    create(t, "alice"_n, asset(1000000, symbol("TKN", 3)));
    t.produce_block();

//    issue( "alice"_n, asset::from_string("500.000 TKN"), "hola" );
    CALL_ACTION(t, "eosio.token", "issue", eosio::pack(std::make_tuple("alice"_n, asset(500000, symbol("TKN", 3)), string("hola"))), "alice");

    auto rows = t.get_table_rows(true, "eosio.token", "TKN", "stat", "TKN", "", 1);
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"500.000 TKN","max_supply":"1000.000 TKN","issuer":"alice"})");
    WARN(t.get_account("alice")->to_string());
    CHECK(t.get_balance("alice", "eosio.token", "TKN") == 500000);

    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "issue",
        eosio::pack(std::make_tuple("alice"_n, asset(500001, symbol("TKN", 3)), string("hola"))),
        "alice",
        "quantity exceeds available supply"
    );

    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "issue",
        eosio::pack(std::make_tuple("alice"_n, asset(-1000, symbol("TKN", 3)), string("hola"))),
        "alice",
        "must issue positive quantity"
    );

    CALL_ACTION(
        t,
        "eosio.token",
        "issue",
        eosio::pack(std::make_tuple("alice"_n, asset(1000, symbol("TKN", 3)), string("hola"))),
        "alice"
    );
}

