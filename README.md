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
- [ ] Inline namespaces (?).
- [x] Optional compile-time inline messages transformation (C++14).
- [x] Python-like formatting (no printf-like formatting support) both inline and result messages.
- [x] Attributes.
- [ ] Scatter-gathered IO (?)
- [x] Scoped attributes.
- [x] Wrappers.
- [x] Custom verbosity.
- [x] Custom attributes formatting.
- [ ] Optional asynchronous pipelining.
-   [ ] Queue with block on overload.
-   [ ] Queue with drop on overload (count dropped message).
- [ ] Colored terminal output.
- [ ] Logger builder.
- [ ] Macro with line and filename attributes.
- [ ] Initializer from json (filename, string).

## Formatters

Formatters in Blackhole are responsible for converting every log record passing into some byte array representation. It can be either human-readable string, json tree or even [protobuf](https://github.com/google/protobuf) packed frame.

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

For more information please read the documentation and visit the following links:

 - http://cppformat.github.io/latest/syntax.html - general syntax.
 - http://en.cppreference.com/w/cpp/chrono/c/strftime - timestamp spec extension.

### Json.

## Sinks

- Null.
- Stream.
- Term.
- File.
- Socket.

## Requirements

- C++11/14 compiler
