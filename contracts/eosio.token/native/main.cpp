#include <eosio/eosio.hpp>
#include <eosio.token/eosio.token.hpp>

using namespace eosio;

template<typename T, typename... Args>
bool native_execute_action( name self, name code, void (T::*func)(Args...)  ) {
   size_t size = action_data_size();

   //using malloc/free here potentially is not exception-safe, although WASM doesn't support exceptions
   constexpr size_t max_stack_buffer_size = 512;
   void* buffer = nullptr;
   if( size > 0 ) {
      buffer = max_stack_buffer_size < size ? malloc(size) : alloca(size);
      read_action_data( buffer, size );
   }

   std::tuple<std::decay_t<Args>...> args;
   datastream<const char*> ds((char*)buffer, size);
   ds >> args;

   // if there is an exception throw from action in debug mode, do not call destructor to avoid mess up by exception throw from destructor.
   T *inst = new T(self, code, ds);

   auto f2 = [&]( auto... a ){
      ((inst)->*func)( a... );
   };

   boost::mp11::tuple_apply( f2, args );
   if ( max_stack_buffer_size < size ) {
      free(buffer);
   }
   delete inst;
   return true;
}

extern "C" __attribute__ ((visibility ("default"))) void token_native_apply( uint64_t receiver, uint64_t code, uint64_t action ) {
   if (code == receiver) {
      switch(action) {
         case "create"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosio::token::create );
            break;
         case "issue"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosio::token::issue );
            break;
         case "retire"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosio::token::retire );
            break;
         case "transfer"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosio::token::transfer );
            break;
         case "open"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosio::token::open );
            break;
         case "close"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosio::token::close );
            break;
      }
   }
}
