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
