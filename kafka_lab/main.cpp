#include "consumer.h"
#include "producer.h"

#include <iostream>
#include <rdkafka.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


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
