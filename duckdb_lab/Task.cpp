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
        driver->ForceRollBack();
    }
    auto end = chrono::high_resolution_clock::now();
    std::cout << "THEREAD[" << std::this_thread::get_id() << "] TASK(" << this->context_->TypeStr() << ") TAKES ";
    std::cout << chrono::duration<double, std::milli>(end-start).count() << std::endl;
    /* std::cout << std::this_thread::get_id() << "[Release Connection] " << (void*)driver.get() << std::endl; */
    runner_->Release(driver);
}
