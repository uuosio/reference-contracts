#include <stdint.h>
#include <chaintester/chaintester.hpp>
#include "generated.h"

using namespace std;

#ifdef __cplusplus

extern "C" {
    void system_native_apply( uint64_t receiver, uint64_t code, uint64_t action );
    void token_native_apply( uint64_t receiver, uint64_t code, uint64_t action );
}

#endif


extern "C" size_t n2s(uint64_t n, char *cstr, size_t length);

static constexpr unsigned int DJBH(const char* cp)
{
  unsigned int hash = 5381;
  while (*cp)
      hash = 33 * hash ^ (unsigned char) *cp++;
  return hash;
}

static constexpr unsigned long long WASM_TEST_ACTION(const char* cls, const char* method)
{
  return static_cast<unsigned long long>(DJBH(cls)) << 32 | static_cast<unsigned long long>(DJBH(method));
}

static uint64_t TEST_METHOD(const char* CLASS, const char *METHOD) {
  return ( (uint64_t(DJBH(CLASS))<<32) | uint32_t(DJBH(METHOD)) );
}

template<typename Arguments>
static std::shared_ptr<JsonObject> CallFunction(ChainTester& tester, const string& account, uint64_t action, const Arguments& args, const string& signer, const string& required_exception_type="", const string& exception_message="") {
    try {
        auto ret = tester.push_action(name(signer), name(account), name(action), args);
        REQUIRE(!ret->HasMember("except"));
        return ret;
    } catch(ChainException& ex) {
        auto& o = ex.value();
        CHECK(o.HasMember("except"));
        auto& except = o["except"];
        // WARN(o.to_string());
        CHECK(except["name"].GetString() == required_exception_type);
        if ("wasm_execution_error" == required_exception_type) {
            auto s =  except["stack"][0]["format"].GetString();
            CHECK(string(s).find(exception_message) != std::string::npos);
        } else if ("eosio_assert_message_exception" == required_exception_type) {
            auto s =  except["stack"][0]["data"]["s"].GetString();
            CHECK(string(s).find(exception_message) != std::string::npos);
        } else {
            auto s =  except["stack"][0]["format"].GetString();
            WARN(s);
            CHECK(string(s).find(exception_message) != std::string::npos);
        }
        return std::make_shared<JsonObject>(o.to_string());
    }
}

#define CALL_TEST_FUNCTION(_TESTER, CLS, MTH, DATA) CallFunction(_TESTER, "testapi", TEST_METHOD(CLS, MTH), DATA)
#define CALL_TEST_FUNCTION_AND_CHECK_EXCEPTION(_TESTER, CLS, MTH, DATA, EXCEPT_TYPE, EXCEPT_MSG) CallFunction(_TESTER, "testapi", TEST_METHOD(CLS, MTH), DATA, EXCEPT_TYPE, EXCEPT_MSG)
#define CALL_TEST_FUNCTION_CHECK_ASSERT_EXCEPTION(_TESTER, CLS, MTH, DATA, EXCEPT_MSG) CallFunction(_TESTER, "testapi", TEST_METHOD(CLS, MTH), DATA, "eosio_assert_message_exception", EXCEPT_MSG)

#define CALL_ACTION(_TESTER, CONTRACT, ACTION, DATA, SIGNER) CallFunction(_TESTER, CONTRACT, eosio::name(ACTION).value, DATA, SIGNER)
#define CALL_ACTION_AND_CHECK_EXCEPTION(_TESTER, CONTRACT, ACTION, SIGNER, EXCEPT_TYPE, EXCEPT_MSG) CallFunction(_TESTER, CONTRACT, eosio::name(ACTION).value, DATA, SIGNER, EXCEPT_TYPE, EXCEPT_MSG)
#define CALL_ACTION_CHECK_ASSERT_EXCEPTION(_TESTER, CONTRACT, ACTION, DATA, SIGNER, EXCEPT_MSG) CallFunction(_TESTER, CONTRACT, eosio::name(ACTION).value, DATA, SIGNER, "eosio_assert_message_exception", EXCEPT_MSG)


string I64Str(int64_t i);
string U64Str(uint64_t i);
string U128Str(unsigned __int128 n);
