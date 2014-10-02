# Core
Core - is a part of Blackhole where almost all dirty work is handled.

This part of manual covers two main entities - **log records** and **logger**
instances.
Objects of the first one are responsible for transporting all important
attributes between internal components, like formatters or sinks.
Logger objects - are the main objects in the library as they represent an entry
point for everything in Blackhole.

 * [Records](#log-records)
 * [Logger](#logger)

## Log Records
There is general transport entity in Blackhole, through which all the
information about the event logging are transferred - records. For example
logging message string, verbosity level value or timestamp can be represented
as such information or **attributes**.

Since every meaningful information chunk in Blackhole is represented as
attributes, log records are just represent inself as an attribute set, which
can be transported from one program point to another and which provides
convenient interface to access it.

Usually there is no need to create a log record objects manually, because they
are created by the logger object instance after corresponding method has been
invoked. More about it can be read in [logger]() section.

Moreover it is **unlikely** you will ever handle with log record objects
manually, if you use macro system for logging, which is preferred. But if you
want to extend Blackhole with your own formatter (it is the place where all
log records die) you should be familiar with records.

Log record is default-constructible. It doesn't contain any attributes after
being constructed by default and is considered to be invalid.

~~~ cpp
record_t record;
assert(false == record.valid());
~~~

A record is valid if it contains at least one attribute. Invalid records are
pointless, so they isn't even handled by the logger.
Blackhole's core system can return invalid record in case if initial attribute
set fails primary filtering, for example, by verbosity level.

You can insert attributes in a record object as like as extract them by name,
but currently there is no way to remove them. This is because I don't see such
necessity in this action right now. Therefore removing can be added in the
future to follow CRUD principe.

To insert an attribute in a log record:

~~~ cpp
record_t record;
record.insert(attribute::make("key", 42));
~~~

There is way to extract an attribute by its name and try to immediately cast to
its true type:

~~~ cpp
assert(42 == record.extract<int>("key"));
~~~

If an attribute with such name haven't been found in attribute set an exception
will be thrown. Similarly an exception will be thrown if desired type is
mismatched with real attribute type keeped in the collection. More about
record API and specifications can be read in [reference]() manual.

At last, you can get underlying attribute set's const reference. This is need,
for example, to iterate over **all** attributes in the record.
Class `set_view_t` is quite nontrivial itself, thus I recommend you to
familiarize with it in [attributes]() section of just in [reference]() manual.

Library allows log records to be safely copied and moved.

### Thread Notes
In multithreaded environments, after being constructed a non-empty log record
is considered to be bound to the current thread, because it may refer to some
thread-specific resources.
For example, the record may contain an thread-specific scoped attributes.
For this reason log records must not be passed between different threads.
