#include <cstdlib>
#include <eosio/eosio.hpp>
#include <eosio/privileged.hpp>
#include <eosio/asset.hpp>

#include "config.hpp"
#include "test.h"

using namespace eosio;

void update_auth(ChainTester& t, string pub_key) {
    eosio::pack(std::make_tuple(asset(0, symbol("EOS", 4))));
    const char* updateauth_args = R"(
    {
        "account": "testapi",
        "permission": "active",
        "parent": "owner",
        "auth": {
            "threshold": 1,
            "keys": [
                {
                    "key": "%s",
                    "weight": 1
                }
            ],
            "accounts": [{"permission":{"actor": "testapi", "permission": "eosio.code"}, "weight":1}],
            "waits": []
        }
    }
    )";
    char _updateauth_args[strlen(updateauth_args) + pub_key.size()+1];
    snprintf(_updateauth_args, sizeof(_updateauth_args), updateauth_args, pub_key.c_str());

    string permissions = R"(
        {
            "testapi": "active"
        }
    )";
    t.push_action("eosio", "updateauth", string(_updateauth_args), permissions);
    t.produce_block();
}

TEST_CASE( "test chain", "[chain]" ) {
    ChainTester t(false);
    set_native_apply(system_native_apply);

    t.enable_debug_contract("eosio", false);

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
    snprintf(buffer, sizeof(buffer), args.c_str(), "testapi");

    t.push_action("eosio", "newaccount", string(buffer), permissions);

    string pub_key = "EOS6AjF6hvF7GSuSd4sCgfPKq5uWaXvGM2aQtEUCwmEHygQaqxBSV";

    vector<string> producers = { "inita",
                                "initb",
                                "initc",
                                "initd",
                                "inite",
                                "initf",
                                "initg",
                                "inith",
                                "initi",
                                "initj",
                                "initk",
                                "initl",
                                "initm",
                                "initn",
                                "inito",
                                "initp",
                                "initq",
                                "initr",
                                "inits",
                                "initt",
                                "initu"
    };

    for (auto& a: producers) {
        snprintf(buffer, sizeof(buffer), args.c_str(), a.c_str());
        t.push_action("eosio", "newaccount", string(buffer), permissions);
        t.produce_block();
    }
    {
        auto ret = t.deploy_contract("eosio", ACTIVATE_WASM, ACTIVATE_ABI);
    }

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
        snprintf(buffer, sizeof(buffer), R"({"feature_digest": "%s"})", digest.c_str());
        auto ret = t.push_action("eosio", "activate", string(buffer), permissions);
    }
    t.produce_block();

    t.enable_debug_contract("eosio", false);

    t.deploy_contract("eosio", BIOS_WASM, BIOS_ABI);
    t.produce_block();

    ecc_public_key _key;
    //EOS6AjF6hvF7GSuSd4sCgfPKq5uWaXvGM2aQtEUCwmEHygQaqxBSV
    memcpy(_key.data(), "\x02\xa8\x91\xe0\xdd\x57\x13\x2e\xd6\x83\xbc\x87\x5d\xac\xc9\x61\xc6\xfd\x5d\xfa\xe6\x80\x0b\xc6\x18\x1a\xb6\x8b\xb8\x48\x25\x1e\x52", 33);
    public_key key{std::in_place_index<0>, _key};

    std::vector<producer_authority> prods;
    for (auto& a: producers) {
        prods.emplace_back(producer_authority{
            .producer_name = eosio::name(a),
            .authority = block_signing_authority_v0{
                .threshold = 1,
                .keys = {
                    key_weight{
                        .key = key,
                        .weight = 1,
                    }
                }
            }
        });
    }

    auto ret = t.push_action("eosio", "setprods", eosio::pack(prods), permissions);
    // WARN(ret->to_string());
    for (int i=0; i<200; i++) {
        t.produce_block();
    }

    vector<eosio::name> prod_names(producers.size());
    for ( uint32_t i = 0; i < producers.size(); i++ ) {
        prod_names[i] = name(producers[i]);
    }

    // CALL_TEST_FUNCTION(t, "test_chain", "test_activeprods", eosio::pack(prod_names));
    // WARN(ret->to_string());
}


TEST_CASE( "test system", "[chain]" ) {
    ChainTester t(false);
    set_native_apply(system_native_apply);

    t.enable_debug_contract("eosio", false);

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
    snprintf(buffer, sizeof(buffer), args.c_str(), "testapi");

    t.push_action("eosio", "newaccount", string(buffer), permissions);

    t.deploy_contract("eosio", ACTIVATE_WASM, ACTIVATE_ABI);

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
        snprintf(buffer, sizeof(buffer), R"({"feature_digest": "%s"})", digest.c_str());
        auto ret = t.push_action("eosio", "activate", string(buffer), permissions);
    }
    t.produce_block();

    t.enable_debug_contract("eosio", false);

    t.deploy_contract("eosio", BIOS_WASM, BIOS_ABI);
    t.produce_block();
}
