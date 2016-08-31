# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]
### Added
- Records are now aware of lightweight process id - LWP (or SPID).

  On Linux an LWP is a process created to facilitate a user-space thread. Each user-thread has a 1x1 mapping to an LWP. An LWP is associated with its own unique positive number, that we store in the record at the construction time. For other platforms there is always 0 stored in the record instead.

## [1.2.1] - Firemaw - 2016-08-29
### Fixed
- Extend optional placeholders grammar.
This change allows to specify fill, align and precision specification for optional placeholders when configuring the pattern formatter. Also the type specifier is now optional, allowing to specify no type, making `libfmt` to select the proper formatting itself. This change should fix a bug, where it’s impossible to specify the default integral value for string-formatted placeholder.

## [1.2.0] - Flamegor - 2016-08-29
### Added
- JSON formatter can now apply custom attributes formatting. This feature can be also configured using "formatting" key in the config.
- String formatter now supports optional placeholders with default value (#150).
### Fixed
- Limit min queue factor value to 2. Otherwise an assertion inside MPSC queue is triggered.
- Repair suddenly broken Google Mocking Library link.

## [1.1.0] - Bleeding Hollow - 2016-08-03
### Added
- Introduce new development handler with eye-candy colored output.

### Changed
- Note, that there are some symbols, that are wrapped into `experimental` namespace. These symbols don't adhere semantic versioning and, well... experimental. Use them with caution and only, where you want to try super-unstable features, which can be changed or even dropped.
- Hide `boost::asio::detail` namespace, which are supposed to be hidden by default, but the ancient GNU linker exports them anyway.

## [1.0.2] - Grove Warden - 2016-07-29
### Fixed
- Bug fix: the library should properly build on GCC 5 (#143).

## [1.0.1] - Ori - 2016-07-18
### Fixed
- Experimental builders will no longer change meaning of AST tokens ([#134]).

## [1.0.0] - The Lich King - 2016-07-13
### Added
- The Blackhole is completely rewritten from the scratch, there is no need to duplicate README here.
- Integration with CodeCov ([#66]).
- Keep a changelog ([#67]).
- Embed libcppformat directly into the Blackhole ([#68]).

### Changed
- Hide rapidjson symbols entirely ([#78]).
- All formatters, sinks and handlers no longer export their symbols and can be constructed only through associated factories as unsized objects.

## [0.5.0] - The Infinite Corruptor - 2015-04-09
### Added
- Inline namespaces ([#42]).
  Allows to link together multiple libraries that are compiled with different Blackhole versions.  
- Variadic placeholder filter policy for string formatter.
- Arch Linux packaging and required fixes ([#45]).

### Changed
- Lightweight process attribute is now available for OS X targets.

## [0.4.1] - Mystic Runesaber - 2015-02-14
### Changed
- Optional printf-like syntax checking option.
  Blackhole's printf-like syntax checking now can be disabled by configuring `BLACKHOLE_CHECK_FORMAT_SYNTAX` macro variable.

### Fixed
- Fixed corrupted move constructor for logger class.

  Add missing copy of an exception handler when moving logger objects, which resulted in bad function call.

- Prevent throwing from pusher destructor ([#40]).

## [0.4.0] - Kargath Bladefist - 2015-01-13
### Added
- Combined logger concept.

  Now all loggers - are just sets of features combined in a single logger object. When specializing combined logger, you should specify additional parameters, by which filtering should occur when opening a record and before they will be merged with internal attributes set. For example you may specify no additional parameters and you get the simplest logger case. Another case - when you want to perform early filtering by verbosity level you can just specify its type with combination of base logger and the filtering function will require that level type.

- Filtering policies, which provide almost costless filtering.

  A composite logger accepts additional policy parameter instead of mixed type and its argument pack. All methods, that opens a record now accepts variadic pack which is forwarded to that policy.

  Additional attributes insertion into an internal set can be implemented using population trait, which is provided as first template parameter in a filtering policy.

- Threading policy support for logger ([#20]).

  A composite logger can be configured with desired threading policy. That means if the logger doesn't require internal synchronization, it won't do it (if the corresponding policy is provided).

- Internal attributes attachment as a feature.

  Every project should define which of internal attributes (PID, TID, etc.) should be attached to every logging event.

- Clang support on non OS X targets.  
- Code coverage analyze using `lcov` util. No more words, just type `make coverage` and you will see ([#38]).
- Multithreaded scalability benchmarks ([#36]).
- Huge number of benchmarks (approximately 300), that test every possible combination of cases. These are generated using preprocessor magic.

### Changed
- Changed attribute set underlying representation ([#18]).

  Now attribute set is internally represented as vector of pairs. This change allows to keep attributes in single memory block, which is good for caching and performance. On the other hand attribute lookup time by user attributes may increase, because they are scanned using linear search. Also I don't see any need for handling duplicates, so now there **can** be duplicates (which actually can be eaten if needed). There are two attributes sets in every record: internal and external. Internal attributes is setting by logger itself: message, severity, timestamp, pid, tid or lwp. All attributes that is configured via scoped sets or wrappers - are external. This distribution allows me in the future to map each internal attribute with its unique number to perform even more faster table lookup.

  This change increases performance of almost every case by approximately 20-30%.

- It's possible to forward all required arguments to final logger's constructor, while creating it through a repository.
- Logger's move constructor, move assignment operator and swap function no longer contain `noexcept` specifications, because given methods actually can throw. This change **breaks API**.
- Disabled tracking feature, because it shouldn't be in a logger itself, but somewhere else. This change **breaks API**.
- Verbose logger concept review.

  No longer possible to open verbose logger's record without specifying severity level. This is required due to fact, that there is a primary filtering now, which strongly required severity level provided. Moreover there can be user specific filtering function that be cannot ignore.

- Proper verbose logger initialization with a given severity level.

  No longer possible to initialize verbose logger with garbage level value, because user specific severity enumeration may start with any value. Instead of this, the User if forced to provide severity threshold explicitly. Verbosity threshold should always be specified, when setting verbosity to a logger, even with filtering function. This change **breaks API**.

- Redesign invalid record concept.

  When opening a record there is a non-zero chance, that you will receive an empty record with no attributes attached. Previously it was considered as invalid record, because there was at least pid and tid attributes attached. But now seems to be there is no need to attach internal attributes before filtering. However now it is considered as valid, just without attributes. This change **breaks API**.

- Filtering function parameters review ([#33]).

  Logging filtering function now accepts lightweight combined attribute set view. Technically it allows to perform faster filtering, because there is no need to construct combined set - it will be constructed only if the filtering succeed. This change **breaks API**.

- Deprecated file `blackhole/log.hpp` file is no longer part of public API, just as planned. This change **breaks API**.
- Internal and external attribute sets initial capacity can be configured using macro variables.
- Documentation has been slightly changed and restyled.
- No longer fetch gtest/gmock as submodules.
  Now these helpful frameworks are obtained via cmake download feature.
- No longer fetch benchmarking framework as submodule. Instead of that, cmake download feature is used.
- Got rid of several unnecessary file includes, which saves some compile time.
- Avoid unnecessary string copying or moving when comparing attributes.
- Explicitly move the value when opening a record with a single attribute.

### Removed
- Drop global logger attribute set.

  After half year of user experience I've realized that nobody uses logging-attached attributes. The main reason of it - because of Wrappers. You can create logger wrapper, attach some attributes and work with it without copying underlying logger resources like file descriptors or sockets. As a result - you can have only one logger for application, but with multiple wrappers.

  This is a big advantage, but this is a **breaking change**.

- Dropped logger concept classes, because it was inconvenient and buggy.

### Fixed
- Fix string literal warnings on some ancient compilers.
- Workaround GCC 4.4 pragmas.
- Checking frontend set emptiness is now always under the lock.
- Fix possible conditional jump in tests.

  To check results from functions, which can go the wrong way - is a good idea! Sometimes `strftime` cannot fill provided buffer completely, because of initial format, for example. The best way for testing environment is to abort program execution and extend the buffer needed to match all cases.

- Fix conditional jump in sticky stream ([#32]).
- Fix comparing signed with signed and vise versa.
- Fix broken resolver test.

## [0.3.2] - Murmur - 2014-11-17
### Fixed
- Fix improper type mapping by name.

  When registering Cartesian product of all possible formatters and sinks with the factory, they are internally mapped into typeid structure to be able to extract proper type factory by its name or by user-defined mapper trait.

  Accidentally I've just forgot to write proper code for default case (mapping by entity name). Now the default type mapping checks whether type name is equal with the given one.

## [0.3.1] - Murky - 2014-10-29
### Fixed
- Fixed improper attributes routing.
  Accidentally all wrapper-attached attributes were routed to the internal section, which contain only internal attributes, like timestamp, severity or message.

  Internal attributes are not shown in variadic placeholders, but all user-defined attributes should.

## [0.3.0] - Epicus Maximus - 2014-10-28
### Added
- String formatter now has **optional** placeholder, which can be fully ignored by the library, when it's not present in a record.
- String formatter now can be configured with prefix, suffix and separator while using optional or variadic placeholders.
- String formatter has learned some convenient error-handling magic and can tell the User where and what type of error has happened.
- It's now possible to represent time value structure in local timezone.
- Any logger-compatible type (based on `logger_base_t`) can be created through a repository.
- Use compiler extension (cross-platform nonetheless) to check log message format correctness in compile time.
- The library now widely uses deprecated attribute feature, which allows to reduce from version to version migration pain.
- Added logger trait concept.
- A lot of documentation added.

### Changed
- Log record has been completely refactored (and documented).
- Completely dropped all scope-specific stuff. Actually, there are three scopes left, but its management is fully incapsulated into the library.
- Completely dropped `blackhole::log::*` namespace, because it's already the Logging Library.
- Logger wrapper can now provide const reference to the underlying logger.
- Dropped 'in situ' substitution mechanism for keyword attributes, because it is creepy and useless.
- Base config objects now return const references instead of copying.
- Allow to use `_` symbol in placeholder name.
- Attribute value holders are now comparable.
- Frontend factory now has more convenient interface.
- Using specialized exception instead of more generic while parsing configuration object.
- More forward declarations, which may be useful with compile time reducing.
- Pack feeder now has overload that accepts string literal, which allows to avoid unnecessary transformations.
- Multiple attribute sets is aggregated into single `view` class, so attribute lookup is performed lazily when it is needed by someone.
- String formatter now internally  uses ADT instead of packed functions.
- Accelerated variadic placeholder handling in string formatter by approximately 30% via avoiding unnecessary string transformations.

### Fixed
- Process id attribute is back and its extraction is much cheaper.
- Message attribute should no longer hangs out with external attributes.
- Fix typo in GCC 4.6 detection macro.
- Fix compatibility with GCC 4.4, which emitted false-positive warning.
- Blackhole should no longer propagate exception raised from `boost::format`, while formatting message string. Instead of this an exception reason will be printed as message.

## [0.2.3] - Weaver - 2014-09-29
### Fixed
- Fix debian control especially for precise.

## [0.2.2] - Captain Cookie - 2014-09-24
### Added
- Move watcher for file sink, which can automatically reopen files after they were moved.

## [0.2.1] - Edwin VanCleef -2014-09-12
### Fixed
- Verbose logger should now properly copy level value when moving.

## [0.2.0] - Hakkari the Blood God - 2014-08-18
### Added
- [Elasticsearch](http://www.elasticsearch.org) sink - allows to send logging events directly to that storage.
- Scoped attributes holder - automatically adds specified attributes to the logger while in its scope.
- Logger adaptor - keeps some attributes until lives.
- Tracing framework - closely integrates with the logging system.
- Configuration parser can properly handle arrays.
- Logger frontends are now thread-aware.
- Streaming sink now allows to use custom `std::stream`.
- Logger object's internal state is now thread-safe.
- Default severity and its mapping function.
- Example of Elasticsearch sink usage.

### Changed
- License is now MIT.
- Relax local attributes transition to the record.
- Opening verbose logger's level type.
- Added macro variable to determine if the platform has c++11 random library.
- Start using implementation files (ipp), which allows to build library in the future.
- Verbose logger now can keep bound verbosity level and filter by it.
- No longer use `boost::filesystem` deprecated API.
- Let the compiler deduce `swap` function it needs to use.
- Migrated from `boost::any` to `boost::variant` configuration.
- More forwards added.
- Disable trace collecting by default.
- Use lightweight process id (LWP) on Linux instead of thread id.
- Logger can now provide its tracking state outside.
- Moving `BLACKHOLE_HEADER_ONLY` declaration to the config file.
- Disable tests and examples by default.
- Logger wrapper's constructor overload now accepts other const wrapper's reference instead of non-const one.
- Changed namespace of `DECLARE_EVENT_KEYWORD`.
- Using new benchmarking framework for regression tests.
- Default mapping from default severity to syslog one.
- Default warning severity mapping to string has been slightly changed.
- Change priority of attribute sets while merging.
- Scoped attributes constructor now has more strictly wrapper concept check.
- Added `DECLARE_LOCAL_KEYWORD` macro.
- Testing frameworks are now included as submodules.
- Continuous integration is used more widely, tests and examples should now be built separately.
- Benchmark added to measure full logger event lifecycle.

### Fixed
- Long and unsigned long values can now be used as attributes.
- Misleading error message when failed to instantiate formatter.
- Fix undefined behavior in syslog sink.
- Fix some conditional memory jumps.
- TCP write handler will now block until the message is completely sent.
- Fix deadlock while invoking move assigning in logger wrapper.
- Forgotten configuration include added.
- Fix mapping of debug severity to string.

## [0.1.0] - Hungry Torso - 2014-04-30
### Changed

- From this point a strong version control begins.

[Unreleased]: (https://github.com/3Hren/blackhole/tree/HEAD)
[0.1.0]: (https://github.com/3Hren/blackhole/tree/v0.1.0)
[0.2.0]: (https://github.com/3Hren/blackhole/tree/v0.2.0)
[0.2.1]: (https://github.com/3Hren/blackhole/tree/v0.2.1)
[0.2.2]: (https://github.com/3Hren/blackhole/tree/v0.2.2)
[0.2.3]: (https://github.com/3Hren/blackhole/tree/v0.2.3)
[0.3.0]: (https://github.com/3Hren/blackhole/tree/v0.3.0)
[0.3.1]: (https://github.com/3Hren/blackhole/tree/v0.3.1)
[0.3.2]: (https://github.com/3Hren/blackhole/tree/v0.3.2)
[0.4.0]: (https://github.com/3Hren/blackhole/tree/v0.4.0)
[0.4.1]: (https://github.com/3Hren/blackhole/tree/v0.4.1)
[0.5.0]: (https://github.com/3Hren/blackhole/tree/v0.5.0)

[#18]: (https://github.com/3Hren/blackhole/issues/18)
[#20]: (https://github.com/3Hren/blackhole/issues/20)
[#32]: (https://github.com/3Hren/blackhole/issues/32)
[#33]: (https://github.com/3Hren/blackhole/issues/33)
[#36]: (https://github.com/3Hren/blackhole/issues/36)
[#42]: (https://github.com/3Hren/blackhole/issues/42)
[#66]: (https://github.com/3Hren/blackhole/issues/66)
[#67]: (https://github.com/3Hren/blackhole/issues/67)
[#68]: (https://github.com/3Hren/blackhole/issues/68)
[#78]: (https://github.com/3Hren/blackhole/issues/78)
[#134]: (https://github.com/3Hren/blackhole/issues/134)

[#38]: (https://github.com/3Hren/blackhole/pull/38)
[#40]: (https://github.com/3Hren/blackhole/pull/40)
[#45]: (https://github.com/3Hren/blackhole/pull/45)
[#143]: (https://github.com/3Hren/blackhole/pull/143)
