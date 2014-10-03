# Core
Core - is a part of Blackhole where almost all dirty work is handled.

This part of manual covers two main entities - **log records** and **logger** instances.

Objects of the first one are responsible for transporting all important attributes between internal components, like formatters or sinks. Logger objects - are the main objects in the library as they represent an entry point for everything in Blackhole.

 * [Records](#log-records)
 * [Logger](#logger)

## Log Records
There is general transport entity in Blackhole, through which all the information about the event logging are transferred - records. For example logging message string, verbosity level value or timestamp can be represented as such information or **attributes**.

Since every meaningful information chunk in Blackhole is represented as attributes, log records are just represent inself as an attribute set, which can be transported from one program point to another and which provides convenient interface to access it.

Usually there is no need to create a log record objects manually, because they are created by the logger object instance after corresponding method has been invoked. More about it can be read in [logger]() section.

Moreover it is **unlikely** you will ever handle with log record objects manually, if you use macro system for logging, which is preferred. But if you want to extend Blackhole with your own formatter (it is the place where all log records die) you should be familiar with records.

Log record is default-constructible. It doesn't contain any attributes after being constructed by default and is considered to be invalid.

~~~ cpp
blackhole::record_t record;
assert(false == record.valid());
~~~

A record is valid if it contains at least one attribute. Invalid records are pointless, so they isn't even handled by the logger. Blackhole's core system can return invalid record in case if initial attribute set fails primary filtering, for example, by verbosity level.

You can insert attributes in a record object as like as extract them by name, but currently there is no way to remove them. This is because I don't see such necessity in this action right now. Therefore removing can be added in the future to follow CRUD principe.

To insert an attribute in a log record:

~~~ cpp
blackhole::record_t record;
record.insert(blackhole::attribute::make("key", 42));
~~~

There is way to extract an attribute by its name and try to immediately cast to its true type:

~~~ cpp
assert(42 == record.extract<int>("key"));
~~~

If an attribute with such name haven't been found in attribute set an exception will be thrown. Similarly an exception will be thrown if desired type is mismatched with real attribute type keeped in the collection. More about record API and specifications can be read in [reference]() manual.

At last, you can get underlying attribute set's const reference. This is need, for example, to iterate over **all** attributes in the record. Class `set_view_t` is quite nontrivial itself, thus I recommend you to familiarize with it in [attributes]() section of just in [reference]() manual.

Library allows log records to be safely copied and moved.

### Thread Notes
In multithreaded environments, after being constructed a non-empty log record is considered to be bound to the current thread, because it may refer to some thread-specific resources. For example, the record may contain an thread-specific scoped attributes. For this reason log records must not be passed between different threads.

## Logger
It would be very strange if a logging library doesn't contain objects named "logger". Exactly logger instances bind user and the library. Exactly through it there is a record creation implemented and its sending to the internal logging system. Exactly the logger determines which records will be sent out and which are not. Finally exactly the logger determines final destinations type and count and how exactly records will be sent.

This section describes what loggers are, why they are needed and how to configure them.

Blackhole provides base logger class, which incapsulates in itself almost all logging work - `logger_base_t`.

This class instances may be created both manually and through special [repository]().

Logger is default-constructible, so there shouldn't be any problems to create it:

~~~ cpp
blackhole::logger_base_t log;
~~~

Newly created logger is empty and completely unusable. Any attempt to open record results in invalid record, which is unusable too. To fill the logger with life, it should be configured with [frontends](), which represents a simple facade over [formatter]()-[sink]() pair. More about them you can read in corresponding sections. For now we assume that we have got valid frontend object from the Void and we want to add it into the logger:

~~~ cpp
auto frontend = // Get frontend object from elsewhere. It must be `std::unique_ptr`.
log.add_frontend(std::move(frontend));
~~~

Usually configured logger have no more than a single frontend. Thus there are situations where you may want to send the same log record into several destinations. For example we want to write just simple string in some file simultaneously with the sending formatted json via TCP socket. In this cases Blackhole makes no constrants for frontend count, which are handled by the logger.

Just as the logger have at least a single frontend inside, it can open records. Usually opening a record is implemented via `open_record` method invokation, which optionally accepts one or multiple attributes, which will be attached to the newly created record if it passes filtering.

~~~ cpp
// Just open record.
auto record1 = log.open_record();

// Open record with specified attribute attached.
auto record2 = log.open_record(blackhole::attribute::make("id", 42));

// Open record with multiple attributes attached.
blackhole::attribute::set_t attributes;
attributes["id"] = 42;
attributes["host"] = "localhost";

auto record3 = log.open_record(std::move(attributes));
~~~

You can try to feed just newly created record to the logger by invoking `push` method, which accepts `record_t` rvalue-reference.

~~~ cpp
log.push(std::move(record));
~~~

This is the place where `record_t` lifecycle ends, because we have just moved it.

Actually in real world it is **very unlikely** that you have to create log records manually and feed them into the logger. Instead of this about 99% use-cases is covered by using special macro.

~~~ cpp
BH_LOG(log, level::debug, "message");
~~~

There are a lot of variants using this macro, which you can see in corresponding documentation [section]().

Logger objects are not allowed to be copied, since it contans non-copyable data structures. Therefore it is allowed to move it, which makes possible getting logger, for example, from function.

~~~ cpp
auto log = make_logger();
~~~

### Switching Off
There is a way to switch off a logger. Disabled logger won't generate any records, but it continue to accept records created externally or before switching it off.

~~~ cpp
log.enabled(false);
auto record = log.open_record();
assert(false == record.valid());

log.enabled();
~~~

Also the logger have own attribute set internally. It is empty by default, but you can add some attribute to the logger, and from this point it will present in all records created by this logger.

~~~ cpp
log.add_attribute(blackhole::attribute::make("name", "internal.console"));
auto record = log.open_record();

assert("internal.console" == record.extract<std::string>("name"));
~~~

Currently there is no way to remove attribute, it's API bug.

### Filtering
Primary filtering is handled by the filter function object, which can be provided with the `set_filter` method. More about creating filters appears in [filtering]() section. Here it will suffice to say that the filter accepts a attribute set and returns a boolean value that tells whether a log record with these attribute values passed filtering or not.

The primary filter is applied to every log record made throughout the application, so it can be used to wipe out excessive log records quickly.

By default there is default filter which accepts all records.

### Exception Handling
A logger objects provide a way to set up exception handling. If an exception takes place during processing in one of the added frontends, the core will invoke an exception handler if one was installed with the `set_exception_handler` method. An exception handler is a nullary function object that is invoked from within a catch clause. More about this you can see in [advanced]() usage documentation.

### Thread Notes
The logger itself is completely thread-safe by default.

### Plans

 * RCU support instead of read/write locks.
 * Locked sink balancing.
 * Fallback logger.
 * Thread policies.
