#pragma once

#include <eosio/asset.hpp>
#include <chaintester/utils.hpp>

#define CORE_SYM_NAME "EOS"
#define CORE_SYM_PRECISION 4

// #define _STRINGIZE1(x) #x
// #define _STRINGIZE2(x) _STRINGIZE1(x)

// #define CORE_SYM_STR ( _STRINGIZE2(CORE_SYM_PRECISION) "," CORE_SYM_NAME )
// #define CORE_SYM  ( ::eosio::chain::string_to_symbol_c( CORE_SYM_PRECISION, CORE_SYM_NAME ) )


int64_t to_int64(const string& s) {
    return strtoll(s.c_str(), NULL, 10);
}

// modify from from_string in libraries/chain/asset.cpp
// use it for test purpose only
asset asset_from_string(const string& from)
{
    string s = trim_copy(from);

    // Find space in order to split amount and symbol
    auto space_pos = s.find(' ');
    eosio::check((space_pos != string::npos), "Asset's amount and symbol should be separated with space");
    auto symbol_str = trim_copy(s.substr(space_pos + 1));
    auto amount_str = s.substr(0, space_pos);

    // Ensure that if decimal point is used (.), decimal fraction is specified
    auto dot_pos = amount_str.find('.');
    if (dot_pos != string::npos) {
        eosio::check((dot_pos != amount_str.size() - 1), "Missing decimal fraction after decimal point");
    }

    // Parse symbol
    uint8_t precision_digit;
    if (dot_pos != string::npos) {
        precision_digit = amount_str.size() - dot_pos - 1;
    } else {
        precision_digit = 0;
    }

    auto sym = eosio::symbol(symbol_str, precision_digit);

    // Parse amount
    int64_t int_part;
    int64_t fract_part;
    if (dot_pos != string::npos) {
        int_part = to_int64(amount_str.substr(0, dot_pos));
        fract_part = to_int64(amount_str.substr(dot_pos + 1));
        if (amount_str[0] == '-') fract_part *= -1;
    } else {
        int_part = to_int64(amount_str);
    }

    int64_t amount = int_part;
    for (int i=0; i<sym.precision(); i++) {
        amount *= 10;
    }
    amount += fract_part;

    return asset(amount, sym);
}

struct core_sym {
   static inline eosio::asset from_string(const std::string& s) {
     return asset_from_string(s + " " CORE_SYM_NAME);
   }
};
