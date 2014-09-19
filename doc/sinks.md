# Sinks

Sinks are designed for sending formatted message to its final destination.

Blackhole supports the next sinks out of the box:

  * [Elasticsearch](sink-elasticsearch.md). Client to [Elasticsearch](http://www.elasticsearch.org/) backend.
  * [Files](sink-files.md). Writes logs to the files. Supports log rotation.
  * [Null](sink-null.md). Transports messages to nowhere. Can be usefull for testing or benchmarks.
  * [Socket](sink-socket.md). Sends messages through the network. Supports TCP and UDP.
  * [Stream](sink-stream.md). Supports `stdout` and `stderr` by default.
  * [Syslog](sink-syslog.md). Writes logs to `syslog`.

[Back to contents](contents.md)