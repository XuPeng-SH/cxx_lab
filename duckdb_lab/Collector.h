#pragma once
#include <atomic>

struct Metric {
    std::string case_name;
    std::vector<double> commit_exec_time;
    std::vector<double> rollback_exec_time;
};

class Collector {
 public:

 protected:
    Metric delivery_metric_;
    Metric new_order_metric_;
    Metric order_metric_;
    Metric stock_level_metric_;
    Metric delivery_metric_;
};
