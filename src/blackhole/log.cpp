//!@todo: stream_t: Implement stream sink.(!)
//!@todo: example: Make stdout/string example with demonstration of formatting other attribute.(!)
//!@todo: api: More verbose error messages.(!)
//!@todo: api: Renaming repository methods.(!)

//!@todo: feature: Verbosity filter from file.(2d)
//!@todo: api: Ability to set global attribute mapper (?)
//!@todo: api: Microseconds support in timestamps.(1-2h)

//!@todo: benchmark: File logging comparing with boost::log.
//!@todo: benchmark: Socket logging with json.

//!@todo: files_t: Make file naming by pattern.
//!@todo: files_t: logrotate support - ability to handle signals to reopen current file (SIGHUP) (?).

//!@todo: performance: Experiment with std::ostringstream or format library for performance check.
//!@todo: performance: Current naive implementation of timestamp formatter is suck and have large performance troubles. Fix it.

//!@todo: api: Make fallback logger. Make it configurable.
//!@todo: aux: Make internal exception class with attribute keeping, e.g. line, file or path.
//!@todo: api: Maybe squish repository_t::init and ::configure methods?
//!@todo: msgpack_t: Attribute mappings.
//!@todo: socket_t: Make asynchronous TCP backend.
