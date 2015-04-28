Summary:	Blackhole
Name:		libblackhole5
Version:	0.5.0
Release:	1%{?dist}

License:	MIT
Group:		System Environment/Libraries
URL:		http://github.com/3Hren/blackhole
Source0:	%{name}-%{version}.tar.bz2
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%if %{defined rhel} && 0%{?rhel} < 6
BuildRequires:	gcc44 gcc44-c++
%define boost_ver 141
%else
%define boost_ver %{nil}
%endif
BuildRequires:	boost%{boost_ver}-devel, boost%{boost_ver}-iostreams, boost%{boost_ver}-system, boost%{boost_ver}-thread
BuildRequires:	cmake


%description
Blackhole is a common logging library.


%package devel
Summary: Development files for %{name}
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}


%description devel
This package contains libraries, header files and developer documentation
needed for developing software which uses the Blackhole logging library.

%prep
%setup -q

%build
%if %{defined rhel} && 0%{?rhel} < 6
export CC=gcc44
export CXX=g++44
CXXFLAGS="-pthread -I/usr/include/boost141" LDFLAGS="-L/usr/lib64/boost141" %{cmake} -DBoost_LIB_DIR=/usr/lib64/boost141 -DBoost_INCLUDE_DIR=/usr/include/boost141 -DBoost_LIBRARYDIR=/usr/lib64/boost141 -DBOOST_LIBRARYDIR=/usr/lib64/boost141 -DCMAKE_CXX_COMPILER=g++44 -DCMAKE_C_COMPILER=gcc44 -DENABLE_EXAMPLES=OFF .
%else
%{cmake} -DENABLE_EXAMPLES=OFF .
%endif

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)

%files devel
%defattr(-,root,root,-)
%{_includedir}/blackhole/*

%changelog
* Thu Apr 09 2015 Evgeny Safronov <division494@gmail.com> - 0.5.0-1
- Release version.
- Feature: variadic placeholder filter policy for string formatter.

* Fri Mar 13 2015 Evgeny Safronov <division494@gmail.com> - 0.5.0-0rc1
- Feature: inline namespaces - allows to link together multiple libraries that are compiled with
  different Blackhole versions.
- Feature: lightweight process attribute for OS X targets.

* Sat Feb 14 2015 Evgeny Safronov <division494@gmail.com> - 0.4.1-1
- Bug fix: fixed corrupted move constructor for logger class.
- Other: optional syntax checking option.

* Tue Jan 13 2015 Evgeny Safronov <division494@gmail.com> - 0.4.0-1
- Release version.

* Mon Dec 29 2014 Evgeny Safronov <division494@gmail.com> - 0.4.0-0rc3
- Bug fix: allow to override LWP attribute presence.
- API: one popular header file (blackhole/log.hpp) is returned back.
- Other: added a little more documentation about logger wrappers.
- Other: mangle internal macro variables.

* Thu Dec 22 2014 Evgeny Safronov <division494@gmail.com> - 0.4.0-0rc2
- Bug fix: fixed filtering using strong enums.
- Other: syntax checking can be disabled now.

* Thu Dec 11 2014 Evgeny Safronov <division494@gmail.com> - 0.4.0-0rc1
- Feature: combined logger concept.
- Feature: filtering policies, which provide almost costless filtering.
- Feature: threading policy support for logger.
- Feature: internal attributes attachment as a feature.
- Bug fix: fix string literal warnings on some ancient compilers.
- Bug fix: workaround GCC 4.4 pragmas.
- Bug fix: checking frontend set emptiness is now always under the lock.
- API: it's possible to forward all required arguments to final logger's constructor, while
  creating it through a repository.
- API: logger's move constructor, move assignment operator and swap function no longer contain
  `noexcept` specifications, because given methods actually can throw.
- API: disabled tracking feature, because it shouldn't be in a logger itself, but somewhere else.
- API: verbose logger concept review.
- API: proper verbose logger initialization with a given severity level.
- API: drop global logger attribute set.
- API: dropped logger concept classes, because it was inconvenient and buggy.
- API: redesign invalid record concept.
- API: filtering function parameters review.
- API: deprecated file `blackhole/log.hpp` file is no longer part of public API, just as planned.
- Other: internal and external attribute sets initial capacity can be configured using macro
  variables.
- Other: clang support on non OS X targets.
- Other: documentation has been slightly changed and restyled.
- Other: code coverage analyze using `lcov` util.
- Other: no longer fetch gtest/gmock as submodules.
- Other: no longer fetch benchmarking framework as submodule.
- Performance: changed attribute set underlying representation.
- Performance: got rid of several unnecessary file includes, which saves some compile time.
- Performance: avoid unnecessary string copying or moving when comparing attributes.
- Performance: explicitly move the value when opening a record with a single attribute.
- Testing: added multithreaded scalability benchmarks.
- Testing: added huge number of benchmarks (approximately 300), that test every possible
  combination of cases.
- Testing: fixed possible conditional jump.
- Testing: fixed comparing signed with signed and vise versa.
- Testing: fixed broken resolver test.

* Mon Nov 17 2014 Evgeny Safronov <division494@gmail.com> - 0.3.2-1
- Bug fix: fixed improper type mapping by name.

* Wed Oct 29 2014 Evgeny Safronov <division494@gmail.com> - 0.3.1-1
- Bug fix: fixed improper attributes routing.

* Tue Oct 28 2014 Evgeny Safronov <division494@gmail.com> - 0.3.0-1
- Release version.

* Mon Oct 20 2014 Evgeny Safronov <division494@gmail.com> - 0.3.0-0rc4
- Bug fix: process id attribute is back.
- Other: allow to use '_' in placeholder name.

* Wed Oct 15 2014 Evgeny Safronov <division494@gmail.com> - 0.3.0-0rc3
- Bug fix: fixed broken include in default severity header.
- Other: fixed compatibility with GCC 4.4, which emitted false warning.

* Mon Oct 13 2014 Evgeny Safronov <division494@gmail.com> - 0.3.0-0rc2
- Bug fix: message attribute should no longer hangs out with external attributes.
- Bug fix: fixed forgotten mapper attaching to formatters.

* Mon Oct 13 2014 Evgeny Safronov <division494@gmail.com> - 0.3.0-0rc1
- Feature: the library now widely uses deprecated attribute feature.
- Feature: string formatter now has optional placeholder.
- Feature: string formatter now can be configured with prefix, suffix and
  separator while using variadic placeholders.
- Feature: string formatter has learned some convenient error-handling magic
  and can tell the User where and what type of error has happened.
- Feature: it's now possible to represent time value structure in local timezone.
- Feature: any logger-compatible type (based on `logger_base_t`) can be created
  through a repository.
- Feature: use compiler extension (cross-platform nonetheless) to check log
  message format correctness in compile time.
- Feature: added logger trait concept.
- Bug fix: fixed typo in gcc 4.6 detection macro.
- API: log record has been completely refactored.
- API: completely dropped all scope-specific stuff.
- API: completely dropped blackhole::log::* namespace.
- API: logger wrapper can now provide const reference to the underlying logger.
- API: dropped 'in situ' substitution mechanism for keyword attributes. This is
  breaking change.
- API: base config objects now return const references instead of copying.
- Performance: pack feeder now has overload that accepts string literal, which
  allows to avoid unnecessary transformations.
- Performance: multiple attribute sets is aggregated into single `view` class,
  so attribute lookup is performed lazily when it is needed by someone.
- Performance: string formatter now internally  uses ADT instead of packed
  functions.
- Performance: accelerated variadic placeholder handling in string formatter
  by approximately 30% via avoiding unnecessary string transformations.
- Other: attribute value holders are now comparable.
- Other: frontend factory now has more convenient interface.
- Other: Blackhole should no longer propagate exception raised from boost::format,
  while formatting message string. Instead of this an exception reason will
  be printed as message.
- Other: using specialized exception instead of more generic while parsing
  configuration object.
- Other: more forward declarations.
- Documentation: more documentation added.
- Testing: more unit tests added as like as benchmarks.

* Mon Sep 29 2014 Evgeny Safronov <division494@gmail.com> - 0.2.3-1
- Misc: fixed debianization for precise, which allows to install package with libboost 1.46 as like as 1.48.

* Wed Sep 24 2014 Evgeny Safronov <division494@gmail.com> - 0.2.2-1
- Feature: added file sink which can automatically reopen files after they are moved.

* Fri Sep 12 2014 Evgeny Safronov <division494@gmail.com> - 0.2.1-1
- Bug fix: verbose logger should now properly copy level value when moving.

* Mon Aug 26 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-2
- Bug fix: fixed dependencies in both debian/control and spec.

* Mon Aug 18 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-1
- Release version.
- Bug fix: replaced accidentally left writer lock by the reader one.
- Bug fix: logger object now get locked while swapping.

* Fri Aug 08 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-0rc10
- Compatibility: fixed compatibility with GCC 4.4.

* Fri Aug 08 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-0rc9
- Bug fix: fixed mapping of debug severity to string.
- Other: change priority of attribute sets while merging.
- Other: scoped attributes constructor now has more strictly wrapper concept check.
- Other: added DECLARE_LOCAL_KEYWORD macro.
- Testing: added datetime generator, string formatter and other benchmarks.
- Testing: added tests to check priority of categorized attribute sets.

* Wed Aug 06 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-0rc8
- Other: default mapping from default severity to syslog one.
- Other: default warning severity mapping to string has been slightly changed.

* Wed Aug 06 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-0rc7
- Feature: logger wrapper now provides underlying logger type.
- Bug fix: forgotten configuration include added.
- Other: logger wrapper's constructor overload now accepts other const
  wrapper's reference instead of non-const one.
- Other: changed namespace of DECLARE_EVENT_KEYWORD.
- Other: using new benchmarking framework for regression tests.
- Testing: more tests for wrapper.

* Mon Aug 04 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-0rc6
- Bug fix: fixed linker error.

* Mon Aug 04 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-0rc5
- Bug fix: fixed deadlock while invoking move assigning in logger wrapper.

* Mon Aug 04 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-0rc4
- Feature: added default severity and its mapping function.

* Sun Aug 03 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-0rc3
- Feature: logger object's internal state is now thread-safe.
- Other: moving `BLACKHOLE_HEADER_ONLY` declaration to the config file.
- Other: disable tests and examples by default.

* Fri Aug 01 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-0rc2
- Feature: logger frontends are now thread-aware.
- Feature: streaming sink now allows to use custom `std::ostream`.
- Bug fix: tcp write handler will now block until the message is completely sent.
- Other: disable trace collecting by default.
- Other: use lightweight process id (LWP) on Linux instead of thread id.
- Other: logger can now provide its tracking state outside.
- Testing: open access to `backend` variables for mocking purpose.

* Tue Jul 29 2014 Evgeny Safronov <division494@gmail.com> - 0.2.0-0rc1
- Much-much more changes are pending. This field will be filled later.
- Feature: Elasticsearch sink - allows to send logging events directly to that storage.
- Feature: scoped attributes holder - automatically adds specified attributes to the logger while in its scope.
- Feature: logger adaptor - keeps some attributes until lives.
- Feature: tracing framework - closely integrates with the logging system.
- Feature: configuration parser can properly handle arrays.
- Bug fix: long and unsigned long values can now be used as attributes.
- Bug fix: fixed misleading error message when failed to instantiate formatter.
- Bug fix: fixed undefined behaviour in syslog sink.
- Bug fix: some conditional memory jumps fixed.
- Other: changed license to MIT.
- Other: relax local attributes transition to the record.
- Other: opening verbose logger's level type.
- Other: added macro variable to determine if the platform has c++11 random library.
- Other: start using implementation files (ipp), which allows to build library in the future.
- Other: verbose logger now can keep bound verbosity level and filter by it.
- Other: no longer use `boost::filesystem` deprecated API.
- Other: let the compiler deduce `swap` function it needs to use.
- Other: migrated from `boost::any` to `boost::variant` configuration.
- Other: more forwards added.
- Example: added example using Elasticsearch sink.
- Testing: testing frameworks are now included as submodules.
- Testing: continious integration is used more widely, tests and examples should now be built separately.
- Testing: benchmark added to measure full logger event lifecycle.

* Fri Apr 25 2014 Evgeny Safronov <division494@gmail.com> - 0.1.0-3
- Bug fix: added forgotten include.

* Mon Mar 31 2014 Evgeny Safronov <division494@gmail.com> - 0.1.0-1
- Initial release.
