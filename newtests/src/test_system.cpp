#include <cstdlib>
#include <eosio/eosio.hpp>
#include <eosio/privileged.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>

#include <chaintester/chaintester.hpp>
#include <chaintester/permission.hpp>
#include <chaintester/base58.hpp>

#include "config.hpp"
#include "test.h"

using namespace eosio;

constexpr symbol core_symbol = symbol("EOS", 4);

void update_auth(ChainTester& t, string pub_key) {
    eosio::pack(std::make_tuple(asset(0, symbol("EOS", 4))));
    auto auth = create_authority(pub_key, permission_level_weight{{"testapi"_n, "eosio.code"_n}, 1});
    t.push_action("testapi"_n, "eosio"_n, "updateauth"_n, std::make_tuple("testapi"_n, "active"_n, "owner"_n, auth));
    t.produce_block();
}

void create_accounts(ChainTester& t, std::vector<name>&& accounts ) {
    auto auth = create_authority("EOS6AjF6hvF7GSuSd4sCgfPKq5uWaXvGM2aQtEUCwmEHygQaqxBSV");
    for (auto& account: accounts) {
        auto args = std::make_tuple("eosio"_n, account, auth, auth);
        t.push_action( "eosio"_n, "eosio"_n, "newaccount"_n, args);
    }
}

void create_core_token(ChainTester& t) {
    t.push_action("eosio.token"_n, "eosio.token"_n, "create"_n, std::make_tuple("eosio"_n, asset(100000000000000, core_symbol)));
    t.push_action("eosio"_n, "eosio.token"_n, "issue"_n, std::make_tuple("eosio"_n, asset(100000000000000, core_symbol), string("")));
    CHECK(t.get_balance("eosio"_n, "eosio.token"_n, "EOS") == 100000000000000);
}

// struct TxAction {
//   std::string account;
//   std::string action;
//   ActionArguments arguments;
//   std::string permissions;
// };

void create_account_with_resources( const name a, const name creator, uint32_t ram_bytes = 8000 ) {
    auto owner_auth = authority{
        .threshold = 1,
        .keys = {{
            .key = decode_public_key("EOS6AjF6hvF7GSuSd4sCgfPKq5uWaXvGM2aQtEUCwmEHygQaqxBSV"),
            .weight = 1
        }},
        .accounts = {},
        .waits = {}
    };

    auto active_auth = authority{
        .threshold = 1,
        .keys = {{
            .key = decode_public_key("EOS6AjF6hvF7GSuSd4sCgfPKq5uWaXvGM2aQtEUCwmEHygQaqxBSV"),
            .weight = 1
        }},
        .accounts = {},
        .waits = {}
    };

    auto new_account = newaccount{
        .creator  = creator,
        .name     = a,
        .owner    = owner_auth,
        .active   = active_auth
    };

    vector<action> acts = {
        action {
            std::vector<permission_level>({{"eosio"_n, "active"_n}}),
            "eosio"_n,
            "newaccount"_n,
            new_account
        }
    };
    // signed_transaction trx;
    // set_transaction_headers(trx);

    // authority owner_auth;
    // owner_auth =  authority( get_public_key( a, "owner" ) );

    // trx.actions.emplace_back( vector<permission_level>{{creator,config::active_name}},
    //                         newaccount{
    //                             .creator  = creator,
    //                             .name     = a,
    //                             .owner    = owner_auth,
    //                             .active   = authority( get_public_key( a, "active" ) )
    //                         });

    // trx.actions.emplace_back( get_action( config::system_account_name, "buyrambytes"_n, vector<permission_level>{{creator,config::active_name}},
    //                                     mvo()
    //                                     ("payer", creator)
    //                                     ("receiver", a)
    //                                     ("bytes", ram_bytes) )
    //                         );
    // trx.actions.emplace_back( get_action( config::system_account_name, "delegatebw"_n, vector<permission_level>{{creator,config::active_name}},
    //                                     mvo()
    //                                     ("from", creator)
    //                                     ("receiver", a)
    //                                     ("stake_net_quantity", core_sym::from_string("10.0000") )
    //                                     ("stake_cpu_quantity", core_sym::from_string("10.0000") )
    //                                     ("transfer", 0 )
    //                                     )
    //                         );

    // set_transaction_headers(trx);
    // trx.sign( get_private_key( creator, "active" ), control->get_chain_id()  );
    // return push_transaction( trx );
}

void basic_setup(ChainTester& t) {
    t.produce_blocks(2);
    create_accounts(t, {"eosio.token"_n, "eosio.ram"_n, "eosio.ramfee"_n, "eosio.stake"_n, "eosio.bpay"_n, "eosio.vpay"_n, "eosio.saving"_n, "eosio.names"_n, "eosio.rex"_n});
    t.produce_blocks(100);

    t.deploy_contract("eosio.token"_n, TOKEN_WASM, TOKEN_ABI);
    t.produce_block();
    create_core_token(t);

    // Assumes previous setup steps were done with core token symbol set to CORE_SYM
    // create_account_with_resources( "alice1111111"_n, config::system_account_name, core_sym::from_string("1.0000"), false );
    // create_account_with_resources( "bob111111111"_n, config::system_account_name, core_sym::from_string("0.4500"), false );
    // create_account_with_resources( "carol1111111"_n, config::system_account_name, core_sym::from_string("1.0000"), false );
}

TEST_CASE( "test system", "[chain]" ) {
    ChainTester t(false);
    set_native_apply(system_native_apply);
    t.enable_debug_contract("eosio"_n, false);
    basic_setup(t);

    t.deploy_contract("eosio"_n, ACTIVATE_WASM, ACTIVATE_ABI);
    
    auto sender = t.new_action_sender();
    sender.add_action("eosio"_n, "init"_n, "eosio"_n, string("hello"));

    vector<string> feature_digests = {
        "1a99a59d87e06e09ec5b028a9cbb7749b4a5ad8819004365d02dc4379a8b7241", //ONLY_LINK_TO_EXISTING_PERMISSION" 
        "2652f5f96006294109b3dd0bbde63693f55324af452b799ee137a81a905eed25", //"FORWARD_SETCODE"
        "299dcb6af692324b899b39f16d5a530a33062804e41f09dc97e9f156b4476707", //"WTMSIG_BLOCK_SIGNATURES"
        "35c2186cc36f7bb4aeaf4487b36e57039ccf45a9136aa856a5d569ecca55ef2b", //"GET_BLOCK_NUM"
        "ef43112c6543b88db2283a2e077278c315ae2c84719a8b25f25cc88565fbea99", //"REPLACE_DEFERRED"
        "4a90c00d55454dc5b059055ca213579c6ea856967712a56017487886a4d4cc0f", //"NO_DUPLICATE_DEFERRED_ID"
        "4e7bf348da00a945489b2a681749eb56f5de00b900014e137ddae39f48f69d67", //"RAM_RESTRICTIONS"
        "4fca8bd82bbd181e714e283f83e1b45d95ca5af40fb89ad3977b653c448f78c2", //"WEBAUTHN_KEY"
        "5443fcf88330c586bc0e5f3dee10e7f63c76c00249c87fe4fbf7f38c082006b4", //"BLOCKCHAIN_PARAMETERS"
        "68dcaa34c0517d19666e6b33add67351d8c5f69e999ca1e37931bc410a297428", //"DISALLOW_EMPTY_PRODUCER_SCHEDULE"
        "6bcb40a24e49c26d0a60513b6aeb8551d264e4717f306b81a37a5afb3b47cedc", //"CRYPTO_PRIMITIVES"
        "8ba52fe7a3956c5cd3a656a3174b931d3bb2abb45578befc59f283ecd816a405", //"ONLY_BILL_FIRST_AUTHORIZER"
        "ad9e3d8f650687709fd68f4b90b41f7d825a365b02c23a636cef88ac2ac00c43", //"RESTRICT_ACTION_TO_SELF"
        "bcd2a26394b36614fd4894241d3c451ab0f6fd110958c3423073621a70826e99", //"GET_CODE_HASH"
        "c3a6138c5061cf291310887c0b5c71fcaffeab90d5deb50d3b9e687cead45071", //"ACTION_RETURN_VALUE"
        "d528b9f6e9693f45ed277af93474fd473ce7d831dae2180cca35d907bd10cb40", //"CONFIGURABLE_WASM_LIMITS2"
        "e0fb64b1085cc5538970158d05a009c24e276fb94e1a0bf6a528b48fbc4ff526", //"FIX_LINKAUTH_RESTRICTION"
        "f0af56d2c5a48d60a4a5b5c903edfb7db3a736a94ed589d0b797df33ff9d3e1d", //"GET_SENDER"
    };

    for (auto& digest: feature_digests) {
        auto raw = hex2bytes(digest);
        t.new_action("eosio"_n, "activate"_n, "eosio"_n).send_raw(raw);
        // auto ret = t.push_action("eosio"_n, "activate"_n, hex2bytes(digest), "eosio"_n);
    }
    t.produce_block();

    t.enable_debug_contract("eosio"_n, true);

    t.deploy_contract("eosio"_n, SYSTEM_WASM, SYSTEM_ABI);
    t.push_action("eosio"_n, "eosio"_n, "init"_n, std::make_tuple(unsigned_int(0), core_symbol));
    t.produce_block();
}
