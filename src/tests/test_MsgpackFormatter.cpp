#include <blackhole/formatter/msgpack.hpp>
#include <blackhole/keyword/message.hpp>
#include <blackhole/keyword/timestamp.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(msgpack_t, Class) {
    formatter::msgpack_t fmt;
    UNUSED(fmt);
}

TEST(msgpack_t, FormatSingleAttribute) {
    record_t record;
    record.insert(keyword::message() = "le message");

    formatter::msgpack_t fmt;
    std::string actual = fmt.format(record);

    msgpack::unpacked unpacked;
    msgpack::unpack(&unpacked, actual.data(), actual.size());
    std::map<std::string, msgpack::object> root;
    unpacked.get().convert<std::map<std::string, msgpack::object>>(&root);

    EXPECT_EQ("le message", root["message"].as<std::string>());
}

TEST(msgpack_t, FormatMultipleAttributes) {
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert(keyword::timestamp() = timeval{ 100500, 0 });

    formatter::msgpack_t fmt;
    std::string actual = fmt.format(record);

    msgpack::unpacked unpacked;
    msgpack::unpack(&unpacked, actual.data(), actual.size());
    std::map<std::string, msgpack::object> root;
    unpacked.get().convert<std::map<std::string, msgpack::object>>(&root);

    EXPECT_EQ("le message", root["message"].as<std::string>());
    EXPECT_EQ(100500, root["timestamp"].as<long>());
}
