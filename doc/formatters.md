# Formatters

Formatters are entities that map log event to string. Currently there are three built-in formatters:

 - **String.**

   Maps log event to plain string by specified pattern with attributes support.
   
   - **Pattern**: Generic log message with optional placeholders. Every log event contains named attributes. These attributes are substituted in its named placeholder resulting in formatted string. For example: `[%(timestamp)s] [%(level)s] %(tid)s: %(message)s [%(...L)s]` after substituting attribute values transforms into `[2014-02-19 11:57:53] [INFO] 0x7fff7134310: this is test message ['id': 42]`. Placeholder `%(...L)s` means substitute **all** attributes with specified scope.
     - Exceptions: An exception will be thrown if no attribute with specific name found.
   
 - **Json.**
 
   Maps log event to json tree.

 - **Msgpack.**

