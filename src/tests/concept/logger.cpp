#include "../global.hpp"

#include <blackhole/detail/concept.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/logger/wrapper.hpp>

using namespace blackhole;

TEST(LoggerConcept, AcceptsBaseLogger) {
    static_assert(
        (concept::logger<logger_base_t>::value),
        "`logger_base_t` should pass concept check"
    );
}

TEST(LoggerConcept, AcceptsVerboseLogger) {
    static_assert(
        (concept::logger<verbose_logger_t<testing::level>>::value),
        "`verbose_logger_t` should pass concept check"
    );
}

TEST(LoggerConcept, AcceptsWrapper) {
    static_assert(
        (concept::logger<wrapper_t<logger_base_t>>::value),
        "`wrapper_t` should pass concept check"
    );
}

TEST(LoggerConcept, DeclinesString) {
    static_assert(
        (!concept::logger<std::string>::value),
        "`std::string` shouldn't pass concept check"
    );
}

namespace testing {

namespace mock {

namespace invalid {

class only_open_t {
public:
    record_t open_record() const;
};

class only_push_t {
public:
    void push(record_t&&) const;
};

class openx1_and_push_t {
public:
    record_t open_record() const;
    void push(record_t&&) const;
};

class openx2_and_push_t {
public:
    record_t open_record() const;
    record_t open_record(attribute::pair_t) const;
    void push(record_t&&) const;
};

} // namespace invalid

class valid_t {
public:
    record_t open_record() const;
    record_t open_record(attribute::pair_t) const;
    record_t open_record(attribute::set_t) const;
    void push(record_t&&) const;
};

} // namespace mock

} // namespace testing

TEST(LoggerConcept, FailOnClassOnlyOpenRecord) {
    static_assert(
        (!concept::logger<testing::mock::invalid::only_open_t>::value),
        "'only_open_t' shouldn't pass concept check"
    );
}

TEST(LoggerConcept, FailOnClassOnlyPushRecord) {
    static_assert(
        (!concept::logger<testing::mock::invalid::only_push_t>::value),
        "'only_push_t' shouldn't pass concept check"
    );
}

TEST(LoggerConcept, FailOnClassPushRecordAndOpenRecordWithNoAttributesProvided) {
    static_assert(
        (!concept::logger<testing::mock::invalid::openx1_and_push_t>::value),
        "'openx1_and_push_t' shouldn't pass concept check"
    );
}

TEST(LoggerConcept, FailOnClassPushRecordAndOpenRecordWithNoAttributesProvidedAndWithPair) {
    static_assert(
        (!concept::logger<testing::mock::invalid::openx2_and_push_t>::value),
        "'openx2_and_push_t' shouldn't pass concept check"
    );
}

TEST(LoggerConcept, PassValidMock) {
    static_assert(
        (concept::logger<testing::mock::valid_t>::value),
        "'valid_t' should pass concept check"
    );
}
