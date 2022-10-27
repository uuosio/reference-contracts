#include <eosio/eosio.hpp>
#include <eosio.system/eosio.system.hpp>

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


extern "C" __attribute__ ((visibility ("default"))) void system_native_apply( uint64_t receiver, uint64_t code, uint64_t action ) {
   if (code == receiver) {
      switch(action) {
         case "init"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::init );
            break;
         case "setacctram"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::setacctram );
            break;
         case "setacctnet"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::setacctnet );
            break;
         case "setacctcpu"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::setacctcpu );
            break;
         case "activate"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::activate );
            break;
         case "delegatebw"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::delegatebw );
            break;
         case "deposit"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::deposit );
            break;
         case "withdraw"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::withdraw );
            break;
         case "buyrex"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::buyrex );
            break;
         case "unstaketorex"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::unstaketorex );
            break;
         case "sellrex"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::sellrex );
            break;
         case "cnclrexorder"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::cnclrexorder );
            break;
         case "rentcpu"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::rentcpu );
            break;
         case "rentnet"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::rentnet );
            break;
         case "fundcpuloan"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::fundcpuloan );
            break;
         case "fundnetloan"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::fundnetloan );
            break;
         case "defcpuloan"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::defcpuloan );
            break;
         case "defnetloan"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::defnetloan );
            break;
         case "updaterex"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::updaterex );
            break;
         case "rexexec"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::rexexec );
            break;
         case "setrex"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::setrex );
            break;
         case "mvtosavings"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::mvtosavings );
            break;
         case "mvfrsavings"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::mvfrsavings );
            break;
         case "consolidate"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::consolidate );
            break;
         case "closerex"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::closerex );
            break;
         case "undelegatebw"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::undelegatebw );
            break;
         case "buyram"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::buyram );
            break;
         case "buyrambytes"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::buyrambytes );
            break;
         case "sellram"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::sellram );
            break;
         case "refund"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::refund );
            break;
         case "regproducer"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::regproducer );
            break;
         case "regproducer2"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::regproducer2 );
            break;
         case "unregprod"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::unregprod );
            break;
         case "setram"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::setram );
            break;
         case "setramrate"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::setramrate );
            break;
         case "voteproducer"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::voteproducer );
            break;
         case "voteupdate"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::voteupdate );
            break;
         case "regproxy"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::regproxy );
            break;
         case "claimrewards"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::claimrewards );
            break;
         case "rmvproducer"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::rmvproducer );
            break;
         case "updtrevision"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::updtrevision );
            break;
         case "bidname"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::bidname );
            break;
         case "bidrefund"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::bidrefund );
            break;
         case "setpriv"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::setpriv );
            break;
         case "setalimits"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::setalimits );
            break;
         case "setparams"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::setparams );
            break;
         case "setinflation"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::setinflation );
            break;
         case "cfgpowerup"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::cfgpowerup );
            break;
         case "powerupexec"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::powerupexec );
            break;
         case "powerup"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::powerup );
            break;
         case "onblock"_n.value:
            native_execute_action( eosio::name(receiver), eosio::name(code), &eosiosystem::system_contract::onblock );
            break;
      }
   }
}
