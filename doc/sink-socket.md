#`Socket`-sink

This sink transport log messages through the network to specified address by TCP or UDP protocol.

When sink works in TCP mode connection can be broken on some reasons. In this case sink will try to restart connection once and rise an exeption in case of unsuccess. The next connection attempt will be with the next log message.

It is not thread-safe.

You need additional header

```
#include <blackhole/sink/socket.hpp>
```

##Registration

Since `socket` sink has two different  hypostasis `tcp` and `udp` you can register it as

```
repository_t::instance().configure<sink::socket_t<boost::asio::ip::tcp>, formatter::string_t>();
//and
repository_t::instance().configure<sink::socket_t<boost::asio::ip::udp>, formatter::string_t>();
```

The code above register `tcp/udp socket`-sink and `string`-formatter pair. How to register another combinations of sinks and formatters check the ["Registration rules" article](registration-rules.md).

##Configuration

Example:

```
sink_config_t sink("tcp");
sink["host"] = "localhost";
sink["port"] = 50030;
```

You can create sink with "tcp" and "udp" properties according to registered type.

  * `host` is the host name of IP-address.
  * `port` is the port (suddenly).

Blackhole connects to `host`:`port` (in case of `tcp`) and send messages to that address (in both `tcp` and `udp` cases).

##Example

In development.