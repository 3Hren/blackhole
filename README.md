Blackhole - eating your logs with pleasure
==========================================
[ ![Codeship Status for 3Hren/blackhole](https://codeship.com/projects/8d0e44f0-64ac-0133-20de-4a7e5d8c8004/status?branch=master)](https://codeship.com/projects/113228)

This is pre-release for Blackhole 1.0.0. For clearness I've dropped all the code and started with empty project.

Some parts (almost all formatters and sinks) will be borrowed from v0.5 branch.

Other stuff will be rewritten completely using C++11/14 standard with minimal boost impact.

If you want stable, but deprecated version, please switch to v0.5 branch.

# Features

## Attributes

Attributes is the core feature of Blackhole. Technically speaking it's a key-value pairs escorting every logging record.

For example we have HTTP/1.1 server which produces access logs like:

```
[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326 - SUCCESS
```

It can be splitted into indexes or attributes:

```
message:   SUCCESS
host:      [::]
user:      esafronov
timestamp: 10/Oct/2000:13:55:36 -0700
method:    GET
uri:       /porn.png
protocol:  HTTP/1.0
status:    200
elapsed:   2326
```

Blackhole allows to specify any number of attributes you want, providing an ability to work with them before of while
you writing them into its final destination. For example, Elasticsearch.

## Shared library

Despite the header-only dark past now Blackhole is developing as a shared library. Such radical change of distributing
process was chosen because of many reasons.

Mainly, header-only libraries has one big disadvantage: any code change may (or not) result in recompiling all its dependencies, otherwise having weird runtime errors with symbol loading race.

The other reason was the personal aim to reduce compile time, because it was fucking huge!

Of course there are disadvantages, such as virtual function call cost and closed doors for inlining, but here my personal benchmark-driven development helped to avoid performance degradation.

## Planning

- [x] Shared library.
- [x] Inline namespaces.
- [x] Optional compile-time inline messages transformation (C++14).
  - [ ] Compile-time placeholder type checking.
  - [ ] Compile-time placeholder spec checking (?).
- [x] Python-like formatting (no printf-like formatting support) both inline and result messages.
- [x] Attributes.
- [x] Scoped attributes.
- [x] Wrappers.
- [x] Custom verbosity.
- [x] Custom attributes formatting.
- [ ] Optional asynchronous pipelining.
  - [ ] Queue with block on overload.
  - [ ] Queue with drop on overload (count dropped message).
- [x] Formatters.
  - [x] String by pattern.
  - [x] JSON with tree reconstruction.
- [ ] Sinks.
  - [x] Colored terminal output.
  - [x] Files.
  - [ ] Syslog.  
  - [x] Socket UDP.
  - [x] Socket TCP.
    - [x] Blocking.
    - [ ] Non blocking.
- [ ] Scatter-gathered IO (?)
- [x] Logger builder.
- [ ] Macro with line and filename attributes.
- [x] Initializer from JSON (filename, string).
- [ ] Inflector.
- [ ] Filter category for sinks, handlers and loggers.

## Formatters

Formatters in Blackhole are responsible for converting every log record passing into some byte array representation. It can be either human-readable string, JSON tree or even [protobuf](https://github.com/google/protobuf) packed frame.

### String

String formatter provides an ability to configure your logging output using pattern mechanics with powerful
customization support.

Unlike previous Blackhole versions now string formatter uses python-like syntax for describing patterns with using *{}*
placeholders and format specifications inside. Moreover now you can specify timestamp specification directly inside the
general pattern or even format it as an microseconds number since epoch.

For example we have the given pattern:
```
[{severity:>7}] [{timestamp:{%Y-%m-%d %H:%M:%S.%f}s}] {scope}: {message}
```

After applying some log events we expect to receive something like this:
```
[  DEBUG] [2015-11-19 19:02:30.836222] accept: HTTP/1.1 GET - / - 200, 4238
[   INFO] [2015-11-19 19:02:32.106331] config: server has reload its config in 200 ms
[WARNING] [2015-11-19 19:03:12.176262] accept: HTTP/1.1 GET - /info - 404, 829
[  ERROR] [2015-11-19 19:03:12.002127] accept: HTTP/1.1 GET - /info - 503, 829
```

As you may notice the severity field is aligned to the right border (see that *>7* spec in pattern), the timestamp is
formatted using default representation with a microseconds extension and so on. Because Blackhole is all about
attributes you can place and format every custom attribute you want, as we just done with *scope* attribute.

The Blackhole supports several predefined attributes, with convenient specifications:

| Placeholder              | Description                                                   |
|--------------------------|---------------------------------------------------------------|
|{severity:s}              | User provided severity string representation                  |
|{severity}, {severity:d}  | Numeric severity value                                        |
|{timestamp:d}             | Number of microseconds since Unix epoch                       |
|{timestamp:{spec}s}       | String representation using *strftime* specification          |
|{timestamp}, {timestamp:s}| The same as *{timestamp:{%Y-%m-%d %H:%M:%S.%f}s}*             |
|{process:s}               | Process name                                                  |
|{process}, {process:d}    | PID                                                           |
|{thread}, {thread::x}     | Thread hex id as an opaque value returned by *pthread_self(3)*|
|{thread:s}                | Thread name or *unnnamed*                                     |
|{message}                 | Logging message                                               |
|{...}                     | All user declared attributes                                  |

For more information please read the documentation and visit the following links:

 - http://cppformat.github.io/latest/syntax.html - general syntax.
 - http://en.cppreference.com/w/cpp/chrono/c/strftime - timestamp spec extension.

### JSON.
JSON formatter provides an ability to format a logging record into a structured JSON tree with attribute handling features, like renaming, routing, mutating and much more.

## Sinks

### Null
Sometimes we need to just drop all logging events no matter what, for example to benchmarking purposes. For these cases there is null appender, which just ignores all records.

- Stream.
- Term.
- File.

### Socket
The socket sinks category contains sinks that write their output to a remote destination specified by a host and port. Currently the data can be sent over either TCP or UDP.

#### TCP
This appender emits formatted logging events using connected TCP socket.

| Option | Type  | Description|
|--------|:-----:|------------|
|host    |string | **Required**.<br/> The name or address of the system that is listening for log events. |
|port    |u16    | **Required**.<br/> The port on the host that is listening for log events. |

#### UDP
Nuff said.

## Runtime Type Information

The library can be successfully compiled and used without RTTI (with *-fno-rtti* flag).

## Possible bottlenecks

- Timestamp formatting
 - [ ] Using system clock - can be replaces with OS specific clocks.
 - [ ] Using `gmtime` - manual `std::tm` generation without mutex shit.
 - [ ] Temporary buffer - affects, but not so much.

# Why another logging library?

That's the first question I ask myself when see *yet another silver-buller library*.

First of all, we required a logger with attributes support. Here `boost::log` was fine, but it didn't compile in our compilers. Sad. After that we've realized that one of our bottlenecks is located in logging part, that's why `boost::log` and `log4cxx` weren't fit in our requirements. Thirdly we are developing for stable, but old linux distributives with relatively old compilers that supports only basic part of C++11.

At last, but not least, all that libraries has one fatal disadvantage - [NIH](https://en.wikipedia.org/wiki/Not_invented_here).

So here we are.

To be honest, let's describe some popular logging libraries, its advantages and disadvantages as one of them may fit your requirements and you may want to use them instead. It's okay.

### Boost.LogV2
Developed by another crazy Russian programmer using dark template magic and Vodka (not sure what was first). It's a perfect and powerful library, seriously.

**Pros:**
- It's a fucking **boost**! Many people don't want to depend on another library, wishing to just `apt get install` instead.
- Have attributes too.
- Large community, less bugs.
- Highly configurable.
- Good documentation.

**Cons:**
- Sadly, but you are restricted with the latest boost versions.
- Hard to hack and extend unless you are fine with templates, template of templates and variadic templates of a templated templates with templates. Or you are Andrei Alexandrescu.
- Relatively poor performance. Higher than `log4cxx` have, but not enough for us.
- Requires RTTI.

### Log4cxx
Logging framework for C++ patterned after Apache log4j. Yeah, Java.

**Pros:**
- Absolutely zero barrier to entry. Really, you just copy-paste the code from tutorial and it works. Amazing!

**Cons:**
- Leaking.
- APR.
- Have no attributes.
- Really slow performance.
- Seems like it's not really supported now.

### Spdlog
Extremely ultra bloody fucking fast logging library. At least the documentation says that. Faster than speed of light!

But everyone knows that even the light is unable to leave from blackhole.

**Pros:**
- Really fast, I checked.
- Header only. Not sure it's an advantage, but for small projects it's fine.
- Easy to extend, because the code itself is plain, straightforward and magically easy to understand.
- No dependencies.
- Nice kitty in author's avatar.

**Cons:**
- Again no attributes, no custom filtering, no custom verbosity levels. You are restricted to functionality provided by this library, nothing more.

# Notable changes
First of all, the entire library was completely rewritten for performance reasons.

- No more attribute copying unless it's really required (for asynchronous logging for example). Nested attributes are now organized in flattened range.
- Dropped `boost::format` into the Hell. It's hard to find more slower library for formatting both in compilation stage and runtime. Instead, the perfect [cppformat](https://github.com/cppformat/cppformat) library with an own compile-time constexpr extensions is used.
- There are predefined attributes with fast read access, like `message`, `severity`, `timestmap` etc.
- With **cppformat** participation there is new Python-like format syntax using placeholder replacement.
- Severity mapping from its numeric representation to strings can now be configured from generic configuration source (from file for example).
- ...

# Requirements

- C++11/14/17 compiler (yep, using C++17 opens additional functionalities).
- Boost.Thread - for TLS.

# Development
## Git workflow

Each feature and fix is developed in a separate branch. Bugs which are discovered during development of a certain feature, may be fixed in the same branch as their parent issue. This is also true for small features.

### Branch structure:
- `master`: master branch - contains stable, working version of VM code.
- `develop`: development branch - all fixes and features are first merged here.
- `issue/<number>/<slug>` or `issue/<slug>`: for issues (both enhancement and bug fixes).
