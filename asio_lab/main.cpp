#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <functional>
#include <thread>

namespace aio = boost::asio;
using error_code = boost::system::error_code;
auto interval = boost::posix_time::seconds(2);

class Server {
 public:
     Server(aio::io_service* io) : timer_(*io, interval) {}

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
         boost::system::error_code ec;
         auto new_expires = timer_.expires_at() + interval;
         timer_.expires_at(new_expires, ec);
         if (ec) {
             std::cout << "Fail to reschedule: " << ec << std::endl;
         }
         timer_.async_wait(std::bind(&Server::Refresh, this, std::placeholders::_1));
     }

     aio::deadline_timer timer_;
};

int main() {
    aio::io_service io_service;
    Server server(&io_service);
    server.Start();
    boost::system::error_code ec;
    io_service.run(ec);
    if (ec) {
        std::cout << "IO service run failed: " << ec << std::endl;
    }
    server.ShutDown();
    return 0;
}
