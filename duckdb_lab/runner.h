#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <iostream>
#include "driver.h"

class Runner {
 public:
     void
     RegisterDriver(DriverPtr driver) {
         std::unique_lock<std::mutex> lk(mtx_);
         in_idle_.push_back(driver);
         /* in_idle_.insert(driver); */
     }

     DriverPtr
     Use() {
         std::unique_lock<std::mutex> lk(mtx_);
         if (in_idle_.size() == 0) return nullptr;
         auto ret = in_idle_.back();
         in_idle_.pop_back();
         /* in_use_.push_back(ret); */
         return ret;
     }

     void
     Release(DriverPtr driver) {
         std::unique_lock<std::mutex> lk(mtx_);
         /* for (auto it = in_use_.begin(); it != in_use_.end()) */
         /* auto ret = in_use_.back(); */
         /* in_use_.pop_back(); */
         in_idle_.push_back(driver);
     }

 protected:
     std::mutex mtx_;
     std::vector<DriverPtr> in_use_;
     std::vector<DriverPtr> in_idle_;
};

using RunnerPtr = std::shared_ptr<Runner>;
