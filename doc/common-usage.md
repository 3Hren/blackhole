#Common usage

The first you need to do in your program is to include header

```(c++)
#include <blackhole/blackhole.hpp>
```

Sometimes you need to include additional headers for example for [`syslog` sink](sink-syslog.md). Be carefull while reading corresponding parts of documentation.


Another important step is to define severity levels. It may look as follows

```
enum class severity_levels {
    debug,
    info,
    warning,
    error
};
```

You are absolutely free in this process. You can specify so much levels as you need. You can even not to specify any levels. Blackhole supports both strongly and weakly typed enumerations for severity and it's better to use strongly-typed one. You should be carefull with [`syslog` sink](sink-syslog.md) to be consistent with the system severity levels.


The next step is to configure frontend (formatter/sink pair) and pass configuration into the `repository_t` object.

When all of yours frontends are configured create logger objects and use them.

```(c++)
verbose_logger_t<severity_levels> log_name = repository_t::instance().logger_frontend_name<severity_levels>();
verbose_logger_t<severity_levels> log_another_name = repository_t::instance().another_logger_frontend_name<severity_levels>();
```


Why sinks and formatters doesn't use strong formatted types in configuration?
*Why not using strongly-typed configuration objects? Mainly because one of the requirements to logger was to be able to initialize it through configuration file, string or whatever else. Internally that weakly and unsafe configuration objects are mapped to its actual formatter's or sink's settings. Intentionally that mappers must be written by developer who has written that formatter/sink. It opens possibility to have full control over which settings must be specified everything and which of them can be passed or initialized by default. Developer also controls error handling. If some required options are not provided or there is typo mistake an exception will be thrown at moment of creation logger object via repository.*

### For those who despise macros
Sometimes it is useful to handle log object directly without using any magic macro. Shortly I'll describe it now. For more advanced documentation you should go to the logger object's reference.

Every log object has `open_record` method which accepts attributes set and returns `log_record_t` object if that attributes set passes filtering stage. If not - an invalid `log_record_t` object is returned. For that reason returned record object must be checked for validity using `record.valid()` method. For our case log record will be created anyway, because we don't have any filters registered now.

The next thing we must do after checking record's validity - is to add message attribute into the record, because our formatter requires it, otherwise we got an formatting exception, which will be eaten by the logger internally or passed to the fallback logger. Entire function is:

```
template<typename Log>
void debug(Log& log, level lvl, const char* message) {    
    log::record_t record = log.open_record(lvl);
    if (record.valid()) {        
        record.attributes.insert({
            keyword::message() = message
        });
        log.push(std::move(record));
    }
}
```

And the usage is:

```
debug(log, level::debug, "log message using log object directly");
```

[Back to contents](contents.md)