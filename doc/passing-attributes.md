##Passing of attributes to the macro
Welcome to the **dynamic attributes** setting into the log event! Every log event can transport any number of additional attributes, which will participate in all subsequent operations associated with it (look at the architecture diagram, if you forgot).

How is it imlemented?

Actually, `BH_LOG` macro returns functional object, which accepts variadic pack that is analizyd in four ways.

**First** way is to explicitly create attributes objects using helper `attributes::make` function:

```
BH_LOG(log, level::debug, "debug event")(
    attribute::make("host", "localhost"),
    attribute::make("answer", 42.0),
    attribute::make("code", 404)
);
```

When we can pass into the `attribute::make()` call one more parameter ( 'L' | 'E' | 'G' | 'T' | 'U' ) that will be treated as the type of the attribute.

  * 'L' - Local attribute;
  * 'E' - Event attribute;
  * 'G' - Global attribute;
  * 'T' - Thread attribute;
  * 'U' - Universe attribute.

For example

```
BH_LOG(log, level::debug, "debug event")(attribute::make("host", "localhost",'G'),
```

**All additional (in the second parentesses) attributes have type `L` by default.**

**Additionly** you can use registered **keywords**:
```
BH_LOG(log, level::debug, "debug event")(
    keyword::host() = "localhost",
    keyword::answer() = 42.0,
    keyword::code() = 404
);
```

Before using keywords you should register it by youself. Blackhole has three registered keyword by default `timestamp`, `severity` and `message`. Once registered keyword couldn't be re-registered. 

??????????????????How to register keyword???????????????????????????????



In fact it is syntactic sugar over the first method. Registered keywords have defined `operator=`, which returns attribute object therefore the first and the second method can be combined:

```
BH_LOG(log, level::debug, "debug event")(
    attribute::make("host", "localhost"),
    keyword::answer() = 42.0,
    attribute::make("code", 404)
);
```

Keywords can be useful not only for these purposes. More details about them will be discussed in reference documentation.

The **third** method is relatively recent. It allows you to specify a list of attributes as key-value pairs, where the key should be converted string and value - should be implicitly converted to `attribute_value_t`. In code, it looks like this:
```
BH_LOG(log, level::debug, "debug event")(
    "host", "localhost",
    "answer", 42.0,
    "code", 404
);
```

Argument correctness is verified at compile time, so if something goes wrong will be given a human-readable `static_assert`.

The **fourth** way involves initialization lists usage, which gives for some people more clean code:

```
BH_LOG(log, level::debug, "debug event")(attribute::list({
    {"host", "localhost"},
    {"answer", 42.0},
    {"code", 404}
}));
```

*Note, that some compilers allows you not to specify `attribute::list` type explicitly. For example:*

```
BH_LOG(log, level::debug, "debug event")({
    {"host", "localhost"},
    {"answer", 42.0},
    {"code", 404}
});
```

*Actually, only GCC 4.6 doesn't allow you to write like this, because it violates the C++11 Standard (see '-fno-deduce-init-list' extension for more details).*

Which of the following methods to choose - is your decision, depending on what you prefer.