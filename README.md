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
[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326
```

It can be splitted into indexes or attributes:

```
host: [::]
user: esafronov
message: ""
timestamp: 10/Oct/2000:13:55:36 -0700
method: GET
uri: /porn.png
protocol: HTTP/1.0
status: 200
elapsed: 2326
```

Blackhole allows to specify any number of attributes you want, providing an ability to work with them before of while
you writing them into its final destination. For example, Elasticsearch.

### Planning

- Shared library.
- Optional compile-time inline messages transformation.
  If literal and C++14 -> ok, else if literal - check arguments, else use `cppformat`.
- Python-like formatting (no printf-like formatting support) both inline and result messages.
- Attributes.
- Scatter-gathered IO (?)
- Scoped attributes.
- Optional thread-safety.
- Wrappers.
- Custom verbosity.
- Custom attributes formatting.
- Optional asynchronous queue.

### Formatters

- String.
- Json.

### Sinks

- Null.
- Stream.
- Files.
- Socket.

### Requirements

- C++11/14 compiler
