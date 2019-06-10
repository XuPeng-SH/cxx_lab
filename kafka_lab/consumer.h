#pragma once
#include <string>
#include <vector>
#include <rdkafkacpp.h>
#include <rdkafka.h>
#include <iostream>

static bool run = true;
static bool exit_eof = true;

struct protodata {
  uint64_t uuid;
  uint64_t position;
  uint64_t next_position;
  std::string gtid;
};

static std::vector<protodata> fulltopic;


class MyEventCB : public RdKafka::EventCb {
public:

    void event_cb(RdKafka::Event &event) {
        switch (event.type()) {
            case RdKafka::Event::EVENT_ERROR:
                std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " <<
                    event.str() << std::endl;
                if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
                    run = false;
                break;

            case RdKafka::Event::EVENT_STATS:
                std::cerr << "\"STATS\": " << event.str() << std::endl;
                break;

            case RdKafka::Event::EVENT_LOG:
                fprintf(stderr, "LOG-%i-%s: %s\n", event.severity(), event.fac().c_str(), event.str().c_str());
                break;

            default:
                std::cerr << "EVENT " << event.type() <<
                    " (" << RdKafka::err2str(event.err()) << "): " <<
                    event.str() << std::endl;
                break;
        }
    }

};

void msg_consume(RdKafka::Message* message, void* opaque);

class MyConsumeCb : public RdKafka::ConsumeCb {
public:
    void consume_cb(RdKafka::Message& msg, void* opaque) {
        msg_consume(&msg, opaque);
    }
};

static void sigterm(int sig) {
    run = false;
}

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

ConsummerKafka::ConsummerKafka() {
}

int ConsummerKafka::init_kafka(int _partition, std::string broker, std::string _topic) {
    global_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    topic_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    brokers = broker;
    partition = _partition;

    topic_name = _topic;
    start_offset = RdKafka::Topic::OFFSET_BEGINNING;
    global_conf->set("metadata.broker.list", brokers, errstr);

    MyEventCB ex_event_cb;
    global_conf->set("event_cb", &ex_event_cb, errstr);

    consumer = RdKafka::Consumer::create(global_conf, errstr);
    if(!consumer) {
        std::cerr << "Failed to create consumer: " << errstr << std::endl;
        exit(1);
    }

    topic = RdKafka::Topic::create(consumer, topic_name, topic_conf, errstr);
    if (!topic) {
        std::cerr << "Failed to create topic: " << errstr << std::endl;
        exit(1);
    }
}

void ConsummerKafka::destory() {
    consumer->stop(topic, partition);
    consumer->poll(1000);
    delete topic;
    delete consumer;
}

int ConsummerKafka::pull_data_from_kafka() {
    RdKafka::ErrorCode resp = consumer->start(topic, partition, start_offset);
    if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Failed top start consumer: " <<
            RdKafka::err2str(resp) << std::endl;
        exit(1);
    }

    MyConsumeCb cb;
    int use_ccb = 0;
    while(run) {
        if(use_ccb) {}
        else {
            RdKafka::Message *msg = consumer->consume(topic, partition, 1000);
            msg_consume(msg, NULL);
            delete msg;
        }
        consumer->poll(0);
    }
}

void msg_consume(RdKafka::Message* message, void* opaque) {
    switch(message->err()) {
        case RdKafka::ERR__TIMED_OUT:
            break;

        case RdKafka::ERR_NO_ERROR:
            std::cout << "Read msg at offset " << message->offset() << std::endl;
            if (message->key()) {
                std::cout << "Key: " << *message->key() << std::endl;
            }
            std::cout << static_cast<const char*>(message->payload()) << std::endl;
            break;

        case RdKafka::ERR__PARTITION_EOF:
            std::cout << "reach last message" << std::endl;

            if(exit_eof) {
                run = false;
            }
            break;

        case RdKafka::ERR__UNKNOWN_TOPIC:
        case RdKafka::ERR__UNKNOWN_PARTITION:
            std::cerr << "Consume failed: " << message->errstr() << std::endl;
            run = false;

        default:
            std::cerr << "Consume failed: " << message->errstr() << std::endl;
            run = false;
    }
}
