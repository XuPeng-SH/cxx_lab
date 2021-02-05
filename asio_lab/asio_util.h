#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <functional>

inline std::shared_ptr<boost::asio::deadline_timer> DoAfter(boost::asio::io_service& io_service,
        const std::function<void()>& fn, uint32_t ms) {
    auto timer = std::make_shared<boost::asio::deadline_timer>(io_service);
    timer->expires_from_now(boost::posix_time::milliseconds(ms));
    timer->async_wait([timer, fn](const boost::system::error_code& error) {
        if (error != boost::asio::error::operation_aborted && fn) {
            fn();
        }
    });

    return timer;
}
