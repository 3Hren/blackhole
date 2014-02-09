# Blackhole [![Build Status](https://travis-ci.org/3Hren/blackhole.png?branch=master)](https://travis-ci.org/3Hren/blackhole)

Yet another logging library.


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
   URL, message string and something else. In Blackhole log attributes is everything. It can be indexed, in can be
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
 - **String.**

   Formatting by pattern with attributes support.
   
 - **Json.**
 - **Msgpack.**


## Sinks
 - **Files.**
  
   Including standard outputs, rotation by size support, by timestamp is on the way.
  
 - **Syslog.**
 - **Socket (tcp/udp).**
