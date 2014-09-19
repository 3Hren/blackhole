# Coding tutorial

Before start this topic you should read the ["Main contepts"](main-concepts.md) part of documentation if you are not yet.

It this topic we will consider two examples that designed to quickly put you into the Blackhole workflow.

  * [The simplest example](#the-simplest-example)
  * [Log into the file with rotation](#log-into-the-files-with-rotation)

## The simplest example

Writing log to `stdout` with minimal settings.

The complete example code:

```
#include <blackhole/blackhole.hpp>

using namespace blackhole;

enum class level {
    debug,
    info,
    warning,
    error
};


void init() {
    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s] [%(severity)s]: %(message)s";

    sink_config_t sink("stream");
    sink["output"] = "stdout";

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "stdout_log", { frontend } }; //"stdout_log" is a name of our config

    repository_t::instance().add_config(config);
}


int main(int, char**) {
    
    init();

    //Do you remember what "stdout_log" it is? (Check the "init" function above)
    verbose_logger_t<level> log = repository_t::instance().create<level>("stdout_log"); 

    BH_LOG(log, level::debug,   "[%d] %s - done", 0, "debug");
    BH_LOG(log, level::info,    "[%d] %s - done", 1, "info");
    BH_LOG(log, level::warning, "[%d] %s - done", 2, "warning");
    BH_LOG(log, level::error,   "[%d] %s - done", 3, "error");
    return 0;
}
```

In Ubuntu you can compile it with the command 

```
g++ simple_example.cpp -osimple_example -std=c++0x -lboost_thread-mt`
```

`enum class level` is a declaration of severity levels for our logger. You are absolutely free in this process. You can specify so much levels as you need. You can even not to specify any levels. Blackhole supports both strongly and weakly typed enumerations for severity and it's better to use strongly-typed one. You should be carefull with [`syslog` sink](sink-syslog.md) to be consistent with the system severity levels.

All logger objects in Blackhole are got from `repository_t` object. It is a singleton that can store all possible frontends configurations. We need to create configuration (formatter and sink pair) and add it to the `repository_t` object as in `init()` function. 

Now you should create logger object as follows:

```
verbose_logger_t<level> log = repository_t::instance().create<level>("stdout_log"); //"stdout_log" is a name of previously created config
```

Having the logger object we can pass in to the main logging macro which has the next signature:

```
BH_LOG(verbose_logger_t<Severity> logger, Severity severity_level, const char* message, Args...);
```

`Args...` - are the message's placeholder arguments for pretty formatting like `printf` style.

As a result of our example, we see the following in the console:

```
    [1396259922.290809] [0]: [0] debug - done
    [1396259922.306820] [1]: [1] info - done
    [1396259922.306926] [2]: [2] warning - done
    [1396259922.307020] [3]: [3] error - done
```

Using of other sinks and formatters may be specific. Before use of them read the corresponding parts of documentation:
  * [Sinks](sinks.md);
  * [Formatters](formatters.md).

Now we recommend you to examine our next example on logging to the files.

## Log into the files with rotation

This example is about using file logger.
We consider how to register complex file sink with rotation support and how to configure it to have multiple files which names will be chosen depending on log event attributes.

The complete example code:

```
#include <blackhole/blackhole.hpp>
#include <blackhole/frontend/files.hpp>

using namespace blackhole;

enum class level {
    debug,
    error
};

void init() {
    //Registering of frontend
    repository_t::instance().configure<
        sink::files_t<
            sink::files::boost_backend_t,
            sink::rotator_t<
                sink::files::boost_backend_t,
                sink::rotation::watcher::size_t
            >
        >,
        formatter::string_t
    >();


    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s] - <%(severity)s>:%(message)s:Issue - %(issue)s";

    sink_config_t sink("files");
    sink["path"] = "./logs/blackhole-%(host)s.log";
    sink["autoflush"] = true;
    sink["rotation"]["pattern"] = "%(filename)s.%N";
    sink["rotation"]["backups"] = std::uint16_t(10);
    sink["rotation"]["size"] = std::uint64_t(4*1024);

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "rotating_file_log", { frontend } };

    repository_t::instance().add_config(config);
}

int main(int, char**) {

    init();
    
    verbose_logger_t<level> log = repository_t::instance().create<level>("rotating_file_log");

    for (int i = 0; i < 64; ++i) {
        BH_LOG(log, level::debug, "debug event")("host", "127.0.0.1", "issue", "The Ultimate Question of Life, the Universe, and Everything.");
        BH_LOG(log, level::error, "error event")("host", "localhost", "issue", "To be, or not to be: that is the question.");
    }

    return 0;
}

```

In Ubuntu you can compile it with the command 

```
g++ rotating_files.cpp -ofiles -std=c++0x -lboost_thread-mt -lboost_filesystem -lboost_system
```

The first difference from the first example you can see is a new header:

```
#include <blackhole/sink/files.hpp>
```

The next difference contained in the following piece of code:

```
repository_t::instance().configure<
        sink::files_t<
            sink::files::boost_backend_t,
            sink::rotator_t<
                sink::files::boost_backend_t,
                sink::rotation::watcher::size_t
            >
        >,
        formatter::string_t
    >();
```

This is a registration code of `files`-sink. It may look frustrating and smells like dark template magic. We are discussing on how to register frontends in the ["Registration rules"](registration-rules.md) part of the documentation.

Required logger's object can be properly created only after registering frontend with corresponding configuration. Without registration an exception will be thrown.

Formatter configuration has no any surprises, just an `%(issue)s` attribute that is not supported with our `BH_LOG` macro from the starting point of view. We will discuss it a bit later.

Configuration of `files`-sink with rotation support looks more interesting than the same for the `stream`-sink in the first example:

```
sink_config_t sink("files");
sink["path"] = "./logs/blackhole-%(host)s.log";
sink["autoflush"] = true;                           //Dump messages to the file immediately
sink["rotation"]["pattern"] = "%(filename)s.%N";    //Pattern for backup file name
sink["rotation"]["backups"] = std::uint16_t(10);    //Number of backups
sink["rotation"]["size"] = std::uint64_t(4*1024);   //Size of log file
```

`%(host)s` placeholder in an attribute of log event, but the `%(filename)s.%N` is not. `%(filename)s.%N` pattern leads to creation of backup filenames from the active log file substituting `%N` number of backup (from 1 to `backups`).

More about `files` sink you can find in [detailed description](sink-files.md) of sink.

Next steps should be already familiar for people who passes previous tutorial. We just create frontend and logger configuration objects and push the last one into the repository.

```
frontend_config_t frontend = { formatter, sink };
log_config_t config{ "rotating_file_log", { frontend } };

repository_t::instance().add_config(config);
```

Only remaining to record a message to log in the `main`-function:

```
BH_LOG(log, level::error, "error event")("host", "localhost", "issue", "To be, or not to be: that is the question.");
```

What do you think about the second braces near our macro?

Welcome to the **dynamic attributes** setting into the log event! Every log event can transport any number of additional attributes, which will participate in all subsequent operations associated with it (look at the architecture diagram, if you forgot). `%(hosts)s` and `%(issue)s` placeholders in code contain the names of this attributes which should be passed to `BH_LOG` in the second parentheses. You can pass this attributes in different manner, read the ["Passing attributes to the macro"](passing-attributes.md) article.

[Back to contents](contents.md)
