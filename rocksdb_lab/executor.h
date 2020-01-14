#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <memory>
#include <rocksdb/status.h>

#include "BlockingQueue.h"
#include "requests.h"

using ThreadPtr = std::shared_ptr<std::thread>;
using Queue = BlockingQueue<RequestPtr>;
using QueuePtr = std::shared_ptr<Queue>;

struct Executor {
    Executor(ThreadPtr t, QueuePtr q) : execute_thread(t), execute_queue(q) {}
    ThreadPtr execute_thread;
    QueuePtr execute_queue;
};

using ExecutorPtr = std::shared_ptr<Executor>;

class RequestExector {
public:
    using Ptr = std::shared_ptr<RequestExector>;
    RequestExector(size_t parallel = 16) : parallel_(parallel) {}

    void Start();
    void Stop();

    rocksdb::Status HandleRequest(RequestPtr request);

    ~RequestExector();

protected:
    void WorkingThread(QueuePtr queue);

    void Enqueue(RequestPtr request);

    mutable std::mutex mtx_;
    size_t parallel_;
    int current_ = 0;
    bool stopped_ = false;
    std::map<int, ExecutorPtr> executors_;
};

namespace lab {
    void request_exector_lab();
};
