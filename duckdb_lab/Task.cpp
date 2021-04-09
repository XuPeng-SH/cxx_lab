#include "Task.h"
#include <thread>
#include <iostream>

using namespace std;

void
Task::Run() {
    assert(this->context_->type_ != ContextType::INVALID);
    DriverPtr driver = nullptr;
    while(!driver) {
        driver = runner_->Use();
    }

    /* std::cout << std::this_thread::get_id() << "[Get     Connection] " << (void*)driver.get() << std::endl; */

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
        driver->ForceRollBack();
    }
    /* std::cout << std::this_thread::get_id() << "[Release Connection] " << (void*)driver.get() << std::endl; */
    runner_->Release(driver);
}
