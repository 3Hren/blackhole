#Messages filtering

Here we discuss the questions of messages filtering.

```
verbose_logger_t<level> log
```

Logger has a method `set_filter` that accept custom function as an argument. Custom function should be of the next type

```
typedef std::function<bool(const attribute::set_view_t& attributes)> filter_t;
```

`attributes` is a hash table of "attribute_name : attribute_value" format. You can write any logic on attribute conditions check and return `true` or `false`. If `true` is returned then logger object will pass this string to sink.

How it can be used? For example you can implement filtering by severity.  Assuming you decided to use three levels of verbosity `debug`, `info` and `error` in your app. You can write funtion that checks level on log message. If message level higher then global app setting function return `false` and message will not be sent.

##Example
In development.

[Back to contents][contents.md]
