#Why the registration is needed

What? Why so complicated? To fully understand why I've made so strange architecture decision, let's look deeper inside the library.

Internally Blackhole has very few dynamicly coupled components. In other words, there are very few classes with *virtual* methods. That decision were made for two reasons.

The first one was an attempt to minimize virtual methods invocation overhead. Blackhole was initially planned as high-performance solution for logging and the main requirement was the fastest log processing .

The second reason was to have an ability to test separate components of the library, replacing others by mock-objects by the power of TDD. The alternative solution would require interface definition for every subcomponent. For example, files sink executes only basic operations with internal backend's interface and it is the backend who handles files itself. Also as a separate component of files sink there is so-called rotator, an interface which determines whether rotation should occur and which handles file rotation. 

To simplify the testing of all this zoo Alexandrescu's [Policy Based Design](http://en.wikipedia.org/wiki/Policy-based_design) was chosen, but it fits this case perfectly.

So, once again, Blackhole has very fwe dynamically coupled components. But they are there, and one of them - is **frontend**. Logger object itself operates with list of frontends and it doesn't care what logic it implements. This opens the possibility of creating frontend plugins and makes available to create extendable plugin system.

So why to register the components used statically? Some library components (formatters, sinks or frontends) can require additional properties or functions definition known at compile-time. For example, syslog sink requires the user to define mapping function that maps user defined severity level enumeration to the syslog's severity.

Just in case, repository's register method's signature is:



```
template<typename Level>
template<class Sink, class Formatter>
void repository_t<Level>::instance().configure();
```

Required logger's object can be properly created only after registering frontend with corresponding configuration. Without registration an exception will be thrown.

========

As we register sink-formatter pair of definite types the following configuration of them is predefined too. Let's register `syslog`-sink with the `string`-formatter:

```
repository_t::instance().configure<sink::syslog_t<level>, formatter::string_t>();
```

Above registration means that we should configure exactly `syslog` with the `string` for example as follows:

```
    formatter_config_t formatter("string");
    formatter["pattern"] = "%(message)s";

    sink_config_t sink("syslog");
    sink["identity"] = "test-application";
```

Otherwise (with another types of sink and/or formatter) repository method `add_config` will rise an exception

```
    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "syslog_string", { frontend } };

    repository_t::instance().add_config(config);
```

========