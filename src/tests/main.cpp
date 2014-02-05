#include "Global.hpp"

/*! @brief Log library specification.
 *
 *  === TYPICAL USAGE VIA MACRO ===
 *
 *  LOG(log, debug, "some information: %s at [%d]", "nothing", 100500, {
 *      keyword::id(42),
 *      keyword::address("127.0.0.1")
 *  });
 *
 *  === TYPYCAL USAGE VIA LOGGER OBJECT ===
 *
 *  log.debug("some information: %s at [%d]", "nothing", 100500, {
 *      keyword::id(42),
 *      keyword::address("127.0.0.1")
 *  });
 *
 *  === YOU CAN CREATE OWN ATTRIBUTES DYNAMICALLY ===
 *
 *  attribute::make("name", "value");
 *
 *
 *  === FORMATTERS ===
 *
 * 1. string: pattern="[%(timestamp)s] [%(level)s]: %(message)s : [%(...L)s]"
 *
 *  === SINKS ===
 *
 * socket: host, port
 *   udp.
 *   tcp:
 *     sync.
 *     async:
 *       queue_strategy.
 *
 *  === USAGE FROM PYTHON ===
 *
 *  log.debug("message from '%s'", "Hell", { "owner": "Satan", "email": "satan@mail.hell" })
 *
 *  === CONFIGURING FROM FILE ===
 *
 * verbosity: info
 * sink: files
 *   path: [/]
 *   format: string
 *     pattern: [%(...)s] ...
 *
 * sink: tcp/udp
 *   address: 0.0.0.0
 *   port: 5000
 *   format: msgpack/json
 *
 * sink: syslog
 *   identity: blah
 *   format: string
 *     pattern: [%(...)s] ...
 *
 */

int main(int argc, char** argv) {
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
