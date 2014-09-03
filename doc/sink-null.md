#`Null`-sink
If you need to push log message to nowhere this sink is your choice. Why you may need so? For testing purpose or in benchmarks for example.

This is a thread-safe sink.

##Registration

Registration code looks like follows:

```
repository_t::instance().configure<sink::null_t, formatter::string_t>();
```

The code above register `null`-sink and `string`-formatter pair. How to register another combinations of sinks and formatters check the ["Registration rules"](registration-rules.md) article.

##Configuration

Sink doesn't have any configuration parameters, you just need to create object with `null` type

```
sink_config_t sink("null");
```

##Example
In development.

[Back to contents](contents.md)