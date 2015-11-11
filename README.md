# Warning

This is pre-release for Blackhole 1.0.0. For clearness I've dropped all the code and started with empty project.

Some parts (almost all formatters and sinks) will be borrowed from v0.5 branch.

Other stuff will be rewritten completely using C++11/14 standard with minimal boost impact.

If you want stable, but deprecated version, please switch to v0.5 branch.

## Features

### Attributes

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

### Shared library

Despite the header-only dark past now Blackhole is developing as a shared library. Such radical change of distributing
process was chosen because of many reasons.

Mainly, header-only libraries has one big disadvantage: any code change may (or not) result in recompiling all its dependencies, otherwise having weird runtime errors with symbol loading race.

The other reason was the personal aim to reduce compile time, because it was fucking huge!

Of course there are disadvantages, such as virtual function call cost and closed doors for inlining, but here my personal benchmark-driven development helped to avoid performance degradation.

### Planning

- [x] Shared library.
- [ ] Inline namespaces (?).
- [x] Optional compile-time inline messages transformation (C++14).
- [x] Python-like formatting (no printf-like formatting support) both inline and result messages.
- [x] Attributes.
- [ ] Scatter-gathered IO (?)
- [ ] Scoped attributes.
- [ ] Optional thread-safety (?).
- [x] Wrappers.
- [ ] Custom verbosity.
- [x] Custom attributes formatting.
- [ ] Optional asynchronous pipelining.
-   [ ] With block on overload.
-   [ ] With drop on overload (count dropped message).
- [ ] Colored terminal output.
- [ ] Builder.
- [ ] Macro with line and filename attributes.
- [ ] Initializer from json (filename, string).

### Formatters

- String.
- Json.

### Sinks

- Null.
- Stream.
- Term.
- File.
- Socket.

### Requirements

- C++11/14 compiler
