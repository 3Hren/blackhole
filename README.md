# Blackhole [![Build Status](https://travis-ci.org/3Hren/blackhole.png?branch=master)](https://travis-ci.org/3Hren/blackhole) 

Yet another logging library.

Please visit [project site](http://3hren.github.io/blackhole/) for more information, including design overview, tutorials, examples and library reference.

## Features

 - **Extremely fast.**

   Blackhole is designed to be the fastest logger in C++.
   This includes dark template magic inside, but you are not paying for things you don't use.
   
 - **Highly configurable.**
   
   You can configure all: supported frontends in your project, severity level enumeration, filtering,
   exception handling, formatting and configuration itself.
   Settings can be specified both in source code and from json or string (e.g. from file). You can
   also implement your own initializators.
 
 - **Extendable.**
 
   You are not restricted with predefined formatters, sinks, frontends or initializators.
   If needed, you can even change formatter-sink connectivity interface (because it is implemented via
   template specialization).

 - **Header only.**

   Again, you are not paying for things you don't use. Don't need syslog sink? Don't include and compile it!
   But as the result your project compiling time may grow up significantly.

 - **Attribute-based.**
 
   Every log event contains multiple attributes. For example, typical HTTP event from request contains HTTP code,
   URL, message string and something else. In Blackhole log attributes is everything. It can be indexed, it can be
   filtered and it is type-safe.
   By the way, have you heard about [Elasticsearch](http://www.elasticsearch.org/)?
   With the Elasticsearch frontend (not implemented yet) you can index your logs on the fly and store it there with
   great support of manipulating, analyzing and monitoring.
   You are also get the Elasticsearch ELK Stack out of the box.

 - **Robust and tested.**
 
   This project is developed using TDD and CI, so any found bugs is fixed without breaking existed functional.


## Platforms and compilers
 - **Linux (GCC 4.4+)**
 - **Mac OS X (clang 3.3+)**


## Dependencies
 - Boost (tested on 1.40, 1.46 and 1.53)
  

## Formatters

Formatters are entities that map log event to string. Currently there are three built-in formatters:

 - **String.**

   Maps log event to plain string by specified pattern with attributes support.
   
   - **Pattern**: Generic log message with optional placeholders. Every log event contains named attributes. These attributes are substituted in its named placeholder resulting in formatted string. For example: `[%(timestamp)s] [%(level)s] %(tid)s: %(message)s [%(...L)s]` after substituting attribute values transforms into `[2014-02-19 11:57:53] [INFO] 0x7fff7134310: this is test message ['id': 42]`. Placeholder `%(...L)s` means substitute **all** attributes with specified scope.
     - Exceptions: An exception will be thrown if no attribute with specific name found.
   
 - **Json.**
 
   Maps log event to json tree.

 - **Msgpack.**


## Sinks
 - **Files.**
  
   Including standard outputs, rotation by size support, by timestamp is on the way.
  
 - **Syslog.**
 - **Socket (tcp/udp).**
