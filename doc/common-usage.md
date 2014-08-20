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


[Back to contents](contents.md)