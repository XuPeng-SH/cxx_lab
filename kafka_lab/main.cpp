#include "consumer.h"

#include <iostream>
#include <rdkafka.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

const int PRODUCER_INIT_FAILED = -1;
const int PRODUCER_INIT_SUCCESS = 0;

const int PUSH_DATA_FAILED = -1;
const int PUSH_DATA_SUCCESS = 0;

class ProducerKafka {
public:
    ProducerKafka() {};
    ~ProducerKafka() {};

    int init_kafka(int partition, char *brokers, const char *topic);
    int push_data_to_kafka(const char* buf, const int buf_len);
    void destroy();

private:
    int partition_;
    rd_kafka_t* handler_;
    rd_kafka_conf_t *conf_;

    rd_kafka_topic_t *topic_;
    rd_kafka_topic_conf_t *topic_conf_;
};

int ProducerKafka::init_kafka(int partition, char *brokers, const char *topic) {
    char tmp[16] = {0};
    char errstr[512] = {0};

    partition_ = partition;

    conf_ = rd_kafka_conf_new();

    snprintf(tmp, sizeof(tmp), "%i", SIGIO);
    rd_kafka_conf_set(conf_, "internal.termination.signal", tmp, NULL, 0);

    topic_conf_ = rd_kafka_topic_conf_new();

    if (!(handler_ = rd_kafka_new(RD_KAFKA_PRODUCER, conf_, errstr, sizeof(errstr)))) {
        fprintf(stderr, "***** Failed to create new producer: %s *****\n", errstr);
        return PRODUCER_INIT_FAILED;
    }

    if (rd_kafka_brokers_add(handler_, brokers) == 0) {
        fprintf(stderr, "***** No valid brokers specified *****\n");
        return PRODUCER_INIT_FAILED;

    }

    topic_ = rd_kafka_topic_new(handler_, topic, topic_conf_);

    return PRODUCER_INIT_SUCCESS;
}

void ProducerKafka::destroy() {
    rd_kafka_topic_destroy(topic_);

    rd_kafka_destroy(handler_);
}

int ProducerKafka::push_data_to_kafka(const char* buffer, const int buf_len) {
    int ret;
    char errstr[512] = {0};

    if(NULL == buffer) {
        return 0;
    }

    ret = rd_kafka_produce(topic_, partition_, RD_KAFKA_MSG_F_COPY, (void*)buffer, (size_t)buf_len, NULL, 0, NULL);

    if(ret == -1) {
        /* fprintf(stderr, "***** Failed to produce to topic %s partition %i: %s*****\n", rd_kafka_topic_name(topic_), partition_, rd_kafka_err2str(rd_kafka_errno2err(errno))); */
        rd_kafka_poll(handler_, 0);
        return PUSH_DATA_FAILED;
    }

    fprintf(stderr, "***** Sent %d bytes to topic:%s partition:%i*****\n", buf_len, rd_kafka_topic_name(topic_), partition_);
    rd_kafka_poll(handler_, 0);
    return PUSH_DATA_SUCCESS;

}

void as_consumer() {
    std::cout << "Consumer mode..." << std::endl;
    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);
    ConsummerKafka consumer;
    consumer.init_kafka(0, "localhost", "test2");
    consumer.pull_data_from_kafka();
}

using namespace std;
int main(int argc, char const *argv[]) {
    if (argc == 2) {
        as_consumer();
        return 0;
    }

    char test_data[100];
    strcpy(test_data, "helloworld");

    ProducerKafka* producer = new ProducerKafka;
    std::string topic = "test2";
    if (PRODUCER_INIT_SUCCESS == producer->init_kafka(0, "localhost:9092", topic.c_str())) {
        printf("producer init success\n");
    } else {
        printf("producer init failed\n");
        return 0;
    }

    while (fgets(test_data, sizeof(test_data), stdin)) {
        size_t len = strlen(test_data);
        if (test_data[len-1] == '\n') {
            test_data[--len] = '\0';
        }
        if (strcmp(test_data, "end") == 0)
            break;

        if (PUSH_DATA_SUCCESS == producer->push_data_to_kafka(test_data, strlen(test_data)))
            printf("push data success %s\n", test_data);
        else
            printf("push data failed %s\n", test_data);
    }

    producer->destroy();

    return 0;
}
