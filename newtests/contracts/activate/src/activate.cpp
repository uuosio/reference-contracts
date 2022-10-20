#include <activate/activate.hpp>
#include <eosio/eosio.hpp>

namespace eosiobios {

void bios::activate( const eosio::checksum256& feature_digest ) {
   require_auth( get_self() );
   preactivate_feature( feature_digest );
}

}
