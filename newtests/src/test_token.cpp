#include <cstdlib>
#include <eosio/eosio.hpp>
#include <eosio/privileged.hpp>
#include <eosio/asset.hpp>

#include <chaintester/permission.hpp>
#include "config.hpp"
#include "test.h"

using namespace eosio;
using share_type          = int64_t;

constexpr name test_symbol_name = name(symbol_code("TKN").raw());

void init_test(ChainTester& t) {
    set_native_apply("eosio.token"_n, token_native_apply);

    const char * TEST_COVERAGE = std::getenv("TEST_COVERAGE");
    if (TEST_COVERAGE == nullptr || string("") == TEST_COVERAGE || string("0") == TEST_COVERAGE || string("FALSE") == TEST_COVERAGE) {
        t.enable_debug_contract("eosio.token"_n, false);
    } else if (string("1") == TEST_COVERAGE || string("TRUE") == TEST_COVERAGE) {
        t.enable_debug_contract("eosio.token"_n, true);
    } else {
        throw std::runtime_error("invalid TEST_COVERAGE ENV");
    }

    auto auth = create_authority("EOS6AjF6hvF7GSuSd4sCgfPKq5uWaXvGM2aQtEUCwmEHygQaqxBSV");

    for (auto& account: { "alice"_n, "bob"_n, "carol"_n, "eosio.token"_n }) {
        t.push_action("eosio"_n, "eosio"_n, "newaccount"_n, std::make_tuple("eosio"_n, account, auth, auth));
    }
    t.deploy_contract("eosio.token"_n, TOKEN_WASM, TOKEN_ABI);
    t.produce_block();
}

std::shared_ptr<JsonObject> create(ChainTester& t, name issuer, asset maximum_supply ) {
    return t.push_action("eosio.token"_n, "eosio.token"_n, "create"_n, std::make_tuple(issuer, maximum_supply));
}

TEST_CASE( "create_tests", "eosio_token_tester" ) {
    ChainTester t(false);
    init_test(t);
    create(t, "alice"_n, asset(1000000, symbol("TKN", 4)));//asset::from_string("1000.000 TKN"));

    auto rows = t.get_table_rows(true, "eosio.token"_n, test_symbol_name, "stat"_n, test_symbol_name, name(), 1);
    WARN(rows->to_string());
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"0.0000 TKN","max_supply":"100.0000 TKN","issuer":"alice"})");
}

TEST_CASE( "create_negative_max_supply", "eosio_token_tester" ) {
    ChainTester t(false);
    init_test(t);
    // name(symbol_code("EOS").raw());
    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "create",
        std::make_tuple("alice"_n, asset(-1000000, symbol("TKN", 4))),
        "eosio.token", "max-supply must be positive"
    );
}

TEST_CASE( "symbol_already_exists", "eosio_token_tester" ) {
    ChainTester t(false);
    init_test(t);

    create(t, "alice"_n, asset(100, symbol("TKN", 0)));
    t.produce_block();

    auto rows = t.get_table_rows(true, "eosio.token"_n, test_symbol_name, "stat"_n, test_symbol_name, name(), 1);
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"0 TKN","max_supply":"100 TKN","issuer":"alice"})");

    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "create",
        std::make_tuple("alice"_n, asset(100, symbol("TKN", 0))),
        "eosio.token", "token with symbol already exists"
    );
}

TEST_CASE( "create_max_supply", "eosio_token_tester" ) {
    ChainTester t(false);
    init_test(t);

    create(t, "alice"_n, asset(4611686018427387903, symbol("TKN", 0)));
    t.produce_block();

    auto rows = t.get_table_rows(true, "eosio.token"_n, test_symbol_name, "stat"_n, test_symbol_name, name(), 1);
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
        std::make_tuple("alice"_n, max),
        "eosio.token",
        "invalid supply" //"magnitude of asset amount must be less than 2^62"
    );
}

TEST_CASE( "create_max_decimals", "eosio_token_tester" ) {
    ChainTester t(false);
    init_test(t);

    create(t, "alice"_n, asset(1000000000000000000, symbol("TKN", 18)));
    t.produce_block();

    auto rows = t.get_table_rows(true, "eosio.token"_n, test_symbol_name, "stat"_n, test_symbol_name, name(), 1);
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
        std::make_tuple("alice"_n, max),
        "eosio.token",
        "invalid supply" //"magnitude of asset amount must be less than 2^62"
    );
}

TEST_CASE( "issue_tests", "eosio_token_tester" ) {
    ChainTester t(false);
    init_test(t);
    create(t, "alice"_n, asset(1000000, symbol("TKN", 3)));
    t.produce_block();

//    issue( "alice"_n, asset::from_string("500.000 TKN"), "hola" );
    CALL_ACTION(t, "eosio.token", "issue", std::make_tuple("alice"_n, asset(500000, symbol("TKN", 3)), string("hola")), "alice");

    auto rows = t.get_table_rows(true, "eosio.token"_n, test_symbol_name, "stat"_n, test_symbol_name, name(), 1);
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"500.000 TKN","max_supply":"1000.000 TKN","issuer":"alice"})");
    // WARN(t.get_account("alice"_n)->to_string());
    CHECK(t.get_balance("alice"_n, "eosio.token"_n, "TKN") == 500000);

    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "issue",
        std::make_tuple("alice"_n, asset(500001, symbol("TKN", 3)), string("hola")),
        "alice",
        "quantity exceeds available supply"
    );

    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "issue",
        std::make_tuple("alice"_n, asset(-1000, symbol("TKN", 3)), string("hola")),
        "alice",
        "must issue positive quantity"
    );

    CALL_ACTION(
        t,
        "eosio.token",
        "issue",
        std::make_tuple("alice"_n, asset(1000, symbol("TKN", 3)), string("hola")),
        "alice"
    );
}

TEST_CASE( "retire_tests", "eosio_token_tester" ) {
    ChainTester t(false);
    init_test(t);

    create(t, "alice"_n, asset(1000000, symbol("TKN", 3)));
    t.produce_block();

    CALL_ACTION(t, "eosio.token", "issue", std::make_tuple("alice"_n, asset(500000, symbol("TKN", 3)), string("hola")), "alice");
    auto rows = t.get_table_rows(true, "eosio.token"_n, test_symbol_name, "stat"_n, test_symbol_name, name(), 1);
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"500.000 TKN","max_supply":"1000.000 TKN","issuer":"alice"})");
    // WARN(t.get_account("alice"_n)->to_string());
    CHECK(t.get_balance("alice"_n, "eosio.token"_n, "TKN") == 500000);

    CALL_ACTION(t, "eosio.token", "retire", std::make_tuple(asset(200000, symbol("TKN", 3)), string("hola")), "alice");
    rows = t.get_table_rows(true, "eosio.token"_n, test_symbol_name, "stat"_n, test_symbol_name, name(), 1);
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"300.000 TKN","max_supply":"1000.000 TKN","issuer":"alice"})");
    // WARN(t.get_account("alice"_n)->to_string());
    CHECK(t.get_balance("alice"_n, "eosio.token"_n, "TKN") == 300000);

   //should fail to retire more than current supply
    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "retire",
        std::make_tuple(asset(500000, symbol("TKN", 3)), string("hola")),
        "alice",
        "overdrawn balance"
    );

//    BOOST_REQUIRE_EQUAL( success(), transfer( "alice"_n, "bob"_n, asset::from_string("200.000 TKN"), "hola" ) );
    CALL_ACTION(
        t,
        "eosio.token",
        "transfer",
        std::make_tuple("alice"_n, "bob"_n, asset(200000, symbol("TKN", 3)), string("hola")),
        "alice"
    );

//    //should fail to retire since tokens are not on the issuer's balance
//    BOOST_REQUIRE_EQUAL( wasm_assert_msg("overdrawn balance"), retire( "alice"_n, asset::from_string("300.000 TKN"), "hola" ) );
    CALL_ACTION_CHECK_ASSERT_EXCEPTION(
        t,
        "eosio.token",
        "retire",
        std::make_tuple(asset(300000, symbol("TKN", 3)), string("hola")),
        "alice",
        "overdrawn balance"
    );
//    //transfer tokens back
//    BOOST_REQUIRE_EQUAL( success(), transfer( "bob"_n, "alice"_n, asset::from_string("200.000 TKN"), "hola" ) );
    CALL_ACTION(
        t,
        "eosio.token",
        "transfer",
        std::make_tuple("bob"_n, "alice"_n, asset(200000, symbol("TKN", 3)), string("hola")),
        "bob"
    );

//    BOOST_REQUIRE_EQUAL( success(), retire( "alice"_n, asset::from_string("300.000 TKN"), "hola" ) );
    CALL_ACTION(
        t,
        "eosio.token",
        "retire",
        std::make_tuple(asset(300000, symbol("TKN", 3)), string("hola")),
        "alice"
    );

    rows = t.get_table_rows(true, "eosio.token"_n, test_symbol_name, "stat"_n, test_symbol_name, name(), 1);
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"0.000 TKN","max_supply":"1000.000 TKN","issuer":"alice"})");
    // WARN(t.get_account("alice"_n)->to_string());
    CHECK(t.get_balance("alice"_n, "eosio.token"_n, "TKN") == 0);

//    //trying to retire tokens with zero supply
//    BOOST_REQUIRE_EQUAL( wasm_assert_msg("overdrawn balance"), retire( "alice"_n, asset::from_string("1.000 TKN"), "hola" ) );
    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "retire"_n,
        std::make_tuple(asset(1000, symbol("TKN", 3)), string("hola"))
    ).send_and_catch_exception().require_assertion("overdrawn balance");
}

TEST_CASE( "transfer_tests", "eosio_token_tester" ) {
    ChainTester t(false);
    init_test(t);

    create(t, "alice"_n, asset(1000, symbol("CERO", 0)));
    t.produce_block();

    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "issue"_n,
        std::make_tuple("alice"_n, asset(1000, symbol("CERO", 0)), string("hola"))
    ).send();
    auto rows = t.get_table_rows(true, "eosio.token"_n, name(symbol_code("CERO").raw()), "stat"_n, name(symbol_code("CERO").raw()), name(), 1);
    CHECK(rows->get_string("rows", 0, "data") == R"({"supply":"1000 CERO","max_supply":"1000 CERO","issuer":"alice"})");
    // WARN(t.get_account("alice"_n)->to_string());
    CHECK(t.get_balance("alice"_n, "eosio.token"_n, "CERO") == 1000);

    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "transfer"_n,
        "alice"_n, "bob"_n, asset(300, symbol("CERO", 0)), string("hola")
    ).send();

    CHECK(t.get_balance("alice"_n, "eosio.token"_n, "CERO") == 700);
    CHECK(t.get_balance("bob"_n, "eosio.token"_n, "CERO") == 300);

    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "transfer"_n,
        std::make_tuple("alice"_n, "bob"_n, asset(701, symbol("CERO", 0)), string("hola"))
    ).send_and_catch_exception().require_assertion("overdrawn balance");
    
    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "transfer"_n,
        std::make_tuple("alice"_n, "bob"_n, asset(-1000, symbol("CERO", 0)), string("hola"))
    ).send_and_catch_exception().require_assertion("must transfer positive quantity");
}

TEST_CASE( "open_tests", "eosio_token_tester" ) {
    ChainTester t(false);
    init_test(t);

    create(t, "alice"_n, asset(1000, symbol("CERO", 0)));
    t.produce_block();
    CHECK(t.get_balance("alice"_n, "eosio.token"_n, "CERO") == 0);

    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "issue"_n,
        std::make_tuple(
            "bob"_n,
            asset(1000, symbol("CERO", 0)),
            string("")
        )
    ).send_and_catch_exception().require_assertion("tokens can only be issued to issuer account");

    // BOOST_REQUIRE_EQUAL( success(), issue( "alice"_n, asset::from_string("1000 CERO"), "issue" ) );
    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "issue"_n,
        "alice"_n, asset(1000, symbol("CERO", 0)), string("")
    ).send();

    CHECK(t.get_balance("alice"_n, "eosio.token"_n, "CERO") == 1000);
    CHECK(t.get_balance("bob"_n, "eosio.token"_n, "CERO") == 0);

    // BOOST_REQUIRE_EQUAL( wasm_assert_msg("owner account does not exist"),
    //                     open( "nonexistent"_n, "0,CERO", "alice"_n ) );
    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "open"_n,
        "nonexistent"_n, symbol("CERO", 0), "alice"_n
    ).send_and_catch_exception().require_assertion("owner account does not exist");
    // BOOST_REQUIRE_EQUAL( success(),
    //                     open( "bob"_n,         "0,CERO", "alice"_n ) );
    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "open"_n,
        std::make_tuple("bob"_n, symbol("CERO", 0), "alice"_n)
    ).send();
    // bob_balance = get_account("bob"_n, "0,CERO");
    // REQUIRE_MATCHING_OBJECT( bob_balance, mvo()
    //     ("balance", "0 CERO")
    // );
    CHECK(t.get_balance("bob"_n, "eosio.token"_n, "CERO") == 0);

    // BOOST_REQUIRE_EQUAL( success(), transfer( "alice"_n, "bob"_n, asset::from_string("200 CERO"), "hola" ) );
    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "transfer"_n,
        std::make_tuple("alice"_n, "bob"_n, asset(200, symbol("CERO", 0)), string("hola"))
    ).send();
    // bob_balance = get_account("bob"_n, "0,CERO");
    // REQUIRE_MATCHING_OBJECT( bob_balance, mvo()
    //     ("balance", "200 CERO")
    // );
    CHECK(t.get_balance("bob"_n, "eosio.token"_n, "CERO") == 200);

    // BOOST_REQUIRE_EQUAL( wasm_assert_msg( "symbol does not exist" ),
    //                     open( "carol"_n, "0,INVALID", "alice"_n ) );
    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "open"_n,
        "carol"_n, symbol("INVALID", 0), "alice"_n
    ).send_and_catch_exception().require_assertion("symbol does not exist");

    // BOOST_REQUIRE_EQUAL( wasm_assert_msg( "symbol precision mismatch" ),
    //                     open( "carol"_n, "1,CERO", "alice"_n ) );
    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "open"_n,
        "carol"_n,
        symbol("CERO", 1),
        "alice"_n
    ).send_and_catch_exception().require_assertion("symbol precision mismatch");
}

TEST_CASE( "close_tests", "eosio_token_tester" ) {
    ChainTester t(false);
    init_test(t);

    create(t, "alice"_n, asset(1000, symbol("CERO", 0)));
    t.produce_block();
    CHECK(t.get_balance("alice"_n, "eosio.token"_n, "CERO") == 0);

    // BOOST_REQUIRE_EQUAL( success(), issue( "alice"_n, asset::from_string("1000 CERO"), "hola" ) );

    // alice_balance = get_account("alice"_n, "0,CERO");
    // REQUIRE_MATCHING_OBJECT( alice_balance, mvo()
    //     ("balance", "1000 CERO")
    // );

    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "issue"_n,
        "alice"_n, asset(1000, symbol("CERO", 0)), string("")
    ).send();

    CHECK(t.get_balance("alice"_n, "eosio.token"_n, "CERO") == 1000);

    // BOOST_REQUIRE_EQUAL( success(), transfer( "alice"_n, "bob"_n, asset::from_string("1000 CERO"), "hola" ) );
    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "transfer"_n,
        "alice"_n, "bob"_n, asset(1000, symbol("CERO", 0)), string("hola")
    ).send();

    // alice_balance = get_account("alice"_n, "0,CERO");
    // REQUIRE_MATCHING_OBJECT( alice_balance, mvo()
    //     ("balance", "0 CERO")
    // );
    CHECK(t.get_balance("alice"_n, "eosio.token"_n, "CERO") == 0);

    // BOOST_REQUIRE_EQUAL( success(), close( "alice"_n, "0,CERO" ) );
    t.new_action_sender().add_action(
        "alice"_n,
        "eosio.token"_n,
        "close"_n,
        "alice"_n, symbol("CERO", 0)
    ).send();
    // alice_balance = get_account("alice"_n, "0,CERO");
    // BOOST_REQUIRE_EQUAL(true, alice_balance.is_null() );
    CHECK(!t.get_balance_ex("alice"_n, "eosio.token"_n, "CERO").has_value());
}
