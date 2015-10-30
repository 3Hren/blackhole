# Warning

This is pre-release for Blackhole 1.0.0. For clearness I've dropped all the code and started with empty project.

Some parts (almost all formatters and sinks) will be borrowed from v0.5 branch.

Other stuff will be rewritten completely using C++11/14 standard with minimal boost impact.

If you want stable, but deprecated version, please switch to v0.5 branch.

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
