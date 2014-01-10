//!@todo: aux: More typed configs for sinks.

//!@todo: msgpack_t: Attribute mappings.
//!@todo: api: Make fallback logger. Make it configurable.
//!@todo: aux: Make internal exception class with attribute keeping, e.g. line, file or path.
//!@todo: performance: Current naive implementation of timestamp formatter is suck and have large performance troubles. Fix it.
//!@todo: performance: Experiment with std::ostringstream or format library for performance check.
//!@todo: filter_t: Make || operations in filtering.
//!@todo: api: Using direct keyword stringify substitution in log macro, i.e. LOG("level=%s", keyword::severity<level>);
//!@todo: api: Mapper.add<T>(keyword, function) - use keyword or string.
//!@todo: files_t: Make auto_flush flag.
//!@todo: files_t: Make file rotation.
//!@todo: files_t: Make file naming by pattern.
//!@todo: files_t: Create directory if not exists.
//!@todo: socket_t: Make asynchronous TCP backend.
