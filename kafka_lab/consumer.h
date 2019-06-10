#pragma once
#include <string>
#include <vector>
#include <rdkafkacpp.h>
#include <rdkafka.h>
#include <iostream>

void sigterm(int sig);

struct protodata {
  uint64_t uuid;
  uint64_t position;
  uint64_t next_position;
  std::string gtid;
};

class MyEventCB : public RdKafka::EventCb {
public:

    void event_cb(RdKafka::Event &event);

};

void msg_consume(RdKafka::Message* message, void* opaque);

class MyConsumeCb : public RdKafka::ConsumeCb {
public:
    void consume_cb(RdKafka::Message& msg, void* opaque);
};


class ConsummerKafka {
public:
    ConsummerKafka();
    ~ConsummerKafka(){}

    int init_kafka(int partition, std::string brokers, std::string topic);
    int pull_data_from_kafka();
    void destory();

private:
    RdKafka::Conf* global_conf;
    RdKafka::Conf* topic_conf;
    std::string brokers;
    std::string errstr;
    RdKafka::Consumer* consumer;
    std::string topic_name;
    RdKafka::Topic *topic;
    int32_t partition;
    int64_t start_offset;
    RdKafka::Message* msg;
};
