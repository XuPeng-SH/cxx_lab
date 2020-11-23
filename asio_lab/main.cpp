#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <functional>
#include <thread>
#include <chrono>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <future>
#include <csignal>

namespace aio = boost::asio;
using error_code = boost::system::error_code;
auto interval = boost::posix_time::milliseconds(100);

using Handler = std::function<void(void)>;

class Pool {
 public:
     Pool(size_t size = 1) {
         for (auto i = 0; i < size; ++i) {
             workers_.emplace_back(std::thread(std::bind(&Pool::WorkThread, this)));
         }
     }

     template <typename F, typename... Args>
     auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using RType = typename std::result_of<F(Args...)>::type;
        auto handler = std::make_shared<std::packaged_task<RType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<RType> fut = handler->get_future();
        {
            std::unique_lock<std::mutex> lck(mutex_);
            cond_.wait(lck, [this]{
                return terminated_ || (queue_.size() < 8);
            });
            if (terminated_) {
                throw std::runtime_error("Pool already terminated");
            }
            queue_.emplace([handler](){(*handler)();});
            std::cout << "Queue Size: " << queue_.size() << std::endl;
        }
        cond_.notify_one();
        return fut;
     }

     ~Pool() {
         {
             std::unique_lock<std::mutex> lck(mutex_);
             terminated_ = true;
         }
         cond_.notify_all();
         for (auto& w : workers_) {
             w.join();
         }
         std::cout << "Pool destoryed" << std::endl;
     }

 private:
     void
     WorkThread() {
         for(;;) {
            Handler handler;
            {
                std::unique_lock<std::mutex> lck(mutex_);
                std::cout << "WorkThread-" << std::this_thread::get_id() << std::endl;
                cond_.wait(lck, [this]{
                    return terminated_ || !queue_.empty();
                });

                if (terminated_) {
                    break;
                }
                handler = std::move(queue_.front());
                queue_.pop();
                cond_.notify_one();
            }
            handler();
         }
     }
     std::vector<std::thread> workers_;
     std::queue<Handler> queue_;
     std::condition_variable cond_;
     std::mutex mutex_;
     std::atomic_bool terminated_ = false;
};

using PoolPtr = std::shared_ptr<Pool>;

using THandlerT = std::function<void(const boost::system::error_code&)>;

struct TimerHandler {
    TimerHandler(aio::io_service& io, int interval_ms, THandlerT h, PoolPtr executors) :
        interval(interval_ms), timer(io, interval), handler(h), pool(executors) {}
    boost::posix_time::milliseconds interval;
    aio::deadline_timer timer;
    THandlerT handler;
    PoolPtr pool;

    void
    Reschedule(const boost::system::error_code& e) {
        pool->Enqueue(handler, e);
        boost::system::error_code ec;
        auto new_expires = timer.expires_at() + interval;
        timer.expires_at(new_expires, ec);
        if (ec) {
            std::cout << "Fail to Reschedule: " << ec << std::endl;
        }
        timer.async_wait(std::bind(&TimerHandler::Reschedule, this, std::placeholders::_1));
    }
};

using TimerHandlerPtr = std::shared_ptr<TimerHandler>;

class Server {
 public:
     Server(aio::io_service* io, PoolPtr pool) : io_(io), timer_(*io, interval), pool_(pool) {}

     void
     RegisterTimerHandler(int interval_ms, THandlerT h) {
         handlers_.emplace_back(std::make_shared<TimerHandler>(*io_, interval_ms, h, pool_));
     }

     bool
     Start() {
         std::cout << "Start server" << std::endl;
         /* timer_.async_wait(std::bind(&Server::Refresh, this, std::placeholders::_1)); */
         for (auto& th : handlers_) {
             th->timer.async_wait(std::bind(&TimerHandler::Reschedule, th, std::placeholders::_1));
         }
     }

     void
     ShutDown() {
         boost::system::error_code ec;
         timer_.cancel();
         if (ec) {
             std::cout << "Fail to cancel: " << ec << std::endl;
         }
         std::cout << "Executing ShutDown" << std::endl;
     }

 private:
     void
     Refresh(const boost::system::error_code& e) {
         std::cout << "Refresh ..." << std::endl;
         Reschedule();
     }
     void
     Reschedule() {
         std::cout << "Reschedule " << std::endl;
         auto f = [](int ms) {
            auto id = std::this_thread::get_id();
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            std::cout << "[" << id << "] Sleep for " << ms << " ms" << std::endl;
         };
         pool_->Enqueue(f, 800);
         boost::system::error_code ec;
         auto new_expires = timer_.expires_at() + interval;
         timer_.expires_at(new_expires, ec);
         if (ec) {
             std::cout << "Fail to reschedule: " << ec << std::endl;
         }
         timer_.async_wait(std::bind(&Server::Refresh, this, std::placeholders::_1));
     }

     aio::io_service* io_;
     aio::deadline_timer timer_;
     PoolPtr pool_;
     std::vector<TimerHandlerPtr> handlers_;
};

Handler signal_func = nullptr;

void
HandleSignal(int signum) {
    switch (signum) {
        default:
            std::cout << "Received Sig: " << signum << std::endl;
            if (signal_func) {
                signal_func();
            }
            exit(0);
    }
}

int main() {
    signal(SIGINT, HandleSignal);
    /* auto pool = Pool(2); */
    /* auto fut1 = pool.Enqueue([](int sec){ */
    /*     std::this_thread::sleep_for(std::chrono::seconds(sec)); */
    /*     std::cout << "Sleep " << sec << std::endl; */
    /* }, 1); */
    /* auto fut2 = pool.Enqueue([](int sec){ */
    /*     std::this_thread::sleep_for(std::chrono::seconds(sec)); */
    /*     std::cout << "Sleep " << sec << std::endl; */
    /* }, 1); */
    /* auto fut3 = pool.Enqueue([](int sec){ */
    /*     std::this_thread::sleep_for(std::chrono::seconds(sec)); */
    /*     std::cout << "Sleep " << sec << std::endl; */
    /* }, 1); */

    /* fut1.get(); */
    /* fut2.get(); */
    /* fut3.get(); */
    /* return 0; */
    auto pool = std::make_shared<Pool>(4);
    aio::io_service io_service;
    Server server(&io_service, pool);
    signal_func = [&server]() {
        server.ShutDown();
    };
    server.RegisterTimerHandler(100, [](const boost::system::error_code&) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::cout << "T1 EXE " << std::this_thread::get_id() << std::endl;
    });
    server.RegisterTimerHandler(100, [](const boost::system::error_code&) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::cout << "T2 EXE " << std::this_thread::get_id() << std::endl;
    });
    server.Start();
    boost::system::error_code ec;

    io_service.run(ec);
    if (ec) {
        std::cout << "IO service run failed: " << ec << std::endl;
    }
    server.ShutDown();
    return 0;
}
