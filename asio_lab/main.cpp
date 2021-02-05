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
#include <deque>
#include "asio_util.h"

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
using ThreadPoolPtr = std::shared_ptr<Pool>;

struct TimerContext {
    using HandlerT =std::function<void(const boost::system::error_code&)>;
    TimerContext(boost::asio::io_service& io, int interval_us, HandlerT& handler, ThreadPoolPtr pool)
        : io_(io), interval_(interval_us), handler_(handler), timer_(io, interval_), pool_(pool) {}

    void
    Reschedule(const boost::system::error_code& ec);

    boost::asio::io_service& io_;
    boost::posix_time::microseconds interval_;
    boost::asio::deadline_timer timer_;
    HandlerT handler_;
    ThreadPoolPtr pool_;
};

void
TimerContext::Reschedule(const boost::system::error_code& ec) {
    pool_->Enqueue(handler_, ec);
    boost::system::error_code e;
    auto new_expires = timer_.expires_at() + interval_;
    timer_.expires_at(new_expires, e);
    if (e) {
        std::cout << "Fail to Reschedule: " << e << std::endl;
    }
    timer_.async_wait(std::bind(&TimerContext::Reschedule, this, std::placeholders::_1));
}

using TimerContextPtr = std::shared_ptr<TimerContext>;

class Server {
 public:
     Server(aio::io_service* io, PoolPtr pool) : io_(io), timer_(*io, interval), pool_(pool) {}

     void
     RegisterTimerHandler(int interval_ms, TimerContext::HandlerT h) {
         handlers_.emplace_back(std::make_shared<TimerContext>(*io_, interval_ms, h, pool_));
     }

     bool
     Start() {
         std::cout << "Start server" << std::endl;
         /* timer_.async_wait(std::bind(&Server::Refresh, this, std::placeholders::_1)); */
         for (auto& th : handlers_) {
             th->timer_.async_wait(std::bind(&TimerContext::Reschedule, th, std::placeholders::_1));
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
     std::vector<TimerContextPtr> handlers_;
     /* std::vector<TimerHandlerPtr> handlers_; */
};

class Connection {
 public:
     Connection(aio::io_service& io_service)
         : io_service_(io_service),
           strand_(io_service),
           socket_(io_service) {
     }

     void
     Write(const std::string& message) {
         strand_.post(std::bind(&Connection::WriteImpl, this, message));
     }

 private:
     void
     WriteImpl(const std::string& message) {
         outbox_.push_back(message);
         if (outbox_.size() > 1) {
             return;
         }
         this->DoWrite();
     }

     void
     DoWrite() {
         const std::string& message = outbox_[0];
         std::cout << __func__ << ": " << message << std::endl;
         aio::async_write(
                 socket_,
                 aio::buffer(message.c_str(), message.size()),
                 strand_.wrap(std::bind(&Connection::WriteHandler, this,
                        std::placeholders::_1, std::placeholders::_2))
                 );
     }

     void
     WriteHandler(const boost::system::error_code& error, const size_t bytes_transferred) {
        outbox_.pop_front();
        if (error) {
            std::cerr << "could not write: " << boost::system::system_error(error).what() << std::endl;
            return;
        }
        if (!outbox_.empty()) {
            this->DoWrite();
        }
     }

     using OutBoxT = std::deque<std::string>;
     aio::io_service& io_service_;
     aio::io_service::strand strand_;
     aio::ip::tcp::socket socket_;
     OutBoxT outbox_;
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
    {
        std::cout << "Start DoAfter " << std::endl;
        aio::io_service io_service;
        auto fn = []() {
            std::cout << "This should be printed 200 ms later" << std::endl;
        };
        DoAfter(io_service, fn, 200);
        io_service.run();
    }
    return 0;
    {
        aio::io_service io_service;
        Connection foo( io_service );
        io_service.run();
    }
    {
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
    }
    return 0;
}
