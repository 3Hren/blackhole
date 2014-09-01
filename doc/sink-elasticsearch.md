#`Elasticsearch`-sink

If you know what is it [Elasticsearch](http://www.elasticsearch.org/) you know why you need that. This sink sends formatted messages to the Elasticsearch cluster.

It is a thread-safe sink. At start it creates thread for message processing. Sink supports thread-save queue with configurable size. This queue stores messages. When the queue is fully filled or by the timer event sink generates bulk request to Ellasticsearch and clears queue. To connect to the Elasticsearch cluster and get it status sink uses predefined set of endpoints. At the request of status Elasticsearch replays with the list of all available nodes which will be used for load balancing. Sink use round-robin algorithm for this purpose. All of the operations with Elasticsearch sink performs asynchronousely.

To use sink include the special header

```
#include <blackhole/sink/elasticsearch.hpp>
```

##Registration

Registration code looks like follows:

```
repository_t::instance().configure<sink::elasticsearch_t, formatter::json_t>();
```

The code above register `elasticsearch`-sink and `JSON`-formatter pair. How to register another combinations of sinks and formatters check the ["Registration rules" article](registration-rules.md).

##Configuration

Example with the default settings

```
sink_config_t sink("elasticsearch");
sink["bulk"] = 100;
sink["interval"] = 1000;
sink["workers"] = 1;
sink["index"] = "logs";
sink["type"] = "log";
sink["endpoints"] = dynamic_t::array_t { "localhost:9200" };
sink["sniffer"]["when"]["start"] = true;
sink["sniffer"]["when"]["error"] = true;
sink["sniffer"]["interval"] = 60000;
sink["connections"] = 20;
sink["retries"] = 3;
sink["timeout"] = 1000;
```

###Parameters

|Parameter|Descriprion|
|---------|-----------|
|bulk|Maximun size of queue. After it reached sink generate bulk request to Elasticsearch.|
|interval|Interval (milliseconds) between bulk requests to Elasticsearch. Bulk requests sending doesn't depend on queue occupancy.|
|workers|Number of Elasticsearch clients in the sink. Each worker is a separate thread with own queue, timers and so on.|
|index|Name of Elasticsearch index wich will store your logs.|
|type|Name of Elasticsearch type inside the `index`.|
|endpoints|List of Elasticsearch nodes which is used to connect to the cluster.|
|sniffer|Settings for algorithm that keeps the Elasticsearch cluster state actual. It has the next parameters:|
||* `["when"]["start"]` - Get the state of cluster at start of sink if `true`.|
||* `["when"]["error"]` - Get the state of cluster on error if `true`. Error can be any, for example error of writing data to the Elasticsearch node.|
||* `["interval"] ` - Interval (milliseconds) of polling of Elasticsearch cluster status. It doesn't depend on `when` properties.|
|connections|Maximum number of connections to one Elasticsearch node. Each bulk request create a connection to node. When Elasticseach is under high load it may have a delay while request processing or it can be just fail on any reason. With this option we can create a queue of bulk requests to the cluster.|
|retries|If sink couldn't send bulk request for any reason it will try to send it again for `retries` times. Retries can be performed to different nodes, depending on nodes availability. After retries number exceeded request is dropped with the message to `stdout`.|
|timeout|Interval (millisecond) of waiting for the replay from Elasticsearch. In node doesn't reply in `timeout` request is treated as failed.|

##Example
In development.