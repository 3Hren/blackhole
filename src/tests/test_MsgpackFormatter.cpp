#include "Mocks.hpp"

using namespace blackhole;

TEST(msgpack_t, Class) {
    formatter::msgpack_t fmt;
    UNUSED(fmt);
}

TEST(msgpack_t, FormatSingleAttribute) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message"
    };

    formatter::msgpack_t fmt;
    std::string actual = fmt.format(record);

    msgpack::unpacked unpacked;
    msgpack::unpack(&unpacked, actual.data(), actual.size());
    std::map<std::string, msgpack::object> root;
    unpacked.get().convert<std::map<std::string, msgpack::object>>(&root);

    EXPECT_EQ("le message", root["message"].as<std::string>());
}

TEST(msgpack_t, FormatMultipleAttributes) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        keyword::timestamp() = 100500
    };

    formatter::msgpack_t fmt;
    std::string actual = fmt.format(record);

    msgpack::unpacked unpacked;
    msgpack::unpack(&unpacked, actual.data(), actual.size());
    std::map<std::string, msgpack::object> root;
    unpacked.get().convert<std::map<std::string, msgpack::object>>(&root);

    EXPECT_EQ("le message", root["message"].as<std::string>());
    EXPECT_EQ(100500, root["timestamp"].as<std::time_t>());
}

//!@todo: Nested attribute objects. It is needed for logstash.
