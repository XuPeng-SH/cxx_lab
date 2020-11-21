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

namespace aio = boost::asio;
using error_code = boost::system::error_code;
auto interval = boost::posix_time::seconds(1);

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
            if (terminated_) {
                throw std::runtime_error("Pool already terminated");
            }
            queue_.emplace([handler](){(*handler)();});
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

class Server {
 public:
     Server(aio::io_service* io, PoolPtr pool) : timer_(*io, interval), pool_(pool) {}

     bool
     Start() {
         std::cout << "Start server" << std::endl;
         timer_.async_wait(std::bind(&Server::Refresh, this, std::placeholders::_1));
     }

     void
     ShutDown() {
         boost::system::error_code ec;
         timer_.cancel();
         if (ec) {
             std::cout << "Fail to cancel: " << ec << std::endl;
         }
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
         pool_->Enqueue(f, 100);
         boost::system::error_code ec;
         auto new_expires = timer_.expires_at() + interval;
         timer_.expires_at(new_expires, ec);
         if (ec) {
             std::cout << "Fail to reschedule: " << ec << std::endl;
         }
         timer_.async_wait(std::bind(&Server::Refresh, this, std::placeholders::_1));
     }

     aio::deadline_timer timer_;
     PoolPtr pool_;
};

int main() {
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
    auto pool = std::make_shared<Pool>(5);
    aio::io_service io_service;
    Server server(&io_service, pool);
    server.Start();
    boost::system::error_code ec;
    io_service.run(ec);
    if (ec) {
        std::cout << "IO service run failed: " << ec << std::endl;
    }
    server.ShutDown();
    return 0;
}
