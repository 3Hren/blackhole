//!@todo: performance: Review string formatting. Use attached stream.
//!@todo: api: Make elasticsearch frontend. Use swarm + asio and only index request with simple response checking.
//!@todo: api: Ability to set global attribute mapper (?)
//!@todo: api: Global thread-safe guarantee. Do not lock if underlying class is thread-safe itself.

//!@todo: api: Make fallback logger. Make it configurable.
//!@todo: aux: Make internal exception class with attribute keeping, e.g. line, file or path.
//!@todo: msgpack_t: Attribute mappings.
//!@todo: socket_t: Make asynchronous TCP backend.

//!@todo: benchmark: File logging comparing with boost::log.
//!@todo: benchmark: Socket logging with json.
