#`JSON`-formatter
Maps log event to JSON tree. It takes attributes of log message and represents this `attribute_name=attribute_value` pair into JSON objects.

For example

```
BH_LOG(log, severity, "message")("attr1", "val1", "attr2", "val2");
```

produce

```
{
	"timestamp":time,
    "severity":severity,
    "message":"text",
    "attr1":"val1",
    "attr2","val2"
 }
 ```

 Using configuration parameters for this formatter you can
   * Rename parameters;
   * Construct hierarchical tree.

##Registration
When we register `json`-formatter with some sink `json_t` should be used. For example

```
repository_t::instance().configure<sink::syslog_t<level>, formatter::json_t>();
```

This code register `syslog`-sink and `json`-formatter pair. How to register another combinations of sinks and formatters check the ["Registration rules" article](registration-rules.md).

##Configuration

Example of configuration

```
formatter_config_t formatter("json");
formatter["newline"] = true;
formatter["mapping"]["message"] = "@message";
formatter["mapping"]["timestamp"] = "@timestamp";
formatter["routing"]["/"] = dynamic_t::array_t { "message", "timestamp" };
formatter["routing"]["/fields"] = "*";
```

  * `newline` - The newline symbol will be added to the end of document if `true`.
  * `mapping` - This property is used to rename attributes. For example setting `formatter["mapping"]["message"] = "@message";` leads to object {"@message":"text"} istead of {"message":"text"}.
  * `routing` - This property is used for tree formatting.

If we will call

```
BH_LOG(log, level::debug, "debug event")("host", "127.0.0.1", "issue", "training");
```

The next output will be produced

```
{
	"@message":"debug event",
	"@timestamp":"time",
	"fields":{
		"host":"127.0.0.1",
		"issue":"training"
	},
}
```


##Example
In development.

[Back to content](contents.md)