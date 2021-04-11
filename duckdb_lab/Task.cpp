#include "Task.h"
#include <thread>
#include <iostream>
#include <chrono>

using namespace std;

void
Task::Run() {
    assert(this->context_->type_ != ContextType::INVALID);
    DriverPtr driver = nullptr;
    while(!driver) {
        driver = runner_->Use();
    }

    /* std::cout << std::this_thread::get_id() << "[Get     Connection] " << (void*)driver.get() << std::endl; */

    auto start = chrono::high_resolution_clock::now();
    try {
        if (this->context_->type_ == ContextType::DELIVERY) {
            driver->DoDelivery(this->context_);
        } else if (this->context_->type_ == ContextType::NEW_ORDER) {
            driver->DoNewOrder(this->context_);
        } else if (this->context_->type_ == ContextType::ORDER_STATUS) {
            driver->DoOrderStatus(this->context_);
        } else if (this->context_->type_ == ContextType::PAYMENT) {
            driver->DoPayment(this->context_);
        } else if (this->context_->type_ == ContextType::STOCK_LEVEL) {
            driver->DoStockLevel(this->context_);
        }
    } catch(std::exception& e) {
        std::cout << "catch error: " << e.what() << std::endl;
        driver->ForceRollBack(this->context_);
    }
    auto now = chrono::high_resolution_clock::now();
    MetricEntry entry;
    entry.is_commit_ = !this->context_->has_rollbacked_;
    entry.ts_ = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    entry.exec_time_us_ = chrono::duration<double, std::micro>(now-start).count();
    /* std::cout << "THEREAD[" << std::this_thread::get_id() << "] TASK(" << this->context_->TypeStr() << ") TAKES "; */
    /* std::cout << ((float)entry.exec_time_us_ / 1000) << std::endl; */

    if (this->context_->type_ == ContextType::DELIVERY) {
        this->context_->collector_.RecordDeliveryEntry(std::move(entry));
    } else if (this->context_->type_ == ContextType::NEW_ORDER) {
        this->context_->collector_.RecordNewOrderEntry(std::move(entry));
    } else if (this->context_->type_ == ContextType::ORDER_STATUS) {
        this->context_->collector_.RecordOrderEntry(std::move(entry));
    } else if (this->context_->type_ == ContextType::PAYMENT) {
        this->context_->collector_.RecordDeliveryEntry(std::move(entry));
    } else if (this->context_->type_ == ContextType::STOCK_LEVEL) {
        this->context_->collector_.RecordStockLevelEntry(std::move(entry));
    }

    /* std::cout << std::this_thread::get_id() << "[Release Connection] " << (void*)driver.get() << std::endl; */
    runner_->Release(driver);
}
