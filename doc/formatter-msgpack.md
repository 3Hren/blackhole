#`Msgpack`-formatter
This formatter get pairs of `attribute_name:attribute_value` and convert it to the msgpack object.

##Registration
When we register `msgpack`-formatter with some sink `msgpack_t` should be used. For example

```
repository_t::instance().configure<sink::syslog_t<level>, formatter::msgpack_t>();
```

##Configuration

It doesn't have any configuration parameter and you just need to type formatter as "msgpack"

```
formatter_config_t formatter("msgpack");
```

##Example
In development.

[Back to content](contents.md)