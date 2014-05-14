#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>

namespace elasticsearch {

namespace defaults {

typedef boost::asio::ip::tcp::endpoint endpoint_type;

const endpoint_type endpoint(boost::asio::ip::address_v4(), 9200);

} // namespace defaults

struct settings_t {
    typedef boost::asio::ip::tcp::endpoint endpoint_type;

    std::string index;
    std::vector<endpoint_type> endpoints;

    struct sniffer_t {
        struct {
            bool start;
            bool error;
        } when;

        long timeout;
        long invertal;
    } sniffer;

    int connections;
    int retries;
    long timeout;


    settings_t() :
        index("log"),
        endpoints(std::vector<endpoint_type>({ defaults::endpoint })),
        sniffer({{ true, true }, 10, 60000 }),
        connections(20),
        retries(3),
        timeout(1000)
    {}
};

} // namespace elasticsearch

/*!
  rtm - retries <= max && retries != -1
  srt - sniff retries
  soe - sniff on error

  (1) rq -(rtm !)-> cb(fail).
         -(rtm v)-> succ -> cb(succ).
                 -> fail -(soe v)-> (2)sniff -> succ -> (1)
                                             -> fail -(srt v)-> (2)
                                                     -(srt !)-> cb(fail).
                         -(soe !)-> (1)

  TestCases:
  1. rq -> succ -> cb(succ).
  2. sniff.when.error=false, retries=0:
     rq -> fail -> cb(fail).
  3. sniff.when.error=true, sniff.retries=0, retries=0:
     rq -> fail -> sniff -> fail -> cb(fail).
  4.
  */
