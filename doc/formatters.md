# Formatters

Formatters are entities that map log event to string. Blackhole supports three built-in formatters:

  * [String](formatter-string.md). Maps log event to plain string by specified pattern with attributes support.
  * [JSON](formatter-json.md). Maps log event to JSON tree.
  * [Msgpack](formatter-msgpack). Gets pairs of `attribute_name:attribute_value` and convert it to the msgpack object.

[Back to contents](contents.md)