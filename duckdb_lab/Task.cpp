#include "Task.h"
#include <thread>
#include <iostream>

using namespace std;

void
DeliveryTask::Run() {
    DriverPtr driver = nullptr;
    while(!driver) {
        driver = runner_->Use();
    }

    /* std::cout << std::this_thread::get_id() << "[Get     Connection] " << (void*)driver.get() << std::endl; */

    try {
        driver->DoDelivery(this->context_);
    } catch(std::exception& e) {
        std::cout << "catch error: " << e.what() << std::endl;
    }
    /* std::cout << std::this_thread::get_id() << "[Release Connection] " << (void*)driver.get() << std::endl; */
    runner_->Release(driver);
}
